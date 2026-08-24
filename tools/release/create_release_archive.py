#!/usr/bin/env python3
"""Create a traceable ZIP, build manifest, and SHA-256 sidecar."""

from __future__ import annotations

import argparse
import hashlib
import json
import zipfile
from datetime import datetime, timezone
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-dir", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--source-tag", required=True)
    parser.add_argument("--source-sha", required=True)
    parser.add_argument("--component", required=True)
    parser.add_argument("--platform", required=True)
    parser.add_argument("--unreal-version", required=True)
    parser.add_argument("--component-sha")
    parser.add_argument("--required-name", action="append", default=[])
    return parser.parse_args()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> int:
    args = parse_args()
    source_dir = args.source_dir.resolve()
    output = args.output.resolve()
    if not source_dir.is_dir():
        raise RuntimeError(f"Source directory does not exist: {source_dir}")
    files = sorted(path for path in source_dir.rglob("*") if path.is_file())
    if not files:
        raise RuntimeError(f"Source directory is empty: {source_dir}")

    names = {path.name for path in files}
    missing = sorted(set(args.required_name) - names)
    if missing:
        raise RuntimeError(f"Required packaged files are missing: {', '.join(missing)}")

    file_entries = []
    for path in files:
        relative = path.relative_to(source_dir).as_posix()
        file_entries.append(
            {
                "path": relative,
                "size": path.stat().st_size,
                "sha256": sha256_file(path),
            }
        )

    created_at = datetime.now(timezone.utc).isoformat()
    build_manifest = {
        "schema_version": 1,
        "created_at": created_at,
        "source_tag": args.source_tag,
        "source_sha": args.source_sha,
        "component": args.component,
        "component_sha": args.component_sha or args.source_sha,
        "platform": args.platform,
        "unreal_version": args.unreal_version,
        "files": file_entries,
    }

    output.parent.mkdir(parents=True, exist_ok=True)
    if output.exists():
        raise RuntimeError(f"Refusing to overwrite existing archive: {output}")
    with zipfile.ZipFile(
        output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=6
    ) as archive:
        for path in files:
            archive.write(path, path.relative_to(source_dir).as_posix())
        archive.writestr(
            "build-manifest.json",
            json.dumps(build_manifest, indent=2, sort_keys=True) + "\n",
        )

    archive_sha = sha256_file(output)
    sidecar_manifest = {
        **{key: value for key, value in build_manifest.items() if key != "files"},
        "archive": output.name,
        "archive_size": output.stat().st_size,
        "archive_sha256": archive_sha,
        "file_count": len(files),
    }
    manifest_path = output.with_suffix(output.suffix + ".manifest.json")
    checksum_path = output.with_suffix(output.suffix + ".sha256")
    manifest_path.write_text(
        json.dumps(sidecar_manifest, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    checksum_path.write_text(f"{archive_sha}  {output.name}\n", encoding="utf-8")
    print(json.dumps(sidecar_manifest))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
