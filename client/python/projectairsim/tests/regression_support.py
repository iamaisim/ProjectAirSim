"""Host-specific regression fixtures; never modify the production client API.

Runtime retains fresh configurations for controller/clock isolation. Unreal
loads one composed scene and binds every test client to that existing scene.
"""
from copy import deepcopy
from math import radians
from pathlib import Path

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.env_actor import EnvActor
from projectairsim.types import Pose, Quaternion, Vector3
from projectairsim.utils import load_scene_config_as_dict

CONFIG = Path(__file__).resolve().parent / "sim_config"
HOST = "unreal"
SESSION = None
LIDARS = {
    "LidarCPU": "scene_test_lidar_drone.jsonc",
    "LidarGPU": "scene_test_lidar_drone_gpu.jsonc",
    "LidarDepth": "scene_test_lidar_depth.jsonc",
    "LidarAvia": "scene_test_lidar_avia.jsonc",
    "LidarMid70": "scene_test_lidar_mid70.jsonc",
}


def read_scene(name, directory=CONFIG):
    return load_scene_config_as_dict(name, str(directory))[0]


def runtime_scene(config):
    config = deepcopy(config)
    for actor in config["actors"]:
        robot = actor.get("robot-config", {})
        robot["sensors"] = [s for s in robot.get("sensors", [])
                            if s["type"] not in {"camera", "lidar", "radar", "distance-sensor"}]
    return config


def unreal_scene():
    """Compose existing fixtures without duplicating their robot definitions."""
    config = read_scene("scene_test_drone.jsonc")
    config["id"] = "SceneUnrealRegression"
    sensors = config["actors"][0]["robot-config"]["sensors"]
    # SwitchStreamingView applies the streaming camera resolution to the
    # viewport. Keep it consistent with the headless CI launch dimensions.
    chase = next(s for s in sensors if s["id"] == "Chase")
    chase["capture-settings"][0].update(width=640, height=480)
    for sensor_id, scene in LIDARS.items():
        sensor = next(s for s in read_scene(scene)["actors"][0]["robot-config"]["sensors"]
                      if s["type"] == "lidar")
        sensor["id"] = sensor_id
        if sensor_id == "LidarCPU":
            # The old standalone stress fixture casts one million rays/second.
            # Use 100k in the combined regression workload; the existing
            # throughput floor and object-detection assertions remain unchanged.
            sensor["points-per-second"] = 100_000
        sensors.append(sensor)
    for sensor_id, scene in [("SensorCamera", "scene_test_drone_sensors.jsonc")]:
        sensor = next(s for s in read_scene(scene)["actors"][0]["robot-config"]["sensors"]
                      if s["id"] == "DownCamera")
        sensor["id"] = sensor_id
        # Annotation tests use request/reply, not high-rate publication.
        # Avoid continuously duplicating the benchmark camera workload.
        sensor["capture-interval"] = 0.1
        sensors.append(sensor)
    collection = read_scene("scene_basic_drone.jsonc", CONFIG.parent / "test_datacollection/configs/sim_config")
    config["environment-actors"] = collection["environment-actors"]
    config["home-geo-point"] = collection["home-geo-point"]
    config["environment-actors"][0]["origin"]["xyz"] = "-500 0 -5"
    return config


class RegressionWorld(World):
    def __init__(self, client, scene_config_name, delay_after_load_sec=0,
                 sim_config_path=CONFIG, **kwargs):
        if HOST == "runtime":
            super().__init__(client, scene_config_name, delay_after_load_sec,
                             str(sim_config_path), **kwargs)
        else:
            if SESSION is None:
                raise RuntimeError("Unreal regression session fixture was not initialized")
            supported = set(LIDARS.values()) | {
                "scene_test_drone.jsonc", "scene_test_drone_sensors.jsonc",
                "scene_test_drone_camera.jsonc", "scene_test_drone_lidar.jsonc",
                "scene_basic_drone.jsonc",
            }
            if scene_config_name not in supported:
                raise ValueError(f"Add {scene_config_name} to the shared Unreal scene explicitly")
            # No LoadScene, delay, or import RPC: this is a new client handle
            # for the same server scene, not a new simulation.
            self.__dict__.update(SESSION.world.__dict__)
            self.client = client
            # The scene is unchanged, so its catalog can be shared safely.
            client.topics = deepcopy(SESSION.topics)
            client.topic_info_updated = True

    def load_scene(self, config, delay_after_load_sec=0):
        if HOST != "runtime":
            raise RuntimeError("Unreal regressions must not reload the shared scene")
        return super().load_scene(runtime_scene(config), delay_after_load_sec)


class UnrealSession:
    def __init__(self):
        self.client = ProjectAirSimClient()
        self.client.connect()
        try:
            self.config = unreal_scene()
            # The composed configuration already contains expanded robot and
            # environment-actor definitions. Initialize the handle directly so
            # World.__init__ does not issue an RPC against the default scene
            # before our first and only load.
            self.world = World.__new__(World)
            self.world.client = self.client
            self.world.sim_config_path = str(CONFIG)
            self.world.sim_instance_idx = -1
            self.world.load_scene(self.config, delay_after_load_sec=2)
            self.world.import_ned_trajectory("null_trajectory", [0, 1],
                                            [0, 0], [0, 0], [0, 0],
                                            [0, 0], [0, 0], [0, 0], [0, 0])
            self.world.switch_streaming_view()
            self.drone = Drone(self.client, self.world, "Drone1")
            self.env_actor = EnvActor(self.client, self.world, "TestEnvActor")
            self.object_pose = self.world.get_object_pose("OrangeBall")
            self.object_scale = self.world.get_object_scale("OrangeBall")
            self.segmentation = self.world.get_segmentation_id_map()
            self.sun = self.world.get_sunlight_intensity()
            self.cloud = self.world.get_cloud_shadow_strength()
            # Initialize a known date; querying the host before SetTimeOfDay is
            # undefined on existing Windows builds.
            self.time_of_day = "2025-06-21 12:00:00"
            self.topics = deepcopy(self.client.topics)
            # PAS topics have one native consumer. Keep the session's reset
            # client on services only so test subscriptions own that channel.
            self.client.disconnect()
            self.client = ProjectAirSimClient()
            self.client.connect_services()
            self.world.client = self.client
            self.drone.client = self.client
            self.env_actor.client = self.client
        except BaseException:
            self.client.disconnect()
            raise

    def reset(self, restore_segmentation=False):
        """Restore mutable state without unloading actors or their sensors."""
        self.world.resume()
        self.drone.cancel_last_task()
        self.drone.disarm()
        self.drone.disable_api_control()
        self.world.pause()
        self.drone.set_pose(Pose({"translation": Vector3({"x": 0, "y": 0, "z": -4}),
                                 "rotation": Quaternion({"w": 1, "x": 0, "y": 0, "z": 0}),
                                 "frame_id": "DEFAULT_ID"}))
        self.world.destroy_all_spawned_objects()
        self.world.set_object_pose("OrangeBall", self.object_pose, True)
        self.world.set_object_scale("OrangeBall", self.object_scale)
        self.world.reset_weather_effects()
        self.world.disable_weather_visual_effects()
        self.world.set_wind_velocity(0, 0, 0)
        self.world.set_sunlight_intensity(self.sun)
        self.world.set_cloud_shadow_strength(self.cloud)
        self.world.set_time_of_day(True, self.time_of_day, False, 0, 1, True)
        self.world.flush_persistent_markers()
        self.env_actor.set_trajectory("null_trajectory", to_loop=True,
                                      time_offset=self.world.get_sim_time() / 1e9,
                                      x_offset=-500, z_offset=-5)
        for sensor in self.config["actors"][0]["robot-config"]["sensors"]:
            if sensor["type"] != "camera":
                continue
            origin = sensor["origin"]
            xyz = [float(v) for v in origin["xyz"].split()]
            rpy = [radians(float(v)) for v in origin["rpy-deg"].split()]
            # scipy avoids the client's deliberate +/-89.9 degree clamp; the
            # original fixture includes cameras pointing exactly down.
            from scipy.spatial.transform import Rotation
            x, y, z, w = Rotation.from_euler("xyz", rpy).as_quat()
            pose = Pose({"translation": Vector3(dict(zip("xyz", xyz))),
                         "rotation": Quaternion(dict(zip("wxyz", (w, x, y, z)))),
                         "frame_id": "DEFAULT_ID"})
            self.drone.set_camera_pose(sensor["id"], pose, wait_for_pose_update=False)
            for capture in sensor["capture-settings"]:
                self.drone.set_field_of_view(sensor["id"], capture["image-type"],
                                             capture["fov-degrees"])
        if restore_segmentation:
            for name, seg_id in self.segmentation.items():
                if name.startswith("TemplateCube_Rounded"):
                    self.world.set_segmentation_id_by_name(name, seg_id, False, True)
        self.world.resume()

    def close(self):
        try:
            self.world.resume()
            self.drone.cancel_last_task()
            self.drone.disarm()
        finally:
            self.client.disconnect()
