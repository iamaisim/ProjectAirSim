import sys
import keyboard
import time
import math

from projectairsim import ProjectAirSimClient
from projectairsim.utils import (
    projectairsim_log,
)

if __name__ == "__main__":
    client = ProjectAirSimClient()

    try:
        client.connect()

        client.set_interactive_feature("viewport_camera", True)

        pos = {"x": -50.0, "y": 0.0, "z": -25.0}
        rot = {"x": 0.0, "y": 0.0, "z": 0.0, "w": 1.0}  # Inicializar con w = 1 para un cuaternión unitario

        message = {"position": pos, "orientation": rot}
        client.publish('/Sim/interactive/sensors/viewport_camera/desired_pose', message)

        aspect_ratio = 1.0
        message = {"value": aspect_ratio}
        client.publish('/Sim/interactive/sensors/viewport_camera/aspect_ratio', message)

        zoom = 0.5
        message = {"value": zoom}
        client.publish('/Sim/interactive/sensors/viewport_camera/zoom', message)

        pose_step = 0.2  # m
        pose_step_step = 0.05
        rot_step = 0.02  # rad
        rot_step_step = 0.005

        print("Controls:")
        print("  Arrow keys: Move position (X, Y)")
        print("  Page Up / Page Down: Move position (Z)")
        print("  W/S: Pitch Up/Down")
        print("  A/D: Yaw Left/Right")
        print("  Q/E: Roll Left/Right")
        print("  + / -: Zoom in/out")
        print("  Z/X: Increase/Decrease translation speed")
        print("  1/2: Increase/Decrease rotation speed")
        print("  Esc: Quit application")

        def normalize_quaternion(q):
            """Normalizes the quaternion to have a magnitude of 1."""
            magnitude = math.sqrt(q["x"]**2 + q["y"]**2 + q["z"]**2 + q["w"]**2)
            q["x"] /= magnitude
            q["y"] /= magnitude
            q["z"] /= magnitude
            q["w"] /= magnitude

        def handle_key_event(e):
            """Handles all key press events and updates the position, rotation, and speed."""
            global pos, rot, client, pose_step, rot_step

            if e.event_type != 'down':
                return

            if e.name == 'esc':  # Esc key to quit
                sys.exit()

            # --- Position controls with arrow keys ---
            if e.name == 'up':
                pos["x"] += pose_step
            if e.name == 'down':
                pos["x"] -= pose_step
            if e.name == 'right':
                pos["y"] += pose_step
            if e.name == 'left':
                pos["y"] -= pose_step

            # --- Page Up / Page Down controls for Z position ---
            if e.name == 'page up':
                pos["z"] -= pose_step
            if e.name == 'page down':
                pos["z"] += pose_step

            # --- Rotation controls ---
            if e.name == 'w':
                rot["y"] += rot_step
            if e.name == 's':
                rot["y"] -= rot_step
            if e.name == 'a':
                rot["z"] += rot_step
            if e.name == 'd':
                rot["z"] -= rot_step
            if e.name == 'q':
                rot["x"] += rot_step
            if e.name == 'e':
                rot["x"] -= rot_step

            # --- Speed controls for translation ---
            if e.name == 'z':
                pose_step += pose_step_step
                print(f"Translation speed increased: {pose_step:.3f} m")
            if e.name == 'x' and pose_step > 0:
                pose_step -= pose_step_step
                print(f"Translation speed decreased: {pose_step:.3f} m")

            # --- Speed controls for rotation ---
            if e.name == '1':
                rot_step += rot_step_step
                print(f"Rotation speed increased: {rot_step:.3f} rad")
            if e.name == '2' and rot_step > 0:
                rot_step -= rot_step_step
                print(f"Rotation speed decreased: {rot_step:.3f} rad")

            # --- Normalize the quaternion ---
            normalize_quaternion(rot)

            # Send the updated position and orientation
            message = {"position": pos, "orientation": rot}
            client.publish('/Sim/interactive/sensors/viewport_camera/desired_pose', message)

        # Register the event hook
        keyboard.hook(handle_key_event)

        print("Listening for key presses...")
        keyboard.wait('esc')  # Wait for ESC key to exit

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        client.set_interactive_feature("viewport_camera", False)
        client.disconnect()
