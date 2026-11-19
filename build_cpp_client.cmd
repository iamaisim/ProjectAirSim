@echo off
REM Copyright (C) Microsoft Corporation.
REM Copyright (C) 2025 IAMAI CONSULTING CORP
REM MIT License.

setlocal EnableExtensions EnableDelayedExpansion

set "BUILD_TYPE=Debug"
set "RUN_TESTS=OFF"

:parse_args
if "%~1"=="" goto args_done
if /I "%~1"=="debug" (
  set "BUILD_TYPE=Debug"
  shift
  goto parse_args
)
if /I "%~1"=="release" (
  set "BUILD_TYPE=Release"
  shift
  goto parse_args
)
if /I "%~1"=="--tests" (
  set "RUN_TESTS=ON"
  shift
  goto parse_args
)
if /I "%~1"=="--test" (
  set "RUN_TESTS=ON"
  shift
  goto parse_args
)
if /I "%~1"=="--help" goto usage
if /I "%~1"=="-h" goto usage
echo Unknown argument: %~1
goto usage_error

:args_done
set "ROOT_DIR=%~dp0"

REM Reuse an already initialized MSVC environment, such as a Developer
REM Command Prompt or the environment prepared by CI.
where /q cl.exe
if not errorlevel 1 goto compiler_ready

set "VS_INSTALL_DIR="
if defined VSINSTALLDIR set "VS_INSTALL_DIR=%VSINSTALLDIR%"

if not defined VS_INSTALL_DIR call :check_vs_install "%ProgramFiles%\Microsoft Visual Studio\2022\Community"
if not defined VS_INSTALL_DIR call :check_vs_install "%ProgramFiles%\Microsoft Visual Studio\2022\Professional"
if not defined VS_INSTALL_DIR call :check_vs_install "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise"
if not defined VS_INSTALL_DIR call :check_vs_install "%SystemDrive%\Progra~2\Microsoft Visual Studio\2022\BuildTools"

if not defined VS_INSTALL_DIR if exist "%SystemDrive%\Progra~2\Microsoft Visual Studio\Installer\vswhere.exe" (
  for /f "tokens=*" %%I in ('"%SystemDrive%\Progra~2\Microsoft Visual Studio\Installer\vswhere.exe" -version "[17.0,18.0)" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do set "VS_INSTALL_DIR=%%I"
)

if not defined VS_INSTALL_DIR (
  echo [ERROR] MSVC was not found. Install Visual Studio 2022 Build Tools with
  echo         the Desktop development with C++ workload.
  exit /b 1
)

call "%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 exit /b 1

:compiler_ready
where /q ninja
if errorlevel 1 (
  echo [ERROR] Ninja was not found. Install Ninja or add it to PATH.
  exit /b 1
)

set "BUILD_DIR=%ROOT_DIR%client\cpp\build_windows\%BUILD_TYPE%"
cmake -S "%ROOT_DIR%client\cpp" -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DBUILD_TESTING=%RUN_TESTS%
if errorlevel 1 exit /b 1
cmake --build "%BUILD_DIR%" --parallel
if errorlevel 1 exit /b 1

if /I "%RUN_TESTS%"=="ON" (
  ctest --test-dir "%BUILD_DIR%" --output-on-failure
  if errorlevel 1 exit /b 1
)

exit /b 0

:check_vs_install
if exist "%~1\VC\Auxiliary\Build\vcvarsall.bat" set "VS_INSTALL_DIR=%~1"
exit /b 0

:usage
echo Usage: build_cpp_client.cmd [debug^|release] [--tests]
echo.
echo Build the standalone ProjectAirSim C++ client without building SimLibs.
echo   debug       Build Debug artifacts (default).
echo   release     Build Release artifacts.
echo   --tests     Build and run the mocked unit tests. A simulator is not required.
exit /b 0

:usage_error
call :usage
exit /b 2
