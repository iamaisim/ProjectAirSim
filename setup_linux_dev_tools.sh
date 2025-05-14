#!/bin/bash
# Copyright (C) Microsoft Corporation. 
# Copyright (C) IAMAI  Consulting Corporation.  

# MIT License.

set -e

sudo apt-get update
# Install lsb_release to check Ubuntu version
sudo apt-get -y install --no-install-recommends lsb-release

if [[ $(lsb_release -rs) != "22.04" ]]; then
    if [[ $(lsb_release -rs) == "18.04" ]]; then
        # Add Kitware's APT repository to get cmake 3.15 or newer on Ubuntu 18.04 following https://apt.kitware.com/
        sudo apt-get -y install \
            apt-transport-https \
            ca-certificates \
            gnupg \
            software-properties-common \
            wget
        wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
        sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'

        # Add LLVM APT repository to get clang-11/libc++-11 on Ubuntu 18.04
        wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
        sudo apt-add-repository 'deb http://apt.llvm.org/bionic llvm-toolchain-bionic-11 main'
    fi

    if [[ $(lsb_release -rs) == "20.04" ]]; then
        sudo apt-get -y install software-properties-common
        # Add Ubuntu proposed main and universe repositories to get clang-13/libc++-13 on Ubuntu 20.04
        sudo apt-add-repository "deb http://archive.ubuntu.com/ubuntu/ focal-proposed main universe"
    fi

    # Install prerequisites
    sudo apt-get -y install --no-install-recommends \
        build-essential \
        rsync \
        make \
        cmake \
        clang-13 \
        libc++-13-dev \
        libc++abi-13-dev \
        ninja-build \
        libvulkan1 \
        vulkan-tools
else
    if [[ $(lsb_release -rs) == "22.04" ]]; then
        sudo apt-get -y install \
            apt-transport-https \
            ca-certificates \
            gnupg \
            software-properties-common \
            wget
        wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
        sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ jammy main'
     fi

    # Install prerequisites
    sudo apt-get -y install --no-install-recommends \
        build-essential \
        rsync \
        make \
        cmake-data=3.31.6-0kitware1ubuntu22.04.1 \
        cmake=3.31.6-0kitware1ubuntu22.04.1 \
        clang-15 \
        clang++-15 \
        libstdc++-12-dev \
        libc++-15-dev \
        libc++abi-15-dev \
        libstdc++-12-dev \
        ninja-build \
        libvulkan1 \
        vulkan-tools
fi
