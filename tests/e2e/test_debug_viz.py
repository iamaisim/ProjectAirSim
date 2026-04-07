"""
Test all debug visualization primitives.
Uses step API to fly the drone (acro mode + throttle), draws debug shapes with labels.
"""

import os
import time

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log

DURATION = 60.0
SIM_ADDR = "172.23.240.1"
STEP_NS = 3_000_000  # 3ms per physics tick
SIM_CONFIG_PATH = os.path.join(
    os.path.dirname(__file__), "..", "..", "client", "python", "example_user_scripts", "sim_config"
)


# Layout helper: NED coordinates
def pos(east_offset, z=-3.0):
    return [0.0, east_offset, z]


def arm_drone(client, topic):
    client.request({"method": f"{topic}/EnableApiControl", "params": {}, "version": 1.0})
    client.request({"method": f"{topic}/Arm", "params": {}, "version": 1.0})
    client.request({"method": f"{topic}/SetAcroMode", "params": {"enabled": True}, "version": 1.0})


def set_controls(client, topic, throttle, roll_rate=0.0, pitch_rate=0.0, yaw_rate=0.0):
    client.request({
        "method": f"{topic}/SetAngleRatesThrottle",
        "params": {"roll_rate": roll_rate, "pitch_rate": pitch_rate,
                   "yaw_rate": yaw_rate, "throttle": throttle},
        "version": 1.0,
    })


def main():
    client = ProjectAirSimClient(address=SIM_ADDR)

    try:
        client.connect()
        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2, sim_config_path=SIM_CONFIG_PATH)

        drone_topic = f"{world.parent_topic}/robots/Drone1"

        # --- Draw all debug primitives ---
        projectairsim_log().info("Drawing debug visualization primitives...")

        spacing = 4.0
        z = -3.0  # 3m above ground (NED)

        label_positions = []
        label_names = []

        def add_label(name, east):
            label_positions.append([0.0, east, z])
            label_names.append(name)

        # 1) POINTS - cluster of red points
        east = 0.0
        add_label("Points", east)
        world.plot_debug_points(
            points=[
                pos(east - 0.3, z), pos(east + 0.3, z), pos(east, z - 0.3),
                pos(east, z + 0.3), pos(east - 0.2, z - 0.2), pos(east + 0.2, z + 0.2),
            ],
            color_rgba=[1.0, 0.0, 0.0, 1.0],
            size=15.0,
            duration=DURATION,
            is_persistent=False,
        )

        # 2) SOLID LINE - green zigzag
        east += spacing
        add_label("Solid Line", east)
        world.plot_debug_solid_line(
            points=[
                pos(east - 1.0, z),
                pos(east - 0.5, z - 0.8),
                pos(east, z),
                pos(east + 0.5, z - 0.8),
                pos(east + 1.0, z),
            ],
            color_rgba=[0.0, 1.0, 0.0, 1.0],
            thickness=3.0,
            duration=DURATION,
            is_persistent=False,
        )

        # 3) DASHED LINE - cyan segments (even number of points)
        east += spacing
        add_label("Dashed Line", east)
        world.plot_debug_dashed_line(
            points=[
                pos(east - 1.0, z), pos(east - 0.5, z),
                pos(east - 0.3, z), pos(east + 0.3, z),
                pos(east + 0.5, z), pos(east + 1.0, z),
            ],
            color_rgba=[0.0, 1.0, 1.0, 1.0],
            thickness=3.0,
            duration=DURATION,
            is_persistent=False,
        )

        # 4) ARROWS - magenta arrows pointing in different directions
        east += spacing
        add_label("Arrows", east)
        world.plot_debug_arrows(
            points_start=[
                pos(east, z),
                pos(east, z),
                pos(east, z),
            ],
            points_end=[
                pos(east + 1.5, z),
                pos(east, z - 1.5),
                pos(east + 1.0, z - 1.0),
            ],
            color_rgba=[1.0, 0.0, 1.0, 1.0],
            thickness=3.0,
            arrow_size=30.0,
            duration=DURATION,
            is_persistent=False,
        )

        # 5) COLORED POINTS DEMO - heatmap gradient blue -> red
        east += spacing
        add_label("Color Gradient", east)
        n = 10
        for i in range(n):
            t = i / max(n - 1, 1)
            world.plot_debug_points(
                points=[pos(east - 1.5 + 3.0 * t, z)],
                color_rgba=[t, 0.0, 1.0 - t, 1.0],
                size=20.0,
                duration=DURATION,
                is_persistent=False,
            )

        # 6) Labels for everything
        world.plot_debug_strings(
            strings=label_names,
            positions=label_positions,
            scale=5.0,
            color_rgba=[1.0, 1.0, 0.0, 1.0],
            duration=DURATION,
        )

        # Big test string near origin
        world.plot_debug_strings(
            strings=["HELLO DEBUG"],
            positions=[[0.0, 0.0, -1.0]],
            scale=10.0,
            color_rgba=[1.0, 1.0, 1.0, 1.0],
            duration=DURATION,
        )

        projectairsim_log().info("All primitives drawn!")

        # --- Fly the drone using step API (acro mode) ---
        projectairsim_log().info("Setting up trace and flying drone via step API...")
        world.set_trace_line(color_rgba=[1.0, 0.5, 0.0, 1.0], thickness=4.0)
        world.toggle_trace()

        arm_drone(client, drone_topic)

        # Step once to settle
        world.step(STEP_NS)

        # Phase 1: Climb (~3s = 1000 steps at 3ms)
        projectairsim_log().info("Climbing...")
        set_controls(client, drone_topic, throttle=0.55)
        for _ in range(1000):
            world.step(STEP_NS)

        # Phase 2: Pitch forward (~2s)
        projectairsim_log().info("Flying forward...")
        set_controls(client, drone_topic, throttle=0.45, pitch_rate=0.5)
        for _ in range(666):
            world.step(STEP_NS)

        # Phase 3: Yaw spin (~2s)
        projectairsim_log().info("Yawing...")
        set_controls(client, drone_topic, throttle=0.45, yaw_rate=1.0)
        for _ in range(666):
            world.step(STEP_NS)

        # Phase 4: Cut throttle and descend (~3s)
        projectairsim_log().info("Descending...")
        set_controls(client, drone_topic, throttle=0.0)
        for _ in range(1000):
            world.step(STEP_NS)

        projectairsim_log().info("Flight done. Keeping connection alive for 30s...")

        # Keep alive so debug drawings stay visible
        time.sleep(30)

    except Exception as err:
        projectairsim_log().error(f"Exception: {err}", exc_info=True)

    finally:
        client.disconnect()


if __name__ == "__main__":
    main()