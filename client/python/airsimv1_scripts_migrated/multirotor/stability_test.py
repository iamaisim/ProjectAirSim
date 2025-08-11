import os
import time
import numpy as np
import sys
import asyncio
import speaker
import wav_reader

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log

script_dir = os.path.dirname(__file__)

def play_sound(wavfile):
    reader = wav_reader.WavReader()
    reader.open(wavfile, 512, speaker.Speaker())
    while True:
        buffer = reader.read()
        if buffer is None:
            break

class Numbers:
    def __init__(self, name):
        self.data = []
        self.name = name

    def add(self, x):
        self.data += [x]

    def is_unstable(self, amount):
        a = np.array(self.data)
        minimum = a.min()
        maximum = a.max()
        mean = np.mean(a)
        stddev = np.std(a)
        projectairsim_log().info("{}: min={}, max={}, mean={}, stddev={}".format(self.name, minimum, maximum, mean, stddev))
        return (maximum - minimum) > amount

async def main():
    client = ProjectAirSimClient()
    client.connect()
    try:
        world = World(client, "scene_drone_classic.jsonc", delay_after_load_sec=2)
        drone = Drone(client, world, "Drone1")
        drone.enable_api_control()

        projectairsim_log().info("### TEST STARTED ###")
        projectairsim_log().info("This test takes 20 minutes.")

        iteration = 0
        while iteration < 10:
            iteration  += 1
            x = Numbers("x")
            y = Numbers("y")
            z = Numbers("z")
                
            projectairsim_log().info("arming the drone...")
            drone.arm()

            projectairsim_log().info("taking off...")
            takeoff_task = drone.takeoff_async()
            await takeoff_task
            time.sleep(1)

            # fly for 2 minutes
            start = time.time()
            while time.time() < start + 120:
                state = drone.get_ground_truth_kinematics()
                print(state)
                x_val = state["pose"]["position"]["x"]
                y_val = state["pose"]["position"]["y"]
                z_val = state["pose"]["position"]["z"]
                x.add(x_val)
                y.add(y_val)
                z.add(z_val)
                projectairsim_log().info("x: {}, y: {}, z: {}".format(x_val, y_val, z_val))
                time.sleep(1)

            projectairsim_log().info("landing...")
            land_task = await drone.land_async()
            await land_task

            projectairsim_log().info("disarming the drone...")
            drone.disarm()

            # more than 50 centimeter drift is unacceptable.
            projectairsim_log().info("Results for iteration {}".format(iteration))
            a = x.is_unstable(0.5)
            b = y.is_unstable(0.5)
            c = z.is_unstable(0.5)

            if a or b or c:
                play_sound(os.path.join(script_dir, "Error.wav"))
                projectairsim_log().info("### Test Failed ###")
                break

            time.sleep(5)
        if iteration == 10:
            projectairsim_log().info("### Test Passed ###")
    finally:
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())