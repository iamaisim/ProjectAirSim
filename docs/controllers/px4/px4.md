# PX4 Autopilot Flight Controller

The [PX4 Autopilot software stack](http://github.com/px4/firmware) is a very popular open-source flight controller with support for a wide variety of boards and sensors as well as a built-in capability for higher level tasks such as mission planning. Please visit [px4.io](http://px4.io) for more information.

**Warning**: While all releases of Project AirSim support PX4, setting up PX4 is not trivial. It may be easier to start with [Simple Flight](../simple_flight.md), the flight controller that comes with Project AirSim, before attempting to use PX4.

## Supported versions of PX4

Project AirSim supports PX4 v1.12.3. Other versions may work but are unsupported.

## Software-In-The-Loop (SITL)

The recommended way to use PX4 with Project AirSim is "Software In The Loop" or SITL.  In this configuration, the PX4 software stack is running as a program on a PC and communicating with Project AirSim on the same computer or another computer on the local network.

See [Use PX4 as Software In The Loop](px4_sitl.md) for setup and use.

## Hardware-In-The-Loop (HITL)

Project AirSim also supports using PX4 as "Hardware In The Loop" or HITL.  In this configuration the PX4 software stack is running on a dedicated controller board and communicating with Project AirSim via USB.  Project AirSim requires that the PX4 hardware supports the [MAVLink protocol](https://mavlink.io) over USB and runs a supported version of the PX4 firmware.

See [Use PX4 as Hardware In The Loop](px4_hitl.md) for setup and use.

## Which configuration?

The following table compares using Project AirSim with PX4 in the SITL and HITL configurations:



Feature | SITL (Recommended) | HITL
------- | ------------------ | ----
 PX4 Device | No | Required
Manual Flight | Via game controller or RC transmitter over USB| Optional via RC transmitter & receiver
Airframes | All Project AirSim airframes | "HIL Quadcopter X" only
Setup & Run | Harder | Easier
Modifying/Debugging PX4 Software | Easier | Harder

---

Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.

MIT License. All rights reserved.
