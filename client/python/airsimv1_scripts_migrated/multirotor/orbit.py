import os
import sys
import math
import time
import argparse
import asyncio
import cv2
import numpy

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.drone import YawControlMode
from projectairsim.types import ImageType

# Make the drone fly in a circle.
class OrbitNavigator:
    def __init__(self, radius = 2, altitude = 10, speed = 2, iterations = 1, center = [1,0], snapshots = None):
        self.radius = radius
        self.altitude = altitude
        self.speed = speed
        self.iterations = iterations
        self.snapshots = snapshots
        self.snapshot_delta = None
        self.next_snapshot = None
        self.z = None
        self.snapshot_index = 0
        self.takeoff = False # whether we did a take off

        if self.snapshots is not None and self.snapshots > 0:
            self.snapshot_delta = 360 / self.snapshots

        if self.iterations <= 0:
            self.iterations = 1

        if len(center) != 2:
            raise Exception("Expecting '[x,y]' for the center direction vector")
        
        # center is just a direction vector, so normalize it to compute the actual cx,cy locations.
        cx = float(center[0])
        cy = float(center[1])
        length = math.sqrt((cx*cx) + (cy*cy))
        cx /= length
        cy /= length
        cx *= self.radius
        cy *= self.radius

        self.client = ProjectAirSimClient()
        self.client.connect()
        self.world = World(self.client, "scene_drone_sensors.jsonc", delay_after_load_sec=2)
        self.drone = Drone(self.client, self.world, "Drone1")
        self.drone.enable_api_control()

        self.home = self.drone.get_ground_truth_kinematics()["pose"]["position"]
        # check that our home position is stable
        start = time.time()
        count = 0
        while count < 100:
            pos = self.drone.get_ground_truth_kinematics()["pose"]["position"]
            if abs(pos["z"] - self.home["z"]) > 1:                                
                count = 0
                self.home = pos
                if time.time() - start > 10:
                    projectairsim_log().info("Drone position is drifting, we are waiting for it to settle down...")
                    start = time
            else:
                count += 1

        self.center = pos
        self.center["x"] += cx
        self.center["y"] += cy

    async def start(self):
        try:
            projectairsim_log().info("arming the drone...")
            self.drone.arm()
            
            # Project AirSim uses NED coordinates so negative axis is up.
            start = self.drone.get_ground_truth_kinematics()["pose"]["position"]
            #landed_state can't be accessed with the current API
            #landed = self.client.getMultirotorState().landed_state 
            if not self.takeoff: 
                self.takeoff = True
                projectairsim_log().info("taking off...")
                takeoff_task = await self.drone.takeoff_async()
                await takeoff_task
                start = self.drone.get_ground_truth_kinematics()["pose"]["position"]
                z = -self.altitude + self.home["z"]
            else:
                projectairsim_log().info("already flying so we will orbit at current altitude {}".format(start["z"]))
                z = start["z"] # use current altitude then

            projectairsim_log().info("climbing to position: {},{},{}".format(start["x"], start["y"], z))
            move_task = await self.drone.move_to_position_async(start["x"], start["y"], z, self.speed)
            await move_task
            self.z = z
            
            projectairsim_log().info("ramping up to speed...")
            count = 0
            self.start_angle = None
            self.next_snapshot = None
            
            # ramp up time
            ramptime = self.radius / 10
            self.start_time = time.time()        

            while count < self.iterations:
                if self.snapshots > 0 and not (self.snapshot_index < self.snapshots):
                    break
                # ramp up to full speed in smooth increments so we don't start too aggressively.
                now = time.time()
                speed = self.speed
                diff = now - self.start_time
                if diff < ramptime:
                    speed = self.speed * diff / ramptime
                elif ramptime > 0:
                    projectairsim_log().info("reached full speed...")
                    ramptime = 0
                    
                lookahead_angle = speed / self.radius            

                # compute current angle
                pos = self.drone.get_ground_truth_kinematics()["pose"]["position"]
                dx = pos["x"] - self.center["x"]
                dy = pos["y"] - self.center["y"]
                actual_radius = math.sqrt((dx*dx) + (dy*dy))
                angle_to_center = math.atan2(dy, dx)

                camera_heading = angle_to_center - math.pi

                # compute lookahead
                lookahead_x = self.center["x"] + self.radius * math.cos(angle_to_center + lookahead_angle)
                lookahead_y = self.center["y"] + self.radius * math.sin(angle_to_center + lookahead_angle)

                vx = lookahead_x - pos["x"]
                vy = lookahead_y - pos["y"]

                self.do_snapshot = False
                if self.track_orbits(angle_to_center * 180 / math.pi):
                    count += 1
                    projectairsim_log().info("completed {} orbits".format(count))
                if self.do_snapshot:
                    await self.take_snapshot()
                
                self.camera_heading = camera_heading
                move_task = await self.drone.move_by_velocity_z_async(vx, vy, z, 1, YawControlMode.MaxDegreeOfFreedom, yaw=camera_heading, yaw_is_rate=False)
                await move_task

            move_task = await self.drone.move_to_position_async(start["x"], start["y"], z, 2)
            await move_task

            if self.takeoff:            
                # if we did the takeoff then also do the landing.
                if z < self.home["z"]:
                    projectairsim_log().info("descending")
                    move_task = await self.drone.move_to_position_async(start["x"], start["y"], self.home["z"] - 5, 2)
                    await move_task

                projectairsim_log().info("landing...")
                land_task = self.drone.land_async()
                await land_task

                projectairsim_log().info("disarming.")
                self.drone.disarm()
        except Exception as err:
            projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)
        finally:
            # Cancel all pending tasks before disconnecting
            pending_tasks = [t for t in asyncio.all_tasks() if t is not asyncio.current_task()]
            for task in pending_tasks:
                task.cancel()
                try:
                    await task
                except asyncio.CancelledError:
                    pass

            if self.client:
                self.client.disconnect()

    def track_orbits(self, angle):
        # tracking # of completed orbits is surprisingly tricky to get right in order to handle random wobbles
        # about the starting point.  So we watch for complete 1/2 orbits to avoid that problem.
        if angle < 0:
            angle += 360

        if self.start_angle is None:
            self.start_angle = angle
            if self.snapshot_delta:
                self.next_snapshot = angle + self.snapshot_delta
            self.previous_angle = angle
            self.shifted = False
            self.previous_sign = None
            self.previous_diff = None            
            self.quarter = False
            return False

        # now we just have to watch for a smooth crossing from negative diff to positive diff
        if self.previous_angle is None:
            self.previous_angle = angle
            return False            

        # ignore the click over from 360 back to 0
        if self.previous_angle > 350 and angle < 10:
            if self.snapshot_delta and self.next_snapshot >= 360:
                self.next_snapshot -= 360
            return False

        diff = self.previous_angle - angle
        crossing = False
        self.previous_angle = angle

        if self.snapshot_delta and angle > self.next_snapshot:            
            projectairsim_log().info("Taking snapshot at angle {}".format(angle))
            self.do_snapshot = True
            self.next_snapshot += self.snapshot_delta

        diff = abs(angle - self.start_angle)
        if diff > 45:
            self.quarter = True

        if self.quarter and self.previous_diff is not None and diff != self.previous_diff:
            # watch direction this diff is moving if it switches from shrinking to growing
            # then we passed the starting point.
            direction = self.sign(self.previous_diff - diff)
            if self.previous_sign is None:
                self.previous_sign = direction
            elif self.previous_sign > 0 and direction < 0:
                if diff < 45:
                    self.quarter = False
                    if self.snapshots <= self.snapshot_index + 1:
                        crossing = True
            self.previous_sign = direction
        self.previous_diff = diff

        return crossing

    async def take_snapshot(self):
        # first hold our current position so drone doesn't try and keep flying while we take the picture.
        pos = self.drone.get_ground_truth_kinematics()["pose"]["position"]
        move_task = await self.drone.move_to_position_async(pos["x"], pos["y"], self.z, 0.5, 10, YawControlMode.MaxDegreeOfFreedom, 
            yaw=self.camera_heading, yaw_is_rate=False)
        await move_task
        images = self.drone.get_images("DownCamera", [ImageType.SCENE])
        image = images[ImageType.SCENE]
        # the photos are saved to the folder the script is in, just as it in the original
        filename = "photo_" + str(self.snapshot_index)
        self.snapshot_index += 1
        img_np = numpy.reshape(image["data"], [image["height"], image["width"], 3])
        file_save_path = os.path.normpath(filename + '.png')
        cv2.imwrite(file_save_path, img_np)
        projectairsim_log().info("Saved snapshot: {}".format(filename))
        self.start_time = time.time()  # cause smooth ramp up to happen again after photo is taken.

    def sign(self, s):
        if s < 0: 
            return -1
        return 1

if __name__ == "__main__":
    args = sys.argv
    args.pop(0)
    arg_parser = argparse.ArgumentParser("Orbit.py makes drone fly in a circle with camera pointed at the given center vector")
    arg_parser.add_argument("--radius", type=float, help="radius of the orbit", default=10)
    arg_parser.add_argument("--altitude", type=float, help="altitude of orbit (in positive meters)", default=20)
    arg_parser.add_argument("--speed", type=float, help="speed of orbit (in meters/second)", default=3)
    arg_parser.add_argument("--center", help="x,y direction vector pointing to center of orbit from current starting position (default 1,0)", default="1,0")
    arg_parser.add_argument("--iterations", type=float, help="number of 360 degree orbits (default 3)", default=3)
    arg_parser.add_argument("--snapshots", type=float, help="number of FPV snapshots to take during orbit (default 0)", default=0)    
    args = arg_parser.parse_args(args)    
    nav = OrbitNavigator(args.radius, args.altitude, args.speed, args.iterations, args.center.split(','), args.snapshots)
    asyncio.run(nav.start())
