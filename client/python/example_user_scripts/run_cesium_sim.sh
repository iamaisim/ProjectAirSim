#!/usr/bin/env bash
# Copyright (C) 2025 IAMAI CONSULTING CORP
# MIT License.
#
# Starts Project AirSim on the Cesium (GISMap) level, a fresh PX4 SITL with the
# MAVLink gimbal enabled, the MediaMTX RTSP server and px4_gimbal_cesium.py. All
# arguments are passed to px4_gimbal_cesium.py (see `python px4_gimbal_cesium.py
# --help`). Ctrl+C stops everything.
#
#   ./run_cesium_sim.sh                                   # Seletar runway demo
#   ./run_cesium_sim.sh --lat 1.3521 --lon 103.8198 --heading 90
#   ./run_cesium_sim.sh --qgc                             # fly from QGroundControl
#
# GimbalCam video: rtsp://127.0.0.1:8554/gimbal
#
# Environment (defaults in brackets):
#   UE_ROOT           Unreal Engine 5.7 [~/UnrealEngine-5.7]
#   PX4_DIR           PX4-Autopilot checkout [~/Documents/PX4-Autopilot]
#   PYTHON            Python with the projectairsim client [conda env projectairsim]
#   MEDIAMTX          MediaMTX RTSP server binary [~/opt/mediamtx/mediamtx]
#   CESIUM_ION_TOKEN  Cesium ion token [read from ~/.cesium_ion_token]
#   SIM_MAP           Unreal level [/Game/GISMap]; use /Game/BlocksMap with --scene
#   LOG_DIR           where the sim and PX4 logs go [/tmp/projectairsim-run]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

UE_ROOT="${UE_ROOT:-$HOME/UnrealEngine-5.7}"
PX4_DIR="${PX4_DIR:-$HOME/Documents/PX4-Autopilot}"
PYTHON="${PYTHON:-$HOME/miniconda3/envs/projectairsim/bin/python}"
MEDIAMTX="${MEDIAMTX:-$HOME/opt/mediamtx/mediamtx}"
SIM_MAP="${SIM_MAP:-/Game/GISMap}"
LOG_DIR="${LOG_DIR:-/tmp/projectairsim-run}"

if [ -z "${CESIUM_ION_TOKEN:-}" ] && [ -f "$HOME/.cesium_ion_token" ]; then
    # shellcheck disable=SC1091
    source "$HOME/.cesium_ion_token"
fi
if [ -z "${CESIUM_ION_TOKEN:-}" ] && [ "$SIM_MAP" = "/Game/GISMap" ]; then
    echo "CESIUM_ION_TOKEN is not set and ~/.cesium_ion_token does not exist." >&2
    exit 1
fi
export CESIUM_ION_TOKEN

for port in 8989 8990 4560; do
    if ss -ltn | grep -q ":$port "; then
        echo "Port $port is in use; is another simulator or PX4 still running?" >&2
        exit 1
    fi
done

mkdir -p "$LOG_DIR"
SIM_PID=""
PX4_PID=""
RTSP_PID=""

cleanup() {
    trap - EXIT INT TERM
    echo "Stopping PX4 and the simulator..."
    [ -n "$PX4_PID" ] && kill "$PX4_PID" 2>/dev/null || true
    [ -n "$RTSP_PID" ] && kill "$RTSP_PID" 2>/dev/null || true
    [ -n "$SIM_PID" ] && kill "$SIM_PID" 2>/dev/null || true
    sleep 3
    [ -n "$SIM_PID" ] && kill -9 "$SIM_PID" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

if ss -ltn | grep -q ":8554 "; then
    echo "Using the RTSP server already on port 8554; GimbalCam video at rtsp://127.0.0.1:8554/gimbal"
elif [ -x "$MEDIAMTX" ]; then
    echo "Starting the RTSP server; GimbalCam video at rtsp://127.0.0.1:8554/gimbal"
    "$MEDIAMTX" "$SCRIPT_DIR/mediamtx_rtsp.yml" > "$LOG_DIR/mediamtx.log" 2>&1 &
    RTSP_PID=$!
else
    echo "MediaMTX not found at $MEDIAMTX; running without the RTSP stream." >&2
    set -- --no-rtsp "$@"
fi

echo "Starting the simulator ($SIM_MAP); log: $LOG_DIR/sim.log"
"$UE_ROOT/Engine/Binaries/Linux/UnrealEditor" \
    "$REPO_ROOT/unreal/Blocks/Blocks.uproject" "$SIM_MAP" \
    -game -windowed -ResX=1280 -ResY=720 -log > "$LOG_DIR/sim.log" 2>&1 &
SIM_PID=$!

until ss -ltn | grep -q ":8990 "; do
    if ! kill -0 "$SIM_PID" 2>/dev/null; then
        echo "The simulator exited during startup; see $LOG_DIR/sim.log" >&2
        exit 1
    fi
    sleep 1
done

# PX4 SITL can only connect to one simulator session, so start a fresh one each
# run. The gimbal module only starts at boot when MNT_MODE_IN is set, hence the
# PX4_PARAM_* overrides.
echo "Starting PX4 SITL; log: $LOG_DIR/px4.log"
(
    cd "$PX4_DIR/build/px4_sitl_default/rootfs"
    PX4_SIM_MODEL=none_iris PX4_PARAM_MNT_MODE_IN=4 PX4_PARAM_MNT_MODE_OUT=2 \
        exec ../bin/px4 -d ../etc -s etc/init.d-posix/rcS
) > "$LOG_DIR/px4.log" 2>&1 &
PX4_PID=$!

until grep -q "Waiting for simulator" "$LOG_DIR/px4.log"; do
    if ! kill -0 "$PX4_PID" 2>/dev/null; then
        echo "PX4 exited during startup; see $LOG_DIR/px4.log" >&2
        exit 1
    fi
    sleep 1
done

cd "$SCRIPT_DIR"
"$PYTHON" -u px4_gimbal_cesium.py "$@"
