"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Flies a PX4 quadrotor with a gimbaled payload camera in a real-world location
(Google Photorealistic 3D Tiles streamed through Cesium for Unreal). The default
spawn is Seletar Airport's runway, Singapore.

- The GimbalCam is published as RTSP (rtsp://127.0.0.1:8554/gimbal by default)
  and announced to QGroundControl as a MAVLink camera, so QGC shows the video
  and its zoom controls work.
- The gimbal can be steered from QGC, or from the GimbalCam window: W/A/S/D to
  move, mouse wheel or +/- to zoom, C to centre, X to look straight down.
- Objects from config_spawned_objects.yml are placed on the ground; type `help`
  in this terminal for commands to spawn, list and delete objects while running.

Gimbal commands go through PX4's gimbal manager on the onboard payload MAVLink
link (PX4 sends to UDP 14030), the same path a companion-computer tracker would
use. Replace gimbal_setpoint() with your own tracking controller.

Usually started through run_cesium_sim.sh, which also launches the simulator,
PX4 and the RTSP server. To run it by hand:
  1. Start Project AirSim on the GISMap level with CESIUM_ION_TOKEN set.
  2. Start MediaMTX (RTSP server) and PX4 SITL with the gimbal enabled at boot:
       cd PX4-Autopilot
       PX4_PARAM_MNT_MODE_IN=4 PX4_PARAM_MNT_MODE_OUT=2 make px4_sitl none_iris
  3. python px4_gimbal_cesium.py [options]

Examples:
  python px4_gimbal_cesium.py                                  # Seletar runway demo
  python px4_gimbal_cesium.py --lat 1.3521 --lon 103.8198 --heading 90
  python px4_gimbal_cesium.py --qgc                            # fly from QGroundControl
  python px4_gimbal_cesium.py --scene scene_px4_gimbal_payload_blocks.jsonc
"""

import argparse
import asyncio
import json
import math
import os
import sys
import time
import urllib.request

import commentjson
import cv2

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log

from payload_mavlink import PayloadLink
from payload_video import CameraWindows, RtspStreamer, image_to_bgr, zoom_to_fov
from spawned_objects import CONSOLE_HELP, ObjectSpawner

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SIM_CONFIG_DIR = os.path.join(SCRIPT_DIR, "sim_config")
TEMPLATE_SCENE = "scene_px4_gimbal_cesium_seletar.jsonc"
GENERATED_SCENE = "scene_px4_gimbal_cesium_custom.jsonc"

# Seletar Airport runway 03/21 centreline, facing up the runway
DEFAULT_LAT, DEFAULT_LON, DEFAULT_HEADING = 1.413425, 103.865000, 33.0
DEFAULT_GROUND_ALT = 12.0  # m MSL at the default spawn

# Must match the GimbalCam settings in robot_quadrotor_px4_gimbal_payload.jsonc
GIMBAL_CAM_RESOLUTION = (1280, 720)
GIMBAL_CAM_HFOV_DEG = 70.0
GIMBAL_CAM_INITIAL_PITCH = -45.0
CONTINUOUS_ZOOM_RATE = 30.0  # percent per second while QGC's zoom button is held


def parse_args():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("--lat", type=float, help="spawn latitude (deg)")
    parser.add_argument("--lon", type=float, help="spawn longitude (deg)")
    parser.add_argument(
        "--heading", type=float, default=None, help="spawn heading (deg from north)"
    )
    parser.add_argument(
        "--ground-alt",
        type=float,
        default=None,
        help="ground elevation at the spawn (m MSL). Looked up online if omitted.",
    )
    parser.add_argument(
        "--spawn-height",
        type=float,
        default=15.0,
        help="spawn this far above --ground-alt; the drone then settles onto "
        "the real tile surface (default 15)",
    )
    parser.add_argument(
        "--geoid",
        type=float,
        default=8.1,
        help="geoid height at the spawn (m); ~8 across Singapore. Only affects "
        "how the Cesium origin lines up with MSL.",
    )
    parser.add_argument(
        "--objects",
        default=os.path.join(SCRIPT_DIR, "config_spawned_objects.yml"),
        help="YAML file of objects to place in the world ('' for none)",
    )
    parser.add_argument(
        "--qgc",
        action="store_true",
        help="load the world and wait; arm, fly missions and steer the gimbal "
        "from QGroundControl or the GimbalCam window instead of the scripted demo",
    )
    parser.add_argument(
        "--rtsp-url",
        default="rtsp://127.0.0.1:8554/gimbal",
        help="where to publish the GimbalCam video",
    )
    parser.add_argument("--no-rtsp", action="store_true", help="don't publish video")
    parser.add_argument(
        "--tile-wait",
        type=float,
        default=45.0,
        help="seconds to keep the sim paused while Cesium streams tiles (default 45)",
    )
    parser.add_argument(
        "--scene",
        help="load this scene config from sim_config/ as-is instead of building "
        "one from --lat/--lon",
    )
    return parser.parse_args()


def lookup_ground_alt(lat, lon):
    """Ground elevation (m MSL) from Open-Meteo's free Copernicus DEM service."""
    url = f"https://api.open-meteo.com/v1/elevation?latitude={lat}&longitude={lon}"
    try:
        with urllib.request.urlopen(url, timeout=10) as resp:
            return float(json.load(resp)["elevation"][0])
    except Exception as err:
        projectairsim_log().warning(
            f"Elevation lookup failed ({err}); pass --ground-alt. Assuming 0 m."
        )
        return 0.0


def build_scene(args):
    """Write a scene config for the requested spawn point and return its name."""
    with open(os.path.join(SIM_CONFIG_DIR, TEMPLATE_SCENE)) as f:
        scene = commentjson.load(f)

    if args.lat is None and args.lon is None:
        lat, lon = DEFAULT_LAT, DEFAULT_LON
        heading = DEFAULT_HEADING if args.heading is None else args.heading
        ground_alt = DEFAULT_GROUND_ALT if args.ground_alt is None else args.ground_alt
    elif args.lat is None or args.lon is None:
        raise SystemExit("--lat and --lon must be given together")
    else:
        lat, lon = args.lat, args.lon
        heading = 0.0 if args.heading is None else args.heading
        ground_alt = (
            lookup_ground_alt(lat, lon) if args.ground_alt is None else args.ground_alt
        )

    projectairsim_log().info(
        f"Spawn {lat:.6f}, {lon:.6f}, heading {heading:.0f} deg, ground {ground_alt:.1f} m MSL"
    )

    drone = scene["actors"][0]
    drone["origin"] = {
        "geo-point": f"{lat} {lon} {ground_alt + args.spawn_height}",
        "rpy-deg": f"0 0 {heading}",
    }
    scene["home-geo-point"] = {"latitude": lat, "longitude": lon, "altitude": ground_alt}
    scene["tiles-altitude-offset"] = args.geoid

    with open(os.path.join(SIM_CONFIG_DIR, GENERATED_SCENE), "w") as f:
        f.write("// Generated by px4_gimbal_cesium.py; overwritten on every run.\n")
        json.dump(scene, f, indent=2)
    return GENERATED_SCENE


def gimbal_setpoint(t):
    """Demo trajectory: slow pan while nodding between nadir and horizon."""
    pitch = -50.0 + 40.0 * math.sin(2 * math.pi * t / 12.0)
    yaw = 60.0 * math.sin(2 * math.pi * t / 20.0)
    return pitch, yaw


class Payload:
    """Ties the camera windows, RTSP stream, MAVLink camera and sim zoom together.

    Everything that talks to the sim runs on the asyncio thread via `tick()`.
    """

    def __init__(self, drone, link, rtsp):
        self.drone = drone
        self.link = link
        self.rtsp = rtsp
        self.zoom_percent = 0.0
        self.zoom_rate = 0.0
        self.applied_zoom = None
        self.last_gimbal_frame = None
        self.last_control_claim = 0.0
        self.windows = CameraWindows(
            on_gimbal=self.steer, on_zoom=self.nudge_zoom,
            initial_pitch=GIMBAL_CAM_INITIAL_PITCH,
        )

    def on_chase_image(self, _, image_msg):
        self.windows.receive("ChaseCam", image_to_bgr(image_msg))

    def on_gimbal_image(self, _, image_msg):
        bgr = image_to_bgr(image_msg)
        self.last_gimbal_frame = bgr
        self.windows.receive("GimbalCam", bgr)
        if self.rtsp:
            self.rtsp.push(bgr)

    def steer(self, pitch_deg, yaw_deg):
        # QGC can take gimbal control at any time; re-claim it when the user
        # starts steering from the keyboard again
        now = time.monotonic()
        if now - self.last_control_claim > 2.0:
            self.link.take_gimbal_control()
        self.last_control_claim = now
        self.link.set_gimbal_attitude(pitch_deg, yaw_deg)

    def nudge_zoom(self, delta_percent):
        self.zoom_percent = max(0.0, min(100.0, self.zoom_percent + delta_percent))

    def tick(self, dt):
        while not self.link.zoom_requests.empty():
            kind, value = self.link.zoom_requests.get()
            if kind == "range":
                self.zoom_percent, self.zoom_rate = value, 0.0
            else:
                self.zoom_rate = CONTINUOUS_ZOOM_RATE * value
        if self.zoom_rate:
            self.nudge_zoom(self.zoom_rate * dt)

        if self.applied_zoom is None or abs(self.zoom_percent - self.applied_zoom) > 0.5:
            fov = zoom_to_fov(GIMBAL_CAM_HFOV_DEG, self.zoom_percent)
            self.drone.set_field_of_view("GimbalCam", 0, fov)
            self.applied_zoom = self.zoom_percent
            self.windows.zoom_percent = self.zoom_percent
            self.link.publish_zoom(self.zoom_percent)

        while not self.link.capture_requests.empty():
            self.link.capture_requests.get()
            self.save_snapshot()

        self.windows.update()

    def save_snapshot(self):
        if self.last_gimbal_frame is None:
            return
        out_dir = os.path.expanduser("~/Pictures/projectairsim")
        os.makedirs(out_dir, exist_ok=True)
        path = os.path.join(out_dir, time.strftime("GimbalCam_%Y%m%d_%H%M%S.png"))
        cv2.imwrite(path, self.last_gimbal_frame)
        self.link.report_image_captured(path)
        projectairsim_log().info(f"Saved {path}")


async def run_ui(payload):
    """Keep the camera windows, zoom and QGC camera requests serviced."""
    last = time.monotonic()
    while True:
        now = time.monotonic()
        payload.tick(now - last)
        last = now
        await asyncio.sleep(0.015)


async def run_console(spawner):
    """Read object commands typed in the terminal."""
    loop = asyncio.get_running_loop()
    print(CONSOLE_HELP)
    while True:
        line = await loop.run_in_executor(None, sys.stdin.readline)
        if not line:  # stdin closed
            return
        spawner.run_command(line)


async def wait_while_sim_alive(world):
    """Return when the simulator stops answering (e.g. its window was closed)."""
    while True:
        await asyncio.sleep(2)
        try:
            world.get_sim_time()
        except Exception:
            projectairsim_log().info("Simulator closed")
            return


async def fly_gimbal_demo(drone, link):
    link.take_gimbal_control()
    drone.enable_api_control()

    # PX4 refuses to arm until its estimator has converged (a few seconds)
    for _ in range(60):
        if drone.arm():
            break
        z = drone.get_ground_truth_kinematics()["pose"]["position"]["z"]
        projectairsim_log().info(f"Waiting for PX4 to be ready to arm (NED z = {z:.2f} m)")
        await asyncio.sleep(1)
    else:
        raise RuntimeError("PX4 did not become ready to arm within 60 s")

    projectairsim_log().info("Takeoff")
    await (await drone.takeoff_async())

    projectairsim_log().info("Climb to 40 m")
    await (await drone.move_to_position_async(
        north=0.0, east=0.0, down=-40.0, velocity=4.0
    ))

    projectairsim_log().info("Sweeping gimbal for 60 s")
    t0 = time.monotonic()
    while (t := time.monotonic() - t0) < 60.0:
        pitch, yaw = gimbal_setpoint(t)
        link.set_gimbal_attitude(pitch, yaw)
        await asyncio.sleep(0.05)  # 20 Hz setpoint stream

    link.set_gimbal_attitude(-90.0, 0.0)

    projectairsim_log().info("Landing")
    await (await drone.land_async())
    drone.disarm()
    drone.disable_api_control()


async def main():
    args = parse_args()
    scene_name = args.scene or build_scene(args)
    is_cesium = args.scene is None or "cesium" in args.scene

    client = ProjectAirSimClient()
    rtsp = None
    link = None
    payload = None
    tasks = []

    try:
        client.connect()
        world = World(client, scene_name, delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")

        if not args.no_rtsp:
            rtsp = RtspStreamer(args.rtsp_url, GIMBAL_CAM_RESOLUTION)
        link = PayloadLink(
            args.rtsp_url if rtsp else "", GIMBAL_CAM_RESOLUTION, GIMBAL_CAM_HFOV_DEG
        )
        payload = Payload(drone, link, rtsp)
        client.subscribe(drone.sensors["Chase"]["scene_camera"], payload.on_chase_image)
        client.subscribe(drone.sensors["GimbalCam"]["scene_camera"], payload.on_gimbal_image)
        tasks.append(asyncio.create_task(run_ui(payload)))

        if is_cesium:
            # Cesium scenes start paused so the ground tiles (and their
            # collision) can stream in first. A location's first visit takes
            # longest; later runs read from Cesium's local cache.
            projectairsim_log().info(
                f"Waiting {args.tile_wait:.0f}s for Cesium tiles before resuming"
            )
            await asyncio.sleep(args.tile_wait)

        world.resume()

        # The spawn is above the ground, so the drone should drop and come to
        # rest. Still falling well below the home altitude means it went
        # through tiles that hadn't loaded yet.
        await asyncio.sleep(5)
        z = drone.get_ground_truth_kinematics()["pose"]["position"]["z"]
        if z > 20.0:
            raise RuntimeError(
                f"Drone fell through the Cesium ground (NED z = {z:.1f} m). The tiles "
                "probably hadn't loaded; run again (they are cached now) or raise "
                "--tile-wait."
            )

        link.wait_for_px4()

        # Ground lookups only see Cesium's collision once the sim is running
        spawner = ObjectSpawner(world, args.objects)
        spawner.spawn_from_config()
        tasks.append(asyncio.create_task(run_console(spawner)))

        if args.qgc:
            projectairsim_log().info(
                "World ready. Connect QGroundControl (UDP, listen 14550, server "
                "127.0.0.1:18570) to fly. Ctrl+C here or close the sim to stop."
            )
            await wait_while_sim_alive(world)
        else:
            await fly_gimbal_demo(drone, link)

    finally:
        for task in tasks:
            task.cancel()
        client.disconnect()
        if payload:
            payload.windows.close()
        if link:
            link.close()
        if rtsp:
            rtsp.close()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
    os._exit(0)  # don't wait on the console thread blocked reading stdin
