# Project AirSim Runtime

`projectairsim-runtime` is a lightweight, engine-independent Project AirSim
host by IAMAI for fast development iterations. It runs `SimServer`,
controllers, and Project AirSim physics while providing a flat ground collision
host without requiring Unreal Engine.

Because Runtime uses the common Project AirSim server, APIs, physics, and
controller layers, compatible client integrations and scene or robot
configurations can move between Runtime and an Unreal Engine host. Runtime is
suited to fast, headless controller, API, physics, automation, and CI workflows;
the Unreal host can be used when the same workflow requires environment meshes,
rendered sensors, or visual fidelity.

## Supported Capabilities

Project AirSim Runtime supports:

- Fast Physics, JSBSim, and Simulink physics models;
- Simple Flight, PX4, ArduPilot, and manual controller workflows;
- the Project AirSim service and topic APIs; and
- non-rendered sensors including GPS, IMU, barometer, magnetometer, and
  airspeed.

Runtime does not provide cameras, LiDAR, radar, Unreal world meshes, or general
mesh and robot-to-robot collisions. Its host-side collision support is limited
to the flat ground plane described below and applies to Fast Physics vehicles.

Build the simulation libraries normally:

```powershell
build.cmd simlibs_debug
```

```bash
./build.sh simlibs_debug
```

Replace `simlibs_debug` with `simlibs_release` for a release build.

The build tree is keyed by the selected toolchain so different Unreal versions
cannot reuse an incompatible CMake compiler cache. On Windows it is generated
under `build/win64/<toolchain>/Debug`; on Linux it is generated under
`build/linux64/<resolved-UE_ROOT-directory>/Debug`, or
`build/linux64/system/Debug` when `UE_ROOT` is unset. For a release build, use
the corresponding `Release` directory.

Start the Runtime from the repository root:

```powershell
$runtime = Get-ChildItem .\build\win64\*\Debug\samples\projectairsim_runtime\projectairsim-runtime.exe |
  Select-Object -First 1
& $runtime.FullName
```

```bash
./build/linux64/system/Debug/samples/projectairsim_runtime/projectairsim-runtime
```

With the Project AirSim Python client environment activated, run the demo in a
second terminal:

```bash
python client/python/example_user_scripts/projectairsim_runtime/projectairsim_runtime_demo.py
```

The demo loads `scene_basic_drone.jsonc`, executes a short flight, displays the
vehicle position and altitude, and closes the viewer when the flight completes.

The collision lifecycle mirrors the Unreal plugin:

1. Project AirSim physics calculates the next robot kinematics.
2. Project AirSim Runtime evaluates the new pose against the ground plane.
3. It writes `CollisionInfo` back to the robot.
4. Fast Physics applies its existing landing or collision response.

Project AirSim Runtime uses the root link's inertial body box and configured
wheel links as contact geometry. For fixed landing gear that is not represented
by wheel actuators, pass the distance from the robot origin to the gear contact
point:

```powershell
projectairsim-runtime.exe --ground-clearance 1.25
```

On Linux:

```bash
./projectairsim-runtime --ground-clearance 1.25
```

The ground defaults to `z = 0` in the local NED frame. Change it with
`--ground-height`. The existing positional topics and services port arguments
remain supported.

Runtime intentionally does not render or emulate Unreal-dependent sensors or
world geometry. Use the Unreal Engine host when a workflow requires those
capabilities.
