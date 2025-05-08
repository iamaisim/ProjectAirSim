# Configuration JSONC Settings

Project AirSim configuration files use the [JSONC (JSON with Comments)](https://commentjson.readthedocs.io/en/latest/) file format. The configuration JSONC settings that client scripts load are stored in a `sim_config/` subfolder relative to the client script:

```
<client scripts>
└─ sim_config
      scene_basic_drone.jsonc
      scene_...
      robot_quadrotor_fastphysics.jsonc
      robot_...
```

The configuration settings are split into two levels:

1. **[Scene config](#scene-config)** - Settings for a specific scene, such as a list of actors and clock settings for that scene. This can be any JSONC file in the `sim_config/` folder, but generally starts with `scene_` as a filename prefix.

2. **[Robot config](#robot-config)** - Settings for a specific robot, such as its link/joint structure, controller, sensors, etc. This can be any JSONC file in the `sim_config/` folder, but generally starts with `robot_` as a filename prefix. Multiple actors in a scene can reference the same config file.

3. **[Environment actors config](#environment-actors-config)** - Settings for a specific environment actor, such as its link/joint structure, controller, sensors, etc. This can be any JSONC file in the `sim_config/` folder. For drones, the filename prefix generally start with `env_actor_`; for cars, they start with `env_actor_car`; for humans, they start with `env_actor_human`; and for particles, they start with `env_particle_effect`. Multiple actors in a scene can reference the same config file.

*Note: Unlike Python, the JSON standard requires double-quotes (`"`) so be sure not to use single-quotes (`'`) in the JSONC configuration files.*

## Scene config

An example scene config look like:

`scene_env_actor.jsonc`
``` json
{
  "id": "SceneBasicDrone",
  "actors": [
    {
      "type": "robot",
      "name": "Drone1",
      "origin": {...},
      "robot-config": "robot_quadrotor_fastphysics.jsonc"
    }
  ],
  "environment-actors": [
    {
      "type": "env_car",
      "name": "car1",
      "origin": {...},
      "env-actor-config": "env_actor_car.jsonc"
    }
  ],
  "clock": {...},
  "home-geo-point": {...},
  "segmentation": {...},
  "scene-type": "UnrealNative"
}
```

The `actors` array contains a block for each robot to spawn in the simulation at its specified `origin` coordinates, and each robot's `robot-config` string points to the robot config JSON file to load for it.

For more detailed info, see **[Scene Configuration Settings](config_scene.md)**.

## Robot config

An example robot config looks like:

`robot_quadrotor_fastphysics.jsonc`
``` json
{
  "physics-type": "fast-physics",
  "links": [...],
  "joints": [...],
  "controller": {...},
  "actuators": [...],
  "sensors": [...]
}
```

For more detailed info, see **[Robot Configuration Settings](config_robot.md)**.

## Environment actors config

An example environment actor config looks like:

`env_actor_car.jsonc`
``` json
{
  "links": [...],
  "joints": [...],
}
```

For more detailed info, see **[Environment Actor Configuration Settings](config_env_actors.md)**.

---

Copyright (C) Microsoft Corporation.  All rights reserved.
