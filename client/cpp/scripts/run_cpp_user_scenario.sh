#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
BUILD_DIR="$ROOT_DIR/client/cpp/build_local"
TARGET="user_scenario_template"
BIN="$BUILD_DIR/$TARGET"

SIMHOST="127.0.0.1"
SIMCONFIG="$ROOT_DIR/client/python/example_user_scripts/sim_config"
SCENE="scene_basic_drone.jsonc"
VEHICLE="Drone1"
EXTRA_ARGS=()

usage() {
  cat <<EOF
Usage: $(basename "$0") [--target TARGET] [--simhost HOST] [--simconfig PATH] [--scene FILE] [--vehicle NAME] [--no-vehicle] [extra app args]

Options:
  --target TARGET      CMake target and binary name (default: user_scenario_template)
  --simhost HOST       Simulation host (default: 127.0.0.1)
  --simconfig PATH     Scene config directory (default: client/python/example_user_scripts/sim_config)
  --scene FILE         Scene file name inside simconfig (default: scene_basic_drone.jsonc)
  --vehicle NAME       Vehicle name in scene (default: Drone1)
  --no-vehicle         Do not pass --vehicle to the target app

Example:
  ./client/cpp/scripts/run_cpp_user_scenario.sh --scene scene_two_drones.jsonc --vehicle Drone1
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --target)
      TARGET="$2"
      BIN="$BUILD_DIR/$TARGET"
      shift 2
      ;;
    --simhost)
      SIMHOST="$2"
      shift 2
      ;;
    --simconfig)
      SIMCONFIG="$2"
      shift 2
      ;;
    --scene)
      SCENE="$2"
      shift 2
      ;;
    --vehicle)
      VEHICLE="$2"
      shift 2
      ;;
    --no-vehicle)
      VEHICLE=""
      shift
      ;;
    --lidar|--radar|--camera|--actor)
      EXTRA_ARGS+=("$1" "$2")
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1"
      usage
      exit 1
      ;;
  esac
done

echo "[INFO] Ensuring $TARGET is built..."
cmake --build "$BUILD_DIR" --target "$TARGET" -j"$(nproc)"

if [[ ! -f "$BIN" ]]; then
  echo "[ERROR] Binary not found: $BIN"
  echo "[INFO] If you passed --target, it must match the executable target name."
  exit 1
fi

echo "[RUN] $TARGET"
CMD=("$BIN" --simhost "$SIMHOST" --simconfig "$SIMCONFIG" --scene "$SCENE")
if [[ -n "$VEHICLE" ]]; then
  CMD+=(--vehicle "$VEHICLE")
fi
CMD+=("${EXTRA_ARGS[@]}")
"${CMD[@]}"
