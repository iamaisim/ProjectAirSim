"""Offline regressions for the config validator; no simulator required."""

import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import jsonschema

import validate_configs as validator


class ConfigValidationTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name).resolve()

    def write(self, name, value):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value), encoding="utf-8")
        return path

    def test_real_schemas_are_not_robot_or_scene_instances(self):
        for name in ("robot", "scene"):
            path = validator.REPO_ROOT / (
                f"client/python/projectairsim/src/projectairsim/schema/{name}_config_schema.jsonc"
            )
            with self.subTest(name=name):
                self.assertEqual(validator.validate_config(path), "schema")

    def test_invalid_schema_is_rejected(self):
        path = self.write("robot_config_schema.jsonc", {
            "$schema": "http://json-schema.org/draft-04/schema#",
            "type": "not-a-json-type",
        })
        with self.assertRaises(jsonschema.SchemaError):
            validator.validate_config(path)

    def test_utf8_bom_is_accepted(self):
        path = self.root / "settings.json"
        path.write_text('{"configurations": []}', encoding="utf-8-sig")
        self.assertEqual(validator.validate_config(path), "jsonc-parse")

    def test_real_negative_fixtures_still_raise_expected_errors(self):
        for name in validator.NEGATIVE_FIXTURES:
            with self.subTest(name=name):
                self.assertEqual(
                    validator.validate_config(validator.REPO_ROOT / name),
                    "expected-invalid",
                )

    def test_negative_fixture_name_elsewhere_does_not_bypass_validation(self):
        path = self.write("robot_test_required_schema.jsonc", {})
        with self.assertRaises(jsonschema.ValidationError):
            validator.validate_config(path)

    def test_negative_fixture_that_becomes_valid_is_rejected(self):
        name = "client/python/projectairsim/tests/sim_config/robot_test_required_schema.jsonc"
        path = self.write(name, {"physics-type": "fast-physics"})
        with patch.object(validator, "REPO_ROOT", self.root):
            with self.assertRaisesRegex(ValueError, "unexpectedly passed"):
                validator.validate_config(path)

    def test_wrong_negative_fixture_error_is_rejected(self):
        name = "client/python/projectairsim/tests/sim_config/robot_test_required_schema.jsonc"
        path = self.write(name, {"physics-type": 123})
        with patch.object(validator, "REPO_ROOT", self.root):
            with self.assertRaisesRegex(ValueError, "expected 'required'"):
                validator.validate_config(path)

    def test_local_schema_reference_checks_instances(self):
        self.write("schema.json", {"type": "object", "required": ["value"]})
        valid = self.write("config.json", {"$schema": "schema.json", "value": 1})
        invalid = self.write("invalid.json", {"$schema": "schema.json"})
        self.assertEqual(validator.validate_config(valid), "schema-ref")
        with self.assertRaises(jsonschema.ValidationError):
            validator.validate_config(invalid)

    def test_missing_schema_reference_is_rejected(self):
        path = self.write("config.json", {"$schema": "missing.json"})
        with self.assertRaises(FileNotFoundError):
            validator.validate_config(path)


if __name__ == "__main__":
    unittest.main()
