#!/usr/bin/env bash
set -euo pipefail

repo_root=''
output_dir=''
ue_root="${UE_ROOT:-}"
expected_ue_version=''
plugins=(ProjectAirSim Drone Rover)

usage() {
    cat <<'EOF'
Usage: package_projectairsim_plugins.sh --repo-root PATH --output-dir PATH
       [--ue-root PATH] [--expected-ue-version MAJOR.MINOR]
EOF
}

die() {
    printf 'Error: %s\n' "$*" >&2
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --repo-root) repo_root="${2:?Missing value for --repo-root}"; shift 2 ;;
        --output-dir) output_dir="${2:?Missing value for --output-dir}"; shift 2 ;;
        --ue-root) ue_root="${2:?Missing value for --ue-root}"; shift 2 ;;
        --expected-ue-version) expected_ue_version="${2:?Missing value for --expected-ue-version}"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) die "Unknown argument: $1" ;;
    esac
done

[[ -n "$repo_root" ]] || die '--repo-root is required.'
[[ -n "$output_dir" ]] || die '--output-dir is required.'
[[ -n "$ue_root" ]] || die 'UE_ROOT or --ue-root is required.'
[[ -x "$repo_root/build.sh" ]] || die "build.sh is missing: $repo_root/build.sh"

build_version="$ue_root/Engine/Build/Build.version"
build_script="$ue_root/Engine/Build/BatchFiles/Linux/Build.sh"
uproject="$repo_root/unreal/Blocks/Blocks.uproject"
[[ -f "$build_version" ]] || die "Unreal Build.version is missing: $build_version"
[[ -x "$build_script" ]] || die "Unreal Build.sh is missing: $build_script"
[[ -f "$uproject" ]] || die "Blocks.uproject is missing: $uproject"

actual_ue_version="$(
    python3 - "$build_version" <<'PY'
import json
import sys
data = json.load(open(sys.argv[1], encoding="utf-8-sig"))
print(f"{data['MajorVersion']}.{data['MinorVersion']}")
PY
)"
if [[ -n "$expected_ue_version" && "$actual_ue_version" != "$expected_ue_version" ]]; then
    die "Expected Unreal $expected_ue_version but found $actual_ue_version at $ue_root."
fi

if [[ -e "$output_dir" && -n "$(find "$output_dir" -mindepth 1 -print -quit)" ]]; then
    die "Output directory is not empty: $output_dir"
fi

(cd "$repo_root" && ./build.sh simlibs_release)
"$build_script" Blocks Linux Shipping "$uproject" -waitmutex

mkdir -p "$output_dir/Plugins"
for plugin in "${plugins[@]}"; do
    source_dir="$repo_root/unreal/Blocks/Plugins/$plugin"
    [[ -d "$source_dir" ]] || die "Approved plugin is missing: $source_dir"
    rsync -a \
        --exclude Binaries \
        --exclude Intermediate \
        --exclude Saved \
        "$source_dir/" "$output_dir/Plugins/$plugin/"
done

printf 'Plugin package created: %s\n' "$output_dir"
