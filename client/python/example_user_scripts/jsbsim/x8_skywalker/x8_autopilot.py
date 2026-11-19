"""Skywalker X8 autopilot adapted to the JSBSim API exposed by Project AirSim.

Controller gains and the cascaded heading/altitude structure are derived from
AOS55/Fixedwing-Airsim. The implementation keeps controller state between
updates, uses no third-party PID package, and talks only through
Drone.get_jsbsim_property()/set_jsbsim_property().
"""

from __future__ import annotations

from dataclasses import dataclass
import math
from typing import Iterable


def clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def wrap_degrees(value: float) -> float:
    return (value + 180.0) % 360.0 - 180.0


@dataclass
class PID:
    kp: float
    ki: float = 0.0
    kd: float = 0.0
    output_min: float = -math.inf
    output_max: float = math.inf
    integral: float = 0.0
    previous_error: float | None = None

    def update(self, error: float, dt: float, derivative: float | None = None) -> float:
        if dt <= 0.0:
            raise ValueError("dt must be positive")
        candidate_integral = self.integral + error * dt
        if derivative is None:
            derivative = (
                0.0
                if self.previous_error is None
                else (error - self.previous_error) / dt
            )
        raw = self.kp * error + self.ki * candidate_integral + self.kd * derivative
        output = clamp(raw, self.output_min, self.output_max)
        # Conditional integration prevents wind-up while an output is saturated.
        if output == raw or (output == self.output_max and error < 0.0) or (
            output == self.output_min and error > 0.0
        ):
            self.integral = candidate_integral
        self.previous_error = error
        return output


class X8Autopilot:
    """Cascaded flight controller for the Fixedwing-Airsim X8 FDM."""

    def __init__(self, drone, control_period_sec: float = 0.02):
        self.drone = drone
        self.dt = control_period_sec
        # These signs follow this implementation's target-minus-state error
        # convention and the X8 FDM's control derivatives. Positive elevator
        # creates a nose-down moment; positive aileron creates positive roll.
        self.pitch = PID(-1.0, 0.0, 0.03, -1.0, 1.0)
        self.roll = PID(0.20, 0.0, -0.089, -1.0, 1.0)
        # Fixedwing-Airsim recreated its PID objects on every call, effectively
        # discarding the integral state. These integral gains are scaled for a
        # persistent 50 Hz controller.
        self.heading = PID(0.01, 0.0002)
        self.altitude = PID(0.0025, 0.00005)
        # Airspeed feedback is a correction around the X8's cruise power.
        # Without this feed-forward, zero airspeed error commands zero throttle
        # and the air-launched aircraft merely glides with a stopped propeller.
        self.cruise_throttle = 0.65
        self.airspeed = PID(0.08, 0.015, 0.0, -0.65, 0.35)

    def get(self, name: str) -> float:
        return float(self.drone.get_jsbsim_property(name))

    def set(self, name: str, value: float) -> None:
        if not self.drone.set_jsbsim_property(name, float(value)):
            raise RuntimeError(f"JSBSim rejected property '{name}'")

    def set_controls(self, *, elevator: float, aileron: float, throttle: float) -> None:
        self.set("fcs/elevator-cmd-norm", clamp(elevator, -1.0, 1.0))
        self.set("fcs/aileron-cmd-norm", clamp(aileron, -1.0, 1.0))
        self.set("fcs/throttle-cmd-norm", clamp(throttle, 0.0, 1.0))

    def hold(
        self, *, heading_deg: float, altitude_ft: float, airspeed_kts: float
    ) -> None:
        heading_error = wrap_degrees(
            heading_deg
            - math.degrees(self.get("attitude/heading-true-rad"))
        )
        roll_command = clamp(
            self.heading.update(heading_error, self.dt),
            math.radians(-30.0),
            math.radians(30.0),
        )
        roll_error = roll_command - self.get("attitude/phi-rad")
        aileron = self.roll.update(
            roll_error,
            self.dt,
            derivative=self.get("velocities/p-rad_sec"),
        )

        altitude_error = altitude_ft - self.get("position/h-sl-ft")
        pitch_command = clamp(
            self.altitude.update(altitude_error, self.dt),
            math.radians(-10.0),
            math.radians(15.0),
        )
        pitch_error = pitch_command - self.get("attitude/theta-rad")
        elevator = self.pitch.update(
            pitch_error,
            self.dt,
            derivative=self.get("velocities/q-rad_sec"),
        )

        true_airspeed_kts = self.get("velocities/vtrue-kts")
        throttle = self.cruise_throttle + self.airspeed.update(
            airspeed_kts - true_airspeed_kts, self.dt
        )
        self.set_controls(
            elevator=elevator,
            aileron=aileron,
            throttle=throttle,
        )


class WaypointFollower:
    """Navigate through local NED waypoints using the X8 heading loop."""

    EARTH_RADIUS_M = 6_378_137.0

    def __init__(
        self,
        autopilot: X8Autopilot,
        waypoints: Iterable[tuple[float, float, float]],
        acceptance_radius_m: float = 75.0,
    ):
        self.autopilot = autopilot
        self.waypoints = list(waypoints)
        if not self.waypoints:
            raise ValueError("at least one waypoint is required")
        self.acceptance_radius_m = acceptance_radius_m
        self.index = 0
        self.origin_lat_rad = math.radians(
            autopilot.get("position/lat-geod-deg")
        )
        self.origin_lon_rad = math.radians(
            autopilot.get("position/long-gc-deg")
        )

    def local_position(self) -> tuple[float, float]:
        lat = math.radians(self.autopilot.get("position/lat-geod-deg"))
        lon = math.radians(self.autopilot.get("position/long-gc-deg"))
        north = (lat - self.origin_lat_rad) * self.EARTH_RADIUS_M
        east = (
            (lon - self.origin_lon_rad)
            * self.EARTH_RADIUS_M
            * math.cos(self.origin_lat_rad)
        )
        return north, east

    def update(self, airspeed_kts: float) -> bool:
        north, east = self.local_position()
        target_north, target_east, target_altitude_ft = self.waypoints[self.index]
        delta_north = target_north - north
        delta_east = target_east - east
        distance = math.hypot(delta_north, delta_east)
        if distance <= self.acceptance_radius_m:
            self.index += 1
            if self.index >= len(self.waypoints):
                return True
            target_north, target_east, target_altitude_ft = self.waypoints[self.index]
            delta_north = target_north - north
            delta_east = target_east - east
        heading_deg = math.degrees(math.atan2(delta_east, delta_north)) % 360.0
        self.autopilot.hold(
            heading_deg=heading_deg,
            altitude_ft=target_altitude_ft,
            airspeed_kts=airspeed_kts,
        )
        return False
