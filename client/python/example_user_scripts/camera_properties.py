"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Payload camera model from a camera_properties.json file: resolution, sensor
size and a zoom lens. The field of view follows from the sensor width and the
current focal length, so zooming from the shortest to the longest focal length
behaves like the real optical zoom.
"""

import json
import math

from projectairsim.utils import projectairsim_log

RESOLUTIONS = {
    "360p": (640, 360),
    "480p": (854, 480),
    "720p": (1280, 720),
    "1080p": (1920, 1080),
    "2k": (2560, 1440),
    "1440p": (2560, 1440),
    "4k": (3840, 2160),
}


class CameraProperties:
    def __init__(self, path=None):
        props = {}
        if path:
            with open(path) as f:
                props = json.load(f)
        self.props = props
        self.name = props.get("camera_id", "GimbalCam")

        res = str(props.get("resolution", "720p")).lower()
        if res in RESOLUTIONS:
            self.resolution = RESOLUTIONS[res]
        elif "x" in res:
            w, h = res.split("x")
            self.resolution = (int(w), int(h))
        else:
            raise ValueError(f"unknown resolution '{res}'")

        sensor = props.get("sensor", {})
        lens = props.get("lens", {})
        zoom = props.get("zoom", {})
        self.sensor_width_mm = float(sensor.get("sensor_width_mm", 6.19))
        self.sensor_height_mm = float(sensor.get("sensor_height_mm", 3.48))
        self.focal_min_mm = float(lens.get("focal_length_mm", 4.3))
        if "focal_length_max_mm" in zoom:
            self.focal_max_mm = float(zoom["focal_length_max_mm"])
        else:
            self.focal_max_mm = self.focal_min_mm * float(zoom.get("optical_zoom_max", 10.0))
        self.focus_distance_m = lens.get("focus_distance_m")
        self.chromatic_aberration = props.get("ue_overrides", {}).get("chromatic_aberration")

    @property
    def max_zoom(self):
        return self.focal_max_mm / self.focal_min_mm

    def focal_length(self, zoom_percent):
        """Focal length (mm) for a zoom position: 0 % = widest, 100 % = longest."""
        return self.focal_min_mm + (self.focal_max_mm - self.focal_min_mm) * zoom_percent / 100.0

    def hfov_deg(self, focal_mm=None):
        f = self.focal_min_mm if focal_mm is None else focal_mm
        return math.degrees(2.0 * math.atan(self.sensor_width_mm / (2.0 * f)))

    def apply_to_camera_config(self, camera):
        """Write these properties into a robot config camera sensor dict."""
        scene_capture = next(
            s for s in camera["capture-settings"] if s.get("image-type", 0) == 0
        )
        scene_capture["width"], scene_capture["height"] = self.resolution
        scene_capture["fov-degrees"] = self.hfov_deg()
        if self.focus_distance_m is not None:
            scene_capture["depth-of-field-focal-region"] = float(self.focus_distance_m)
        if self.chromatic_aberration is not None:
            scene_capture["chromatic-aberration-intensity"] = float(self.chromatic_aberration)

        overrides = self.props.get("ue_overrides", {})
        if overrides.get("ue_noise_enabled"):
            camera["noise-settings"] = [
                {
                    "enabled": True,
                    "image-type": 0,
                    "rand-contrib": overrides.get("ue_noise_rand_contrib", 0.2),
                    "rand-speed": overrides.get("ue_noise_rand_speed", 100000.0),
                    "rand-size": overrides.get("ue_noise_rand_size", 500.0),
                    "rand-density": overrides.get("ue_noise_rand_density", 2.0),
                }
            ]

        unsupported = []
        lens = self.props.get("lens", {})
        if lens.get("distortion_enabled"):
            unsupported.append("lens distortion")
        if self.props.get("platform", {}).get("vibration_enabled"):
            unsupported.append("platform vibration")
        if self.props.get("sensor", {}).get("noise_enabled"):
            unsupported.append("sensor noise (use ue_noise_* instead)")
        if unsupported:
            projectairsim_log().warning(
                "camera_properties: not simulated yet: " + ", ".join(unsupported)
            )

    def describe(self):
        return (
            f"{self.name}: {self.resolution[0]}x{self.resolution[1]}, "
            f"{self.focal_min_mm:g}-{self.focal_max_mm:g} mm "
            f"({self.max_zoom:.1f}x optical), HFOV {self.hfov_deg():.1f}-"
            f"{self.hfov_deg(self.focal_max_mm):.1f} deg"
        )
