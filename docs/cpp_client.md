# Project AirSim C++ Client

The Project AirSim C++ client is a native client library for connecting to a
running Project AirSim simulation server. It provides the same main programming
model as the Python client--a `Client`, a `World`, and robot-specific objects
such as `Drone`--for applications that need a C++17 interface.

The C++ client source is in `client/cpp/`.

## Client layout

The main C++ client components are:

| Path | Purpose |
| --- | --- |
| `client/cpp/ProjectAirsimClientLib/` | High-level client API for connecting to the server, loading worlds, and controlling robots. |
| `client/cpp/ProjectAirSimMessageLib/` | Message serialization helpers used by the client. |
| `client/cpp/NNGI/` | NNG-based transport wrapper. |
| `client/cpp/example_user_apps/HelloDrone/` | Minimal example application. |
| `client/cpp/example_user_apps/CppClientScenarios/` | Scenario-oriented example application. |
| `client/cpp/example_user_apps/UserScenarioTemplate/` | Starting point for custom C++ user scenarios. |
| `client/cpp/scripts/` | Linux helper scripts for building and running scenario examples. |

The public API headers are under:

```text
client/cpp/ProjectAirsimClientLib/Include/ProjectAirsimClient/
```

Most applications should include the aggregate header:

```cpp
#include <ProjectAirsimClient/ProjectAirsimClient.h>
```

All public client types are in the `microsoft::projectairsim::client`
namespace. The examples commonly use this alias:

```cpp
namespace pasc = microsoft::projectairsim::client;
```

## Prerequisites

To build the C++ client, install:

- CMake 3.20 or newer
- A C++17 compiler
- Ninja on Linux, when using the repository `build.sh` targets
- Visual Studio 2022 C++ build tools on Windows, when using `build.cmd`

The Linux CMake build fetches third-party dependencies such as NNG,
`nlohmann-json`, and msgpack on first configure. Eigen3 is used from the system
when available, or fetched automatically.

## Building on Linux

From the repository root, build the C++ client with:

```bash
./build.sh cpp_client_debug
./build.sh cpp_client_release
```

The C++ client is also built as part of the simulation library targets:

```bash
./build.sh simlibs_debug
./build.sh simlibs_release
```

Build outputs are placed under:

| Build | Output directory |
| --- | --- |
| Debug | `client/cpp/build_linux/Debug/` |
| Release | `client/cpp/build_linux/Release/` |

The main static libraries are generated in the selected build directory:

```text
libProjectAirsimClient.a
libProjectAirSimMessageLib.a
libNNGI.a
```

Example executables are generated in the same directory, including:

```text
hello_drone
cpp_client_scenarios
user_scenario_template
```

You can also build the client manually with CMake:

```bash
cmake -S client/cpp -B client/cpp/build_linux/Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build client/cpp/build_linux/Debug -j$(nproc)
```

## Building on Windows

From a Windows command prompt, use the repository build wrapper:

```bat
build.cmd cpp_client_debug
build.cmd cpp_client_release
```

The Windows Visual Studio projects place C++ client library artifacts under:

```text
client\cpp\libraries\x64\Debug\
client\cpp\libraries\x64\Release\
```

The `HelloDrone` example executable is placed under:

```text
client\cpp\example_user_apps\HelloDrone\x64\Debug\
client\cpp\example_user_apps\HelloDrone\x64\Release\
```

When running `hello_drone.exe`, make sure the corresponding
`client\cpp\libraries\x64\<Configuration>\` directory is on `PATH`, or copy the
required DLLs next to the executable.

## Running HelloDrone

Start a Project AirSim simulation environment first, either from Unreal Editor
with the Project AirSim plugin or from a packaged environment such as Blocks.

The `hello_drone` example connects to the simulation server, loads
`scene_basic_drone.jsonc`, arms `Drone1`, takes off, moves up, lands, and
disconnects.

On Linux:

```bash
./client/cpp/build_linux/Debug/hello_drone \
  --simhost 127.0.0.1 \
  --simconfig client/python/example_user_scripts/sim_config
```

On Windows:

```bat
client\cpp\example_user_apps\HelloDrone\x64\Debug\hello_drone.exe ^
  --simhost 127.0.0.1 ^
  --simconfig client\python\example_user_scripts\sim_config
```

The `--simconfig` directory must contain the scene file and related robot
configuration files. The repository includes ready-to-run examples in:

```text
client/python/example_user_scripts/sim_config/
```

## Creating a C++ application

For a custom Linux CMake application, build the client first and then link
against the generated libraries. The simplest in-repository workflow is to add a
new executable target to `client/cpp/CMakeLists.txt`, link it to
`ProjectAirsimClient`, and rebuild.

For example:

```cmake
add_executable(my_drone_app
    example_user_apps/MyDroneApp/MyDroneApp.cpp
)

target_link_libraries(my_drone_app
    PRIVATE
        ProjectAirsimClient
)

if(NOT WIN32)
    target_link_libraries(my_drone_app PRIVATE Threads::Threads)
endif()
```

Then build the target:

```bash
cmake --build client/cpp/build_linux/Debug --target my_drone_app -j$(nproc)
```

For custom scenarios, start from:

```text
client/cpp/example_user_apps/UserScenarioTemplate/UserScenarioTemplate.cpp
```

The helper script can build and run a scenario target from
`client/cpp/build_local`:

```bash
cmake -S client/cpp -B client/cpp/build_local -DCMAKE_BUILD_TYPE=Debug
./client/cpp/scripts/run_cpp_user_scenario.sh --target user_scenario_template
```

## Minimal client code

The following shows the basic C++ client flow. See
`client/cpp/example_user_apps/HelloDrone/HelloDrone.cpp` for a complete example
with argument parsing and error handling.

```cpp
#include <ProjectAirsimClient/ProjectAirsimClient.h>

#include <iostream>
#include <memory>

namespace pasc = microsoft::projectairsim::client;

int main() {
  auto client = std::make_shared<pasc::Client>();
  if (client->Connect("127.0.0.1") != pasc::Status::OK) {
    return 1;
  }

  auto world = std::make_shared<pasc::World>();
  if (world->Initialize(client,
                        "scene_basic_drone.jsonc",
                        "client/python/example_user_scripts/sim_config",
                        2.0f) != pasc::Status::OK) {
    return 1;
  }

  auto drone = std::make_shared<pasc::Drone>();
  if (drone->Initialize(client, world, "Drone1") != pasc::Status::OK) {
    return 1;
  }

  bool ok = false;
  drone->EnableAPIControl(&ok);
  drone->Arm(&ok);

  drone->TakeoffAsync().Wait();
  drone->MoveByVelocityAsync(0.0f, 0.0f, -1.0f, 4.0).Wait();
  drone->LandAsync().Wait();

  drone->Disarm(&ok);
  drone->DisableAPIControl(&ok);
  client->Disconnect();

  return 0;
}
```

## Async operations

Movement APIs return `AsyncResult`. Call `Wait()` to block until the operation is
complete and retrieve its `Status`.

```cpp
auto takeoff = drone->TakeoffAsync();
pasc::Status status = takeoff.Wait();
```

You can also poll for completion, but still call `Wait()` afterward to retrieve
the final status:

```cpp
auto move = drone->MoveByVelocityAsync(0.0f, 0.0f, -1.0f, 4.0);

while (!move.FIsDone()) {
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

pasc::Status status = move.Wait();
```

## More examples

Additional example applications are available under:

```text
client/cpp/example_user_apps/
```

The scenario runner scripts under `client/cpp/scripts/` demonstrate common
flight, sensor, rover, environment actor, lidar, radar, and battery workflows.

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI CONSULTING CORP

MIT License. All rights reserved.
