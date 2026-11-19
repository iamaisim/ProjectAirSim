#!/usr/bin/env bash
# Copyright (C) Microsoft Corporation.
# Copyright (C) 2025 IAMAI CONSULTING CORP
# MIT License.

set -euo pipefail

usage() {
    cat <<'EOF'
Usage: ./build_cpp_client.sh [debug|release] [--tests]

Build the standalone ProjectAirSim C++ client without building SimLibs.
  debug       Build Debug artifacts (default).
  release     Build Release artifacts.
  --tests     Build and run the mocked unit tests. A simulator is not required.
EOF
}

build_type=Debug
run_tests=false

for arg in "$@"; do
    case "$arg" in
        debug) build_type=Debug ;;
        release) build_type=Release ;;
        --tests|--test) run_tests=true ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown argument: $arg" >&2; usage >&2; exit 2 ;;
    esac
done

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="$root_dir/client/cpp/build_linux/$build_type"

cmake -S "$root_dir/client/cpp" -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE="$build_type" \
    -DBUILD_TESTING="$run_tests"
cmake --build "$build_dir" --parallel

if "$run_tests"; then
    ctest --test-dir "$build_dir" --output-on-failure
fi
