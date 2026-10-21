"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Video side of the simulated gimbal payload:

- RtspStreamer: encodes GimbalCam frames to H.264 with ffmpeg and publishes them
  to an RTSP server (MediaMTX, started by run_cesium_sim.sh)
- CameraWindows: OpenCV windows for the chase and gimbal cameras, with keyboard
  and mouse-wheel control of the gimbal and zoom in the GimbalCam window
"""

import shutil
import subprocess
import threading
import time

import cv2
import numpy as np

from projectairsim.utils import projectairsim_log


def image_to_bgr(image_msg):
    data = np.frombuffer(image_msg["data"], dtype=np.uint8)
    return data.reshape(image_msg["height"], image_msg["width"], -1)[:, :, :3]


class RtspStreamer:
    """Publishes the latest frame to `url` at a steady frame rate.

    Frames from the sim arrive unevenly, so a writer thread repeats the newest
    one at `fps` to keep players (QGC, OpenCV, VLC) happy.
    """

    def __init__(self, url, resolution, fps=30):
        if shutil.which("ffmpeg") is None:
            raise RuntimeError("ffmpeg not found; install it or run with --no-rtsp")
        self.url = url
        self.width, self.height = resolution
        self.fps = fps
        self._frame = None
        self._lock = threading.Lock()
        self._stop = threading.Event()
        self._proc = self._start_ffmpeg()
        self._thread = threading.Thread(target=self._write_loop, daemon=True)
        self._thread.start()
        projectairsim_log().info(f"GimbalCam RTSP stream: {url}")

    def _start_ffmpeg(self):
        encoders = subprocess.run(
            ["ffmpeg", "-hide_banner", "-encoders"], capture_output=True, text=True
        ).stdout
        if "h264_nvenc" in encoders:
            codec = ["-c:v", "h264_nvenc", "-preset", "p1", "-tune", "ll", "-zerolatency", "1"]
        else:
            codec = ["-c:v", "libx264", "-preset", "ultrafast", "-tune", "zerolatency"]
        cmd = [
            "ffmpeg", "-hide_banner", "-loglevel", "error",
            "-f", "rawvideo", "-pix_fmt", "bgr24",
            "-s", f"{self.width}x{self.height}", "-framerate", str(self.fps), "-i", "-",
            *codec, "-g", str(self.fps), "-bf", "0", "-pix_fmt", "yuv420p",
            "-f", "rtsp", "-rtsp_transport", "tcp", self.url,
        ]
        return subprocess.Popen(cmd, stdin=subprocess.PIPE)

    def push(self, bgr):
        if bgr.shape[1] != self.width or bgr.shape[0] != self.height:
            bgr = cv2.resize(bgr, (self.width, self.height))
        with self._lock:
            self._frame = np.ascontiguousarray(bgr)

    def _write_loop(self):
        period = 1.0 / self.fps
        next_t = time.monotonic()
        while not self._stop.is_set():
            with self._lock:
                frame = self._frame
            if frame is not None:
                try:
                    self._proc.stdin.write(frame.tobytes())
                except (BrokenPipeError, ValueError):
                    projectairsim_log().warning("RTSP encoder stopped; is MediaMTX running?")
                    return
            next_t += period
            time.sleep(max(0.0, next_t - time.monotonic()))

    def close(self):
        self._stop.set()
        self._thread.join(timeout=2)
        try:
            self._proc.stdin.close()
        except Exception:
            pass
        self._proc.terminate()


class CameraWindows:
    """Chase and GimbalCam windows; all OpenCV calls stay on the caller's thread.

    GimbalCam window controls:
      W / S          tilt up / down          A / D   pan left / right
      mouse wheel    zoom in / out           + / -   zoom in / out
      C              centre (level, ahead)   X       look straight down
    """

    PAN_STEP_DEG = 3.0
    ZOOM_STEP_PERCENT = 5.0

    def __init__(self, on_gimbal, on_zoom, initial_pitch=-45.0):
        self.on_gimbal = on_gimbal  # called with (pitch_deg, yaw_deg)
        self.on_zoom = on_zoom  # called with a zoom delta in percent
        self.pitch = initial_pitch
        self.yaw = 0.0
        self.zoom_text = ""
        self._frames = {}
        self._lock = threading.Lock()
        cv2.namedWindow("ChaseCam", cv2.WINDOW_NORMAL)
        cv2.resizeWindow("ChaseCam", 640, 360)
        cv2.namedWindow("GimbalCam", cv2.WINDOW_NORMAL)
        cv2.resizeWindow("GimbalCam", 960, 540)
        cv2.setMouseCallback("GimbalCam", self._on_mouse)

    def receive(self, name, bgr):
        """Store a frame (any thread)."""
        with self._lock:
            self._frames[name] = bgr

    def _on_mouse(self, event, x, y, flags, param):
        if event == cv2.EVENT_MOUSEWHEEL:
            self.on_zoom(self.ZOOM_STEP_PERCENT if flags > 0 else -self.ZOOM_STEP_PERCENT)

    def _move(self, d_pitch, d_yaw):
        self.pitch = max(-90.0, min(30.0, self.pitch + d_pitch))
        self.yaw = (self.yaw + d_yaw + 180.0) % 360.0 - 180.0
        self.on_gimbal(self.pitch, self.yaw)

    def update(self):
        """Draw the latest frames and handle input; call every few ms."""
        with self._lock:
            frames = dict(self._frames)
        if "ChaseCam" in frames:
            cv2.imshow("ChaseCam", frames["ChaseCam"])
        if "GimbalCam" in frames:
            view = frames["GimbalCam"].copy()
            hud = (
                f"pitch {self.pitch:+.0f}  yaw {self.yaw:+.0f}  {self.zoom_text}   "
                "WASD gimbal, wheel or +/- zoom, C centre, X down"
            )
            cv2.putText(view, hud, (12, 28), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 0), 4)
            cv2.putText(view, hud, (12, 28), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 1)
            cv2.imshow("GimbalCam", view)

        key = cv2.waitKeyEx(1)
        if key == -1:
            return
        ch = chr(key & 0xFF).lower() if key & 0xFF < 128 else ""
        step = self.PAN_STEP_DEG
        if ch == "w":
            self._move(step, 0)
        elif ch == "s":
            self._move(-step, 0)
        elif ch == "a":
            self._move(0, -step)
        elif ch == "d":
            self._move(0, step)
        elif ch == "c":
            self._move(-self.pitch, -self.yaw)
        elif ch == "x":
            self._move(-90.0 - self.pitch, -self.yaw)
        elif ch in ("+", "="):
            self.on_zoom(self.ZOOM_STEP_PERCENT)
        elif ch in ("-", "_"):
            self.on_zoom(-self.ZOOM_STEP_PERCENT)

    def close(self):
        cv2.destroyAllWindows()
