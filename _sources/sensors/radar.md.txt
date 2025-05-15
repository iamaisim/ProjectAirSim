# Radar sensor overview

The radar sensor is currently implemented as an ideal ground-truth based radar that does line-traces for each beam over a set rectangular field of view (FOV). One full frame of FOV beam line-traces is done at every sim time interval specified in the sensor's settings.

## Radar detections

For any beam line-trace intersections with an object's mesh, a radar detection is generated with the value of the range, azimuth, elevation, relative velocity along the beam direction, and radar cross-section (RCS) in square meters. The range, azimuth, and elevation are determined by the position of the intersection point relative to the sensor. The relative velocity is determined by the ground-truth kinematics of the object relative to the ground-truth kinematics of what the sensor is attached to. The RCS value is determined by the size of a bounding sphere of the detected object's mesh to roughly approximate the overall scale of the object, and currently doesn't account for any material or reflectivity effects.

## Radar tracks

The radar sensor also does track processing at a separate sim time interval from the detections, generally at a lower frequency. At each track update, the accumulated detections since the last track update are processed and each detection is associated to an object based on the ground-truth name of the object. Since the detections are based on ground-truth, every detection for the same object will have the same values and only one detection is needed to update each object's track. For each detected object, a track ID number is assigned and the detection's ground-truth values are used to output the track data with the value of the ID, azimuth, elevation, range, relative position, relative velocity, relative acceleration, and RCS. Since the values are based on ground-truth, no EKF or state estimation is done and the values are just passed through directly. Before a track is created for a newly detected object, 3 of the last 5 track updates have to have associated detections to the object. An existing track that hasn't had an associated detection in the last 3 updates will then be deleted.

# Radar sensor settings

## Sample config

```json
{
"sensors": [
  {
    "id": "radar1",
    "type": "radar",
    "enabled": true,
    "parent-link": "RadarMountPoint",
    "fov": {...},
    "range-max": 500.0,
    "range-resolution": 1.0,
    "velocity-max": 100.0,
    "velocity-resolution": 1.0,
    "detection-interval": 0.02,
    "track-interval": 0.2,
    "rcs-adjust-factor": 0.1,
    "draw-debug-points": false,
    "origin": {
      "xyz": "0.6 0 0",
      "rpy-deg": "0 0 0"
    },
    "masks": [...]
  }
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `id` | string | Name of radar sensor. |
| `type` | `radar` | Sensor type specifying this as a radar sensor. |
| `enabled` | bool | Whether sensor is enabled or not. |
| `parent-link` | string | Name of the link that the sensor is attached to. |
| `fov` | [FOV settings](#fov-settings) | Field of view settings. See below for details. |
| `range-max` | float (>0) | Max range for the radar beams in meters. |
| `range-resolution` | float (>0) | Resolution of the measured range values in meters. |
| `velocity-max` | float | Max amplitude of measured relative velocity in m/s. Negative values are if the object is moving toward the sensor, and positive values are moving away. |
| `velocity-resolution` | float (>0) | Resolution of the measured velocity values in m/s. |
| `detection-interval` | float | Time interval between each set of detections in seconds. Each set is a full FOV frame of beams. |
| `track-interval` | float | Time interval between each track update in seconds. Each track update is based on the accumulated detections since the last update. |
| `rcs-adjust-factor` | float (>0) | Global multiplier to adjust every measured RCS value. |
| `draw-debug-points` | bool | If true, green dots will be rendered at the beam hit points on the detected objects. This is not supported in Unreal's Shipping configuration. |
| `origin`: `xyz` | string of 3 floats | Position offset "X Y Z" of the sensor relative to the parent link's origin in **meter** units. Defaults to all zero if no `origin` is specified. |
| `origin`: `rpy` | string of 3 floats | Rotation offset "Roll Pitch Yaw" of the sensor relative to the parent link's origin in **radian** units. Defaults to all zero if no `origin` is specified. |
| `masks` | [Mask settings](#mask-settings) | Array of mask settings. See below for details. |

## FOV settings

Field of view settings define the area that the beams will be sweeped across as a rectangular frame.

```json
{
"fov": {
    "azimuth-max": 0.785,
    "azimuth-min": -0.785,
    "elevation-max": 0.524,
    "elevation-min": -0.524,
    "azimuth-resolution": 0.0175,
    "elevation-resolution": 0.0175
  },
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `azimuth-max` | float | Max azimuth angle in radians. Positive azimuth is in the rightward rotation direction from the centerline normal to the sensor's face. |
| `azimuth-min` | float | Min azimuth angle in radians. Negative azimuth is in the leftward rotation direction from the centerline normal to the sensor's face. |
| `elevation-max` | float | Max elevation angle in radians. Positive elevation is in the upward rotation direction from the centerline normal to the sensor's face. |
| `elevation-min` | float | Min elevation angle in radians. Negative elevation is in the downward rotation direction from the centerline normal to the sensor's face. |
| `azimuth-resolution` | float | Angular resolution between each beam in the azimuth sweep direction in radians. |
| `elevation-resolution` | float | Angular resolution between each beam in the elevation sweep direction in radians. |

## Mask settings

Mask settings define sections of the FOV that should NOT be included in the radar's output detections. Any detection that fits within all of the mask's windows for FOV area, range, velocity, and RCS will be filtered out. Any number of masks can be applied to a single radar sensor.

```json
{
"masks": [
    {
      "azimuth-min": -0.8,
      "azimuth-max": 0.8,
      "elevation-min": -0.8,
      "elevation-max": -0.2,
      "range-min": 10.0,
      "range-max": 200.0,
      "velocity-min": -15.0,
      "velocity-max": 15.0,
      "rcs-sqm-min": 0.01,
      "rcs-sqm-max": 100.0
    }
  ]
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `azimuth-min` | float | Min azimuth angle of the masked window in radians. |
| `azimuth-max` | float | Max azimuth angle of the masked window in radians. |
| `elevation-min` | float | Min elevation angle of the masked window in radians. |
| `elevation-max` | float | Max elevation angle of the masked window in radians. |
| `range-min` | float | Min range of the masked window in meters. |
| `range-max` | float | Max range of the masked window in meters. |
| `velocity-min` | float | Min relative velocity of the masked window in m/s. |
| `velocity-max` | float | Max relative velocity of the masked window in m/s. |
| `rcs-sqm-min` | float | Min RCS of the masked window in m^2. |
| `rcs-sqm-max` | float | Max RCS of the masked window in m^2. |

---

Copyright (C) Microsoft Corporation.  All rights reserved.
