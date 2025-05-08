# Environment Actors Configuration Settings

{# include enable_internal_docs.tpp #}
Environment actors in Project AirSim are elements within the simulation environment that enhance its realism and complexity. These actors are not directly involved in primary simulation tasks but contribute to the overall immersive and authentic experience. They include a variety of entities designed to replicate real-world surroundings, which help in creating a comprehensive testing scenario for autonomous systems.

The environment actors have the capacity of having a defined trajectory to follow. It can be configured in the same settings file **[Trajectory settings](#trajectory-settings)**, inside the script or using a path file.

There are different types of environment actors: 
    - Generic Environment Actors: **[Generic Environment Actors configuration overview settings](#generic-environment-actors-configuration-overview)**
    - Cars: **[Car configuration overview overview settings](#car-configuration-overview)** 
    - Humans: **[Human configuration overview settings](#human-configuration-overview)**

The Generic Actors and Cars are configured as tree structures of **links** (components with shape) and **joints** (connections and constraints between links).

The Humans are only configured by a **link**.

## Trajectory settings

The environment actors' settings can also include the trajectory they will follow. In order to confifure this, a series of parameter have to be set in `script`.

For example:

``` json
{
  "script": {
    "loop": true,
    "auto_start": true,
    "trajectory": {
      "name": "right_and_descend_config",
      "time_sec": [1, 3, 6, 9, 12],
      "pose_x": [3, 3, 3, 3, 3],
      "pose_y": [0, 5, 10, 15, 20],
      "pose_z": [-5, -4, -4, -4, -4],
      "pose_roll": [0, 0, 0, 0, 0],
      "pose_pitch": [0, 0, 0, 0, 0],
      "pose_yaw": [0, 0, 0, 0, 0],
      "velocity_linear_x": [1, 1, 1, 1, 1],
      "velocity_linear_y": [1, 1, 1, 1, 1],
      "velocity_linear_z": [0, 0, 0, 0, 0]
    }
  },
  {...}
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
|   `loop`  | bool  | Whether the movement of the actor should loop. |
|`auto_start`| bool | Whether the movement of the actor should begin automatically. |
|`trajectory`| object | The path that the actor will follow. |
|`trajectory`: `name`| string | The name of the trajectory to follow.|
|`trajectory`: `time_sec`| array | The times, in seconds, of each point on the trajectory.|
|`trajectory`: `pose_x`| array of numbers | The x pose at each point on the trajectory.|
|`trajectory`: `pose_y`| array of numbers | The y pose at each point on the trajectory.|
|`trajectory`: `pose_z`| array of numbers | The z pose at each point on the trajectory.|
|`trajectory`: `pose_roll`| array of numbers | The roll at each point on the trajectory.|
|`trajectory`: `pose_pitch`| array of numbers | The pitch pose at each point on the trajectory.|
|`trajectory`: `pose_yaw`| array of numbers | The yaw pose at each point on the trajectory.|
|`trajectory`: `velocity_linear_x`| array of numbers | The x velocities at each point on the trajectory.|
|`trajectory`: `velocity_linear_y`| array of numbers | The y velocities at each point on the trajectory.|
|`trajectory`: `velocity_linear_z`| array of numbers | The z velocities at each point on the trajectory.|

## Generic Environment Actors configuration overview 

Versatile entities that can be added to the scene. 

Project AirSim currently comes with some base configurations for environment actors to demonstrate how to configure them for simulation.

`env_actor_quadrotor.jsonc`
``` json
{
  "links": [...],
  "joints": [...]
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `links`   | Array of **[Link settings](#link-settings-actors)** | Link settings for each link component of the actors. |
| `joints`  | Array of **[Joint settings](#joint-settings-actors)** | Joint settings for connecting links together. |

## Link settings actors

A link is a component with shape. The link settings consist of collision and visual elements.

``` json
"links": [
  {
    "name": "Frame",
    "collision": {...},
    "revolutions-per-sec": 2,
    "axis": "0 0 1",
    "parent-link": "Frame",
    "visual": {...}
  },
  ...
]
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `name` | string | Name identifier for the link. |
| `collision` | **[Collision settings](#collision-settings)** | Collision settings for collision detection and response. |
| `revolutions-per-sec` | number | The ammount of revolutions per second that the link will have. Use only if the link is a rotating component. |
| `axis` | string of 3 numbers | The axis around which the link will rotate. Use only if the link is a rotating component.|
| `parent-link` | string | Name identifier for the parent link. Use only in child links.|
| `visual` | **[Visual settings](#visual-settings)** | Visual settings for the link's rendered mesh. |

### Collision settings

The link's collision settings consist of an enabled flag and parameters for restitution and friction coefficients used in calculating its collision response. For environment actors, it must be set to false.

``` json
"collision": {
  "enabled": false
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `enabled` | bool | Flag to enable/disable collision detection for this link's **[visual mesh](#visual-settings)**. Disabling collision may be useful for non-physics "computer vision" mode while still being able to see the car's mesh visually. This defaults to `true` if omitted. **Note:** For environment actors the collision must be set to FALSE.|

### Visual settings

The link's visual settings consist of what visual element will be rendered for the link. For generic environment actors, only `unreal_mesh` (static mesh) geometry types are supported.

``` json
"visual": {
        "geometry": {
          "type": "unreal_mesh",
          "name": "/Drone/Quadrotor1",
          "scale": "1.0 1.0 1.0"
        }
      }
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `geometry`: `type` | `unreal_mesh` | The type of mesh geometry to use (currently only `unreal_mesh` is supported for generic environment actors). The mesh must be imported into the Unreal environment as a .uasset to be available at runtime. |
| `geometry`: `name` | string | For `unreal_mesh`, this should be the Unreal content path to the mesh in the environment. **Note:** The PIVOT in the Frame of the Static Mesh has to be set in the base of the Z axis of the chassis' wheel axle, and in the center of the X and Y axis. |
| `geometry`: `scale` | string of 3 floats | Adjust the mesh's scale. Defaults to all 1.0 if not specified. **Note:** A link's scale will also be inherited by any child links to maintain relative size, so their scales may also need adjustment to compensate. |

## Joint settings

A joint is a connection between two links (a single parent link and a single child link) in order to define a relationship for relative motion. Constraints for the joint type can specify the type of joint motion to allow.

``` json
"joints": [
  {
      "id": "Frame_Prop_FL",
      "origin": {
        "xyz": "0.253 -0.253 -0.01",
        "rpy-deg": "0 0 0"
      },
      "type": "fixed",
      "parent-link": "Frame",
      "child-link": "Prop_FL",
      "axis": "0 0 1"
    },
  ...
]
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `id` | string | Name identifier for the joint. |
| `origin`: `xyz` | string of 3 floats | Position "X Y Z" of the joint's child link relative to the joint's parent link in **meter** units. Defaults to all zero if no `origin` is specified. |
| `origin`: `rpy` | string of 3 floats | Rotation "Roll Pitch Yaw" of the joint's child link relative to the joint's parent link in **radian** units. Defaults to all zero if no `origin` is specified. |
| `type` | `fixed`, `continuous`, `revolute` | Type of joint constraint. `fixed` means the pose of the `child-link` should stay fixed relative to the `parent-link`. `continuous` means the `child-link` can rotate continously around the `axis` normal. `revolute` means the `child-link` can rotate like a hinge around the `axis` up to an angle `limit`. |
| `parent-link` | string | Name identifier for the joint's parent link. |
| `child-link` | string | Name identifier for the joint's child link. |
| `axis` | `"1 0 0"` for X axis, `"0 1 0"` for Y axis, `"0 0 1"` for Z axis | Rotation axis for joint motion. |

{#ifdef INTERNAL_DOCS #}
{# include begin_internal.md #}

## Car configuration overview

Ground vehicles designed to simulate real-world traffic and driving conditions. This is a grounded environment actor since it is affected by gravity. It also has wheels that follow the trajectory given.

Project AirSim currently comes with some base configurations for environment cars to demonstrate how to configure them for simulation.

`env_actor_car.jsonc`
``` json
{
  "links": [...],
  "joints": [...]
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `links`   | Array of **[Link settings](#link-settings-cars)** | Link settings for each link component of the car. **Note:** For Skeletal Meshes there is only one link necessary. |
| `joints`  | Array of **[Joint settings](#joint-settings-cars)** | Joint settings for connecting links together. **Note:** Skeletal Meshes don't use joints. |

## Link settings cars

For environment cars, the links settings are similar to actors setting but it only includes `name`, `collision` and `visual`

``` json
"links": [
  {
    "name": "Frame",
    "collision": {...},
    "visual": {...}
  },
  ...
]
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `name` | string | Name identifier for the link. |
| `collision` | **[Collision settings](#collision-settings)** | Collision settings for collision detection and response. **Note:** The same as actors collision settings. |
| `visual` | **[Visual settings](#visual-settings-cars)** | Visual settings for the link's rendered mesh for cars. |

**Note:** If the cars are using static meshes, the settings must include a "Normal" link to allow yaw rotation independently of the rotation in x and y. For example:

``` json
{
    "name": "FL_Normal",
    "collision": {
    "enabled": false
    },
    "visual": {
    "origin": {
        "xyz": "1.2 -0.7 0.0",
        "rpy-deg": "0 0 0"
    },
    "geometry": {
        "type": "unreal_mesh",
        "name": "no_frame_file"
    }
    }
}
```

### Visual settings cars

The link's visual settings consist of what visual element will be rendered for the link. For environment cars, only `unreal_mesh` (static mesh) geometry types are supported.

``` json
"visual": {
  "origin": {
    "xyz": "1.2 -0.7 0.0",
    "rpy-deg": "0 0 0"
  },
  "geometry": {
    "type": "unreal_mesh",
    "name": "/ProjectAirSim/SUV/SUV_Chassis",
    "scale": "1.0 1.0 1.0"
  },
  "material": {
    "colored-texture": "/ProjectAirSim/SUV/AutomotiveMaterials/Materials/CarPaint/M_Carpaint_violet"
  }
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `origin`: `xyz` | string of 3 floats | Position "X Y Z" of the joint's child link relative to the joint's parent link in **meter** units. Defaults to all zero if no `origin` is specified. |
| `origin`: `rpy-deg` | string of 3 floats | Rotation "Roll Pitch Yaw" of the joint's child link relative to the joint's parent link in **degree** units. Defaults to all zero if no `origin` is specified. |
| `geometry`: `type` | `unreal_mesh`, `skeletal_mesh` | The type of mesh geometry to use. The mesh must be imported into the Unreal environment as a .uasset to be available at runtime. Skeletal ans static mesh are currently supported. |
| `geometry`: `name` | string | This should be the Unreal content path to the mesh in the environment.  **Note:** For the `unreal_mesh`, the PIVOT in the Frame of the Static Mesh has to be set in the base of the Z axis of the chassis' wheel axle, and in the center of the X and Y axis. While for the `skeletal_mesh` the PIVOT has to be set in the base of the Z axis of the whole car. |
| `geometry`: `scale` | string of 3 floats | Adjust the mesh's scale. Defaults to all 1.0 if not specified. **Note:** A link's scale will also be inherited by any child links to maintain relative size, so their scales may also need adjustment to compensate. |
| `material`: `colored-texture` | string | This should be the Unreal Material or Material Instance content path to change the color of the chassis. **Note:** In the Static Mesh search for the chassis element that needs to be changed, and in the Slot Name make sure the word "Carpaint" is included, otherwise the new material will not show up. |

## Joint settings cars

A joint is a connection between two links (a single parent link and a single child link) in order to define a relationship for relative motion. Constraints for the joint type can specify the type of joint motion to allow.

``` json
"joints": [
  {
      "id": "Frame_FL_Normal",
      "type": "fixed",
      "parent-link": "Frame",
      "child-link": "FL_Normal",
      "axis": "0 0 1"
    },
  ...
]
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `id` | string | Name identifier for the joint. |
| `type` | `fixed`, `continuous`, `revolute` | Type of joint constraint. `fixed` means the pose of the `child-link` should stay fixed relative to the `parent-link`. `continuous` means the `child-link` can rotate continously around the `axis` normal. `revolute` means the `child-link` can rotate like a hinge around the `axis` up to an angle `limit`. **Note:** Cars currently only use the `fixed` type. |
| `parent-link` | string | Name identifier for the joint's parent link. |
| `child-link` | string | Name identifier for the joint's child link. |
| `axis` | `"1 0 0"` for X axis, `"0 1 0"` for Y axis, `"0 0 1"` for Z axis | Rotation axis for joint motion. |

{#ifdef INTERNAL_DOCS #}
{# include begin_internal.md #}

## Human configuration overview

Representations of pedestrians within the simulation. This is a grounded environment actor since it is affected by gravity, that uses an skeletal mesh for refined movement.

`env_actor_human.jsonc`
``` json
{
  "links": [...]
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `links` | Array of **[Link settings](#link-settings-humans)** | Link settings for each link component of the human. **Note:** The humans only need one link for the skeletal mesh |

## Link settings humans

A link is a physical component with shape. The link settings consist of collision and visual elements.

``` json
"links": [
  {
    "name": "Frame",
    "collision": {...},
    "visual": {...}
  },
  ...
]
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `name` | string | Name identifier for the link. |
| `collision` | **[Collision settings](#collision-settings)** | Collision settings for collision detection and response. **Note:** The same as actors collision settings. |
| `visual` | **[Visual settings](#visual-settings-humans)** | Visual settings for the link's rendered mesh. |

### Visual settings humans

The link's visual settings consist of what visual element will be rendered for the link. For environment humans, only `skeletal_mesh` geometry types are supported.

``` json
"visual": {
  "geometry": {
    "type": "skeletal_mesh",
    "name": "/ProjectAirSim/Scanned3DPeoplePack/RP_Character/rp_carla_rigged_001_ue4/rp_carla_rigged_001_ue4",
    "scale": "1.0 1.0 1.0"
  }
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `geometry`: `type` | `skeletal_mesh` | The type of mesh geometry to use (currently only `skeletal_mesh` is supported). The mesh must be imported into the Unreal environment as a .uasset to be available at runtime. |
| `geometry`: `name` | string | For `skeletal_mesh`, this should be the Unreal content path to the mesh in the environment. **Note:** Make sure this is a skeletal mesh uasset, otherwise it will not show on the screen. |
| `geometry`: `scale` | string of 3 floats | Adjust the mesh's scale. Defaults to all 1.0 if not specified. |

---

Copyright (C) Microsoft Corporation.  All rights reserved