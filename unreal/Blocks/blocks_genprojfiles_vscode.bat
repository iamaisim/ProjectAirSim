REM Copyright (C) Microsoft Corporation. All rights reserved.

@echo off
if "%UE_ROOT%" == "" (
  echo:
  echo:ERROR: UE_ROOT environmant variable is not set. It must be set to the target ^
Unreal engine's root folder path, ex. C:\Program Files\Epic Games\UE_5.0
) else (
  REM Generate VS Code UE project workspace files (overwrites .vscode\settings.json)
  echo:Generating VS Code project files with environment variable UE_ROOT=%UE_ROOT%
  cd %~dp0
  "%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -vscode -project="%~dp0\Blocks.uproject" -game

  REM Insert projectairsim project folder into UE-generated Block.code-workspace
  echo:{> AirSimBlocks.code-workspace
  echo:	"folders": [>> AirSimBlocks.code-workspace
  echo:		{>> AirSimBlocks.code-workspace
  echo:			"name": "projectairsim",>> AirSimBlocks.code-workspace
  echo:			"path": "../..">> AirSimBlocks.code-workspace
  echo:		},>> AirSimBlocks.code-workspace
  for /f "skip=2 delims=*" %%a in (Blocks.code-workspace) do (
    echo:%%a>>AirSimBlocks.code-workspace
  )
  move AirSimBlocks.code-workspace Blocks.code-workspace

  REM Fix UE's generated game target binary names from UnrealGame to Blocks in launch.json
  SETLOCAL ENABLEDELAYEDEXPANSION
  for /f "delims=" %%a in (.vscode\launch.json) do (
    SET s=%%a
    SET s=!s:UnrealGame-=Blocks-!
    SET s=!s:UnrealGame.exe=Blocks.exe!
    SET s=!s:"D:\build\++UE5\Sync"="D:\\build\\++UE5\\Sync"!
    SET s=!s:"externalTerminal"="internalConsole"!
    echo !s! >> .vscode\airsimlaunch.json
  )
  move .vscode\airsimlaunch.json .vscode\launch.json
)
