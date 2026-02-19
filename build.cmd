REM Copyright (C) Microsoft Corporation.
REM Copyright (C) 2025 IAMAI CONSULTING CORP
REM MIT License.

@echo off
setlocal ENABLEDELAYEDEXPANSION

set ROOT_DIR=%~dp0

REM =====================================================
REM Check Visual Studio environment
REM =====================================================

if "%VisualStudioVersion%"=="16.0" goto ver_ok
if "%VisualStudioVersion%"=="17.0" goto ver_ok

echo:
echo You need to run this command from x64 Native Tools Command Prompt for VS 2019 or VS 2022.
goto :buildfailed_nomsg

:ver_ok

where /q nmake
if errorlevel 1 (
  echo:
  echo nmake not found.
  goto :buildfailed_nomsg
)

REM =====================================================
REM Unreal Engine detection (OPTIONAL)
REM UE_ROOT is optional to allow standalone / Unity builds
REM =====================================================

set UE_DETECTED=0
set UE_MINOR=

if "%UE_ROOT%"=="" (
  echo:
  echo UE_ROOT not set. Building without Unreal Engine integration.
  goto :select_msvc_default
)

set BUILD_VERSION_FILE=%UE_ROOT%\Engine\Build\Build.version

if not exist "%BUILD_VERSION_FILE%" (
  echo:
  echo UE_ROOT is set but Build.version not found:
  echo %BUILD_VERSION_FILE%
  echo Falling back to standalone build.
  goto :select_msvc_default
)

for /f "tokens=2 delims=:," %%A in ('findstr /i "MinorVersion" "%BUILD_VERSION_FILE%"') do (
  set UE_MINOR=%%A
)

set UE_MINOR=%UE_MINOR: =%

echo Detected Unreal Engine version: 5.%UE_MINOR%
set UE_DETECTED=1

REM =====================================================
REM Select MSVC version (UE-aware)
REM =====================================================

if "%UE_MINOR%"=="2" (
  set MSVC_VER=14.37
) else if "%UE_MINOR%"=="7" (
  set MSVC_VER=14.39
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
set MSVC_VER=14.37

REM =====================================================
REM Initialize MSVC environment
REM =====================================================

:msvc_ready

echo Using MSVC toolset version: %MSVC_VER%

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 -vcvars_ver=%MSVC_VER%
if errorlevel 1 (
  echo:
  echo [WARNING] Failed to initialize MSVC %MSVC_VER%
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
  chdir /d %ROOT_DIR%
  echo:
  echo Build Failed.
  exit /b 1
