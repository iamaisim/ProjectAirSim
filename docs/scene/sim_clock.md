# Simulation Clock

The simulation clock is a core component of the simulation scene configuration and execution.

*Note: When using the Unreal Editor's play-in-editor mode, there are buttons to **Pause/Resume/SingleStep** the game, **but these buttons are NOT supported** by Project AirSim since they are linked directly to the internals of the Unreal engine ticks. To perform sim clock control functions, please use the SimClock client API instead.*

## Sim Clock Config

For details on the configuration options, see **[clock settings](../config_scene.md#clock-settings)**.

## Sim Clock API

For details on the API, see **[Sim Clock API](../api.md#sim-clock-api)**.

## Example Usage Scenarios

#### #1 - Standard simulation with the option to pause/resume, do manual step control, scale faster/slower than real-time execution

For most scenarios, the `steppable` clock would be used since it gives deterministic fixed delta sim time steps for each execution loop set by `step-ns`, and also allows usage of the SimClock APIs to pause/resume and do manual step controls.

The `real-time-update-rate` sets the actual real-time period between each execution loop, so the simulation advancement can be scaled faster/slower than real-time be the ratio between `step-ns` and `real-time-update-rate`. If `step-ns` is larger than `real-time-update-rate`, then sim time will advance faster than real-time, and vice-versa.

*Note: When using physics that are external from Unreal, such as FastPhysics for drones, the rendering loop runs separately from the sim loop (as fast as the GPU can handle, but generally at a slower rate) so that the simulation can advance with higher fidelity time steps without requiring a rendered image for every step.*

```json
"clock": {
  "type": "steppable",
  "step-ns": 3000000,
  "real-time-update-rate": 3000000,
  "pause-on-start": true|false
}
```
#### #2 - Real-time simulation to run everything by actual time (can't pause/resume/scale), such as for non-physics mode

For running the simulation in a non-deterministic way where the sim clock is just stepped by the amount of actual time that passed between each execution loop, the `real-time` clock type can be used.

This is useful, for example, when running **non-physics "computer vision" mode** where the robot is moved arbitrarily around the scene and there is no physical meaning to the sim time except to be a timestamp on sensor data. Another example could be a scenario where a human operator or hardware component is interacting with the simulation as it runs just like they would with an actual version of the robot in the real-world.

```json
"clock": {
  "type": "real-time",
  "real-time-update-rate": 3000000,
}
```

#### #3 - Use an engine-driven clock (host loop outside the sim scheduler)

Use the `engine-driven` clock when another runtime is responsible for advancing the simulation loop and supplying elapsed time into Project AirSim while keeping deterministic fixed sim steps (`step-ns`).

From the **Unreal plugin** perspective this clock mode is **engine-driven** (the sim does not run its own periodic tick); on **Unreal Engine** builds the name is the **unreal-driven-clock**. Normally `AUnrealScene::Tick` forwards each frame's `DeltaTime` and drains fixed `step-ns` ticks. With [`clock.engine-substepping`](../config_scene.md#unreal-engine-clock-options), it instead runs one complete scene tick for each consumed Chaos substep using the actual substep duration, even when the scene has no Unreal Physics robot. Controllers, physics models, actuators, sensors, and timestamps therefore use that same measured delta. In this mode, `step-ns` is Chaos' maximum substep duration and Unreal's maximum of 16 substeps per frame is used. Other clock types ignore `engine-substepping` and do not capture Chaos substeps.

```json
"clock": {
  "type": "engine-driven",
  "step-ns": 3000000,
  "engine-fixed-fps": 50.0,
  "engine-substepping": true
}
```

#### #4 - Use an external-clock for future external control integration

Use the `external-clock` type when you want the same current behavior as `engine-driven`, but with a distinct configuration label reserved for future control of simlibs by another app or system.

```json
"clock": {
  "type": "external-clock",
  "step-ns": 3000000
}
```

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI CONSULTING CORP

MIT License. All rights reserved.
