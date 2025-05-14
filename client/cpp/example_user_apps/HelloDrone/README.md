# HelloDrone - Example Project AirSim application

This example project illustrates the basics of a C++ application controlling a
Project AirSim simulation through the Project AirSim C++ client library.


## Building:
1. Open the solution file "HelloDrone.sln" in Visual Studio 2019.
2. Build the project (Build-->"Build Solution".)
3. The executable `hello_drone.exe` is in the project output directory.
4. Note that `hello_drone.exe` imports the `AirSimClient.dll` and `NNGI.dll`
dynamically-loaded libraries.  These two files are in the
<code>client\cpp\libraries\\<i>build_platform</i>\\<i>build_configuration</i>\\</code>
directory. Add this directory to the `PATH` environment variable or copy those
DLL's to the directory containing `hello_drone.exe`.

## Running:
1. Before running the Hello Drone application, launch a basic Project AirSim simulation
environment.

1. From a command prompt or terminal, run `hello_drone.exe /?` to see the command
line parameters.  Generally, run the command with the form:

	<code>hello_drone --simhost</code> <i>Project_AirSim_server_address</i> <code>--simconfig</code> <i>path_to_sim_config_directory</i>

The application is looking for the scene configuration file `scene_basic_drone.jsonc` which
is in the `client/python/example_user_scripts/sim_config` directory of the
Project AirSim client distribution package.

## Explanation:
See the function `main()` in HelloDrone.cpp.

* The application parses the command line to obtain the simulation server
hostname/address and simulation configuration file directory specified by the
user.

* The log output sink is set to the function `MyCustomLogSink()`.  This function
simply outputs the log output string to the standard output.  More
sophisticated apps may want to direct the output to a file, to a window of
the application, or process the output like filtering by severity.  The default
log sink sends the log to the debug output.

* A `microsoft::projectairsim::client::Client` object is created and
initialized to open a connection to the Project AirSim server.  This object
represents the connection to the server and provides access to server
functions not provided by the `World` and `Drone` classes.

* A `microsoft::projectairsim::client::World` object is created and initialized
to load the scene configuration file, `scene_basic_drone.json`, into the
simulation server.  The `World` object provides access to the scene running on
the server.

* A `microsoft::projectairsim::client::Drone` object is created and initialized
to the drone created by the scene configuration file, `Drone1`.  The `Drone`
object provides access to a drone created in the scene.

* The method `Drone::EnableAPIControl()` enables control of the drone through the
other methods in the Drone object.  When API control is disabled, the drone
ignores calls to those other methods.

* The method `Drone::Arm()` makes the drone ready to fly.  The drone's motors
are disabled until the drone is armed.

* The method `Drone::TakeoffAsync()` is called to launch the drone from the
ground.  Methods that can take time to execute (like movement such
as `TakeoffAsync()`) return an `AsyncResult` object so that the caller can
continue while the drone executes the command in parallel.  The app calls
`AsyncResult::Wait()` (a blocking call) when it is ready to wait for the
command to complete and get the `Status`-type value indicating the result of
the command.

* The method `Drone::MoveByVelocityAsync()` commands the drone to move at
1 meter/second straight up for 4 seconds.  Here the application calls
`AsyncResult::FIsDone()` in a loop to illustrate a non-blocking way to check
whether the command is done executing instead of calling `AsyncResult::Wait()`.
Note that `AsyncResult::Wait()` must still be called after `AsyncResult::FIsDone()`
returns true in order to obtain the `Status` result of the command.  Note also
that the body of the loop calls `std::this_thread::sleep_for()` when
`AsyncResult::FIsDone()` returns false to be more multitasking-friendly than
spinning in a tight loop.

* The method `Drone::LandAsync()` commands the drone to land.

* The drone is then disarmed and API control disabled via the `Drone::Disarm()`
and `Drone::DisableAPIControl()` methods mirroring the arm and control-enable
calls at the start.

* Finally the application disconnects from the Project AirSim simulation
server via `Client::Disconnect()`.  The `World` and `Drone` objects are no longer
valid once `Client::Disconnect()` is called.  While the `Client` object will
automatically disconnect when it is deleted, it is good practice to call
`Client::Disconnect()` explicitly.

---

Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.

MIT License.
