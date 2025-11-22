import argparse
import asyncio
import time
import projectairsim
from projectairsim import Drone, World
import keyboard

# --- Drone Control Functions ---

async def takeoff(drone):
    """Arms the drone and takes off to a default altitude."""
    print("Arming the drone...")
    drone.arm()
    print("Taking off...")
    await drone.takeoff_async()
    time.sleep(1)


async def land(drone):
    """Lands the drone."""
    print("Landing...")
    await drone.land_async()
    print("Disarming the drone...")
    drone.disarm()

# --- Main Control Loop ---

async def run_keyboard_control(drone):
    """
    Controls the drone using keyboard inputs.

    Args:
        drone: The Drone object.
    """

    # Enable API control
    drone.enable_api_control()

    # Takeoff
    await takeoff(drone)

    # Speed settings
    speed = 5  # m/s
    yaw_speed = 20  # degrees/s
    duration = 0.1  # seconds

    print("\n--- Keyboard Control ---")
    print("W/S: Pitch (Forward/Backward)")
    print("A/D: Roll (Left/Right)")
    print("Up/Down Arrows: Throttle (Altitude)")
    print("Left/Right Arrows: Yaw (Rotation)")
    print("L: Land")
    print("Q: Quit")
    print("--------------------")

    keep_running = True

    while keep_running:
        # Reset velocity components
        vx, vy, vz, yaw_rate = 0, 0, 0, 0

        # Pitch
        if keyboard.is_pressed('w'):
            vx = speed

        elif keyboard.is_pressed('s'):
            vx = -speed

        # Roll
        if keyboard.is_pressed('a'):
            vy = -speed

        elif keyboard.is_pressed('d'):
            vy = speed

        # Throttle
        if keyboard.is_pressed('up'):
            vz = -speed  # Negative Z is up

        elif keyboard.is_pressed('down'):
            vz = speed

        # Yaw
        if keyboard.is_pressed('left'):
            yaw_rate = -yaw_speed

        elif keyboard.is_pressed('right'):
            yaw_rate = yaw_speed

        # Land and exit
        if keyboard.is_pressed('l'):
            await land(drone)
            keep_running = False

        # Quit
        if keyboard.is_pressed('q'):
            keep_running = False

        # Move the drone in its body frame
        # vx, vy, vz are now interpreted as forward/backward, right/left, up/down relative to the drone
        if vx != 0 or vy != 0 or vz != 0:
            await drone.move_by_velocity_body_frame_async(vx, vy, vz, duration)
        if yaw_rate != 0:
            await drone.rotate_by_yaw_rate_async(yaw_rate, duration)
        await asyncio.sleep(0.01)

# --- Main Execution ---

async def main():
    parser = argparse.ArgumentParser(
        description="Example of using keyboard to control a drone in Project AirSim."
    )

    # ... (parser arguments remain the same) ...
    parser.add_argument(
        "--address",
        help=("the IP address of the host running Project AirSim"),
        type=str,
        default="127.0.0.1",
    )

    parser.add_argument(
        "--sceneconfigfile",
        help=(
            'the Project AirSim scene config file to load, defaults to "scene_basic_drone.jsonc"'
        ),

        type=str,
        default="scene_basic_drone.jsonc",
    )

    parser.add_argument(
        "--simconfigpath",
        help=(
            'the directory containing Project AirSim config files, defaults to "sim_config"'
        ),
        type=str,
        default="sim_config/",
    )

    parser.add_argument(
        "--topicsport",
        help=(
            "the TCP/IP port of Project AirSim's topic pub-sub client connection "
            '(see the Project AirSim command line switch "-topicsport")'
        ),
        type=int,
        default=8989,
    )

    parser.add_argument(
        "--servicesport",
        help=(
            "the TCP/IP port of Project AirSim's services client connection "
            '(see the Project AirSim command line switch "-servicessport")'
        ),
        type=int,
        default=8990,
    )

    args = parser.parse_args()
    client = projectairsim.ProjectAirSimClient(
        address=args.address,
        port_topics=args.topicsport,
        port_services=args.servicesport,
    )

    drone = None
    try:
        client.connect()
        world = projectairsim.World(
            client=client,
            scene_config_name=args.sceneconfigfile,
            sim_config_path=args.simconfigpath,
        )
        drone = Drone(client, world, "Drone1")

        await run_keyboard_control(drone)

    except Exception as e:
        print(f"An error occurred: {e}")

    finally:
        if drone:
            drone.disarm()
            drone.disable_api_control()
        client.disconnect()

        print("Cleaned up and disconnected.")

if __name__ == "__main__":
    asyncio.run(main())
