@echo off
REM Copyright (C) IAMAI CONSULTING CORP
REM
REM MIT License. All rights reserved.

set "SCRIPT_DIR=%~dp0"

echo === Cleaning Blocks Project ===
echo Project directory: %SCRIPT_DIR%

REM Clean ProjectAirSim plugin first
set "PROJECTAIRSIM_CLEAN=%SCRIPT_DIR%Plugins\ProjectAirSim\clean_plugin.bat"
if exist "%PROJECTAIRSIM_CLEAN%" (
    echo.
    echo --- Calling ProjectAirSim plugin clean ---
    call "%PROJECTAIRSIM_CLEAN%"
) else (
    echo.
    echo Warning: ProjectAirSim clean script not found or not a .bat file
)

REM Clean other plugins (Drone, Rover)
for %%D in (Drone Rover) do (
    if exist "%SCRIPT_DIR%Plugins\%%D" (
        echo.
        echo --- Cleaning plugin: %%D ---
        
        if exist "%SCRIPT_DIR%Plugins\%%D\Binaries" (
            echo Removing: "%SCRIPT_DIR%Plugins\%%D\Binaries"
            rd /s /q "%SCRIPT_DIR%Plugins\%%D\Binaries"
        )
        
        if exist "%SCRIPT_DIR%Plugins\%%D\Intermediate" (
            echo Removing: "%SCRIPT_DIR%Plugins\%%D\Intermediate"
            rd /s /q "%SCRIPT_DIR%Plugins\%%D\Intermediate"
        )
    )
)

REM Clean main Blocks project
echo.
echo --- Cleaning Blocks project ---

if exist "%SCRIPT_DIR%Binaries" (
    echo Removing: "%SCRIPT_DIR%Binaries"
    rd /s /q "%SCRIPT_DIR%Binaries"
)

if exist "%SCRIPT_DIR%Intermediate" (
    echo Removing: "%SCRIPT_DIR%Intermediate"
    rd /s /q "%SCRIPT_DIR%Intermediate"
)

if exist "%SCRIPT_DIR%DerivedDataCache" (
    echo Removing: "%SCRIPT_DIR%DerivedDataCache"
    rd /s /q "%SCRIPT_DIR%DerivedDataCache"
)

REM Remove Saved folder (logs, autosaves, etc.) - optional
REM if exist "%SCRIPT_DIR%Saved" (
REM     echo Removing: "%SCRIPT_DIR%Saved"
REM     rd /s /q "%SCRIPT_DIR%Saved"
REM )

REM Remove generated project files (optional)
REM del /F /Q "%SCRIPT_DIR%*.sln" 2>nul
REM rd /s /q "%SCRIPT_DIR%.vs" 2>nul

echo.
echo === Blocks Clean Complete ===
echo.
echo To rebuild, run:
echo   blocks_genprojfiles_vscode.bat  (regenerate project files)
echo   Then build from VS Code or command line
