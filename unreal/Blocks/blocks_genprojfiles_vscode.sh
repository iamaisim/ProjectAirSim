#!/bin/bash
# Copyright (C) Microsoft Corporation.
# Copyright (C) 2025 IAMAI CONSULTING CORP
# MIT License.

if [ -z "$UE_ROOT" ]
then
  echo
  echo ERROR: UE_ROOT environment variable is not set. It must be set to the target \
    Unreal engine\'s root folder path, ex. /home/projectairsimuser/UnrealEngine-5.0.3
else
  # Find the .uproject file in the current directory
  SCRIPTDIR=$(dirname "$(readlink -f "$0")")
  cd $SCRIPTDIR
  UPROJECT_FILE=$(find . -maxdepth 1 -name "*.uproject" | head -n 1)

  if [ -z "$UPROJECT_FILE" ]
  then
    echo "ERROR: No .uproject file found in the current directory."
    exit 1
  fi

  # Extract the project name from the .uproject file
  PROJECT_NAME=$(basename "$UPROJECT_FILE" .uproject)

  # Generate VS Code UE project files (overwrites .vscode/settings.json)
  echo Generating VS Code project files with environment variable UE_ROOT=$UE_ROOT for project $PROJECT_NAME
  $UE_ROOT/Engine/Build/BatchFiles/Linux/Build.sh -projectfiles -vscode -project="$SCRIPTDIR/$UPROJECT_FILE" -game

  # Insert projectairsim project folder into UE-generated .code-workspace file
  echo "{" > AirSim$PROJECT_NAME.code-workspace
  echo "	\"folders\": [" >> AirSim$PROJECT_NAME.code-workspace
  echo "		{" >> AirSim$PROJECT_NAME.code-workspace
  echo "			\"name\": \"projectairsim\"," >> AirSim$PROJECT_NAME.code-workspace
  echo "			\"path\": \"../..\"" >> AirSim$PROJECT_NAME.code-workspace
  echo "		}," >> AirSim$PROJECT_NAME.code-workspace
  sed '1,2d' $PROJECT_NAME.code-workspace >> AirSim$PROJECT_NAME.code-workspace
  mv AirSim$PROJECT_NAME.code-workspace $PROJECT_NAME.code-workspace

  # Fix UE's generated game target binary names from UnrealGame to the project name in launch.json
  sed -i "s/UnrealGame-/$PROJECT_NAME-/g" .vscode/launch.json
  sed -i "s/UnrealGame\"/$PROJECT_NAME\"/g" .vscode/launch.json

  # Add Project AirSim Python debugging entries to UE-generated VS Code files.
  if command -v python3 >/dev/null 2>&1
  then
    python3 "$SCRIPTDIR/../../tools/update_blocks_vscode.py" --blocks-dir "$SCRIPTDIR"
  elif command -v python >/dev/null 2>&1
  then
    python "$SCRIPTDIR/../../tools/update_blocks_vscode.py" --blocks-dir "$SCRIPTDIR"
  else
    echo "WARNING: python was not found, skipping Project AirSim Python VS Code debug configuration."
  fi
fi
