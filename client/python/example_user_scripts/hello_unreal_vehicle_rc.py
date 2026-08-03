"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Project AirSim Unreal vehicle example with arrow-key remote control.

Loads scene_unreal_vehicle.jsonc and drives the Unreal/Chaos vehicle using
the arrow keys (or WASD). Focus the OpenCV chase-camera window while driving.
Press Q or Esc to stop.
"""

import asyncio
import time

import cv2

from projectairsim import ProjectAirSimClient, World
from projectairsim.unreal_vehicle import UnrealVehicle
from projectairsim.utils import projectairsim_log, unpack_image


def set_controls(
    vehicle: UnrealVehicle,
    throttle: float = 0.0,
    brake: float = 0.0,
    steering: float = 0.0,
):
    vehicle.set_parameter("throttle", throttle)
    vehicle.set_parameter("brake", brake)
    vehicle.set_parameter("steering", steering)


def stop_vehicle(vehicle: UnrealVehicle):
    set_controls(vehicle, throttle=0.0, brake=0.0, steering=0.0)


def get_vector(data: dict, *path: str) -> dict:
    value = data
    for key in path:
        if not isinstance(value, dict):
            return {}
        value = value.get(key, {})
    return value if isinstance(value, dict) else {}


async def log_kinematics(vehicle: UnrealVehicle, label: str):
    kin = vehicle.get_kinematics()
    if not isinstance(kin, dict):
        projectairsim_log().info(f"{label}: {kin}")
        return

    pos = get_vector(kin, "pose", "position") or get_vector(kin, "position")
    vel = get_vector(kin, "twist", "linear") or get_vector(kin, "linear_velocity")

    projectairsim_log().info(
        f"{label}: "
        f"pos=({pos.get('x', 0):.2f}, {pos.get('y', 0):.2f}, {pos.get('z', 0):.2f}) "
        f"vel=({vel.get('x', 0):.2f}, {vel.get('y', 0):.2f}, {vel.get('z', 0):.2f})"
    )


class KeyHoldTracker:
    """Tracks held keys using repeated key events from OpenCV waitKeyEx."""

    _ARROW_UP = frozenset({65362, 2490368})
    _ARROW_DOWN = frozenset({65364, 2621440})
    _ARROW_LEFT = frozenset({65361, 2424832})
    _ARROW_RIGHT = frozenset({65363, 2555904})
    _QUIT_KEYS = frozenset({ord("q"), ord("Q"), 27})

    def __init__(self, hold_timeout_sec: float = 0.2):
        self._hold_timeout_sec = hold_timeout_sec
        self._active_keys: dict[str, float] = {}
        self._quit = False

    def poll(self, key_code: int):
        now = time.monotonic()

        if key_code in self._QUIT_KEYS:
            self._quit = True
        if key_code in self._ARROW_UP or key_code in (ord("w"), ord("W")):
            self._active_keys["up"] = now
        if key_code in self._ARROW_DOWN or key_code in (ord("s"), ord("S")):
            self._active_keys["down"] = now
        if key_code in self._ARROW_LEFT or key_code in (ord("a"), ord("A")):
            self._active_keys["left"] = now
        if key_code in self._ARROW_RIGHT or key_code in (ord("d"), ord("D")):
            self._active_keys["right"] = now

        expired = [
            key
            for key, last_seen in self._active_keys.items()
            if now - last_seen > self._hold_timeout_sec
        ]
        for key in expired:
            del self._active_keys[key]

    def is_pressed(self, key: str) -> bool:
        return key in self._active_keys

    def should_quit(self) -> bool:
        return self._quit


class VehicleRCConfig:
    """Tuning values for arrow-key vehicle control."""

    def __init__(
        self,
        throttle: float = 0.7,
        brake: float = 1.0,
        steering: float = 0.75,
        poll_interval_sec: float = 0.02,
        status_interval_sec: float = 2.0,
        key_hold_timeout_sec: float = 0.2,
        camera_sensor_id: str = "Chase",
        window_name: str = "Unreal Vehicle RC",
    ):
        self.throttle = throttle
        self.brake = brake
        self.steering = steering
        self.poll_interval_sec = poll_interval_sec
        self.status_interval_sec = status_interval_sec
        self.key_hold_timeout_sec = key_hold_timeout_sec
        self.camera_sensor_id = camera_sensor_id
        self.window_name = window_name


class ArrowKeyVehicleController:
    """Maps arrow keys to throttle, brake, and steering actuator commands."""

    def __init__(self, vehicle: UnrealVehicle, config: VehicleRCConfig | None = None):
        self.vehicle = vehicle
        self.config = config or VehicleRCConfig()
        self._running = False

    @staticmethod
    def print_controls(window_name: str):
        print(f"\n--- Arrow-Key Vehicle Control ({window_name} window) ---")
        print("Up / W:      throttle")
        print("Down / S:    brake")
        print("Left / A:    steer left")
        print("Right / D:   steer right")
        print("Q / Esc:     quit")
        print(f"Keep the '{window_name}' window focused while driving.")
        print("------------------------------------------------------\n")

    def _read_controls(self, key_tracker: KeyHoldTracker) -> tuple[float, float, float]:
        throttle = 0.0
        brake = 0.0
        steering = 0.0

        if key_tracker.is_pressed("up"):
            throttle = self.config.throttle
        elif key_tracker.is_pressed("down"):
            brake = self.config.brake

        if key_tracker.is_pressed("left"):
            steering = -self.config.steering
        elif key_tracker.is_pressed("right"):
            steering = self.config.steering

        return throttle, brake, steering

    async def run(self):
        self.print_controls(self.config.window_name)
        self._running = True
        elapsed_since_status = 0.0

        cur_image = [None]

        def on_image(_, image_msg):
            cur_image[0] = image_msg

        chase_topic = self.vehicle.sensors[self.config.camera_sensor_id]["scene_camera"]
        self.vehicle.client.subscribe(chase_topic, on_image)

        key_tracker = KeyHoldTracker(hold_timeout_sec=self.config.key_hold_timeout_sec)

        try:
            while self._running and not key_tracker.should_quit():
                if cur_image[0] is not None:
                    cv2.imshow(self.config.window_name, unpack_image(cur_image[0]))

                key_code = cv2.waitKeyEx(1)
                key_tracker.poll(key_code)

                throttle, brake, steering = self._read_controls(key_tracker)
                set_controls(self.vehicle, throttle, brake, steering)

                elapsed_since_status += self.config.poll_interval_sec
                if elapsed_since_status >= self.config.status_interval_sec:
                    await log_kinematics(self.vehicle, "Driving")
                    elapsed_since_status = 0.0

                await asyncio.sleep(self.config.poll_interval_sec)
        finally:
            self._running = False
            stop_vehicle(self.vehicle)
            cv2.destroyAllWindows()


async def main():
    client = ProjectAirSimClient()

    try:
        client.connect()
        projectairsim_log().info("Connected to Project AirSim")

        world = World(client, "scene_unreal_vehicle.jsonc", delay_after_load_sec=2)
        vehicle = UnrealVehicle(client, world, "UnrealVehicle")
        controller = ArrowKeyVehicleController(vehicle)

        await log_kinematics(vehicle, "Initial")
        await controller.run()
        await log_kinematics(vehicle, "Final")
        projectairsim_log().info("Done")

    except KeyboardInterrupt:
        projectairsim_log().info("Interrupted")
    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)
    finally:
        cv2.destroyAllWindows()
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())
