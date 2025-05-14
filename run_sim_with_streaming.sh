#!/bin/bash
# Copyright (C) Microsoft Corporation. 
# Copyright (C) IAMAI  Consulting Corporation.  

# MIT License.

set -e

if [ "$#" -eq 0 ]
then
    echo "Usage: run_sim_with_streaming.sh {sim binary executable}"
    echo "Example: ./run_sim_with_streaming.sh ./Blocks/Development/LinuxNoEditor/Blocks.sh"
    exit 1
fi

SIM_BINARY_EXE=$1
SIM_BINARY_EXE_PATH=$(dirname "$1")

echo "Launching signalling web server (requires sudo elevation for running node.js)..."
sudo echo ""  # run a dummy sudo command to wait to complete the sudo password prompt before launching the server into the background and starting the sim server logging std output
sudo "$SIM_BINARY_EXE_PATH"/Samples/PixelStreaming/WebServers/SignallingWebServer/platform_scripts/bash/Start_SignallingServer.sh --StreamerPort 8888 &

# Wait a bit for signalling server to be ready before launching sim to avoid unnecessary connection error log messages
sleep 3

echo "Launching sim binary with offscreen rendering and PixelStreaming..."
"$SIM_BINARY_EXE" -PixelStreamingURL=ws://127.0.0.1:8888 -RenderOffScreen -Log
