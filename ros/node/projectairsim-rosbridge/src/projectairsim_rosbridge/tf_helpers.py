"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
ROS bridge for Project AirSim: Transform broadcasting
"""
import logging
import threading
import traceback
import time
import rclpy
import tf2_ros
#from tf2_ros.transform_listener import TransformListener
#from tf2_ros.buffer import Buffer

import geometry_msgs.msg as rosgeommsg

from projectairsim.utils import projectairsim_log
from .node import ROSNode
from . import utils


class TFBroadcaster:
    """
    This class broadcasts transform frames using the ROS tf facility.

    Once a frame is added via add_frame(), it is automatically broadcast
    until removed via remove_frame().  Once added, the transform of a frame
    is updated via set_frame().

    Broadcasting is initially stopped and can be started and stopped via
    start() and stop().
    """

    # -------------------------------------------------------------------------
    # TFBroadcaster Types
    # -------------------------------------------------------------------------
    class Frame:
        """
        Contains the info for an individual transform frame.
        """

        # Frame update handling
        UPDATE_AUTO = 0  # Automatically update the timestamp and broadcast
        UPDATE_NEVER = 1  # Only broadcast if updated since last pass
        UPDATE_STATIC = 2  # Only broadcast if updated since last pass and use
        # static broadcaster which latches the transform
        # (good for frame transforms that never change)

        def __init__(
            self,
            frame_id: str,
            frame_id_parent: str,
            transform: rosgeommsg.Transform = None,
            timestamp=None,
            update_mode=UPDATE_AUTO,
        ):
            """
            Constructor.

            The frame timestamp is set at creation and by set_transform().
            The ROS tf facility requires a frame to have a new timestamp when
            it's broadcasted so, normally, the timestamp is automatically
            updated by the broadcast thread when the frame hasn't been updated
            since the last pass.  The update_mode parameter can direct
            the broadcaster to handle this situation differently:

                UPDATE_AUTO - Automatically update the timestamp and broadcast
                UPDATE_NEVER - Don't broadcast unless updated
                UPDATE_STATIC - Don't broadcast unless updated and use a
                    static broadcaster

            Arguments:
                frame_id - Name of this transform frame
                frame_id_parent - Name of the parent transform frame
                transform - Initial transform of the frame
                update_mode - How to handle this frame when not updated
                    since the last broadcast pass
            """
            self.is_updated = (
                False
            )  # If True, transform has been updated since this flags was reset last
            self.lock = threading.Lock()  # Access guard to self.transform_stamped
            self.transform_stamped = (
                rosgeommsg.TransformStamped()
            )  # Transform to broadcast
            self.update_mode = (
                update_mode
            )  # How handle the frame is not updated since the last broadcast pass

            # Initialize frame
            self.transform_stamped.header.frame_id = frame_id_parent
            self.transform_stamped.child_frame_id = frame_id

            self.set_transform(
                transform
                if transform is not None
                else rosgeommsg.Transform(
                    translation=rosgeommsg.Vector3(x=0.0, y=0.0, z=0.0),
                    rotation=rosgeommsg.Quaternion(x=0.0, y=0.0, z=0.0, w=1.0),
                ),
                timestamp,
            )

        def set_transform(self, transform: rosgeommsg.Transform, timestamp):
            """
            Set the transform of the frame.

            This method is thread-safe.

            Arguments:
                transform - The new transform of the frame
                timestamp - Timestamp of the update
            """
            with self.lock:
                self.transform_stamped.header.stamp = timestamp
                self.transform_stamped.transform = transform
                self.is_updated = True

    # -------------------------------------------------------------------------
    # TFBroadcaster Methods
    # -------------------------------------------------------------------------
    def __init__(self, ros_node: ROSNode, logger: logging = None, projectairsim_world = None):
        """
        Constructor.
        """
        self.frames = {}  # Frames to be broadcast, guarded by self.lock_frames
        self.broadcast_thread = threading.Thread(
            target=self._broadcast_thread_fn
        )  # Broadcasting thread
        self.event_notify = (
            threading.Event()
        )  # Event to indicate self.event_quit or self.event_run has been set or a new frame has been added
        self.event_quit = (
            threading.Event()
        )  # Event to request broadcasting thread to quit
        self.event_run = (
            threading.Event()
        )  # Event to request broadcasting thread to broadcast
        self.lock_frames = threading.Lock()  # Access guard to self.frames
        self.logger = logger if not None else projectairsim_log()  # Logging object
        self.rate = ros_node.create_rate(20)  # Transform broadcast rate
        self.ros_node = ros_node
        self.static_transform_broadcaster = (
            ros_node.create_static_transform_broadcaster()
        )  # Static transform broadcaster
        self.transform_broadcaster = (
            ros_node.create_transform_broadcaster()
        )  # Transform broadcaster
        self.projectairsim_world = projectairsim_world

        # Start broadcasting thread (but broadcasting is initially stopped and must be started with start())
        self.broadcast_thread.start()

    def __del__(self):
        """
        Destructor.
        """
        self.clear()

    def set_world(self, world):
        self.projectairsim_world = world

    def add_frame(
        self,
        frame_id: str,
        frame_id_parent: str,
        transform: rosgeommsg.Transform = None,
    ):
        """
        Adds a new frame.  This call is ignored if the frame already exists.
        To update a frame's transform, use set_frame().

        Arguments:
            frame_id - Name of the frame
            frame_id - Name of the frame's parent
            transform - The initial transform of the new frame (or the identity transform if None)
        """
        with self.lock_frames:
            if frame_id not in self.frames:
                self.frames[frame_id] = self.Frame(
                    frame_id,
                    frame_id_parent,
                    transform,
                    self.ros_node.get_time_to_msg(self.ros_node.get_time_now()),
                )
                self.logger.info(f'Adding transform frame "{frame_id}"')
        self.event_notify.set()  # Let broadcast thread know the frame list is not empty

    def clear(self):
        """
        Stop broadcasting and free resources.
        """
        self.event_quit.set()
        self.event_notify.set()

    def remove_frame(self, frame_id: str):
        """
        Remove the specified frame.


        Arguments:
            frame_id - The name of the frame to remove
        """
        with self.lock_frames:
            if frame_id in self.frames:
                del self.frames[frame_id]
                self.logger.info(f'Removing transform frame "{frame_id}"')

    def set_frame(self, frame_id: str, transform: rosgeommsg.Transform, timevalue=None):
        """
        Set the transform of a frame.  The frame must have been added via
        add_frame().

        Arguments:
            frame_id - Name of the frame
            transform - The frame's new transform
            timevalue - The time of the update or the current ROS time if unspecified
        """
        if timevalue is None:
            timevalue = self.ros_node.get_time_now()
        with self.lock_frames:
            if frame_id in self.frames:
                self.frames[frame_id].set_transform(
                    transform, self.ros_node.get_time_to_msg(timevalue)
                )

    def start(self):
        """
        Start broadcasting frames, if any.  If there are no frames yet,
        broadcasting will start automatically when a frame is added.
        """
        self.logger.info("Transform broadcast enabled")
        self.event_run.set()
        self.event_notify.set()

    def stop(self):
        """
        Stop broadcasting frames.  Existing frames are retained.  Frames can
        be added, removed, and updated but the changes aren't broadcast until
        start() is called.
        """
        self.logger.info("Transform broadcast disabled")
        self.event_run.clear()

    def _broadcast_thread_fn(self):
        """
        Transform broadcast thread.  We periodically broadcast frames here.
        """
        try:
            rosinterruptexception = self.ros_node.ROSInterruptException

            while True:
                # Wait until either we should run or quit
                while not self.event_quit.is_set():
                    with self.lock_frames:
                        if self.event_run.is_set() and self.frames:
                            # We're set to run and there's at least one frame
                            break

                    # Block until something changes
                    self.event_notify.wait()
                    self.event_notify.clear()

                if self.event_quit.is_set():
                    break  # We should quit

                # Broadcast each frame
                if self.projectairsim_world is None:
                    timestamp_cur = self.ros_node.get_time_now_msg()
                else:
                    try:
                        now = self.projectairsim_world.get_sim_time()
                        timestamp_cur = utils.to_ros_timestamp(now)
                    except:
                        timestamp_cur = self.ros_node.get_time_now_msg()
                with self.lock_frames:
                    for pair in self.frames.items():
                        frame = pair[1]
                        with frame.lock:
                            if frame.is_updated:
                                if frame.update_mode == self.Frame.UPDATE_STATIC:
                                    self.static_transform_broadcaster.sendTransform(
                                        frame.transform_stamped
                                    )
                                else:
                                    self.transform_broadcaster.sendTransform(
                                        frame.transform_stamped
                                    )
                            elif frame.update_mode == self.Frame.UPDATE_AUTO:
                                frame.transform_stamped.header.stamp = timestamp_cur
                                self.transform_broadcaster.sendTransform(
                                    frame.transform_stamped
                                )

                            frame.is_updated = False

                # Wait until it's time to broadcast again
                try:
                    self.rate.sleep()
                except rosinterruptexception:
                    # Thread was interrupted, exit
                    self.event_quit.set()
        except:
            self.logger.error(
                f"TF broadcast loop Caught exception: {traceback.format_exc()}"
            )

class TFListener:
    """
    This class listens to transform frames using the ROS tf facility.

    It receives transform messages and can query the latest available transform.
    The transforms are updated periodically based on the ROS time.

    Listens to transforms and processes them as needed.
    """

    def __init__(self, ros_node: ROSNode, logger: logging = None):
        """
        Constructor.
        
        Arguments:
        ros_node -- The ROS node instance.
        logger -- Optional logging object.
        """
        self.ros_node = ros_node
        self.tf_buffer = ros_node.create_transform_buffer()
        self.tf_listener = ros_node.create_transform_listener(self.tf_buffer)
        self.logger = logger if logger is not None else projectairsim_log()

    def __del__(self):
        """
        Destructor.
        """
        self.stop()
        self.thread.join()

    def get_latest_transform(self, target_frame: str, source_frame: str, curr_time: rclpy.time.Time):
        """
        Get the latest available transform for the specified target and source frames.

        Arguments:
        target_frame -- The target frame of the transform.
        source_frame -- The source frame of the transform.

        Returns:
        The latest transform or None if not available.
        """
        #with self.lock:
        return self.tf_buffer.lookup_transform(target_frame, source_frame, curr_time)
            

    def stop(self):
        """
        Stop listening and free resources.
        """
        self.stop_event.set()