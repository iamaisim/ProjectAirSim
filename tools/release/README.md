# Unreal release tools

These tools provide the small, public, environment-independent interface used
by the private Project AirSim release orchestrator:

- `prepare_unreal_environment.py` installs those plugins into one Unreal
  project and updates its Project AirSim packaging configuration.
- `create_release_archive.py` creates the release ZIP, embedded build manifest,
  JSON sidecar, and SHA-256 sidecar.

Plugin compilation and packaging continue to use the repository's existing
`build.sh package_plugin` and `build.cmd package_plugin` targets. These tools do
not introduce another build path.

Private repository names, environment branch mappings, credentials, runner
configuration, platform selection, and Unreal invocation belong in the private
orchestrator repository.
