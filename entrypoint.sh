#!/bin/bash
# Entrypoint for the projectairsim-dev image.
#
# Phase 2 (Unreal Engine one-time setup) runs HERE, at container runtime, against the
# bind-mounted engine checkout -- NOT during `docker build`. Reasons:
#   - It writes 15-20 GB of generated content; baking that into image layers defeats the
#     purpose of bind-mounting the engine source in the first place.
#   - Setup.sh/GenerateProjectFiles.sh need no GitHub/Epic credentials (only the initial
#     `git clone` of the engine repo does, and that already happened on the host). Setup.sh's
#     GitDependencies tool fetches prebuilt binaries from Epic's dependency CDN over plain
#     HTTPS, so this step is safe to automate with no secrets involved.
#   - It is idempotent: skipped automatically on every subsequent container start once the
#     markers below exist, so only the first run pays the download/generation cost.
set -e

UE_SETUP_MARKER="${UE_ROOT}/Engine/Build/OneTimeSetupPerformed"
UE_PROJECTFILES_MARKER="${UE_ROOT}/Engine/Intermediate/ProjectFiles"

run_ue_phase2() {
    if [ -z "$UE_ROOT" ] || [ ! -f "${UE_ROOT}/Setup.sh" ]; then
        echo " [phase2] UE_ROOT ('$UE_ROOT') not mounted or missing Setup.sh -- skipping engine setup."
        return
    fi

    if [ -f "$UE_SETUP_MARKER" ]; then
        echo " [phase2] Setup.sh: already completed (marker found) -- skipping."
    else
        echo " [phase2] Setup.sh --force: first run, downloads bundled Clang toolchain + .NET + prebuilt"
        echo "          binaries (~15-20 GB over HTTPS, no credentials needed). This can take a while..."
        ( cd "$UE_ROOT" && ./Setup.sh --force )
        echo " [phase2] Setup.sh: done."
    fi

    if [ -d "$UE_PROJECTFILES_MARKER" ] && [ -n "$(ls -A "$UE_PROJECTFILES_MARKER" 2>/dev/null)" ]; then
        echo " [phase2] GenerateProjectFiles.sh: already completed -- skipping."
    else
        echo " [phase2] Running GenerateProjectFiles.sh..."
        ( cd "$UE_ROOT" && ./GenerateProjectFiles.sh )
        echo " [phase2] GenerateProjectFiles.sh: done."
    fi
}

run_ue_phase2

if [ -t 1 ]; then
    cat <<EOF
================================================================================
 Project AirSim dev container
--------------------------------------------------------------------------------
 UE_ROOT            = ${UE_ROOT}
 ProjectAirSim repo  = /workspace/ProjectAirSim   (bind-mounted)
 Unreal Engine 5.2   = ${UE_ROOT} (bind-mounted)

 Phase 1 (system prerequisites) and Phase 2 (engine Setup.sh + GenerateProjectFiles.sh,
 idempotent) are already handled automatically by this image/entrypoint.

 Phase 3 - Build SimLibs + Blocks:
   cd /workspace/ProjectAirSim
   ./build.sh simlibs_debug
   ./build.sh blocks_debuggame

 Phase 4 - Launch the Editor (requires DISPLAY + Vulkan/X11 passthrough, see docker-compose.yml):
   \$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh ShaderCompileWorker Linux Development  # once
   \$UE_ROOT/Engine/Binaries/Linux/UnrealEditor-Linux-DebugGame \\
     /workspace/ProjectAirSim/unreal/Blocks/Blocks.uproject
   # NOTE: blocks_debuggame builds "UnrealEditor-Linux-DebugGame", not plain "UnrealEditor".
   # See docs/docker.md for the full troubleshooting history of this environment.

 Run a client (from the host or another container with the ports reachable):
   pixi run hello-drone
================================================================================
EOF

    echo " GPU / Vulkan check:"
    if command -v nvidia-smi >/dev/null 2>&1 && nvidia-smi >/dev/null 2>&1; then
        echo "   nvidia-smi : OK ($(nvidia-smi --query-gpu=name --format=csv,noheader))"
    else
        echo "   nvidia-smi : NOT AVAILABLE -- did you run with --gpus all?"
    fi
    if command -v vulkaninfo >/dev/null 2>&1; then
        gpu_line=$(vulkaninfo --summary 2>/dev/null | grep -m1 "deviceName")
        if [ -n "$gpu_line" ]; then
            echo "   vulkaninfo : OK ($(echo "$gpu_line" | awk -F'= ' '{print $2}'))"
        else
            echo "   vulkaninfo : FAILED -- check --gpus all, --device /dev/dri, and"
            echo "                NVIDIA_DRIVER_CAPABILITIES includes graphics,display,utility"
        fi
    fi
    echo "================================================================================"
fi

exec "$@"
