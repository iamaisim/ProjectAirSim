# Lidar sensor

The lidar sensor is currently implemented as an ideal sensor that projects simulated laser beams over a cylindrical or circular field of view (FOV). The sensor's settings specify the scan pattern and rate.

## Lidar detections

Each beam intersecting with an object's mesh generates a lidar return with a 3D position relative to the sensor. The intensity of the return (which is affected many factors including atmospheric conditions, reflective properties of the object, and orientation of the object) is currently not simulated.

# Lidar sensor settings

## Sample config

```json
{
"sensors": [
  {
    "id": "lidar1",
    "type": "lidar",
    "enabled": true,
    "parent-link": "LidarMountPoint",
    "lidar-type": "generic-cylindrical",
    "number-of-channels": 16,
    "range" : 100,
    "points-per-second": 100000,
    "report-frequency": 0,
    "horizontal-rotation-frequency":10.0,
    "horizontal-fov-start-deg": 0.0,
    "horizontal-fov-end-deg": 360.0,
    "vertical-rotation-frequency": 0.0,
    "vertical-fov-lower-deg": -45.0,
    "vertical-fov-upper-deg": -15.0,
    "radial-scaling": 1.0,
    "disable-self-hits": false,
    "draw-debug-points": false,
    "origin": {
      "xyz": "0.6 0 0",
      "rpy-deg": "0 0 0"
    }
  }]
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `id` | string | Name of lidar sensor. |
| `type` | `lidar` | Sensor type specifying this as a lidar sensor. |
| `enabled` | bool | Whether sensor is enabled. |
| `parent-link` | string | Name of the link that the sensor is attached to. |
| `lidar-type` | string | Lidar sensor type. See below for details. |
| `number-of-channels` | integer | Number of simultaneous scan channels. |
| `range` | float | Maximum sensing range (meters.) |
| `points-per-second` | integer | Number of points returned per second (sensor total, not per channel.) |
| `report-frequency` | float | How often the sensor data is returned.  See below for details.  Defaults to zero meaning as frequently as possible. (&geq;0 Hertz, values <0.5 same as 0) |
| `horizontal-rotation-frequency` | float | Horizontal scan rate (revolutions per second.) |
| `horizontal-fov-start-deg` | float | Horizontal field of view starting angle. See below for details. (0.0 to 360.0 degrees) |
| `horizontal-fov-end-deg` | float | Horizontal field of view ending angle. See below for details. (0.0 to 360.0 degrees) |
| `vertical-rotation-frequency` | float | Vertical scan rate (revolutions per second.) |
| `vertical-fov-start-deg` | float | Vertical field of view starting angle. See below for details. (-90.0 to +90.0 degrees) |
| `vertical-fov-end-deg` | float | Vertical field of view ending angle. See below for details. (-90.0 to +90.0 degrees) |
| `radial-scaling` | float | Scan position scaling factor in radial direction. See below for details. |
| `disable-self-hits` | bool | If true, lidar traces will pass through the sensor's owner robot to prevent self hits. Defaults to false if not specified. |
| `draw-debug-points` | bool | If true, markers will be rendered at the beam hit points on the detected objects. This is not supported in Unreal's Shipping configuration. Defaults to false if not specified. |
| `origin`: `xyz` | string of 3 floats | Position offset "X Y Z" of the sensor relative to the parent link's origin in **meters**. Defaults to all zero if `origin` is not specified. |
| `origin`: `rpy` | string of 3 floats | Rotation offset "Roll Pitch Yaw" of the sensor relative to the parent link's origin in **radians**. Defaults to all zero if `origin` is not specified. |

## Point rate and report frequency

A sensor "report" consists of a set of 3D points detected by the sensor and their corresponding segmentation IDs.  The number of points in each report varies so that the average number of points reported per second matches the `points-per-second` setting.

With the default `report-frequency` setting of zero, the sensor publishes reports as frequently as possible which is usually once per simulation step.  A non-zero `report-frequency` of 0.5 or more sets the average report publication rate with the reports varying size to maintain the specified `points-per-second` overall.  Note that the simulation step size affects how well the sensor can meet the `report-frequency` setting from report to report but the average report rate will approach the setting value.

## FOV settings

The field of view (FOV) settings define the area where data is returned.

The `horizontal-fov-*-deg` settings specify the start and ending horizontal angles of the sweep area where 0&deg; is straight ahead, increasing right (clockwise from overhead) relative to the sensor.

The `vertical-fov-*-deg` settings specify the start and ending vertical angles of the sweep area where +90&deg; is straight up and -90&deg; is straight down relative to the sensor.

A particular lidar type may ignore one or both the horizontal and vertical FOV settings.  See the [lidar type](#lidar_type) section below for details.

## Lidar types

The `lidar-type` setting specifies sensor characteristics not specified by the other settings and may set default values for those other settings.  Most importantly, `lidar-type` specifies the laser scan pattern.

The possible `lidar-type` values and their descriptions are:
| Value | Description |
| ----- | ----------- |
| `generic_cylindrical` | The sensor scans a cylindrical volume. |
| `generic_rosette` | The sensor beams scan a conical volume with a hypotrochoid curve. |
| `livox_mid70` | The sensor simulates a Livox Mid-70 lidar sensor.
| `livox_avia` | The sensor simulates a Livox Avia lidar sensor.

### Generic cylindrical lidar type
The `generic_cylindrical` lidar type is similar to the Velodyne Puck lidar sensor (https://velodynelidar.com/products/puck).  It has multiple laser beams (specified by `number-of-channels`) which sweep out shallow horizontal cones.  The laser beams are equally spaced vertically to cover the vertical FOV.  The beams sweep horizontally 360&deg;, but data is returned only when the beams are within the horizontal FOV range.

The `vertical-rotation-frequency` and `radial-scaling` settings are ignored.

### Generic rosette lidar type
The `generic_rosette` lidar type has multiple laser beams (specified by `number-of-channels`) which sweep out a conical volume in a "rosette" pattern based on a hypotrochoid roulette curve: a "rolling circle" travels on the inside of the perimeter of a larger "fixed circle" while the "tracing point" attached at a fixed radius on the rolling circle traces out the curve.  For easier configuration, `generic_rosette` allows the angular rate of the "rolling circle" to be specified directly through `vertical-rotation-frequency` rather than through ratios of the "rolling" and "fixed" circles.  `vertical-rotation-frequency` can be negative which generates a epitrochoid curve instead (where the rolling circle travels on the outside of the fixed circle.)  `horizontal-rotation-frequency` specifies the angular rate at which the rolling circle travels around the fixed circle.

The positions in polar coordinates of the points sampled from the rosette pattern are mapped to the directions the lasers are pointed.  The polar angle is used directly as the horizontal angle of the laser.  Like `generic_cylindrical`, the beams sweep horizontally 360&deg; but data is only returned when the beams are within the horizontal FOV range specified by the `horizontal-fov-*-deg` settings.

The polar radius is mapped to the vertical FOV.  The radius of the tracing point is automatically set so that the polar radius ranges from 0.0 to 1.0.  A polar radius of 0.0 corresponds to `vertical-fov-lower-deg` while a polar radius of 1.0 corresponds to `vertical-fov-upper-deg`.

The `radial-scaling` setting fine-tunes the shape of the scanning pattern curve by scaling the tracing point's angular offset from the rolling circle center's angular position (as seen from fixed circle's center,) causing the tracing point to travel in an ovoidal path around the rolling circle.  A value of "1.0" (the default) leaves the curve unmodified.  A value greater the 1.0 "stretches" the position of the tracing point away from the center of the rolling circle along the radial path around the center of the fixed circle.  A value less than 1.0 compresses the tracing point's position towards the center of the rolling circle.  A hypotrochoid curve typically has "petals" and values greater than 1.0 widens those petals while values less than 1.0 narrows them.

### Livox Mid-70 lidar type
The `livox_mid70` lidar type simulates the Livox Mid-70 lidar sensor (https://www.livoxtech.com/mid-70).  It is based on the `generic_rosette` lidar type with appropriate default values for the Mid-70 thus none of the lidar characteristic settings such as `number-of-channels` or `horizontal-rotation-frequency` need to be specified.  These settings can still be specified, for instance, to tweak the sensor to better match an actual unit or simulate a closely related but slightly different model.

A standard `livox_mid70` config would be similar to this:
```json
{
"sensors": [
  {
    "id": "lidar1",
    "type": "lidar",
    "enabled": true,
    "parent-link": "LidarMountPoint",
    "lidar-type": "livox_mid70",
    "draw-debug-points": false,
    "origin": {
      "xyz": "0.6 0 0",
      "rpy-deg": "0 0 0"
    }
  }
}
```

---

Copyright (C) Microsoft Corporation.  All rights reserved.
