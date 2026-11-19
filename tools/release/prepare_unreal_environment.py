#!/usr/bin/env python3
"""Prepare an Unreal project to build with the Project AirSim plugins."""

from __future__ import annotations

import argparse
import json
import shutil
from pathlib import Path


DEFAULT_PLUGINS = ("ProjectAirSim", "Drone", "Rover")
GAME_MAPS_SECTION = "[/Script/EngineSettings.GameMapsSettings]"
PACKAGING_SECTION = "[/Script/UnrealEd.ProjectPackagingSettings]"
GAME_MODE = "GlobalDefaultGameMode=/Script/ProjectAirSim.ProjectAirSimGameMode"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--project-dir", required=True, type=Path)
    parser.add_argument("--plugin-source", required=True, type=Path)
    parser.add_argument("--target-platform", default="Linux")
    parser.add_argument("--replace-existing-plugins", action="store_true")
    parser.add_argument("--plugin", action="append", dest="plugins")
    return parser.parse_args()


def find_uproject(project_dir: Path) -> Path:
    projects = sorted(project_dir.glob("*.uproject"))
    if len(projects) != 1:
        names = ", ".join(path.name for path in projects) or "none"
        raise RuntimeError(
            f"Expected exactly one .uproject in {project_dir}; found: {names}"
        )
    return projects[0]


def copy_plugins(
    source: Path, target: Path, plugins: tuple[str, ...], replace: bool
) -> None:
    target.mkdir(parents=True, exist_ok=True)
    for name in plugins:
        source_dir = source / name
        target_dir = target / name
        if not source_dir.is_dir():
            raise RuntimeError(f"Required plugin directory is missing: {source_dir}")
        if target_dir.exists():
            if not replace:
                raise RuntimeError(
                    f"Plugin already exists: {target_dir}. "
                    "Use --replace-existing-plugins for an ephemeral checkout."
                )
            shutil.rmtree(target_dir)
        shutil.copytree(
            source_dir,
            target_dir,
            ignore=shutil.ignore_patterns("Intermediate", "Saved", ".git"),
        )


def update_uproject(path: Path, plugins: tuple[str, ...], platform: str) -> None:
    data = json.loads(path.read_text(encoding="utf-8-sig"))
    entries = data.setdefault("Plugins", [])
    by_name = {
        entry.get("Name"): entry for entry in entries if isinstance(entry, dict)
    }
    for name in plugins:
        entry = by_name.get(name)
        if entry is None:
            entries.append({"Name": name, "Enabled": True})
        else:
            entry["Enabled"] = True

    platforms = data.setdefault("TargetPlatforms", [])
    if platform and platform not in platforms:
        platforms.append(platform)

    path.write_text(json.dumps(data, indent="\t") + "\n", encoding="utf-8")


def ensure_ini_value(path: Path, section: str, key: str, value: str) -> None:
    lines = path.read_text(encoding="utf-8-sig").splitlines() if path.exists() else []
    assignment = f"{key}={value}"
    try:
        section_index = lines.index(section)
    except ValueError:
        if lines and lines[-1]:
            lines.append("")
        lines.extend((section, assignment))
    else:
        section_end = next(
            (
                index
                for index in range(section_index + 1, len(lines))
                if lines[index].startswith("[") and lines[index].endswith("]")
            ),
            len(lines),
        )
        replacement_index = next(
            (
                index
                for index in range(section_index + 1, section_end)
                if lines[index].startswith(f"{key}=")
            ),
            None,
        )
        if replacement_index is None:
            lines.insert(section_end, assignment)
        else:
            lines[replacement_index] = assignment

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def ensure_ini_entries(path: Path, section: str, entries: tuple[str, ...]) -> None:
    lines = path.read_text(encoding="utf-8-sig").splitlines() if path.exists() else []
    try:
        section_index = lines.index(section)
    except ValueError:
        if lines and lines[-1]:
            lines.append("")
        lines.append(section)
        section_index = len(lines) - 1

    section_end = next(
        (
            index
            for index in range(section_index + 1, len(lines))
            if lines[index].startswith("[") and lines[index].endswith("]")
        ),
        len(lines),
    )
    existing = set(lines[section_index + 1 : section_end])
    for entry in entries:
        if entry not in existing:
            lines.insert(section_end, entry)
            section_end += 1

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    args = parse_args()
    project_dir = args.project_dir.resolve()
    plugin_source = args.plugin_source.resolve()
    plugins = tuple(args.plugins or DEFAULT_PLUGINS)

    if not project_dir.is_dir():
        raise RuntimeError(f"Project directory does not exist: {project_dir}")
    if not plugin_source.is_dir():
        raise RuntimeError(f"Plugin source does not exist: {plugin_source}")

    uproject = find_uproject(project_dir)
    copy_plugins(
        plugin_source,
        project_dir / "Plugins",
        plugins,
        args.replace_existing_plugins,
    )
    update_uproject(uproject, plugins, args.target_platform)
    ensure_ini_value(
        project_dir / "Config" / "DefaultEngine.ini",
        GAME_MAPS_SECTION,
        "GlobalDefaultGameMode",
        "/Script/ProjectAirSim.ProjectAirSimGameMode",
    )
    ensure_ini_entries(
        project_dir / "Config" / "DefaultGame.ini",
        PACKAGING_SECTION,
        tuple(
            f'+DirectoriesToAlwaysCook=(Path="/{plugin}")' for plugin in plugins
        ),
    )

    print(
        json.dumps(
            {
                "project": uproject.name,
                "platform": args.target_platform,
                "plugins": plugins,
            }
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
