# Changelog

All notable changes to this project will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]

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

[Unreleased]: https://github.com/iamaisim/ProjectAirSim/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/iamaisim/ProjectAirSim/compare/v0.1.1...v0.2.0
[0.1.1]: https://github.com/iamaisim/ProjectAirSim/releases/tag/v0.1.1