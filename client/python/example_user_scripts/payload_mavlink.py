"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

MAVLink side of the simulated gimbal payload, on PX4's onboard payload link
(PX4 sends to UDP 14030):

- gimbal control through PX4's gimbal manager (GIMBAL_MANAGER_SET_ATTITUDE), the
  same way a companion-computer tracker would steer a real gimbal
- a MAVLink camera component (MAV_COMP_ID_CAMERA) so QGroundControl finds the
  RTSP video stream on its own and its zoom controls drive the camera's FOV

PX4 forwards messages between this link and the ground-station link, so QGC
talks to the camera component through PX4.
"""

import math
import os
import queue
import threading
import time

os.environ["MAVLINK20"] = "1"  # gimbal v2 and camera messages need MAVLink 2
from pymavlink import mavutil
from pymavlink.quaternion import QuaternionBase

from projectairsim.utils import projectairsim_log

PAYLOAD_LINK = "udpin:0.0.0.0:14030"
TRACKER_SYSID = 1  # same vehicle
TRACKER_COMPID = mavutil.mavlink.MAV_COMP_ID_ONBOARD_COMPUTER
CAMERA_COMPID = mavutil.mavlink.MAV_COMP_ID_CAMERA

# PX4's gimbal manager runs in the autopilot; it ignores setpoints sent to the
# broadcast component, so always address it explicitly
PX4_SYSID = 1
PX4_COMPID = mavutil.mavlink.MAV_COMP_ID_AUTOPILOT1
GIMBAL_DEVICE_COMPID = mavutil.mavlink.MAV_COMP_ID_GIMBAL

GIMBAL_FLAGS = (
    mavutil.mavlink.GIMBAL_MANAGER_FLAGS_ROLL_LOCK
    | mavutil.mavlink.GIMBAL_MANAGER_FLAGS_PITCH_LOCK
)

M = mavutil.mavlink


class PayloadLink:
    """One UDP link to PX4 shared by the gimbal controller and camera component.

    Zoom requests from QGC are queued in `zoom_requests` as ("range", percent)
    or ("continuous", -1/0/1) for the main thread to apply to the sim, since the
    sim client must not be used from this link's reader thread.
    """

    def __init__(self, video_uri, camera):
        self.video_uri = video_uri
        self.camera = camera  # CameraProperties
        self.zoom_percent = 0.0
        self.zoom_requests = queue.Queue()
        self.capture_requests = queue.Queue()
        self.image_count = 0

        self._mav = mavutil.mavlink_connection(
            PAYLOAD_LINK, source_system=TRACKER_SYSID, source_component=TRACKER_COMPID
        )
        self._send_lock = threading.Lock()
        self._boot = time.monotonic()
        self._stop = threading.Event()
        self._px4_seen = threading.Event()
        self._threads = [
            threading.Thread(target=self._read_loop, daemon=True),
            threading.Thread(target=self._heartbeat_loop, daemon=True),
        ]
        for t in self._threads:
            t.start()

    def close(self):
        self._stop.set()
        for t in self._threads:
            t.join(timeout=2)
        self._mav.close()

    def wait_for_px4(self, timeout=30):
        """Wait for the next PX4 heartbeat (e.g. from a freshly started PX4)."""
        self._px4_seen.clear()
        if not self._px4_seen.wait(timeout):
            raise RuntimeError("No heartbeat from PX4 on the payload link (UDP 14030)")
        projectairsim_log().info("Payload link up")

    # ------------------------------------------------------------------
    # Gimbal

    def take_gimbal_control(self):
        """Make this link the gimbal manager's primary controller.

        Whoever configures last is in control, so QGC can take it back at any
        time and this can take it again.
        """
        self._send_as(
            TRACKER_COMPID,
            lambda mav: mav.command_long_send(
                PX4_SYSID,
                PX4_COMPID,
                M.MAV_CMD_DO_GIMBAL_MANAGER_CONFIGURE,
                0,
                TRACKER_SYSID,  # primary control sysid
                TRACKER_COMPID,  # primary control compid
                -1,  # secondary control sysid: unchanged
                -1,  # secondary control compid: unchanged
                0,
                0,
                0,  # gimbal device id: all
            ),
        )

    def set_gimbal_attitude(self, pitch_deg, yaw_deg, pitch_rate_dps=math.nan,
                            yaw_rate_dps=math.nan):
        """Stream a gimbal setpoint. Angles are relative to the vehicle heading."""
        q = QuaternionBase([0.0, math.radians(pitch_deg), math.radians(yaw_deg)]).q
        self._send_as(
            TRACKER_COMPID,
            lambda mav: mav.gimbal_manager_set_attitude_send(
                PX4_SYSID,
                PX4_COMPID,
                GIMBAL_FLAGS,
                0,  # gimbal device id: all
                [float(v) for v in q],
                math.nan,
                math.radians(pitch_rate_dps),
                math.radians(yaw_rate_dps),
            ),
        )

    # ------------------------------------------------------------------
    # Camera component

    def publish_zoom(self, zoom_percent):
        """Report the current zoom to QGC (0 = widest, 100 = most zoomed)."""
        self.zoom_percent = zoom_percent
        self._send_camera_settings()

    def report_image_captured(self, path):
        self.image_count += 1
        self._send_as(
            CAMERA_COMPID,
            lambda mav: mav.camera_image_captured_send(
                self._ms(), int(time.time() * 1e6), 0, 0, 0, 0, 0,
                [1.0, 0.0, 0.0, 0.0], self.image_count - 1, 1,
                path.encode()[:205],
            ),
        )

    def _ms(self):
        return int((time.monotonic() - self._boot) * 1000)

    def _send_as(self, compid, send):
        with self._send_lock:
            self._mav.mav.srcComponent = compid
            send(self._mav.mav)

    def _heartbeat_loop(self):
        while not self._stop.wait(1.0):
            for compid, mav_type in (
                (TRACKER_COMPID, M.MAV_TYPE_ONBOARD_CONTROLLER),
                (CAMERA_COMPID, M.MAV_TYPE_CAMERA),
            ):
                self._send_as(
                    compid,
                    lambda mav, t=mav_type: mav.heartbeat_send(
                        t, M.MAV_AUTOPILOT_INVALID, 0, 0, M.MAV_STATE_ACTIVE
                    ),
                )

    def _read_loop(self):
        while not self._stop.is_set():
            msg = self._mav.recv_match(blocking=True, timeout=0.5)
            if msg is None:
                continue
            msg_type = msg.get_type()
            if msg_type == "HEARTBEAT":
                if msg.get_srcSystem() == PX4_SYSID and msg.get_srcComponent() == PX4_COMPID:
                    self._px4_seen.set()
            elif msg_type == "COMMAND_LONG" and msg.target_component == CAMERA_COMPID:
                self._handle_camera_command(msg)

    def _ack(self, msg, result):
        self._send_as(
            CAMERA_COMPID,
            lambda mav: mav.command_ack_send(
                msg.command, result, 0, 0, msg.get_srcSystem(), msg.get_srcComponent()
            ),
        )

    def _handle_camera_command(self, msg):
        cmd = msg.command
        accepted, unsupported = M.MAV_RESULT_ACCEPTED, M.MAV_RESULT_UNSUPPORTED

        if cmd == M.MAV_CMD_REQUEST_MESSAGE:
            requested = int(msg.param1)
            senders = {
                M.MAVLINK_MSG_ID_CAMERA_INFORMATION: self._send_camera_information,
                M.MAVLINK_MSG_ID_VIDEO_STREAM_INFORMATION: self._send_video_stream_information,
                M.MAVLINK_MSG_ID_CAMERA_SETTINGS: self._send_camera_settings,
                M.MAVLINK_MSG_ID_CAMERA_CAPTURE_STATUS: self._send_capture_status,
            }
            sender = senders.get(requested)
            self._ack(msg, accepted if sender else unsupported)
            if sender:
                sender()
        elif cmd == M.MAV_CMD_REQUEST_CAMERA_INFORMATION:
            self._ack(msg, accepted)
            self._send_camera_information()
        elif cmd == M.MAV_CMD_REQUEST_VIDEO_STREAM_INFORMATION:
            self._ack(msg, accepted)
            self._send_video_stream_information()
        elif cmd == M.MAV_CMD_REQUEST_CAMERA_SETTINGS:
            self._ack(msg, accepted)
            self._send_camera_settings()
        elif cmd == M.MAV_CMD_REQUEST_CAMERA_CAPTURE_STATUS:
            self._ack(msg, accepted)
            self._send_capture_status()
        elif cmd == M.MAV_CMD_SET_CAMERA_ZOOM:
            zoom_type, value = int(msg.param1), msg.param2
            if zoom_type == M.ZOOM_TYPE_RANGE:
                self.zoom_requests.put(("range", max(0.0, min(100.0, value))))
            elif zoom_type == M.ZOOM_TYPE_CONTINUOUS:
                self.zoom_requests.put(("continuous", int(math.copysign(1, value)) if value else 0))
            elif zoom_type == M.ZOOM_TYPE_STEP:
                self.zoom_requests.put(("range", max(0.0, min(100.0, self.zoom_percent + 10 * value))))
            else:
                self._ack(msg, unsupported)
                return
            self._ack(msg, accepted)
        elif cmd == M.MAV_CMD_IMAGE_START_CAPTURE:
            self.capture_requests.put(True)
            self._ack(msg, accepted)
        elif cmd == M.MAV_CMD_SET_CAMERA_MODE:
            self._ack(msg, accepted)
        else:
            self._ack(msg, unsupported)

    def _send_camera_information(self):
        flags = (
            M.CAMERA_CAP_FLAGS_CAPTURE_IMAGE
            | M.CAMERA_CAP_FLAGS_HAS_VIDEO_STREAM
            | M.CAMERA_CAP_FLAGS_HAS_BASIC_ZOOM
        )
        self._send_as(
            CAMERA_COMPID,
            lambda mav: mav.camera_information_send(
                self._ms(),
                list(b"ProjectAirSim".ljust(32, b"\0")),
                list(self.camera.name.encode()[:32].ljust(32, b"\0")),
                0,
                self.camera.focal_min_mm,
                self.camera.sensor_width_mm,
                self.camera.sensor_height_mm,
                self.camera.resolution[0],
                self.camera.resolution[1],
                0,
                flags,
                0,
                b"",
                GIMBAL_DEVICE_COMPID,
            ),
        )

    def _send_video_stream_information(self):
        self._send_as(
            CAMERA_COMPID,
            lambda mav: mav.video_stream_information_send(
                1,
                1,
                M.VIDEO_STREAM_TYPE_RTSP,
                M.VIDEO_STREAM_STATUS_FLAGS_RUNNING,
                30.0,
                self.camera.resolution[0],
                self.camera.resolution[1],
                4_000_000,
                0,
                int(self.camera.hfov_deg()),
                b"GimbalCam",
                self.video_uri.encode(),
                M.VIDEO_STREAM_ENCODING_H264,
            ),
        )

    def _send_camera_settings(self):
        self._send_as(
            CAMERA_COMPID,
            lambda mav: mav.camera_settings_send(
                self._ms(), M.CAMERA_MODE_VIDEO, self.zoom_percent, math.nan
            ),
        )

    def _send_capture_status(self):
        self._send_as(
            CAMERA_COMPID,
            lambda mav: mav.camera_capture_status_send(
                self._ms(), 0, 0, 0.0, 0, math.nan, self.image_count
            ),
        )
