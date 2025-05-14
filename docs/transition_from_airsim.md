# Transitioning from AirSim

## Converting an AirSim Unreal environment to AirSim vNext

To convert an existing AirSim Unreal environment to AirSim vNext, you can perform the following steps:

1. Delete the AirSim plugin folder in `<environment>/Plugins/AirSim`

2. Edit the environment project's `<environment>.uproject` file:

    In "Modules", delete:

        "AdditionalDependencies": [
            "AirSim"
        ]

    In "Plugins", delete:

        {
        "Name": "AirSim",
        "Enabled": true
        },

3. Add AirSim vNext to the environment following **[How to add the AirSim vNext Plugin to a custom Unreal environment](use_plugin.md#how-to-add-the-airsim-v-next-plugin-to-a-custom-unreal-environment)**, including the part about setting the `GameMode` to `AirSimVNextGameMode` since `AirSimGameMode` is no longer valid.

4. On first opening of the `<environment>.uproject` file with Unreal Engine 4.25, there may be some warnings about failing to load some AirSim content such as:

        Failed to load /AirSim/StarterContent/Materials/M_Tech_Hex_Tile_Pulse.M_Tech_Hex_Tile_Pulse Referenced by StaticMeshComponent0
        Failed to load /AirSim/VehicleAdv/SUV/AutomotiveMaterials/Materials/Glass/M_Glass.M_Glass Referenced by StaticMeshComponent0
        ...

    These warnings can be cleared by simply re-saving the environment's levels after AirSim has been deleted from it.

5. Check the environment's config ini files in `<environment>/Config` to remove any AirSim-specific paths or settings.

## Major changes

1. The Sky system has been upgraded to take advantage of latest UE Sky Atmosphere and Volumetric clouds.
    - The lighting itself is the same. However, there are new API to fine tune the lighting in the scene. Checkout SetSunLightIntesity(), SetCloudShadowStrength() in the API Overview.
    - If you currently use your own scene/environment and work with AirsimVnext by dropping it into the scene as a plugin. This might break existing TimeOfDay APIs. See the "Trasition your scene from SkySphere to SunSky" section below for dealing with this properly.


2. SetSunAngle(pitch, yaw) is discontinued and has been replaced by SetSunAngleFromDateTime(datetime, is_dst) which is more intuitive and supports more realistic sun positions depending on the location of the scene.

## Transition from SkySphere to SunSky 
SunSky is the new Unreal environment that provides a more natural and integrated lighting conditions. See https://docs.unrealengine.com/4.27/en-US/BuildingWorlds/LightingAndShadows/SunSky/ for more details. 

General guidelines to update your environment to SunSky:

1. First enable the "Sun Position Calculator" plugin using the Unreal Editor.
2. Delete any existing "DirectionalLight", "SkyLight", "SkyAtmosphere", "ExpoenntialHeightFog" actors from your scene.
3. Add the SunSky Actor using the "Place Actors" widget.
4. The initial scene might be very bright since the default lighting are configured to be very bright. To correct this, select the SunSky actor from the World Outliner and goto the inherited DirectionalLight actor inside the components of the SunSky.
5. There is a 'Intensity' field inside this directionalLight actor that can adjusted to your needs.
6. Make sure the ID Name of the SunSky actor  is "SunSky_2" (you can see this by hovering the mouse over the actor label in the World Outliner actor liss). The "SunSky_2" is the default unless there are other SunSky actors in your scene.
7. You can also add "Volumetric Clouds" actor to the scene to make the sky dome more realistic.

## Converting an AirSim OSS configuration to AirSim vNext

Refer to the [documentation on configuration](config.md) for information about Project AirSim's configuration files and what settings are available.

The following is a rough guide on how to handle each element from the AirSim OSS configuration in Project AirSim. Some items are introduced without highlighting their position in the Project AirSim configuration hierarchy. Refer to the [documentation](config.md) to know the exact positions of each configuration item.

**"SimMode":** 

Project Airsim assumes that a Multirotor vehicle is in use. There is no Car support. For ComputerVision, follow the [computer vision guide](tbd).

**"ViewMode", "CameraDirector":** 

In the Robot Config, the first listed camera with `"streaming-enabled: true"` is the viewpoint camera for that robot. Configure the `"origin"` and `"gimbal"` to obtain the desired view.

**"TimeOfDay":** 

Not configurable. Use the `world.SetTimeOfDay()` client API call.

**"OriginGeopoint":** 

Set `"home-geo-point"` in the scene configuration.

**"SubWindows":** 

No longer part of configuration. Use the `ImageDisplay` clientside API to work with subwindows.

**"Recording":** 

Server-side recording is not supported yet. All data should be recorded clientside.

**"ClockSpeed":**

To control the clock speed, select a `"steppable"` clock in the scene config. The ratio of `"step-ns"` to `"real-time-update-rate"` replaces `"ClockSpeed"`. For example, the following configuration is equivalent to '"ClockSpeed": 0.5`: 
```
"clock": {
    "type": "steppable",
    "step-ns": 1500000,
    "real-time-update-rate": 3000000,
    "pause-on-start": false
  }
```

**"Segmentation":**

Set `"segmentation"` in the Scene Config.

| Old Setting Name | New Setting Name |
| --------------------|----------------------|
|"InitMethod": "" or "InitMethod": "CommonObjectsRandomIDs" | "initialized-ids": true |
|"InitMethod": "None" | "initialized-ids": false |
|"OverrideExisting" | ignore-existing |
|"MeshNamingMethod" | use-owner-name |

**"Wind":**

Not configurable. Use the `world.SetWindVelocity()` client API call.

**"VehicleType":**

For multirotor vehicles, set up the `"controller"` in the vehicle config e.g.:
```
"controller": {
  "id": "Simple_Flight_Controller",
  "airframe-setup": "quadrotor-x",
  "type": "simple-flight-api",
  "simple-flight-api-settings": {...}
}
```
Set an appropriate name for the id, then specify `"simple-flight-api"`, `"px4-api"`, or `"manual-controller-api"` as the `"type"` and fill in the `"simple-flight-api-settings"`, `"px4-settings"`, or `"manual-controller-api-setttings"` as appropriate.
PhysXCar and ArduRover have no equivalents, as cars are unimplemented in Project AirSim.
For ComputerVision, follow the [computer vision guide](tbd).

**"PawnPath":**

Set the `"visual"` in the vehicle config. E.g:
```
"visual": {
  "geometry": {
    "type": "unreal_mesh",
    "name": "/Drone/Quadrotor1",
    "scale": "1.0 1.0 1.0"
  }
}
```

**"DefaultVehicleState":**

Not configurable. The drone always starts disarmed.

**"AutoCreate":**

Not needed.

**"RC":**

Not selected through config. See the [remote control guide](controllers/simple_flight#using-an-xbox-controller) for information on how to set up a remote controller with Project Airsim.

**"X", "Y", "Z", "Yaw", "Roll", "Pitch":**

In the scene settings, set `"origin"` to
```
{
    "xyz": "x, y, z",
    "rpy": "r, p, y"
}
```
Project AirSim uses radians for roll, pitch, and yaw. You will need to convert your values from degrees to radians.

**"IsFpvVehicle":**

Any vehicle with `"IsFpvVehicle": true` should be listed first in the config instead. In general, the vehicle camera will follow the first vehicle listed.

**"Px4 Settings":**
| Old Setting Name | New Setting Name |
| --------------------|----------------------|
| Lockstep | lock-step |
| ControlIp | control-ip-address |
| ControlPortLocal | control-port |
| ControlPortRemote | control-port |
| QgcHostIp | qgc-host-ip |
| QgcPort | qgc-port |
| TcpPort | tcp-port |
| SerialPort | serial-port |
| UseSerial | use-serial |
| UseTcp | use-tcp |
| Parameters | parameters |
| Other settings | Not present. |

**"Sensors":**

SensorTypes are now denoted with string values instead of enum values:
| Old Type | New Type |
| --------------------|----------------------|
| 0 | camera |
| 1 | barometer |
| 2 | imu |
| 3 | gps |
| 4 | magnetometer |
| 5 | (distance sensor isn't implemented yet) |
| 6 | lidar |

Make sure to add: `"parent-link": "Frame"` to all sensor configurations.

**"Cameras":**

Note that the new `"compress"` and `"pixels-as-float"` settings  in `"capture-settings"` are required, as is `"capture-enabled"`.

| Old Setting Name | New Setting Name |
| --------------------|----------------------|
| Enabled | enabled |
| CaptureSettings | capture-settings |
| ImageType | image-type (some image types are not yet implemented) |
| Width | width |
| Height | height |
| FOV_Degrees | fov-degrees |
| TargetGamma | target-gamma |
| AutoExposureSpeed | Not implemented. |
| AutoExposureBias | Not implemented. |
| AutoExposureMaxBrightness | Not implemented. |
| MotionBlurAmount | Not implemented. |
| ProjectionMode | Not implemented. |
| OrthoWidth | Not implemented. |
| RandContrib | rand-contrib |
| RandSpeed | rand-speed |
| RandSize | rand-size |
| RandDensity | rand-density |
| HorzWaveContrib | horz-wave-contrib |
| HorzWaveStrength | horz-wave-strength |
| HorzWaveVertSize | horz-wave-vert-size |
| HorzWaveScreenSize | horz-wave-screen-size |
| HorzNoiseLinesContrib | horz-noise-lines-contrib |
| HorzNoiseLinesDensityY | horz-noise-lines-density-y |
| HorzNoiseLinesDensityXY | horz-noise-lines-density-xy |
| HorzDistortionContrib | horz-distortion-contrib |
| HorzDistortionStrength | horz-distortion-strength |

**"Barometer":**
| Old Setting Name | New Setting Name |
| --------------------|----------------------|
| Enabled | enabled |
| PressureFactorSigma | pressure-factor-sigma |
| PressureFactorTau | pressure-factor-tau |
| UncorrelatedNoiseSigma | uncorrelated-noise-sigma |
| UpdateLatency | update-latency |
| UpdateFrequency | update-frequency |
| StartupDelay | startup-delay |

**"Imu":**
| Old Setting Name | New Setting Name |
| --------------------|----------------------|
| Enabled | enabled |
| AngularRandomWalk | gyroscope : { angle-random-walk } |
| GyroBiasStabilityTau | gyroscope: { tau } |
| GyroBiasStability | gyroscope: { bias-stability } |
| VelocityRandomWalk | accelerometer: { velocity-random-walk } |
| AccelBiasStabilityTau | accelerometer: { tau } |
| AccelBiasStability | accelerometer: { bias-stability } |

**"Gps":**
| Old Setting Name | New Setting Name |
| --------------------|----------------------|
| Enabled | enabled |
| EphTimeConstant | eph-time-constant |
| EpvTimeConstant | epv-time-constant |
| EphInitial | eph-initial |
| EpvInitial | epv-initial |
| EphFinal | eph-final |
| EpvFinal | epv-final |
| EphMin3d | eph-min_3d |
| EphMin2d | eph-min_2d |
| UpdateLatency | Not implemented. |
| UpdateFrequency | Not implemented. |
| StartupDelay | Not implemented. |

**"Magnetometer":**
| Old Setting Name | New Setting Name |
| --------------------|----------------------|
| Enabled | enabled |
| NoiseSigma | noise-sigma |
| ScaleFactor | scale-factor |
| NoiseBias | noise-bias |
| UpdateLatency | Not implemented. |
| UpdateFrequency | Not implemented. |
| StartupDelay | Not implemented. |

**"Lidar":**
| Old Setting Name | New Setting Name |
| --------------------|----------------------|
| Enabled | enabled |
| NumberOfChannels | number-of-channels |
| Range | range |
| PointsPerSecond | points-per-second |
| RotationsPerSecond | horizontal-rotation-frequency or vertical-rotation-frequency |
| HorizontalFOVStart | horizontal-fov-start-deg |
| HorizontalFOVEnd | horizontal-fov-end-deg |
| VerticalFOVUpper | vertical-fov-upper-deg |
| VerticalFOVLower | vertical-fov-lower-deg |
| X, Y, Z | origin: { "xyz": "x, y, z" } |
| Roll, Pitch, Yaw | origin: { "rpy": "r p y" } |
| DrawDebugPoints | draw-debug-points |
| DataFrame | Not implemented? |
| ExternalController | Not implemented? |

**"PhysicsEngineName":**

Set `"physics-type"` in the robot configuration.
| Old Engine Name | New Engine Name |
| --------------------|----------------------|
| FastPhysicsEngine | fast-physics |
| ExternalPhysicsEngine | non-physics |

**"LocalHostIp", "ApiServerPort":**

Not part of configuration. Use the `address`, `port_topics`, and `port_services` parameters when initializing the `AirSimVNextClient`.

## Converting an AirSim OSS client code to AirSim vNext

### General considerations

You cannot connect to Project AirSim with more than one client script at a time. If a second script connects, the first script will suffer a fatal error.

Unlike AirSim OSS, Project AirSim's client does not connect when it is instantiated. To connect to the server, use the `client.connect()` function of the client object.

While in AirSim OSS all method calls were made through the `client`, in Project AirSim there is a hierarchy of classes such as `World` for accesing methods that apply to the environment and `Drone` for methods that apply to drones.

Project Airsim has a logging framework which should be used in preference to `print()`. E.g. `airsim_vnext_log().info("")`.

Project Airsim's client does not automatically set up the environment from a `settings.json` file. One must initialize it manually using a valid scene configuration file.

```
world = World(client, "scene_config.jsonc")
```

Note that the file path is relative to `.\sim_config` where `.` is the location of the script. For information on writing scene configuration files see the [Scene Configuration Settings documentation](config_scene.md).

To take control of a drone, initialize it using the name it has in the config.

```       
drone = Drone(client, world, "Drone1")
```
You can then call the drone's method's to control it. If multiple drones exist in the scene, each one can be initialized and controlled in this fashion. This replaces the `vehicle_name` parameter from AirSim OSS's functions.

Project AirSim uses Radians as a standard for all interactions. Make sure that your code does not give commands in Degrees.

### Note on asynchronous methods

All asynchronous methods in Project AirSim use the [asyncio](https://docs.python.org/3/library/asyncio.html) library for Python, meaning they must be handled with asyncio's syntax. For example, instead of using

```
client.takeoffAsync().join()
```

as you would in AirSim OSS, use 

```
takeoff_task = await drone.takeoff_async()
await takeoff_task
```
We `await` takeoff_async, which retrieves an asychronous task. We then execute the task by `await`ing it.

## Reference table for api migration

### Methods

#### Connection and API Control

| AirSim OSS      | Project AirSim | Observations |
| ----------- | ----------- | ----------- |
| client.enableApiControl(bool)      | drone.enable_api_control(), drone.disable_api_control()       |        |
| client.isApiControlEnabled() | Not implemented.| |
| client.reset()      | World()       | To reset the scene, instantiate the same World again.       |
| client.confirmConnection() | Not implemented. | |
| client.ping() | Not implemented.| |
| client.simRunConsoleCommand() | Not implemented. | |

#### Drone Control

| AirSim OSS      | Project AirSim | Observations |
| ----------- | ----------- | ----------- |
| client.armDisarm(bool)      | drone.arm(), drone.disarm()       |        |
| client.takeoffAsync()      | drone.takeoff_async()       |       |
| client.landAsync()      | drone.land_async()       | Default timeout has changed from 60 to 3e38 seconds.       |
| client.hoverAsync()      | drone.hover_async()       |        |
| client.moveByVelocityAsync(vx, vy, vz, duration, drivetrain, yaw_mode)     | drone.move_by_velocity_async(self,v_north,v_east,v_down,duration,yaw_control_mode,yaw_is_rate,yaw)       | YawMode struct does not exist. Pass yaw and yaw_is_rate separately.        |
|client.moveByVelocityZAsync(vx, vy, vz, duration, drivetrain, yaw_mode)     | drone.move_by_velocity_z_async(self,v_north,v_east,v_down,duration,yaw_control_mode,yaw_is_rate,yaw)       | YawMode struct does not exist. Pass yaw and yaw_is_rate separately.        |
| client.moveToPositionAsync(self, x, y, z, velocity, timeout_sec, drivetrain, yaw_mode, lookahead, adaptive_lookahead) | drone.move_to_position_async(self, north, east, down, velocity, timeout_sec, yaw_control_mode, yaw_is_rate, yaw, lookahead, adaptive_lookahead) | YawMode struct does not exist. Pass yaw and yaw_is_rate separately. |
| client.moveOnPathAsync() | drone.move_on_path_async() | Path is now given as a 2D array. |
| client.moveToZAsync() | Not implemented.| |
| client.simSetVehiclePose() | drone.set_pose() | |
| client.simSetKinematics() | drone.get_ground_truth_kinematics() | |
| client.moveByManualAsync() | [See info on how to set up remote controller.](controllers/simple_flight#using-an-xbox-controller) | |
| client.moveByRc() | flight_rc.set() | Only needs to be called once. |


#### Images
| AirSim OSS      | Project AirSim | Observations |
| ----------- | ----------- | ----------- |
| client.simGetImage() | Not implemented. | Use drone.get_images with only one image_type_id to request a single image. |
| client.simGetImages(requests) | drone.get_images(camera_id, image_type_ids) | Instead of sending a set of ImageRequests, call requests a set of ImageTypes from a given camera. |
| client.simSetCameraPose() | drone.set_camera_pose() | |
| client.simGetCameraInfo() | Not implemented. | |
| client.simSetCameraFov() | Not implemented. | |
| client.simGetDistortionParams() | Not implemented. | |
| client.simSetDistortionParams() | Not implemented. | |
| client.simSetDetectionFilterRadius() | Not implemented. |
| client.simAddDetectionFilterMeshName() | Configurable. Set "object_id" in the "annotations" section of the [camera config.](sensors/camera_capture_settings) | |
| client.simGetDetections() | Use drone.get_images on a camera with "annotations" configured. The returned data will include an "annotations" section. | |
| client.simClearDetectionMeshNames() | Not implemented. | |

#### Sensors and Ground Truth
| AirSim OSS      | Project AirSim | Observations |
| ----------- | ----------- | ----------- |
| client.getMultirotorState()      | No equivalent.       | The API for estimated kinematics is not implemented yet. Use GetGroundTruthKinematics for now; it's the same unless a flight controller with estimation such as PX4 is in use. Collision info can be obtained by using self.client.subscribe() to subscribe to the collision sensor.       |
| client.getImuData()      | drone.get_imu_data(sensor_name)       |        |
| client.getBarometerData()      | drone.get_barometer_data(sensor_name)       |        |
| client.getMagnetometerData()      | drone.get_magnetometer_data(sensor_name)       |        |
| client.getGpsData()      | drone.get_gps_data(sensor_name)       |        |
| client.getLidarData() | Not implemented. | Use self.client.subscribe() to subscribe to the LiDAR sensor and handle LiDAR data in the callback. |
| client.simGetVehiclePose() | drone.get_ground_truth_pose() | |
| client.getHomeGeoPoint() | Not implemented. | |

#### Simulation 
| AirSim OSS      | Project AirSim | Observations |
| ----------- | ----------- | ----------- |
| client.simSetTraceLine() | world.set_trace_line() | |
| client.simSetWind(wind) | world.set_wind_velocity(n, e, d) | Wind isn't implemented in the physics yet, but the API exists. |
| client.simEnableWeather() | Run world.enable_weather_visual_effects(), world.disable_weather_visual_effects() | |
| client.simSetWeatherParameter() | world.set_weather_visual_effects_param() |  |
| client.simTestLineOfSightToPoint() | Not implemented. | |
| client.simTestLineOfSightBetweenPoints() | Not implemented. | |
| client.simGetWorldExtents() | Not implemented. | |
| client.simPause() | world.pause(), world.resume() | |
| client.simContinueForTime() | world.continue_for_sim_time() | Units have changed from seconds to nanoseconds. |
| client.simAddVehicle() | Not supported. | ProjectAirsim does not currently support adding new drones while the sim is running. All required drones should be added to the scene configuration. |
| client.simGetSegmentationObjectId() | world.get_segmentation_id_by_name() | |
| client.simSetSegmentationObjectId() | world.set_segmentation_id_by_name() | |
| client.simSetObjectMaterial() | Not implemented. | |
| client.simSetObjectMaterialFromTexture() | world.set_object_material() | Use the `file:///` protocol to access local textures. |
| client.simListAssets() | Not implemented. | |
| client.simSpawnObject() | world.spawn_object() | |
| client.simDestroyObject() | world.destroy_object() | |
| client.simListSceneObjects() | world.list_objects() | |
| client.simGetObjectPose() | world.get_object_pose() | |
| client.simSetObjectPose() | world.set_object_pose() | |
| client.simGetObjectScale() | world.get_object_scale() | |
| client.simSetObjectScale() | world.set_object_scale() | |
| client.simSwapTextures() | Not implemented. | |
| client.simSetLightIntensity() | Not implemented. | |
| client.simPlotArrows() | world.plot_debug_arrows() ||
| client.simPlotPoints() | world.plot_debug_points() ||
| client.simPlotLineStrip() | world.plot_debug_solid_line() ||
| client.simPlotLineList() | world.plot_debug_dashed_line() ||
| client.simPlotStrings() | world.plot_debug_strings() ||
| client.simPlotTransforms() | world.plot_debug_transforms() | |
| client.simPlotTransformsWithNames() | world.plot_debug_transforms_with_names() | |
| client.simFlushPersistentMarkers() | world.flush_persistent_markers() | |

#### Utilities
| AirSim OSS      | Project AirSim | Observations |
| ----------- | ----------- | ----------- |
| airsim.to_eularian_angles() | projectairsim.utils.quaternion_to_rpy() | |
| airsim.to_quaternion() | projectairsim.utils.rpy_to_quaternion() | |

### Types

#### Images

#### Image Types
| AirSim OSS      | Project AirSim | Observations |
| ----------- | ----------- | ----------- |
|Scene = 0 |    SCENE = 0 ||
|    DepthPlanar = 1 |    DEPTH_PLANAR = 1||
|    DepthPerspective = 2 | DEPTH_PERSPECTIVE = 2 ||
|    DepthVis = 3 |||
|    DisparityNormalized = 4 |||
|    Segmentation = 5 |SEGMENTATION = 3||
|    SurfaceNormals = 6 |||
|    Infrared = 7 |||
|    OpticalFlow = 8 |||
|    OpticalFlowVis = 9   |||

#### Classes 
| AirSim OSS      | Project AirSim | Observations |
| ----------- | ----------- | ----------- |
| ImuDataOSS      | ImuDataPA       | The orientation parameter for OSS is in Euler and for PA in Quaternions|

#### Migrated AirSim v1 Scripts examples

In the "../client/python/airsimv1_scripts_migrated" folder, you will find examples of AirSim scripts that have been translated to work with Project AirSim. These scripts demonstrate how to adapt existing functionalities from AirSim OSS to Project AirSim, serving as a practical reference for migrating your own scripts.

---

Copyright (C) Microsoft Corporation. 
Copyright (c) 2025 IAMAI Simulations.

MIT License. All rights reserved.
