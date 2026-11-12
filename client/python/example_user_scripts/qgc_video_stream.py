"""
Copyright (C) Microsoft Corporation.
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Streams a Project AirSim camera to QGroundControl (or any other H.264 video
player) as a live low-latency video feed.

The camera images are received through the normal pub/sub camera topic, encoded
to H.264 by FFmpeg and sent over UDP, either as RTP (QGC "UDP h.264 Video
Stream") or as MPEG-TS (QGC "MPEG-TS Video Stream"). No changes to the sim or
to the camera sensor are needed; the camera's image type only has to have
"capture-enabled": true in the robot config.

Requirements:
    FFmpeg on PATH (or pass --ffmpeg with the full path to the executable).

QGroundControl setup (default --transport rtp):
    Application Settings > General > Video
        Source:   UDP h.264 Video Stream
        UDP URL:  0.0.0.0:5600
        Low Latency Mode: on (QGC otherwise buffers the video for ~200 ms)

Examples:
    # Load scene_basic_drone.jsonc and stream DownCamera to QGC on this PC
    python qgc_video_stream.py

    # Attach to a scene that another script (e.g. px4_quadrotor.py) already
    # loaded, without reloading it, and stream to QGC on another machine
    python qgc_video_stream.py --scene-config scene_px4_sitl.jsonc --no-load-scene \
        --qgc-ip 192.168.1.50
"""

import argparse
import queue
import subprocess
import sys
import time

import cv2
import numpy as np

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import (
    load_scene_config_as_dict,
    projectairsim_log,
    unpack_image,
)

# Image types that are delivered as 8-bit images and can be encoded as video.
# Values are the topic names created by Robot.set_sensor_topics().
IMAGE_TYPE_TOPICS = {
    "scene": "scene_camera",
    "segmentation": "segmentation_camera",
    "depth-vis": "depth_vis_camera",
    "surface-normals": "surface_normals_camera",
}


def parse_args():
    parser = argparse.ArgumentParser(
        description="Stream a Project AirSim camera to QGroundControl over UDP."
    )
    parser.add_argument(
        "--qgc-ip",
        default="127.0.0.1",
        help="IP address of the machine running QGroundControl",
    )
    parser.add_argument("--port", type=int, default=5600, help="UDP video port")
    parser.add_argument(
        "--transport",
        choices=("rtp", "mpegts"),
        default="rtp",
        help="rtp: QGC 'UDP h.264 Video Stream', mpegts: QGC 'MPEG-TS Video Stream'",
    )
    parser.add_argument("--scene-config", default="scene_basic_drone.jsonc")
    parser.add_argument("--sim-config-path", default="sim_config/")
    parser.add_argument(
        "--no-load-scene",
        dest="load_scene",
        action="store_false",
        help="Attach to the scene that is already running in the sim instead of "
        "(re)loading --scene-config. --scene-config must match the running scene.",
    )
    parser.add_argument("--robot", default="Drone1", help="Robot actor name")
    parser.add_argument("--camera", default="DownCamera", help="Camera sensor ID")
    parser.add_argument(
        "--image-type", choices=sorted(IMAGE_TYPE_TOPICS), default="scene"
    )
    parser.add_argument("--fps", type=int, default=20, help="Output frame rate")
    parser.add_argument(
        "--width",
        type=int,
        default=0,
        help="Output width in pixels (0 = camera width, or keep aspect with --height)",
    )
    parser.add_argument(
        "--height",
        type=int,
        default=0,
        help="Output height in pixels (0 = camera height, or keep aspect with --width)",
    )
    parser.add_argument("--bitrate", default="2M", help="Target video bitrate")
    parser.add_argument(
        "--encoder",
        choices=("libx264", "h264_nvenc", "h264_qsv", "h264_amf"),
        default="libx264",
        help="H.264 encoder. Hardware encoders reduce CPU load.",
    )
    parser.add_argument("--ffmpeg", default="ffmpeg", help="FFmpeg executable")
    parser.add_argument(
        "--first-frame-timeout",
        type=float,
        default=10.0,
        help="Seconds to wait for the first camera image",
    )
    return parser.parse_args()


def attach_world(client, scene_config, sim_config_path):
    """Creates a World for an already loaded scene without reloading it."""
    config_dict, _ = load_scene_config_as_dict(scene_config, sim_config_path)
    world = World(client)
    world.sim_config = config_dict
    world.parent_topic = f"/Sim/{config_dict['id']}"
    world.home_geo_point = config_dict.get("home-geo-point")
    return world


def output_size(src_width, src_height, width, height):
    if width <= 0 and height <= 0:
        width, height = src_width, src_height
    elif width <= 0:
        width = round(src_width * height / src_height)
    elif height <= 0:
        height = round(src_height * width / src_width)
    # H.264 with yuv420p needs even dimensions.
    return max(2, width - width % 2), max(2, height - height % 2)


def to_bgr_frame(image_msg, width, height):
    frame = unpack_image(image_msg)
    if frame.ndim == 2 or frame.shape[2] == 1:
        frame = cv2.cvtColor(frame, cv2.COLOR_GRAY2BGR)
    elif frame.shape[2] == 4:
        frame = cv2.cvtColor(frame, cv2.COLOR_BGRA2BGR)
    if frame.shape[1] != width or frame.shape[0] != height:
        frame = cv2.resize(frame, (width, height), interpolation=cv2.INTER_AREA)
    return np.ascontiguousarray(frame, dtype=np.uint8)


def build_ffmpeg_command(args, width, height):
    gop = str(max(1, args.fps))  # about one keyframe per second so QGC can join quickly
    cmd = [
        args.ffmpeg,
        "-hide_banner",
        "-loglevel",
        "warning",
        # Frames are written as soon as they arrive, so stamp them with the
        # wall clock (1 ms time base) instead of assuming a constant rate.
        "-use_wallclock_as_timestamps",
        "1",
        "-f",
        "rawvideo",
        "-framerate",
        "1000",
        "-pix_fmt",
        "bgr24",
        "-s",
        f"{width}x{height}",
        "-i",
        "-",
        "-an",
        "-fps_mode",
        "passthrough",
        "-c:v",
        args.encoder,
    ]

    # Low-latency settings: no B-frames, lookahead or frame buffering.
    if args.encoder == "libx264":
        cmd += [
            "-preset",
            "ultrafast",
            "-tune",
            "zerolatency",
            "-profile:v",
            "baseline",
            # Repeat SPS/PPS so a player that connects mid-stream can decode.
            "-x264-params",
            "repeat-headers=1:scenecut=0",
        ]
    elif args.encoder == "h264_nvenc":
        cmd += ["-preset", "p1", "-tune", "ull", "-rc", "cbr", "-delay", "0", "-zerolatency", "1"]
    elif args.encoder == "h264_qsv":
        cmd += ["-async_depth", "1"]
    elif args.encoder == "h264_amf":
        cmd += ["-usage", "ultralowlatency"]

    cmd += [
        "-pix_fmt",
        "yuv420p",
        "-bf",
        "0",
        "-g",
        gop,
        "-b:v",
        args.bitrate,
        "-maxrate",
        args.bitrate,
        "-bufsize",
        args.bitrate,
        "-flush_packets",
        "1",
    ]

    if args.transport == "rtp":
        cmd += ["-f", "rtp", "-payload_type", "96", f"rtp://{args.qgc_ip}:{args.port}"]
    else:
        cmd += [
            "-muxdelay",
            "0",
            "-f",
            "mpegts",
            f"udp://{args.qgc_ip}:{args.port}?pkt_size=1316",
        ]
    return cmd


def put_latest(frame_queue, image_msg):
    """Keeps only the newest image so a slow encoder never builds up latency."""
    try:
        frame_queue.get_nowait()
    except queue.Empty:
        pass
    try:
        frame_queue.put_nowait(image_msg)
    except queue.Full:
        pass


def main():
    args = parse_args()
    if sys.platform == "win32":
        # The default ~15.6 ms Windows timer makes sleeps and queue timeouts
        # overshoot, which lowers the frame rate and adds latency.
        import ctypes

        ctypes.windll.winmm.timeBeginPeriod(1)

    client = ProjectAirSimClient()
    ffmpeg_proc = None
    topic = None

    try:
        client.connect()

        if args.load_scene:
            world = World(
                client,
                args.scene_config,
                delay_after_load_sec=2,
                sim_config_path=args.sim_config_path,
            )
        else:
            world = attach_world(client, args.scene_config, args.sim_config_path)

        drone = Drone(client, world, args.robot)

        topic_name = IMAGE_TYPE_TOPICS[args.image_type]
        topic = drone.sensors.get(args.camera, {}).get(topic_name)
        if topic is None:
            raise RuntimeError(
                f"Camera '{args.camera}' has no '{args.image_type}' capture enabled. "
                "Set \"capture-enabled\": true for that image type in the robot config."
            )

        latest = queue.Queue(maxsize=1)
        client.subscribe(topic, lambda _, image_msg: put_latest(latest, image_msg))
        projectairsim_log().info(f"Subscribed to {topic}")

        first_image = latest.get(timeout=args.first_frame_timeout)
        width, height = output_size(
            first_image["width"], first_image["height"], args.width, args.height
        )
        frame = to_bgr_frame(first_image, width, height)

        ffmpeg_cmd = build_ffmpeg_command(args, width, height)
        projectairsim_log().info(
            f"Streaming {args.camera}/{args.image_type} {width}x{height}@{args.fps} "
            f"to {args.qgc_ip}:{args.port} ({args.transport})"
        )
        projectairsim_log().info("FFmpeg command: " + " ".join(ffmpeg_cmd))
        ffmpeg_proc = subprocess.Popen(ffmpeg_cmd, stdin=subprocess.PIPE, bufsize=0)

        # Send each new image as soon as it arrives, at most --fps times per
        # second. If no new image arrives within one frame period (sim paused or
        # slower than --fps), repeat the last one so the stream stays alive.
        # Repeats do not delay the next real image.
        period = 1.0 / max(1, args.fps)
        last_write = last_image_write = 0.0
        frames_sent = 0
        last_log = time.monotonic()
        while True:
            if ffmpeg_proc.poll() is not None:
                raise RuntimeError(f"FFmpeg exited with code {ffmpeg_proc.returncode}")

            try:
                image_msg = latest.get(
                    timeout=max(0.0, last_write + period - time.monotonic())
                )
            except queue.Empty:
                image_msg = None

            if image_msg is not None:
                wait = last_image_write + period - time.monotonic()
                if wait > 0:
                    time.sleep(wait)
                    try:
                        image_msg = latest.get_nowait()  # use the newest image
                    except queue.Empty:
                        pass
                frame = to_bgr_frame(image_msg, width, height)

            ffmpeg_proc.stdin.write(frame.tobytes())
            last_write = time.monotonic()
            if image_msg is not None:
                last_image_write = last_write
            frames_sent += 1

            if last_write - last_log >= 10.0:
                projectairsim_log().info(f"Sent {frames_sent} frames")
                last_log = last_write

    except KeyboardInterrupt:
        projectairsim_log().info("Stopping video stream")

    except queue.Empty:
        projectairsim_log().error(
            f"No image received from '{topic}' within {args.first_frame_timeout} s. "
            "Is the sim running and the clock not paused?"
        )

    except FileNotFoundError:
        projectairsim_log().error(
            f"FFmpeg executable '{args.ffmpeg}' was not found. Install FFmpeg or "
            "pass --ffmpeg with its full path."
        )

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        if topic is not None:
            try:
                client.unsubscribe(topic)
            except Exception:
                pass
        if ffmpeg_proc is not None:
            try:
                ffmpeg_proc.stdin.close()
            except OSError:
                pass
            try:
                ffmpeg_proc.wait(timeout=2)
            except subprocess.TimeoutExpired:
                ffmpeg_proc.kill()
        client.disconnect()


if __name__ == "__main__":
    main()
