#!/bin/bash
# Copyright (C) Microsoft Corporation. 
# Copyright (C) 2025 IAMAI CONSULTING CORP
# MIT License.

set -e

# Reset on every invocation, including when reusing a Unity-enabled CMake cache.
unity=OFF
build_args=()
for arg in "$@"; do
    if [ "$arg" = "--unity" ]; then
        unity=ON
    else
        build_args+=("$arg")
    fi
done

# Inform the user that the environment variable UE_ROOT is not set.
if [ -z "$UE_ROOT" ]; then
    echo "Warning: The UE_ROOT environment variable is not set." >&2
fi

if [ "$(uname -s)" = "Darwin" ]; then
    make -f build_macos.mk "${build_args[@]}" PAS_BUILD_UNITY="$unity"
else
    make -f build_linux.mk "${build_args[@]}" PAS_BUILD_UNITY="$unity"
fi
