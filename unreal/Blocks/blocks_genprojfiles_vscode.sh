#!/bin/bash
# Copyright (C) Microsoft Corporation. 
# Copyright (C) IAMAI  Consulting Corporation.  

# MIT License.

if [ -z "$UE_ROOT" ]
then
  echo
  echo ERROR: UE_ROOT environmant variable is not set. It must be set to the target \
    Unreal engine\'s root folder path, ex. /home/projectairsimuser/UnrealEngine-5.0.3
else
  # Generate VS Code UE project files (overwrites .vscode\settings.json)
  echo Generating VS Code project files with environment variable UE_ROOT=$UE_ROOT
  SCRIPTDIR=$(dirname "$(readlink -f "$0")")
  cd $SCRIPTDIR
  $UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh -projectfiles -vscode -project="$SCRIPTDIR/Blocks.uproject" -game

  # Insert projectairsim project folder into UE-generated Block.code-workspace
  echo "{" > AirSimBlocks.code-workspace
  echo "	\"folders\": [" >> AirSimBlocks.code-workspace
  echo "		{" >> AirSimBlocks.code-workspace
  echo "			\"name\": \"projectairsim\"," >> AirSimBlocks.code-workspace
  echo "			\"path\": \"../..\"" >> AirSimBlocks.code-workspace
  echo "		}," >> AirSimBlocks.code-workspace
  sed '1,2d' Blocks.code-workspace >> AirSimBlocks.code-workspace
  mv AirSimBlocks.code-workspace Blocks.code-workspace

  # Fix UE's generated game target binary names from UnrealGame to Blocks in launch.json
  sed -i 's/UnrealGame-/Blocks-/g' .vscode/launch.json
  sed -i 's/UnrealGame"/Blocks"/g' .vscode/launch.json
fi
