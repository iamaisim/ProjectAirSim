# Project AirSim Documentation

Welcome to the Project AirSim documentation!

```{include} ../../README.md
:relative-docs: docs_build/source
:relative-images: docs_build/source/images
```

```{toctree}
:maxdepth: 2
:caption: Getting Started
internal/use_source.md
internal/use_prebuilt.md
internal/dev_setup_linux.md
internal/dev_setup_win.md
internal/headless_cloud.md
internal/vscode_user_settings.md
---
:maxdepth: 2
:caption: ROS Integration
ros/ros.md
ros/ros2.md
ros/ros_examples.md
ros/example_build_map.md
ros/example_navigate_map.md
---
:maxdepth: 2
:caption: Sensors
sensors/battery.md
sensors/lidar.md
sensors/radar.md
sensors/segmentation.md
sensors/camera_capture_settings.md
sensors/camera_post_processing_with_nn.md
sensors/camera_streaming.md
---
:maxdepth: 2
:caption: Data Collection
datacollection/config.md
datacollection/data_aggregation.md
datacollection/data_generation.md
datacollection/randomizations.md
datacollection/trajectory.md
---
:maxdepth: 2
:caption: Physics
physics/fast_physics.md
physics/matlab_physics.md
internal/physics/unreal_physics.md
---
:maxdepth: 2
:caption: Scene and Simulation
scene/sim_clock.md
scene/weather_visual_effects.md
internal/scene/sim_clock_internal.md
internal/sensors/display.md
