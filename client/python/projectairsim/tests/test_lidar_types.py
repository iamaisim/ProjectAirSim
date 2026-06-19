"""
Copyright (C) 2025 IAMAI CONSULTING CORP -- MIT License.
Pytest suite that validates LiDAR object detection for every available
LiDAR type in ProjectAirSim.

Each test case loads a scene, flies Drone1 to the target waypoint, and
asserts that every known object (OrangeBall and Cone_5) was detected at
least once by at least one sensor in that scene.

Run:
    pytest test_lidar_all_types.py -v
    pytest test_lidar_all_types.py -v --plot    # enable 3-D viewer
    pytest test_lidar_all_types.py -v -k gpu    # single variant
"""
from __future__ import annotations
import asyncio, threading
from pathlib import Path
from typing import List, Tuple

import numpy as np
import pytest
from scipy.spatial.transform import Rotation

from projectairsim import Drone, ProjectAirSimClient, World
from projectairsim.utils import projectairsim_log

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

_SIM_CONFIG = str(Path(__file__).resolve().parent / "sim_config")

KNOWN_OBJECTS = {
    "OrangeBall": {"center": [91.15,  32.10, -5.70], "half_extents": [5.0, 5.0, 5.0], "detection_radius": 8.0},
    "Cone_5":     {"center": [91.40, -35.40, -6.00], "half_extents": [5.0, 5.0, 5.0], "detection_radius": 8.0},
}

# NED absolute waypoint: north, east, down, velocity m/s
FLIGHT_WAYPOINT = (45.30, -4.40, -16.40, 5.0)

# ---------------------------------------------------------------------------
# LiDAR scenarios
#   Each entry: (scene_file, [(sensor_id, human_label), ...])
# ---------------------------------------------------------------------------

LIDAR_SCENARIOS = [
    pytest.param(
        "scene_test_lidar_drone_gpu.jsonc",
        [("lidar1", "GPU-Cylindrical")],
        id="gpu",
    ),
    pytest.param(
        "scene_test_lidar_drone.jsonc",
        [("lidar1", "CPU-Cylindrical")],
        id="cpu",
    ),
    pytest.param(
        "scene_test_lidar_depth.jsonc",
        [("lidar1", "Depth")],
        id="depth",
    ),
    pytest.param(
        "scene_test_lidar_avia.jsonc",
        [("lidar1", "Livox-Avia")],
        id="avia",
    ),
    pytest.param(
        "scene_test_lidar_mid70.jsonc",
        [("lidar1", "Livox-Mid70")],
        id="mid70",
    ),
]

# ---------------------------------------------------------------------------
# Geometry helpers
# ---------------------------------------------------------------------------

def _make_bounds() -> list[dict]:
    bounds = []
    for name, obj in KNOWN_OBJECTS.items():
        cx, cy, cz = obj["center"]
        hx, hy, hz = obj["half_extents"]
        bounds.append({
            "name": name,
            "center": list(obj["center"]),
            "detection_radius": obj["detection_radius"],
            "min": [cx - hx, cy - hy, cz - hz],
            "max": [cx + hx, cy + hy, cz + hz],
        })
    return bounds


def _transform(pts: np.ndarray, pose: dict) -> np.ndarray:
    """Transform point cloud from body/sensor frame to world NED."""
    r = pose["rotation"]
    rot = Rotation.from_quat([r["x"], r["y"], r["z"], r["w"]]).as_matrix()
    t = pose["translation"]
    return (rot @ pts.astype(np.float64).T).T + [t["x"], t["y"], t["z"]]


def _check(world_pts: np.ndarray, bounds: list[dict]) -> dict[str, int]:
    """Return {object_name: point_count_within_detection_radius}."""
    return {
        b["name"]: int(np.sum(np.linalg.norm(world_pts - b["center"], axis=1) <= b["detection_radius"]))
        for b in bounds
    }

# ---------------------------------------------------------------------------
# Optional 3-D viewer
# ---------------------------------------------------------------------------

def _setup_viewer(bounds: list[dict], n: int):
    import matplotlib.pyplot as plt
    from mpl_toolkits.mplot3d import Axes3D  # noqa: F401
    plt.ion()
    try:
        fig = plt.figure(figsize=(7 * n, 6))
    except Exception:
        # Tk/display unavailable -- fall back to non-interactive Agg backend
        plt.switch_backend("Agg")
        fig = plt.figure(figsize=(7 * n, 6))
    axes = [fig.add_subplot(1, n, i + 1, projection="3d") for i in range(n)]
    plt.tight_layout()
    return fig, axes


def _update_viewer(fig, axes, bounds, results, labels, drone_pos, best_dets):
    import matplotlib.pyplot as plt
    all_pts = np.array([b["min"] for b in bounds] + [b["max"] for b in bounds])
    lims = [(all_pts[:, i].min() - 20, all_pts[:, i].max() + 20) for i in range(3)]

    for si, (ax, data, title) in enumerate(zip(axes, results, labels)):
        ax.cla()
        ax.set_title(title); ax.set_xlabel("N"); ax.set_ylabel("E"); ax.set_zlabel("D (up=-D)")
        ax.set_xlim(lims[0]); ax.set_ylim(lims[1]); ax.set_zlim(lims[2])
        ax.invert_zaxis()  # NED: negative D = higher altitude -> invert so up looks up

        for b in bounds:
            mn, mx = b["min"], b["max"]
            x, y, z = [mn[0], mx[0]], [mn[1], mx[1]], [mn[2], mx[2]]
            for i in range(2):
                for j in range(2):
                    ax.plot([x[i], x[i]], [y[j], y[j]], z, c="gray", alpha=0.4)
                    ax.plot([x[i], x[i]], y, [z[j], z[j]], c="gray", alpha=0.4)
                    ax.plot(x, [y[i], y[i]], [z[j], z[j]], c="gray", alpha=0.4)

        accum = best_dets[si] if si < len(best_dets) else {}
        cur   = data[1] if data else {}
        for b in bounds:
            cx, cy, cz = b["center"]
            ever_ok = accum.get(b["name"], 0) > 0
            now_ok  = cur.get(b["name"], 0) > 0
            color  = "lime" if now_ok else ("cyan" if ever_ok else "red")
            marker = "o" if ever_ok else "x"
            status = "OK-now" if now_ok else ("OK-prev" if ever_ok else "MISS")
            ax.scatter([cx], [cy], [cz], c=color, s=120, marker=marker,
                       label=f"{b['name']} {status}", zorder=6)

        if drone_pos is not None:
            ax.scatter(*[[v] for v in drone_pos], c="yellow", s=80, marker="^",
                       label="drone", zorder=5)

        if data and data[0] is not None:
            world_pts = data[0]
            near = np.zeros(len(world_pts), dtype=bool)
            for b in bounds:
                near |= np.linalg.norm(world_pts - b["center"], axis=1) <= b["detection_radius"]
            if near.any():
                pts = world_pts[near]
                ax.scatter(pts[:, 0], pts[:, 1], pts[:, 2],
                           c=pts[:, 2], cmap="viridis", s=4, label=f"hit ({near.sum()})")
            detected = [b["name"] for b in bounds if accum.get(b["name"], 0) > 0]
            ax.set_title(f"{title} | det: {detected or 'none'}")

        ax.legend(loc="upper right", fontsize=6, markerscale=4)

    fig.canvas.draw_idle()
    plt.pause(0.01)

# ---------------------------------------------------------------------------
# Flight + collection logic
# ---------------------------------------------------------------------------

async def _run_scenario(
    scene: str,
    sensors: List[Tuple[str, str]],
    plot: bool,
) -> dict[str, dict[str, int]]:
    """
    Fly Drone1 to FLIGHT_WAYPOINT, collect LiDAR point clouds.
    Returns {sensor_id: {object_name: max_pts_detected_in_any_frame}}.
    """
    client = ProjectAirSimClient()
    bounds = _make_bounds()
    lock   = threading.Lock()
    n      = len(sensors)
    state  = {
        "raw":      [None] * n,
        "result":   [None] * n,
        "drone_pos": None,
        "msgs":     [0] * n,
        "best":     [{} for _ in range(n)],
    }
    stop       = asyncio.Event()
    show_fig   = [False]
    log_active = [True]
    fig        = None
    axes       = None
    drone: Drone | None = None

    def _cb(idx: int):
        def callback(_, msg):
            pc = msg.get("point_cloud", []) if isinstance(msg, dict) else []
            if len(pc) >= 3:
                with lock:
                    state["raw"][idx] = msg
                    state["msgs"][idx] += 1
        return callback

    try:
        client.connect()
        world = World(client, scene, delay_after_load_sec=2, sim_config_path=_SIM_CONFIG)
        drone = Drone(client, world, "Drone1")

        for idx, (sensor_id, _) in enumerate(sensors):
            client.subscribe(drone.sensors[sensor_id]["lidar"], _cb(idx))

        # Resolve real object positions from sim
        try:
            real_poses = world.get_object_poses(list(KNOWN_OBJECTS.keys()))
            for bound, pose in zip(bounds, real_poses):
                cx, cy, cz = pose.translation.x, pose.translation.y, pose.translation.z
                hx, hy, hz = KNOWN_OBJECTS[bound["name"]]["half_extents"]
                bound["center"] = [cx, cy, cz]
                bound["min"]    = [cx - hx, cy - hy, cz - hz]
                bound["max"]    = [cx + hx, cy + hy, cz + hz]
                projectairsim_log().info(f"  {bound['name']} real pos: N={cx:.2f} E={cy:.2f} D={cz:.2f}")
        except Exception as exc:
            projectairsim_log().warning(f"Could not resolve real object positions: {exc}")

        if plot:
            fig, axes = _setup_viewer(bounds, n)
            show_fig[0] = True

        labels = [lbl for _, lbl in sensors]
        tick   = [0]

        async def _loop():
            while not stop.is_set():
                tick[0] += 1
                try:
                    pose = drone.get_ground_truth_pose()
                except Exception:
                    await asyncio.sleep(0.05); continue

                t = pose["translation"]
                with lock:
                    state["drone_pos"] = np.array([t["x"], t["y"], t["z"]])
                    raws = list(state["raw"])

                log_now = log_active[0] and tick[0] % 100 == 0
                for idx, (_, label) in enumerate(sensors):
                    raw = raws[idx]
                    if raw is None: continue
                    pc = raw.get("point_cloud", [])
                    if len(pc) < 3: continue
                    pts = np.array(pc, dtype=np.float32).reshape(-1, 3)
                    w   = _transform(pts, pose)
                    dets = _check(w, bounds)
                    if log_now:
                        for b in bounds:
                            cnt = dets[b["name"]]
                            dists = np.linalg.norm(w - b["center"], axis=1)
                            min_d = float(dists.min()) if len(dists) else float("inf")
                            tag = (
                                f"DETECTED ({cnt} pts)"
                                if cnt
                                else f"not detected  min_dist={min_d:.1f}m"
                            )
                            projectairsim_log().info(f"  [{label}] {b['name']}: {tag}")
                    with lock:
                        state["result"][idx] = (w, dets)
                        for name, cnt in dets.items():
                            state["best"][idx][name] = max(state["best"][idx].get(name, 0), cnt)

                if show_fig[0] and fig is not None:
                    with lock:
                        results   = list(state["result"])
                        dp        = state["drone_pos"]
                        best_snap = [dict(b) for b in state["best"]]
                    _update_viewer(fig, axes, bounds, results, labels, dp, best_snap)
                    await asyncio.sleep(0)
                else:
                    await asyncio.sleep(0.05)

        task = asyncio.create_task(_loop())
        drone.enable_api_control()
        drone.arm()

        projectairsim_log().info("-> takeoff")
        await (await drone.takeoff_async(timeout_sec=20))

        n_n, n_e, n_d, vel = FLIGHT_WAYPOINT
        projectairsim_log().info(f"-> move_to_position ({n_n:.1f}, {n_e:.1f}, {n_d:.1f}) vel={vel}")
        await (await drone.move_to_position_async(north=n_n, east=n_e, down=n_d, velocity=vel))

        # Stop viewer and logging before landing
        show_fig[0]   = False
        log_active[0] = False
        if fig is not None:
            import matplotlib.pyplot as _plt
            _plt.close(fig); fig = None

        stop.set(); task.cancel()
        try: await task
        except asyncio.CancelledError: pass

        drone.disarm()
        drone.disable_api_control()

    finally:
        if drone:
            try:
                for sensor_id, _ in sensors:
                    client.unsubscribe(drone.sensors[sensor_id]["lidar"])
                drone.disarm(); drone.disable_api_control()
            except Exception: pass
        client.disconnect()

    with lock:
        result: dict[str, dict[str, int]] = {}
        for idx, (sensor_id, label) in enumerate(sensors):
            best = state["best"][idx]
            detected   = [name for name, cnt in best.items() if cnt > 0]
            missed     = [name for name, cnt in best.items() if cnt == 0]
            projectairsim_log().info(
                f"[{label}] msgs={state['msgs'][idx]} | detected={detected} | missed={missed}"
            )
            result[sensor_id] = best
    return result

# ---------------------------------------------------------------------------
# Pytest
# ---------------------------------------------------------------------------

@pytest.mark.parametrize("scene,sensors", LIDAR_SCENARIOS)
def test_lidar_detects_objects(scene, sensors, request):
    """
    Fly to the waypoint with each LiDAR type and assert that OrangeBall
    and Cone_5 are detected at least once by at least one sensor.
    """
    plot   = request.config.getoption("--plot")
    result = asyncio.run(_run_scenario(scene, sensors, plot=plot))

    # Object is considered detected if ANY sensor in this scene saw it
    undetected = [
        obj for obj in KNOWN_OBJECTS
        if not any(result.get(sid, {}).get(obj, 0) > 0 for sid, _ in sensors)
    ]

    if undetected:
        per_sensor = "; ".join(
            f"{lbl}: missed {[n for n,c in result.get(sid,{}).items() if c==0]}"
            for sid, lbl in sensors
        )
        pytest.fail(
            f"[{scene}] Not detected by any sensor: {undetected}\n  {per_sensor}"
        )
