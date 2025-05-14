# Simple Flight Controller for Drones
{# include enable_internal_docs.tpp #}

The Simple Flight flight controller built into Project AirSim enables you to get your drone flying quickly and easily.

Simple Flight currently supports quadrotor, hexarotor, VTOL quad-x tailsitter, and VTOL quad tiltrotor airframes.

## Setup

To use Simple Flight as the flight controller for a robot, in the robot config file under the `controller` section set the `type` parameter to `simple-flight-api`.

For additional Simple Flight settings, see [Simple Flight settings in Robot config](../config_robot.md#simple-flight-settings).

## Usage

To use Simple Flight to fly a robot programmatically with a Python mission script:
1. Enable API control by calling `projectairsim.Drone.enable_api_control()`.  This enables Client API functions to command the vehicle through Simple Flight.
2. Call `projectairsim.Drone.arm()` to ready the vehicle for take-off.
3. Call `projectairsim.Drone.takeoff_async()` to launch the vehicle from the ground.
4. Call Client API functions such as `projectairsim.Drone.move_by_velocity_async()` to fly the vehicle.
5. To land, call `projectairsim.Drone.land_async()`.
6. (Optional) Disarm the vehicle with `projectairsim.Drone.disarm()`
7. (Optional) Disable API control with `projectairsim.Drone.disable_api_control()`

To use Simple Flight to fly a robot manually with a manual controller like an Xbox game controller, see [Using an Xbox controller](#using-an-xbox-game-controller), below.

{# ifdef INTERNAL_DOCS #}
***NOTE: THE FOLLOWING PARAGRAPH AND TABLE IS PRERELEASE.***

The param `allow_api_always` is currently not yet configurable through a config file.  It is always true thus "RC enable API control" will currently never have an effect.

Project AirSim currently only supports one client connected at a time, so a separate mission script cannot run simultaneously with the sample Xbox RC controller script.  A combined script that shares a single ProjectAirSimClient object with both scripts, however, would allow both to run simultaneously.  At any point in time, the vehicle can be controlled by either a mission script programmatically or by the RC controller manually, but not both.  The following table shows when each method has control of the vehicle:

| Condition | RC | RC | RC | Prog | Prog | Prog | None |
| --------- | -- | -- | -- | ---- | ---- | ---- | ---- |
| RC controller connected | Yes | Yes | Yes | Yes | Yes | No | No |
| Param `allow_api_always` | No | No | Yes | No | Yes | &mdash; | &mdash; |
| RC enable API control | No | Yes | &mdash; | Yes | &mdash; | &mdash; | &mdash; |
| Client API enable_api_control | &mdash; | No | No | Yes | Yes | Yes | No |

A dash ("&mdash;") means it doesn't matter whether the condition is in effect or not.

***END PRERELEASE.***
{# endif INTERNAL_DOCS #}

## Using an Xbox controller

Simple Flight supports using a PC game controller as a manual remote control input device as if it were a remote control transmitter in a radio link to the flight controller on the vehicle.  Run the example script `client\python\example_user_scripts\xbox_rc.py` to use an Xbox game controller connected to the PC as an RC controller for Simple Flight.  By default, the Xbox controls are mapped to the following Simple Flight functions:

| Xbox Control | Simple Flight Function | Notes |
| ------------ | -----------------------| ----- |
| Left joystick vertical | Throttle | Fully down is no throttle, fully up is full throttle.
| Left joystick horizontal | X-roll axis | Vehicle roll level or rate |
| Right joystick vertical | Y-pitch axis | Vehicle pitch level or rate |
| Right joystick horizontal | Z-yaw axis | Vehicle yaw level or rate |
| Start button | Arm vehicle | Throttle must be fully down and button pressed for at least 100 ms |
| Back button | Disarm vehicle | Throttle must be fully down and button pressed for at least 100 ms |
| X button | Toggle API control enable | Enables or disables programmatic control of the vehicle through the Client API |
| Y button | Toggle between angle level and rate control | With angle level control (the default), the axis controls set the axis angles.  With angle rate control, the axis controls set the axis rotation rates.

### Command-line arguments
The `xbox_rc.py` script takes the following arguments:

|  Switch  | Value | Default | Description |
| -------- | ------| --------| ----------- |
| <code>&#x2011;&#x2011;address</code> | string | 127.0.0.1 | The IP address of the host running Project AirSim (e.g., <code>&#x2011;&#x2011;address=127.0.0.1</code>). |
| <code>&#x2011;&#x2011;rcconfigfile</code> | string | "xbox_rc_config.jsonc" | The name of the RC  config file loaded to setup the mapping from the PC controller to the Simple Flight RC input.  The file is relative to the current directory. |
| <code>&#x2011;&#x2011;sceneconfigfile</code> | string | "scene_basic_drone.jsonc" | The name of the Project AirSim scene config file loaded to setup the simulation.  This file is loaded from the sim config path directory. |
| <code>&#x2011;&#x2011;simconfigpath</code> | string | "sim_config/" | The path to the directory containing the Project AirSim config files.  The path is relative to the current working directory. |
| <code>&#x2011;&#x2011;servicesport</code> | integer | 8990 | The Project AirSim services TCP/IP port.  This can be changed by the [-servicesport](../command_line_switches.md#command_line_switches) command-line switch to Project AirSim.
| <code>&#x2011;&#x2011;topicsport</code> | integer | 8989 | The Project AirSim pub-sub TCP/IP port.  This can be changed by the [-topicsport](../command_line_switches.md#command_line_switches) command-line switch to Project AirSim.

### Xbox controller configuration

The RC configuration file has the following overall structure:
``` json
{
  "channel_map": {
    "0": {...},
    "1": {...},
    ...
  },
  "channel_map_oob": {
    "arm_vehicle": {...},
    ...
  }
}
```

The `channel_map` section contains a channel entry for each RC input channel to the flight controller, labeled "0", "1", etc.  The `channel_map_oob` section is similar but each channel entry is labeled with the name of out-of-band (OOB) function.  See [Simple Flight-specific configuration](#simple-flight-specific-configuration), below, for the function of each RC input channel and the OOB functions supported by Simple Flight.

Each channel entry has the following structure:
``` json
{
  "input_channel": "xLeft",
  "input_range": {
    "min": -27500.0,
    "max": 27500.0
    },
  "input_dead_range": {
    "min": -5000.0,
    "max": 5000.0
    },
  "output_range": {
    "min": -1.0,
    "max": 1.0
    }
}
```
with the following parameters:

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `input_channel` | string | The name of input controller channel from which to get the value for this RC channel. |
| `input_range` | range | Expected range of native values from the input controller channel.  They correspond to the `output_range` values. |
| `input_dead_range` | range | The range of native values from the input controller channel that are mapped to 0.0.  This compensates for controls resting at a non-zero value when released. |
| `output_range` | range | The range of values output to the flight controller or OOB function.  They correspond to the `input_range` values. |
| `max` | float | The maximum value of the range. |
| `min` | float | The minimum value of the range. |


Most RC input channels use the range [-1..+1], but there are exceptions such as the Simple Flight throttle channel which uses the range [0..+1].

Note that the output to the flight controller can be inverted with respect to the input from the input controller by swapping the `output_range` `min` and `max` values.  For instance, the sample `xbox_rc_config.jsonc` file has for channel 3 (the Y-pitch axis) set `output_range.min` to `1.0` and `output_range.max` to `-1.0`, opposite of the norm.  This causes channel 3 to be +1.0 when the right joystick is pull all the way down and reporting a value of -32768, and channel 3 to be -1.0 when the right joystick is pushed all the way up and reporting a value of 32767.


### Simple Flight-specific configuration
Simple Flight's RC input channel assigments are detailed in the [RC input channels](#rc-input-channels) section, below.  The `SimpleFlightRC` class (see [Using other game controllers](#using-other-game-controllers), below) also supports the following OOB functions.  The OOB function name is used as the label for a channel entry in the `channel_map_oob` section.

| OOB Function | Values | Description |
| ------------ | :----: | ----------- |
| <code>arm_vehicle</code> | 0.0, 1.0 | Request to arm the vehicle.  This is a convenience feature so that the user need not put the PC input controls in just the right positions to satisfy the special pattern (see [RC input channels](#rc-input-channels), below.)  Must be asserted for at least 100 ms and the throttle channel must be less than 0.1.
| <code>disarm_vehicle</code> | 0.0, 1.0 | Request to disarm the vehicle.  This is a convenience feature so that the user need not put the PC input controls in just the right positions to satisfy the special pattern (see [RC input channels](#rc-input-channels), below.)  Must be asserted for at least 100 ms and the throttle channel must be less than 0.1.

## Using other game controllers

The `xbox_rc.py` Python script uses the `projectairsim.rc.XboxInputControllerSF` class to read the state of the Xbox game controller.  This class uses the [inputs library](https://pypi.org/project/inputs/) to read input controllers in a platform-agnostic way.  The input controller state is then passed to the `projectairsim.rc.SimpleFlightRC` class which, based on the RC config data, retrieves the individual control values assigned to each channel, converts the values, and pushes them to the Simple Flight flight controller in Project AirSim.  See the comments within each class for more detailed information.

The minimum steps to adapt a different PC controller are as follows:
1. Create a new class inheriting from `projectairsim.rc.BaseInputController` and implement the `read()` method.
2. Create a configuration file that maps the controls from the input controller class to the Simple Flight RC channels.
3. Create a copy of `xbox_rc.py` that instantiates your input controller class instead of `XBoxInputControllerSF`.

The `BaseInputController.read()` method returns a dictionary of control names and their numeric values.  See `projectairsim.rc.XboxInputController` for how this is done for the Xbox game controller using the `inputs` library.  The control names should clearly identify the control but can be anything appropriate since they are only used in the RC config to map the control values from the input controller to the RC input channels.

Some flight controller channels prefer to stay "on" or "off" like a switch and not temporarily "on" like a button, but the typical PC input controller may not have any switches.  The `projectairsim.rc.VirtualToggleSwitch` class can be used to turn a button into a toggle switch control that toggles between "on" and "off" each time the button is pressed.  The `projectairsim.rc.XboxInputControllerSF` class extends the `projectairsim.rc.XboxInputController` class by using the `VirtualToggleSwitch` class to add two virtual toggle switch controls.

## Simple Flight API

Simple Flight supports the following Project AirSim topics via the pub-sub client API.

### Subscribed topics:

<code>/Sim/*&lt;scene_name&gt;*/robots/*&lt;robot_name&gt;*/simple_flight/rc_input</code>
[(FlightControlRCInputMessage)](#flightcontrolrcinputmessage)
<ul><li style="list-style-type: none;">A client publishes messages to this topic to act like a standard remote control radio link.  The message contains an array of "channels" which control various functions of Simple Flight.  See <a href="#rc-input-channels">RC input channels</a> for more information.  When the message contains fewer channels than expected, Simple Flight leaves the missing channels with their previous values (initially 0.0).  If the message has more channels than expected, the excess channels are ignored.  Messages must be published to this channel periodically.  When a message is received, Simple Flight marks the controller as "connected".  If a subsequent message is not received within the RC connection update timeout (defaulting to 500 ms), Simple Flight marks the controller as "disconnected".</li></ul>

### Message types

#### FlightControlRCInputMessage
``` C
float channels[]
```
<ul><li style="list-style-type: none;">`channels` is a variable-length array of floating-point values.  The first entry in the array is the value of channel #0, the second is channel #1, etc.</li></ul>

### RC input channels
Simple flight assigns the RC input channels to following functions:

| Channel # | Values | Function |
| :-------: | :----: | -------- |
| 0 | -1.0&nbsp;..&nbsp;+1.0 | X-roll axis angle or angle rate |
| 1 | -1.0&nbsp;..&nbsp;+1.0 | Z-yaw axis angle or angle rate |
| 2 | 0.0&nbsp;..&nbsp;+1.0 | Throttle |
| 3 | -1.0&nbsp;..&nbsp;+1.0 | Y-pitch axis angle or angle rate |
| 4 | 0.0, 1.0 | Select whether the axis channels specify the angle (0.0) or angle rate of change (1.0) |
| 5 | 0.0, 1.0 | Client API control disabled (0.0) or enabled (1.0)

RC input channels 0, 1, and 3 specify a control value for their respective axes.  The state of channel 4 determines whether Simple Flight interprets the axis control value as an angle or as a rate of change of the angle.

Simple Flight recognizes two special RC channel value patterns to arm or disarm the vehicle which must be held continuously for at least 100 ms.  This allows the user to arm or disarm the vehicle by putting the input controls in a specific arrangement.

The `xbox_rc.py` script (specifically the `XboxInputControllerSF` class) also enables the Back and Start buttons to generate these patterns to make it more convenient to arm and disarm the vehicle.  See [Using an Xbox controller](#using-an-xbox-controller), above.

To arm the vehicle, the following RC input channels must have these values for at least 100 ms:

| Channel | Function | Value |
| :-----: | -------- | :---: |
| 0 | X-roll | &leq; -0.9 |
| 1 | Z-yaw | &geq; 0.9 |
| 2 | Throttle | &leq; 0.1 |
| 3 | Y-pitch |  &geq; 0.9 |

To disarm the vehicle, the following RC input channels must have these values for at least 100 ms:

| Channel | Function | Value |
| :-----: | -------- | :---: |
| 0 | X-roll | &geq; 0.9 |
| 1 | Z-yaw | &leq; -0.9 |
| 2 | Throttle | &leq; 0.1 |
| 3 | Y-pitch |  &geq; 0.9 |

---

Copyright (C) Microsoft Corporation. 
Copyright (c) 2025 IAMAI Simulations.

MIT License. All rights reserved.
