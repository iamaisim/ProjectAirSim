#!/bin/bash
# Run the Radar scenario

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

SIMHOST="127.0.0.1"
SIMCONFIG="$REPO_ROOT/client/python/example_user_scripts/sim_config"
SCENE="scene_radar_tower.jsonc"
VEHICLE="RadarTower1"
RADAR_NAME="radar1"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
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
        --radar)
            RADAR_NAME="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "[INFO] Running Radar scenario"
echo "[INFO] Sim host: $SIMHOST"
echo "[INFO] Vehicle: $VEHICLE, Radar: $RADAR_NAME"

exec "$SCRIPT_DIR/run_cpp_user_scenario.sh" \
    --target user_radar_scenario \
    --simhost "$SIMHOST" \
    --simconfig "$SIMCONFIG" \
    --scene "$SCENE" \
    --vehicle "$VEHICLE" \
    --radar "$RADAR_NAME"
