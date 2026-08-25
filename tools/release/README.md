# Unreal release tools

These tools provide the public, environment-independent interface used by the
private Project AirSim release orchestrator:

- `package_projectairsim_plugins.sh` builds the approved `ProjectAirSim`,
  `Drone`, and `Rover` plugins on Linux.
- `prepare_unreal_environment.py` installs those plugins into one Unreal
  project and updates its Project AirSim packaging configuration.
- `package_unreal_environment.sh` packages that project with Unreal
  `BuildCookRun`.
- `create_release_archive.py` creates the release ZIP, embedded build manifest,
  JSON sidecar, and SHA-256 sidecar.

Private repository names, environment branch mappings, credentials, and runner
configuration belong in the private orchestrator repository.

The automated release target is Linux with Unreal Engine 5.2. Both packaging
scripts validate the engine installed at `UE_ROOT` when
`--expected-ue-version 5.2` is supplied.
