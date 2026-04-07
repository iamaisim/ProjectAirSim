"""CPU usage profiling suite — baseline before busy-wait fixes.

Reproducible workload: one scene, one drone, fixed dt, no randomization.
Each case runs for a measurement window and reports per-thread CPU.

Cases:
  1. Sim running, no client connected          (manual — observe before connect)
  2. Client connected only (no scene load)
  3. Client connected + scene loaded, no stepping
  4. Step loop only, no image calls             (dt = 3ms)
  5. Step loop + GetImages every step           (dt = 3ms)
  6. Step loop + GetImages every 10 steps       (dt = 3ms)
  7. Step loop only                             (dt = 10ms)
  8. Step loop only                             (dt = 20ms)
  9. Idle after all stepping

Run from: ProjectAirSim repo root
Requires: UE editor with NavGym map in Play mode.

Usage:
    uv run pytest tests/test_cpu_usage.py -v -s
"""

import os
import threading
import time
from pathlib import Path

import pytest

psutil = pytest.importorskip("psutil")

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.types import ImageType

SIM_ADDRESS = "172.23.240.1"
STEP_3MS = 3_000_000
STEP_10MS = 10_000_000
STEP_20MS = 20_000_000

NAV_JAX_SIM_CONFIG = str(
    Path(__file__).resolve().parent.parent.parent
    / "nav-jax"
    / "sims"
    / "airsim"
    / "sim_config"
)
SCENE_CONFIG = "scene_gate_course_rl.jsonc"

MEASURE_SEC = 5.0  # measurement window per case


# ── Helpers ──────────────────────────────────────────────────────────


def get_thread_cpu(proc: psutil.Process) -> list[dict]:
    """Get per-thread CPU usage for a process. Returns list sorted by CPU% desc."""
    try:
        threads = proc.threads()
    except (psutil.NoSuchProcess, psutil.AccessDenied):
        return []
    # threads() gives cumulative user+system time; we sample twice to get delta
    return [
        {"tid": t.id, "user": t.user_time, "system": t.system_time}
        for t in threads
    ]


def measure_thread_cpu(proc: psutil.Process, duration: float) -> list[dict]:
    """Measure per-thread CPU% over a duration. Returns top threads by CPU%."""
    before = {t["tid"]: t for t in get_thread_cpu(proc)}
    time.sleep(duration)
    after = {t["tid"]: t for t in get_thread_cpu(proc)}

    results = []
    for tid, a in after.items():
        b = before.get(tid)
        if b is None:
            continue
        user_delta = a["user"] - b["user"]
        sys_delta = a["system"] - b["system"]
        total_pct = (user_delta + sys_delta) / duration * 100
        results.append({"tid": tid, "cpu_pct": total_pct,
                        "user_pct": user_delta / duration * 100,
                        "sys_pct": sys_delta / duration * 100})
    results.sort(key=lambda x: x["cpu_pct"], reverse=True)
    return results


def measure_case(duration: float) -> dict:
    """Measure process + system + per-thread CPU over a time window."""
    proc = psutil.Process(os.getpid())
    proc.cpu_percent()
    psutil.cpu_percent(percpu=True)

    thread_cpu = measure_thread_cpu(proc, duration)

    return {
        "process_cpu_pct": proc.cpu_percent(),
        "system_cpu_pct": psutil.cpu_percent(),
        "per_core_pct": psutil.cpu_percent(percpu=True),
        "threads": thread_cpu,
        "num_python_threads": threading.active_count(),
    }


def run_step_loop(world, dt_ns, duration_sec, get_images_fn=None,
                  image_every_n=0):
    """Run steps for a fixed duration. Returns step count and elapsed time."""
    proc = psutil.Process(os.getpid())
    proc.cpu_percent()
    psutil.cpu_percent(percpu=True)

    t0 = time.perf_counter()
    n_steps = 0

    # Run for at least duration_sec
    while time.perf_counter() - t0 < duration_sec:
        world.step(dt_ns)
        n_steps += 1
        if get_images_fn and image_every_n > 0 and n_steps % image_every_n == 0:
            get_images_fn()

    elapsed = time.perf_counter() - t0
    thread_cpu = measure_thread_cpu(proc, 0.01)  # snapshot

    return {
        "process_cpu_pct": proc.cpu_percent(),
        "system_cpu_pct": psutil.cpu_percent(),
        "per_core_pct": psutil.cpu_percent(percpu=True),
        "threads": thread_cpu,
        "num_python_threads": threading.active_count(),
        "steps": n_steps,
        "elapsed_sec": elapsed,
        "steps_per_sec": n_steps / elapsed,
        "avg_step_ms": elapsed / n_steps * 1000,
    }


def print_report(label: str, stats: dict):
    """Pretty-print a case report."""
    print(f"\n{'='*70}")
    print(f"  {label}")
    print(f"{'='*70}")
    print(f"  Python process CPU:   {stats['process_cpu_pct']:.1f}%")
    print(f"  System CPU (total):   {stats['system_cpu_pct']:.1f}%")
    print(f"  Python threads:       {stats['num_python_threads']}")

    # Hot cores
    cores = stats["per_core_pct"]
    hot = [(i, p) for i, p in enumerate(cores) if p > 50]
    if hot:
        print(f"  Hot cores (>50%):     {len(hot)}")
        for i, p in hot[:5]:
            print(f"    Core {i:2d}: {p:.0f}%")

    # Top threads by CPU
    threads = stats.get("threads", [])
    top = [t for t in threads if t["cpu_pct"] > 1.0]
    if top:
        print(f"  Top threads by CPU:")
        for t in top[:5]:
            print(f"    TID {t['tid']:>6}: {t['cpu_pct']:5.1f}% "
                  f"(user {t['user_pct']:.1f}%, sys {t['sys_pct']:.1f}%)")

    # Step metrics if present
    if "steps" in stats:
        print(f"  Steps:                {stats['steps']}")
        print(f"  Elapsed:              {stats['elapsed_sec']:.2f}s")
        print(f"  Steps/sec:            {stats['steps_per_sec']:.1f}")
        print(f"  Avg step latency:     {stats['avg_step_ms']:.2f}ms")
    print()


# ── Fixtures ─────────────────────────────────────────────────────────


@pytest.fixture(scope="module")
def client():
    c = ProjectAirSimClient(address=SIM_ADDRESS)
    c.connect()
    yield c
    c.disconnect()


@pytest.fixture(scope="module")
def world(client):
    w = World(
        client,
        SCENE_CONFIG,
        sim_config_path=NAV_JAX_SIM_CONFIG,
        delay_after_load_sec=2,
    )
    return w


@pytest.fixture(scope="module")
def drone(client, world):
    """Create Drone object and arm for step API."""
    d = Drone(client, world, "Drone1")
    topic = f"{world.parent_topic}/robots/Drone1"
    client.request({"method": f"{topic}/EnableApiControl", "params": {}, "version": 1.0})
    client.request({"method": f"{topic}/Arm", "params": {}, "version": 1.0})
    client.request({"method": f"{topic}/SetAcroMode", "params": {"enabled": True}, "version": 1.0})
    return d


# ── Tests ────────────────────────────────────────────────────────────


ALL_RESULTS = {}


class TestCPUProfile:
    """Systematic CPU profiling across cases."""

    def test_case2_connected_only(self, client):
        """Case 2: Client connected, no scene loaded yet."""
        time.sleep(1)
        stats = measure_case(MEASURE_SEC)
        print_report("CASE 2: Client connected only", stats)
        ALL_RESULTS["2_connected"] = stats

    def test_case3_scene_loaded_idle(self, client, world):
        """Case 3: Scene loaded, no stepping."""
        time.sleep(1)
        stats = measure_case(MEASURE_SEC)
        print_report("CASE 3: Scene loaded, idle (no stepping)", stats)
        ALL_RESULTS["3_scene_idle"] = stats

    def test_case4_step_3ms(self, client, world, drone):
        """Case 4: Step loop, dt=3ms, no images."""
        # Warm up
        for _ in range(20):
            world.step(STEP_3MS)
        stats = run_step_loop(world, STEP_3MS, MEASURE_SEC)
        print_report("CASE 4: Step only, dt=3ms", stats)
        ALL_RESULTS["4_step_3ms"] = stats

    def test_case5_step_3ms_images_every(self, client, world, drone):
        """Case 5: Step loop, dt=3ms, GetImages every step."""
        get_img = lambda: drone.get_images("Chase", [ImageType.SCENE])
        stats = run_step_loop(world, STEP_3MS, MEASURE_SEC,
                              get_images_fn=get_img, image_every_n=1)
        print_report("CASE 5: Step + GetImages every step, dt=3ms", stats)
        ALL_RESULTS["5_step_3ms_img1"] = stats

    def test_case6_step_3ms_images_every10(self, client, world, drone):
        """Case 6: Step loop, dt=3ms, GetImages every 10 steps."""
        get_img = lambda: drone.get_images("Chase", [ImageType.SCENE])
        stats = run_step_loop(world, STEP_3MS, MEASURE_SEC,
                              get_images_fn=get_img, image_every_n=10)
        print_report("CASE 6: Step + GetImages every 10 steps, dt=3ms", stats)
        ALL_RESULTS["6_step_3ms_img10"] = stats

    def test_case7_step_10ms(self, client, world, drone):
        """Case 7: Step loop, dt=10ms, no images."""
        stats = run_step_loop(world, STEP_10MS, MEASURE_SEC)
        print_report("CASE 7: Step only, dt=10ms", stats)
        ALL_RESULTS["7_step_10ms"] = stats

    def test_case8_step_20ms(self, client, world, drone):
        """Case 8: Step loop, dt=20ms, no images."""
        stats = run_step_loop(world, STEP_20MS, MEASURE_SEC)
        print_report("CASE 8: Step only, dt=20ms", stats)
        ALL_RESULTS["8_step_20ms"] = stats

    def test_case9_idle_after_stepping(self, client, world, drone):
        """Case 9: Idle after all stepping."""
        time.sleep(1)
        stats = measure_case(MEASURE_SEC)
        print_report("CASE 9: Idle after stepping", stats)
        ALL_RESULTS["9_idle_after"] = stats

    def test_summary(self):
        """Print compact summary table."""
        r = ALL_RESULTS
        print(f"\n{'='*70}")
        print(f"  PROFILING SUMMARY")
        print(f"{'='*70}")

        # Idle/connected cases
        print(f"\n  {'Case':<40} {'Py CPU':>8} {'Sys CPU':>9} {'Threads':>8}")
        print(f"  {'-'*40} {'-'*8} {'-'*9} {'-'*8}")
        for key, label in [
            ("2_connected", "2. Connected only"),
            ("3_scene_idle", "3. Scene loaded, idle"),
            ("9_idle_after", "9. Idle after stepping"),
        ]:
            if key in r:
                s = r[key]
                print(f"  {label:<40} {s['process_cpu_pct']:>6.1f}% "
                      f"{s['system_cpu_pct']:>7.1f}% {s['num_python_threads']:>7}")

        # Stepping cases
        print(f"\n  {'Case':<40} {'Py CPU':>8} {'Steps/s':>9} {'Lat ms':>8}")
        print(f"  {'-'*40} {'-'*8} {'-'*9} {'-'*8}")
        for key, label in [
            ("4_step_3ms", "4. Step 3ms, no images"),
            ("5_step_3ms_img1", "5. Step 3ms + img every step"),
            ("6_step_3ms_img10", "6. Step 3ms + img every 10"),
            ("7_step_10ms", "7. Step 10ms, no images"),
            ("8_step_20ms", "8. Step 20ms, no images"),
        ]:
            if key in r:
                s = r[key]
                print(f"  {label:<40} {s['process_cpu_pct']:>6.1f}% "
                      f"{s['steps_per_sec']:>8.1f} {s['avg_step_ms']:>7.2f}")

        # Top thread from worst idle case
        for key in ["2_connected", "3_scene_idle"]:
            if key in r and r[key]["threads"]:
                top = r[key]["threads"][0]
                print(f"\n  Hottest thread ({key}): "
                      f"TID {top['tid']} at {top['cpu_pct']:.1f}%")
        print()
