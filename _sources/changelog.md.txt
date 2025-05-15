# Changelog

## [2023.9.15] - 2023-09-15
- Add ability to spawn non-cooperative environment actors near a robot's path using `world.get_random_free_position_near_point()` and `world.get_random_free_position_near_path()` APIs demonstrated by example `non_cooperative_actor_demo.py` script
- Add asynchronous GIS tile loading for Unity
- Fix glTF runtime asset spawning to correct scale/rotations and account for the gfTF scene hierarchy transforms
- Enable support for Python 3.10 and update `msgpack` to 1.0.5
- Add `open3d` as an explicit client library dependency and pin to version 0.16 for correct LidarDisplay view control
- Add VS2022 builds for C++ client library
- Improvements to path planning logic in `motion_planner.py` example and data collection module
- Added preliminary wheeled rovor support and `hello_rover.py` initial example
- Added simulink_controller_quadrotor.py example that uses a Simulink controller model to fly a quadrotor with FastPhysics
- Fixes for configuring Unity camera sensor pose using degree units and changing pose at runtime
- Renaming for validation components for better clarity
- Improvements to the data collection module for running on Azure
- Added `world.get_surface_elevation_at_point()` API to query the height of the ground at a given point

## [2023.8.2] - 2023-08-02
- Improvements to blackshark example
- Handle missing tiles
- Added blackshark renderer type
- Enable combined Simulink physics and controller in a single model
- Add Simulink S-Function Interface for Control Models
- Clear stale topics on client disconnect
- Fix client exception from receiving stale sim subscription not in topic dictionary
- Improve lidar benchmarker test timing accuracy and increase throughput

## [2023.6.30] - 2023-06-30
- Fix crash from using `real-time` clock with lidar sensor
- Add `world.spawn_object_at_geo()` API support to Unity sim plugin
- Add initial `vr-mode` flag to scene config to disable sim streaming camera viewport manipulation to avoid conflicting with a VR headset's viewport control

## [2023.6.16] - 2023-06-16
- Preserve original scan pattern ordering for generic cylindrical lidar sensor point cloud arrays and default to reporting no-hit returns to get the full scan pattern in the set order
- Fixes for transform tree and Euler-Quaternion conversion singularity handling
- Fix UE version to 5.1 for SyntheticEnv and GeoSpecificEnv uproject files
- Add support for spawning skeletal mesh assets
- Start adding sim server infrastructure for enabling an interactive viewport camera
- Reduce idle CPU usage for sim server on Linux
- Data collection module cleanup for coordinate handling and geodetic conversion

## [2023.6.2] - 2023-06-02
- Improvements to test bench and validation bench features, including import/exporting test benches and saving test bench results
- Add lidar sensor config flags `report-point-cloud` and `report-azimuth-elevation-range` to enable reporting hits as xyz (meters) and/or azimuth/elevation/range (radians/meters)
- Add DALL-E example integration with Autonomy Blocks package for augmenting camera sensor images
- Add `world.get_weather_visual_effects_param()` and `world.get_time_of_day()` APIs
- Fix for LidarDisplay custom view option
- Enable forward compatibility with UE 5.2
- Upgrade to UE 5.1 (not backwards compatible with 5.0)
- Fix packaging of Unity plugin/projects
- Improved validation feature example scripts
- Add test suite feature for automating scenario generation
- Add option to configure reliability of topics to simulate dropped messages
- Data generation framework bug fixes
- Add fault injection feature for simulating camera and rotor faults

## [2023.3.27] - 2023-03-27
- Add GPS, IMU, Barometer & Magnetometer sensor support to ROS Bridge
- Add various data augmentations to the data collection module
- Improvements to Xbox RC controller example `xbox_rc.py` scripts
- Add `show-debug-plots` config value to camera sensor capture settings to allow visualizing debug drawing in the camera images
- Add `actors_to_ignore` parameter to `world.create_voxel_grid()` API
- Add validation module examples with XML output capability
- Fix yaw control for body frame move APIs with SimpleFlight controller
- Add `ardupilot_quadrotor.py` and `ardupilot_hexarotor.py` example scripts for using ArduPilot controller
- Add `datacollection` Python client module for data generation and example scripts in a `datacollection/` subfolder
- Update packaging of UnrealEnv_GeoSpecificEnv and UnrealEnv_SyntheticEnv binaries

## [2023.2.24] - 2023-02-24
- Add initial ArduPilot controller support through UDP port connections to ArduPilot SITL and example script ardupilot_quadrotor.py.
- Add `livox_avia` lidar sensor type and update lidar_livox.py example script to demonstrate either the `avia` or `mid70` scan patterns
- Improve performance of world.create_voxel_grid() API
- Add initial validation suite client scripts with example battery_simple_with_validation.py and validation_module_example.py
- Fix UE version check in server log to be 5.0

## [2023.1.27] - 2023-01-27
- Add optional parameters to ImageDisplay add_image() for resizing the image visualization pop-up windows
- Build Simulink S-function MEX file for Linux and fix example model time step settings
- Integrate robot link/joint transform tree hierarchy with FastPhysics and MatlabPhysics models
- Add JSON config validation schema for environment actor parts of the configs
- Fix log warnings about "No message published for topic..." on initial subscription
- Fix crashes from invalid config IDs and make sim server able to reload a new scene after a failed partial-loading

## [2023.1.13] - 2023-01-13
- Add yaw rate parameter to `drone.move_by_heading_async()` and `drone.rotate_to_yaw_async()` APIs to be able to limit the turning rate
- Add `drone.set_external_force()` API to apply an external force vector directly on the drone
- Fix for camera capture interval not throttling rendering load
- Add ability to spawn objects in the scene by scene config using a `spawn-objects` section
- Add laser index array to lidar point cloud data
- Rename world spawn object APIs for better clarity: `spawn_object_at_lat_lon()` -> `spawn_object_at_geo()`, `spawn_runtime_object()` -> `spawn_object_from_file()`, `spawn_runtime_object_at_lat_lon` -> `spawn_object_from_file_at_geo`
- Add `drone.get_ground_truth_geo_location()` and `drone.get_estimated_geo_location()` APIs
- Add a configurabe distance filter for camera object detection bounding box annotations

## [2023.1.4] - 2023-01-04
- Add option in lidar sensor config to report no-hit returns using `report-no-return-points` and `no-return-point-value`, enabled by default for Livox Mid-70
- Add option in config JSON rotation fields to use `rpy-deg` to set the values in degrees instead of `rpy` radians
- Add drone APIs to get multirotor controller state info: get_estimated_kinematics(), get_landed_state(), can_arm(), get_ready_state()
- Add drone.camera_draw_frustum() API to visualize a camera sensor's view frustum with debug lines
- Add world.set_object_texture_from_file() API to set an object texture from a client-local file
- Add drone.is_api_control_enabled() API
- Add a water plane Unreal asset in Drone content plugin
- ROS bridge fixes and improvements
- Example script API renaming fixes
- Add drone.move_by_velocity_body_frame_async() and drone.move_by_velocity_body_frame_z_async() APIs
- Add velocity parameter to drone.go_home_async() API
- Add geo coordinate versions of drone APIs
- Add drone.get_airspeed_data() request-response API
- Publish topic for rotor actuator speed, angle, torque, and thrust data

## [2022.11.23] - 2022-11-23
- Switch versioning to date-style format
- Fix a bug that was preventing GIS tiles from rendering at certain geo positions/altitudes
- Add a hello_gis.py example that uses a built-in mini GIS tileset in the GeoSpecificEnv binary
- Add world.set_object_material() and world.set_object_material_from_texture() APIs to change an objects material/texture at runtime
- Add Simulink physics S-function block mask parameter to allow an AirSim time step that's different from the Simulink model time step
- Document all user API method docstrings including description, arguments with types, and return value with type
- Update autonomy_utils client library for latest autonomy block inference model server and workflow
- Allow modifying SimpleFlight controller parameters through config JSON
- Allow subscribing multiple times with separate callbacks to a single topic
- Add drone.camera_look_at_object() API to automatically keep a camera pointing at a target object in the scene
- Improvements to world object's scene config handling
- Fix camera projection matrix to regenerate when FOV changes to fix zoomed bounding boxes

## [0.1.23] - 2022-10-31
- Change Project AirSim Python Client API names to use snake case
- Fix UE5 breaking ID Name lookups
- Added jsbsim trajectory generator with example script
- Link wind visuals to physics and add rain/snow/leaves to depth and segmentation views
- Add support for ROS2
- Add distance sensor

## [0.1.22] - 2022-10-10
- Update to UE 5.0.3 and Ubuntu 20.04
- Renaming to Project AirSim
- Rename and clean up example user scripts
- Update quadtiltrotor example script to be in DFW GIS scene

## [0.1.21] - 2022-09-26
- Port example scripts from AirSim OSS
- Enable link rotation for environment actors
- Scene configuration can now be loaded and modified before being sent to simulation
- Add DepthVis, SurfaceNormals, and DisparityNormalized camera image types
- Fix SpawnRuntimeObject to allow actors to be initially created without a mesh

## [0.1.20] - 2022-09-13
- Enhance SpawnObject client API to support Unreal Engine blueprints
- Add API call to get segmentation ID mappings

## [0.1.19] - 2022-09-03
- Clear Debug Text on FlushPersistentMarkers and World()
- Add Gimbal Actuator for mavlink gimbal v2 in PX4 controller
- Expose camera info topics properly in python client
- Enable setting trajectories (NED + geographic) for Env Actors via API

## [0.1.18] - 2022-08-15
- Improve scene loading performance in Unity
- Fix pitch-roll-yaw order in Python scripts using quaternion_to_eulerian_angles()
- Add task name to async tasks in Python client scripts to help debug failed tasks
- Fix spelling of "Eulerian" in code
- Implement weather APIs in Unity
- Add GetPose() to Python client API Drone class

## [0.1.17] - 2022-08-03
- Add support for additional tensor input formats
- Add manual controller to allow setting control signals directly by API
- Improve runtime asset material rendering
- Add AirTaxi asset to Drone plugin
- Initial hello AVX demo client-side connector script
- Fix for unpack_image handling of depth images
- Fix mesh scaling specified in robot config not being applied
- Connect all rotor/control surface input signals in Simulink model
- Fix calculation of rotor rotation angle used for setting the rotation rate
- Fix depth camera image to be uint16 mm
- Standardize Simulink model along with user script
- Add annotations to image topic
- Added debug plot functions to client API
- Make 3D BBox available as service method API

## [0.1.16] - 2022-07-10
- Fix some weather effects not clearing properly
- Fix various ImageDisplay and LidarDisplay issues
- Enhance Unity support
- Update WorldSimApi to support scene reloading
- Update docs for autonomy blocks
- Reduce EAP drop sizes

## [0.1.15] - 2022-06-20
- Fix camera FOV bug.
- Add debugging plot functions and vehicle tracing support to client API
- Package ONNX DLLs with Unity Plugins folder
- Add "start-landed" config option
- Fix simulated Livox Mid-70 sensor not having the same orientation as a physical unit
- Add GIS tile support to Unity implementation
- Improve Tiltrotor physics and compatibility with PX4

## [0.1.14] - 2022-06-06
- Add TensorRT as optional execution provider for Onnx
- Improved Simpleflight fixed-wing forward speed control
- Add SetMissionMode() and RequestControl() client API methods
- Port Simpleflight improvements from AirSim v1

## [0.1.13] - 2022-05-23
- Fix ROS Bridge not using the correct transform frame for LIDAR data
- Fix autonomy module packaging
- Add BlueprintCallable static methods for HUD Blueprint to get sim topic data
- Add Simpleflight tiltrotor controller
- Add S-function compatible with both VTOL and quadrotor models
- Add 3D projected vertices to annotations
- Add Onnx post processing inside Camera sensor
- Clip projected 3D bounding boxes to image coords
- Improve Unity integration

## [0.1.12] - 2022-05-09
- Add support for client authorization
- Add "disable-self-hits" optional config flag for LIDAR sensors
- Addition of a standard VTOL model in MathWorks Simulink
- Initial demo of Unity integration
- Battery sensor code and document improvements
- Add access to published topic data for Unreal actors

## [0.1.11] - 2022-05-03
- Autonomy Gym doc updates and DAA Bonsai inkling sample updates
- Fixes for using PX4 v1.12.3 and WSL2

## [0.1.10] - 2022-04-12
- Create a standard VTOL airframe to fly with PX4 and FastPhysics
- Enhanced Lidar with material-independent intensity
- Fix mismatch in client BatteryState enum
- Add SetGroundTruthKinematics API
- Updates to trainable sim packaging
- Add focal length / zoom to camera sensor
- Miscellaneous bug fixes

## [0.1.9] - 2022-03-27
- Fix parameter count error when calling MoveByHeading client method
- Fix log calls using the wrong format for narrow strings
- Compute intensity for GPU lidar through a shader
- Add planner script
- Add example script for drone move API's

## [0.1.8] - 2022-03-14
- Improve GIS processing
- Add GIS tile altitude offset config
- Enable Voxel Grid Generation
- Add World scene logging for Drone construction
- Add energy consumption model to battery simulation.
- Fix TimeOfDay settings reset at the beginning of a scene
- Enhance AI block
- Update autonomy docs and wheel installation step

## [0.1.7] - 2022-01-31
- Integrate Autonomy Block #1 client package and samples
- Add variable LIDAR reporting frequency
- Fix errors with PX4 when the commands are sent too soon

## [0.1.6] - 2022-01-24
- Add SwitchStreamingView API for cycling streaming camera view and update cycling logic
- Fix rotor torque calculation to use filtered control signal instead of control signal input
- Switch from Sky Sphere blueprint to Sun Sky/Volumetric Cloud system (SetSunAngle API replaced by SetSunPositionFromDateTime)
- Fix missing cv2 import for using compressed camera images
- Update to UE 4.27.2 to include latest PixelStreaming signalling web server
- Update PX4 documentation for using WSL2, multiple vehicles, and corrected port info
- Enable build scripts to use Visual Studio 2022
- Enable PX4 HITL connect to be canceled
- Add more documentation and example scripts for Livox Mid-70 LIDAR sensor
- Fix crashes from invalid parent/child link references in robot configurations
- Add documentation about installing system prerequisites, such as Vulkan libraries for Linux

## [0.1.5] - 2021-12-07
- Add Unreal Engine prerequisites installer into binaries
- Add example user script that pulls live weather data and adds live weather effects
- Add support for streaming camera images with h.264/WebRTC
- Add support for simulating Livox Mid-70 LIDAR sensor
- Reorganize ROS Bridge folder
- Add two ROS examples using Project AirSim with the ROS MoveIt! package

## [0.1.4] - 2021-11-22
- Add Drone.GetAnnotations() to retrieve annotations with objects of interest in camera sensor view
- Add image_utils.draw_bbox3d() for clients to draw 3D bounding box annotations
- Add range validation on drone home geographical location
- Fix spurious warning when using Unreal Engine version 4.27
- Fix objects added during runtime not showing up in segmentation cameras
- Add battery sensor

## [0.1.3] - 2021-10-29
- Move to Unreal Engine 4.27 (NOTE: 4.27 now required to build UE environment projects)
- Fix bounding box annotated images not being published if camera images are not rendered (e.g., "--nullrhi" and no image subscribers)
- Merge separate flags enabling 2D and 3D bounding box annotated images into a single flag enabling all annotated images

## [0.1.2] - 2021-10-10
- Add option to spawn robots by lat/lon/alt instead of local NED coordinates by replacing the origin's `xyz` with `geo-point`
- Fix for accounting for rotation in oriented 3D bounding box center
- Add global wind speed to scene config and add `world.SetWindVelocity()` API, with `hello_wind.py` example script
- Fix depth image unpacking as `16UC1` format to preserve depth value accuracy (units are distance in millimeters) and fix naming from `depth_planner` to `depth_planar`
- Add `hello_weather_apis.py` and `hello_time_apis.py` example scripts
- Add `world.ResetWeatherEffects()` API and fix weather settings persisting even if scene is reloaded
- Fix crash from calculating camera projection matrix on a disabled scene camera
- Fix ImageDisplay bug that could endlessly fill image queues if display pop-ups are closed by the ESC key while the image subscription stays active
- Add support for using a PC game controller as an RC controller for SimpleFlight
- Add ROS bridge client wrapper with pub/sub support for robot pose, camera pose, camera images, lidar point clouds, radar detections/tracks, scene loading, and robot/camera transform frames
- Add `drone.SetCameraPose()` API for changine a camera's pose, with `hello_camera_setpose.py` example script
- Add `LidarDisplay` point cloud colorization by each point's segmentation ID

## [0.1.1] - 2021-09-10
- Expose PX4 lock step timeout thresholds as optional config settings
- Disable QGC proxy in example PX4 SITL settings to avoid unintended conflicts with external mavlink proxy utilities
- Improve PX4 parameter setting robustness by increasing number and frequency of retries before throwing an exception
- Added optional `tiles-dir-is-client-relative` setting to resolve a client-relative path to local glTF tiles
- Enabled Python 3.9 support by removing `open3d` requirement (`open3d` still needs to be pip installed manually to use LidarDisplay visualization utility)
- Added `jsonschema` as an explicit client package dependency
- Added VTOL fixed wing flight support to Simple Flight controller
- Support getting bounding box annotated images, with `hello_bbox.py` example script
- Support loading glTF assets at runtime, with `runtime_assets.py` example script

## [0.1.0] - 2021-08-06
- Initial release

---

Copyright (C) Microsoft Corporation.  All rights reserved.
