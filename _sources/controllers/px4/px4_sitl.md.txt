# Using a PX4 Controller as Software-In-The-Loop (SITL)

In SITL, the PX4 flight controller software is running on the same computer as Project AirSim or on another computer on the local network.

See [Supported Versions of PX4](px4.md#supported-versions-of-px4) for versions of PX4 supported by Project AirSim in the SITL configuration.

## Supported airframes

All Project AirSim airframes are supported when using PX4 SITL.  See [Starting a New Flight Session](#starting-a-new-flight-session) for how to launch PX4 for a specific airframe.

## Setting up PX4 Software-In-The-Loop

For manual flight simulation, you will also need a controller supported by your operating system such as an XBox game controller (see [Remote Controller](#remote-controller), below.)

The [PX4 software](http://dev.px4.io) provides a "software-in-loop" simulation (SITL) version of their stack that runs in natively in Linux. If you are on Windows then you can use the [Cygwin Toolchain](https://docs.px4.io/master/en/setup/dev_env_windows_cygwin.html).

**Note:** Whenever you stop the Project AirSim Unreal application you must restart PX4 before you can begin another flight session with Project AirSim and PX4.  See [Starting a New Flight Session](#starting-a-new-flight-session), below.

### Setup the PX4 build environment for Windows

Before setting up PX4, ensure that the Project AirSim [Client Setup](/client_setup.md) has been completed first.

On Windows, install the [Cygwin Toolchain](https://docs.px4.io/master/en/setup/dev_env_windows_cygwin.html).

1. Follow the steps through cloning the repository.  You may skip the example step to run JMAVSim.
2. In the PX4 console window (started by `run-console.bat`), navigate into the PX4-Autopilot repo directory if not there already:

        cd PX4-Autopilot

3. Find the latest stable release from [https://github.com/PX4/PX4-Autopilot/releases](https://github.com/PX4/PX4-Autopilot/releases)
    and checkout the source code matching that release, for example:

        git checkout v1.12.3

4. Build PX4 for the first time with the command:

        make px4_sitl none_iris

    Note that this may take a while.  PX4 will automatically start at the end of the build.


Proceed to [The First Flight Session](#the-first-a-flight-session), below.

### Setup the PX4 build environment for Linux

Before setting up PX4, ensure that the Project AirSim [Client Setup](/client_setup.md) has been completed first.

For reference, the official setup steps for PX4 on Linux [are here.](https://docs.px4.io/master/en/dev_setup/dev_env_linux_ubuntu.html)  Follow the instructions for [Gazebo, JMAVSim and NuttX (Pixhawk) Targets](https://docs.px4.io/master/en/dev_setup/dev_env_linux_ubuntu.html#gazebo-jmavsim-and-nuttx-pixhawk-targets).

The following steps are the Linux-relevant parts from [our own version of the PX4 build instructions](px4_build.md) which is a bit more concise about what we need exactly:

1. Open a bash terminal
2. Get the PX4 source code:

        mkdir -p PX4
        cd PX4
        git clone https://github.com/PX4/PX4-Autopilot.git --recursive
        bash ./PX4-Autopilot/Tools/setup/ubuntu.sh --no-nuttx --no-sim-tools
        cd PX4-Autopilot

3. Find the latest stable release from [https://github.com/PX4/PX4-Autopilot/releases](https://github.com/PX4/PX4-Autopilot/releases)
    and checkout the source code matching that release, for example:

        git checkout v1.12.3

4. Build PX4 for the first time with the command:

        make px4_sitl none_iris

    Note that this may take a while.  PX4 will automatically start at the end of the build.

Proceed to [The first flight session](#the-first-a-flight-session), below.

## The first flight session
1. You should see a message saying the SITL PX4 app is waiting for the simulator (Project AirSim) to connect.  You will also see information about which ports are configured for a MAVLink connection to the PX4 app.

        INFO  [simulator] Waiting for simulator to connect on TCP port 4560
        INFO  [init] Mixer: etc/mixers/quad_w.main.mix on /dev/pwm_output0
        INFO  [mavlink] mode: Normal, data rate: 4000000 B/s on udp port 14570 remote port 14550
        INFO  [mavlink] mode: Onboard, data rate: 4000000 B/s on udp port 14580 remote port 14540

    **Note:** This is also an interactive PX4 console, type `help` to see the
    list of commands you can enter here.  They are mostly low level PX4
    commands, but some of them can be useful for debugging.

    **Note:** In recent versions of PX4, the console may not respond until after the simulator has connected.  Pressing the Enter key should display a "pxh>" prompt when the console is ready to accept commands.

2. Now edit [the robot configuration settings](/config_robot.md) file to make sure you have matching UDP and TCP port settings:

        {
            ...
            "controller": {
                "id": "PX4_Controller",
                "type": "px4-api",
                "px4-settings": {
                    "lock-step": true,
                    "use-tcp": true,
                    "tcp-port": 4560,
                    "control-ip-address": "127.0.0.1",
                    "control-port": 14540,
                    "parameters": {
                        "NAV_RCL_ACT": 0,
                        "NAV_DLL_ACT": 0,
                        "COM_OBL_ACT": 1,
                        "LPE_LAT": 47.641468,
                        "LPE_LON": -122.140165
                    }
                }
            },
            ...
        }

    Notice that the PX4 simulator is using the TCP networking protocol, so we must set `"use-tcp"` to `true`.  We are also enabling `lock-step` mode (see [PX4 LockStep](px4_lockstep.md) for more information.)

3. In your network firewall software, open incoming TCP port 4560 and incoming UDP port 14540.
4. Run the Project AirSim Unreal application.  It should start with an empty Blocks environment.
5. If not started already, start a command line window and activate the Project AirSim Python client environment (see [Project AirSim Client Setup](/client_setup.md).)
6. In the client command line window, navigate to the `projectairsim_client\ProjectAirSim_Example_User_Scripts` directory and run the command:

        python hello_drone.py

    Project AirSim should connect to the PX4 SITL simulation via TCP. You should see a bunch of messages from the SITL PX4 console window. Specifically, the following messages tell you that Project AirSim is connected properly and that GPS fusion is stable:

        INFO  [simulator] Simulator connected on UDP port 14560
        INFO  [mavlink] partner IP: 127.0.0.1
        INFO  [ecl/EKF] EKF GPS checks passed (WGS-84 origin set)
        INFO  [ecl/EKF] EKF commencing GPS fusion

    If you do not see these messages then check your port settings.

7. [*QGroundControl*](http://qgroundcontrol.com/) can be used with PX4 SITL.  Make sure there are no PX4 devices plugged in otherwise *QGroundControl* will connect to that instead.

    Manual flight requires a user input device, but note that we don't have a physical PX4 device to which a remote control (RC) can be directly connected.  The alternatives are to use a) an Xbox 360 controller connected to your computer, b) an RC connected to your computer over USB (as supported by the FrSky Taranis X9D Plus, for example), or c) an RC connected to your computer using a trainer USB cable.  An RC connected to your computer using method b) or c) will look like a game controller. You will need to do extra set up in *QGroundControl* to use a virtual joystick for RC control.

    You do not need to do this unless you plan to fly a drone manually in Project AirSim.  Autonomous flight using the Python API does not require an RC (see [No Remote Control](#no-remote-control), below.)

## Setting the GPS origin

These settings are specified in the `parameters` section of the sample robot configuration, `robot_quadrotor_fastphysics.jsonc`:
```json
    "LPE_LAT": 47.641468,
    "LPE_LON": -122.140165,
```

PX4 in SITL mode needs to be configured with the correct home location.
The home location needs to be set to the same coordinates as [home-geo-point](/config_scene.md#main-scene-configuration).

You can run the following commands in the SITL PX4 console window to check
whether these values are set correctly.

```shell
param show LPE_LAT
param show LPE_LON
```

## Smooth offboard transitions

This setting is specified in the `parameters` section of the sample robot configuration, `robot_quadrotor_fastphysics.jsonc`:

```json
    "COM_OBL_ACT": 1
```

This tells the drone to automatically hover after each offboard control command finishes (the default setting is to land).  Hovering makes for a smoother transition between multiple offboard commands.  You can check this setting by running the following PX4 console command:

```shell
param show COM_OBL_ACT
```

## Check the home position

If you are using DroneShell to execute commands (e.g., arm, takeoff, etc.), you must wait until after the home position is set before issuing commands. You will see these messages appear in the PX4 SITL console when the home position is set:

    INFO  [commander] home: 47.6414680, -122.1401672, 119.99
    INFO  [tone_alarm] home_set

After these messages appear, the DroneShell `pos` command should report this position and commands should be accepted by PX4.  If you attempt to takeoff without a home position you will see the message:

    WARN  [commander] Takeoff denied, disarm and re-try

After the home position is set, check the local position reported by the `pos` command:

    Local position: x=-0.0326988, y=0.00656854, z=5.48506

If the Z coordinate value is large, like in this example, then takeoff might not work as expected.  Try Restarting PX4 and Project AirSim.

## No remote control

These settings are specified in the `parameters` section of the sample robot configuration, `robot_quadrotor_fastphysics.jsonc`:

    "NAV_RCL_ACT": 0,
    "NAV_DLL_ACT": 0,

These parmeters are required if you plan to fly the SITL mode PX4 with no remote control (using only Python scripts, for example.)  These parameters stop the PX4 from entering "failsafe mode" every time a move command is completed.  You can use the following PX4 command to verify that these values are set correctly:

```shell
param show NAV_RCL_ACT
param show NAV_DLL_ACT
```

**WARNING:** ***DO NOT*** do this on a real drone as it is too dangerous to fly without these failsafe measures enabled.

You can also run the following commands in the PX4 console to set these parameters manually:

```shell
param set NAV_RCL_ACT 0
param set NAV_DLL_ACT 0
```


## Using PX4 Mavlink based Gimbal
To use Gimbal with Mavlink support from PX4. You need to configure a couple of things.
1. Enable PX4 Gimbal using the following parameteres in the `parameters` section of the sample robot configuration,
`robot_quadrotor_px4_gimbal.jsonc`:

        "MNT_MODE_IN": 4,
        "MNT_MODE_OUT": 2

2. Configure a gimbal (which is a type of an actuator with basic configuration as follows):

```json
    {
      "name": "Gimbal_Chase_Actuator",
      "type": "gimbal",
      "enabled": true,
      "parent-link": "Frame",
      "child-link": "Prop_RR",
      "origin": {
        "xyz": "-10.0 0.0 -1.0",
        "rpy-deg": "0 -11.46 0"
      }
    }
```
3. Then link this with the appropriate camera by adding gimbal section of the camera.

```json
{
      "id": "Chase",
      "type": "camera",
      "enabled": true,
      "parent-link": "Frame",
      "capture-interval": 0.03,
      "capture-settings": [
        {
          "image-type": 0,
          "width": 1280,
          "height": 720,
          "fov-degrees": 90,
          "capture-enabled": false,
          "streaming-enabled": true,
          "pixels-as-float": false,
          "compress": false,
          "target-gamma": 2.5
        }
      ],
      "gimbal": {
        "gimbal-id": "Gimbal_Chase_Actuator",
        "lock-roll": true,
        "lock-pitch": true,
        "lock-yaw": false
      },
      ...
      ...
```

## Ending a flight session

To end a flight session:

1. Stop the Python client script, if any.  On Windows, you may need to press `Ctrl+Break` instead of `Ctrl+C`.
2. Stop the Project AirSim simulation.  If Project AirSim is running as a stand-alone executable, close the Project AirSim application window.
3. Stop the PX4 SITL simulation by running the PX4 console command `shutdown`.  If PX4 is still running the startup script and the PX4 console is not responding to commands, press `Ctrl+C`.

## Starting a new flight session
**Note:** Before starting a new flight session, first stop any instance of PX4 SITL that was connected to a simulation (such as Project AirSim.)  Once connected to any simulation (not just Project AirSim), a PX4 SITL instance will not reconnect another simulation instance and will tie up the PX4 SITL network ports, interfering with a new flight session.

To start a flight session, start PX4 by running the same command to build it:

1. If not already open, open a command line window (on Windows, the PX4 console window; on Linux, a bash terminal).
2. If not already there, navigate to the `PX4-Autopilot` repo directory.

        cd PX4-Autopilot

3. Use following command to build and start PX4 firmware in SITL mode:

        make px4_sitl none_airframe

    where `airframe` is one of the following:

    Project AirSim Airframe | `airframe` String
    --------------------- | --------------------
    Quadrotor | `iris`
    VTOL Fixed-Wing Tailsitter | `tailsitter`

    For instance to run PX4 SITL for the VTOL fixed-wing tailsitter airframe, run the following command:

        make px4_sitl none_tailsitter

## Remote controller

*QGroundControl* enables manual flight in Project AirSim using a remote control or joystick-like game controller (such as an Xbox game controller) connected to your PC.  See [*QGroundControl* Joystick Setup](https://docs.qgroundcontrol.com/master/en/SetupView/Joystick.html) for more information.

## Advanced PX4 SITL configurations

PX4 SITL is usually run on the same computer as Project AirSim with one instance of each, but Project AirSim and PX4 support more complex configurations.

### Multiple robots with PX4
See [Multi-vehicle simulation with PX4](px4_multi_vehicle.md).

### Running PX4 remotely

PX4 SITL is usually run on the same computer as Project AirSim but it is possible to run PX4 on separate computer.  See [Running Simulation on a Remote Server](https://docs.px4.io/v1.testing/en/simulation/#running-simulation-on-a-remote-server) in the PX4 documentation.

In Project AirSim, put the IP address of the remote computer running PX4 into the `control-ip-address` parameter of the `controller/px4-settings` section in the [robot configuration settings](../../config_robot.md#PX4_communication_port_settings).

### Running PX4 in Windows Subsystem for Linux version 2 (WSL 2)

See [PX4 Software-in-the-Loop with WSL 2](px4_sitl_wsl2.md).

### Network firewall configuration

When running PX4 remotely or in WSL2, you'll need configure your network firewall appropriately.

For both configurations:

* On the Project AirSim computer, allow incoming connections to the TCP/IP port `tcp-port`.


For PX4 running remotely:

* On the Project AirSim computer, allow outgoing connections to the UDP/IP port `control-port-remote`.
* On the PX4 computer, allow incoming connections to the UDP/IP port `control-port-remote`.

For PX4 in WSL2:

* On the Project AirSim computer, allow incoming connections to the UDP/IP port `control-port`.


---

Copyright (C) Microsoft Corporation.  All rights reserved.
