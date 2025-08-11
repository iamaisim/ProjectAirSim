import sys
import time
import argparse
import asyncio

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log
from projectairsim.drone import YawControlMode

class SurveyNavigator:
    def __init__(self, args):
        self.boxsize = args.size
        self.stripewidth = args.stripewidth
        self.altitude = args.altitude
        self.velocity = args.speed

        self.client = ProjectAirSimClient()
        self.client.connect()
        self.world = World(self.client, "scene_basic_drone.jsonc", delay_after_load_sec=2)
        self.drone = Drone(self.client, self.world, "Drone1")
        self.drone.enable_api_control()

    async def start(self):
        try:
            projectairsim_log().info("arming the drone...")
            self.drone.arm()

            projectairsim_log().info("taking off...")

            # in the original script there's a bunch of checks regarding whether the drone is landed
            # but it's guaranteed to start landed here
            takeoff_task = await self.drone.takeoff_async()
            await takeoff_task
            
            # Project AirSim uses NED coordinates so negative axis is up.
            x = -self.boxsize
            z = -self.altitude

            projectairsim_log().info("climbing to altitude: " + str(self.altitude))
            move_task = await self.drone.move_to_position_async(0, 0, z, self.velocity)
            await move_task

            projectairsim_log().info("flying to first corner of survey box")
            move_task = await self.drone.move_to_position_async(x, -self.boxsize, z, self.velocity)
            await move_task
            
            # let it settle there a bit.
            hover_task = await self.drone.hover_async()
            await hover_task
            time.sleep(2)

            # now compute the survey path required to fill the box 
            path = []
            distance = 0
            while x < self.boxsize:
                distance += self.boxsize 
                path.append([x, self.boxsize, z])
                x += self.stripewidth            
                distance += self.stripewidth 
                path.append([x, self.boxsize, z])
                distance += self.boxsize 
                path.append([x, -self.boxsize, z]) 
                x += self.stripewidth  
                distance += self.stripewidth 
                path.append([x, -self.boxsize, z])
                distance += self.boxsize 
            
            projectairsim_log().info("starting survey, estimated distance is " + str(distance))
            trip_time = distance / self.velocity
            projectairsim_log().info("estimated survey time is " + str(trip_time))
            try:
                path_task = await self.drone.move_on_path_async(path, self.velocity, trip_time, YawControlMode.ForwardOnly, False, 0.0, self.velocity + (self.velocity/2), 1)
                await path_task
            except:
                value = sys.exc_info()
                projectairsim_log().error("moveOnPath threw exception: " + str(value))
                pass

            projectairsim_log().info("flying back home")
            move_task = await self.drone.move_to_position_async(0, 0, z, self.velocity)
            await move_task
            
            if z < -5:
                projectairsim_log().info("descending")
                move_task = await self.drone.move_to_position_async(0, 0, -5, 2)
                await move_task

            projectairsim_log().info("landing...")
            land_task = await self.drone.land_async()
            await land_task

            projectairsim_log().info("disarming.")
            self.drone.disarm()
        finally:
            self.client.disconnect()

if __name__ == "__main__":
    args = sys.argv
    args.pop(0)
    arg_parser = argparse.ArgumentParser("Usage: survey boxsize stripewidth altitude")
    arg_parser.add_argument("--size", type=float, help="size of the box to survey", default=50)
    arg_parser.add_argument("--stripewidth", type=float, help="stripe width of survey (in meters)", default=10)
    arg_parser.add_argument("--altitude", type=float, help="altitude of survey (in positive meters)", default=30)
    arg_parser.add_argument("--speed", type=float, help="speed of survey (in meters/second)", default=5)
    args = arg_parser.parse_args(args)
    nav = SurveyNavigator(args)
    asyncio.run(nav.start())