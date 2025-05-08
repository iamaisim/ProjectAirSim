"""
Copyright (C) Microsoft Corporation. All rights reserved.

Demonstrates the use of the set_object_material(), 
set_object_texture_from_packaged_asset(), and set_object_texture_from_file() 
APIs, which allow the user to modify the materials and textures of objects 
in the scene.
"""

import asyncio

from projectairsim import ProjectAirSimClient, World, Drone
from projectairsim.utils import projectairsim_log, rpy_to_quaternion
from projectairsim.types import Vector3, Pose, Quaternion


def spawn_assets(world):
    # Spawn landing pads
    pad_asset_path = "BasicLandingPad"
    ref_frame = "DEFAULT_ID"
    rotation = Quaternion({"w": 1, "x": 0, "y": 0, "z": 0})
    scale = [3.0, 3.0, 3.0]
    enable_physics = False

    pad1_name = "LeafMaterialPad"
    translation = Vector3({"x": 20, "y": 4, "z": -1.5})
    pose: Pose = Pose(
        {"translation": translation, "rotation": rotation, "frame_id": ref_frame}
    )
    retval = world.spawn_object(pad1_name, pad_asset_path, pose, scale, enable_physics)
    projectairsim_log().info("Spawned object successfully with name: " + retval)

    pad2_name = "DefaultMaterialGridPad"
    translation = Vector3({"x": 20, "y": -8.4, "z": -1.5})
    pose: Pose = Pose(
        {"translation": translation, "rotation": rotation, "frame_id": ref_frame}
    )
    retval = world.spawn_object(pad2_name, pad_asset_path, pose, scale, enable_physics)
    projectairsim_log().info("Spawned object successfully with name: " + retval)

    pad3_name = "SampleTexturePad"
    translation = Vector3({"x": 3, "y": 4, "z": -2.5})
    pose: Pose = Pose(
        {"translation": translation, "rotation": rotation, "frame_id": ref_frame}
    )
    retval = world.spawn_object(pad3_name, pad_asset_path, pose, scale, enable_physics)
    projectairsim_log().info("Spawned object successfully with name: " + retval)

    return pad1_name, pad2_name, pad3_name


# Async main function to wrap async drone commands
async def main():
    # Create a Project AirSim client
    client = ProjectAirSimClient()

    try:
        # Connect to simulation environment
        client.connect()

        # Create a World object to interact with the sim world and load a scene
        world = World(
            client, "scene_object_material_manipulate.jsonc", delay_after_load_sec=2
        )

        drone = Drone(client, world, "Drone1")

        # New desired pose for chase cam to see landing pad visual modifications
        pos = {"x": -5.0, "y": 0.0, "z": -8.0}
        rot = {"r": 0, "p": -0.7, "y": 0}

        # Convert Eulerian angle to Quaternion
        (w, x, y, z) = rpy_to_quaternion(rot["r"], rot["p"], rot["y"])

        translation = Vector3({"x": pos["x"], "y": pos["y"], "z": pos["z"]})
        rotation = Quaternion({"w": w, "x": x, "y": y, "z": z})
        transform = {"translation": translation, "rotation": rotation}
        pose = Pose(transform)

        projectairsim_log().info("Setting chase camera to new pose to visualize landing pads.")
        drone.set_camera_pose("Chase", pose)

        pad1, pad2, pad3 = spawn_assets(world)

        # Pause to see before-after change
        await asyncio.sleep(1)

        # ------------------------------------------------------------------------------

        # Modify objects
        projectairsim_log().info("Setting LeafMaterialPad to use leaf material asset.")

        # Change the material
        material_path = "/ProjectAirSim/Weather/WeatherFX/Materials/M_Leaf_master"
        status = world.set_object_material(pad1, material_path)

        # Change the texture to a packaged texture
        projectairsim_log().info(
            "Setting DefaultMaterialGridPad's texture to default material grid texture."
        )
        texture_path = "/Game/Geometry/Textures/T_Default_Material_Grid_M"
        status = world.set_object_texture_from_packaged_asset(pad2, texture_path)

        # Change the texture to the one from PNG file
        projectairsim_log().info(
            "Setting SampleTexturePad's texture to the one in sample_texture.png."
        )
        texture_path = "assets/sample_texture.png"
        status = world.set_object_texture_from_file(pad3, texture_path)

    except Exception as err:
        projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)

    finally:
        # Always disconnect from the simulation environment to allow next connection
        client.disconnect()


if __name__ == "__main__":
    asyncio.run(main())  # Runner for async main function
