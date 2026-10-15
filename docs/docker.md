# Running Project AirSim with Unreal Engine in Docker

This page documents a Docker-based development environment for building and running
**Project AirSim SimLibs + the Unreal Engine 5.2 `Blocks` environment** on Linux, with
NVIDIA GPU-accelerated Vulkan rendering for the Editor. It covers the three files that
implement it (`Dockerfile`, `docker-compose.yml`, `entrypoint.sh`, all at the repo root),
why they are built the way they are, and the real problems encountered (and fixed) while
getting the Editor to actually render inside a container.

This is a supplemental, container-based alternative to
**[Developer Initial Setup for Linux](development/dev_setup_linux.md)** and
**[Build From Source as a Developer](development/use_source.md)** -- it automates the same
build steps, inside Ubuntu 22.04, without installing anything on the host besides Docker
and the NVIDIA Container Toolkit.

---

## Prerequisites (host)

- Linux host with an NVIDIA GPU, proprietary driver, and the
  **[NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)**
  installed and configured for Docker (`docker info` should list `nvidia` under `Runtimes`).
- A Unreal Engine **5.2** source checkout matching `unreal/Blocks/Blocks.uproject`'s
  `"EngineAssociation": "5.2"`. This is cloned from Epic's private GitHub repo on the host
  (requires a GitHub account linked to Epic Games), **not** inside the container -- see
  [Why the engine isn't cloned by the Dockerfile](#why-the-engine-isnt-cloned-by-the-dockerfile).
- A clone of this repository.
- An X server reachable from the container (XWayland works) if you want the Editor's GUI
  window; run `xhost +local:docker` once per host login session.

## Files

| File | Purpose |
|---|---|
| `Dockerfile` | Ubuntu 22.04 image: build toolchain, Vulkan rendering runtime libraries, pixi, non-root user matching the host UID/GID. |
| `docker-compose.yml` | Bind-mounts the ProjectAirSim repo and the Unreal Engine checkout, wires up GPU passthrough, X11 forwarding, and host networking. |
| `entrypoint.sh` | Runs Phase 2 (engine `Setup.sh`/`GenerateProjectFiles.sh`) idempotently against the bind-mounted engine on container start, then prints a usage banner and a GPU/Vulkan self-check. |
| `.env.example` | Template for the two host-specific path variables (`PROJECTAIRSIM_HOST_PATH`, `UE_ROOT_HOST_PATH`). Copy to `.env` (gitignored) and edit before first use. |

## Quick start

```bash
cd ProjectAirSim

# One-time: tell Compose where your Unreal Engine checkout lives.
cp .env.example .env
# edit .env and set UE_ROOT_HOST_PATH to your Unreal Engine 5.2 checkout path

# Once per host login session, so the container can draw to your X/XWayland display:
xhost +local:docker

docker compose build
docker compose run --rm projectairsim-dev
```

The first `docker compose run` performs the one-time engine setup automatically (see
[Phase 2](#phase-2-engine-setup-automatic-idempotent) below) and can take a while
(~15-20 GB download). It prints a banner with the exact next commands, and a GPU/Vulkan
self-check so you know immediately if GPU passthrough is working:

```text
 GPU / Vulkan check:
   nvidia-smi : OK (NVIDIA GeForce RTX 5070 Laptop GPU)
   vulkaninfo : OK (NVIDIA GeForce RTX 5070 Laptop GPU)
```

From there, inside the container:

```bash
# Phase 3: build SimLibs + the Blocks Unreal project (one-time per source tree; see below)
cd /workspace/ProjectAirSim
./build.sh simlibs_debug
./build.sh blocks_debuggame

# Phase 4: build the shader compiler (once) and launch the Editor
$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh ShaderCompileWorker Linux Development
$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Linux-DebugGame \
  /workspace/ProjectAirSim/unreal/Blocks/Blocks.uproject
```

From the host (or another container that can reach `localhost:8989`/`8990`):

```bash
pixi run hello-drone
```

---

## Design decisions and why

### Bind mounts, not `COPY`

Neither the ProjectAirSim repo nor the Unreal Engine checkout are copied into the image.
Both are bind-mounted, with host-side paths coming from a `.env` file (`cp .env.example
.env`, then set `UE_ROOT_HOST_PATH`) rather than being hardcoded in `docker-compose.yml`:

```yaml
- ${PROJECTAIRSIM_HOST_PATH:-.}:/workspace/ProjectAirSim   # defaults to "." (this repo)
- ${UE_ROOT_HOST_PATH:?...}:/workspace/UnrealEngine         # required, no safe default
```

`PROJECTAIRSIM_HOST_PATH` defaults to `.` since `docker-compose.yml` lives at the repo root,
so the common case needs no configuration. `UE_ROOT_HOST_PATH` has no universal default
(checkout location and engine version vary per machine) and Compose fails immediately with a
clear message if it isn't set, rather than silently mounting the wrong thing.

This matters for more than convenience:

- The engine checkout plus its generated content (`Setup.sh`, `GenerateProjectFiles.sh`)
  is **15-20+ GB**. Baking that into image layers would make every `docker build` slow and
  every rebuild re-download it.
- `build.sh`'s `CMAKE_BUILD_DIR` (see `build_linux.mk`) and Unreal's own `Intermediate`/
  `Binaries` output directories are **inside these mounted trees**, not in the container's
  ephemeral writable layer. That means build artifacts persist across container restarts
  exactly like a normal host build -- see [Phase 3](#phase-3-build-simlibs--blocks-manual-one-time-per-source-tree).

### Why the engine isn't cloned by the Dockerfile

Cloning `EpicGames/UnrealEngine` requires a GitHub account linked to Epic Games. Doing this
as a `docker build` `RUN` step would mean either:

- baking a GitHub credential/SSH key into image layers (a real secret-leak risk any time
  the image is shared), or
- failing outright, since a plain `docker build` has no access to the host's SSH agent or
  credential helper.

So the engine clone happens **once, manually, on the host**, matching `Blocks.uproject`'s
`"EngineAssociation": "5.2"`. The Dockerfile only assumes an already-cloned checkout is
bind-mounted in.

**`Setup.sh`/`GenerateProjectFiles.sh`, however, need no GitHub credentials at all** --
`Setup.sh`'s `GitDependencies` tool fetches prebuilt binaries from Epic's own dependency CDN
over plain HTTPS, not via `git`/GitHub auth. That is why those two steps *are* safe to
automate inside the container (see Phase 2 below) with zero secrets involved.

### Why not build `FROM` an existing AirSim-related image

An image already on this host, `ghcr.io/soterinc/airsim-cameras`, has Vulkan + a build
toolchain preinstalled and looked like a shortcut. It was deliberately **not** used as a
base, because inspecting its build history (`docker history`) showed:

- It is `FROM ubuntu:22.04` itself -- no head start over building from scratch.
- It bakes in **Cosys-AirSim** (a different, unrelated AirSim fork) plus a GStreamer/RTSP
  streaming stack -- none of it applicable to this repository's Unreal plugin.
- Its `clang` resolves to clang-14 with no `libc++-13-dev`/`libc++abi-13-dev` at all --
  this repo's root `CMakeLists.txt` hardcodes `clang-13`/`clang++-13` by name when `UE_ROOT`
  is unset, so the same symlink workaround would be needed regardless.
- It is a private registry image; a Dockerfile that only works because of a pre-cached
  private image isn't reproducible for anyone else.

### No NVIDIA driver or CUDA packages in the image

The image installs only `libvulkan1`/`vulkan-tools` (the Vulkan *loader*), never an NVIDIA
driver or CUDA toolkit package. This was verified directly: running a **stock**
`ubuntu:22.04` container with `--gpus all` and no driver packages installed already gets
`libcuda.so`, `libnvidia-ml.so`, and the Vulkan ICD JSON mounted in by the NVIDIA Container
Toolkit at `docker run` time. Baking a driver into the image would risk a version mismatch
against the host's kernel module (NVIDIA requires the two to match exactly), which would
*break* GPU access rather than help it.

---

## GPU / Vulkan passthrough: what's actually required

Getting `vulkaninfo` (and therefore the Unreal Editor) to see the GPU inside the container
took real debugging, not just `--gpus all`. The required `docker run`/compose flags:

```bash
--gpus all
--device /dev/dri
-e NVIDIA_DRIVER_CAPABILITIES=compute,utility,graphics,display,video
-e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix
```

### The non-obvious part: `libegl1` and `libxext6` are required even for pure Vulkan

With only `libvulkan1`/`vulkan-tools` installed, `vulkaninfo` failed with:

```text
ERROR: [Loader Message] Code 0 : loader_scanned_icd_add: Could not get 'vkCreateInstance'
via 'vk_icdGetInstanceProcAddr' for ICD libGLX_nvidia.so.0
Cannot create Vulkan instance.
```

This looked like a broken driver/ICD, but the device nodes, ICD JSON, and driver `.so`
files were all present and correct (confirmed with `nm -D` showing the expected exported
symbol at the identical address as the host's copy). `strace -f -e trace=openat` on
`vulkaninfo` revealed the real cause: NVIDIA's combined `libGLX_nvidia.so` ICD internally
`dlopen()`s `libEGL.so.1` during `vkCreateInstance` -- without it present, instance creation
silently fails. The Vulkan loader itself also links `libXext.so.6`. Adding `libegl1 libgl1
libxext6` to the image fixed it completely; `vulkaninfo --summary` then correctly reports
the real GPU:

```text
deviceType  = PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
deviceName  = NVIDIA GeForce RTX 5070 Laptop GPU
driverName  = NVIDIA
```

---

## The four build/run phases

### Phase 1: system prerequisites (`Dockerfile`, build time)

Everything from `build-essential` through `vulkan-tools`, `python3-venv`/`python3-pip`,
plus the GUI/Vulkan runtime libraries described above. Baked into the image; never repeated.

### Phase 2: engine setup (automatic, idempotent)

`entrypoint.sh` runs `Setup.sh --force` and `GenerateProjectFiles.sh` against the
bind-mounted `$UE_ROOT` **every time the container starts**, but checks for completion
markers first and skips what's already done:

| Step | Marker checked |
|---|---|
| `Setup.sh --force` | `$UE_ROOT/Engine/Build/OneTimeSetupPerformed` |
| `GenerateProjectFiles.sh` | `$UE_ROOT/Engine/Intermediate/ProjectFiles` (non-empty) |

So only the very first container run pays the ~15-20 GB download/generation cost; every
subsequent run (or a fresh container against the same bind-mounted engine) skips straight
past it. If `UE_ROOT` isn't mounted or doesn't contain `Setup.sh`, this step is skipped with
a warning instead of failing the container.

### Phase 3: build SimLibs + Blocks (manual, one-time per source tree)

```bash
cd /workspace/ProjectAirSim
./build.sh simlibs_debug
./build.sh blocks_debuggame
```

This is **not** required on every `docker run`. Because the build output directories
(`build/linux64/<UE-dirname>/...` for SimLibs, `unreal/Blocks/Intermediate`,
`unreal/Blocks/Binaries`, `unreal/Blocks/Plugins/ProjectAirSim/SimLibs` for Blocks) are all
inside the bind-mounted repo, they persist on the host disk, not in the container's
ephemeral layer. CMake/Ninja and UnrealBuildTool are both incremental build systems, so:

- Starting a brand-new container against the same bind-mounted paths sees the finished
  build and does a fast no-op check.
- Editing a `.cpp` file and re-running `./build.sh simlibs_debug` only recompiles what
  changed -- not a full rebuild.
- The only way to lose this is deleting `build/` or `unreal/Blocks/Intermediate`, or if the
  compose file stopped bind-mounting these paths.

`blocks_debuggame` produces a **`DebugGame`** editor binary, named
`UnrealEditor-Linux-DebugGame` -- not plain `UnrealEditor` (see Phase 4).

### Phase 4: build `ShaderCompileWorker` and launch the Editor (manual)

```bash
$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh ShaderCompileWorker Linux Development
$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Linux-DebugGame \
  /workspace/ProjectAirSim/unreal/Blocks/Blocks.uproject
```

`ShaderCompileWorker` is a separate Unreal target the Editor shells out to for shader
compilation; it is not produced by `blocks_debuggame` and must be built once. Add `-game`
to the Editor command to boot straight into Play/Simulation mode.

---

## Adding a second Unreal environment besides Blocks

Phases 1, 2, and `simlibs_debug` are environment-agnostic -- they never need to be repeated
for additional Unreal projects. To add a new environment (`MyCity.uproject`) alongside
`Blocks`, see **[Using Project AirSim Plugin in Custom Environments](use_plugin.md)**:

1. Copy `unreal/Blocks/Plugins/` into the new project's root (or use `./build.sh
   package_plugin` for a clean copy without `Binaries`/`Intermediate`).
2. Set `GlobalDefaultGameMode=/Script/ProjectAirSim.ProjectAirSimGameMode` in the new
   project's `Config/DefaultEngine.ini`.
3. Ensure the project has a `Source/` folder (File -> New C++ Class... -> None, if not).
4. Build just that project against the already-built Editor:
   ```bash
   $UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh MyCity Linux DebugGame -project=/path/to/MyCity.uproject
   $UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh MyCityEditor Linux DebugGame -project=/path/to/MyCity.uproject
   ```

This takes a few minutes instead of the ~1-2 hours the first environment costs, since the
Editor and `ShaderCompileWorker` are already built.

---

## Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `bash: .../UnrealEditor: No such file or directory` | `blocks_debuggame` builds `UnrealEditor-Linux-DebugGame`, not `UnrealEditor` | Use the `-Linux-DebugGame` binary name (see Phase 4). |
| `vkCreateInstance` / `ERROR_INCOMPATIBLE_DRIVER` from `vulkaninfo`, even with `--gpus all` working | `libegl1`/`libxext6` missing -- NVIDIA's ICD `dlopen()`s `libEGL.so.1` internally | Install `libegl1 libgl1 libxext6` (already in the Dockerfile). |
| `dlopen failed: libnss3.so` + blocking "Plugin 'Bridge' failed to load" dialog that then exits the Editor | Epic's built-in Quixel Bridge plugin's embedded Chromium UI needs Mozilla NSS; it's enabled engine-wide even though it's unrelated to ProjectAirSim | Install `libnss3` (already in the Dockerfile), or run `apt-get install -y libnss3` inside an already-running container without rebuilding. |
| `nvidia-smi: NOT AVAILABLE` in the entrypoint banner | Container started without GPU flags | Re-run with `--gpus all --device /dev/dri -e NVIDIA_DRIVER_CAPABILITIES=compute,utility,graphics,display,video`. |
| Editor window doesn't appear / X11 connection refused | `xhost` not granted for the container | Run `xhost +local:docker` on the host once per login session. |
| `Setup.sh` runs again on every container start | `UE_ROOT`'s `Engine/Build/OneTimeSetupPerformed` marker missing, or the bind mount isn't pointing at the same host path each time | Confirm `docker-compose.yml`'s engine volume path matches your actual checkout; the marker is written inside that bind-mounted tree, so it should persist. |

---

Copyright (C) 2025 IAMAI CONSULTING CORP

MIT License. All rights reserved.
