# ProjectAirSim C++ Client

Native C++ client library for interacting with the ProjectAirSim simulation server.

## C++ Client Assets

- Documentation: `client/cpp/docs/`
- Scenario scripts: `client/cpp/scripts/`
- Scenario app: `client/cpp/example_user_apps/CppClientScenarios/`
- User scenario template app: `client/cpp/example_user_apps/UserScenarioTemplate/`

Primary docs entrypoint: `client/cpp/docs/INDEX.md`.

Preferred end-user flow: build and run a single custom scenario app (instead of running all predefined scenarios). See `client/cpp/docs/README_CPP_CLIENT_SCENARIOS.md` and `client/cpp/scripts/run_cpp_user_scenario.sh`.

## Overview

The C++ client provides the same simulation control capabilities as the Python client but as a set of linkable static libraries, suitable for performance-critical applications or for projects that cannot use Python.

### Libraries

| Library | Description |
|---|---|
| `NNGI` | NNG-based IPC/TCP transport wrapper |
| `ProjectAirSimMessageLib` | Message serialization (msgpack + JSON) |
| `ProjectAirsimClient` | High-level simulation API (Client, World, Drone) |

### Public API Headers (`ProjectAirsimClientLib/Include/ProjectAirsimClient/`)

| Header | Contents |
|---|---|
| `ProjectAirsimClient.h` | Main include — pulls in all other headers |
| `Client.h` | `Client` — connects to the simulation server |
| `World.h` | `World` — loads scenes, controls physics step |
| `Drone.h` | `Drone` — takeoff, move, land, arm/disarm |
| `AsyncResult.h` | `AsyncResult` — non-blocking operation handles |
| `Status.h` | `Status` enum and `GetStatusString()` |
| `Types.h` | `LandedState`, `ReadyState`, vectors, poses |
| `Log.h` | Logging with configurable sink |

All public types live in the `microsoft::projectairsim::client` namespace (alias: `pasc`).

---

## Building

### Prerequisites

- CMake ≥ 3.20
- C++17 compiler (GCC ≥ 10 or Clang ≥ 13)
- Internet access on first build (FetchContent downloads NNG, nlohmann-json, msgpack, optionally Eigen3)

System Eigen3 is used if available (`sudo apt install libeigen3-dev`), otherwise it is fetched automatically.

### Via `build.sh` (recommended)

The C++ client is built automatically as part of the simulation library build steps:

```bash
# Debug build (sim libs + C++ client)
./build.sh simlibs_debug

# Release build (sim libs + C++ client)
./build.sh simlibs_release

# C++ client only (skips sim libs)
./build.sh cpp_client_debug
./build.sh cpp_client_release
```

Binaries are placed in:
- `client/cpp/build_linux/Debug/hello_drone`
- `client/cpp/build_linux/Release/hello_drone`

### Via `build.cmd` on Windows

Windows builds reuse the existing `ProjectAirsimClientLib` and `HelloDrone` solution artifacts:

```bat
build.cmd cpp_client_debug
build.cmd cpp_client_release
```

Outputs are placed in the existing project locations:
- `client/cpp/libraries/x64/Debug`
- `client/cpp/libraries/x64/Release`
- `client/cpp/example_user_apps/HelloDrone/x64/Debug`
- `client/cpp/example_user_apps/HelloDrone/x64/Release`

### Manual CMake build

```bash
cd client/cpp

# Configure
cmake -S . -B build_linux/Debug   -DCMAKE_BUILD_TYPE=Debug
cmake -S . -B build_linux/Release -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build_linux/Debug   -j$(nproc)
cmake --build build_linux/Release -j$(nproc)
```

### Cleaning

```bash
# Via build.sh (also cleans sim libs)
./build.sh clean

# Manual
rm -rf client/cpp/build_linux
```

---

## Running the Example: HelloDrone

`hello_drone` performs a minimal flight sequence: **arm → takeoff → move up 1 m/s for 4 s → land → disarm**.

### Requirements

- Simulation server must be running (Unreal Editor with ProjectAirSim plugin, or packaged Blocks environment)
- A `sim_config/` directory with scene configuration files (`.jsonc`)

### Command-line options

```
hello_drone [--simhost host_or_ip] [--simconfig sim_config_path]

  --simhost host_or_ip      Hostname or IP of the simulation server (default: "localhost")
  --simconfig sim_config_path  Path to the sim_config directory (default: "sim_config/")
```

### Example

```bash
cd client/cpp/build_linux/Debug

./hello_drone \
  --simhost 127.0.0.1 \
  --simconfig ../../../../client/python/example_user_scripts/sim_config/
```

The sim_config directory must contain a `scene_basic_drone.jsonc` (or equivalent scene file matching your Unreal environment setup). Example configs are in `client/python/example_user_scripts/sim_config/`.

### Expected output

```
Looking for scene file "sim_config/scene_basic_drone.jsonc"
Ready state: ready (...)
TakeoffAsync: starting
TakeoffAsync: completed
Move-Up invoked
Move-Up completed
LandAsync: starting
Landed state: landed
LandAsync: completed
Drone landed.
```

---

## Writing Your Own App

### 1. Minimal CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyDroneApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Point to the pre-built client libraries
set(CPP_CLIENT_ROOT "/path/to/ProjectAirSim/client/cpp")
set(PROJECTAIRSIM_ROOT "/path/to/ProjectAirSim")

# Import the static libraries (already built via build.sh)
add_library(ProjectAirsimClient STATIC IMPORTED)
set_target_properties(ProjectAirsimClient PROPERTIES
    IMPORTED_LOCATION "${CPP_CLIENT_ROOT}/build_linux/Release/libProjectAirsimClient.a"
    INTERFACE_INCLUDE_DIRECTORIES
        "${CPP_CLIENT_ROOT}/ProjectAirsimClientLib/Include;${PROJECTAIRSIM_ROOT}/core_sim/include"
)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE ProjectAirsimClient pthread)
```

> **Tip**: The simplest approach is to add your source files directly to `client/cpp/CMakeLists.txt` alongside `hello_drone` and rebuild via `build.sh cpp_client_debug`.

### 2. Minimal app skeleton

```cpp
#include <ProjectAirsimClient/ProjectAirsimClient.h>
#include <iostream>

namespace pasc = microsoft::projectairsim::client;

int main() {
    // Optional: redirect logs to stdout
    pasc::log.SetLogSink([](pasc::Log::Severity, const char* msg) noexcept {
        std::cout << msg << std::endl;
    });

    // Connect to the running simulation server
    auto client = std::make_shared<pasc::Client>();
    if (client->Connect("localhost") != pasc::Status::OK) return 1;

    // Load scene and initialize world
    auto world = std::make_shared<pasc::World>();
    if (world->Initialize(client, "sim_config/scene_basic_drone.jsonc",
                          "sim_config/", 2.0f) != pasc::Status::OK) return 1;

    // Connect to a drone defined in the scene
    auto drone = std::make_shared<pasc::Drone>();
    if (drone->Initialize(client, world, "Drone1") != pasc::Status::OK) return 1;

    // Arm and enable API control
    bool ok;
    drone->EnableAPIControl(&ok);
    drone->Arm(&ok);

    // Fly!
    drone->TakeoffAsync().Wait();
    drone->MoveByVelocityAsync(0.0f, 0.0f, -1.0f, 3.0).Wait();  // up 1 m/s for 3 s
    drone->LandAsync().Wait();

    drone->Disarm(&ok);
    drone->DisableAPIControl(&ok);
    client->Disconnect();
    return 0;
}
```

### 3. Async operations

All movement commands return an `AsyncResult`. Two usage patterns:

```cpp
// Pattern 1: blocking wait
auto result = drone->TakeoffAsync();
pasc::Status status = result.Wait();  // blocks until done

// Pattern 2: poll + wait
auto result = drone->MoveByVelocityAsync(0, 0, -1, 5.0);
while (!result.FIsDone())
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
status = result.Wait();  // must still call Wait() after FIsDone() returns true
```

---

## Architecture

```
hello_drone (executable)
    └── ProjectAirsimClient (static lib)
            ├── ProjectAirSimMessageLib (static lib)
            │       └── nlohmann-json, msgpack, Eigen3
            └── NNGI (static lib)
                    └── nng (static lib, fetched via FetchContent)
```

All dependencies except `libeigen3-dev` (optional system package) are fetched automatically by CMake FetchContent on first configure.
