# Experimental Unity Host Reference

## Status

The Unity integration in this directory is experimental, unmaintained reference
code. IAMAI does not currently validate, package, release, or provide support
for it. It is not part of the Project AirSim supported-platform matrix, and
compatibility with the current Project AirSim `main` branch or current Unity
versions is not guaranteed.

The checked-in example project records Unity Editor version `2020.3.36f1`.

## Purpose

This code is retained because it demonstrates how the engine-independent
Project AirSim simulation libraries can be hosted by a 3D engine other than
Unreal Engine. It is useful as an architectural reference for evaluating or
developing additional simulation hosts.

## Opt-in native builds

Normal SimLibs builds do **not** compile `sim_unity_wrapper` or stage Unity
plugin dependencies. Add `--unity` to explicitly include those artifacts:

```powershell
# Windows: run from the repository root
.\build.cmd --unity simlibs_release
.\build.cmd --unity simlibs_debug
```

```bash
# Linux/macOS: run from the repository root
./build.sh --unity simlibs_release
./build.sh --unity simlibs_debug
```

The flag can appear before or after the target. It includes the Unity wrapper
and its dependency staging under `unity/BlocksUnity/Assets/Plugins`; shared
SimLibs needed by Runtime and Unreal still build normally without the flag.
It does not build the Unity Editor project or imply supported Unity compatibility.

Each build-script invocation defaults back to Unity disabled, even when reusing
a build directory previously configured with `--unity`. Existing plugin files
are left on disk; disabling Unity prevents new builds/copies, not deletion.

For direct CMake use, the equivalent option is
`-DPROJECTAIRSIM_BUILD_UNITY=ON`. It defaults to `OFF` in a new cache. CMake
remembers explicit options, so pass `-DPROJECTAIRSIM_BUILD_UNITY=OFF` to disable
Unity again when configuring directly. Direct Make/NMake users can pass
`PAS_BUILD_UNITY=ON` (default `OFF`).

## Contents

- `BlocksUnity/`: Unity example project, scenes, assets, and C# host code.
- `sim_unity_wrapper/`: native wrapper that exposes Project AirSim simulation
  functionality to the Unity host.

This directory should not be treated as a production-ready Unity package or as
evidence of feature parity with the maintained Unreal Engine host.

---

Copyright (C) 2026 IAMAI CONSULTING CORP

MIT License
