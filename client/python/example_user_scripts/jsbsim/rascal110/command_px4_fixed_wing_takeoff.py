"""Send PX4 a fixed-wing takeoff target ahead of the runway heading."""

import argparse
import math
import time

from pymavlink import mavutil


EARTH_RADIUS_M = 6_378_137.0


def wait_message(connection, message_type: str, timeout_sec: float):
    deadline = time.monotonic() + timeout_sec
    while time.monotonic() < deadline:
        message = connection.recv_match(
            type=message_type,
            blocking=True,
            timeout=min(1.0, deadline - time.monotonic()),
        )
        if message is not None:
            return message
    raise TimeoutError(f"PX4 did not publish {message_type}")


def wait_command_ack(connection, command: int, timeout_sec: float) -> None:
    deadline = time.monotonic() + timeout_sec
    while time.monotonic() < deadline:
        ack = connection.recv_match(
            type="COMMAND_ACK",
            blocking=True,
            timeout=min(1.0, deadline - time.monotonic()),
        )
        if ack is None or ack.command != command:
            continue
        if ack.result != mavutil.mavlink.MAV_RESULT_ACCEPTED:
            raise RuntimeError(
                f"PX4 rejected command {command} with result {ack.result}"
            )
        return
    raise TimeoutError(f"PX4 did not acknowledge command {command}")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--connection", default="udpin:0.0.0.0:14550")
    parser.add_argument("--altitude-gain", type=float, default=100.0)
    parser.add_argument("--distance", type=float, default=1500.0)
    parser.add_argument("--min-pitch", type=float, default=10.0)
    args = parser.parse_args()

    connection = mavutil.mavlink_connection(
        args.connection,
        source_system=255,
        source_component=mavutil.mavlink.MAV_COMP_ID_MISSIONPLANNER,
    )
    heartbeat = connection.wait_heartbeat(timeout=20.0)
    target_system = heartbeat.get_srcSystem()
    target_component = heartbeat.get_srcComponent()
    global_position = wait_message(connection, "GLOBAL_POSITION_INT", 20.0)
    attitude = wait_message(connection, "ATTITUDE", 20.0)

    latitude_deg = global_position.lat * 1e-7
    longitude_deg = global_position.lon * 1e-7
    altitude_m = global_position.alt * 1e-3
    heading_rad = attitude.yaw
    north_m = args.distance * math.cos(heading_rad)
    east_m = args.distance * math.sin(heading_rad)
    target_latitude_deg = latitude_deg + math.degrees(north_m / EARTH_RADIUS_M)
    target_longitude_deg = longitude_deg + math.degrees(
        east_m / (EARTH_RADIUS_M * math.cos(math.radians(latitude_deg)))
    )
    target_altitude_m = altitude_m + args.altitude_gain

    print(
        "TAKEOFF_TARGET="
        f"lat={target_latitude_deg:.7f},lon={target_longitude_deg:.7f},"
        f"alt={target_altitude_m:.1f},heading={math.degrees(heading_rad):.1f},"
        f"distance={args.distance:.1f}",
        flush=True,
    )
    connection.mav.command_long_send(
        target_system,
        target_component,
        mavutil.mavlink.MAV_CMD_NAV_TAKEOFF,
        0,
        args.min_pitch,
        0.0,
        0.0,
        math.degrees(heading_rad),
        target_latitude_deg,
        target_longitude_deg,
        target_altitude_m,
    )
    wait_command_ack(
        connection, mavutil.mavlink.MAV_CMD_NAV_TAKEOFF, 10.0
    )
    connection.mav.command_long_send(
        target_system,
        target_component,
        mavutil.mavlink.MAV_CMD_COMPONENT_ARM_DISARM,
        0,
        1.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
    )
    wait_command_ack(
        connection, mavutil.mavlink.MAV_CMD_COMPONENT_ARM_DISARM, 10.0
    )
    print("PX4_TAKEOFF_ACCEPTED", flush=True)


if __name__ == "__main__":
    main()
