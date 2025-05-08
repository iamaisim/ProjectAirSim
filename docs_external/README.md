# Welcome to Project AirSim

_Non-public information provided under the terms of Non-Disclosure Agreement with Microsoft Corporation_

**Project AirSim** is an enterprise grade platform to enable rapid development of machine intelligence-centric autonomous systems, involving agents like drones and factory robots operating in the physical world.

In the initial phase, the platform will provide two solution components:

1. **AI-First Simulation** platform

2. Collection of **Autonomy Blocks**

The current drops contain the simulation platform; the autonomy blocks will be added in future drops.

_Project AirSim_ is a name used for pre-release drops and the product name is likely to change in future releases.

## AI-First simulation platform

Related to Microsoft's **[AirSim](https://github.com/microsoft/AirSim)**, the simulation plaform leverages **[Unreal Engine 4](https://www.unrealengine.com/)** to provide photo-realistic visuals, while providing the simulation framework needed to integrate custom physics, controllers, actuators, and sensors to develop an autonomous system.

The simulation platform consists of 3 main layers:

1. **Project AirSim Sim Libs** - Base infrastructure for defining a generic robot structure and simulation scene tick loop

2. **Project AirSim Plugin** - Host package (currently an Unreal Plugin) that builds on the sim libs core to connect external components (controller, physics, rendering) at runtime that are specific to each configured robot-type scenario (ex. quadrotor drones)

3. **Project AirSim Client Library** - End-user library to enable API calls to interact with the robot and simulation over a network connection

The Project AirSim simulation platform currently supports Windows 10/Server 2019 and Ubuntu 20.04. For more info about hardware specs for working with Project AirSim, see **[System Specifications](system_specs.md)**.

![Drone flying in Blocks environment](images/drone_in_blocks.jpg)

## Autonomy blocks

Autonomy blocks are a collection of ML-centric building blocks that enable rapid development of autonomous solutions like drone landing using camera, drone navigation, etc.
If you are interested in exploring and/or building machine learning applications and models for autonomous perception/planning/control tasks, you can leverage the pre-trained models, datasets and learning environments in Project AirSim.
Learn more about the initial offering here:

*Coming soon! Stay tuned for the next release*

## What's New

For a complete list of changes, view our **[Changelog](changelog.md)**.

## Getting started

### Early Access Program (EAP) package

If you've received Project AirSim as an EAP package, you can get started with the **[EAP quickstart](eap_quickstart.md)**.

###  Drop-in Project AirSim Plugin

If you're ready to use the Project AirSim plugin in your own custom environment, see **[Use Project AirSim Plugin in custom environments](use_plugin.md)**.

### Transitioning from AirSim

See **[Transitioning from AirSim](transition_from_airsim.md)** for guidance on converting an AirSim Unreal environment and client code from AirSim to Project AirSim.

## Reference

### Configuration JSONC Settings

- **[Overview](config.md)**
- **[Scene Settings](config_scene.md)**
- **[Robot Settings](config_robot.md)**

### Client API

- **[Overall API Info](api.md)**

### ROS Integration

- **[ROS Bridge Setup and Use](ros/ros.md)**
- **[ROS Bridge Examples](ros/ros_examples.md)**

### Controllers

- **[Flight Controllers](controllers/controllers.md)**
- **[Simple Flight Controller](controllers/simple_flight.md)**
- **[PX4 Flight Controller](controllers/px4/px4.md)**

### Sensors

- Airspeed
- Barometer
- **[Camera](sensors/camera_capture_settings.md)**
- GPS
- IMU
- **[Lidar](sensors/lidar.md)**
- Magnetometer
- **[Radar](sensors/radar.md)**

### Scene

- **[Simulation Clock](scene/sim_clock.md)**
- **[Weather Visual Effects](scene/weather_visual_effects.md)**

### Physics

- **[Fast Physics](physics/fast_physics.md)**

## FAQ

If you run into problems, please check the **[FAQ](faq.md)** for help.

## Support

Please see the [Support page](support.md) for obtaining support for Project AirSim.

## License

Please see the [License page](license.md) for Project AirSim license information.

---

Copyright (C) Microsoft Corporation.  All rights reserved.
