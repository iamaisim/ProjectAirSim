import json
import subprocess
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path


TOOLS_DIR = Path(__file__).resolve().parents[1]


class ReleaseToolsTests(unittest.TestCase):
    def test_prepare_environment_and_create_archive(self) -> None:
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            project = root / "Example"
            plugins = root / "PluginSource"
            project.mkdir()
            (project / "Example.uproject").write_text(
                json.dumps({"FileVersion": 3}), encoding="utf-8"
            )
            for name in ("ProjectAirSim", "Drone", "Rover"):
                plugin = plugins / name
                plugin.mkdir(parents=True)
                (plugin / f"{name}.uplugin").write_text("{}", encoding="utf-8")

            subprocess.run(
                [
                    sys.executable,
                    str(TOOLS_DIR / "prepare_unreal_environment.py"),
                    "--project-dir",
                    str(project),
                    "--plugin-source",
                    str(plugins),
                    "--target-platform",
                    "Linux",
                ],
                check=True,
            )
            uproject = json.loads(
                (project / "Example.uproject").read_text(encoding="utf-8")
            )
            self.assertEqual(uproject["TargetPlatforms"], ["Linux"])
            self.assertEqual(
                {entry["Name"] for entry in uproject["Plugins"]},
                {"ProjectAirSim", "Drone", "Rover"},
            )
            self.assertIn(
                "GlobalDefaultGameMode=/Script/ProjectAirSim.ProjectAirSimGameMode",
                (project / "Config" / "DefaultEngine.ini").read_text(
                    encoding="utf-8"
                ),
            )

            packaged = root / "Packaged"
            packaged.mkdir()
            (packaged / "Example.sh").write_text("#!/bin/sh\n", encoding="utf-8")
            output = root / "ProjectAirSim-v-test-Example-Linux.zip"
            subprocess.run(
                [
                    sys.executable,
                    str(TOOLS_DIR / "create_release_archive.py"),
                    "--source-dir",
                    str(packaged),
                    "--output",
                    str(output),
                    "--source-tag",
                    "v-test",
                    "--source-sha",
                    "a" * 40,
                    "--component",
                    "Example",
                    "--platform",
                    "Linux",
                    "--unreal-version",
                    "5.2",
                    "--required-name",
                    "Example.sh",
                ],
                check=True,
            )
            self.assertTrue(output.with_suffix(".zip.manifest.json").is_file())
            self.assertTrue(output.with_suffix(".zip.sha256").is_file())
            with zipfile.ZipFile(output) as archive:
                self.assertIn("Example.sh", archive.namelist())
                self.assertIn("build-manifest.json", archive.namelist())


if __name__ == "__main__":
    unittest.main()
