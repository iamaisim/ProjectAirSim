#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/../.." && pwd)"
source_parent="$repo_root/unreal/Blocks/Plugins/ProjectAirSim/Content/VehicleAdv"
source_path="$source_parent/SUV"
output_path="${1:-$PWD/ProjectAirSim-SUV-Assets-v1.0.0.zip}"

if [[ ! -f "$source_path/SuvCarPawn.uasset" ]]; then
    echo "The SUV asset is not installed at '$source_path'." >&2
    exit 1
fi
if [[ -e "$output_path" ]]; then
    echo "Output archive already exists: '$output_path'." >&2
    exit 1
fi

output_dir="$(dirname "$output_path")"
mkdir -p -- "$output_dir"
output_path="$(cd "$output_dir" && pwd)/$(basename "$output_path")"

(cd "$source_parent" && zip -q -r "$output_path" SUV)
echo "Created: $output_path"
echo "SHA256: $(sha256sum "$output_path" | awk '{print toupper($1)}')"
