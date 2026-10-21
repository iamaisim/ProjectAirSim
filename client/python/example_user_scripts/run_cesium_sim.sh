#!/usr/bin/env bash
# Copyright (C) 2025 IAMAI CONSULTING CORP
# MIT License.
#
# Starts Project AirSim on the Cesium (GISMap) level, a fresh PX4 SITL with the
# MAVLink gimbal enabled, the MediaMTX RTSP server and px4_gimbal_cesium.py.
# Ctrl+C, or closing the simulator window, stops everything.
#
#   ./run_cesium_sim.sh                                   # Seletar runway demo
#   ./run_cesium_sim.sh --qgc --spotter                   # fly from QGC, with a spotter
#   ./run_cesium_sim.sh --px4-params my.params --pi-ip 192.168.199.73 --qgc
#
# Launcher options (everything else goes to px4_gimbal_cesium.py, see --help):
#   --vehicle quad|vtol    airframe (default quad; vtol is not available yet)
#   --px4-params FILE      PX4 parameters applied at boot. Accepts "NAME VALUE"
#                          lines (as in ArduPilot .parm files) or a QGC .params
#                          export. Repeatable; later files win.
#   --pi-ip IP             also send MAVLink to a companion computer at IP, on
#                          UDP ports --pi-ports (default 15540,15541), and
#                          announce the video at rtsp://<this laptop>:8554/gimbal
#   --pi-ports P1[,P2...]  companion-computer MAVLink ports
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

# --- Launcher options --------------------------------------------------------
VEHICLE=quad
PX4_PARAM_FILES=()
PI_IP=""
PI_PORTS="15540,15541"
SCRIPT_ARGS=()
while [ $# -gt 0 ]; do
    case "$1" in
        --vehicle) VEHICLE="$2"; shift 2 ;;
        --px4-params) PX4_PARAM_FILES+=("$(realpath "$2")"); shift 2 ;;
        --pi-ip) PI_IP="$2"; shift 2 ;;
        --pi-ports) PI_PORTS="$2"; shift 2 ;;
        *) SCRIPT_ARGS+=("$1"); shift ;;
    esac
done

case "$VEHICLE" in
    quad) PX4_MODEL=none_iris ;;
    vtol)
        echo "VTOL is not available with PX4 yet (PX4 v1.17 has no sim VTOL airframe)." >&2
        exit 1 ;;
    *) echo "--vehicle must be quad or vtol" >&2; exit 1 ;;
esac

# PX4 reads PX4_PARAM_<NAME>=<value> from the environment at boot, before any
# module starts. The gimbal needs MNT_MODE_* at boot, so they are set first and
# a params file can still override them.
export PX4_PARAM_MNT_MODE_IN=4 PX4_PARAM_MNT_MODE_OUT=2
for params_file in ${PX4_PARAM_FILES[@]+"${PX4_PARAM_FILES[@]}"}; do
    [ -f "$params_file" ] || { echo "No such params file: $params_file" >&2; exit 1; }
    count=0
    while read -r name value; do
        export "PX4_PARAM_$name=$value"
        count=$((count + 1))
    done < <(awk '
        { sub(/#.*/, ""); gsub(/,/, " ") }
        NF >= 5 && $1 ~ /^[0-9]+$/ && $2 ~ /^[0-9]+$/ { print $3, $4; next }  # QGC .params
        NF >= 2 { print $1, $2 }                                                # NAME VALUE
    ' "$params_file" | grep -E '^[A-Z][A-Z0-9_]* [-+0-9.eE]+$')
    echo "PX4 parameters: $count from $params_file"
done

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
RTSP_PID=""
rm -f "$LOG_DIR/px4.pid"

cleanup() {
    trap - EXIT INT TERM
    echo "Stopping PX4 and the simulator..."
    # px4_gimbal_cesium.py starts PX4 (and restarts it for 'goto'); stop the
    # last one in case the script didn't get to
    [ -f "$LOG_DIR/px4.pid" ] && kill "$(cat "$LOG_DIR/px4.pid")" 2>/dev/null || true
    [ -n "$RTSP_PID" ] && kill "$RTSP_PID" 2>/dev/null || true
    [ -n "$SIM_PID" ] && kill "$SIM_PID" 2>/dev/null || true
    sleep 3
    [ -n "$SIM_PID" ] && kill -9 "$SIM_PID" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

# --- RTSP server -------------------------------------------------------------
if ss -ltn | grep -q ":8554 "; then
    echo "Using the RTSP server already on port 8554"
elif [ -x "$MEDIAMTX" ]; then
    echo "Starting the RTSP server; log: $LOG_DIR/mediamtx.log"
    "$MEDIAMTX" "$SCRIPT_DIR/mediamtx_rtsp.yml" > "$LOG_DIR/mediamtx.log" 2>&1 &
    RTSP_PID=$!
else
    echo "MediaMTX not found at $MEDIAMTX; running without the RTSP stream." >&2
    SCRIPT_ARGS=(--no-rtsp ${SCRIPT_ARGS[@]+"${SCRIPT_ARGS[@]}"})
fi

if [ -n "$PI_IP" ]; then
    # Announce the stream on the address the companion computer can reach
    LAN_IP="$(ip route get "$PI_IP" | awk '{for (i = 1; i < NF; i++) if ($i == "src") print $(i + 1)}')"
    SCRIPT_ARGS=(--rtsp-advertise-url "rtsp://$LAN_IP:8554/gimbal" ${SCRIPT_ARGS[@]+"${SCRIPT_ARGS[@]}"})
    echo "Companion computer $PI_IP: MAVLink on UDP $PI_PORTS, video at rtsp://$LAN_IP:8554/gimbal"
fi

# --- Simulator ---------------------------------------------------------------
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

# --- PX4 ---------------------------------------------------------------------
# px4_gimbal_cesium.py starts PX4 SITL itself after loading the scene, and
# restarts it when 'goto' moves to a new place
export PX4_DIR PX4_MODEL PI_IP PI_PORTS LOG_DIR

# --- Mission script ----------------------------------------------------------
cd "$SCRIPT_DIR"
"$PYTHON" -u px4_gimbal_cesium.py ${SCRIPT_ARGS[@]+"${SCRIPT_ARGS[@]}"}
