"""Compare depth_lidar depth and point clouds against a planar depth camera."""

import asyncio
import time
from pathlib import Path

import numpy as np

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.image_utils import ImageDisplay
from projectairsim.lidar_utils import LidarTopDownRenderer
from projectairsim.utils import projectairsim_log


def resolve_lidar_cloud_topic(drone: Drone, sensor_id: str = "lidar1") -> str:
    sensor_topics = drone.sensors.get(sensor_id)
    if sensor_topics is None:
        available_sensors = ", ".join(sorted(drone.sensors.keys()))
        raise RuntimeError(
            f"Sensor '{sensor_id}' not found in robot config. "
            f"Available sensors: [{available_sensors}]"
        )

    return sensor_topics["lidar"]


def lidar_point_count(lidar_msg) -> int:
    if lidar_msg is None or "point_cloud" not in lidar_msg:
        return 0

    return int(np.asarray(lidar_msg["point_cloud"], dtype=np.float32).size // 3)


def safe_topic_callback(callback_name: str, callback):
    def wrapped(topic, msg):
        try:
            callback(topic, msg)
        except Exception as err:
            projectairsim_log().error(
                f"Callback '{callback_name}' failed for topic '{topic.path}': {err}",
                exc_info=True,
            )

    return wrapped


class PointCloudStats:
    def __init__(self, cloud_name: str):
        self.cloud_name = cloud_name
        self.message_count = 0
        self.last_report_wall = time.time()

    def receive(self, cloud_msg):
        point_count = lidar_point_count(cloud_msg)
        if point_count == 0:
            return

        self.message_count += 1
        now = time.time()
        if self.message_count == 1 or now - self.last_report_wall >= 1.0:
            points = np.asarray(cloud_msg["point_cloud"], dtype=np.float32).reshape((-1, 3))
            projectairsim_log().info(
                f"{self.cloud_name}: msg={self.message_count} "
                f"points={point_count} "
                f"forward=[{float(points[:, 0].min()):.3f},{float(points[:, 0].max()):.3f}] "
                f"right=[{float(points[:, 1].min()):.3f},{float(points[:, 1].max()):.3f}] "
                f"down=[{float(points[:, 2].min()):.3f},{float(points[:, 2].max()):.3f}]"
            )
            self.last_report_wall = now


async def main():
    client = ProjectAirSimClient()
    lidar_cloud_stats = PointCloudStats("lidar1.lidar")
    image_display = ImageDisplay(num_subwin=1, subwin_width=640, subwin_height=640)
    lidar_cloud_window = "Lidar PointCloud"
    lidar_cloud_renderer = LidarTopDownRenderer(
        width=640,
        height=640,
        range_m=60.0,
    )
    lidar_cloud_topic = None
    drone = None

    try:
        client.connect()

        sim_config_path = str(Path(__file__).resolve().parent / "sim_config")
        world = World(
            client,
            "scene_lidar_depth.jsonc",
            delay_after_load_sec=2,
            sim_config_path=sim_config_path,
        )
        drone = Drone(client, world, "Drone1")

        lidar_cloud_topic = resolve_lidar_cloud_topic(drone, "lidar1")
        projectairsim_log().info(
            f"Subscribing lidar pointcloud stream: {lidar_cloud_topic}"
        )

        image_display.add_image(lidar_cloud_window, subwin_idx=0)
        image_display.start()

        client.subscribe(
            lidar_cloud_topic,
            safe_topic_callback(
                "lidar_cloud",
                lambda _, lidar_msg: handle_lidar_cloud(
                    lidar_msg,
                    lidar_cloud_stats,
                    lidar_cloud_renderer,
                    image_display,
                    lidar_cloud_window,
                ),
            ),
        )
        drone.enable_api_control()
        drone.arm()

        projectairsim_log().info("Ascending")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-2.0, duration=3.0
        )
        await move_task

        projectairsim_log().info("Moving north")
        move_task = await drone.move_by_velocity_async(
            v_north=4.0, v_east=0.0, v_down=0.0, duration=5.0
        )
        await move_task

        projectairsim_log().info("Moving east")
        move_task = await drone.move_by_velocity_async(
            v_north=0.0, v_east=4.0, v_down=0.0, duration=5.0
        )
        await move_task

        projectairsim_log().info("Returning")
        move_task = await drone.move_by_velocity_async(
            v_north=-4.0, v_east=-4.0, v_down=2.0, duration=5.0
        )
        await move_task

        projectairsim_log().info(
            "GPU depth lidar summary: "
            f"lidar_cloud_messages={lidar_cloud_stats.message_count}"
        )

        if lidar_cloud_stats.message_count == 0:
            raise RuntimeError(
                "depth_lidar did not publish any point clouds."
            )

    finally:
        if lidar_cloud_topic is not None:
            client.unsubscribe(lidar_cloud_topic)

        if drone is not None:
            drone.disarm()
            drone.disable_api_control()

        image_display.stop()
        client.disconnect()


def handle_lidar_cloud(
    lidar_msg,
    lidar_cloud_stats,
    lidar_cloud_renderer,
    image_display,
    lidar_cloud_window,
):
    lidar_cloud_stats.receive(lidar_msg)
    image_display.receive(
        lidar_cloud_renderer.render_image_msg(lidar_msg),
        lidar_cloud_window,
    )


if __name__ == "__main__":
    asyncio.run(main())
