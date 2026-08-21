#!/bin/bash
# Copyright (C) IAMAI CONSULTING CORP
#
# MIT License. All rights reserved.

# Clean script for ProjectAirSim Unreal plugin
# Removes only compilation and intermediate files, preserving source code and SimLibs

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== Cleaning ProjectAirSim Plugin ==="
echo "Plugin directory: $SCRIPT_DIR"

# Remove plugin Binaries (compiled .so/.dll files)
if [ -d "$SCRIPT_DIR/Binaries" ]; then
    echo "Removing: $SCRIPT_DIR/Binaries"
    rm -rf "$SCRIPT_DIR/Binaries"
fi

# Remove plugin Intermediate (build intermediates)
if [ -d "$SCRIPT_DIR/Intermediate" ]; then
    echo "Removing: $SCRIPT_DIR/Intermediate"
    rm -rf "$SCRIPT_DIR/Intermediate"
fi

# Also clean the main Blocks project build artifacts (optional, comment out if not needed)
BLOCKS_DIR="$SCRIPT_DIR/../.."

if [ -d "$BLOCKS_DIR/Binaries" ]; then
    echo "Removing: $BLOCKS_DIR/Binaries"
    rm -rf "$BLOCKS_DIR/Binaries"
fi

if [ -d "$BLOCKS_DIR/Intermediate" ]; then
    echo "Removing: $BLOCKS_DIR/Intermediate"
    rm -rf "$BLOCKS_DIR/Intermediate"
fi

if [ -d "$BLOCKS_DIR/DerivedDataCache" ]; then
    echo "Removing: $BLOCKS_DIR/DerivedDataCache"
    rm -rf "$BLOCKS_DIR/DerivedDataCache"
fi

# Remove any .so files that might have been copied to unexpected locations
find "$SCRIPT_DIR" -name "*.so" -path "*/Binaries/*" -delete 2>/dev/null

echo "=== Clean complete ==="
echo ""
echo "Preserved directories:"
echo "  - Source/ (source code)"
echo "  - SimLibs/ (prebuilt libraries)"
echo "  - Content/ (assets)"
echo "  - Resources/ (plugin resources)"
