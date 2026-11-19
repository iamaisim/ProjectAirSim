#!/usr/bin/env bash
# Package an Unreal project with the Project AirSim plugin installed.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
PREPARE_SCRIPT="$PROJECT_ROOT/prepare_projectairsim_env.sh"

PLATFORM="${PLATFORM:-Linux}"
CONFIGURATION="${CONFIGURATION:-Shipping}"
MAX_PARALLEL_ACTIONS="${MAX_PARALLEL_ACTIONS:-4}"
COOK_PROCESS_COUNT="${COOK_PROCESS_COUNT:-1}"
SHADER_UNUSED_PERCENT="${SHADER_UNUSED_PERCENT:-75}"
MIN_FREE_GB="${MIN_FREE_GB:-80}"
PROCESS_NICE="${PROCESS_NICE:-10}"
PROCESS_IONICE_CLASS="${PROCESS_IONICE_CLASS:-2}"
PROCESS_IONICE_PRIORITY="${PROCESS_IONICE_PRIORITY:-7}"

if [[ "$PLATFORM" != "Linux" ]]; then
  echo "ERROR: This packaging script is configured for Linux only. Use PLATFORM=Linux or omit PLATFORM." >&2
  exit 1
fi

if [[ -z "${PROJECTAIRSIM_TARGET_PLATFORM:-}" ]]; then
  export PROJECTAIRSIM_TARGET_PLATFORM=LinuxNoEditor
fi

mapfile -t UPROJECT_FILES < <(find "$PROJECT_ROOT" -maxdepth 1 -type f -name "*.uproject" | sort)
if [[ "${#UPROJECT_FILES[@]}" -eq 0 ]]; then
  echo "ERROR: No .uproject file found in: $PROJECT_ROOT" >&2
  exit 1
fi

if [[ "${#UPROJECT_FILES[@]}" -gt 1 ]]; then
  echo "ERROR: More than one .uproject file found in: $PROJECT_ROOT" >&2
  printf '  %s\n' "${UPROJECT_FILES[@]}" >&2
  echo "Move this script into the target project root or leave only one .uproject there." >&2
  exit 1
fi

UPROJECT="${UPROJECT_FILES[0]}"
PROJECT_NAME="$(basename "$UPROJECT" .uproject)"
ARCHIVE_DIR="${ARCHIVE_DIR:-$PROJECT_ROOT/Packaged/$PROJECT_NAME-$PLATFORM-$CONFIGURATION}"

detect_ue_root() {
  if [[ -n "${UE_ROOT:-}" ]]; then
    printf '%s\n' "$UE_ROOT"
    return
  fi

  if [[ -f "$PROJECT_ROOT/Makefile" ]]; then
    awk -F'= ' '/^UNREALROOTPATH = / { print $2; exit }' "$PROJECT_ROOT/Makefile"
  fi
}

UE_ROOT_DETECTED="$(detect_ue_root)"
if [[ -z "$UE_ROOT_DETECTED" ]]; then
  echo "ERROR: UE_ROOT is not set and Unreal root could not be detected from Makefile." >&2
  exit 1
fi

RUN_UAT="$UE_ROOT_DETECTED/Engine/Build/BatchFiles/RunUAT.sh"
if [[ ! -x "$RUN_UAT" ]]; then
  echo "ERROR: RunUAT.sh not found or not executable: $RUN_UAT" >&2
  echo "Set UE_ROOT to the Unreal Engine root, for example: export UE_ROOT=/path/to/UnrealEngine" >&2
  exit 1
fi

"$PREPARE_SCRIPT"

mkdir -p "$ARCHIVE_DIR"

echo "Packaging $PROJECT_NAME"
echo "UE_ROOT: $UE_ROOT_DETECTED"
echo "Platform: $PLATFORM"
echo "Configuration: $CONFIGURATION"
echo "Archive: $ARCHIVE_DIR"
echo "Resource limits: MaxParallelActions=$MAX_PARALLEL_ACTIONS CookProcessCount=$COOK_PROCESS_COUNT ShaderUnusedPercent=$SHADER_UNUSED_PERCENT"

UAT_CMD=(
  "$RUN_UAT" BuildCookRun
  -project="$UPROJECT"
  -noP4
  -platform="$PLATFORM"
  -clientconfig="$CONFIGURATION"
  -serverconfig="$CONFIGURATION"
  -build
  -cook
  -allmaps
  -stage
  -pak
  -archive
  -archivedirectory="$ARCHIVE_DIR"
  -utf8output
  -ubtargs="-MaxParallelActions=$MAX_PARALLEL_ACTIONS"
  -AdditionalCookerOptions="-CookProcessCount=$COOK_PROCESS_COUNT -ini:Engine:[DevOptions.Shaders]:PercentageUnusedShaderCompilingThreads=$SHADER_UNUSED_PERCENT"
)

if [[ -n "${EXTRA_UAT_ARGS:-}" ]]; then
  # shellcheck disable=SC2206
  EXTRA_UAT_ARGS_ARRAY=($EXTRA_UAT_ARGS)
  UAT_CMD+=("${EXTRA_UAT_ARGS_ARRAY[@]}")
fi

if command -v ionice >/dev/null 2>&1; then
  ionice -c "$PROCESS_IONICE_CLASS" -n "$PROCESS_IONICE_PRIORITY" nice -n "$PROCESS_NICE" "${UAT_CMD[@]}"
else
  nice -n "$PROCESS_NICE" "${UAT_CMD[@]}"
fi

echo "Package created in: $ARCHIVE_DIR"
