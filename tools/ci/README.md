# Requested PR validation and local checks

## Requesting CI

For PRs targeting `main` or `main_public`, add `run-ci` to request validation.
PR validation does not run on push, PR open/reopen/update, or manual dispatch. Remove/re-add
`run-ci` to validate a newer commit; retaining the label does not rerun tests.

The native sequence is Linux SimLibs on self-hosted Ubuntu 22.04, then selected Windows,
then selected macOS Debug on GitHub-hosted runners. Config/Python checks run first when
relevant. A failure blocks subsequent stages. Documentation builds independently on pushes to main that change README.md,
docs/**, or client/python/projectairsim/docs/**; it is not a PR prerequisite.

To include final Linux regression, first add `run-regressions`, then add
`run-ci`. Adding `run-regressions` alone does not run jobs. With both labels,
regression runs after Windows, macOS, and all other relevant checks succeed.
This full request overrides native file filters, so even docs-only changes can
request the complete pipeline. Without that label, file filters avoid native
builds for unrelated changes and no final regression runs.

`pr_ci_orchestrator.yml` calls reusable stages for change detection, configs,
Python matrix/result, initial Linux, Windows, macOS, final Linux,
and result reporting. Each stage owns its runner. macOS compiles only
Debug without tests. No tests rerun on merge.

## Build configurations

Initial Linux uses UE 5.2 and builds/tests SimLibs Debug only. Selected Windows
builds the standalone C++ client Debug, then SimLibs Debug without tests.
Selected macOS builds SimLibs Debug without tests. Both platform labels are
optional; macOS waits for Windows when both are selected.

Final Linux uses a clean UE 5.7 checkout. SimLibs unit tests, standalone C++
build/tests and its package-consumer check, and ROS2 build/tests all use Debug.
Only packaged integration additionally builds SimLibs Release and Blocks Shipping.
Unreal reuses that same job's Release libraries without another SimLibs build.
The simulator is launched from its Shipping package. Linux uploads no artifacts.

`run-regressions` replaces the former regression label. Add it before run-ci
for final regression. No stage-resume shortcut is supported.

## Local checks

Use a virtual environment with `projectairsim[datacollection]`, `pytest`, and
`PyYAML` installed. From the repository root:

```text
python -W error::SyntaxWarning -m compileall -q -f client/python/projectairsim/src client/python/projectairsim/tests client/python/example_user_scripts
python -m unittest discover -s tools/ci -p test_validate_configs.py -v
python tools/ci/check_layered_ci_flows.py
# Requires CMake and Ninja; checks Unity gating with isolated fixture artifacts.
python tools/ci/check_unity_build_opt_in.py
# On Linux/WSL with GNU Make and Bash:
python tools/ci/check_linux_build_reuse.py
python tools/ci/validate_configs.py <changed JSON/JSONC paths>
```

To validate every tracked config from Windows or Linux without GitHub Actions:

```python
import subprocess
import sys

paths = subprocess.check_output(
    ["git", "ls-files", "*.json", "*.jsonc"], text=True
).splitlines()
subprocess.run([sys.executable, "tools/ci/validate_configs.py", *paths], check=True)
```

From `client/python/projectairsim/tests`, run the offline client tests:

```text
python -m pytest --sim-host offline -q test_json_schema.py test_datacollection/test_datacollector_config.py
```

The validator accepts UTF-8 with or without a BOM. Recognized JSON Schema
meta-schema declarations are checked as schemas before filename classification.
The three negative fixtures in `test_json_schema.py` have an exact-path allowlist:
each must still fail with its expected validation keyword. Invalid syntax,
missing references, another error kind, an unexpectedly valid fixture, or the
same filename elsewhere still fails validation.

The flow checker reads the actual job conditions and evaluates the expression
and glob subset used here. It tests events, labels, component selection, failures,
reusable-workflow triggers, and cache inputs. It is not the complete GitHub
Actions runtime and does not execute builds. Use `actionlint` for syntax checks.
Passing these local checks does not claim native builds or Unreal/ROS integration.

Unreal roots are read from host UE_ROOT_5_2/UE_ROOT_5_7, with bounded interactive
bashrc fallback when absent from the runner service environment. No UE secrets
are used. ROS tooling is reused when installed; provisioning needs noninteractive
sudo and network access. The setup scripts are sourced before enabling nounset.

## Python regression hosts

The final Linux job runs three disjoint suites. Every test must declare exactly
one of `offline`, `runtime`, `unreal`, or the existing optional `px4` marker.
Unclassified or multiply classified tests fail collection.

1. Offline checks run without a simulator.
2. PAS Runtime runs control, clock/step, pose/kinematics, battery, non-rendered
   sensor, connection, and client CPU checks before packaging Unreal. Runtime
   configurations exclude rendered sensors and retain fresh scenes for control
   and clock isolation. The runner owns and cleans up only its Runtime process.
3. Unreal runs rendering, image/pose correspondence, world geometry, materials,
   weather, occupancy-based trajectory planning, and every LiDAR variant.
   A session fixture composes the existing sensor configurations into one
   scene. Test clients bind that scene without reloading it. The suite rejects
   a second LoadScene RPC and reports the measured load count.

See [Python regression instructions](../../client/python/projectairsim/tests/README.md)
for local commands, shared-state isolation, and performance interpretation.
