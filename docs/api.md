# API Overview

---

## API Notes

1. Stability: APIs are evolving at this stage and could change based on the feedback. So please do provide feedback.
2. AirSim Compatibility: It is NOT a goal to maintain strict backward compatibility with GitHub AirSim project APIs. Wherever appropriate, learnings from AirSim project are used to update the APIs in this version. At the same time, a lot of familiar concepts have been carried over to enable smooth transition for users who are familiar with GitHub AirSim.

---

## API Concepts

1. Entities: The user code interacts with the simulation backend via these 3 objects:
    - Client: provides APIs for connection management between user code (a.k.a. simulation client) and simulation backend.
    - World:  provides APIs for simulation environment management (loading of scene, clock management, weather management, etc.)
    - Robot specific entities, e.g. Drone: provides APIs for robot control as well as sensor data.
2. Controls and Sensor data channels: The user code will typically send the control commands to the simulation backend using request-response model while it would subscribe to sensor data via the publish-subscribe model.
3. Async APIs: APIs that end with *Async return the control back to the user code immediately after invoking the call. It is expected that the users will use language specific async models to interact with these APIs, e.g., for Python clients, the user code would use asyncio package to interact with the *Async APIs.
4. Units: Unless specified otherwise, the API inputs and outputs are assumed to be in SI units (meters and radians) and in NED convention.

---

## Client Authorization

The Project AirSim server may optionally enable client authorization verification (to enable this, see the ["-clientauthpubkey" command line option](command_line_switches.md)).  When client authorization verification is enabled, clients must present a valid client authorization token immediately after connecting and before calling other API methods (include creating a `World` object).  If a valid unexpired token is not set or the token expires, API calls return a "client not authorized" error and the client won't received updates to subscribed topics.

To present a client authorization token, call the [`set_authorization_token()`]() method.  This method returns a `datetime` object for when the authorization expires (or expired.)  The client must present a new token before the current token expires to continue using the client API.

---

## API Example

### Python

(please see hello_drone.py for complete code)

```python
    import asyncio
    # Import ProjectAirSim client libraries
    from projectairsim import ProjectAirSimClient, World, Drone

    async def main():
        # Create client object
        client = ProjectAirSimClient()
        client.connect()

        # Create world object and load the scene from its config
        world = World(client, "scene_basic_drone.jsonc", delay_after_load_sec=2)

        # Create drone object
        drone = Drone(client, world, "Drone1")

        # Control drone
        move_up = await drone.move_by_velocity_async(
            v_north=0.0, v_east=0.0, v_down=-1.0, duration=4.0
        )
        await move_up

        client.disconnect()

    if __name__ == "__main__":
        asyncio.run(main())
```

---

## Client

### Client Object

- [`ProjectAirSimClient(address, port_topics, port_services)`]()

### Connection API

- [`connect()`]()
- [`disconnect()`]()

### Authorization API
- [`set_authorization_token(token)`]()

### Low Level Communication API

- [`get_topic_info()`]()
- [`subscribe(topic, callback)`]()
- [`publish(topic, message)`]()
- [`unsubscribe(topic)`]()
- [`unsubscribe_all()`]()
- [`request(request_data)`]()
- [`request_async(request_data, callback)`]()

---

## World

### World Object

- [`World(client, scene_config, delay_after_load_sec, sim_config_path)`]()

### Scene Control API

- [`load_scene(scene_config, delay_after_load_sec)`]()
- [`set_time_of_day(status, datetime, is_dst, clock_speed, update_interval, move_sun)`]()
- [`set_sun_angle_from_date_time(datetime, is_dst)`]() - Uses the date and time to set the position of the sun in the sky
- [`set_sunlight_intensity(intensity)`]() - Sets the sun light intensity in the scene. Default is 2.75. Range of intensity is 0-75000. However, beyond 10 it's too bright to observe objects in the scene.
- [`get_sunlight_intensity()`]() - Returns the value of Sun light intensity in the scene.
- [`set_cloud_shadow_strength(strength)`]() - Sets the strength of clouds shadows onto the scene. Disabled by default. Range varies from 0.0 to 1.0.
- [`get_cloud_shadow_strength()`]() - Gets the cloud shadow strength value.

### Scene Object API

For Unreal objects, the `object_name` refers to the object ID which can be found in the Outliner panel in the Unreal Editor. You can toggle visibility of `ID Name` by right clicking the the Item Label header and selecting it.

- [`list_actors()`]()
- [`list_objects(name_regex)`]()
- [`get_object_pose(object_id)`]()
- [`set_object_pose(object_name, object_pose, teleport)`]()
- [`get_object_scale(object_id)`]()
- [`set_object_scale(object_name, object_scale)`]()
- [`spawn_object(object_name, asset_path, object_pose, object_scale, enable_physics)`]()
- [`spawn_object_at_geo(object_name, asset_path, latitude, longitude, altitude, rotation, object_scale, enable_physics)`]()
- [`destroy_object(object_name)`]()
- [`set_object_texture_from_url(object_name, url)`]()
- [`swap_object_texture(tag, tex_id)`]()
- [`set_segmentation_id_by_name(mesh_name, segmentation_id, is_name_regex, use_owner_name)`]()
- [`get_segmentation_id_by_name(mesh_name, use_owner_name)`]()

### Sim Clock API

- [`get_sim_clock_type()`]() - Get the current sim clock's type (**steppable** or **real-time**).
- [`get_sim_time()`]() - Get the sim clock's current sim time in nanosec.
- [`pause()`]() - Pause the advancement of sim time.
- [`resume()`]() - Resume the advancement of sim time.
- [`is_paused()`]() - Check whether the sim clock is currently paused or not.
- [`continue_for_sim_time(delta_time_nanos, wait_until_complete)`]() - Allow sim time to advance until **delta_time_nanos** has passed and then pause.
- [`continue_until_sim_time(target_time_nanos, wait_until_complete)`]() - Allow sim time to advance until reaching **target_time_nanos** and then pause.
- [`continue_for_n_steps(n_steps, wait_until_complete)`]() - Allow sim time to advance for **n_steps** number of time steps and then pause.
- [`continue_for_single_step(wait_until_complete)`]() - Allow sim time to advance for a single time step and then pause.

### Weather Visual Effects

- [`enable_weather_visual_effects()`]() - Enable weather effects in the scene.
- [`disable_weather_visual_effects()`]() - Disable weather effects in the scene.
- [`reset_weather_effects()`]() - Clear scene of weather effects.
- [`set_weather_visual_effects_param(param, value)`]() - Add weather effects based on a parameter type which includes both falling and fallen (ground cover) leaves, rain, and snow as well as dust and fog for a given density value ranging from 0 to 1.

### Mapping API

- [`create_voxel_grid(position, x_size, y_size, z_size, res, n_z_resolution, use_segmentation, write_file, file_path)`]() - Creates a voxel grid for the current scene. Can Output a `.binvox` file that represets the grid and returns a 1D occupancy array.

The index in the occupancy array for a particular point (needs to be within the bounds determined through the params) in the scene can be calculated as follows:

```
Center Coords: x_c, y_c, z_c // in m
Desired Point: x, y, z // in m

array-index = x_idx + x_cells * (z_idx + z_cells * y_idx)

Here,
x_cells = x_size/resolution (similar for y_cells)
z_cells = z_size/(resolution * n_z_resolution)
x_idx = x + x_cells/2 - x_c (similar for y_idx and z_idx)
```

Value of the the array at this index (True or False) determines the occupancy of that cell.

### New Feature: Voxel Grid Segmentation (`use_segmentation=True`)

When `use_segmentation` is set to **True**, the voxel grid will store the **segmentation ID** of objects, rather than a simple binary representation (1 or 0). In this case, the voxel grid array will contain the segmentation ID for each voxel, which corresponds to the object occupying that space.

#### Key Behavior:

- **Multiple Objects in a Voxel**: If more than one object occupies the same voxel grid volume, the object with the **largest segmentation ID** will be stored in that voxel.
  
- **Segmentation ID 0**: If any object has a segmentation ID of **0**, it could be mistaken for an unoccupied space (since the default is often 0 for empty voxels). To resolve this ambiguity, see next section.

#### Solutions for Segmentation ID 0:

1. **Cross-reference with a Non-Segmented Grid**: To accurately interpret voxel data when `use_segmentation=True`, consider performing an additional check using a voxel grid generated with `use_segmentation=False`. This will help you differentiate between empty space and an object with ID 0.

2. **Avoid Using Segmentation ID 0**: A simpler solution is to avoid setting segmentation ID 0 for objects you want to include in the segmentation. By assigning a non-zero segmentation ID to these objects, you prevent any confusion with unoccupied space.


### Visual Debugging Tools

- [`flush_persistent_markers()`]() - Clears the world of any debugging traces.
- [`plot_debug_points(points, color_rgba, size, duration, is_persistent)`]() - Plots debugging points in the world.
- [`plot_debug_solid_line(points, color_rgba, thickness, duration, is_persistent)`]() - Renders a solid debug line in the world.
- [`plot_debug_dashed_line(points, color_rgba, thickness, duration, is_persistent)`]() - Renders a dashed debug line in the world. Works only with an even number of points.
- [`plot_debug_arrows(points_start, points_end, color_rgba, thickness, arrow_size, duration, is_persistent)`]() - Renders debug arrows in the world.
- [`plot_debug_strings(strings, positions, scale, color_rgba, duration)`]() - Creates debug text in the world.
- [`plot_debug_transforms(poses, scale, thickness, duration, is_persistent)`]() - Renders debug coordinate system indicators in the world.
- [`plot_debug_transforms_with_names(poses, names, tf_scale, tf_thickness, text_scale, text_color_rgba, duration)`]() - Renders debug coordinate system indicators with text in the world.
- [`toggle_trace()`]() - Toggle displaying the trace of the drone's path. Pressing 'T' while focused on the Unreal window also toggles displaying the trace.
- [`set_trace_line(color_rgba, thickness)`]() - Configure parameters for the trace line.

| Parameter | Description |
| --- | ----------- |
| color_rgba | List of red, green, blue, alpha values in that specific order (each normalized to 1.0) |
| duration | (seconds) |
| is_persistent | If True, 'duration' is ignored and markers are displayed until removed by flush_persistent_markers() |
| names/strings | List of strings |
| points/points_start/points_end/positions | List of list of floats, list of floats refers to [x, y, z] |
| poses | List of Pose objects, each pose has a translation and rotation component |
| scale | Scaling factor for text or transform |
| size | Scale of the point or arrow |
| thickness | Thickness of line or transform |

Refer to the example user script "hello_debug_utils.py" for more details on how to use this visual debugging API.

---

## Drone

### Drone Object

- [`Drone(client, world, name)`]()

### Control API

#### Prerequisites

- [`enable_api_control(), disable_api_control()`]()
- [`arm(), disarm()`]()

#### Control commands

- [`cancel_last_task()`]()
- [`takeoff_async(timeout_sec, callback)`]()
- [`land_async(timeout_sec, callback)`]()
- [`go_home_async(timeout_sec, callback)`]()
- [`hover_async(callback)`]()
- [`move_by_velocity_async(v_north, v_east, v_down, duration, yaw_control_mode, yaw_is_rate, yaw, callback)`]()
- [`move_by_velocity_z_async(v_north, v_east, z, duration, yaw_control_mode, yaw_is_rate, yaw, callback)`]()
- [`move_to_position_async(north, east, down, velocity, timeout_sec,  yaw_control_mode, yaw_is_rate, yaw, lookahead, adaptive_lookahead, callback)`]()
- [`move_on_path_async(path, velocity, timeout_sec, yaw_control_mode, yaw_is_rate, yaw, lookahead, adaptive_lookahead, callback)`]()
- [`rotate_to_yaw_async(yaw, timeout_sec, margin, callback)`]()
- [`rotate_by_yaw_rate_async(yaw_rate, duration, callback)`]()
- [`move_by_heading_async(heading, speed, climb_rate, duration, heading_margin, timeout_sec, callback)`]()

##### Manual Controller commands

- [`set_control_signals(control_signal_map: Dict)`]() - Manually set the control signal outputs. The parameter `control_signal_map` dictionary has keys as the target actuator ID string (e.g. `"Prop_FR_actuator"`) and values as the control signal output float value (e.g. `0.3`). Multiple control signals can be set in a single call and the values will remain as set until the next call to set them again.

### State API

- [`get_ground_truth_kinematics()`]()
- [`get_ground_truth_pose()`]()
- [`set_pose(pose, reset_kinematics)`]()
- [`set_mission_mode(callback)`]()
- [`set_vtol_mode_async(vtol_mode, callback)`]()
- [`set_ground_truth_kinematics(kinematics: Dict)`]() - Allows the client to set the Kinematic state of the drone. Here the dictionary `kinematics` is in the following format.

```
position = Vector3({"x": 0, "y": 0, "z": 0})
orientation = Quaternion({"w": 0, "x": 0, "y": 0, "z": 0})
linear_twist = Vector3({"x": 0, "y": 0, "z": 0})
angular_twist = Vector3({"x": 0, "y": 0, "z": 0})
linear_accn = Vector3({"x": 0, "y": 0, "z": 0})
angular_accn = Vector3({"x": 0, "y": 0, "z": 0})

accn = {"angular": angular_accn, "linear": linear_accn}
pose = {"orientation": orientation, "position": position}
twist = {"angular": angular_twist, "linear": linear_twist}

kinematic = {"time_stamp": 0, "pose": pose, "twist": twist, "accels": accn}

drone.set_ground_truth_kinematics(kinematic)
```
Note: `linear_twist["z"]` needs to have a -ve value (upwards direction) for the drone to move with the specified kinematics


### Sensors API

#### Camera API

- [`get_images(camera_id, image_type_ids)`]() - if annotations are enabled, this will also output bounding boxes. See [Annotations Settings](sensors/camera_capture_settings.md#annotation-settings)

#### IMU API

- [`get_imu_data(sensor_name)`]()

#### GPS API

- [`get_gps_data(sensor_name)`]()

#### Barometer API

- [`get_barometer_data(sensor_name)`]()

#### Magnetometer API

- [`get_magnetometer_data(sensor_name)`]()

### Airspeed API

- [`get_airspeed_data(sensor_name)`]()

#### Battery API

- [`get_battery_state(sensor_name)`]()
- [`set_battery_remaining()`]()
- [`get_battery_drain_rate(sensor_name)`]()
- [`set_battery_drain_rate(desired_drain_rate)`]()
- [`set_battery_remaining(desired_battery_remaining)`]()
- [`set_battery_health_status(is_desired_state_healthy)`]()


---

Copyright (C) Microsoft Corporation. 
Copyright (c) 2025 IAMAI Simulations.

MIT License. All rights reserved.
