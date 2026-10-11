# MATLAB Physics

Project AirSim connects existing MATLAB/Simulink models to the simulation through TCP request/reply bridges. A physics model receives actuator loads, environment data, and collision information, then returns vehicle kinematics. A controller model receives vehicle state and sensor measurements, then returns actuator commands. The stock examples demonstrate each interface separately and together.

The physics bridge supports up to eight rotors and eight control surfaces. The stock physics model uses the Aerospace Blockset 6DOF block; FastPhysics is used only in the controller-only example.

## Prerequisites

- MATLAB and Simulink with Aerospace Blockset for the stock physics models.
- UAV Toolbox for the controller-only example's Waypoint Follower block.
- A 64-bit Python interpreter compatible with the installed MATLAB release and the Project AirSim client, in an activated virtual environment.
- A compatible C++ MEX compiler when building the bridges from source.
- A running Project AirSim simulation server, such as the Blocks environment in the Unreal editor.

Use MathWorks' [Python compatibility table](https://www.mathworks.com/support/requirements/python-compatibility.html) and [supported compilers](https://www.mathworks.com/support/requirements/supported-compilers.html) for the selected release. MATLAB release, Python version, and MEX compiler compatibility are separate requirements.

### Release compatibility

The MEX build scripts and stock examples have been tested with MATLAB R2026b on Linux. For other releases and for Windows, confirm Python and compiler support in the MathWorks tables above.

## One-time setup

### MATLAB Engine API for Python

Activate the Python environment used by the Project AirSim client. In the MATLAB Command Window, run `matlabroot` to find the installation folder, then install the Engine package from that folder:

```
python -m pip install "<matlabroot>/extern/engines/python"
python -c "import matlab.engine; print(matlab.engine.__file__)"
```

Installing from the MATLAB folder can require write access to that folder, for example an Administrator command prompt on Windows. Alternatively, install the `matlabengine` package from PyPI with the version that matches the MATLAB release. In R2026a and later, MATLAB also includes a preinstalled Engine package in `<matlabroot>/extern/engines/python/dist`, which can be added to `PYTHONPATH` instead of installing it.

Use the Engine package from the same MATLAB release as the desktop session. See MathWorks' [Engine installation instructions](https://www.mathworks.com/help/matlab/matlab_external/install-the-matlab-engine-for-python.html) for details.

### Build the Simulink MEX bridges

Prebuilt MEX bridges for Windows (`.mexw64`) and Linux (`.mexa64`) are included in `client/python/example_user_scripts/simulink/`. Build them from source when they do not load in the installed MATLAB release or when the bridge sources change.

First, build the sim libs as described in **[Build From Source as a Developer](../development/use_source.md)**. From the Project AirSim repository root, run the following on Windows from the `x64 Native Tools Command Prompt for VS 2022`:

```
build.cmd simlibs_release
```

On Linux, run:

```
./build.sh simlibs_release
```

The sim libs build stages the NNG libraries and MessagePack headers used by the MEX build scripts under each S-function directory's `_deps/` folder. The MEX scripts use the same MessagePack configuration as the native build, without requiring Boost headers. Keep the bridge sources and sim libs from the same checkout.

In the MATLAB Command Window, start at the repository root and select a C++ compiler with `mex -setup C++`. On Windows, select Microsoft Visual C++, which is also used to build the sim libs. Then build and copy the bridges into the stock model directory:

```matlab
repo = pwd;
model_dir = fullfile(repo, 'client', 'python', 'example_user_scripts', 'simulink');
cd(fullfile(repo, 'physics', 'matlab_sfunc'))
buildMatlabPhysicsModelSfcn
copyfile(['matlab_physics_model_sfcn.' mexext], model_dir)
cd(fullfile(repo, 'vehicle_apis', 'multirotor_api', 'matlab_sfunc'))
buildMatlabControlModelSfcn
copyfile(['matlab_control_model_sfcn.' mexext], model_dir)
cd(repo)
```

These build commands can also run with `matlab -batch` after selecting the compiler. Copying the rebuilt files replaces the bundled binaries for the current platform. Rebuilt binaries are specific to the platform and MATLAB release used to build them; do not commit them unless intentionally updating the bundled binaries.

## Desktop startup

Open a MATLAB desktop under the same user account as the Python client. From the repository root in MATLAB:

```matlab
cd(fullfile(pwd, 'client', 'python', 'example_user_scripts', 'simulink'))
load_quadrotor_simulink_physics_model
```

The loader supplies the stock aircraft parameters, opens the model, and shares the desktop as `MATLABEngine`. Use the loader corresponding to the desired example:

| Example | MATLAB loader | Python script |
| --- | --- | --- |
| Quadrotor physics with SimpleFlight | `load_quadrotor_simulink_physics_model` | `simulink_physics_quadrotor.py` |
| Quadtiltrotor physics with SimpleFlight | `load_quadtiltrotor_simulink_physics_model` | `simulink_physics_quadtiltrotor.py` |
| Simulink controller with FastPhysics | `load_quadrotor_simulink_control_model_navigation` | `simulink_controller_quadrotor.py` |
| Simulink physics and controller | `load_quadrotor_simulink_combined_model` | `simulink_combined_quadrotor.py` |

Leave the MATLAB desktop open while running the examples.

## Run an example

Start the simulation server, for example by pressing Play in the Unreal editor. In the activated Python environment, run from the repository root:

```
cd client/python/example_user_scripts
python simulink_physics_quadrotor.py
```

The Python script connects to the shared Engine, loads its scene, and starts the Simulink model through the Engine API. Physics examples execute takeoff, flight, and landing commands. The controller-only example follows stock waypoints in MATLAB. The combined example sends a repeating actuator ramp to demonstrate both bridges; it does not provide waypoint guidance. Press Enter in the terminal to end either controller example. When an example ends, the script stops the Simulink model through the Engine API.

Run live examples one at a time. Load the corresponding MATLAB parameters before switching examples. End the Python example before stopping the simulation server, and keep the desktop open for another run. The scene and robot configurations are in `client/python/example_user_scripts/sim_config/`; the loaders, models, and MEX files are in its sibling `simulink/` directory.

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI CONSULTING CORP

MIT License. All rights reserved.
