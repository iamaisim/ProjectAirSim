#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
BUILD_DIR="$ROOT_DIR/client/cpp/build_local"
BIN="$BUILD_DIR/cpp_client_scenarios"

SIMHOST="127.0.0.1"
SIMCONFIG="$ROOT_DIR/client/python/example_user_scripts/sim_config"
ONLY=""

usage() {
  cat <<EOF
Usage: $(basename "$0") --only LIST [--simhost HOST] [--simconfig PATH]

Options:
  --simhost HOST      Simulation host (default: 127.0.0.1)
  --simconfig PATH    Scene config directory (default: client/python/example_user_scripts/sim_config)
  --only LIST         Comma-separated scenarios to run (required).
                      Allowed: basic,sensors,two_drones,wind,battery,rover,env_actor,static_sensor,lidar,radar

Example:
  ./client/cpp/scripts/run_cpp_client_scenarios.sh --simhost 127.0.0.1 --only sensors,two_drones
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --simhost)
      SIMHOST="$2"
      shift 2
      ;;
    --simconfig)
      SIMCONFIG="$2"
      shift 2
      ;;
    --only)
      ONLY="$2"
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

if [[ -z "$ONLY" ]]; then
  echo "[ERROR] Missing required argument: --only"
  echo "[INFO] This script no longer runs all scenarios by default."
  echo "[INFO] For custom single-scenario apps, use client/cpp/scripts/run_cpp_user_scenario.sh"
  usage
  exit 1
fi

# Rebuild target if needed
echo "[INFO] Ensuring cpp_client_scenarios is built..."
cmake --build "$BUILD_DIR" --target cpp_client_scenarios -j"$(nproc)" 

if [[ ! -f "$BIN" ]]; then
  echo "[ERROR] Binary not found: $BIN"
  exit 1
fi

IFS=',' read -ra SCENARIOS <<< "$ONLY"
for scenario in "${SCENARIOS[@]}"; do
  scenario=$(echo "$scenario" | xargs)  # Trim whitespace
  echo "[RUN] $scenario"

  case "$scenario" in
    basic|sensors|two_drones|wind|battery)
      if "$BIN" --scenario "$scenario" --simhost "$SIMHOST" --simconfig "$SIMCONFIG"; then
        echo "[DONE] $scenario"
      else
        echo "[FAIL] $scenario"
        exit 1
      fi
      ;;
    rover)
      if "$ROOT_DIR/client/cpp/scripts/run_cpp_user_scenario.sh" \
          --target user_rover_scenario \
          --simhost "$SIMHOST" \
          --simconfig "$SIMCONFIG" \
          --scene scene_basic_rover.jsonc \
          --vehicle Rover1; then
        echo "[DONE] $scenario"
      else
        echo "[FAIL] $scenario"
        exit 1
      fi
      ;;
    env_actor)
      if "$ROOT_DIR/client/cpp/scripts/run_cpp_user_scenario.sh" \
          --target user_env_actor_scenario \
          --simhost "$SIMHOST" \
          --simconfig "$SIMCONFIG" \
          --scene scene_env_actor.jsonc \
          --no-vehicle; then
        echo "[DONE] $scenario"
      else
        echo "[FAIL] $scenario"
        exit 1
      fi
      ;;
    static_sensor)
      if "$ROOT_DIR/client/cpp/scripts/run_cpp_user_scenario.sh" \
          --target user_static_sensor_scenario \
          --simhost "$SIMHOST" \
          --simconfig "$SIMCONFIG" \
          --scene scene_computer_vision.jsonc \
          --vehicle CV; then
        echo "[DONE] $scenario"
      else
        echo "[FAIL] $scenario"
        exit 1
      fi
      ;;
    lidar)
      if "$ROOT_DIR/client/cpp/scripts/run_cpp_user_scenario.sh" \
          --target user_lidar_scenario \
          --simhost "$SIMHOST" \
          --simconfig "$SIMCONFIG" \
          --scene scene_lidar_drone.jsonc \
          --vehicle Drone1 \
          --lidar Lidar1; then
        echo "[DONE] $scenario"
      else
        echo "[FAIL] $scenario"
        exit 1
      fi
      ;;
    radar)
      if "$ROOT_DIR/client/cpp/scripts/run_cpp_user_scenario.sh" \
          --target user_radar_scenario \
          --simhost "$SIMHOST" \
          --simconfig "$SIMCONFIG" \
          --scene scene_radar_tower.jsonc \
          --vehicle RadarTower1 \
          --radar radar1; then
        echo "[DONE] $scenario"
      else
        echo "[FAIL] $scenario"
        exit 1
      fi
      ;;
    *)
      echo "[ERROR] Unknown scenario: $scenario"
      usage
      exit 1
      ;;
  esac
done

echo "All selected scenarios completed."
