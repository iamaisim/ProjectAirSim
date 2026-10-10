# MATLAB Physics

Project AirSim connects existing MATLAB/Simulink models to the simulation through TCP request/reply bridges. A physics model receives actuator loads, environment data, and collision information, then returns vehicle kinematics. A controller model receives vehicle state and sensor measurements, then returns actuator commands. The stock examples demonstrate each interface separately and together.

The physics bridge supports up to eight rotors and eight control surfaces. The stock physics model uses the Aerospace Blockset 6DOF block; FastPhysics is used only in the controller-only example.

## Prerequisites

- MATLAB and Simulink with Aerospace Blockset for the stock physics models.
- UAV Toolbox for the controller-only example's Waypoint Follower block.
- A 64-bit Python interpreter compatible with the installed MATLAB release and the Project AirSim client, in an activated virtual environment.
- A compatible C++ MEX compiler when building the bridges from source.
- A running Project AirSim Unreal simulation server for the stock physics flight examples. The controller-only example also supports Runtime.

Use MathWorks' [Python compatibility table](https://www.mathworks.com/support/requirements/python-compatibility.html) and [supported compilers](https://www.mathworks.com/support/requirements/supported-compilers.html) for the selected release. MATLAB release, Python version, and MEX compiler compatibility are separate requirements.

### Release compatibility

The Engine API and both MEX bridges are verified on Linux x86-64 with MATLAB/Simulink R2026b, Python 3.14, and g++ 15. All four stock example configurations load and compile in this setup. Live flight also requires the corresponding simulator host and working TCP bridges, as described below.

## One-time setup

### MATLAB Engine API for Python

Activate the Python environment used by the Project AirSim client. In the MATLAB Command Window, run `matlabroot` to find the installed release. From a Linux shell:

```bash
export MATLAB_ROOT="/path/to/MATLAB"
python -m pip install "$MATLAB_ROOT/extern/engines/python"
python -c "import matlab.engine; print(matlab.engine.__file__)"
```

R2026a and later also provide a preinstalled Engine package. This can be used when the MATLAB installation is read-only:

```bash
export PYTHONPATH="$MATLAB_ROOT/extern/engines/python/dist${PYTHONPATH:+:$PYTHONPATH}"
python -c "import matlab.engine; print(matlab.engine.__file__)"
```

Select the Engine package from the same MATLAB release as the desktop session. See MathWorks' [Engine installation instructions](https://www.mathworks.com/help/matlab/matlab_external/install-the-matlab-engine-for-python.html) for other platforms and release-matched PyPI packages.

### Build the Simulink MEX bridges

Use the [source-development setup](../development/use_source.md) to build the simulation libraries. From the Project AirSim repository root:

```bash
./build.sh simlibs_release
```

The source build stages NNG libraries and MessagePack headers under each S-function directory's `_deps/` folder. The MEX scripts use the same MessagePack configuration as the native build, without requiring Boost headers. Keep the bridge sources and simulation libraries from the same checkout.

In the MATLAB Command Window, start at the repository root and select a C++ compiler with `mex -setup C++`. Then build and copy the bridges into the stock model directory:

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

These build commands can also run with `matlab -batch` after selecting the compiler. Rebuild the MEX files for the current platform and MATLAB release rather than relying on a binary built for another installation. Generated MEX files remain local build outputs.

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

Leave MATLAB open. On Linux, the desktop session must have a valid display environment for this Simulink workflow; noninteractive MEX compilation does not imply that live model control works without a display.

## Run an example

Press Play in the Unreal editor. In the activated Python environment, run from the repository root:

```bash
cd client/python/example_user_scripts
python simulink_physics_quadrotor.py
```

The Python script connects to the shared Engine, loads its scene, and starts the Simulink model through the Engine API. Physics examples execute takeoff, flight, and landing commands. The controller-only example follows stock waypoints in MATLAB. The combined example sends a repeating actuator ramp to demonstrate both bridges; it does not provide waypoint guidance. Press Enter in the terminal to end either controller example.

Runtime supplies ground contact only for FastPhysics. It can run the controller-only example, but the stock MATLAB physics takeoff and landing sequences require Unreal ground contact.

Run live examples one at a time. Load the corresponding MATLAB parameters before switching examples. End the Python example before stopping the simulation server, and keep the desktop open for another run. The scene and robot configurations are in `client/python/example_user_scripts/sim_config/`; the loaders, models, and MEX files are in its sibling `simulink/` directory.

The stock S-functions wait synchronously for TCP requests. If a peer stops responding, the model can remain blocked and prevent an Engine stop command from completing. Restarting the MATLAB session may be required to release that connection.

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI CONSULTING CORP

MIT License. All rights reserved.
