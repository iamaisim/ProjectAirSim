#!/bin/bash
# Copyright (C) Microsoft Corporation. 
# Copyright (C) 2025 IAMAI CONSULTING CORP
# MIT License.

set -e

# Inform the user that the environment variable UE_ROOT is not set.
if [ -z "$UE_ROOT" ]; then
    echo "The UE_ROOT environment variable is not set. Please set it first by executing the command: export UE_ROOT=/path/to/UnrealEngine" >&2
    exit 1
fi

make -f build_linux.mk $1
