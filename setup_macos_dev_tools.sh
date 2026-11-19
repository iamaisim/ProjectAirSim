#!/bin/bash
# Copyright (C) Microsoft Corporation.
# Copyright (C) 2025 IAMAI CONSULTING CORP
# MIT License.

set -euo pipefail

if [ "$(uname -s)" != "Darwin" ]; then
    echo "ERROR: setup_macos_dev_tools.sh must be run on macOS." >&2
    exit 1
fi

if ! command -v brew >/dev/null 2>&1; then
    echo "ERROR: Homebrew is required to install the macOS build tools." >&2
    exit 1
fi

brew install cmake ninja openssl@3 zlib

echo "macOS build tools are ready."
echo "OpenSSL root: $(brew --prefix openssl@3)"
echo "zlib root: $(brew --prefix zlib)"
