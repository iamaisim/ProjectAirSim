"""Unit tests for the X8 controller that do not require a running simulator."""

import math
from pathlib import Path
import sys
import unittest

# Support running this test both directly and through unittest discovery from
# the repository root.
SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from x8_autopilot import PID, WaypointFollower, X8Autopilot, wrap_degrees


class FakeDrone:
    def __init__(self):
        self.properties = {
            "attitude/heading-true-rad": 0.0,
            "attitude/phi-rad": 0.0,
            "attitude/theta-rad": 0.0,
            "velocities/p-rad_sec": 0.0,
            "velocities/q-rad_sec": 0.0,
            "velocities/vtrue-kts": 20.0,
            "position/h-sl-ft": 300.0,
            "position/lat-geod-deg": 0.0,
            "position/long-gc-deg": 0.0,
        }

    def get_jsbsim_property(self, name):
        return self.properties[name]

    def set_jsbsim_property(self, name, value):
        self.properties[name] = value
        return True


class X8AutopilotTests(unittest.TestCase):
    def test_wrap_degrees_uses_shortest_turn(self):
        self.assertEqual(wrap_degrees(340.0), -20.0)
        self.assertEqual(wrap_degrees(-340.0), 20.0)

    def test_pid_preserves_integral_and_clamps_output(self):
        pid = PID(0.0, 1.0, output_min=-0.15, output_max=0.15)
        self.assertAlmostEqual(pid.update(1.0, 0.1), 0.1)
        self.assertAlmostEqual(pid.update(1.0, 0.1), 0.15)
        self.assertAlmostEqual(pid.integral, 0.1)

    def test_hold_generates_bounded_controls_and_throttle(self):
        drone = FakeDrone()
        autopilot = X8Autopilot(drone)
        autopilot.hold(
            heading_deg=90.0,
            altitude_ft=400.0,
            airspeed_kts=42.0,
        )
        self.assertLessEqual(abs(drone.properties["fcs/aileron-cmd-norm"]), 1.0)
        self.assertLessEqual(abs(drone.properties["fcs/elevator-cmd-norm"]), 1.0)
        self.assertGreater(drone.properties["fcs/throttle-cmd-norm"], 0.0)
        self.assertLessEqual(drone.properties["fcs/throttle-cmd-norm"], 1.0)

    def test_hold_opposes_positive_roll(self):
        drone = FakeDrone()
        drone.properties["attitude/phi-rad"] = math.radians(15.0)
        autopilot = X8Autopilot(drone)
        autopilot.hold(heading_deg=0.0, altitude_ft=300.0, airspeed_kts=20.0)
        self.assertLess(drone.properties["fcs/aileron-cmd-norm"], 0.0)

    def test_hold_opposes_positive_pitch(self):
        drone = FakeDrone()
        drone.properties["attitude/theta-rad"] = math.radians(10.0)
        autopilot = X8Autopilot(drone)
        autopilot.hold(heading_deg=0.0, altitude_ft=300.0, airspeed_kts=20.0)
        # Positive elevator in this X8 FDM produces a nose-down moment.
        self.assertGreater(drone.properties["fcs/elevator-cmd-norm"], 0.0)

    def test_waypoint_follower_advances_at_acceptance_radius(self):
        drone = FakeDrone()
        autopilot = X8Autopilot(drone)
        follower = WaypointFollower(
            autopilot,
            [(0.0, 0.0, 300.0), (100.0, 0.0, 300.0)],
            acceptance_radius_m=10.0,
        )
        self.assertFalse(follower.update(42.0))
        self.assertEqual(follower.index, 1)
        expected_lat_rad = 100.0 / follower.EARTH_RADIUS_M
        drone.properties["position/lat-geod-deg"] = math.degrees(expected_lat_rad)
        self.assertTrue(follower.update(42.0))


if __name__ == "__main__":
    unittest.main()
