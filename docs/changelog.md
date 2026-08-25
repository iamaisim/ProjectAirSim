# Changelog

All notable changes to this project will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]

## [0.3.0] - 2026-08-24
### Added
- JSBSim fixed-wing simulation support, including Cessna 310 and Skywalker X8 examples
- A standalone C++ client package and ROS 2 C++ bridge
- GPU LiDAR 360-degree scanning support and additional LiDAR validation coverage
- Python client `step()` API and CPU-usage tests

### Changed
- Updated the C++ and Python client packages to version 0.3.0
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

[Unreleased]: https://github.com/iamaisim/ProjectAirSim/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/iamaisim/ProjectAirSim/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/iamaisim/ProjectAirSim/compare/v0.1.1...v0.2.0
[0.1.1]: https://github.com/iamaisim/ProjectAirSim/releases/tag/v0.1.1
