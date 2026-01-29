REM Copyright (C) Microsoft Corporation.
REM Copyright (C) 2025 IAMAI CONSULTING CORP
REM MIT License.

@echo off
setlocal ENABLEDELAYEDEXPANSION

set ROOT_DIR=%~dp0

REM =====================================================
REM Check Visual Studio environment
REM =====================================================

if "%VisualStudioVersion%" == "16.0" goto ver_ok
if "%VisualStudioVersion%" == "17.0" goto ver_ok

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
REM Detect Unreal Engine version
REM =====================================================

if "%UE_ROOT%"=="" (
  echo:
  echo [ERROR] UE_ROOT environment variable is not set.
  echo Please set UE_ROOT to your Unreal Engine root directory.
  goto :buildfailed_nomsg
)

set BUILD_VERSION_FILE=%UE_ROOT%\Engine\Build\Build.version

if not exist "%BUILD_VERSION_FILE%" (
  echo:
  echo [ERROR] Build.version not found at:
  echo %BUILD_VERSION_FILE%
  goto :buildfailed_nomsg
)

set UE_MINOR=

for /f "tokens=2 delims=:," %%A in ('findstr /i "MinorVersion" "%BUILD_VERSION_FILE%"') do (
  set UE_MINOR=%%A
)

set UE_MINOR=%UE_MINOR: =%

echo Detected Unreal Engine version: 5.%UE_MINOR%

REM =====================================================
REM Select MSVC version based on UE version
REM =====================================================

if "%UE_MINOR%"=="2" (
  set MSVC_VER=14.37
) else if "%UE_MINOR%"=="7" (
  set MSVC_VER=14.39
) else (
  echo:
  echo [ERROR] Unsupported Unreal Engine version 5.%UE_MINOR%
  echo Only UE 5.2 and UE 5.7 are supported.
  goto :buildfailed_nomsg
)

echo Using MSVC toolset version: %MSVC_VER%

REM =====================================================
REM Re-initialize MSVC environment with correct toolset
REM =====================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 -vcvars_ver=%MSVC_VER%
if errorlevel 1 (
  echo:
  echo [ERROR] Failed to initialize MSVC %MSVC_VER%
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
  chdir /d %ROOT_DIR%
  echo:
  echo Build Failed.
  exit /b 1
