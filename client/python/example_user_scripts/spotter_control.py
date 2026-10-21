"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Control window for a walking human "spotter" in a running Cesium scene.

Pick a point from config_spotter_points.yml and Spawn, Teleport or Walk to it,
or walk the spotter by hand: click the window, then hold W/S to walk
forward/back and A/D to turn. The spotter follows the ground and plays walk and
idle animations (UE's template Mannequin, installed by install_spotter_assets.sh).

Started by px4_gimbal_cesium.py --spotter, or by hand while a scene is running:
    python spotter_control.py --scene-file sim_config/scene_px4_gimbal_cesium_custom.jsonc
"""

import argparse
import math
import os
import time
import tkinter as tk
from tkinter import ttk

import commentjson
import yaml

from projectairsim import ProjectAirSimClient, World
from projectairsim.types import Pose, Quaternion, Vector3
from projectairsim.utils import geo_to_ned_coordinates, rpy_to_quaternion

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

SPOTTER_NAME = "Spotter"
MESH = "SKM_Manny_Simple"
WALK_ANIM = "MF_Unarmed_Walk_Fwd"
IDLE_ANIM = "MM_Idle"
WALK_ANIM_SPEED = 1.6  # m/s the walk animation is authored for
MESH_YAW_OFFSET_DEG = -90.0  # the Mannequin mesh faces +Y; rotate it to face +X
TICK_MS = 50
TURN_RATE_DPS = 90.0
ARRIVE_M = 0.3
GROUND_PROBE_M = 1.0  # sample the ground this far ahead/behind, clear of the body
EARTH_M_PER_DEG = 111_320.0


def attach_world(client, scene_file):
    """World handle for an already-loaded scene, without reloading it."""
    with open(scene_file) as f:
        scene = commentjson.load(f)
    world = World.__new__(World)
    world.client = client
    world.parent_topic = f"/Sim/{scene['id']}"
    world.home_geo_point = scene["home-geo-point"]
    drone_name = scene["actors"][0]["name"]
    return world, f"/Sim/{scene['id']}/robots/{drone_name}"


class Spotter:
    """A Mannequin on the ground, moved in NED metres around the scene origin."""

    def __init__(self, world):
        self.world = world
        self.home = world.home_geo_point
        self.north = self.east = 0.0
        self.down = 0.0
        self.heading = 0.0  # deg from north
        self.speed = 1.4  # m/s
        self.target = None  # (north, east) while walking to a point
        self.manual = {"forward": 0, "turn": 0}
        self.walking = False
        self.spawned = False

    # Geodesy -----------------------------------------------------------
    def to_ned(self, lat, lon):
        north, east, _ = geo_to_ned_coordinates(
            self.home, [lat, lon, self.home["altitude"]]
        )
        return north, east

    def to_geo(self):
        lat = self.home["latitude"] + self.north / EARTH_M_PER_DEG
        lon = self.home["longitude"] + self.east / (
            EARTH_M_PER_DEG * math.cos(math.radians(self.home["latitude"]))
        )
        return lat, lon

    # Sim ---------------------------------------------------------------
    def ground_down(self, north, east):
        """Ground height (NED down) under a point, probing clear of the body."""
        h = math.radians(self.heading)
        samples = []
        for sign in (1, -1):
            z = self.world.get_surface_elevation_at_point(
                north + sign * GROUND_PROBE_M * math.cos(h),
                east + sign * GROUND_PROBE_M * math.sin(h),
            )
            if z is not None and not math.isnan(z):
                samples.append(z)
        return sum(samples) / len(samples) if samples else self.down

    def _pose(self):
        w, x, y, z = rpy_to_quaternion(
            0.0, 0.0, math.radians(self.heading + MESH_YAW_OFFSET_DEG)
        )
        return Pose(
            {
                "translation": Vector3({"x": self.north, "y": self.east, "z": self.down}),
                "rotation": Quaternion({"w": w, "x": x, "y": y, "z": z}),
                "frame_id": "DEFAULT_ID",
            }
        )

    def spawn(self, north, east):
        if self.spawned:
            self.world.destroy_object(SPOTTER_NAME)
        self.north, self.east = north, east
        self.down = self.ground_down(north, east)
        self.world.spawn_object(SPOTTER_NAME, MESH, self._pose(), [1.0, 1.0, 1.0], False)
        self.spawned = True
        self.walking = None  # force the idle animation to start
        self._set_walking(False)

    def remove(self):
        if self.spawned:
            self.world.destroy_object(SPOTTER_NAME)
        self.spawned = False
        self.target = None

    def teleport(self, north, east):
        self.target = None
        self.north, self.east = north, east
        self.down = self.ground_down(north, east)
        self.world.set_object_pose(SPOTTER_NAME, self._pose(), True)

    def _set_walking(self, walking):
        if walking == self.walking:
            return
        self.walking = walking
        if walking:
            self.world.set_object_animation(
                SPOTTER_NAME, WALK_ANIM, True, self.speed / WALK_ANIM_SPEED
            )
        else:
            self.world.set_object_animation(SPOTTER_NAME, IDLE_ANIM, True, 1.0)

    def step(self, dt):
        """Advance walking (to a target or under manual control) by dt seconds."""
        if not self.spawned:
            return
        move = 0.0
        if self.manual["turn"]:
            self.heading = (self.heading + self.manual["turn"] * TURN_RATE_DPS * dt) % 360
        if self.manual["forward"]:
            self.target = None
            move = self.manual["forward"] * self.speed * dt
        elif self.target:
            dn, de = self.target[0] - self.north, self.target[1] - self.east
            dist = math.hypot(dn, de)
            if dist < ARRIVE_M:
                self.target = None
            else:
                self.heading = math.degrees(math.atan2(de, dn)) % 360
                move = min(self.speed * dt, dist)

        if move:
            h = math.radians(self.heading)
            self.north += move * math.cos(h)
            self.east += move * math.sin(h)
            self.down = self.ground_down(self.north, self.east)
        if move or self.manual["turn"]:
            self.world.set_object_pose(SPOTTER_NAME, self._pose(), True)
        self._set_walking(bool(move))


class SpotterWindow:
    def __init__(self, root, spotter, points_file, drone_topic):
        self.root = root
        self.spotter = spotter
        self.points_file = points_file
        self.drone_topic = drone_topic
        self.points = self._load_points()

        root.title("Spotter")
        frame = ttk.Frame(root, padding=10)
        frame.grid(sticky="nsew")

        ttk.Label(frame, text="Points").grid(row=0, column=0, sticky="w")
        self.listbox = tk.Listbox(frame, height=10, width=34, exportselection=False)
        self.listbox.grid(row=1, column=0, rowspan=7, sticky="nsew")
        for p in self.points:
            self.listbox.insert(tk.END, p["name"])
        if self.points:
            self.listbox.selection_set(0)

        buttons = [
            ("Spawn here", self.on_spawn),
            ("Teleport", self.on_teleport),
            ("Walk to", self.on_walk),
            ("Stop", self.on_stop),
            ("Remove spotter", self.on_remove),
            ("Save position as point", self.on_save_point),
        ]
        for i, (label, cmd) in enumerate(buttons):
            ttk.Button(frame, text=label, command=cmd, width=24).grid(
                row=1 + i, column=1, padx=(10, 0), pady=2, sticky="ew"
            )

        ttk.Label(frame, text="Speed (m/s)").grid(row=8, column=0, sticky="w", pady=(10, 0))
        self.speed = tk.DoubleVar(value=spotter.speed)
        ttk.Scale(frame, from_=0.5, to=4.0, variable=self.speed,
                  command=self.on_speed).grid(row=9, column=0, columnspan=2, sticky="ew")

        self.status = tk.StringVar(value="Pick a point and press Spawn here.")
        ttk.Label(frame, textvariable=self.status, justify="left").grid(
            row=10, column=0, columnspan=2, sticky="w", pady=(10, 0)
        )
        ttk.Label(
            frame,
            text="Click this window, then hold W/S to walk and A/D to turn.",
            foreground="gray",
        ).grid(row=11, column=0, columnspan=2, sticky="w")

        for key, field, value in (
            ("w", "forward", 1), ("s", "forward", -1), ("a", "turn", -1), ("d", "turn", 1),
        ):
            root.bind(f"<KeyPress-{key}>", lambda e, f=field, v=value: self._key(f, v))
            root.bind(f"<KeyRelease-{key}>", lambda e, f=field: self._key(f, 0))

        self.last_tick = time.monotonic()
        self.root.after(TICK_MS, self.tick)

    def _load_points(self):
        if not os.path.exists(self.points_file):
            return []
        with open(self.points_file) as f:
            return (yaml.safe_load(f) or {}).get("points") or []

    def _selected(self):
        sel = self.listbox.curselection()
        if not sel:
            self.status.set("Select a point first.")
            return None
        p = self.points[sel[0]]
        return self.spotter.to_ned(float(p["lat"]), float(p["lon"]))

    def _run(self, action):
        try:
            action()
        except Exception as err:
            self.status.set(f"Error: {err}")

    def _key(self, field, value):
        self.spotter.manual[field] = value

    def on_spawn(self):
        target = self._selected()
        if target:
            self._run(lambda: self.spotter.spawn(*target))

    def on_teleport(self):
        target = self._selected()
        if target and self.spotter.spawned:
            self._run(lambda: self.spotter.teleport(*target))

    def on_walk(self):
        target = self._selected()
        if target and self.spotter.spawned:
            self.spotter.target = target

    def on_stop(self):
        self.spotter.target = None

    def on_remove(self):
        self._run(self.spotter.remove)

    def on_speed(self, _):
        self.spotter.speed = self.speed.get()
        if self.spotter.walking:
            self.spotter.walking = None  # restart the walk at the new play rate

    def on_save_point(self):
        if not self.spotter.spawned:
            return
        lat, lon = self.spotter.to_geo()
        name = f"Point {len(self.points) + 1}"
        self.points.append({"name": name, "lat": round(lat, 7), "lon": round(lon, 7)})
        self.listbox.insert(tk.END, name)
        with open(self.points_file, "w") as f:
            yaml.safe_dump({"points": self.points}, f, sort_keys=False)
        self.status.set(f"Saved {name} to {os.path.basename(self.points_file)}")

    def _drone_distance(self):
        try:
            k = self.spotter.world.client.request(
                {"method": f"{self.drone_topic}/GetGroundTruthKinematics",
                 "params": {}, "version": 1.0}
            )
            p = k["pose"]["position"]
            return math.dist(
                (p["x"], p["y"], p["z"]),
                (self.spotter.north, self.spotter.east, self.spotter.down - 1.7),
            )
        except Exception:
            return None

    def tick(self):
        now = time.monotonic()
        dt, self.last_tick = now - self.last_tick, now
        try:
            self.spotter.step(dt)
            if self.spotter.spawned:
                lat, lon = self.spotter.to_geo()
                state = "walking" if self.spotter.walking else "standing"
                dist = self._drone_distance()
                drone = f"   drone {dist:.0f} m away" if dist is not None else ""
                self.status.set(
                    f"{state} at {lat:.6f}, {lon:.6f}, heading {self.spotter.heading:.0f}{drone}"
                )
        except Exception as err:
            self.status.set(f"Error: {err}")
        self.root.after(TICK_MS, self.tick)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument(
        "--scene-file",
        default=os.path.join(SCRIPT_DIR, "sim_config", "scene_px4_gimbal_cesium_custom.jsonc"),
        help="scene config of the running scene (for its id and home point)",
    )
    parser.add_argument(
        "--points",
        default=os.path.join(SCRIPT_DIR, "config_spotter_points.yml"),
        help="YAML file of named points",
    )
    args = parser.parse_args()

    client = ProjectAirSimClient()
    client.connect()
    world, drone_topic = attach_world(client, args.scene_file)
    spotter = Spotter(world)

    root = tk.Tk()
    SpotterWindow(root, spotter, args.points, drone_topic)
    try:
        root.mainloop()
    finally:
        try:
            spotter.remove()
        except Exception:
            pass
        client.disconnect()


if __name__ == "__main__":
    main()
