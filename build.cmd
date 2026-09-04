@echo off
REM Copyright (C) Microsoft Corporation.
REM Copyright (C) 2025 IAMAI CONSULTING CORP
REM MIT License.

setlocal ENABLEDELAYEDEXPANSION

set "ROOT_DIR=%~dp0"

REM =====================================================
REM Check Visual Studio environment
REM =====================================================

if "%VisualStudioVersion%"=="16.0" (
  echo Detected Visual Studio environment: VS 2019
) else if "%VisualStudioVersion%"=="17.0" (
  echo Detected Visual Studio environment: VS 2022
) else (
  echo Visual Studio developer environment not detected. Initializing VS 2022 build tools.
)

REM =====================================================
REM Unreal Engine detection (OPTIONAL)
REM UE_ROOT is optional to allow standalone / Unity builds
REM =====================================================

set "UE_DETECTED=0"
set "UE_MINOR="

if "%UE_ROOT%"=="" (
  echo:
  echo UE_ROOT not set. Building without Unreal Engine integration.
  goto :select_msvc_default
)

set "BUILD_VERSION_FILE=%UE_ROOT%\Engine\Build\Build.version"

if not exist "!BUILD_VERSION_FILE!" (
  echo:
  echo UE_ROOT is set but Build.version not found:
  echo !BUILD_VERSION_FILE!
  echo Falling back to standalone build.
  goto :select_msvc_default
)

for /f "tokens=2 delims=:," %%A in ('findstr /i "MinorVersion" "!BUILD_VERSION_FILE!"') do (
  set "UE_MINOR=%%A"
)

set "UE_MINOR=%UE_MINOR: =%"

echo Detected Unreal Engine version: 5.%UE_MINOR%
set "UE_DETECTED=1"

REM =====================================================
REM Select MSVC version (UE-aware)
REM =====================================================

if "%UE_MINOR%"=="2" (
  set "MSVC_VER=14.37"
) else if "%UE_MINOR%"=="7" (
  set "MSVC_VER=14.44"
) else (
  echo:
  echo Unsupported Unreal Engine version 5.%UE_MINOR%
  echo Falling back to default toolset.
  goto :select_msvc_default
)

goto :msvc_ready

REM =====================================================
REM Default MSVC (standalone / Unity)
REM =====================================================

:select_msvc_default
echo Using default MSVC toolset (standalone / Unity build)
set "MSVC_VER="

REM =====================================================
REM Initialize MSVC environment
REM =====================================================

:msvc_ready

if defined MSVC_VER (
  echo Requested MSVC toolset version: %MSVC_VER%
) else (
  echo Requested MSVC toolset version: latest installed
)

set "VSWHERE=%SystemDrive%\Progra~2\Microsoft Visual Studio\Installer\vswhere.exe"
set "VS_INSTALL_DIR="
set "MSVC_TOOLS_VERSION="

if defined VSINSTALLDIR set "VS_INSTALL_DIR=%VSINSTALLDIR%"

call :check_vs_install "%ProgramFiles%\Microsoft Visual Studio\2022\Community"
call :check_vs_install "%ProgramFiles%\Microsoft Visual Studio\2022\Professional"
call :check_vs_install "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise"
call :check_vs_install "%SystemDrive%\Progra~2\Microsoft Visual Studio\2022\BuildTools"

if defined VS_INSTALL_DIR goto :vs_install_found
if not exist "!VSWHERE!" goto :vs_install_found
for /f "tokens=*" %%I in ('call "!VSWHERE!" -version "[17.0,18.0)" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do set "VS_INSTALL_DIR=%%I"

:vs_install_found

if not defined VS_INSTALL_DIR (
  echo:
  echo [ERROR] Visual Studio 2022 with C++ build tools was not found.
  goto :buildfailed_nomsg
)

if defined MSVC_VER (
  for /d %%I in ("!VS_INSTALL_DIR!\VC\Tools\MSVC\%MSVC_VER%*") do (
    if not defined MSVC_TOOLS_VERSION set "MSVC_TOOLS_VERSION=%%~nxI"
  )
) else (
  for /f "delims=" %%I in ('dir /b /ad /o-n "!VS_INSTALL_DIR!\VC\Tools\MSVC" 2^>NUL') do (
    if not defined MSVC_TOOLS_VERSION set "MSVC_TOOLS_VERSION=%%I"
  )
)

if not defined MSVC_TOOLS_VERSION (
  echo:
  if defined MSVC_VER (
    echo [ERROR] MSVC toolset %MSVC_VER% was not found under:
  ) else (
    echo [ERROR] No MSVC toolset was found under:
  )
  echo !VS_INSTALL_DIR!\VC\Tools\MSVC
  if "%UE_DETECTED%"=="1" (
    echo Install the requested MSVC toolset or update build.cmd for your Unreal Engine version.
  ) else (
    echo Install the Visual Studio C++ build tools.
  )
  goto :buildfailed_nomsg
)

echo Using Visual Studio installation: %VS_INSTALL_DIR%
echo Using MSVC tools version: %MSVC_TOOLS_VERSION%

call "%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvarsall.bat" x64 -vcvars_ver=%MSVC_TOOLS_VERSION%
if errorlevel 1 (
  echo:
  echo [ERROR] Failed to initialize MSVC %MSVC_TOOLS_VERSION%
  goto :buildfailed_nomsg
)

set PATH=%VS_INSTALL_DIR%\MSBuild\Current\Bin\amd64;%VS_INSTALL_DIR%\MSBuild\Current\Bin;%PATH%
set MSBUILD_TOOLSET_ARGS=/p:VCToolsVersion=%MSVC_TOOLS_VERSION%

if "!UE_DETECTED!"=="1" (
  set "PAS_TOOLCHAIN_ID=UE5.!UE_MINOR!-MSVC!MSVC_TOOLS_VERSION!"
) else (
  set "PAS_TOOLCHAIN_ID=system-MSVC!MSVC_TOOLS_VERSION!"
)
echo Using Project AirSim build tree: build\win64\!PAS_TOOLCHAIN_ID!

where /q nmake
if errorlevel 1 (
  echo:
  echo [ERROR] nmake not found after initializing Visual Studio build tools.
  goto :buildfailed_nomsg
)

REM =====================================================
REM Build
REM =====================================================

nmake /f build_windows.mk %*
if errorlevel 1 (
  goto :buildfailed_nomsg
)

exit /b 0

REM =====================================================
REM Error handling
REM =====================================================

:buildfailed_nomsg
  chdir /d "%ROOT_DIR%"
  echo:
  echo Build Failed.
  exit /b 1

:check_vs_install
  if defined VS_INSTALL_DIR exit /b 0
  if exist "%~1\VC\Auxiliary\Build\vcvarsall.bat" set "VS_INSTALL_DIR=%~1"
  exit /b 0
