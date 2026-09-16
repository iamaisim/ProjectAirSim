# Python regressions by simulator host

Install `projectairsim[datacollection]`, `pytest`, `pytest-timeout`, and `psutil`
in the active Python environment. Run from this directory:

```sh
python -m pytest --sim-host offline -v --timeout=120
python -m pytest --sim-host runtime -v --timeout=180
python -m pytest --sim-host unreal -v --timeout=180
```

The last two commands require the corresponding simulator on localhost ports
8989/8990. Run them sequentially, never against the same live server in parallel.
From the repository root, Runtime can instead be managed automatically:

```sh
python tools/ci/run_runtime_regressions.py --runtime /path/to/projectairsim-runtime
```

The runner refuses occupied ports and always stops its own Runtime process,
including after a failing pytest run. CI executes offline and Runtime checks
before building/starting Blocks Shipping, then runs the Unreal partition.

Every test declares one host marker. Collection rejects missing or conflicting
markers, and `--collect-only --sim-host HOST` lists each partition without
starting a simulator. The default host is `unreal`; use `--sim-host offline`
even when selecting an individual offline test file. PX4 remains opt-in via
`--sim-host px4 -m px4 px4_test_sitl.py` and requires its existing SITL setup.

## Runtime coverage

Control and flight commands, connection lifecycle, simulation clocks, stepping,
kinematics, pose, battery, wind, IMU/GPS/barometer/magnetometer/airspeed, and
client CPU profiling use Runtime. The hello-drone smoke checks flight and IMU;
image content and image/pose correspondence remain covered by Unreal tests. The test-only `RegressionWorld` removes
rendered sensors from these configurations before sending them to the server.
It preserves fresh scenes for cases that change controllers, clocks, or battery
configuration. Production client scene-loading behavior is unchanged.

## One Unreal scene

`regression_support.unreal_scene()` composes the existing fixtures into
`SceneUnrealRegression`: one drone with standard and annotation
cameras; CPU, GPU, depth, Avia, and Mid70 LiDARs; and the data-collection
environment actor. Sensor settings remain sourced from their original files, with a documented
combined workload: CPU LiDAR uses 100,000 points/s instead of the isolated
1,000,000-point stress setting. Camera benchmarks reuse DownCamera instead of
adding duplicate captures. The annotation camera captures every 100 ms;
DownCamera keeps its original interval and all throughput floors are unchanged.
The streaming view starts at 640x480, matching CI, so switching views does not
change the viewport size halfway through the suite. The shared geodetic origin matches the Blocks trajectory
fixtures. Each test client attaches to this scene without a load/unload cycle.

A session-wide RPC guard rejects a second `LoadScene` request, including one
hidden inside data-collection helpers. The test report prints the observed
load count. Planning still requests real Unreal voxel grids; it does not
replace occupancy or rendering with mock data.

Before and after each Unreal case, the fixture restores drone pose/control,
temporary objects, landmark scale/pose, weather, wind, camera pose/FOV,
segmentation, lighting, and the environment actor's stationary trajectory.
Material tests use disposable actors. URL texture tests use a local HTTP
server, and generated trajectories are copied for each test to avoid order
dependencies. A failed test still runs cleanup.

Camera and LiDAR throughput now measure the combined regression scene, not
isolated sensor scenes. Keep that workload distinction when comparing earlier
benchmark results. Existing throughput and detection assertions remain active;
request/reply depth benchmarking requests depth images rather than RGB.

The one-scene requirement covers this Python Unreal regression suite. Native
SimLibs/ROS unit tests and separately opted-in PX4 tests retain their own setup.
