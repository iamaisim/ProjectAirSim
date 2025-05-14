# Robot Configuration Settings

{# include enable_internal_docs.tpp #}

Project AirSim robots are configured as tree structures of **links** (physical components with mass and shape) and **joints** (connections and constraints between links).

The robot can have a **physics type** associated with it to determine a physics engine that will parse the robot structure and construct a corresponding physics body using the data relevant for that physics type.

The robot can also have a **controller** that will decide the control outputs to apply to the robot's actuators.

The array of **actuators** can be specified with the associated links to apply forces and torques to, based on the control outputs.

An array of **sensors** can configure various types of sensors such as cameras, IMUs, etc, can be attached to the robot's links.

Project AirSim currently comes with some base configurations for quadrotor drones to demonstrate how to configure a robot for simulation.

## Robot configuration overview (drone example)

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

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `physics-type` | **[Physics type](#physics-type)** | Physics to use to control the robot's motion |
| `links` | Array of **[Link settings](#link-settings)** | Link settings for each link component of the robot. |
| `joints` | Array of **[Joint settings](#joint-settings)** | Joint settings for connecting links together. |
| `controller` | **[Controller settings](#controller-settings)** | Controller settings to drive the robot's actuators. |
| `actuators` | Array of **[Actuator settings](#actuator-settings)** | Actuator settings for each applying forces and torques to the robot's links. |
| `sensors` | Array of **[Sensor settings](#sensor-settings)** | Sensor settings for attaching sensors to the robot's links. |

## Physics type

### Non-Physics  (Computer Vision mode)

``` json
"physics-type": "non-physics"
```

**Non-physics** mode means that the robot will not move unless its pose is set directly with API calls from the client. This can be useful for "Computer Vision" mode to gather sensor data at manually-specified poses in the simulation world, or if the robot's motion will be determined by an external algorithm.

### Fast Physics

``` json
"physics-type": "fast-physics"
```

**[Fast Physics](physics/fast_physics.md)** is a basic light-weight physics model made for aerial drone flight. Fast Physics makes it easy to get started with flying a drone out-of-the-box.

{# ifdef INTERNAL_DOCS #}
{# include begin_internal.md #}
### Unreal Physics

``` json
"physics-type": "unreal-physics"
```

**[Unreal physics](internal/physics/unreal_physics.md)** uses the **[PhysX](https://developer.nvidia.com/gameworks-physx-overview)** engine that's built-in to Unreal. PhysX can calculate motion for multi-jointed robots with constraints on each joint, as well as rigid body dynamics for aerial drone flight.

*Note: The simulation will automatically detect when any actor in the scene is configured to use Unreal Physics, and this will link the physics calculation step to Unreal's rendering step, so the [sim clock setting](config_scene.md#steppable-clock) for `step-ns` may need to be much slower (20 ms = ~50 FPS) to maintain reasonable simulation advancement rate.*

{# include end_internal.md #}

{# endif INTERNAL_DOCS #}
## Link settings

A link is a physical component with mass and shape. The link settings consist of inertial, collision, and visual elements.

``` json
"links": [
  {
    "name": "Frame",
    "inertial": {...},
    "collision": {...},
    "visual": {...}
  },
  ...
]
```


| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `name` | string | Name identifier for the link. |
| `inertial` | **[Inertial settings](#inertial-settings)** | Inertial settings that affect the rigid body motion of the link. |
| `collision` | **[Collision settings](#collision-settings)** | Collision settings for collision detection and response. |
| `visual` | **[Visual settings](#visual-settings)** | Visual settings for the link's rendered mesh. |

### Inertial settings

The link's inertial settings consist of mass, origin, inertia, and aerodynamic drag.

``` json
"inertial": {
  "origin": {
    "xyz": "0.253 -0.253 -0.01",
    "rpy-deg": "0 0 0"
  },
  "mass": 1.0,
  "inertia": {
    "type": "geometry",
    ...
  },
  "aerodynamics": {
    "drag-coefficient": 0.325,
    "type": "geometry",
    ...
  }
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `origin`: `xyz` | string of 3 floats | Position "X Y Z" of the link relative to the link's parent joint in **meter** units. Defaults to all zero if no `origin` is specified. |
| `origin`: `rpy` | string of 3 floats | Rotation "Roll Pitch Yaw" of the link relative to the link's parent joint in **radian** units. Defaults to all zero if no `origin` is specified. |
| `mass` | float | Mass of this link in SI **kg** units. |
| `inertia`: `type` | `geometry`, `matrix`, `point-mass` | Inertia matrix specification type. |
| `aerodynamics`: `drag-coefficient` | float | The Cd drag coefficient for the link. |
| `aerodynamics`: `type` | `geometry`, `cross-section-areas-xyz` | Aerodynamic drag face specification type. |

**Inertia**

The link's rotational inertia matrix (inertia tensor) is specified assuming symmetry around the principle axes so any off-diagonal components (those other than Ixx, Iyy, Izz) are assumed to be zero.

If specified as a `matrix` type, directly set the diagonal component values:

``` json
"inertia": {
  "type": "matrix",
  "ixx": 1.0,
  "iyy": 1.0,
  "izz": 1.0
}
```

If specified as a `geometry` type (only `box` is currently supported), set the `size` values as "X Y Z" dimensions in SI **meter** units :

``` json
"inertia": {
  "type": "geometry",
  "geometry": {
    "box": {
      "size": "0.180 0.110 0.040"
    }
  }
}
```

If specified as a `point-mass` type, no details are needed because the inertia matrix for this link will be all zero, and the mass will only affect the inertia of other connected links by the Parallel Axis Theorem.

``` json
"inertia": {
  "type": "point-mass"
}
```

**Aerodynamics**

The link's aerodynamic drag is specified as the `drag-coefficient` Cd and cross-sectional face areas on each of the link's principle axes.

If specified as cross-sectional areas type, directly set the face areas in SI **square meter** units for the "X Y Z" axes:

``` json
"aerodynamics": {
  "drag-coefficient": 0.325,
  "type": "cross-section-areas-xyz",
  "cross-section-areas-xyz": "1.0 1.0 1.0"
}
```

If specified as a `geometry` type (`box` or `cylinder` are currently supported), then set the geometry's dimensions and the cross-sectional areas will be calculated automatically.

`box` with "X Y Z" dimensions set in SI **meter** units:
``` json
"aerodynamics": {
  "drag-coefficient": 0.325,
  "type": "geometry",
  "geometry": {
    "box": {
      "size": "0.180 0.110 0.040"
    }
  }
}
```

`cylinder` with `radius` and `length` dimensions set in SI **meter** units:
``` json
"aerodynamics": {
  "drag-coefficient": 0.325,
  "type": "geometry",
  "geometry": {
    "cylinder": {
      "radius": 0.1143,
      "length": 0.01
    }
  }
}
```

### Collision settings

The link's collision settings consist of an enabled flag and parameters for restitution and friction coefficients used in calculating its collision response.

``` json
"collision": {
  "enabled": true,
  "restitution": 0.1,
  "friction": 0.5
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `enabled` | bool | Flag to enable/disable collision detection for this link's **[visual mesh](#visual-settings)**. Disabling collision may be useful for non-physics "computer vision" mode while still being able to see the robot's mesh visually. This defaults to `true` if omitted, but if the link does not specify a visual mesh, collisions cannot be detected so this setting would not be used. |
| `restitution` | float | Restitution coefficient for how the link's velocity will react normal to the collision (0.0 = no bounce, 1.0 = bounce off with equal velocity as impact). |
| `friction` | float | Friction coefficient for how the link's velocity will react tangential to the collision (0.0 = no tangential velocity reduction, 1.0 = full tangential velocity reduction). |

### Visual settings

The link's visual settings consist of what visual element will be rendered for the link. Currently, only `unreal_mesh` geometry types are supported.

The `visual` element can be omitted if no visual mesh is desired, such as for non-physics "computer vision" mode, and this will also effectively disable collision detection since there is no mesh to collide with.

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
| `geometry`: `type` | `unreal_mesh` | The type of mesh geometry to use (currently only `unreal_mesh` is supported). The mesh must be imported into the Unreal environment as a .uasset to be available at runtime. |
| `geometry`: `name` | string | For `unreal_mesh`, this should be the Unreal content path to the mesh in the environment. |
| `geometry`: `scale` | string of 3 floats | Adjust the mesh's scale. Defaults to all 1.0 if not specified. **Note:** A link's scale will also be inherited by any child links to maintain relative size, so their scales may also need adjustment to compensate. |

## Joint settings

A joint is a connection between two links (a single parent link and a single child link) in order to define a relationship for relative motion. Constraints for the joint type can specify the type of joint motion to allow.

``` json
"joints": [
  {
    "id": "Frame_Prop_FL",
    "type": "fixed",
    "parent-link": "Frame",
    "child-link": "Prop_FL",
    "axis": "0 0 1",
    "limit": 1.57,
    "damping-constant": 1
  },
  ...
]
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `id` | string | Name identifier for the joint. |
| `origin`: `xyz` | string of 3 floats | Position "X Y Z" of the joint's child link relative to the joint's parent link in **meter** units. Defaults to all zero if no `origin` is specified. |
| `origin`: `rpy` | string of 3 floats | Rotation "Roll Pitch Yaw" of the joint's child link relative to the joint's parent link in **radian** units. Defaults to all zero if no `origin` is specified. |
| `parent-link` | string | Name identifier for the joint's parent link. |
| `child-link` | string | Name identifier for the joint's child link. |
| `type` | `fixed`, `continuous`, `revolute` | Type of joint constraint. `fixed` means the pose of the `child-link` should stay fixed relative to the `parent-link`. `continuous` means the `child-link` can rotate continously around the `axis` normal. `revolute` means the `child-link` can rotate like a hinge around the `axis` up to an angle `limit`. |
| `axis` | `"1 0 0"` for X axis, `"0 1 0"` for Y axis, `"0 0 1"` for Z axis | Rotation axis for joint motion. |
| `limit` | float | Rotation angle limit in **radians**. |
| `damping-constant` | float | Damping coefficient for rotation motion over time. |

Example of fixed joint for drone propellers using **[Fast Physics](physics/fast_physics.md)** which visually rotates the propeller meshes without using joint physics:

``` json
"joints": [
  {
    "id": "Frame_Prop_FL",
    "type": "fixed",
    "parent-link": "Frame",
    "child-link": "Prop_FL",
    "axis": "0 0 1"
  },
  ...
]
```

{#ifdef INTERNAL_DOCS #}
{# include begin_internal.md #}

Example of continuous rotation joint for drone propellers using **[Unreal Physics](internal/physics/unreal_physics.md)** which rotates the propeller meshes using some applied torque on the propellers:

``` json
"joints": [
  {
    "id": "Frame_Prop_FL",
    "origin": {
      "xyz": "0.253 -0.253 -0.01",
      "rpy-deg": "0 0 0"
    },
    "type": "continuous",
    "parent-link": "Frame",
    "child-link": "Prop_FL",
    "axis": "0 0 1",
    "damping-constant": 1
  },
  ...
]
```

{# include end_internal.md #}

{#endif INTERNAL_DOCS #}
## Controller settings

A controller is used to decide how to command the robot's actuators in order to apply forces and torques on the robot's links. Only a single controller per robot is currently supported.

``` json
"controller": {
  "id": "Simple_Flight_Controller",
  "airframe-setup": "quadrotor-x",
  "type": "simple-flight-api",
  "simple-flight-api-settings": {...}
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `id` | string | Name identifier for the controller. |
| `airframe-setup` | string | Type of airframe. Simple Flight currently supports `quadrotor-x`, `hexarotor-x`, `vtol-quad-x-tailsitter`, and `vtol-quad-tiltrotor`, defaulting to `quadrotor-x` if omitted. PX4 ignores this setting because the airframe type is determined by the PX4 firmware that is running. |
| `type` | string | Type of the controller. Currently, **[`simple-flight-api`](controllers/simple_flight.md)**, **[`px4-api`](controllers/px4/px4.md)**, and `manual-controller-api` are supported. |
| `simple-flight-api-settings` | See [Simple Flight settings](#simple-flight-settings) | Settings specific to the `simple-flight-api` controller type. |
| `px4-settings` | See [PX4 settings](#px4-settings) | Settings specific to the `px4-api` controller type. |
| `manual-controller-api-settings` | See [Manual Controller settings](#manual-controller-settings) | Settings specific to the `manual-controller-api` controller type. |

### Simple Flight settings

The `simple-flight-api-settings` collection contains the settings for the Simple Flight controller:

``` json
"simple-flight-api-settings": {
  "actuator-order": [
    ...
  ]
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `actuator-order` | Array of actuator `id` tags | The actuator name identifiers to link to the controller's commanded outputs.  See [Actuator order settings](#actuator-order-settings) below for the order. |

### PX4 settings

The `px4-settings` collection contains the settings for the PX4 flight controller:

``` json
"px4-settings": {
  "lock-step" : true,
  "use-serial": false,
  "serial-port": "*",
  "serial-baud-rate": 115200,
  "use-tcp": true,
  "tcp-port": 4560,
  "control-ip-address": "127.0.0.1",
  "control-port-local": 14540,
  "control-port-remote": 14540, //Used only when control-ip-address is not the local host
  "qgc-host-ip": "", //Set only when enabling GCS proxy
  "qgc-port": 14550, //Set only when enabling GCS proxy
  "parameters": {
    "NAV_RCL_ACT": 0,
    "NAV_DLL_ACT": 0,
    "COM_OBL_ACT": 1,
    "LPE_LAT": 47.641468,
    "LPE_LON": -122.140165
  },
  "actuator-order": [
    ...
  ]
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `lock-step` | bool | Enables lock-step mode with the PX4 SITL configuration.  When true, Project AirSim and PX4 execute synchronously and can run faster or slower than real time as needed.  When false, Project AirSim and PX4 execute asynchronously in real time and can lost step with each if either cannot run fast enough.
| `use-serial` | bool | When true, Project AirSim connects to a PX4 HITL configuration over a serial port.  Takes precedence over `use-tcp`.
| `serial-port` | string | When `use-serial` is true, specifies the serial port to the PX4 HITL device.  On Windows, this is the COM port (e.g., "COM3").  On Linux, this is the serial port device (e.g., "/dev/ttyS0").  The special value "`*`" causes Project AirSim to search the available serial ports for known Pixhawk hardware devices and use the first one.
| `serial-baud-rate` | integer | When `use-serial` is true, specifies the serial port communication rate to the PX4 HITL device.
| `use-tcp` | bool | When true, Project AirSim connects to a PX4 SITL simulation over TCP/IP.  `use-serial` takes precedence over `use-tcp`.
| `tcp-port` | integer | When `use-tcp` is true, specifies the TCP/IP port where the PX4 SITL simulation will communicate with Project AirSim for simulation messages.  Usually `4560`.
| `local-host-ip` | integer | This setting is used when PX4 SITL is running remotely.  When `use-tcp` is true, it specifies the IP address of the network adapter of the Project AirSim computer where `tcp-port` and `control-port` port will be opened by Project AirSim to communicate with PX4 SITL simulation running remotely.  See [PX4 communication port settings](#px4-communication-port-settings), below.  Usually unassigned or `127.0.0.1`.
| `control-ip-address` | string | When `use-tcp` is true, specifies the IP address where PX4 will connect to Project AirSim for external developer/offboard API messages, usually the IPv4 loopback address `127.0.0.1`.  See [PX4 communication port settings](#px4-communication-port-settings), below.  The values `local` and `localhost` are aliases for `127.0.0.1`.  The value `remote` will use the IP address of the PX4 host connecting to the simulation port, `tcp-port`.
| `control-port` | string | When `use-tcp` is true, specifies the UDP/IP port where PX4 will connect to Project AirSim for external developer/offboard API messages.  When there are multiple PX4-controlled vehicles, the first nine vehicle instances use ports `14540` through `14549` sequentially.  Subsequent vehicle instances share port `14549`.  See [PX4 communication port settings](#px4-communication-port-settings), below.  `control-port-local` is an alias.
| `control-port-remote` | string | When `use-tcp` is true and `control-ip-address` is not the local host, specifies the UDP/IP Project AirSim where Project AirSim will connect for external developer/offboard API messages.  See [PX4 communication port settings](#px4-communication-port-settings), below.  Usually `14580`.
| `qgc-host-ip` | string | When non-empty, specifies the IP address where Project AirSim will connect to the ground control station (such as *QGroundControl*).  See [Ground control station settings](#ground-control-station-settings), below.
| `qgc-port` | integer | When `qgc-host-ip` is non-empty, specifies the UDP/IP port where Project AirSim will connect to the ground control station.  See [Ground control station settings](#ground-control-station-settings), below.  Usually `14550`.
| `actuator-order` | Array of actuator `id` tags | The actuator name identifiers to link to the controller's commanded outputs.  See [Actuator order settings](#actuator-order-settings) below for the order. |

#### PX4 communication port settings

In a PX4 HITL configuration, PX4 communicates with Project AirSim over the MAVLink communications channel (either serial or serial over USB).  PX4 sends control messages such as actuator settings while Project AirSim sends messages such as sensor data updates.

In a PX4 SITL configuration, PX4 communicates with Project AirSim using two separate communications channels: one for simulator messages (mainly sensor data) and another for API/offboard messages (actuator settings and flight control.)  Simulation messages are sent over the TCP/IP port specified by `tcp-port`.  API/offboard messages are sent over the UDP/IP port specified by `control-port`.  This enables PX4 to support additional SITL features such as locked-step mode.

When using multiple vehicles in the simulation, each robot has its own external developer/offboard API instance.  The first nine Project AirSim robot instances configure `control-port` to ports `14590` through `14598` sequentially.  The tenth and subsequent instances sets `control-port` to port `14599` which is shared.  All robots set `tcp-port` to the same value.

Project AirSim supports several different PX4 SITL configurations: on the same computer, different computers, and Windows Subsystem for Linux 2 (WSL2.)  For more information on these configurations, see [Advanced PX4 SITL configurations](controllers/px4/px4_sitl.md#alternate_px4_sitl_configurations).  The following tables show how to set the communication settings for each configuration.

<u>Settings for SITL simulation communications channel</u>:
| SITL Configuration | `tcp-port` | `local-host-ip` |
| ------------------ | ---------- | --------------- |
| Same computer      | Usually `4560` | (Unassigned) |
| Different computer | Usually `4560` | IP address of Project AirSim's network adapter to PX4 host |
| PX4 in WSL2 | Usually `4560` | IP address of Project AirSim's virtual network adapter to WSL2 |

PX4 connects to Project AirSim at TCP/IP port `tcp-port`.

For PX4 in WSL2, the value for `local-host-ip` can be found by running the command `ipconfig` in a Command Prompt window and looking for IP address under "Ethernet adapter vEthernet (WSL)".


<u>Settings for SITL API/offboard communications channel</u>:

| SITL Configuration | `control-port` | `control-port-remote` | `control-ip-address` | Notes |
| ------------------ | -------------- | --------------------- | -------------------- | ----- |
| Same computer       | Usually `14540` | (Unassigned) | `127.0.0.1` |
| Different computer | (Ignored) | Usually `14580` | IP address of PX4 host | Use [MAVLink Router](https://docs.px4.io/master/en/simulation/#running-simulation-on-a-remote-server) running on the PX4 host. |
| PX4 in WSL2 | Usually `14540` | (Unassigned) | `remote` | In WSL2, set environment variable `PX4_SIM_HOST_ADDR` to Project AirSim host IP address.  MAVLink Router is not used.

When running PX4 on the same computer or in WSL2, PX4 connects to Project AirSim at UDP/IP port `control-port`.  When running PX4 remotely, Project AirSim connects to PX4 at the UDP/IP port `control-port-remote`.

#### Ground control station settings

On a physical vehicle, PX4 communicates with a GCS (aka. Ground Control Station such as *QGroundControl*) through a telemetry link (usually via radio).  In a PX4 SITL configuration, the PX4 SITL simulation connects directly to the GCS software over the local computer network.  In a PX4 HITL configuration, only one application can connect to the PX4 device hardware at a time, so the GCS can't connect to the PX4 device when it's connected Project AirSim.  Project AirSim, however, can act as a proxy enabling a GCS to be used with the simulation.

When the GCS application is run on the same computer as Project AirSim and PX4, do the following:

| PX4 Configuration | Setup (ground control station on same computer) |
| ----------------- | ----- |
| HITL | Project AirSim will proxy the GPC.  Set `qgc-host-ip` to `127.0.0.1` (the IPv4 loopback address) and `qgc-port` to the GCS' UDP/IP port (usually `14550`).
| SITL | PX4 will connect to the GCS directly.  Leave `qgc-host-ip` unset or set to an empty string (`qgc-port` will be ignored). |

Project AirSim's GCS proxy also allows the GCS application can be run on a remote computer with both PX4 HITL and SITL configurations:

* Set `qgc-host-ip` to the IP address of the computer running the GCS application.
* Set `qgc-port` to the UDP/IP port of the GCS application (usually `14550`.)
* Remember to configure the network firewall software of Project AirSim's computer to allow outgoing connections to the GCS' UDP/IP port and of the GCS computer to allow incoming connections to the same port.

### Actuator order settings

Under `simple-flight-api-settings` and `px4-settings` this array maps the control outputs from the controller to the robot's actuators.

``` json
"simple-flight-api-settings": {
  "actuator-order": [
    {
      "id": "Prop_FR_actuator"
    },
    {
      "id": "Prop_RL_actuator"
    },
    {
      "id": "Prop_FL_actuator"
    },
    {
      "id": "Prop_RR_actuator"
    }
  ]
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `actuator-order` | Array of actuator `id` tags | The actuator name identifiers to link to the controller's commanded outputs.  See below for the order. |

The order and number of the actuator `id` tags depends on the airframe:

| Project AirSim Airframe | Actuator Order |
| -------- | -------------- |
| `quadrotor-x` | [Quadrotor-X order](https://docs.px4.io/master/en/airframes/airframe_reference.html#quadrotor-x) (front right, rear left, front left, rear right) |
| `hexarotor-x` | [Hexarotor-X order](https://docs.px4.io/master/en/airframes/airframe_reference.html#hexarotor-x) (front right, rear left, front left, rear right) |
| `vtol-quad-x-tailsitter` | \*[VTOL Quad Tailsitter order](https://docs.px4.io/master/en/airframes/airframe_reference.html#vtol-quad-tailsitter) (front right, rear left, front left, rear right, unused output port, elevon left, elevon right)
| `vtol-quad-tiltrotor` | \*[VTOL Tiltrotor order](https://docs.px4.io/master/en/airframes/airframe_reference.html#vtol-tiltrotor) (rotor outboard left, rotor inboard left, rotor inboard right, rotor outboard right, tilt outboard left, tilt inboard left, tilt inboard right, tilt outboard right, aileron left, aileron right, elevator, rudder)

\**Note: The PX4 documentation does not currently show the correct order.*

### Manual Controller settings

The `manual-controller-api-settings` collection contains the settings for the Manual Controller:

```json
"manual-controller-api-settings": {
  "actuator-order": [
    {
      "id": "Prop_FR_actuator",
      "initial-value": 0.0
    },
    {
      "id": "Prop_RL_actuator",
      "initial-value": 0.0
    },
    {
      "id": "Prop_FL_actuator",
      "initial-value": 0.0
    },
    {
      "id": "Prop_RR_actuator",
      "initial-value": 0.0
    }
  ]
}
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `actuator-order` | Array of actuator `id` tags (string) and optional `initial-value` control signal values (float). | The actuator name identifiers to link to the controller's commanded outputs. If omitted, the default `initial-value` is `0.0`. Since there is no actual control logic or corresponding airframe, the order and number of linked actuators can be set freely. |

## Actuator settings

Actuators are used to apply forces and torques to links based on controller commands, and can be attached to parent links.

``` json
"actuators": [
  {
    "name": "Prop_FL_actuator",
    "type": "rotor",
    "enabled": true,
    "parent-link": "Frame",
    "child-link": "Prop_FL",
    "origin": {
      "xyz": "0.253 -0.253 -0.01",
      "rpy-deg": "0 0 0"
    },
    "rotor-settings": {...}
  },
  ...
]
```

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `name` | string | Name identifier for the actuator. |
| `type` | `rotor`, `lift-drag-control-surface`, or `tilt` | Type of actuator.  See below. |
| `enabled` | bool | Enable or disable the actuator without having to delete the config object. |
| `parent-link` | string | Name identifier for the actuator's parent link, which the actuator will apply its forces/torques to. |
| `child-link` | string | Name identifier for the actuator's child link, which is currently only used to visually spin propeller links without using joint physics. |
| `origin`: `xyz` | string of 3 floats | Position "X Y Z" of the actuator's applied forces/torques relative to the parent link's origin in **meter** units. Defaults to all zero if no `origin` is specified. |
| `origin`: `rpy` | string of 3 floats | Rotation "Roll Pitch Yaw" of the actuator's applied forces/torques relative to the parent link's origin in **radian** units. Defaults to all zero if no `origin` is specified. |
| `lift-drag-control-surface-settings` | See below | Settings specific to the lift-draw-control-surface actuator type. |
| `rotor-settings` | See below | Settings specific to the rotor actuator type. |
| `tilt-settings` | See below | Settings specific to the tilt actuator type. |

#### Actuator types

The supported actuator types are:

| Actuator Type | Description |
| ------------- | ----------- |
| `lift-drag-control-surface` | Provides movement for wing-like surfaces such as ailerons and rudders. |
| `rotor` | An actuator that supplies continuous rotation such as the motor for a propeller. |
| `tilt` | An actuator that supplies limited rotation such as the motor for a tilting rotor pod.  Similar to `lift-drag-control-surface` but more flexible (and more complex to configure.) |

The settings for each actuator type is below.

### Lift-drag-control actuator settings:

``` json
"lift-drag-control-surface-settings": {
  "target": "Prop_FL_actuator",
  "rotation-rate": 0.524,
  "smoothing-tc": 0.5
}
```
| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `rotation-rate` | float | Maximum surface rotation from zero (radians.) |
| `smoothing-tc` | float | Smoothing time constant used in a first-order filter to simulate actuator dynamics (inertia, delay). |

### Rotor actuator settings:

``` json
"rotor-settings": {
  "turning-direction": "clock-wise",
  "normal-vector": "0.0 0.0 -1.0",
  "coeff-of-thrust": 0.109919,
  "coeff-of-torque": 0.040164,
  "max-rpm": 6396.667,
  "propeller-diameter": 0.2286,
  "smoothing-tc": 0.005
}
```
| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `turning-direction` | `clock-wise`, `counter-clock-wise` | Rotation direction of the rotor. Clockwise is in the positive right-hand rotation around axis, while counter-clockwise is in the opposite direction. |
| `normal-vector` | string of 3 floats | Normal vector to define the rotation axis of the rotor in NED frame. Currently, only `"0.0 0.0 -1.0"` (straight up) is supported. |
| `coeff-of-thrust` | float | Propeller's coefficient of thrust, CT. |
| `coeff-of-torque` | float | Propeller's coefficient of torque, CP. |
| `max-rpm` | float | Max rotation speed in **rotations-per-minute** units. |
| `propeller-diameter` | float | Propeller diameter in **meter** units, used to calculate the rotor's max thrust and torque. |
| `smoothing-tc` | float | Smoothing time constant used in a first-order filter to simulate actuator dynamics (inertia, delay). |

### Tilt actuator settings:

``` json
"tilt-settings": {
  "target": "Prop_FL_actuator",
  "angle-min": 0.0,
  "angle-max": 1.57,
  "axis": "0.0 -1.0",
  "smoothing-tc": 0.5,
  "input-map":{...}
}
```
| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `axis` | string of 3 floats | Normal vector to define the rotation axis of the target actuator in NED frame. |
| `angle-min` | float | Tilt angle at minimum control input, -1.0 (radians.) |
| `angle-max` | float | Tilt angle at maximum control input, +1.0 (radians.) |
| `input-map` | See below | Settings for mapping the control input. |
| `target` | string | ID of the target actuator being tilted by this actuator. |
| `smoothing-tc` | float | Smoothing time constant used in a first-order filter to simulate actuator dynamics (inertia, delay). |

Unlike the `lift-drag-control-surface` actuator, the `tilt` actuator sets the orientation of the `target` actuator in addition to rotating the geometry of the `child-link` object to handle situations where the child-link geometry is not the same as that of the target actuator.

### Input-map settings

``` json
"input-map": {
  "clamp-input": true,
  "clamp-output": true,
  "input-min": -1.0,
  "input-max": 1.0,
  "output-min": -1.0,
  "output-max": 1.0,
  "scale": 1.0,
}
```
| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `clamp-input` | bool | Whether the input control signal is first clamped to the input range (limited to a value at or between `input-min` and `input-max`). |
| `clamp-output` | bool | Whether the output control signal is clamped to the output range. |
| `input-min` | float | Minimum input range value. |
| `input-max` | float | Maximum input range value. |
| `scale` | float | Scale factor to apply to normalized input value. |
| `output-min` | float | Minimum output range value. |
| `output-max` | float | Maximum output range value. |

Different flight controllers and control mixers may define the control signal for a particular actuator differently such as the range of the signal (e.g., [-1, +1] vs. [0, +1]) or polarity (-1 to +1 is clockwise vs. counter-clockwise.)  While generally the actuator settings can be chosen to account for many of these differences, some situations may require more manipulation than those settings allow.

The input map setting linearly maps the input control signal sent to the actuator to an output control signal processed by the actuator.  By setting the input range and output range, the input from the flight controller is mapped to the range preferred by the actuator.  The scale setting magnifies or diminishes the input signal and a negative scale setting inverts the input signal.  Finally, the `clamp-input` and `clamp-output` setting determine whether the input signal is clamped to the input range before processing and whether the output signal is clamped to the output range after processing.

The signal processed by the actuator from the control input signal is calculated by the input map as follows:
```
input-value = control-input clamped to [input-min, input-max] if clamp-input is true; control-input otherwise

output-value = (input_value - input-min) / (input-max - input-min) * scale * (output-max - output-min) + output_min

actuator-signal = output-value clamped to [output-min, output-max] if clamp-output is true; output-value otherwise
```

The default settings shown in the JSON example at the top of this section is a 1:1 mapping of the input to the output (no change) with the output signal clamped to the range [-1, +1].

## Sensor settings

Sensors can be attached to parent links to measure data about the environment from the link's position or about the link itself.

``` json
"sensors": [
  {
    "id": "DownCamera",
    "type": "camera",
    "enabled": true,
    "parent-link": "Frame",
    ...
  },
  ...
]
```
| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `id` | string | Name identifier for the sensor. |
| `type` | `airspeed`, `barometer`, `camera`, `gps`, `imu`, `lidar`, `magnetometer`, `radar` | Type of sensor. |
| `enabled` | bool | Enable or disable the sensor without having to delete the config object. |
| `parent-link` | string | Name identifier for the sensor's parent link, which the sensor will be attached to. |

For each sensor type's specific settings, see the sensor pages:

- Airspeed Settings
- Barometer Settings
- **[Camera Settings](sensors/camera_capture_settings.md)**
- GPS Settings
- IMU Settings
- **[Lidar Settings](sensors/lidar.md)**
- Magnetometer Settings
- **[Radar Settings](sensors/radar.md)**

---

Copyright (C) Microsoft Corporation. 
Copyright (c) 2025 IAMAI Simulations.

MIT License. All rights reserved.
