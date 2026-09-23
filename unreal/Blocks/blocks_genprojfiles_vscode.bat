REM Copyright (C) Microsoft Corporation. 
REM Copyright (C) 2025 IAMAI CONSULTING CORP

REM MIT License.

@echo off
if "%UE_ROOT%" == "" (
  echo:
  echo:ERROR: UE_ROOT environmant variable is not set. It must be set to the target ^
Unreal engine's root folder path, ex. C:\Program Files\Epic Games\UE_5.0
) else (
  REM Generate VS Code UE project workspace files (overwrites .vscode\settings.json)
  echo:Generating VS Code project files with environment variable UE_ROOT=%UE_ROOT%
  cd %~dp0
  call "%UE_ROOT%\Engine\Build\BatchFiles\GetDotnetPath.bat"
  if errorlevel 1 (
    echo:ERROR: Unable to configure the bundled .NET runtime for %UE_ROOT%.
    exit /b 1
  )
  "%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -vscode -project="%~dp0\Blocks.uproject" -game
  if errorlevel 1 (
    echo:ERROR: UnrealBuildTool failed to generate the VS Code project files.
    exit /b 1
  )

  if not exist "Blocks.code-workspace" (
    echo:ERROR: UnrealBuildTool did not create Blocks.code-workspace.
    exit /b 1
  )

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
  if exist ".vscode\launch.json" (
    if exist ".vscode\airsimlaunch.json" del /q ".vscode\airsimlaunch.json"
    for /f "delims=" %%a in (.vscode\launch.json) do (
      SET s=%%a
      SET s=!s:UnrealGame-=Blocks-!
      SET s=!s:UnrealGame.exe=Blocks.exe!
      SET s=!s:"D:\build\++UE5\Sync"="D:\\build\\++UE5\\Sync"!
      SET s=!s:"externalTerminal"="internalConsole"!
      echo !s! >> .vscode\airsimlaunch.json
    )
    move /y .vscode\airsimlaunch.json .vscode\launch.json
  ) else (
    echo:NOTE: This Unreal version did not generate launch.json; adding Project AirSim entries.
  )

  REM Add Project AirSim Python debugging entries to UE-generated VS Code files.
  where python >nul 2>nul
  if !ERRORLEVEL! == 0 (
    python "%~dp0..\..\tools\update_blocks_vscode.py" --blocks-dir "%~dp0."
  ) else (
    echo:WARNING: python was not found, skipping Project AirSim Python VS Code debug configuration.
  )
)
