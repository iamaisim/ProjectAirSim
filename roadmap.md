# Project AirSim Roadmap

This page consolidates the open items labeled
[`roadmap`](https://github.com/iamaisim/ProjectAirSim/labels/roadmap) in
issues, pull requests and discussions, plus proposed items that do not have an
issue yet. It describes planned direction, not delivery commitments or dates.
Items marked **Proposal** are not implemented.

Each item's GitHub issue, pull request or discussion is its source of truth.
Delivered work is recorded in [`docs/changelog.md`](docs/changelog.md) and
removed from this page.

| Status | Meaning |
|---|---|
| Open | Open issue or discussion; no implementation merged yet. |
| In review | Open pull request. |
| Proposal | Proposed direction; no issue or implementation yet. |

## 1. Build and development environment

| Item | Type | Status | Notes |
|---|---|---|---|
| [#231 Optional Docker dev environment for SimLibs + Unreal Engine 5.2](https://github.com/iamaisim/ProjectAirSim/pull/231) | PR | In review | Labels: `docker`. |

## 2. Physics and flight dynamics

| Item | Type | Status | Notes |
|---|---|---|---|
| [Discussion #6 Underwater robot dynamics](https://github.com/iamaisim/ProjectAirSim/discussions/6) | Discussion | Open | Community request. |
| [SimpleFlight for AI training](#5-simpleflight-for-ai-training-parity-with-px4--ardupilot) | — | Proposal | See section 5. |
| Wind, gusts and turbulence in Fast Physics | — | Proposal | Wind fields with gusts and turbulence, configurable per scene, that affect Fast Physics vehicles and wind-sensitive sensors such as airspeed. References: Dryden and von Kármán continuous turbulence models and the discrete "1-cosine" gust (MIL-F-8785C, MIL-HDBK-1797); power-law or logarithmic wind-shear profiles. Open-source reference models and implementations exist for this; contributions building on them are welcome. |

## 3. Simulation core, sensors and rendering

| Item | Type | Status | Notes |
|---|---|---|---|
| [#181 Lumen GI / reflections camera capture settings](https://github.com/iamaisim/ProjectAirSim/pull/181) | PR | In review | Labels: `camera`. |
| [#243 Hide other robots' weather actors from each robot's cameras](https://github.com/iamaisim/ProjectAirSim/pull/243) | PR | In review | |
| [#112 Tile actors docs](https://github.com/iamaisim/ProjectAirSim/pull/112) | PR | In review | Labels: `px4`. |
| Fisheye and lens distortion camera models | — | Proposal | Camera lens models (for example Brown-Conrady and fisheye, including fields of view above 180°) with real intrinsics, published consistently in camera info. References: Brown-Conrady radial-tangential distortion and OpenCV's fisheye (Kannala-Brandt / equidistant) camera model; the unified camera model and its double-sphere extension; ROS `sensor_msgs/CameraInfo` distortion models (`plumb_bob`, `rational_polynomial`, `equidistant`). Open-source reference models and implementations exist for this; contributions building on them are welcome. |

## 4. AI training and evaluation

| Item | Type | Status | Notes |
|---|---|---|---|
| Deterministic server-side episode record and replay | — | Proposal | Record an episode (poses, controls, events) on the server and replay it identically for debugging, regression testing and regenerating data with different sensors. References: ROS 2 bag recording (rosbag2) and the MCAP file format; seeded, simulation-clock-driven execution for bit-identical replay. Open-source reference models and implementations exist for this; contributions building on them are welcome. |
| Seeded domain randomization of scene, sensors and physics | — | Proposal | Extend object randomization to lighting, materials, weather, sensor parameters and physical parameters (mass, inertia, motors), controlled by a seed and the API. References: domain randomization for sim-to-real transfer (Tobin et al., 2017); the Gymnasium `reset(seed=...)` seeding convention. Open-source reference models and implementations exist for this; contributions building on them are welcome. |
| Drone scenarios and benchmarks | — | Proposal | Extend the existing Python test bench (`projectairsim.test` and `projectairsim.validate`: scene and environment variations, range and reach validation tasks, fault injection, CSV/XML reports and coverage estimates) with a documented scenario format that includes spawn poses and mission goals, a set of reference drone benchmark scenarios, and aggregate metrics to compare autonomy stacks and policies. Results should be timestamped with simulation time so reruns are reproducible. References: ASAM OpenSCENARIO for scenario description; standard metrics such as success rate, collision rate and absolute/relative trajectory error (ATE/RPE). Open-source reference models and implementations exist for this; contributions building on them are welcome. |

---

## 5. SimpleFlight for AI training (parity with PX4 / ArduPilot)

Make SimpleFlight a training-grade flight controller. Close the functional gaps
that matter compared with PX4 / ArduPilot SITL, and keep what makes it useful for
AI work: it runs in-process, with no external SITL and no network round-trip.
Add what reinforcement-learning and imitation-learning pipelines need:
deterministic and fast stepping, many parallel environments, randomized dynamics
and RL-friendly control interfaces.

References: the Gymnasium API (including vectorized environments); domain
randomization for sim-to-real transfer (Tobin et al., 2017); PX4 and ArduPilot
parameter references for controller tuning parity. Open-source reference models
and implementations exist for this; contributions building on them are welcome.

The goal is not to re-implement PX4 or ArduPilot. PX4 / ArduPilot SITL remain the
path for firmware-in-the-loop validation.
