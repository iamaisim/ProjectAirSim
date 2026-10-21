#!/usr/bin/env bash
# Copyright (C) 2025 IAMAI CONSULTING CORP
# MIT License.
#
# Copies Unreal Engine's template Mannequin (mesh, skeleton, walk and idle
# animations) into the Blocks project so spotter_control.py can spawn a walking
# human. The assets come from your own UE install and are not committed to git.
#
#   ./install_spotter_assets.sh            # UE_ROOT defaults to ~/UnrealEngine-5.7

set -euo pipefail

UE_ROOT="${UE_ROOT:-$HOME/UnrealEngine-5.7}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC="$UE_ROOT/Templates/TemplateResources/High/Characters/Content/Mannequins"
# The template assets reference /Game/Characters/Mannequins/..., so keep that path
DEST="$SCRIPT_DIR/../../../unreal/Blocks/Content/Characters/Mannequins"

if [ ! -d "$SRC" ]; then
    echo "Mannequin template not found at $SRC; set UE_ROOT to your UE 5.7 install." >&2
    exit 1
fi

mkdir -p "$(dirname "$DEST")"
rsync -a --delete "$SRC/" "$DEST/"
echo "Installed the Mannequin into $(cd "$DEST" && pwd)"
echo "Restart the simulator to pick it up."
