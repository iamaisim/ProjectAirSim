# How to Modify a Drone's Visual Appearance

A drone's appearance can be easily changed by modifying the `visual`: `geometry`: `name` reference in the `Frame` link of its **[robot config](config_robot.md)** to any mesh path that is included in the Unreal environment. This can be any custom mesh that has been imported into the environment, or a standard Unreal Engine primitive mesh such as a cone shown below:

``` json
"links": [
  {
    "name": "Frame",
    "inertial": {...},
    "collision": {...},
    "visual": {
      "geometry": {
        "type": "unreal_mesh",
        "name": "/Engine/BasicShapes/Cone"
      }
    }
  },
  ...
]
```

![Drone as a cone](images/projectairsim_plugin_spawn_cone.jpg)

## How to import a custom mesh

To use a custom mesh, it first needs to be **[imported into Unreal](https://docs.unrealengine.com/en-US/Engine/Content/Types/StaticMeshes/HowTo/Importing/index.html)**.

Once the asset is available in the environment's Content Browser, its content path can be found by hovering over the asset and combining the asset's listed Unreal `Path` with the mesh's name. For example, the `Quadrotor1` mesh in the `Drone` plugin has Unreal Path = `/Drone` so the full path is `/Drone/Quadrotor1`. This full path can then be used as a `visual`: `geometry`: `name` string to be loaded for the robot's `Frame` link when the simulation starts.

If the mesh size needs to be visually adjusted, you can use the `scale` setting to modify the scale on its "X Y Z" axes.

**Note:** Any link's scaling will be inherited by its child links in the tree, so the child links may need to also be adjusted to compensate.

Example of uniform 2x mesh scaling:

``` json
"geometry": {
  "type": "unreal_mesh",
  "name": "/Drone/Quadrotor1",
  "scale": "2.0 2.0 2.0"
}
```

---

Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.

MIT License. All rights reserved.
