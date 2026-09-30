#!/usr/bin/env bash
# Prefer the runner service environment; recover user shell settings if missing.
set -euo pipefail
case "${1:-}" in
  5.2) variable=UE_ROOT_5_2 ;;
  5.7) variable=UE_ROOT_5_7 ;;
  5.8) variable=UE_ROOT_5_8 ;;
  *) echo "Unsupported Unreal version: ${1:-missing}" >&2; exit 1 ;;
esac

selected="${!variable:-}"
if [[ -z "$selected" ]]; then
  # Interactive Bash reads .bashrc, including files with a noninteractive guard.
  # A separate file prevents shell startup output from becoming part of the path.
  capture=$(mktemp)
  trap 'rm -f -- "$capture"' EXIT
  timeout --foreground --kill-after=5s 30s bash -ic 'printf "%s" "${!1:-}" > "$2"' bash "$variable" "$capture" </dev/null >/dev/null
  selected=$(cat "$capture")
fi
if [[ -z "$selected" ]]; then
  echo "$variable is required in the runner environment or runner user's .bashrc." >&2
  exit 1
fi
if [[ "$selected" == *$'\n'* || "$selected" == *$'\r'* ]]; then
  echo "$variable must be a single-line path." >&2
  exit 1
fi
selected=$(realpath -e -- "$selected")
if [[ ! -x "$selected/Engine/Build/BatchFiles/Linux/Build.sh" ]]; then
  echo "$variable must point to an Unreal installation with executable Linux Build.sh." >&2
  exit 1
fi
printf '%s=%s\nUE_ROOT=%s\n' "$variable" "$selected" "$selected" >> "$GITHUB_ENV"
