# Camera Video Stream to QGroundControl

## Overview

QGroundControl (QGC) can show a live video feed next to the map, but it only accepts a few stream types, for example RTP/H.264 or MPEG-TS over UDP, or RTSP. It cannot play the WebRTC stream used by [Camera Streaming](camera_streaming.md) (Unreal PixelStreaming), and the `qgc-host-ip`/`qgc-port` robot settings only forward MAVLink telemetry, not video.

The example script `client/python/example_user_scripts/qgc_video_stream.py` bridges this gap without any change to the sim:

1. It subscribes to a camera image topic through the normal Project AirSim pub/sub API.
2. It pipes the raw frames into FFmpeg, which encodes them as low-latency H.264.
3. FFmpeg sends the video over UDP as RTP (default) or MPEG-TS to the machine running QGC.

The script only keeps the newest camera image and sends it as soon as it arrives, so a slow encoder or network never builds up latency (see the Latency section below). While the sim is paused, it repeats the last frame so the stream stays open in QGC.

This is independent of MAVLink. QGC can be connected to PX4/ArduPilot at the same time, or not at all.

## Requirements

- A running Project AirSim sim (Unreal Editor or packaged build).
- The Project AirSim Python client (see [Client Setup](../client_setup.md)).
- [FFmpeg](https://ffmpeg.org/download.html) on `PATH`, or pass its full path with `--ffmpeg`.
- A camera image type with `"capture-enabled": true` in the robot config. Supported image types are `scene`, `segmentation`, `depth-vis` and `surface-normals`. For example, `DownCamera` in `robot_quadrotor_fastphysics.jsonc` and `robot_quadrotor_px4_sitl.jsonc` already has its scene image enabled.

## QGroundControl setup

In **Application Settings > General > Video**:

| `--transport` | QGC Source | QGC setting |
|---|---|---|
| `rtp` (default) | UDP h.264 Video Stream | UDP URL `0.0.0.0:5600` |
| `mpegts` | MPEG-TS Video Stream | UDP URL `0.0.0.0:5600` |

Port `5600` is QGC's default; use `--port` if you change it. If QGC runs on another machine, allow inbound UDP on that port in its firewall. For the lowest delay, also enable **Low Latency Mode** on the same page (see the Latency section below).

## Running the stream

Run the script from `client/python/example_user_scripts`.

**Standalone.** Load `scene_basic_drone.jsonc` and stream `DownCamera` to QGC on the same PC:

```
python qgc_video_stream.py
```

**Together with a flight script.** If another script (for example `px4_quadrotor.py`) already loaded the scene, attach to it without reloading it. `--scene-config` must name the scene that is currently loaded, so the script knows its robots and cameras:

```
python qgc_video_stream.py --scene-config scene_px4_sitl.jsonc --no-load-scene --qgc-ip 192.168.1.50
```

**Other camera, size or encoder.** Stream the chase camera at 640 px wide (height keeps the aspect ratio) with the NVIDIA hardware encoder:

```
python qgc_video_stream.py --no-load-scene --camera Chase --width 640 --encoder h264_nvenc
```

The video should appear in QGC within about a second; a keyframe is sent every second, so QGC can join at any time.

## Options

| Option | Default | Description |
|---|---|---|
| `--qgc-ip` | `127.0.0.1` | IP address of the machine running QGC |
| `--port` | `5600` | UDP video port |
| `--transport` | `rtp` | `rtp` or `mpegts` (see the table above) |
| `--scene-config` | `scene_basic_drone.jsonc` | Scene config file name |
| `--sim-config-path` | `sim_config/` | Folder that contains the scene config |
| `--no-load-scene` | off | Attach to the already loaded scene instead of loading `--scene-config` |
| `--robot` | `Drone1` | Robot actor name |
| `--camera` | `DownCamera` | Camera sensor ID |
| `--image-type` | `scene` | `scene`, `segmentation`, `depth-vis` or `surface-normals` |
| `--fps` | `20` | Output frame rate |
| `--width`, `--height` | `0` | Output size. `0` keeps the camera size; set one of them to keep the aspect ratio |
| `--bitrate` | `2M` | Target video bitrate |
| `--encoder` | `libx264` | `libx264`, `h264_nvenc`, `h264_qsv` or `h264_amf` |
| `--ffmpeg` | `ffmpeg` | FFmpeg executable |

## Latency

The script itself adds very little delay: each new image is sent as soon as it arrives, frames are stamped with the wall clock, and all encoders are configured without B-frames, lookahead or frame buffering. Most of the delay you see is on the QGC side.

Measured on Windows with the default `libx264` encoder at 20 fps, 640x360:

| Stage | Delay |
|---|---|
| Sim: camera capture to image arriving in the client | 3â€“6 ms (sim time) |
| Script: image arrival to FFmpeg (incl. resize), `--fps` â‰¥ camera rate | ~5 ms median |
| FFmpeg encode + UDP send | ~3 ms |
| QGC end to end, default settings | ~360 ms |
| QGC end to end, **Low Latency Mode** on | ~170 ms |

To minimize latency:

- In QGC, enable **Application Settings > General > Video > Low Latency Mode**. By default QGC buffers incoming RTP video in a jitter buffer (200 ms by default in GStreamer) and syncs frames to their timestamps. Low Latency Mode removes both. This is the largest single improvement and is useful for any QGC video source, not only this one.
- Keep `--fps` at or above the camera rate (`1 / capture-interval`). If `--fps` is lower, the script has to skip images and the image it sends can be up to one camera interval old.
- On a local network, latency is dominated by QGC buffering and display, not by the network.

## Performance notes

- Every captured frame is copied from the GPU and sent to the client, so the camera's capture `width`/`height` and the sensor's `capture-interval` set the cost on the sim side. Choose a capture resolution close to the stream resolution instead of downscaling a large image.
- A hardware encoder (`h264_nvenc`, `h264_qsv`, `h264_amf`) reduces CPU load. It must be supported by your GPU and FFmpeg build.
- The stream FPS is independent of the camera rate. If the camera delivers fewer images than `--fps`, the last frame is repeated to fill the gaps; new images are still sent immediately.
- On Windows, the script raises the system timer resolution to 1 ms while it runs, so frame pacing is accurate.

## Using RTSP instead

QGC can also play RTSP streams. To publish an RTSP stream, run an RTSP server such as [MediaMTX](https://github.com/bluenviron/mediamtx), change the FFmpeg output in `build_ffmpeg_command()` to `-f rtsp rtsp://<server>:8554/airsim`, and set the QGC source to **RTSP Video Stream** with that URL. For a single QGC on the local network, the default UDP stream needs no extra server and has lower latency.

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI CONSULTING CORP

MIT License. All rights reserved.
