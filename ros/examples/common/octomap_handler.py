import asyncio
from asyncio.exceptions import InvalidStateError
import os
import pickle

import rospy
import moveit_msgs
import moveit_msgs.msg as rosmoveitmsgs
import octomap_msgs.msg as rosoctomapmsgs

from projectairsim.utils import projectairsim_log


class OctomapHandler:
    def __init__(self):
        self.subscriber = None

    def __del__(self):
        self.shutdown()

    def shutdown(self):
        if self.subscriber is not None:
            self.subscriber.unregister()
            self.subscriber = None

    def handle_planning_scene(self, planning_scene: rosmoveitmsgs.PlanningScene):
        try:
            projectairsim_log().info(f"Saving octomap to file {self.path_octomap}")

            # Save the octomap to the file
            with open(self.path_octomap, "wb") as file_out:
                pickle.dump(planning_scene.world.octomap, file_out)

        except Exception as e:
            self.exception_save_octomap = e  # Save exception for save_octomap

        finally:
            self.shutdown()
            self.octomap_save_is_completed = True

    async def load_octomap(self, path: str):
        # Load the octomap object
        with open(os.path.abspath(path), "rb") as file_in:
            octomap_with_pose = pickle.load(file_in)

        if type(octomap_with_pose) is not rosoctomapmsgs.OctomapWithPose:
            raise ValueError("File data is not an octomap")

        # Connect to the MoveIt! planning scene service
        apply_planning_scene = rospy.ServiceProxy(
            name="apply_planning_scene",
            service_class=moveit_msgs.srv.ApplyPlanningScene,
        )
        loops_remaining = 30
        while True:
            try:
                apply_planning_scene.wait_for_service(timeout=1.0)
                break
            except TimeoutError:
                loops_remaining -= 1
                if loops_remaining <= 0:
                    raise

        # Create message with octomap and set the planning scene
        planning_scene = rosmoveitmsgs.PlanningScene()
        planning_scene.is_diff = False
        planning_scene.world.octomap = octomap_with_pose
        planning_scene.world.octomap.header.stamp = rospy.Time.now()
        apply_planning_scene(planning_scene)

    async def save_octomap_async(self, path: str):
        if self.subscriber is not None:
            raise InvalidStateError("Another call to save_octomap() is in progress")

        self.path_octomap = os.path.abspath(path)
        self.exception_save_octomap = None
        self.octomap_save_is_completed = False

        # Subscribe to the planning scene monitoring topic to get the current octomap
        self.subscriber = rospy.Subscriber(
            "/move_group/monitored_planning_scene",
            rosmoveitmsgs.PlanningScene,
            self.handle_planning_scene,
        )

        # Wait until the octomap has been received and saved
        while not self.octomap_save_is_completed:
            await asyncio.sleep(1)

        # If we got an exception trying to save the octomap, re-raise it
        if self.exception_save_octomap is not None:
            raise self.exception_save_octomap
