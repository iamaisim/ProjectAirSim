from projectairsim import ProjectAirSimClient, World
from projectairsim.types import Vector3, Quaternion, Pose
from projectairsim.utils import rpy_to_quaternion
import numpy as np
import time
import math

client = ProjectAirSimClient()
client.connect()

def to_radians_then_quaternion(pitch, roll, yaw):
    pitch = pitch * math.pi/180
    roll = roll * math.pi/180
    yaw = yaw * math.pi/180
    quat = rpy_to_quaternion(pitch, roll, yaw)
    return Quaternion({"w": quat[0], "x": quat[1], "y": quat[2], "z": quat[3]})

try:
    world = World(client, "scene_drone_classic.jsonc", delay_after_load_sec=2)

    # plot red arrows for 30 seconds
    world.plot_debug_arrows(points_start = [[x,y,0] for x, y in zip(np.linspace(0,10,20), np.linspace(0,0,20))], 
                            points_end = [[x,y,0] for x, y in zip(np.linspace(0,10,20), np.linspace(10,10,20))], 
                            color_rgba = [1.0, 0.0, 1.0, 1.0], duration = 30.0, arrow_size = 10, thickness = 1, is_persistent = False)

    # plot magenta arrows for 15 seconds
    world.plot_debug_arrows(points_start = [[x,y,-3] for x, y in zip(np.linspace(0,10,20), np.linspace(0,0,20))], 
                            points_end = [[x,y,-5] for x, y in zip(np.linspace(0,10,20), np.linspace(10,20,20))], 
                            color_rgba = [1.0, 1.0, 0.0, 1.0], duration = 15.0, arrow_size = 20, thickness = 3, is_persistent = False)

    # plot red arrows for 10 seconds
    world.plot_debug_arrows(points_start = [[x,y,z] for x, y, z in zip(np.linspace(0,10,20), np.linspace(0,0,20), np.linspace(-3,-10, 20))], 
                            points_end = [[x,y,z] for x, y, z in zip(np.linspace(0,10,20), np.linspace(10,20,20), np.linspace(-5,-8, 20))], 
                            color_rgba = [1.0, 0.0, 0.0, 1.0], duration = 10.0, arrow_size = 100, thickness = 5, is_persistent = False)

    # plot 2 white arrows which are persistent 
    world.plot_debug_arrows(points_start = [[x,y,-2] for x, y in zip(np.linspace(0,10,20), np.linspace(0,20,20))], 
                            points_end = [[x,y,-5] for x, y in zip(np.linspace(3,17,20), np.linspace(5,28,20))], 
                            color_rgba = [1.0, 1.0, 1.0, 1.0], duration = 5.0, arrow_size = 100, thickness = 1, is_persistent = True)

    # plot points 
    world.plot_debug_points(points = [[x,y,-5] for x, y in zip(np.linspace(0,-10,20), np.linspace(0,-20,20))], color_rgba=[1.0, 0.0, 0.0, 1.0], size = 25, duration = 20.0, is_persistent = False)
    world.plot_debug_points(points = [[x,y,z] for x, y, z in zip(np.linspace(0,-10,20), np.linspace(0,-20,20), np.linspace(0,-5,20))], color_rgba=[0.0, 0.0, 1.0, 1.0], size = 10, duration = 20.0, is_persistent = False)
    world.plot_debug_points(points = [[x,y,z] for x, y, z in zip(np.linspace(0,10,20), np.linspace(0,-20,20), np.linspace(0,-7,20))], color_rgba=[1.0, 0.0, 1.0, 1.0], size = 15, duration = 20.0, is_persistent = False)

    # plot line strip. 0-1, 1-2, 2-3
    world.plot_debug_solid_line(points = [[x,y,-5] for x, y in zip(np.linspace(0,-10,10), np.linspace(0,-20,10))], color_rgba=[1.0, 0.0, 0.0, 1.0], thickness = 5, duration = 30.0, is_persistent = False)

    # plot line list. 0-1, 2-3, 4-5. Must be even. 
    world.plot_debug_dashed_line(points = [[x,y,-7] for x, y in zip(np.linspace(0,-10,10), np.linspace(0,-20,10))], color_rgba=[1.0, 0.0, 0.0, 1.0], thickness = 5, duration = 30.0, is_persistent = False)

    # plot transforms 
    world.plot_debug_strings(strings = ["Microsoft Project AirSim" for i in range(5)], positions = [[x,y,-1] for x, y in zip(np.linspace(0,5,5), np.linspace(0,0,5))], 
                            scale = 1, color_rgba=[1.0, 1.0, 1.0, 1.0], duration = 1200.0)

    # world.plot_debug_transforms(poses = [Pose({"translation":Vector3({"x":x,"y":y,"z":0}), "rotation":to_radians_then_quaternion(pitch=0.0, roll=0.0, yaw=yaw)}) for x, y, yaw in zip(np.linspace(0,10,10), np.linspace(0,0,10), np.linspace(0,np.pi,10))], 
    #                         scale = 35, thickness = 5, duration = 1200.0, is_persistent = False)

    # world.plot_debug_transforms(poses = [Pose({"translation":Vector3({"x":x,"y":y,"z":0}), "rotation":to_radians_then_quaternion(pitch=0.0, roll=roll, yaw=0.0)}) for x, y, roll in zip(np.linspace(0,10,10), np.linspace(1,1,10), np.linspace(0,np.pi,10))], 
    #                         scale = 35, thickness = 5, duration = 1200.0, is_persistent = False)

    world.plot_debug_transforms_with_names(poses = [Pose({"translation":Vector3({"x":x,"y":y,"z":0}), "rotation":to_radians_then_quaternion(pitch=0.0, roll=0.0, yaw=yaw)}) for x, y, yaw in zip(np.linspace(0,10,10), np.linspace(0,0,10), np.linspace(0,np.pi,10))], 
                                    names=["tf_yaw_" + str(idx) for idx in range(10)], tf_scale = 35, tf_thickness = 5, text_scale = 1, text_color_rgba = [1.0, 1.0, 1.0, 1.0], duration = 1200.0)

    world.plot_debug_transforms_with_names(poses = [Pose({"translation":Vector3({"x":x,"y":y,"z":0}), "rotation":to_radians_then_quaternion(pitch=0.0, roll=roll, yaw=0.0)}) for x, y, roll in zip(np.linspace(0,10,10), np.linspace(1,1,10), np.linspace(0,np.pi,10))], 
                                    names=["tf_roll_" + str(idx) for idx in range(10)], tf_scale = 35, tf_thickness = 5, text_scale = 1, text_color_rgba = [1.0, 1.0, 1.0, 1.0], duration = 1200.0)

    world.plot_debug_transforms_with_names(poses = [Pose({"translation":Vector3({"x":x,"y":y,"z":0}), "rotation":to_radians_then_quaternion(pitch=pitch, roll=0.0, yaw=0.0)}) for x, y, pitch in zip(np.linspace(0,10,10), np.linspace(-1,-1,10), np.linspace(0,np.pi,10))],
                                    names=["tf_pitch_" + str(idx) for idx in range(10)], tf_scale = 35, tf_thickness = 5, text_scale = 1, text_color_rgba = [1.0, 1.0, 1.0, 1.0], duration = 1200.0)

    time.sleep(20.0)

    world.flush_persistent_markers()
finally:
    client.disconnect()