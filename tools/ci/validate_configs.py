#!/usr/bin/env python3
"""Validate changed Project AirSim JSON/JSONC config files."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

import commentjson
import jsonschema

from projectairsim.utils import (
    load_scene_config_as_dict,
    load_text_resource,
    validate_json,
)


REPO_ROOT = Path(__file__).resolve().parents[2]

# These fixtures exercise validation failures in test_json_schema.py. Keep the
# allowlist path-specific: another invalid config must still fail CI.
NEGATIVE_FIXTURES = {
    "client/python/projectairsim/tests/sim_config/scene_test_schema.jsonc": "required",
    "client/python/projectairsim/tests/sim_config/robot_test_required_schema.jsonc": "required",
    "client/python/projectairsim/tests/sim_config/robot_test_type_schema.jsonc": "type",
}


def _load_jsonc(path: Path):
    with path.open(encoding="utf-8-sig") as handle:
        return commentjson.load(handle)


def _validate_schema_reference(config_path: Path) -> bool:
    config = _load_jsonc(config_path)
    schema_ref = config.get("$schema") if isinstance(config, dict) else None
    if not schema_ref:
        return False

    schema_path = (config_path.parent / schema_ref).resolve()
    if not schema_path.exists():
        raise FileNotFoundError(f"{config_path}: schema reference not found: {schema_ref}")

    schema = _load_jsonc(schema_path)
    jsonschema.validate(instance=config, schema=schema)
    return True


def _validate_instance(path: Path) -> str:
    name = path.name

    if name.startswith("scene_"):
        load_scene_config_as_dict(name, str(path.parent))
        return "scene"

    if name.startswith("robot_"):
        schema_text = load_text_resource("schema/robot_config_schema.jsonc")
        validate_json(_load_jsonc(path), schema_text)
        return "robot"

    if _validate_schema_reference(path):
        return "schema-ref"

    _load_jsonc(path)
    return "jsonc-parse"


def validate_config(path: Path) -> str:
    path = path.resolve()
    config = _load_jsonc(path)
    # A recognized JSON Schema meta-schema declares a schema document, not a
    # robot/scene instance. Check it before looking at filename prefixes.
    if isinstance(config, dict) and "$schema" in config:
        validator = jsonschema.validators.validator_for(config, default=None)
        if validator is not None:
            validator.check_schema(config)
            return "schema"

    relative = path.relative_to(REPO_ROOT).as_posix() if path.is_relative_to(REPO_ROOT) else None
    expected_error = NEGATIVE_FIXTURES.get(relative)
    if expected_error is not None:
        try:
            _validate_instance(path)
        except jsonschema.ValidationError as exc:
            if exc.validator != expected_error:
                raise ValueError(
                    f"Negative fixture expected {expected_error!r}, got {exc.validator!r}"
                ) from exc
            return "expected-invalid"
        raise ValueError("Negative fixture unexpectedly passed schema validation")

    return _validate_instance(path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("files", nargs="*", help="Config files to validate")
    args = parser.parse_args()

    failures: list[tuple[str, str]] = []
    checked = 0

    for raw_path in args.files:
        path = (REPO_ROOT / raw_path).resolve()
        if not path.exists() or path.suffix.lower() not in {".json", ".jsonc"}:
            continue

        checked += 1
        try:
            kind = validate_config(path)
            print(f"ok: {path.relative_to(REPO_ROOT)} ({kind})")
        except Exception as exc:  # noqa: BLE001 - CI should report all config failures.
            failures.append((raw_path, f"{type(exc).__name__}: {exc}"))

    if failures:
        print("\nConfig validation failures:", file=sys.stderr)
        for file_name, error in failures:
            print(f"- {file_name}: {error}", file=sys.stderr)
        return 1

    print(f"Validated {checked} config file(s).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
