# Unreal release tools

These tools provide the public, environment-independent part of the Project
AirSim release build:

- `package_projectairsim_plugins.sh` builds the approved `ProjectAirSim`,
  `Drone`, and `Rover` plugins on Linux.
- `prepare_unreal_environment.py` installs those plugins into one Unreal
  project and updates its Project AirSim packaging configuration.
- `package_unreal_environment.sh` packages that project with Unreal
  `BuildCookRun`.
- `create_release_archive.py` creates the release ZIP, embedded build manifest,
  JSON sidecar, and SHA-256 sidecar.

The scripts intentionally contain no private repository names, environment
branch mapping, credentials, or runner configuration. Release orchestration
belongs in the private Project AirSim repository.

The current automated release target is Linux with Unreal Engine 5.2. The
packaging script validates the engine installed at `UE_ROOT` when
`--expected-ue-version 5.2` is supplied.
