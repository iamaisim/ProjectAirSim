# Multiple Robots in a Simulation

Project AirSim supports multiple robots in the same simulation, including multiple robots of the same or different types.

Each robot can also have it's own physics type that will run on the common simulation clock and interact with the same common environment, but the robots can not interact with each other except for basic collision detection. Also, multiple physics types for a single robot is not supported.

## How to use multiple robots

You can simply add another actor block in the **[Scene Configuration Settings](config_scene.md)** and give it a new name identifier. The client can interact with each drone based on their names.

``` json
"actors": [
  {
    "type": "robot",
    "name": "Drone1",
    "origin": {
      "xyz": "0.0 0.0 -10.0",
      "rpy-deg": "0 0 0"
    },
    "ref": "robot_quadrotor_fastphysics.jsonc"
  },
  {
    "type": "robot",
    "name": "Drone2",
    "origin": {
      "xyz": "0.0 2.0 -10.0",
      "rpy-deg": "0 0 0"
    },
    "ref": "robot_quadrotor_fastphysics.jsonc"
  }
],
```

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI Consulting Corp.

MIT License. All rights reserved.
