# How to Modify a Drone's Physical Geometry

The physical geometry of the drone can be modified through the **[Robot Configuration Settings](config_robot.md)**.

For example, to move the `robot_quadrotor_fastphysics.jsonc` propellers to be farther away from the frame at x = 0.5 m and y = 0.5 m, you can adjust the propeller link origins for the link's `inertial` component to change the physics representation, and again for the link's `visual` component to change the rendered position of the propeller mesh:

``` json
{
  "name": "Prop_FL",
  "inertial": {
    "origin": {
      "xyz": "0.50 -0.50 -0.01",
      "rpy-deg": "0 0 0"
    },
    "mass": 0.055,
    "inertia": {...},
    "aerodynamics": {...}
  },
  "visual": {
    "origin": {
      "xyz": "0.50 -0.50 -0.01",
      "rpy-deg": "0 0 0"
    },
    "geometry": {
      "type": "unreal_mesh",
      "name": "/Drone/Propeller"
    }
  }
},
```

Although the physical configuration can be modified to affect the physics and positioning of the components, the meshes may not visually align any more so the visual appearance may also need to be modified as described in **[How to Modify a Drone's Visual Appearance](modify_drone_visual.md)**.

---

Copyright (C) Microsoft Corporation.  All rights reserved.
