"""Load the Rascal PX4 scene and validate a runway takeoff."""

import argparse
import math
import time
from pathlib import Path

import commentjson

from projectairsim import ProjectAirSimClient
from projectairsim.utils import load_scene_config_as_dict, projectairsim_log


SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_CONFIG_DIR = SCRIPT_DIR / "sim_config"
SCENE_NAME = "scene_rascal110_px4_sitl.jsonc"
ROBOT_NAME = "rascal110_px4"
TARGET_ALTITUDE_GAIN_M = 100.0
MAX_WALL_TIME_SEC = 360.0
SAMPLE_PERIOD_SEC = 0.1
SENSOR_ENDPOINTS = {
    "imu": "IMU1/imu_kinematics",
    "gps": "GPS/gps",
    "barometer": "Barometer/barometer",
    "magnetometer": "Magnetometer/magnetometer",
    "airspeed": "Airspeed/airspeed",
}


def request(client, method: str, params=None):
    return client.request(
        {"method": method, "params": params or {}, "version": 1.0}
    )


def xyz(message: dict, group: str, field: str) -> tuple[float, float, float]:
    value = message[group][field]
    return value["x"], value["y"], value["z"]


def subtract(left, right):
    return tuple(a - b for a, b in zip(left, right))


def add(left, right):
    return tuple(a + b for a, b in zip(left, right))


def scale(values, factor: float):
    return tuple(value * factor for value in values)


def norm(values) -> float:
    return math.sqrt(sum(value * value for value in values))


def finite_message(message: dict) -> bool:
    values = (
        *xyz(message, "pose", "position"),
        *xyz(message, "twist", "linear"),
        *xyz(message, "twist", "angular"),
        *xyz(message, "accels", "linear"),
        *xyz(message, "accels", "angular"),
    )
    return all(math.isfinite(value) for value in values)


def numeric_values(value):
    if isinstance(value, dict):
        for child in value.values():
            yield from numeric_values(child)
    elif isinstance(value, list):
        for child in value:
            yield from numeric_values(child)
    elif isinstance(value, (int, float)):
        yield float(value)


def load_scene(
    client: ProjectAirSimClient, config_dir: Path, scene_name: str
) -> str:
    scene, _ = load_scene_config_as_dict(scene_name, str(config_dir), -1)
    return request(
        client,
        "/Sim/LoadScene",
        {"scene_config": commentjson.dumps(scene)},
    )


def quaternion_to_rpy(orientation: dict) -> tuple[float, float, float]:
    w = orientation["w"]
    x = orientation["x"]
    y = orientation["y"]
    z = orientation["z"]
    roll = math.atan2(
        2.0 * (w * x + y * z),
        1.0 - 2.0 * (x * x + y * y),
    )
    pitch_argument = 2.0 * (w * y - z * x)
    pitch = math.asin(max(-1.0, min(1.0, pitch_argument)))
    yaw = math.atan2(
        2.0 * (w * z + x * y),
        1.0 - 2.0 * (y * y + z * z),
    )
    return roll, pitch, yaw


def angle_difference(left: float, right: float) -> float:
    return math.atan2(math.sin(left - right), math.cos(left - right))


def capture_flight(
    client: ProjectAirSimClient,
    robot_path: str,
) -> tuple[list[dict], dict[str, list[dict]]]:
    samples = []
    sensor_samples = {name: [] for name in SENSOR_ENDPOINTS}
    wall_deadline = time.monotonic() + MAX_WALL_TIME_SEC
    initial_z = None
    liftoff_timestamp = None

    while time.monotonic() < wall_deadline:
        sample = request(client, f"{robot_path}/GetGroundTruthKinematics")
        if not finite_message(sample):
            raise RuntimeError("Project AirSim returned non-finite kinematics")
        if samples and sample["time_stamp"] == samples[-1]["time_stamp"]:
            time.sleep(SAMPLE_PERIOD_SEC)
            continue

        sample["wall_time"] = time.monotonic()
        samples.append(sample)
        if initial_z is None:
            initial_z = xyz(sample, "pose", "position")[2]

        for name, endpoint in SENSOR_ENDPOINTS.items():
            sensor_samples[name].append(
                request(client, f"{robot_path}/sensors/{endpoint}")
            )

        position = xyz(sample, "pose", "position")
        velocity = xyz(sample, "twist", "linear")
        altitude_gain = initial_z - position[2]
        horizontal_speed = norm(velocity[:2])
        if (
            liftoff_timestamp is None
            and altitude_gain >= 3.0
            and horizontal_speed >= 10.0
        ):
            liftoff_timestamp = sample["time_stamp"]
            projectairsim_log().info(
                "Rascal liftoff detected: %.1f m/s, %.1f m altitude gain",
                horizontal_speed,
                altitude_gain,
            )
        if (
            liftoff_timestamp is not None
            and altitude_gain >= TARGET_ALTITUDE_GAIN_M
            and sample["time_stamp"] - liftoff_timestamp >= 10_000_000_000
        ):
            projectairsim_log().info(
                "Rascal reached %.1f m altitude gain", altitude_gain
            )
            break
        time.sleep(SAMPLE_PERIOD_SEC)

    return samples, sensor_samples


def analyze(samples: list[dict], sensor_samples: dict[str, list[dict]]) -> dict:
    if len(samples) < 10:
        raise RuntimeError("Not enough Project AirSim samples")
    initial_position = xyz(samples[0], "pose", "position")
    initial_z = initial_position[2]
    roll_start_index = next(
        (
            index
            for index, sample in enumerate(samples)
            if norm(xyz(sample, "twist", "linear")[:2]) >= 5.0
        ),
        None,
    )
    liftoff_index = next(
        (
            index
            for index, sample in enumerate(samples)
            if initial_z - xyz(sample, "pose", "position")[2] >= 3.0
            and norm(xyz(sample, "twist", "linear")[:2]) >= 10.0
        ),
        None,
    )
    target_index = next(
        (
            index
            for index, sample in enumerate(samples)
            if initial_z - xyz(sample, "pose", "position")[2]
            >= TARGET_ALTITUDE_GAIN_M
        ),
        None,
    )
    if roll_start_index is None:
        raise RuntimeError("PX4 never started a takeoff roll")
    if liftoff_index is None:
        raise RuntimeError("PX4 never achieved liftoff")
    if target_index is None:
        raise RuntimeError(
            f"PX4 never reached {TARGET_ALTITUDE_GAIN_M:.0f} m altitude gain"
        )

    heading_sample = next(
        sample
        for sample in samples[roll_start_index : liftoff_index + 1]
        if norm(xyz(sample, "twist", "linear")[:2]) >= 10.0
    )
    heading_velocity = xyz(heading_sample, "twist", "linear")
    reference_heading = math.atan2(heading_velocity[1], heading_velocity[0])
    heading_unit = (math.cos(reference_heading), math.sin(reference_heading))
    lateral_unit = (-heading_unit[1], heading_unit[0])

    climb_segment = []
    for sample in samples[roll_start_index : target_index + 1]:
        altitude_gain = initial_z - xyz(sample, "pose", "position")[2]
        if altitude_gain <= 50.0:
            climb_segment.append(sample)

    cross_track_values = []
    heading_errors = []
    roll_values = []
    pitch_values = []
    for sample in climb_segment:
        displacement = subtract(
            xyz(sample, "pose", "position")[:2], initial_position[:2]
        )
        cross_track_values.append(
            abs(sum(value * axis for value, axis in zip(displacement, lateral_unit)))
        )
        velocity = xyz(sample, "twist", "linear")
        if norm(velocity[:2]) >= 10.0:
            heading = math.atan2(velocity[1], velocity[0])
            heading_errors.append(
                abs(math.degrees(angle_difference(heading, reference_heading)))
            )
        roll, pitch, _ = quaternion_to_rpy(sample["pose"]["orientation"])
        roll_values.append(abs(math.degrees(roll)))
        pitch_values.append(abs(math.degrees(pitch)))

    steps = []
    residuals = []
    jump_candidates = []
    speeds = []
    altitude_gains = []
    highest_altitude_gain = -math.inf
    maximum_altitude_drawdown = 0.0
    for previous, current in zip(samples, samples[1:]):
        dt_sec = (current["time_stamp"] - previous["time_stamp"]) * 1e-9
        if dt_sec <= 0.0:
            continue
        previous_velocity = xyz(previous, "twist", "linear")
        current_velocity = xyz(current, "twist", "linear")
        delta = subtract(
            xyz(current, "pose", "position"),
            xyz(previous, "pose", "position"),
        )
        predicted = scale(add(previous_velocity, current_velocity), 0.5 * dt_sec)
        step = norm(delta)
        residual = norm(subtract(delta, predicted))
        speed = max(norm(previous_velocity), norm(current_velocity))
        steps.append(step)
        residuals.append(residual)
        speeds.append(speed)
        if residual > 20.0 or step > (speed + 15.0) * dt_sec + 5.0:
            jump_candidates.append(current["time_stamp"])
        altitude_gain = initial_z - xyz(current, "pose", "position")[2]
        altitude_gains.append(altitude_gain)
        highest_altitude_gain = max(highest_altitude_gain, altitude_gain)
        maximum_altitude_drawdown = max(
            maximum_altitude_drawdown,
            highest_altitude_gain - altitude_gain,
        )

    sensor_failures = []
    maximum_sensor_lag_sec = 0.0
    for name, messages in sensor_samples.items():
        if len(messages) != len(samples):
            sensor_failures.append(
                f"{name}: {len(messages)} samples for {len(samples)} kinematics"
            )
            continue
        if any(
            not all(math.isfinite(value) for value in numeric_values(message))
            for message in messages
        ):
            sensor_failures.append(f"{name}: non-finite value")
        maximum_sensor_lag_sec = max(
            maximum_sensor_lag_sec,
            max(
                abs(message["time_stamp"] - sample["time_stamp"]) * 1e-9
                for message, sample in zip(messages, samples)
            ),
        )

    gps_fix_at_liftoff = sensor_samples["gps"][liftoff_index]["fix_type"]
    barometer_gain = (
        sensor_samples["barometer"][target_index]["altitude"]
        - sensor_samples["barometer"][roll_start_index]["altitude"]
    )
    final_position = xyz(samples[target_index], "pose", "position")
    horizontal_distance = norm(
        subtract(final_position[:2], initial_position[:2])
    )
    sim_takeoff_duration = (
        samples[target_index]["time_stamp"]
        - samples[roll_start_index]["time_stamp"]
    ) * 1e-9

    failures = list(sensor_failures)
    max_cross_track = max(cross_track_values, default=0.0)
    max_heading_error = max(heading_errors, default=0.0)
    max_roll = max(roll_values, default=0.0)
    max_speed = max(speeds, default=0.0)
    if jump_candidates:
        failures.append(f"{len(jump_candidates)} kinematics jump candidate(s)")
    if gps_fix_at_liftoff < 3:
        failures.append(f"GPS fix at liftoff was {gps_fix_at_liftoff}")
    if max_cross_track > 50.0:
        failures.append(f"initial-climb cross-track reached {max_cross_track:.1f} m")
    if max_heading_error > 25.0:
        failures.append(f"initial heading error reached {max_heading_error:.1f} deg")
    if max_roll > 35.0:
        failures.append(f"initial roll reached {max_roll:.1f} deg")
    if maximum_altitude_drawdown > 20.0:
        failures.append(
            f"altitude drawdown reached {maximum_altitude_drawdown:.1f} m"
        )
    if max_speed > 120.0:
        failures.append(f"speed reached {max_speed:.1f} m/s")
    if abs(barometer_gain - TARGET_ALTITUDE_GAIN_M) > 15.0:
        failures.append(
            f"barometer gain {barometer_gain:.1f} m disagrees with ground truth"
        )

    return {
        "samples": len(samples),
        "sim_takeoff_duration_sec": sim_takeoff_duration,
        "altitude_gain_m": max(altitude_gains),
        "barometer_altitude_gain_m": barometer_gain,
        "horizontal_distance_m": horizontal_distance,
        "max_cross_track_first_50m_m": max_cross_track,
        "max_heading_error_first_50m_deg": max_heading_error,
        "max_roll_first_50m_deg": max_roll,
        "max_pitch_first_50m_deg": max(pitch_values, default=0.0),
        "max_speed_mps": max_speed,
        "max_altitude_drawdown_m": maximum_altitude_drawdown,
        "max_position_step_m": max(steps, default=0.0),
        "max_prediction_residual_m": max(residuals, default=0.0),
        "jump_candidates": len(jump_candidates),
        "gps_fix_at_liftoff": gps_fix_at_liftoff,
        "max_sensor_timestamp_lag_sec": maximum_sensor_lag_sec,
        "failures": failures,
    }


def print_result(result: dict) -> None:
    for key, value in result.items():
        if key != "failures":
            print(f"{key}={value}")
    print(f"failures={len(result['failures'])}")
    for failure in result["failures"]:
        print(f"failure={failure}")


def main() -> None:
    global TARGET_ALTITUDE_GAIN_M
    parser = argparse.ArgumentParser()
    parser.add_argument("--config-dir", type=Path, default=DEFAULT_CONFIG_DIR)
    parser.add_argument("--scene", default=SCENE_NAME)
    parser.add_argument(
        "--target-altitude-gain", type=float, default=TARGET_ALTITUDE_GAIN_M
    )
    args = parser.parse_args()
    TARGET_ALTITUDE_GAIN_M = args.target_altitude_gain

    client = ProjectAirSimClient()
    try:
        client.connect()
        scene_id = load_scene(client, args.config_dir.resolve(), args.scene)
        robot_path = f"/Sim/{scene_id}/robots/{ROBOT_NAME}"
        print(f"PX4_SCENE_READY={scene_id}", flush=True)
        samples, sensors = capture_flight(client, robot_path)
        result = analyze(samples, sensors)
        print_result(result)
        if result["failures"]:
            raise RuntimeError(
                f"Rascal takeoff failed {len(result['failures'])} check(s)"
            )
    finally:
        client.disconnect()


if __name__ == "__main__":
    main()
