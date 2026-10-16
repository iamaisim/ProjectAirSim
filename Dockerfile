# ---------------------------------------------------------------------------------------------------------------------
#
# Dockerfile - Development image for building & running Project AirSim with Unreal Engine 5.2 on Linux.
#
# This image installs the OS-level toolchain only. The Unreal Engine source checkout and the
# ProjectAirSim repo are NOT copied into the image (they are large and you already have them on
# the host) -- bind-mount them instead via docker-compose.yml, which reads host paths from a
# .env file (copy .env.example to .env and set UE_ROOT_HOST_PATH first):
#
#   Host (from .env)        -> Container
#   PROJECTAIRSIM_HOST_PATH -> /workspace/ProjectAirSim   (defaults to ".")
#   UE_ROOT_HOST_PATH       -> /workspace/UnrealEngine     (required, no default)
#
# Why Ubuntu 22.04: it is Project AirSim's primary supported Linux platform and matches the
# self-hosted CI runner (see docs/development/use_source.md, tools/ci/README.md).
#
# Build (prefer `docker compose build`, which also wires up .env/GPU/X11 -- see
# docker-compose.yml and docs/docker.md):
#   docker build -t projectairsim-dev --build-arg USER_ID=$(id -u) --build-arg GROUP_ID=$(id -g) .
#
# ---------------------------------------------------------------------------------------------------------------------

FROM ubuntu:22.04

ARG USER_ID=1000
ARG GROUP_ID=1000
ARG USERNAME=dev

ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8 \
    TZ=UTC

# ---------------------------------------------------------------------------------------------------------------------
# System prerequisites
#   - build-essential/rsync/make/cmake/ninja-build : required by build.sh / build_linux.mk
#   - clang-13/libc++-13-dev/libc++abi-13-dev       : fallback toolchain for building WITHOUT UE_ROOT
#                                                       (unused for the UE_ROOT path below, Unreal
#                                                       brings its own bundled Clang -- see
#                                                       docs/development/dev_setup_linux.md)
#   - libeigen3-dev                                 : physics math dependency (core_sim/physics)
#   - libvulkan1/vulkan-tools/mesa-vulkan-drivers    : Vulkan loader + vulkaninfo diagnostics.
#     The actual NVIDIA driver/ICD is NOT installed here -- it is injected at `docker run` time
#     by the NVIDIA Container Toolkit from the host (verified: a stock ubuntu:22.04 container with
#     --gpus all already gets libcuda.so/libnvidia-ml.so/the Vulkan ICD mounted in with zero
#     driver packages installed). Do NOT add nvidia-driver-* or cuda-toolkit-* here -- a
#     driver/userspace mismatch vs. the host kernel module breaks GPU access.
#   - libegl1/libgl1/libxext6                       : REQUIRED even for pure Vulkan rendering.
#     Verified by strace: NVIDIA's combined libGLX_nvidia.so ICD dlopen()s libEGL.so.1 internally
#     during vkCreateInstance, and the Vulkan loader itself links libXext.so.6. Without these two,
#     vulkaninfo fails with "Could not get 'vkCreateInstance' via 'vk_icdGetInstanceProcAddr'"
#     even though the ICD, device nodes, and driver libraries are all present and correct.
#   - git                                           : UnrealBuildTool / GitDependencies invoke git
#   - python3/python3-venv/python3-pip               : Project AirSim Python client
#   - GUI/X11/XWayland runtime libs                 : required to run UnrealEditor with a window
#     forwarded from the host (libx11-6, libxcursor1, libxinerama1, libxi6, libxrandr2, libxss1,
#     libxtst6, libsm6, libasound2, libgtk-3-0)
#   - libnss3                                       : Mozilla NSS, required by the built-in Quixel
#     Bridge plugin's embedded Chromium (CEF) browser UI. Without it the Editor shows a blocking
#     "Plugin 'Bridge' failed to load" dialog on startup (Bridge is an Epic default plugin, not a
#     ProjectAirSim dependency, but it is enabled engine-wide and still attempts to load).
#   - sudo                                          : UE's Setup.sh may invoke `sudo apt-get`
#     if it thinks a dependency is missing; harmless no-op here since deps are preinstalled.
# ---------------------------------------------------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential rsync make cmake ninja-build \
        clang-13 libc++-13-dev libc++abi-13-dev \
        libeigen3-dev \
        libvulkan1 vulkan-tools mesa-vulkan-drivers \
        git git-lfs ca-certificates curl wget gnupg lsb-release \
        python3 python3-venv python3-pip \
        libx11-6 libxext6 libxcursor1 libxinerama1 libxi6 libxrandr2 libxss1 libxtst6 \
        libsm6 libasound2 libgtk-3-0 libegl1 libgl1 libnss3 \
        sudo \
    && rm -rf /var/lib/apt/lists/*

# Symlink the system Clang 13 package to the names Project AirSim's CMakeLists.txt expects
# when UE_ROOT is not set (see root CMakeLists.txt + AGENTS.md for this exact workaround).
RUN ln -sf /usr/bin/clang-13 /usr/bin/clang-13 \
    && ln -sf /usr/bin/clang++-13 /usr/bin/clang++-13

# ---------------------------------------------------------------------------------------------------------------------
# Non-root user matching the host UID/GID so bind-mounted files stay writable from outside.
# ---------------------------------------------------------------------------------------------------------------------
RUN (getent group ${GROUP_ID} || groupadd -g ${GROUP_ID} ${USERNAME}) \
    && (getent passwd ${USER_ID} || useradd -m -u ${USER_ID} -g ${GROUP_ID} -s /bin/bash ${USERNAME}) \
    && echo "${USERNAME} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${USERNAME} \
    && chmod 0440 /etc/sudoers.d/${USERNAME}

# ---------------------------------------------------------------------------------------------------------------------
# Pixi (used by the repo's pixi.toml tasks: `pixi run hello-drone`, etc.)
# ---------------------------------------------------------------------------------------------------------------------
RUN curl -fsSL https://pixi.sh/install.sh | PIXI_HOME=/usr/local/pixi bash \
    && ln -s /usr/local/pixi/bin/pixi /usr/local/bin/pixi

ENV UE_ROOT=/workspace/UnrealEngine
WORKDIR /workspace/ProjectAirSim

USER ${USERNAME}
ENV HOME=/home/${USERNAME}

COPY --chown=${USERNAME}:${USERNAME} entrypoint.sh /usr/local/bin/entrypoint.sh
USER root
RUN chmod +x /usr/local/bin/entrypoint.sh
USER ${USERNAME}

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["/bin/bash"]
