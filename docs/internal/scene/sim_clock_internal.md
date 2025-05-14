# Simulation Clock

The simulation clock is a core component of the simulation scene configuration and execution.

*Note: When using the Unreal Editor's play-in-editor mode, there are buttons to **Pause/Resume/SingleStep** the game, **but these buttons are NOT supported** by Project AirSim since they are linked directly to the internals of the Unreal Engine ticks. To perform sim clock control functions, please use the SimClock client API instead.*

## Sim clock config

For details on the configuration options, see **[clock settings](../../config_scene.md#clock-settings)**.

## Sim clock API

For details on the API, see **[Sim Clock API](../../api.md#sim-clock-api)**.

## Example usage scenarios

#### #1 - Standard simulation with the option to pause/resume, do manual step control, scale faster/slower than real-time execution

The `steppable` clock is recommended for most scenarios since it uses deterministic fixed-delta sim-time steps for each iteration of the simulation loop (set by `step-ns` in nanoseconds), and enables pause/resume and manual control of the clock via the SimClock APIs.

`real-time-update-rate` sets the real-time period in nanoseconds between each iteration of the simulation loop. The simulation will advance faster or slower than real-time according to the ratio of `step-ns` to `real-time-update-rate`. If `step-ns` is larger than `real-time-update-rate`, then sim time will advance faster than real-time, and vice-versa.

*Note: When using physics that are external to Unreal, such as FastPhysics for drones, the rendering loop runs separately from the sim loop (as fast as the GPU can handle, but generally at a slower rate) so that the simulation can advance with higher fidelity time steps without requiring a rendered image for every step.*

```json
"clock": {
  "type": "steppable",
  "step-ns": 3000000,
  "real-time-update-rate": 3000000,
  "pause-on-start": true|false
}
```

#### #2 - Full manual clock step control by client API calls only

When you want the simulation to advance only when commanded to do so via a call to the SimClock API, set `step-ns` to zero so that sim-time will not advance automatically (whether paused or unpaused.) This "lock-step" behavior is useful, for example, when using Project AirSim in a perception-action loop.

```json
"clock": {
  "type": "steppable",
  "step-ns": 0,
  "real-time-update-rate": 3000000,
  "pause-on-start": false
}
```

#### #3 - Use Unreal Physics with the option to pause/resume, do manual step control (but can't scale with real-time)

If any robot actors in a scene are configured to use the **[Unreal's Physics](../physics/unreal_physics.md)** system, the physics update will be done in step with rendering the scene and each iteration of the simulation loop (and each advance of sim-time) will take longer.

The **same `steppable` clock settings can be used** as in non-Unreal Physics scenarios, but if `step-ns` is left set to a very small time step, such as 3 ms, the simulation will advance much slower than real-time due to the rendering occuring at each 3 ms sim-time step.

Generally, if Unreal Physics is used, **setting a larger `step-ns` such as 20 ms (=50 FPS)** will allow the simulation to advance closer to real-time on a typical GPU, but at the cost of lower fidelity physics and larger control time steps.

`real-time-update-rate` can still remain small for a fast update rate, such as 3 ms, because the sim update loop will just be checking for the next physics/rendering calculation on each loop and will not actually advance the sim-time until physics/rendering has completed its step.

```json
"clock": {
  "type": "steppable",
  "step-ns": 20000000,
  "real-time-update-rate": 3000000,
  "pause-on-start": true|false
}
```

#### #4 - Real-time simulation to run everything in actual time (can't pause/resume/scale), such as for non-physics mode

For running the simulation in a non-deterministic way where the sim clock is stepped by the amount of actual time passed between each execution loop, use the `real-time` clock type.

This is useful, for example, when running in a **non-physics "computer vision" mode** where the robot is moved arbitrarily around the scene with no physical meaning to sim-time except as a timestamp on sensor data. Another example would be where a human operator or hardware component is interacting with the simulation just like they would with a physical robot in the real world.

```json
"clock": {
  "type": "real-time",
  "real-time-update-rate": 3000000,
}
```

---

Copyright (C) Microsoft Corporation. 
Copyright (c) 2025 IAMAI Simulations.

MIT License. All rights reserved.
