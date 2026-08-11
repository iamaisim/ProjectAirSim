#!/usr/bin/env python3
# Copyright (C) 2025 IAMAI CONSULTING CORP
#
# MIT License.

"""Add Project AirSim Python debugging settings to UE-generated Blocks VS Code files."""

import argparse
import json
from pathlib import Path


PYTHON_CONFIGS = [
    {
        "name": "Python: current script (cwd = script folder)",
        "type": "debugpy",
        "request": "launch",
        "program": "${file}",
        "cwd": "${fileDirname}",
        "console": "integratedTerminal",
        "justMyCode": False,
    },
    {
        "name": "Project AirSim: hello_drone.py",
        "type": "debugpy",
        "request": "launch",
        "program": "${workspaceFolder}\\..\\..\\client\\python\\example_user_scripts\\hello_drone.py",
        "cwd": "${workspaceFolder}\\..\\..\\client\\python\\example_user_scripts",
        "console": "integratedTerminal",
        "justMyCode": False,
    },
]

PYTHON_EXTRA_PATH = "../../client/python/projectairsim/src"


def write_json(path: Path, data: dict) -> None:
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def update_launch_json(vscode_dir: Path) -> None:
    launch_path = vscode_dir / "launch.json"
    if launch_path.exists():
        launch = json.loads(launch_path.read_text(encoding="utf-8"))
    else:
        launch = {"version": "0.2.0", "configurations": []}
    configurations = launch.setdefault("configurations", [])

    by_name = {config.get("name"): index for index, config in enumerate(configurations)}
    for config in PYTHON_CONFIGS:
        existing_index = by_name.get(config["name"])
        if existing_index is None:
            configurations.append(config)
        else:
            configurations[existing_index] = config

    write_json(launch_path, launch)


def update_settings_json(vscode_dir: Path) -> None:
    settings_path = vscode_dir / "settings.json"
    if settings_path.exists():
        settings = json.loads(settings_path.read_text(encoding="utf-8"))
    else:
        settings = {}

    extra_paths = settings.setdefault("python.analysis.extraPaths", [])
    if PYTHON_EXTRA_PATH not in extra_paths:
        extra_paths.append(PYTHON_EXTRA_PATH)

    write_json(settings_path, settings)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--blocks-dir",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "unreal" / "Blocks",
        help="Path to the Blocks Unreal project directory.",
    )
    args = parser.parse_args()

    vscode_dir = args.blocks_dir.resolve() / ".vscode"
    update_launch_json(vscode_dir)
    update_settings_json(vscode_dir)


if __name__ == "__main__":
    main()
