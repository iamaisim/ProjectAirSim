:: Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
@echo off

:: Simple example launcher script to bring up a sim binary with PixelStreaming
:: enabled, and bring up the SignallingWebServer to host the web streaming view.

if [%1]==[] goto usage

set SIM_BINARY_EXE=%1
set SIM_BINARY_EXE_PATH=%~p1

:: Launch signalling web server
call start PowerShell.exe -command "%SIM_BINARY_EXE_PATH%\Samples\PixelStreaming\WebServers\SignallingWebServer\platform_scripts\cmd\Start_SignallingServer.ps1 --StreamerPort 8888"

:: Launch sim binary with offscreen rendering and PixelStreaming enabled
call start %SIM_BINARY_EXE% -PixelStreamingURL=ws://127.0.0.1:8888 -RenderOffScreen -Log

exit /B 0

:usage
echo Usage: run_sim_with_streaming.cmd {sim binary executable}
echo Example: run_sim_with_streaming.cmd .\Blocks\Development\WindowsNoEditor\Blocks.exe
exit /B 1
