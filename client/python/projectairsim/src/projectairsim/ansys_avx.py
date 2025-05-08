"""
Copyright (C) Microsoft Corporation. All rights reserved.

Environment variable "ANSYS_ROOT" must be set to the root Ansys installation path:
e.g. "C:\Program Files\ANSYS Inc\v231"

AVX prereqs must be installed (gRPC, etc) in the Python environment and need to run
make_proto.bat to generate the avx\vss\ API scripts. To do it in the
"{ANSYS_ROOT}\Optical Products\VRXPERIENCE\APIs\VSS_API\Samples\python"
system installed location that's appended to the sys.path in this script, need to use an
Administrator-elevated command prompt when running it. See the following for more info:
"{ANSYS_ROOT}\Optical Products\VRXPERIENCE\APIs\VSS_API\Samples\python\README.md"

Also, the default AVX setting for data retention buffer size needs to be increased to
fix an error where our client ends up receiving empty data frames for the notified
data IDs. Increase the data retention number from 2 to 50000 in the following two JSONs:
{ANSYS_ROOT}\Optical Products\VRXPERIENCE\VSS\DataRetentionStrategy.config.json
{ANSYS_ROOT}\Optical Products\VRXPERIENCE\VSS\VSSSpawner\DataRetentionStrategy.config.json
"""

import os
import json
import time
import datetime
import sys
import queue
from threading import Thread
from typing import Dict, List, Tuple

import cv2
import numpy as np

from projectairsim.utils import projectairsim_log, quaternion_to_rpy

from grpc import insecure_channel, RpcError
import google.protobuf.empty_pb2 as gpb_em
from google.protobuf.json_format import Parse

# Import AVX protobuf modules from:
# {ANSYS_ROOT}/Optical Products/VRXPERIENCE/APIs/VSS_API/Samples/AvxApiPython/avx/vss folders
sys.path.append(
    os.environ["ANSYS_ROOT"]
    + r"/Optical Products/VRXPERIENCE/APIs/VSS_API/Samples/AvxApiPython"
)
from avx.vss.simulation import configuration_pb2
from avx.vss.simulation import simulation_parameters_pb2
from avx.vss.simulation import simulation_pb2_grpc
from avx.vss.simulation import world_update_pb2
from avx.vss.data_access import sensor_data_access_pb2_grpc
from avx.vss.data_access import sensor_data_output_notification_pb2_grpc
from avx.vss.sensor_data.sensor_data_pb2 import SensorData
from avx.vss.sensor_data_format_pb2 import PixelFormat


class AnsysAVXConnector:
    def __init__(
        self,
        asset_path: str,
        track_file: str,
        sim_parameter_file: str,
        sensor_config_file: str,
        update_pose_interval_ms: int = -1,
        start_datetime: str = "2021-9-21 9:00:00",
    ):
        # AVX config parameters
        self.vss_input_port = "54321"  # Default port value of AVX
        self.vss_output_port = "54545"  # AVX custom data access port
        self.vss_host = "localhost"  # Default host value of AVX
        self.vss_installation_dir = os.path.join(
            os.environ["ANSYS_ROOT"], "Optical Products/VRXPERIENCE/VSS"
        )  # AVX's default installation path
        self.avx_outputs_directory = None  # asset_path + "/output"
        self.avx_stub = None
        self.avx_data_notif_stub = None
        self.data_stub = None

        self.avx_thread = None

        self.avx_data_notif_thread = None
        self.run_avx_data_notif_thread = True

        self.update_avx_thread = None
        self.run_update_avx_thread = True

        self.update_pose_interval_nanos = update_pose_interval_ms * 1000000

        self.asset_path = asset_path

        # Track file that contains the static environment scene data
        self.track_file = os.path.join(self.asset_path, track_file)

        # Sensor config and sim parameter settings
        self.simulation_parameter_path = os.path.join(
            self.asset_path, sim_parameter_file
        )
        self.sensor_config_path = os.path.join(self.asset_path, sensor_config_file)

        # Note: This sets the AVX time-of-day and sun position for lighting the scene.
        start_datetime_obj = datetime.datetime.strptime(
            start_datetime, "%Y-%m-%d %H:%M:%S"
        )
        self.starting_timestamp = int(time.mktime(start_datetime_obj.timetuple()))

        self.max_msg_data_bytes = int(80 * 1e6)  # 80 MB

        # (name, asset filename, origin dict)
        self.avx_actors: List(Tuple) = []
        self.avx_objects: List(Tuple) = []

        self.robot_pose_q = queue.SimpleQueue()

        self.image_data_callback = None
        self.thermal_data_callback = None
        self.lidar_data_callback = None

    def add_actor(self, name: str, asset_filename: str, origin: Dict):
        # TODO Handle multiple robots
        if len(self.avx_actors) > 0:
            projectairsim_log().error(
                f"Only one actor is currently supported. {name} will not be added."
            )
            return
        avx_origin = self.ned_to_avx_origin(origin)
        self.avx_actors.append((name, asset_filename, avx_origin))

    def add_object(self, name: str, asset_filename: str, origin: Dict, tag_name: str):
        avx_origin = self.ned_to_avx_origin(origin)
        self.avx_objects.append((name, asset_filename, avx_origin, tag_name))

    def push_robot_pose(self, pose_msg):
        # TODO Handle multiple robots
        if (
            self.robot_pose_q.empty()
        ):  # and (pose_msg["time_stamp"] % (interval_ms * 1e6)) == 0:
            self.robot_pose_q.put(pose_msg)

    def set_image_data_callback(self, callback_func):
        self.image_data_callback = callback_func

    def set_thermal_data_callback(self, callback_func):
        self.thermal_data_callback = callback_func

    def set_lidar_data_callback(self, callback_func):
        self.lidar_data_callback = callback_func

    def start_avx(self):
        # Start AVX Sensor simulator in a separated thread
        projectairsim_log().info("Starting AVX process...")
        self.avx_thread = Thread(
            target=self.avx_launch_vss,
            args=(
                self.vss_installation_dir,
                self.vss_input_port,
                self.vss_output_port,
                self.avx_outputs_directory,
            ),
        )
        self.avx_thread.start()

        self.update_avx_thread = Thread(
            target=self.update_avx_poses, args=(lambda: self.run_update_avx_thread,)
        )
        self.avx_data_notif_thread = Thread(
            target=self.avx_data_notification,
            args=(lambda: self.run_avx_data_notif_thread,),
        )
        # Update AVX thread and data notification thread will be started after AVX
        # scene is initialized.

    def connect_avx(self):
        # -- Create a client connection to AVX --
        projectairsim_log().info("Create a client connection to AVX...")
        channel = insecure_channel(self.vss_host + ":" + self.vss_input_port)
        self.avx_stub = simulation_pb2_grpc.SimulationStub(channel)

        self.avx_data_notif_stub = (
            sensor_data_output_notification_pb2_grpc.SensorDataNotifierStub(channel)
        )

        data_channel = insecure_channel(
            self.vss_host + ":" + self.vss_output_port,
            options=[
                ("grpc.max_send_message_length", self.max_msg_data_bytes),
                ("grpc.max_receive_message_length", self.max_msg_data_bytes),
            ],
        )
        self.data_stub = sensor_data_access_pb2_grpc.DataAccessStub(data_channel)

        # Manually give AVX time to finish launching until figuring out a way to use
        # the Status gRPC message to do a wait loop
        time.sleep(10)

        projectairsim_log().info("Done creating a client connection to AVX.")

    def load_avx_scene(self):
        # -- Load AVX scene --
        projectairsim_log().info("Load AVX scene...")
        my_configuration = configuration_pb2.Configuration()
        my_configuration.scene.track.id = self.track_file
        my_configuration.deploy_parameters.host = self.vss_host
        my_configuration.deploy_parameters.min_port = 13000
        my_configuration.deploy_parameters.max_port = 13100
        json_file = open(self.simulation_parameter_path, encoding="utf-8")
        data = json.load(json_file)
        my_configuration.simulation_parameters.CopyFrom(
            Parse(json.dumps(data), simulation_parameters_pb2.SimulationParameters())
        )
        file = open(self.sensor_config_path, "rb")
        my_configuration.sensors.sensor_configuration = file.read()

        for name, asset_filename, _ in self.avx_actors:
            asset = my_configuration.scene.assets.add()
            asset.resource.id = self.asset_path + asset_filename
            asset.vss_identity.id = name

        for name, asset_filename, _, _ in self.avx_objects:
            asset = my_configuration.scene.assets.add()
            asset.resource.id = self.asset_path + asset_filename
            asset.vss_identity.id = name

        # Hard-coded to use first actor's name as the ego actor ID
        my_configuration.ego_vehicle_identity.id = self.avx_actors[0][0]

        self.avx_stub.Load(my_configuration, wait_for_ready=True)

    def initialize_avx_scene(self):
        # -- Initialize scene state --
        projectairsim_log().info("Initialize AVX scene...")
        world_update = world_update_pb2.WorldUpdate()
        world_update.simulation_time.nanos = 0
        world_update.simulation_time.seconds = 0
        for name, _, origin in self.avx_actors:
            asset_update = world_update.object_updates.add()
            asset_update.key.vss_identity.id = name
            asset_update.key.tag.name = "Vehicle"
            asset_update.kinematic_properties.position.x = origin["xyz"][0]
            asset_update.kinematic_properties.position.y = origin["xyz"][1]
            asset_update.kinematic_properties.position.z = origin["xyz"][2]
            asset_update.kinematic_properties.orientation.roll = origin["rpy"][0]
            asset_update.kinematic_properties.orientation.pitch = origin["rpy"][1]
            asset_update.kinematic_properties.orientation.yaw = origin["rpy"][2]
            asset_update.kinematic_properties.velocity.x = 0
            asset_update.kinematic_properties.velocity.y = 0
            asset_update.kinematic_properties.velocity.z = 0

        for name, _, origin, tag_name in self.avx_objects:
            asset_update = world_update.object_updates.add()
            asset_update.key.vss_identity.id = name
            # TODO Only seems to be spawned if tagged as a "Vehicle". The "Object" tag
            # doesn't work for some reason.
            asset_update.key.tag.name = tag_name
            asset_update.kinematic_properties.position.x = origin["xyz"][0]
            asset_update.kinematic_properties.position.y = origin["xyz"][1]
            asset_update.kinematic_properties.position.z = origin["xyz"][2]
            asset_update.kinematic_properties.orientation.roll = origin["rpy"][0]
            asset_update.kinematic_properties.orientation.pitch = origin["rpy"][1]
            asset_update.kinematic_properties.orientation.yaw = origin["rpy"][2]
            asset_update.kinematic_properties.velocity.x = 0
            asset_update.kinematic_properties.velocity.y = 0
            asset_update.kinematic_properties.velocity.z = 0

        env_update = world_update.environment_updates.add()
        env_update.date_time.seconds = self.starting_timestamp

        self.avx_stub.Initialize(world_update, wait_for_ready=True)

        # Start processing pose updates
        self.update_avx_thread.start()

        # Start receiving sensor data notifications
        self.avx_data_notif_thread.start()

    def stop_avx_scene(self):
        projectairsim_log().info("Stop AVX scene...")

        self.run_avx_data_notif_thread = False
        if self.avx_data_notif_thread.is_alive():
            self.avx_data_notif_thread.join()

        self.run_update_avx_thread = False
        if self.update_avx_thread.is_alive():
            self.update_avx_thread.join()

    def shutdown_avx(self):
        projectairsim_log().info("Shut down AVX process...")
        try:
            # Just killing AVX without stopping/unloading seems more reliable...
            # self.avx_stub.Stop(gpb_em.Empty(), wait_for_ready=True)
            # self.avx_stub.Unload(gpb_em.Empty(), wait_for_ready=True)
            self.avx_stub.Kill(gpb_em.Empty(), wait_for_ready=False)
            if self.avx_thread.is_alive():
                self.avx_thread.join()
        except Exception as err:
            projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)
            pass

    def avx_launch_vss(
        self, vss_installation_dir, vss_input_port, data_access=None, output_dir=None
    ):
        """
        Start AVX Sensor Simulator with data access and feedback control option
        """
        command = [
            "cd",
            "/d",
            f'"{vss_installation_dir.__str__()}"',
            "&&",
            "VRXPERIENCE.SensorsSimulator.exe",
            "-p",
            vss_input_port,
            "-d",
            data_access,
            "-fbc",
            "-w",  # better to always add this to prevent workgroup license rejection
        ]
        if output_dir:
            try:
                os.makedirs(output_dir)
            except FileExistsError:
                # directory already exists
                pass
            command.extend(["--outputDir", f'"{output_dir}"'])
        command = " ".join(command)
        os.system(command)

    def avx_data_notification(self, run_lambda):
        projectairsim_log().info("Start avx_data_notification() thread...")
        data_descriptions_stream = self.avx_data_notif_stub.Subscribe(gpb_em.Empty())

        while run_lambda():
            for data_description in data_descriptions_stream:
                # data_time_sec = (
                #     data_description.simulation_time.seconds
                #     + data_description.simulation_time.nanos / 1e9
                # )

                try:
                    sensor_data_buffer = self.data_stub.RequestData(
                        data_description.data_id, wait_for_ready=True
                    )
                    sensor_data = SensorData()
                    sensor_data.ParseFromString(sensor_data_buffer.data)

                    if (
                        data_description.metadata.HasField("camera_metadata")
                        and len(sensor_data.camera_data.entries) > 0
                    ):
                        # Process camera sensor image data
                        raw_cam = sensor_data.camera_data.entries[
                            0
                        ].image_data.camera_data
                        img_width = (
                            data_description.metadata.camera_metadata.image_width
                        )
                        img_height = (
                            data_description.metadata.camera_metadata.image_height
                        )

                        img = np.frombuffer(raw_cam, dtype="uint8")

                        # check if pixel format or not
                        pixel_format = PixelFormat.Name(
                            data_description.metadata.camera_metadata.pixel_format
                        )

                        # Convert AVX RGBA image data buffer into AirSim BGR buffer if RGBA32 format
                        if pixel_format == "RGBA32":
                            depth = 4
                        elif pixel_format == "GRAY8":
                            depth = 1

                        img = np.reshape(img, (img_height, img_width, depth))

                        if pixel_format == "RGBA32":
                            img = cv2.cvtColor(img, cv2.COLOR_RGBA2BGR)
                            img = np.reshape(img, (img_height * img_width * 3))

                            airsim_image_data = {
                                "data": img,
                                "encoding": "BGR",
                                "height": img_height,
                                "width": img_width,
                            }
                            if self.image_data_callback is not None:
                                self.image_data_callback(airsim_image_data)

                        elif pixel_format == "GRAY8":
                            airsim_image_data = {
                                "data": img,
                                "encoding": pixel_format,
                                "height": img_height,
                                "width": img_width,
                            }
                            if self.thermal_data_callback is not None:
                                self.thermal_data_callback(airsim_image_data)

                    elif data_description.metadata.HasField("lidar_metadata"):
                        # Process lidar sensor point cloud data
                        lidar_frame = np.array(
                            sensor_data.lidar_data.entries[0]
                            .point_cloud_data.point_clouds[0]
                            .data
                        )
                        point_cloud_xyz = []
                        intensity_cloud = []
                        assert len(lidar_frame) % 4 == 0
                        for i in range(0, len(lidar_frame), 4):
                            ned_x, ned_y, ned_z = self.avx_to_ned_xyz(
                                (lidar_frame[i], lidar_frame[i + 1], lidar_frame[i + 2])
                            )
                            point_cloud_xyz.append(ned_x)
                            point_cloud_xyz.append(ned_y)
                            point_cloud_xyz.append(ned_z)
                            intensity_cloud.append(lidar_frame[i + 3])

                        airsim_lidar_data = {
                            "point_cloud": point_cloud_xyz,
                            "intensity_cloud": intensity_cloud,
                        }

                        if self.lidar_data_callback is not None:
                            self.lidar_data_callback(airsim_lidar_data)
                except RpcError as rpc_error:
                    projectairsim_log().error("rpc_error = %s", str(rpc_error))

                if not run_lambda():
                    data_descriptions_stream.cancel()
                    break

        projectairsim_log().info("End avx_data_notification() thread.")

    def update_avx_poses(self, run_lambda):
        projectairsim_log().info("Start update_avx_poses() thread...")
        prev_timestamp = 0

        while run_lambda():
            if self.robot_pose_q.empty():
                time.sleep(0.001)
                continue

            pose_msg = self.robot_pose_q.get()
            pos = pose_msg["position"]
            rot = pose_msg["orientation"]

            if (
                pose_msg["time_stamp"] - prev_timestamp
            ) < self.update_pose_interval_nanos:
                time.sleep(0.001)
                continue

            # -- Step AVX sim --
            world_update = world_update_pb2.WorldUpdate()
            world_update.simulation_time.nanos = pose_msg["time_stamp"] % 1000000000
            world_update.simulation_time.seconds = int(
                pose_msg["time_stamp"] / 1000000000
            )
            for name, _, _ in self.avx_actors:
                asset_update = world_update.object_updates.add()
                asset_update.key.vss_identity.id = name
                asset_update.key.tag.name = "Vehicle"
                avx_x, avx_y, avx_z = self.ned_to_avx_xyz(
                    (pos["x"], pos["y"], pos["z"])
                )
                avx_roll, avx_pitch, avx_yaw = self.ned_to_avx_rpy(
                    quaternion_to_rpy(rot["w"], rot["x"], rot["y"], rot["z"])
                )
                asset_update.kinematic_properties.position.x = avx_x
                asset_update.kinematic_properties.position.y = avx_y
                asset_update.kinematic_properties.position.z = avx_z
                asset_update.kinematic_properties.orientation.pitch = avx_pitch
                asset_update.kinematic_properties.orientation.roll = avx_roll
                asset_update.kinematic_properties.orientation.yaw = avx_yaw
                # TODO Get full kinematics topic from sim and pass velocity to AVX
                asset_update.kinematic_properties.velocity.x = 0
                asset_update.kinematic_properties.velocity.y = 0
                asset_update.kinematic_properties.velocity.z = 0

            t1 = time.time()
            response = self.avx_stub.Update(world_update, wait_for_ready=True)
            t2 = time.time()

            # projectairsim_log().info(
            #     f"Received message after RUNNING sim step {pose_msg['time_stamp'] / 1e9}:",
            #     f" {response.message} Elapsed time={int((t2-t1)*1000)} ms",
            # )

            prev_timestamp = pose_msg["time_stamp"]

        projectairsim_log().info("End update_avx_poses() thread.")

    def ned_to_avx_xyz(self, ned_xyz: Tuple):
        # NED x -> AVX -z
        # NED y -> AVX +x
        # NED z -> AVX -y
        avx_xyz = (ned_xyz[1], -ned_xyz[2], -ned_xyz[0])
        return avx_xyz

    def avx_to_ned_xyz(self, avx_xyz: Tuple):
        # NED x <- AVX -z
        # NED y <- AVX +x
        # NED z <- AVX -y
        ned_xyz = (-avx_xyz[2], avx_xyz[0], -avx_xyz[1])
        return ned_xyz

    def ned_to_avx_rpy(self, ned_rpy: Tuple):
        # NED roll -> AVX -roll
        # NED pitch -> AVX pitch
        # NED yaw -> AVX -yaw
        avx_rpy = (-ned_rpy[0], ned_rpy[1], -ned_rpy[2])
        return avx_rpy

    def ned_to_avx_origin(self, origin: Dict):
        xyz = origin["xyz"]
        rpy = origin["rpy"]
        avx_x, avx_y, avx_z = self.ned_to_avx_xyz((xyz[0], xyz[1], xyz[2]))
        avx_roll, avx_pitch, avx_yaw = self.ned_to_avx_rpy((rpy[1], rpy[0], rpy[2]))
        origin["xyz"] = [avx_x, avx_y, avx_z]
        origin["rpy"] = [avx_roll, avx_pitch, avx_yaw]
        return origin
