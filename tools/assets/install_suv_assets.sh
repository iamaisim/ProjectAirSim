#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/../.." && pwd)"
manifest="$script_dir/suv-assets.json"

manifest_value() {
    sed -n "s/^[[:space:]]*\"$1\":[[:space:]]*\"\([^\"]*\)\".*/\1/p" "$manifest"
}

archive_path=""
url=""
expected_sha256="$(manifest_value sha256)"
destination_root="$repo_root/unreal/Blocks/Plugins/ProjectAirSim/Content/VehicleAdv"
force=false
ensure=false

usage() {
    cat <<'EOF'
Usage: install_suv_assets.sh [options]

Options:
  --archive PATH       Install from a local ZIP instead of downloading it.
  --url URL            Override the download URL from suv-assets.json.
  --sha256 HASH        Override the expected SHA-256 checksum.
  --destination PATH   Override the VehicleAdv destination directory.
  --force              Replace an existing SUV installation.
  --ensure             Install or update the SUV to the manifest URL.
  -h, --help           Show this help.
EOF
}

while (($#)); do
    case "$1" in
        --archive) archive_path="$2"; shift 2 ;;
        --url) url="$2"; shift 2 ;;
        --sha256) expected_sha256="$2"; shift 2 ;;
        --destination) destination_root="$2"; shift 2 ;;
        --force) force=true; shift ;;
        --ensure) ensure=true; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
done

if [[ -n "$archive_path" && -n "$url" ]]; then
    echo "Specify only one of --archive or --url." >&2
    exit 2
fi

if [[ -z "$archive_path" && -z "$url" ]]; then
    url="$(manifest_value url)"
fi

destination="$destination_root/SUV"
metadata_path="$destination/.projectairsim-suv-assets.json"
if [[ "$ensure" == true && -d "$destination" ]]; then
    installed_url=""
    if [[ -f "$metadata_path" ]]; then
        installed_url="$(sed -n 's/^[[:space:]]*"url":[[:space:]]*"\([^"]*\)".*/\1/p' "$metadata_path")"
    fi
    if [[ -n "$url" && "$installed_url" == "$url" ]]; then
        echo "SUV assets already match manifest URL; skipping download."
        exit 0
    fi
    force=true
fi

staging_dir="$(mktemp -d)"
downloaded_archive=""
cleanup() {
    rm -rf -- "$staging_dir"
    if [[ -n "$downloaded_archive" ]]; then
        rm -f -- "$downloaded_archive"
    fi
}
trap cleanup EXIT

if [[ -z "$archive_path" ]]; then
    downloaded_archive="$(mktemp --suffix=.zip)"
    curl --fail --location --show-error --output "$downloaded_archive" "$url"
    archive_path="$downloaded_archive"
fi

actual_sha256="$(sha256sum "$archive_path" | awk '{print toupper($1)}')"
expected_sha256="$(printf '%s' "$expected_sha256" | tr '[:lower:]' '[:upper:]')"
if [[ "$actual_sha256" != "$expected_sha256" ]]; then
    echo "SUV asset checksum mismatch. Expected $expected_sha256 but got $actual_sha256." >&2
    exit 1
fi

unzip -q "$archive_path" -d "$staging_dir"
staged_suv="$staging_dir/SUV"
if [[ ! -f "$staged_suv/SuvCarPawn.uasset" ]]; then
    echo "Invalid SUV asset pack: SUV/SuvCarPawn.uasset was not found." >&2
    exit 1
fi

if [[ -e "$destination" ]]; then
    if [[ "$force" != true ]]; then
        echo "SUV assets already exist at '$destination'. Use --force to replace them." >&2
        exit 1
    fi
    rm -rf -- "$destination"
fi

mkdir -p -- "$destination_root"
mv -- "$staged_suv" "$destination"
cat > "$metadata_path" <<EOF
{
  "url": "$url"
}
EOF
echo "Installed SUV assets at: $destination"
