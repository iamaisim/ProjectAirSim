#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_dir=''
plugin_source=''
archive_dir=''
ue_root="${UE_ROOT:-}"
configuration='Shipping'
expected_ue_version=''
max_parallel_actions='4'
minimum_free_gb='80'

usage() {
    cat <<'EOF'
Usage: package_unreal_environment.sh --project-dir PATH --plugin-source PATH
       --archive-dir PATH [--ue-root PATH] [--configuration NAME]
       [--expected-ue-version MAJOR.MINOR] [--max-parallel-actions N]
       [--minimum-free-gb N]
EOF
}

die() {
    printf 'Error: %s\n' "$*" >&2
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --project-dir) project_dir="${2:?Missing value for --project-dir}"; shift 2 ;;
        --plugin-source) plugin_source="${2:?Missing value for --plugin-source}"; shift 2 ;;
        --archive-dir) archive_dir="${2:?Missing value for --archive-dir}"; shift 2 ;;
        --ue-root) ue_root="${2:?Missing value for --ue-root}"; shift 2 ;;
        --configuration) configuration="${2:?Missing value for --configuration}"; shift 2 ;;
        --expected-ue-version) expected_ue_version="${2:?Missing value for --expected-ue-version}"; shift 2 ;;
        --max-parallel-actions) max_parallel_actions="${2:?Missing value for --max-parallel-actions}"; shift 2 ;;
        --minimum-free-gb) minimum_free_gb="${2:?Missing value for --minimum-free-gb}"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) die "Unknown argument: $1" ;;
    esac
done

[[ -n "$project_dir" ]] || die '--project-dir is required.'
[[ -n "$plugin_source" ]] || die '--plugin-source is required.'
[[ -n "$archive_dir" ]] || die '--archive-dir is required.'
[[ -n "$ue_root" ]] || die 'UE_ROOT or --ue-root is required.'
[[ -d "$project_dir" ]] || die "Project directory not found: $project_dir"
[[ -d "$plugin_source" ]] || die "Plugin source not found: $plugin_source"

mapfile -t uprojects < <(find "$project_dir" -maxdepth 1 -type f -name '*.uproject' -print | sort)
[[ ${#uprojects[@]} -eq 1 ]] || die "Expected exactly one .uproject in $project_dir."
uproject="${uprojects[0]}"

run_uat="$ue_root/Engine/Build/BatchFiles/RunUAT.sh"
build_version="$ue_root/Engine/Build/Build.version"
[[ -x "$run_uat" ]] || die "RunUAT.sh is missing or not executable: $run_uat"
[[ -f "$build_version" ]] || die "Unreal Build.version is missing: $build_version"

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

if [[ -e "$archive_dir" && -n "$(find "$archive_dir" -mindepth 1 -print -quit)" ]]; then
    die "Archive directory is not empty: $archive_dir"
fi

free_gb="$(df -Pk "$project_dir" | awk 'NR==2 {printf "%d", $4 / 1024 / 1024}')"
if (( free_gb < minimum_free_gb )); then
    die "Only ${free_gb} GiB free; at least ${minimum_free_gb} GiB is required."
fi

python3 "$script_dir/prepare_unreal_environment.py" \
    --project-dir "$project_dir" \
    --plugin-source "$plugin_source" \
    --target-platform Linux \
    --replace-existing-plugins

mkdir -p "$archive_dir"
"$run_uat" BuildCookRun \
    -project="$uproject" \
    -noP4 \
    -platform=Linux \
    -clientconfig="$configuration" \
    -serverconfig="$configuration" \
    -build \
    -cook \
    -allmaps \
    -stage \
    -pak \
    -compressed \
    -archive \
    -archivedirectory="$archive_dir" \
    -prereqs \
    -nodebuginfo \
    -utf8output \
    -ubtargs="-MaxParallelActions=$max_parallel_actions"

printf 'Package created: %s\n' "$archive_dir"
