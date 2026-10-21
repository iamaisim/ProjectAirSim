"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Places objects (cars, targets, your own glTF/GLB models) at lat/lon positions
on the ground of a Cesium world, from config_spawned_objects.yml or from
commands typed while the sim runs.
"""

import math
import os

import yaml

from projectairsim.types import BoxAlignment, Pose, Quaternion, Vector3
from projectairsim.utils import geo_to_ned_coordinates, projectairsim_log, rpy_to_quaternion

CONSOLE_HELP = """Commands (type while the sim runs):
  list                                  objects spawned by this session
  spawn ASSET LAT LON [HEADING] [LENGTH_M]
                                        spawn an Unreal asset on the ground
  delete NAME | delete all              remove spawned objects
  reload                                delete all, then respawn from the config file
  assets [REGEX]                        list spawnable Unreal assets
  help"""


class ObjectSpawner:
    def __init__(self, world, config_path):
        self.world = world
        self.config_path = config_path
        self.spawned = {}  # name -> description

    # ------------------------------------------------------------------
    # Config file

    def spawn_from_config(self):
        if not self.config_path or not os.path.exists(self.config_path):
            return
        with open(self.config_path) as f:
            config = yaml.safe_load(f) or {}
        base_dir = os.path.dirname(os.path.abspath(self.config_path))
        for i, obj in enumerate(config.get("objects") or []):
            try:
                self.spawn(
                    name=obj.get("name", f"Object{i + 1}"),
                    lat=float(obj["lat"]),
                    lon=float(obj["lon"]),
                    heading=float(obj.get("heading", 0.0)),
                    asset=obj.get("asset"),
                    model_file=(
                        os.path.join(base_dir, os.path.expanduser(obj["file"]))
                        if obj.get("file")
                        else None
                    ),
                    length=obj.get("length"),
                    scale=obj.get("scale"),
                    height=float(obj.get("height", 0.0)),
                    physics=bool(obj.get("physics", False)),
                )
            except Exception as err:
                projectairsim_log().warning(f"Could not spawn config object {i + 1}: {err}")

    # ------------------------------------------------------------------
    # Spawning and deleting

    def spawn(self, name, lat, lon, heading=0.0, asset=None, model_file=None,
              length=None, scale=None, height=0.0, physics=False):
        """Spawn an Unreal asset or a glTF/GLB file standing on the ground.

        `length` scales the object uniformly so its longest horizontal side is
        that many metres; `scale` sets [x, y, z] directly instead.
        """
        if (asset is None) == (model_file is None):
            raise ValueError("give exactly one of 'asset' or 'file'")

        home = self.world.home_geo_point
        north, east, _ = geo_to_ned_coordinates(home, [lat, lon, home["altitude"]])
        ground = self.world.get_surface_elevation_at_point(north, east)
        if ground is None or math.isnan(ground):
            raise RuntimeError(
                f"no ground under {lat:.6f}, {lon:.6f} (tiles not loaded that far "
                "from the drone?)"
            )

        # Spawn unrotated so the world-aligned bounding box measures the
        # object's own length; the heading is applied once it is sized
        pose = Pose(
            {
                "translation": Vector3({"x": north, "y": east, "z": ground - height}),
                "rotation": Quaternion({"w": 1.0, "x": 0.0, "y": 0.0, "z": 0.0}),
                "frame_id": "DEFAULT_ID",
            }
        )
        first_scale = [float(s) for s in scale] if scale else [1.0, 1.0, 1.0]

        if asset is not None:
            name = self.world.spawn_object(name, asset, pose, first_scale, physics)
            source = asset
        else:
            ext = os.path.splitext(model_file)[1].lower()
            if ext not in (".glb", ".gltf"):
                raise ValueError(f"{model_file}: only .glb and .gltf models are supported")
            with open(model_file, "rb") as f:
                data = f.read()
            name = self.world.spawn_object_from_file(
                name, ext[1:], data, ext == ".glb", pose, first_scale, physics
            )
            source = os.path.basename(model_file)

        box = self._bbox(name)
        if not box or max(box["size"].values()) <= 0:
            projectairsim_log().warning(
                f"{name}: the sim reports no bounds, so it can't be resized or set on "
                "the ground; it stays at its asset's own size and origin"
            )
        else:
            if length and not scale:
                self._scale_to_length(name, float(length))
            self._rest_on_ground(name, ground - height)
        self._set_heading(name, heading)

        self.spawned[name] = f"{source} at {lat:.6f}, {lon:.6f}, heading {heading:.0f}"
        projectairsim_log().info(f"Spawned {name}: {self.spawned[name]}")
        return name

    def _bbox(self, name):
        return self.world.get_3d_bounding_box(name, BoxAlignment.WORLD_AXIS)

    def _scale_to_length(self, name, length):
        size = self._bbox(name)["size"]
        current = max(size["x"], size["y"])
        if current <= 0:
            return
        factor = length / current
        old = self.world.get_object_scale(name)
        self.world.set_object_scale(name, [s * factor for s in old])

    def _rest_on_ground(self, name, ground_down):
        """Move the object so the bottom of its bounding box touches the ground."""
        box = self._bbox(name)
        bottom = box["center"]["z"] + box["size"]["z"] / 2.0  # NED: +z is down
        pose = self.world.get_object_pose(name)
        pose["translation"]["z"] += ground_down - bottom
        self.world.set_object_pose(name, pose, True)

    def _set_heading(self, name, heading):
        w, x, y, z = rpy_to_quaternion(0.0, 0.0, math.radians(heading))
        pose = self.world.get_object_pose(name)
        pose["rotation"] = Quaternion({"w": w, "x": x, "y": y, "z": z})
        self.world.set_object_pose(name, pose, True)

    def delete(self, name):
        if name == "all":
            for spawned_name in list(self.spawned):
                self.delete(spawned_name)
            return
        self.world.destroy_object(name)
        self.spawned.pop(name, None)
        projectairsim_log().info(f"Deleted {name}")

    # ------------------------------------------------------------------
    # Console

    def run_command(self, line):
        parts = line.split()
        if not parts:
            return
        cmd, args = parts[0].lower(), parts[1:]
        try:
            if cmd == "list":
                if not self.spawned:
                    print("No spawned objects.")
                for name, desc in self.spawned.items():
                    print(f"  {name}: {desc}")
            elif cmd == "spawn" and len(args) >= 3:
                self.spawn(
                    name=f"{args[0]}_{len(self.spawned) + 1}",
                    asset=args[0],
                    lat=float(args[1]),
                    lon=float(args[2]),
                    heading=float(args[3]) if len(args) > 3 else 0.0,
                    length=float(args[4]) if len(args) > 4 else None,
                )
            elif cmd == "delete" and len(args) == 1:
                self.delete(args[0])
            elif cmd == "reload":
                self.delete("all")
                self.spawn_from_config()
            elif cmd == "assets":
                for asset in self.world.list_assets(args[0] if args else ".*"):
                    print(f"  {asset}")
            else:
                print(CONSOLE_HELP)
        except Exception as err:
            print(f"Error: {err}")
