REM Copyright (C) Microsoft Corporation. 
REM Copyright (C) 2025 IAMAI CONSULTING CORP

REM MIT License.

@echo off
if "%UE_ROOT%" == "" (
  echo:
  echo:ERROR: UE_ROOT environmant variable is not set. It must be set to the target ^
Unreal engine's root folder path, ex. C:\Program Files\Epic Games\UE_5.0
) else (
  REM Generate Visual Studio 2022 UE project Block.sln solution file
  echo:Generating Visual Studio 2022 project files with environment variable UE_ROOT=%UE_ROOT%
  cd %~dp0
  call "%UE_ROOT%\Engine\Build\BatchFiles\GetDotnetPath.bat"
  if errorlevel 1 (
    echo:ERROR: Unable to configure the bundled .NET runtime for %UE_ROOT%.
    exit /b 1
  )
  "%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -2022 -project="%~dp0\Blocks.uproject" -game
  if errorlevel 1 (
    echo:ERROR: UnrealBuildTool failed to generate the Visual Studio project files.
    exit /b 1
  )
)
