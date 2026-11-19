# Flight Controllers

A flight controller provides automatic control of the vehicle.  The amount of automatic control can vary from fully autonomous flight to assisted manual flight.

Project AirSim supports the following flight controllers: Simple Flight, PX4 Autopilot, ArduPilot, and Manual Controller.

## Simple Flight

[Simple Flight](simple_flight.md) is a lightweight flight controller that comes with Project AirSim and enables you to setup and start flying quickly and easily.  Simple Flight currently supports quadrotor, hexarotor, VTOL quad-x tailsitter, and VTOL quad tiltrotor airframes.

## PX4 Autopilot

[PX4 Autopilot](px4/px4.md) is a popular open-source flight controller.  Project AirSim using PX4 currently supports the quadrotor airframe in the Hardware-In-The-Loop (HITL) configuration, and quadrotor, hexarotor, VTOL quad-x tailsitter, and VTOL quad tiltrotor airframes in the Software-In-The-Loop (SITL) configuration.

If you are not familiar with setting up PX4, you may find it easier to start with Simple Flight if your airframe is supported.

## ArduPilot

[ArduPilot](ardupilot.md) is an open-source flight controller supported through a UDP/IP Software-In-The-Loop (SITL) connection. Project AirSim includes ready-to-run quadrotor and hexarotor examples. Its Client API flight commands are not supported; control the vehicle through ArduPilot or a ground control station such as Mission Planner or QGroundControl.

## Manual Controller

Manual Controller is a pass-through controller type with control signal outputs that are set completely manually by API and optionally starts with initial values set by config. See [Manual Controller settings](../config_robot.md#manual-controller-settings) and [Manual Controller commands](../api.md#manual-controller-commands) for more details.

## JSBSim

JSBSim is a flight dynamics model (FDM) that can be used to simulate how an aircraft moves and responds to control inputs. A flight controller is the system that decides which control inputs to send, such as throttle, elevator, aileron, rudder, or rotor commands.

## Comparing controllers

The following chart compares the flight controllers when used with Project AirSim:

Feature | Simple Flight | PX4 | ArduPilot | Manual Controller | JSBSim
------- | ------------- | --- | --------- | ----------------- | ------
Project AirSim Airframes | Quadrotor, hexarotor, VTOL quad-x tailsitter, VTOL quad tiltrotor | Quadrotor, hexarotor, VTOL quad-x tailsitter, VTOL quad tiltrotor in SITL, quadrotor only in HITL | Quadrotor, hexarotor in SITL | Any | Any
Controller Hardware | None | None for SITL, required for HITL | None for SITL | None | None
Setup | Easy | Harder | Harder | Easy | Easy
Use | Easy | Harder | Harder | Manual | Easy
Tuning Support | No | Yes | Yes | N/A | No
---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI CONSULTING CORP

MIT License. All rights reserved.
