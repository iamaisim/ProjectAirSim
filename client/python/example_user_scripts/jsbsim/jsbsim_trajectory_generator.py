"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Script to generate a trajectory from a simulated vehicle 
in a JSBSim script. The trajectory can then be consumed 
by an EnvActor using ImportNEDTrajectoryFromCSV (see 
jsbsim_env_actor.py). The trajectory has an update 
frequency of 10HZ independent of the delta t set in the 
JSBSim script. 
To use models from the JSBSim repository, download 
JSBSim from https://github.com/JSBSim-Team/jsbsim/releases 
then generate a trajectory, for example, for 
a Cessna c310 use the following command (the trajectory 
is already defined inside c3104.xml): 

python jsbsim_trajectory_generator.py \
    --root="{path to JSBSim folder}"  \
    --script="scripts\c3104.xml"\
    --out=JSBSimTrajectory.csv


Usage:
    python jsbsim_trajectory_generator.py \
        --root="{JSBSim root folder}"     \
        --script={JSBSim script relative path}     \
        --out={CSV output file}
"""
import jsbsim
import os
import math
import numpy
import argparse


class JSBSimSimulation:
    def getJSBSimPosition(self):
        # Get the position in NED coordinates
        # distance-from-start-* is modulus of distance from start so we use the lat/lon sign
        x = math.copysign(self.fdm["position/distance-from-start-lat-mt"],
                          self.fdm["position/lat-gc-deg"]-self.home_position_lat)
        y = math.copysign(self.fdm["position/distance-from-start-lon-mt"],
                          self.fdm["position/long-gc-deg"]-self.home_position_lon)
        z = -self.fdm["position/h-sl-meters"]
        return (x, y, z)

    def getJSBSimLinearVelocity(self):
        vx = self.fdm["velocities/v-north-fps"] * 0.3048
        vy = self.fdm["velocities/v-east-fps"] * 0.3048
        vz = self.fdm["velocities/v-down-fps"] * 0.3048

        return (vx, vy, vz)

    def getJSBSimOrientation(self):
        pitch = self.fdm["attitude/pitch-rad"]
        roll = self.fdm["attitude/roll-rad"]
        yaw = self.fdm["attitude/psi-deg"] * (math.pi / 180.0)
        return (roll, pitch, yaw)

    def save_segment(self, file):
        row = [self.fdm.get_sim_time()]
        row.extend(self.getJSBSimPosition())
        row.extend(self.getJSBSimOrientation())
        row.extend(self.getJSBSimLinearVelocity())
        a = numpy.asarray(row)
        a = [a]
        numpy.savetxt(file, a, fmt="%f", delimiter=",")

    def __init__(self, root_dir, script_path):
        root_dir = os.path.abspath(root_dir)
        self.fdm = jsbsim.FGFDMExec(root_dir=root_dir)
        self.fdm.load_script(script_path)
        success = self.fdm.run_ic()
        self.home_position_lat = self.fdm["position/lat-gc-deg"]
        self.home_position_lon = self.fdm["position/long-gc-deg"]
        if not success:
            raise RuntimeError(
                "JSBSim failed to initialise simulation conditions.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser("JSBSim Trajectory Generator")

    parser.add_argument("--root", help="JSBSim root folder",
                        default="C:\\jsbsim")

    parser.add_argument(
        "--script",
        help="JSBSim script path",
        default="scripts\\c3104.xml",
    )

    parser.add_argument(
        "--out", help="Path to CSV output file", default=".\\JSBSimTrajectory.csv"
    )

    args = parser.parse_args()
    simulation = JSBSimSimulation(args.root, args.script)

    f = open(args.out, "w")
    f.write(
        "time (s),pose_x (m),pose_y (m),pose_z (m),pose_roll (rad),pose_pitch (rad),pose_yaw (rad),vel_lin_x (m/s),vel_lin_y (m/s),vel_lin_z (m/s)\n"
    )
    simulation.save_segment(f)  # save initial state

    # Set relative segment update
    sim_frequency_hz = 1 / simulation.fdm.get_delta_t()
    actor_frequency_update = 10  # 10Hz
    relative_update = (
        actor_frequency_update / sim_frequency_hz
    )  # rate between the trajectory and JSBSim
    actor_update = 0

    i = 0
    old_time = simulation.fdm.get_sim_time()
    while simulation.fdm.run() and simulation.fdm.get_sim_time() > old_time:

        old_time = curr_time = simulation.fdm.get_sim_time()
        print(f"Simulation time: {curr_time}", end='\r')  # current JSBSim simulation time

        i += 1
        segment_i = relative_update * i
        actor_update_old = actor_update
        actor_update = segment_i // 1.0

        if actor_update > actor_update_old:
            simulation.save_segment(f)

    f.close()
