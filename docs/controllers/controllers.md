# Flight Controllers

A flight controller provides automatic control of the vehicle.  The amount of automatic control can vary from fully autonomous flight to assisted manual flight.

Project AirSim supports the following flight controllers: Simple Flight, PX4 Autopilot, and Manual Controller.

## Simple Flight

[Simple Flight](simple_flight.md) is a lightweight flight controller that comes with Project AirSim and enables you to setup and start flying quickly and easily.  Simple Flight currently supports quadrotor, hexarotor, VTOL quad-x tailsitter, and VTOL quad tiltrotor airframes.

## PX4 Autopilot

[PX4 Autopilot](px4/px4.md) is a popular open-source flight controller.  Project AirSim using PX4 currently supports the quadrotor airframe in the Hardware-In-The-Loop (HITL) configuration, and quadrotor, hexarotor, VTOL quad-x tailsitter, and VTOL quad tiltrotor airframes in the Software-In-The-Loop (SITL) configuration.

If you are not familiar with setting up PX4, you may find it easier to start with Simple Flight if your airframe is supported.

## Manual Controller

Manual Controller is a pass-through controller type with control signal outputs that are set completely manually by API and optionally starts with initial values set by config. See [Manual Controller settings](../config_robot.md#manual-controller-settings) and [Manual Controller commands](../api.md#manual-controller-commands) for more details.

## Comparing controllers

The following chart compares the flight controllers when used with Project AirSim:

Feature | Simple Flight | PX4 | Manual Controller
------- | ------------- | --- | ---
Project AirSim Airframes | Quadrotor, hexarotor, VTOL quad-x tailsitter, VTOL quad tiltrotor | Quadrotor, hexarotor, VTOL quad-x tailsitter, VTOL quad tiltrotor in SITL, quadrotor only in HITL | Any
Controller Hardware | None | None for SITL, required for HITL | None
Setup | Easy | Harder | Easy
Use | Easy | Harder | Manual
Tuning Support | No | Yes | N/A

---

Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.

MIT License. All rights reserved.
