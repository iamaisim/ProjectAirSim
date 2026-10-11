# Matlab Physics

Project AirSim allows users to import their existing Simulink physics models as an alternative to the built-in Fast Physics or other flight dynamics models. The AirSim server transmits information about the applied vehicular forces and the environmental state which the Simulink model processes to compute the vehicle's kinematic state, all through TCP.

## Prerequisites

Download and install Matlab + Simulink 2022a with the Aerospace Blockset add-on.

## Install Matlab Engine API for Python

In order to connect to Matlab from the Python client, the Matlab Engine API for Python must be installed. More information can be found on the MathWorks [documentation](https://www.mathworks.com/help/matlab/matlab_external/install-the-matlab-engine-for-python.html) but the general installation steps are prescribed below:
1. Run a command prompt as Administrator
1. Activate a Python virtual environment and navigate to the directory where the Matlab API Python library is installed: `cd "C:\Program Files\MATLAB\R2022a\extern\engines\python"`
1. Install the Matlab API Python library: `python setup.py install`

## Example Scripts with Simulink Physics

The example Python client scripts and configuration files can be found in `client/python/example_user_scripts/` and the corresponding Matlab model files can be found in `client/python/example_user_scripts/simulink/`. There are currently two user scripts - `simulink_quadrotor_standardized_model.py` and `simulink_quadtiltrotor_standardized_model.py` - both of which utilize the same Simulink model but with different parameter values for each airframe. The AirSim Simulink interface currently supports up to eight rotors and eight control surfaces.

### Directory Information
- Scripts beginning with `simulink_` in their name utilize Simulink physics. Each script references a scene configuration file.
- The `sim_config/` folder contains both scene configuration files and robot configuration files. Note the former contains a `physics-connection` parameter to connect to Matlab and references the latter in the `robot-config` parameter. The robot configuration file has its `physics-type` parameter set to `matlab-physics`.
- The `simulink/` folder contains the Matlab model loader m-file scripts, the Simulink models, and the S-function mex file. Each Matlab script loads the vehicle parameters, opens the Simulink model, and configures the Matlab Engine for the client script to use Python APIs to control. The Simulink model contains the drone and fixed-wing physics for the vehicle, and references the vehicle parameters loaded via the Matlab script.

### Running the Scripts
1. Open Matlab 2022a.
1. Navigate the Matlab file explorer to the `client/python/example_user_scripts/simulink/` folder.
1. Run the model loader m-file from the **Matlab console**:
   1. ex. `>> load_quadrotor_simulink_physics_model`
1. Launch and run the simulation server.
1. In an activated Python environment, navigate to `client/python/example_user_scripts/` and run the corresponding client script:
    1. ex. `python simulink_quadrotor_standardized_model.py`
1. After the simulation scene has started, it will automatically start the Simulink model through the Matlab Python API.
1. Once the script completes, it will automatically stop the Simulink model through the Matlab Python API. To restart the script, just run it again. To stop the simulation while the it is in progress, press Ctrl-C from the Python script console output to stop the in-progress flight command, stop Simulink, and disconnect the client. The simulation can be started again by simply running the script again.

## Transport lifecycle

The stock physics and controller blocks open their listeners before their first model update. The server waits for each reply without busy polling; stopping the scene closes its sockets and interrupts pending connection, send, or receive operations. A delayed physics reply keeps the current numerical step pending. It does not produce a replacement state or skip a plant update.

By default, each block waits indefinitely to receive the next request from the simulation and to send its reply. An optional fourth S-function parameter sets a transport timeout in wall-clock seconds, for example `hostname,portnumber,timestep,30`. A positive finite value bounds each wait, independent of the model sample period and simulation clock; zero, a negative value, or `Inf` keeps the indefinite wait. With a timeout set, pausing, debugging, or loading a scene longer than the timeout stops the model with an error. After a model error, stop or reload the scene before restarting the workflow. Each block closes its own listener, so other instances remain usable. The MEX modules remain loaded until MATLAB exits to keep NNG worker code available between runs; restart MATLAB before rebuilding a loaded module.

Runtime supplies flat-ground contact information to MATLAB physics vehicles. The external plant owns the contact response; see the [Runtime capabilities](../../samples/projectairsim_runtime/README.md).

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI CONSULTING CORP

MIT License. All rights reserved.
