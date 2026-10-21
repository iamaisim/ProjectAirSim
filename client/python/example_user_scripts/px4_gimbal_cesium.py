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
- The GimbalCam's resolution, sensor and zoom lens come from
  camera_properties.json; zooming moves along the lens's focal-length range.
- --spotter opens a window to spawn and walk a human around the world.

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
import subprocess
import sys
import time
import urllib.request

import commentjson
import cv2

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.types import Pose, Quaternion, Vector3
from projectairsim.utils import load_scene_config_as_dict, projectairsim_log

from camera_properties import CameraProperties
from payload_mavlink import PayloadLink
from payload_video import CameraWindows, RtspStreamer, image_to_bgr
from px4_sitl import Px4Sitl
from spawned_objects import CONSOLE_HELP, ObjectSpawner

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SIM_CONFIG_DIR = os.path.join(SCRIPT_DIR, "sim_config")
TEMPLATE_SCENE = "scene_px4_gimbal_cesium_seletar.jsonc"
GENERATED_SCENE = "scene_px4_gimbal_cesium_custom.jsonc"
TEMPLATE_ROBOT = "robot_quadrotor_px4_gimbal_payload.jsonc"
GENERATED_ROBOT = "robot_quadrotor_px4_gimbal_payload_custom.jsonc"

# Seletar Airport runway 03/21 centreline, facing up the runway
DEFAULT_LAT, DEFAULT_LON, DEFAULT_HEADING = 1.413425, 103.865000, 33.0
DEFAULT_GROUND_ALT = 12.0  # m MSL at the default spawn

GIMBAL_CAM_INITIAL_PITCH = -45.0  # matches the gimbal origin in the robot config


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
        "--camera-props",
        default=os.path.join(SCRIPT_DIR, "camera_properties.json"),
        help="camera_properties.json for the GimbalCam (resolution, sensor, zoom lens)",
    )
    parser.add_argument(
        "--spotter",
        action="store_true",
        help="open the spotter window to spawn and walk a human in the world",
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
    parser.add_argument(
        "--rtsp-advertise-url",
        help="video URL announced to QGC and companion computers, if different "
        "from --rtsp-url (e.g. this laptop's LAN address)",
    )
    parser.add_argument("--no-rtsp", action="store_true", help="don't publish video")
    parser.add_argument(
        "--tile-wait",
        type=float,
        default=90.0,
        help="longest time to keep the sim paused while Cesium streams the ground "
        "under the drone (default 90)",
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


def build_robot(camera):
    """Write the payload robot config with this camera and return its name."""
    with open(os.path.join(SIM_CONFIG_DIR, TEMPLATE_ROBOT)) as f:
        robot = commentjson.load(f)
    gimbal_cam = next(s for s in robot["sensors"] if s["id"] == "GimbalCam")
    camera.apply_to_camera_config(gimbal_cam)
    with open(os.path.join(SIM_CONFIG_DIR, GENERATED_ROBOT), "w") as f:
        f.write("// Generated by px4_gimbal_cesium.py; overwritten on every run.\n")
        json.dump(robot, f, indent=2)
    return GENERATED_ROBOT


def build_scene(args, robot_config):
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
    drone["robot-config"] = robot_config

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

    def __init__(self, link, rtsp, camera):
        self.drone = None
        self.link = link
        self.rtsp = rtsp
        self.camera = camera
        self.zoom_speed = float(
            camera.props.get("zoom", {}).get("zoom_speed_percent_per_s", 30.0)
        )
        self.zoom_percent = 0.0
        self.zoom_rate = 0.0
        self.applied_zoom = None
        self.last_gimbal_frame = None
        self.last_control_claim = 0.0
        self.windows = CameraWindows(
            on_gimbal=self.steer, on_zoom=self.nudge_zoom,
            initial_pitch=GIMBAL_CAM_INITIAL_PITCH,
        )

    def attach(self, client, drone):
        """Follow a (re)loaded drone: subscribe to its cameras and reapply zoom."""
        self.drone = drone
        self.applied_zoom = None
        self.windows.pitch, self.windows.yaw = GIMBAL_CAM_INITIAL_PITCH, 0.0
        client.subscribe(drone.sensors["Chase"]["scene_camera"], self.on_chase_image)
        client.subscribe(drone.sensors["GimbalCam"]["scene_camera"], self.on_gimbal_image)

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
                self.zoom_rate = self.zoom_speed * value
        if self.zoom_rate:
            self.nudge_zoom(self.zoom_rate * dt)

        if self.drone and (
            self.applied_zoom is None or abs(self.zoom_percent - self.applied_zoom) > 0.5
        ):
            focal = self.camera.focal_length(self.zoom_percent)
            self.drone.set_field_of_view("GimbalCam", 0, self.camera.hfov_deg(focal))
            self.applied_zoom = self.zoom_percent
            self.windows.zoom_text = (
                f"{focal:.1f} mm ({focal / self.camera.focal_min_mm:.1f}x)"
            )
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
        try:
            payload.tick(now - last)
        except Exception as err:  # e.g. the scene is reloading under us
            projectairsim_log().debug(f"UI tick skipped: {err}")
        last = now
        await asyncio.sleep(0.015)


GOTO_HELP = """  goto LAT LON [HEADING]                move the drone to a new place (restarts PX4)"""


async def run_console(session):
    """Read commands typed in the terminal."""
    loop = asyncio.get_running_loop()
    print(CONSOLE_HELP + "\n" + GOTO_HELP)
    while True:
        line = await loop.run_in_executor(None, sys.stdin.readline)
        if not line:  # stdin closed
            return
        parts = line.split()
        if parts and parts[0].lower() == "goto":
            try:
                lat, lon = float(parts[1]), float(parts[2])
                heading = float(parts[3]) if len(parts) > 3 else 0.0
            except (IndexError, ValueError):
                print("Usage: goto LAT LON [HEADING]")
                continue
            if not session.args.qgc:
                print("goto is only available in --qgc mode")
                continue
            try:
                await session.load(lat, lon, heading)
            except Exception as err:
                print(f"goto failed: {err}")
        else:
            session.spawner.run_command(line)


async def wait_for_ground(world, max_wait, min_wait=10.0, stable_reads=3):
    """Ground height (NED down) under the drone once Cesium's collision there
    has loaded and stopped changing, or None after max_wait seconds."""
    projectairsim_log().info("Waiting for the Cesium ground under the drone to load")
    t0 = time.monotonic()
    last, stable = None, 0
    while time.monotonic() - t0 < max_wait:
        z = world.get_surface_elevation_at_point(0.0, 0.0)
        if z is not None and not math.isnan(z):
            stable = stable + 1 if last is not None and abs(z - last) < 0.05 else 0
            last = z
            if stable >= stable_reads and time.monotonic() - t0 >= min_wait:
                projectairsim_log().info(
                    f"Ground ready after {time.monotonic() - t0:.0f}s (NED z = {z:.2f} m)"
                )
                return z
        await asyncio.sleep(1)
    return None


def place_drone_above_ground(drone, ground_down, clearance=0.5):
    pose = drone.get_ground_truth_kinematics()["pose"]
    drone.set_pose(
        Pose(
            {
                "translation": Vector3({"x": 0.0, "y": 0.0, "z": ground_down - clearance}),
                "rotation": Quaternion(pose["orientation"]),
                "frame_id": "DEFAULT_ID",
            }
        ),
        True,
    )


async def wait_while_sim_alive(session):
    """Return when the simulator stops answering (e.g. its window was closed)."""
    failures = 0
    while failures < 3:
        await asyncio.sleep(2)
        if session.loading:
            continue
        try:
            session.world.get_sim_time()
            failures = 0
        except Exception:
            failures += 1
    projectairsim_log().info("Simulator closed")


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


class Session:
    """The running world: loads a location, (re)starts PX4 for it and keeps the
    camera windows, objects and spotter following the current scene."""

    def __init__(self, args, client, camera, payload, link):
        self.args = args
        self.client = client
        self.camera = camera
        self.payload = payload
        self.link = link
        self.px4 = Px4Sitl()
        self.robot_config = None if args.scene else build_robot(camera)
        self.world = None
        self.drone = None
        self.spawner = None
        self.spotter = None
        self.loading = False

    async def load(self, lat=None, lon=None, heading=None):
        """Load the scene at a location (the --lat/--lon ones if not given)."""
        args = self.args
        if lat is not None:
            args.lat, args.lon, args.heading, args.ground_alt = lat, lon, heading, None
        scene_name = args.scene or build_scene(args, self.robot_config)
        is_cesium = args.scene is None or "cesium" in args.scene

        self.loading = True
        try:
            self._stop_spotter()
            # Load the scene first: it drops the old PX4's connection cleanly.
            # Then start a fresh PX4, which connects to the new scene.
            if self.world is None:
                self.world = World(self.client, scene_name, delay_after_load_sec=2)
            else:
                projectairsim_log().info(f"Moving to {lat:.6f}, {lon:.6f}")
                config, _ = load_scene_config_as_dict(scene_name, sim_config_path=SIM_CONFIG_DIR)
                self.world.load_scene(config, delay_after_load_sec=2)
            self.px4.stop()
            self.px4.start()

            self.drone = Drone(self.client, self.world, "Drone1")
            self.payload.attach(self.client, self.drone)

            if is_cesium:
                # Cesium scenes start paused so the ground tiles (and their
                # collision) can stream in first. Wait until the ground under
                # the drone is solid, then set the drone just above it.
                ground = await wait_for_ground(self.world, args.tile_wait)
                if ground is None:
                    projectairsim_log().warning(
                        "No stable ground under the drone yet; resuming anyway"
                    )
                else:
                    place_drone_above_ground(self.drone, ground)

            self.world.resume()

            # The drone should come to rest on the ground. Still falling well
            # below the home altitude means it went through tiles that hadn't
            # loaded yet.
            await asyncio.sleep(5)
            z = self.drone.get_ground_truth_kinematics()["pose"]["position"]["z"]
            if z > 20.0:
                raise RuntimeError(
                    f"Drone fell through the Cesium ground (NED z = {z:.1f} m). The "
                    "tiles probably hadn't loaded; try again (they are cached now) or "
                    "raise --tile-wait."
                )

            self.link.wait_for_px4()

            # Ground lookups only see Cesium's collision once the sim is running
            self.spawner = ObjectSpawner(self.world, args.objects)
            self.spawner.spawn_from_config()

            if args.spotter:
                self.spotter = subprocess.Popen(
                    [sys.executable, os.path.join(SCRIPT_DIR, "spotter_control.py"),
                     "--scene-file", os.path.join(SIM_CONFIG_DIR, scene_name)]
                )
        finally:
            self.loading = False

    def _stop_spotter(self):
        if self.spotter:
            self.spotter.terminate()
            self.spotter = None

    def close(self):
        self._stop_spotter()
        self.px4.stop()


async def main():
    args = parse_args()
    camera = CameraProperties(args.camera_props if os.path.exists(args.camera_props) else None)
    projectairsim_log().info(f"GimbalCam {camera.describe()}")

    client = ProjectAirSimClient()
    rtsp = None
    link = None
    payload = None
    session = None
    tasks = []

    try:
        client.connect()
        if not args.no_rtsp:
            rtsp = RtspStreamer(args.rtsp_url, camera.resolution)
        advertised_url = args.rtsp_advertise_url or args.rtsp_url
        link = PayloadLink(advertised_url if rtsp else "", camera)
        payload = Payload(link, rtsp, camera)
        tasks.append(asyncio.create_task(run_ui(payload)))

        session = Session(args, client, camera, payload, link)
        await session.load()
        tasks.append(asyncio.create_task(run_console(session)))

        if args.qgc:
            projectairsim_log().info(
                "World ready. Connect QGroundControl (UDP, listen 14550, server "
                "127.0.0.1:18570) to fly. Type 'goto LAT LON [HEADING]' to move. "
                "Ctrl+C here or close the sim to stop."
            )
            await wait_while_sim_alive(session)
        else:
            await fly_gimbal_demo(session.drone, link)

    finally:
        for task in tasks:
            task.cancel()
        if session:
            session.close()
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
