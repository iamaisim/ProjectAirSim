REM Copyright (C) Microsoft Corporation. All rights reserved.

@echo off
setlocal
set ROOT_DIR=%~dp0

if "%VisualStudioVersion%" == "16.0" goto ver_ok
if "%VisualStudioVersion%" == "17.0" goto ver_ok
echo:
echo:You need to run this command from x64 Native Tools Command Prompt for VS 2019 or VS 2022.
goto :buildfailed_nomsg

:ver_ok
where /q nmake
if errorlevel 1 (
  echo:
  echo:nmake not found.
  goto :buildfailed_nomsg
)

nmake /f build_windows.mk %*
if errorlevel 1 (
  goto :buildfailed_nomsg
)

exit /b 0

:buildfailed_nomsg
  chdir /d %ROOT_DIR%
  echo:
  echo:Build Failed.
  exit /b 1
