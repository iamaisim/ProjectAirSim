# ArduPilot Flight Controller

[ArduPilot](https://ardupilot.org/) is a flexible open-source autopilot that can run with Project AirSim through Software-In-The-Loop (SITL). The repository includes quadrotor and hexarotor examples in [`client/python/example_user_scripts/ardupilot`](../../client/python/example_user_scripts/ardupilot).

**Compatibility note:** this integration has not been recently validated against the latest ArduPilot release. Start with the supplied examples and their parameter files before adapting it to a different vehicle.

## Supported airframes and control

Project AirSim includes these ArduPilot SITL examples:

| Airframe | Scene configuration | Client script | Parameter file |
| --- | --- | --- | --- |
| Quadrotor | `sim_config/scene_ardu_quadrotor.jsonc` | `ardupilot_quadrotor.py` | `project-airsim-quad.param` |
| Hexarotor | `sim_config/scene_ardu_hexarotor.jsonc` | `ardupilot_hexarotor.py` | `project-airsim-hexa.param` |

The examples display camera streams and load the scene; Project AirSim Client API flight commands such as arm, takeoff, and move are not supported with `ardupilot-api`. Control the vehicle from the ArduPilot console or through a ground control station (GCS), for example Mission Planner or QGroundControl.

## ArduPilot settings

Set the robot controller type to `ardupilot-api` and configure `ardupilot-settings`:

```json
"controller": {
  "id": "ArduPilot_Controller",
  "airframe-setup": "hexarotor-x",
  "type": "ardupilot-api",
  "ardupilot-settings": {
    "local-host-ip": "127.0.0.1",
    "ardupilot-ip": "127.0.0.1",
    "ardupilot-udp-port": 9003,
    "local-host-udp-port": 9002,
    "use-distance-sensor": true,
    "actuator-order": [
      { "id": "Prop_2_actuator" }
    ]
  }
}
```

| Parameter | Type | Description |
| --- | --- | --- |
| `local-host-ip` | string | IP address of the Project AirSim host. |
| `ardupilot-ip` | string | IP address of the ArduPilot host. |
| `ardupilot-udp-port` | integer | UDP port on the ArduPilot host. The examples use `9003`. |
| `local-host-udp-port` | integer | UDP port on the Project AirSim host. The examples use `9002`. |
| `use-distance-sensor` | boolean | Sends distance-sensor data to ArduPilot when enabled. |
| `actuator-order` | array | Ordered actuator identifiers connected to the ArduPilot outputs. |

ArduPilot and Project AirSim exchange simulator messages, including sensor and actuator data, over UDP/IP. The Project AirSim receive port is `local-host-udp-port`.

### Network configurations

| Configuration | `ardupilot-ip` | `local-host-ip` |
| --- | --- | --- |
| Same Linux computer | `127.0.0.1` | `127.0.0.1` |
| Separate computers | ArduPilot host IP | Project AirSim host IP |
| ArduPilot in WSL1 | `127.0.0.1` | `127.0.0.1` |
| ArduPilot in WSL2 | WSL virtual adapter IP | WSL virtual adapter IP |

For WSL2, run `ipconfig` on Windows and use the IPv4 address of the WSL virtual network adapter in both settings. When using separate hosts or WSL, configure firewalls to allow the required UDP traffic. A GCS on another host also needs outgoing access from the Project AirSim host and incoming access on its configured UDP port.

## Install and run ArduPilot SITL

Complete the Project AirSim installation and [client setup](../client_setup.md), then prepare ArduPilot on a Linux host, VM, WSL1, or WSL2. The standard ArduPilot prerequisites may change, so consult the [ArduPilot build documentation](https://ardupilot.org/dev/docs/building-setup-linux.html) if the setup script reports an error.

```bash
git clone --recursive https://github.com/ArduPilot/ardupilot.git
cd ardupilot
Tools/environment_install/install-prereqs-ubuntu.sh -y
```

Copy the matching parameter file from the Project AirSim examples directory into ArduPilot's `Tools/autotest` directory. For a hexarotor, copy `project-airsim-hexa.param`; for a quadrotor, copy `project-airsim-quad.param`.

Start the hexarotor SITL instance from the ArduPilot repository:

```bash
cd Tools/autotest
python sim_vehicle.py -v ArduCopter -f airsim-copter --add-param-file=project-airsim-hexa.param
```

When ArduPilot runs in WSL2 or on a different computer, append `--sim-address=<project-airsim-host-ip>` to that command.

### WSL1 display setup

If you use an X Windows application in WSL1, install and start an X server such as VcXsrv. In XLaunch, select **Multiple windows**, then **Start no client**, and enable **Clipboard** without Native OpenGL. Set `DISPLAY` in WSL; the following persistent configuration uses the Windows-side nameserver address:

```bash
export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0
```

If the X client cannot connect, enable **Disable access control** in the X server only when appropriate for your environment.

## Run an example flight session

1. Start ArduPilot SITL with the matching parameter file.
2. In a terminal with the Project AirSim Python client environment activated, change to `client/python/example_user_scripts/ardupilot`.
3. Run `python ardupilot_hexarotor.py` or `python ardupilot_quadrotor.py`.
4. Control the vehicle from the ArduPilot console or a GCS.

For a local Linux host or WSL1, the checked-in configurations already use loopback addresses. For WSL2 or separate machines, update `local-host-ip` and `ardupilot-ip` in the matching `sim_config/robot_ardu_*.jsonc` file before running the example.

Always stop Project AirSim before stopping ArduPilot. After Project AirSim stops, restart ArduPilot before beginning another session.

### Useful ArduPilot console commands

| Command | Notes |
| --- | --- |
| `arm throttle` | Arms the vehicle. |
| `mode guided` | Wait until the console reports that EKF3 is using GPS before switching to Guided mode. |
| `takeoff <alt>` | Takes off to the requested altitude in metres. Wait for completion before the next command. |
| `guided <lat> <lon> <alt>` | Flies to latitude/longitude in degrees and altitude in metres. Wait for the waypoint to be reached. |
| `mode land` | Lands the vehicle. |

If `AP: Arm: Main loop slow...` appears, try `arm throttle` again. If it persists, review the simulation clock settings such as `step-ns`.

## Autotune a different vehicle

Vehicles other than the supplied examples should be autotuned for stable flight. Autotune can take hours.

1. Run `arm throttle`.
2. Run `rc3 1800` and wait until the vehicle is clear of obstacles.
3. Run `rc3 1500`.
4. Run `mode autotune`.

ArduPilot reports the tuned roll, pitch, and yaw values in its console. Copy them to the corresponding `ATC_RAT_*`, `ATC_ANG_*`, and `ATC_ACCEL_*` entries in a `.param` file, then start SITL with `--add-param-file=<file>`. The supplied `project-airsim-hexa.param` and `project-airsim-quad.param` files are reference examples.

It is safe to ignore an `AP: Auto-Tune: failing to level` message if it appears during the procedure.

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI CONSULTING CORP

MIT License. All rights reserved.
