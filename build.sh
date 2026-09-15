#!/bin/bash
# Copyright (C) Microsoft Corporation. 
# Copyright (C) 2025 IAMAI CONSULTING CORP
# MIT License.

set -e

# Inform the user that the environment variable UE_ROOT is not set.
if [ -z "$UE_ROOT" ]; then
    echo "Warning: The UE_ROOT environment variable is not set." >&2
fi

if [ "$(uname -s)" = "Darwin" ]; then
    make -f build_macos.mk "$@"
else
    make -f build_linux.mk "$@"
fi
