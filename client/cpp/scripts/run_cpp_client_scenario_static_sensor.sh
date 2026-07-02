#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
"$SCRIPT_DIR/run_cpp_user_scenario.sh" \
  --target user_static_sensor_scenario \
  --scene scene_computer_vision.jsonc \
  --vehicle CV \
  "$@"
