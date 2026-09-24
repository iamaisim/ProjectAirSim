# Changelog

All notable changes to this project will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]

### Added
- Native `AWheeledVehiclePawn` integration with indexed Python, C++, and ROS 2 control and zero-wiring SimpleDrive support

## [1.0.2] - 2026-09-11

### Fixed
- Python camera display failure on `16FC1` depth images: convert only the visualization copy to finite, normalized float32 before OpenCV display, preserving the original metric float16 data ([#211](https://github.com/iamaisim/ProjectAirSim/pull/211), fixes [#210](https://github.com/iamaisim/ProjectAirSim/issues/210))

### Changed
- Updated the project README and vehicle illustration ([#208](https://github.com/iamaisim/ProjectAirSim/pull/208))
- Updated Python and C++ client package versions to 1.0.2

## [1.0.1] - 2026-09-06

### Added
- Configurable Unreal vehicle bridge actuators that map controller outputs to Blueprint parameter indices

### Changed
- Improved Unreal vehicle physics updates driven by Chaos substeps
- Fixed ROS 2 depth image conversion from float16 to float32
- Improved image throughput and added the Python client `step()` API

## [1.0.0] - 2026-09-01
### Added
- Project AirSim Runtime sample application and modular scene configuration support
- Blueprint vehicle integration, including Unreal vehicle client APIs, SimpleDrive examples, and SUV asset installation tools

### Changed
- Expanded integration and setup documentation for the runtime, modular configurations, and Blueprint vehicles
- Updated the C++ and Python client packages to version 1.0.0

### Fixed
- C++ client requests that could wedge because of an uninitialized flag
- Depth image packing to preserve raw FP16 meter values with the `16FC1` format
- Skywalker X8 forest swarm viewport camera behavior

## [0.3.0] - 2026-08-24
### Added
- JSBSim fixed-wing simulation support, including Cessna 310 and Skywalker X8 examples
- A standalone C++ client package and ROS 2 C++ bridge
- GPU LiDAR 360-degree scanning support and additional LiDAR validation coverage
- Python client `step()` API and CPU-usage tests

### Changed
- Improved simulator build, toolchain, and CI support for Unreal Engine 5.7

### Fixed
- Coordinate conversion precision in the Python ROS bridge
- GPU LiDAR behavior and standalone simulator builds on Unreal Engine 5.7

## [0.2.0] - 2026-05-29
### Added
- Unreal Engine 5.7 support
- Build commit hash service and client helper API
- Engine-driven and external simulation clock modes, including schema and demo updates
- DepthLiDAR sensor support with UE 5.7 compatibility

### Changed
- Relaxed Open3D version constraints
- Improved `UE_ROOT` configuration handling

### Fixed
- Missing dependency handling and Linux dev tool installation issues
- Depth copy behavior from Unreal
- `make_base_specs()` behavior for local file specs
- Namespace and unit-test integration issues

## [0.1.1] - 2025-07-30
### Added
- Core Project AirSim platform baseline

### Fixed
- `__has_feature` macro MSVC compatibility for Windows toolchains

[Unreleased]: https://github.com/iamaisim/ProjectAirSim/compare/v1.0.2...HEAD
[1.0.2]: https://github.com/iamaisim/ProjectAirSim/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/iamaisim/ProjectAirSim/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/iamaisim/ProjectAirSim/compare/v0.3.0...v1.0.0
[0.3.0]: https://github.com/iamaisim/ProjectAirSim/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/iamaisim/ProjectAirSim/compare/v0.1.1...v0.2.0
[0.1.1]: https://github.com/iamaisim/ProjectAirSim/releases/tag/v0.1.1
