#!/bin/bash
# Copyright (C) IAMAI CONSULTING CORP
#
# MIT License. All rights reserved.

# Clean script for Blocks Unreal project
# Removes compilation and intermediate files for the project and all plugins

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=== Cleaning Blocks Project ==="
echo "Project directory: $SCRIPT_DIR"

# Clean ProjectAirSim plugin first
PROJECTAIRSIM_CLEAN="$SCRIPT_DIR/Plugins/ProjectAirSim/clean_plugin.sh"
if [ -x "$PROJECTAIRSIM_CLEAN" ]; then
    echo ""
    echo "--- Calling ProjectAirSim plugin clean ---"
    "$PROJECTAIRSIM_CLEAN"
else
    echo "Warning: ProjectAirSim clean script not found or not executable"
fi

# Clean other plugins (Drone, Rover)
for plugin_dir in "$SCRIPT_DIR/Plugins/Drone" "$SCRIPT_DIR/Plugins/Rover"; do
    if [ -d "$plugin_dir" ]; then
        echo ""
        echo "--- Cleaning plugin: $(basename $plugin_dir) ---"
        
        if [ -d "$plugin_dir/Binaries" ]; then
            echo "Removing: $plugin_dir/Binaries"
            rm -rf "$plugin_dir/Binaries"
        fi
        
        if [ -d "$plugin_dir/Intermediate" ]; then
            echo "Removing: $plugin_dir/Intermediate"
            rm -rf "$plugin_dir/Intermediate"
        fi
    fi
done

# Clean main Blocks project
echo ""
echo "--- Cleaning Blocks project ---"

if [ -d "$SCRIPT_DIR/Binaries" ]; then
    echo "Removing: $SCRIPT_DIR/Binaries"
    rm -rf "$SCRIPT_DIR/Binaries"
fi

if [ -d "$SCRIPT_DIR/Intermediate" ]; then
    echo "Removing: $SCRIPT_DIR/Intermediate"
    rm -rf "$SCRIPT_DIR/Intermediate"
fi

if [ -d "$SCRIPT_DIR/DerivedDataCache" ]; then
    echo "Removing: $SCRIPT_DIR/DerivedDataCache"
    rm -rf "$SCRIPT_DIR/DerivedDataCache"
fi

# Remove Saved folder (logs, autosaves, etc.) - optional
# Uncomment if you want to clean this too
# if [ -d "$SCRIPT_DIR/Saved" ]; then
#     echo "Removing: $SCRIPT_DIR/Saved"
#     rm -rf "$SCRIPT_DIR/Saved"
# fi

# Remove generated project files (optional)
# Uncomment if you want to regenerate project files
# rm -f "$SCRIPT_DIR"/*.sln 2>/dev/null
# rm -rf "$SCRIPT_DIR"/.vs 2>/dev/null

echo ""
echo "=== Blocks Clean Complete ==="
echo ""
echo "To rebuild, run:"
echo "  ./blocks_genprojfiles_vscode.sh  (regenerate project files)"
echo "  Then build from VS Code or command line"
