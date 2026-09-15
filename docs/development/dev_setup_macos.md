# Developer Setup for macOS

## Current support

macOS support currently covers **native SimLibs and headless Project AirSim
Runtime source builds on Apple Silicon (ARM64)**. Release builds and the
SimLibs unit suite were validated on an ARM64 image of the `macos-15` CI runner. Runtime is
linked as part of that build; runtime flight, client integration, and shutdown
are not yet exercised on macOS in CI.

| Component | macOS status |
| --- | --- |
| SimLibs | Release build and unit tests validated on macOS 15 ARM64 |
| Project AirSim Runtime | Native executable built; macOS integration validation pending |
| Unreal plugin and Blocks | Not currently validated or supported on macOS |
| Intel Macs / universal runtime binaries | Not validated |
| Downloadable macOS runtime distribution | Not produced by the macOS CI workflow |

This describes the source build capability, not availability in a published
release. The runtime provides the shared simulation core without Unreal or
rendered sensors; see [Runtime capabilities](../../samples/projectairsim_runtime/README.md).
Individual controller and external simulator integrations require separate
macOS validation.

## Prerequisites

Use an Apple Silicon Mac with Apple's C++ command-line toolchain and Homebrew
available. The tested OS is macOS 15; other macOS versions are not covered by
this validation. Unreal Engine is not required for this build.

From the repository root, install the dependencies:

```bash
chmod +x setup_macos_dev_tools.sh build.sh
./setup_macos_dev_tools.sh
```

The script installs CMake, Ninja, OpenSSL 3, and zlib through Homebrew.

## Build and test

From the repository root:

```bash
./build.sh simlibs_release
./build.sh test_simlibs_release
```

The macOS build uses `build/macos/Release`. Debug targets are also available
as `simlibs_debug` and `test_simlibs_debug`; the CI validation described above
covers Release.

The runtime executable is generated at:

```text
build/macos/Release/samples/projectairsim_runtime/projectairsim-runtime
```

To start a local runtime evaluation from the repository root:

```bash
./build/macos/Release/samples/projectairsim_runtime/projectairsim-runtime
```

This launch command is the next integration validation step; a successful
build alone does not establish that a complete flight or shutdown works on
macOS. Follow the [Runtime guide](../../samples/projectairsim_runtime/README.md)
for client usage and host limitations.

`package_simlibs` stages simulation libraries for custom projects. It does not
create a distributable runtime app or an Unreal/Blocks package for Mac.
