#!/bin/bash
# Copyright (C) Microsoft Corporation. All rights reserved.

set -e

# Install UE Vulkan rendering prerequisites
sudo apt-get update
sudo apt-get -y install --no-install-recommends \
    libvulkan1 \
    vulkan-utils

echo "Setup complete."
echo "Note: You can run 'nvidia-smi' and 'vulkaninfo' commands to check if the NVIDIA GPU driver and Vulkan libraries are installed correctly."
