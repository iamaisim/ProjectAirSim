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

## Standard-command external controller

The `matlab-vtol-controller-api` controller exposes the standard Drone movement services to an external Simulink controller. It sends state and sensor data, flight setpoints, lifecycle state, and an ordered effector description over NNG request/reply. The external model owns control allocation and physical units. The existing `matlab-controller-api` and its block ports retain their original contract.

Configure the robot controller as follows:

```json
"controller": {
  "id": "ExternalController",
  "type": "matlab-vtol-controller-api",
  "matlab-vtol-controller-api-settings": {
    "control-connection": { "ip": "127.0.0.1", "port": 8998 },
    "actuator-order": [
      { "id": "rotor", "kind": "rotor", "units": "rpm", "initial-value": 0.0 },
      { "id": "surface", "kind": "surface", "units": "rad", "initial-value": 0.0 }
    ]
  }
}
```

| Setting | Default | Meaning |
|---|---|---|
| `actuator-order` | Required | Nonempty ordered effector list; each entry has a unique `id`, `kind`, and `units`. There is no fixed output-count limit. Robot actuators whose IDs are not listed receive 0; a warning listing them is logged when the controller loads. |
| `initial-value` | `0.0` | Per-effector output while disabled, disarmed, reset, or unavailable. |
| `control-connection` | `127.0.0.1:8998` | External controller listener. Scene-level control connection settings take precedence. |
| `request-timeout-ms` | `1000` | Wall-clock limit for each send or receive operation. |
| `command-period` | `0.02` | Simulated seconds between movement-command updates. |
| `command-handoff-grace-period` | `0.0` | Nonnegative finite simulated seconds to retain a completed velocity, velocity-with-altitude, or heading setpoint while awaiting replacement. |
| `takeoff-z` | `-3.0` | Relative NED takeoff height in metres; must be negative. |

These defaults describe the generic API and do not define an aircraft operating envelope. The model reports ready/landed status. API control must be enabled and the model ready before arming. Hover retains an explicit position/yaw objective until superseded or cancelled. Other task completion sends idle by default; the model defines idle hold behavior. A positive `command-handoff-grace-period` retains setpoints from successful positive-duration velocity and heading requests until replacement or the first controller update at or after the simulated deadline. This allowance does not delay the service reply. Position and takeoff/land completion retain immediate idle behavior. Cancellation and failed requests clear the setpoint immediately, including during a handoff. Disabling, disarming, reset, timeout, and invalid replies restore configured initial outputs. Attitude, PWM, angular-rate, and C++ gain commands are unsupported and raise an error. `SetVTOLMode` supplies a requested mode to the model, rather than confirming a completed physical transition.

A failed exchange (send or receive timeout, transport error, or invalid reply) disarms the vehicle, sends idle, and restores the configured initial outputs, so the vehicle does not continue on stale outputs. While the controller is unavailable, every controller update retries the exchange and waits up to `request-timeout-ms`, so simulation time advances slowly instead of running the vehicle without control. One warning is logged per outage, and a trace message is logged when a valid reply is received again. The client must re-arm after the model reports ready.

`request-timeout-ms` limits how long the simulation waits for the model. The block's optional transport timeout limits how long the model waits for the simulation. By default, the block waits indefinitely for the next request and to send its reply, so pausing or debugging the simulation does not stop the model. A positive finite block timeout bounds each wait in wall-clock seconds, independent of the model sample period and simulation clock; zero, a negative value, or `Inf` keeps the indefinite wait. With a timeout set, pausing, debugging, or loading a scene longer than the timeout stops the model with an error, and the controller's next exchange then fails and disarms the vehicle as described above.

### Message and block contract

`VTOLControlModelInputMessage` is a MessagePack map with `version: 1`, `state`, `setpoint`, and `effectors`. `state` retains the legacy control-model kinematics and sensor fields, including the simulation timestamp in nanoseconds. Each effector contains `id`, `kind`, `units`, and `initial_value`. Replies contain `version: 1`, the identical `time_stamp`, exactly one finite `control_values` entry per configured effector, and Boolean `ready`/`landed` status. Missing required fields, other versions, mismatched timestamps/counts, and nonfinite outputs are rejected. Values pass through without unit conversion; actuator or plant settings must use the declared units.

Build the new MEX block from `vehicle_apis/multirotor_api/matlab_sfunc`:

```matlab
buildMatlabVTOLControlModelSfcn
```

The block closes its listener at model termination. The MEX module remains
loaded until MATLAB exits so its NNG workers remain valid between runs.
Restart MATLAB before rebuilding a module that has been used.

The block `matlab_vtol_control_model_sfcn` takes four parameters: host character vector, port character vector, positive sample period, and an N-by-3 cell array of effector IDs, kinds, and units. An optional fifth parameter sets the transport timeout in seconds, for example `'127.0.0.1','8998',0.003,effectors,30`. The effector metadata must match the robot configuration exactly. Use a fixed-step solver. Input 1 is the N-element control vector; input 2 is `[ready; landed]`, with each status equal to 0 or 1. Outputs 1–7 retain the stock kinematics, airspeed, barometer, IMU, magnetometer, distance, and GPS layouts. Output 8 is the command vector:

| Index | Field | Meaning |
|---|---|---|
| 1 | `mode` | 0 idle, 1 velocity, 2 velocity with altitude, 3 position, 4 heading/speed |
| 2 | `frame` | 0 world NED, 1 body FRD |
| 3–5 | `x`, `y`, `z` | Velocity (m/s), position (m), or heading (rad), speed (m/s), vertical velocity (m/s), according to mode |
| 6–7 | `yaw_is_rate`, `yaw` | Yaw rate in rad/s when true; absolute world yaw in radians otherwise |
| 8 | `vtol_mode` | -1 unspecified, 0 multirotor, 1 fixed wing |
| 9–10 | `api_enabled`, `armed` | Boolean lifecycle state |
| 11 | `request` | 0 none, 1 takeoff, 2 land, 3 hover; movement setpoints accompany the request |
| 12 | Time | Simulation time in seconds |

For mode 2, `z` is an absolute NED altitude. Standard Drone body-frame services rotate horizontal velocity into world NED once at command start and send world-frame setpoints throughout the task. Later yaw changes do not rotate that velocity. This follows the shared Drone command implementation and differs from Simple Flight multirotors, which keep the commanded velocity body-relative while yaw changes. Yaw handling and nonpositive-duration results follow the shared Drone command implementation. The protocol also supports body-frame setpoints through the low-level controller commands. The bridge replies with the previous step's controller output and then exposes the new input, preserving the stock discrete exchange sequence. Readiness and optional flight-mode handling belong to the external controller.

### Generic quadrotor example

The example loader `load_external_quadrotor_simulink_control_model` builds an in-memory model from the supplied quadrotor navigation controller, preserving its gains and mixing. Its command guidance generates position/yaw references from the generic setpoints. It uses the stock Fast Physics quadrotor and four normalized motor outputs; it does not represent a VTOL transition or an external aircraft's dynamics.

Add the newly built MEX directory to the MATLAB path, run the loader from `client/python/example_user_scripts/simulink`, and leave its shared MATLAB engine open. Run `simulink_external_quadrotor.py` from `client/python/example_user_scripts` against a running Project AirSim server. The script connects to the only shared MATLAB session, regardless of its name. Close unused shared sessions before running it; ambiguous selection stops before connection or model changes. It uses normal Drone services for arming, hover, world/body velocity, position, and landing. The model is built in memory and configuration copies use a temporary directory. Run live examples sequentially.

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI CONSULTING CORP

MIT License. All rights reserved.
