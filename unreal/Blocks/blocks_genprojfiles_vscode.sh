#!/bin/bash
# Copyright (C) Microsoft Corporation.
# Copyright (C) 2025 IAMAI CONSULTING CORP
# MIT License.

set -euo pipefail

if [ -z "$UE_ROOT" ]
then
  echo
  echo ERROR: UE_ROOT environmant variable is not set. It must be set to the target \
    Unreal engine\'s root folder path, ex. /home/projectairsimuser/UnrealEngine-5.0.3
else
  # Generate VS Code UE project files (overwrites .vscode\settings.json)
  echo "Generating VS Code project files with environment variable UE_ROOT=$UE_ROOT"
  SCRIPTDIR=$(cd "$(dirname "$0")" && pwd)
  cd "$SCRIPTDIR"

  if [ "$(uname -s)" = "Darwin" ]; then
    UE_GENPROJ="$UE_ROOT/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh"
  else
    UE_GENPROJ="$UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh"
  fi

  if [ ! -x "$UE_GENPROJ" ]; then
    echo "ERROR: Unreal project generation script not found: $UE_GENPROJ"
    exit 1
  fi

  if [ "$(uname -s)" = "Darwin" ]; then
    "$UE_GENPROJ" -projectfiles -vscode -project="$SCRIPTDIR/Blocks.uproject" -game
  else
    "$UE_GENPROJ" -projectfiles -vscode -project="$SCRIPTDIR/Blocks.uproject" -game
  fi

  # Insert projectairsim project folder into UE-generated Block.code-workspace
  echo "{" > AirSimBlocks.code-workspace
  echo "	\"folders\": [" >> AirSimBlocks.code-workspace
  echo "		{" >> AirSimBlocks.code-workspace
  echo "			\"name\": \"projectairsim\"," >> AirSimBlocks.code-workspace
  echo "			\"path\": \"../..\"" >> AirSimBlocks.code-workspace
  echo "		}," >> AirSimBlocks.code-workspace
  if [ ! -f Blocks.code-workspace ]; then
    echo "ERROR: Unreal did not generate Blocks.code-workspace"
    exit 1
  fi
  sed '1,2d' Blocks.code-workspace >> AirSimBlocks.code-workspace
  mv AirSimBlocks.code-workspace Blocks.code-workspace

  # Fix UE's generated game target binary names from UnrealGame to Blocks in launch.json
  if [ -f .vscode/launch.json ]; then
    if [ "$(uname -s)" = "Darwin" ]; then
      sed -i '' 's/UnrealGame-/Blocks-/g' .vscode/launch.json
      sed -i '' 's/UnrealGame"/Blocks"/g' .vscode/launch.json
    else
      sed -i 's/UnrealGame-/Blocks-/g' .vscode/launch.json
      sed -i 's/UnrealGame"/Blocks"/g' .vscode/launch.json
    fi
  fi
fi
