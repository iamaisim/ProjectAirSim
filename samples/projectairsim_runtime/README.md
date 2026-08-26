# Project AirSim Runtime

`projectairsim-runtime` is a lightweight, engine-independent Project AirSim
host by IAMAI for fast development iterations. It runs `SimServer`,
controllers, and Project AirSim physics while providing a flat ground collision
host without requiring Unreal Engine.

Build the simulation libraries normally:

```powershell
build.cmd simlibs_debug
```

```bash
./build.sh simlibs_debug
```

Replace `simlibs_debug` with `simlibs_release` for a release build.

The debug executable is generated at
`build/win64/Debug/samples/projectairsim_runtime/projectairsim-runtime.exe` on Windows
and `build/linux64/Debug/samples/projectairsim_runtime/projectairsim-runtime` on Linux.
For a release build, use the corresponding `build/win64/Release` or
`build/linux64/Release` path.

Start the Runtime from the repository root:

```powershell
.\build\win64\Debug\samples\projectairsim_runtime\projectairsim-runtime.exe
```

```bash
./build/linux64/Debug/samples/projectairsim_runtime/projectairsim-runtime
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

This host intentionally does not render or emulate Unreal sensors, world
meshes, or robot-to-robot collision. It is intended for quick controller,
client, API, and physics iteration with `fast-physics` robots.
