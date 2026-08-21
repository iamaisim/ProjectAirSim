@echo off
REM Copyright (C) IAMAI CONSULTING CORP
REM
REM MIT License. All rights reserved.

set "SCRIPT_DIR=%~dp0"

echo === Cleaning ProjectAirSim Plugin ===
echo Plugin directory: %SCRIPT_DIR%

REM Remove plugin Binaries (compiled .dll files)
if exist "%SCRIPT_DIR%Binaries" (
    echo Removing: "%SCRIPT_DIR%Binaries"
    rd /s /q "%SCRIPT_DIR%Binaries"
)

REM Remove plugin Intermediate (build intermediates)
if exist "%SCRIPT_DIR%Intermediate" (
    echo Removing: "%SCRIPT_DIR%Intermediate"
    rd /s /q "%SCRIPT_DIR%Intermediate"
)

REM Also clean the main Blocks project build artifacts (optional)
set "BLOCKS_DIR=%SCRIPT_DIR%..\..\"

if exist "%BLOCKS_DIR%Binaries" (
    echo Removing: "%BLOCKS_DIR%Binaries"
    rd /s /q "%BLOCKS_DIR%Binaries"
)

if exist "%BLOCKS_DIR%Intermediate" (
    echo Removing: "%BLOCKS_DIR%Intermediate"
    rd /s /q "%BLOCKS_DIR%Intermediate"
)

if exist "%BLOCKS_DIR%DerivedDataCache" (
    echo Removing: "%BLOCKS_DIR%DerivedDataCache"
    rd /s /q "%BLOCKS_DIR%DerivedDataCache"
)

echo === Clean complete ===
echo.
echo Preserved directories:
echo   - Source/ (source code)
echo   - SimLibs/ (prebuilt libraries)
echo   - Content/ (assets)
echo   - Resources/ (plugin resources)
