# Scene Configuration Settings

The scene config JSONC file specifies the settings for the scene's actors, clock, and other scene-level settings.

The scene is loaded from the JSONC file that is specified when the `World` object is initialized in the client script. The client can also reload a specified scene config file through an API call while the simulation is running.

## Main scene configuration

`scene_basic_drone.jsonc`
``` json
{
  "id": "SceneBasicDrone",
  "actors": [...],
  "environment-actors": [...],
  "environment-objects": [...],
  "clock": {...},
  "home-geo-point": {...},
  "segmentation": {...},
  "scene-type": "UnrealNative"
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `id` | string | Name identifier for the scene (simulation world). |
| `actors` | array of **[Actor settings](#actor-settings)** | Actor settings for each actor to spawn in the scene. |
| `environment actors` | array of **[Environment actor settings](#environment-actor-settings)** | Environment actor settings for each actor to spawn in the scene. |
| `environment objects` | array of **[Environment object settings](#environment-object-settings)** | Environment object settings for each object to spawn in the scene. |
| `clock` | **[Clock settings](#clock-settings)** | Simulation clock settings. |
| `home-geo-point` | **[Home GeoPoint settings](#home-geopoint-settings)** | GeoPoint coordinates (latitude, longitude, altitude) corresponding to the scene origin. |
| `segmentation` | **[Segmentation ID settings](#segmentation-settings)** | Segmentation ID settings for automatically assigning segmentation IDs to objects in the scene. |
| `scene-type` | string | (Optional) Use 'CustomGIS' if you are running a geospecific GIS scenario. Defaults to 'UnrealNative' for non-GIS scenes. |
| `tiles-dir` | string | (Optional) Local path containing the GIS tiles used when `scene-type` is set as `CustomGIS`. |
| `tiles-altitude-offset` | float | (Optional) Our tiles have altitude values of WGS84 ellipsoidal height. Sometimes other altitude models are preferred, so you can use this setting to offset the tiles by the provided value in meters. Most GIS software use a geoid relative altitude, so you can query the geoid height at some coord and enter it here: https://geographiclib.sourceforge.io/cgi-bin/GeoidEval |
| `tiles-lod-max` | integer | (Optional) Maximum level of detail for tiles. Default is 19. Maximum allowed is 23. |
| `tiles-lod-min` | integer | (Optional) Minimum level of detail for tiles. Default is 13. Minimum allowed is 13. |

## Actor settings

``` json
"actors": [
  {
    "type": "robot",
    "name": "Drone1",
    "origin": {
      "xyz": "109.05 -7.5 -19.42", // or "geo-point": "33.047329 -97.292618 4.5"
      "rpy-deg": "0 0 0"
    },
    "robot-config": "robot_quadrotor_fastphysics.jsonc",
    "start-landed": true
  },
  ...
]
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `type` | `robot` | Type of actor, currently only `robot` is supported. |
| `name` | string | Name identifier for the actor. |
| `origin`: `xyz` | string of 3 floats | Spawning origin position "X Y Z" coordinates with units of SI **meters** in the **NED** frame (up = negative z). **Note:** "0 0 0" is at the global UE origin, not relative to `PlayerStart`. |
| `origin`: `geo-point` | string of 3 floats | Alternative to `xyz`. Spawning origin position at "lat lon alt" coordinates. |
| `origin`: `rpy` | string of 3 floats | Spawning origin orientation "Roll Pitch Yaw" angles with units of SI **radians** with right-hand rotation around the NED frame X Y Z axes. |
| `robot-config` | string | Filename for JSONC config file for this actor's **[Robot Configuration Settings](config_robot.md)**. |
| `start-landed` | bool | (Optional) Start from a manually-set landed state at the spawned origin (without needing to drop onto a mesh with collision). For FastPhysics, a landed state means that the position is locked until a net vertical force up exceeds the weight of the body (to counter gravity forces) such as from rotors during takeoff, or there is already an upward velocity. Defaults to false if omitted. |

## Environment actor settings

``` json
"environment-actors": [
  {
    "type": "env_car",
    "name": "car1",
    "origin": {
      "xyz": "3.0 0.0 -5.0",
      "rpy-deg": "0 0 0"
    },
    "env-actor-config": "env_actor_car.jsonc"
  },
  ...
]
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `type` | `env_actor`, `env_car`, `env_human` | Types of environment actors: `env_actor` is for generic environment actors, `env_car` for cars, `env_human` for humans. |
| `name` | string | Name identifier for the actor. |
| `origin`: `xyz` | string of 3 floats | Spawning origin position "X Y Z" coordinates with units of SI **meters** in the **NED** frame (up = negative z). **Note:** "0 0 0" is at the global UE origin, not relative to `PlayerStart`. |
| `origin`: `geo-point` | string of 3 floats | Alternative to `xyz`. Spawning origin position at "lat lon alt" coordinates. |
| `origin`: `rpy` | string of 3 floats | Spawning origin orientation "Roll Pitch Yaw" angles with units of SI **radians** with right-hand rotation around the NED frame X Y Z axes. |
| `env-actor-config` | string | Filename for JSONC config file for this actor's **[Environment Actor Configuration Settings](config_env_actors.md)**. |

## Environment object settings

``` json
"environment-objects": [
  {
    "type": "env_particle_effect",
    "name": "fire1",
    "origin": {
      "xyz": "3.0 0.0 -5.0",
      "rpy-deg": "0 0 0"
    },
    "env-object-config": "env_particle_effect_nfire_02.jsonc"
  },
  ...
]
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `type` | `env_particle_effect` | Types of environment objects: `env_particle_effect` like fire and smoke assets. |
| `name` | string | Name identifier for the environment object. |
| `origin`: `xyz` | string of 3 floats | Spawning origin position "X Y Z" coordinates with units of SI **meters** in the **NED** frame (up = negative z). **Note:** "0 0 0" is at the global UE origin, not relative to `PlayerStart`. |
| `origin`: `geo-point` | string of 3 floats | Alternative to `xyz`. Spawning origin position at "lat lon alt" coordinates. |
| `origin`: `rpy` | string of 3 floats | Spawning origin orientation "Roll Pitch Yaw" angles with units of SI **radians** with right-hand rotation around the NED frame X Y Z axes. |
| `env-object-config` | string | Filename for JSONC config file for this object's **[Environment Object Configuration Settings](config_env_objects.md)**. |


## Clock settings

``` json
"clock": {
  "type": "steppable",
  "step-ns": 3000000,
  "real-time-update-rate": 3000000,
  "pause-on-start": false
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `type` | `steppable`, `real-time` | Type of simulation clock. Defaults to `steppable` if omitted. |
| `step-ns` | integer | Step size in **nanosec** for `steppable` clock. Defaults to 20000000 (20 ms) if omitted. |
| `real-time-update-rate` | integer | Real-time execution period in **nanosec** between each simulation loop and clock update for all clock types. Defaults to 3000000 (3 ms) if omitted. |
| `pause-on-start` | bool | Flag to start with simulation paused for `steppable` clock. Defaults to false if omitted. |

There are 2 supported simulation clock types:

1. Steppable clock
2. Real-time clock

See below for example of how to configure each type of simulation clock.

### Steppable clock

``` json
"clock": {
  "type": "steppable",
  "step-ns": 3000000,
  "real-time-update-rate": 3000000,
  "pause-on-start": false
}
```

Steppable clock is a **fixed-step clock** with sim time step = `step-ns` nanosecs that is stepped every execution period set by `real-time-update-rate` real-time nanosecs.

For example, with `step-ns` = 3 ms and `real-time-update-rate` = 3 ms, the sim clock will advance by exactly 3 ms every execution period, which is also every ~3 ms so the simulation will appear to move at ~1x real-time but with deterministic fixed time steps.

With `step-ns` = 6 ms and `real-time-update-rate` = 3 ms, the sim clock will advance by exactly 6 ms every execution period, which is still 3 ms so the simulation will appear to move at ~2x faster than real-time.

The `pause-on-start` option allows the simulation to start paused to allow full-manual control of the time step advancement through the clock APIs.

### Real-time clock

``` json
"clock": {
  "type": "real-time",
  "real-time-update-rate": 3000000
}
```

Real-time clock is a **variable-step clock** with sim time step = real-time step between each execution period set by `real-time-update-rate` real-time nanosecs. Since the real-time step between each execution period varies slightly by the system clock, the sim time step will also have variations.

**Note:** Since `real-time` clock follows real-time advancement, it can not be paused and is not deterministic or repeatable. In order to do the simulation at a scaled multiple of real-time, use a steppable clock with the desired ratio set between `step-ns` and `real-time-update-rate` as described in the [Steppable clock](#steppable-clock) section.

## Home GeoPoint settings

``` json
"home-geo-point": {
  "latitude": 47.641468,
  "longitude": -122.140165,
  "altitude": 122.0
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `latitude` | float | Latitude coordinate of the scene origin. |
| `longitude` | float | Longitude coordinate of the scene origin. |
| `altitude` | float | Altitude value of the scene origin in SI **meters** (positive values are higher, not NED). |

The home GeoPoint is used to correlate the simulation scene origin to a position on Earth to use as a reference point for GPS sensor values and for physics calculations such as air temperature, pressure, etc.

The default value is set for the Microsoft Building 99 parking garage in Redmond, WA, but a new desired location's latitude/longitude values can be easily looked up by right-clicking anywhere on **[Bing maps](https://www.bing.com/maps/)**.

## Segmentation settings

``` json
"segmentation": {
  "initialize-ids": true,
  "ignore-existing": false,
  "use-owner-name": true
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `initialize-ids` | bool | Flag to initialize the segmentation ID for the scene objects at simulation start. Defaults to `false` if omitted to reduce sim startup time. |
| `ignore-existing` | bool | Flag to ignore any existing segmentation IDs already set and initialize them for everything. Defaults to `false` if omitted. |
| `use-owner-name` | bool | Flag to use each mesh component's owner's name (usually the actor name) instead of the mesh component's name for hashing the segmentation ID. Defaults to `true` if omitted. |

The segmentation IDs can be set on scene objects of class `UStaticMeshComponent`, `USkinnedMeshComponent`, and `ALandscapeProxy`, including any derived classes (instanced static meshes, etc). The IDs are set using Unreal's CustomDepth Stencil parameter so that the object can be rendered with all pixels set to the RGB color that is mapped to the **[segmentation pallet](sensors/segmentation.md#segmentation-pallet)** based on its segmentation ID.

The segmentation ID (CustomDepth Stencil value) can be **[set manually on objects using the Unreal Editor](sensors/segmentation.md#assigning-segmentation-ids-in-unreal-editor)**, or using the client's world API `SetSegmentationIDByName`. To get the existing segmentation ID of an object, you can use the client's world API `GetSegmentationIDByName`.

To have the segmentation IDs automatically assigned at the start of simulation, set `initialize-ids` to `true`. If `ignore-existing` is set `false`, then any object that has already been manually assigned an ID (the CustomDepth Stencil rendering is already enabled on the object) will keep its original value. Otherwise, all objects will get reassigned segmentation IDs that are calculated by a hash method based on the sum of the chars in the name (lowercased and ignoring number/symbol chars) and then modulo 256 to get an ID in the allowed range of 0~255. If `use-owner-name` is `true`, the name used for this hash will be the mesh object's owner's name (ex. the StaticMeshActor's name that contains the StaticMeshComponent that will be assigned the ID). Using the owner's name may provide better grouping of the IDs and uses the names that are readable in the Editor's object tree.

To find out what object names were assigned what segmentation ID, check the simulation server log's output for `InitSegmentationID` lines, such as:

```
[2020.08.13-15.20.45:719] [InitSegmentationID] seg_id:  95, seg_name: 'cylinder8'
[2020.08.13-15.20.45:720] [InitSegmentationID] seg_id:  95, seg_name: 'cylinder_2'
[2020.08.13-15.20.45:720] [InitSegmentationID] seg_id: 148, seg_name: 'ground'
[2020.08.13-15.20.45:720] [InitSegmentationID] seg_id:   0, seg_name: 'templatecube_48'
[2020.08.13-15.20.45:720] [InitSegmentationID] seg_id: 241, seg_name: 'templatecube_rounded_1'
...
```

---

Copyright (C) Microsoft Corporation. 
Copyright (c) 2025 IAMAI Simulations.

MIT License. All rights reserved.
