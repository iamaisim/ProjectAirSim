"""Run multiple Rascal aircraft, each connected to its own PX4 SITL instance.

Project AirSim must already be running. QGroundControl may be started before or
after this script; PX4 advertises every instance on its normal GCS link.
"""

from __future__ import annotations

import argparse
import copy
import json
import queue
import shlex
import subprocess
import tempfile
import threading
import time
from pathlib import Path

import commentjson

from projectairsim import ProjectAirSimClient
from projectairsim.utils import load_scene_config_as_dict


SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_CONFIG_DIR = SCRIPT_DIR.parent / "jsbsim" / "rascal110" / "sim_config"
DEFAULT_PX4_REPO = "/home/unreal/repos/PX4-Autopilot"
BASE_ROBOT_CONFIG = "robot_rascal110_px4_sitl.jsonc"


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Load N Rascals and start one PX4 SITL instance per aircraft."
    )
    parser.add_argument("--count", type=int, default=2)
    parser.add_argument("--distro", default="ubuntu_ros2")
    parser.add_argument("--px4-repo", default=DEFAULT_PX4_REPO)
    parser.add_argument(
        "--host",
        default="127.0.0.1",
        help="Windows host address reachable from WSL (mirrored WSL uses 127.0.0.1)",
    )
    parser.add_argument("--config-dir", type=Path, default=DEFAULT_CONFIG_DIR)
    parser.add_argument("--spacing", type=float, default=8.0)
    parser.add_argument("--startup-timeout", type=float, default=90.0)
    parser.add_argument(
        "--keep-config",
        type=Path,
        help="Write generated configs here instead of a temporary directory",
    )
    args = parser.parse_args()
    if args.count < 1 or args.count > 10:
        parser.error("--count must be between 1 and 10")
    return args


def generate_configs(
    source_dir: Path, output_dir: Path, count: int, host: str, spacing: float
) -> str:
    with (source_dir / BASE_ROBOT_CONFIG).open(encoding="utf-8") as stream:
        base_robot = commentjson.load(stream)

    actors = []
    for instance in range(count):
        robot = copy.deepcopy(base_robot)
        settings = robot["controller"]["px4-settings"]
        settings["tcp-port"] = 4560 + instance
        settings["control-port"] = 14540 + instance
        settings["local-host-ip"] = host
        settings["qgc-host-ip"] = host
        settings["qgc-port"] = 14550
        robot_name = f"rascal110_px4_{instance}"
        robot_file = f"robot_{robot_name}.json"
        (output_dir / robot_file).write_text(
            json.dumps(robot, indent=2) + "\n", encoding="utf-8"
        )
        actors.append(
            {
                "type": "robot",
                "name": robot_name,
                "origin": {
                    "xyz": f"3 {-20 + instance * spacing:g} -0.5",
                    "rpy": "0 0 3.14",
                },
                "robot-config": robot_file,
            }
        )

    scene = {
        "id": f"SceneRascal110PX4Multi{count}",
        "actors": actors,
        "clock": {
            "type": "steppable",
            "step-ns": 4000000,
            "real-time-update-rate": 4000000,
            "pause-on-start": False,
        },
        "home-geo-point": {
            "latitude": 29.4296741,
            "longitude": -95.16384722,
            "altitude": 7.5,
        },
        "segmentation": {
            "initialize-ids": True,
            "ignore-existing": False,
            "use-owner-name": True,
        },
        "scene-type": "UnrealNative",
    }
    scene_name = "scene_rascal110_px4_multi.json"
    (output_dir / scene_name).write_text(
        json.dumps(scene, indent=2) + "\n", encoding="utf-8"
    )
    return scene_name


def load_scene(config_dir: Path, scene_name: str) -> str:
    scene, _ = load_scene_config_as_dict(scene_name, str(config_dir), -1)
    client = ProjectAirSimClient()
    try:
        client.connect()
        scene_id = client.request(
            {
                "method": "/Sim/LoadScene",
                "params": {"scene_config": commentjson.dumps(scene)},
                "version": 1.0,
            }
        )
        print(f"PX4_MULTI_SCENE_READY={scene_id}", flush=True)
        return str(scene_id)
    finally:
        client.disconnect()


def px4_command(repo: str, host: str, instance: int) -> str:
    repo_q = shlex.quote(repo)
    host_q = shlex.quote(host)
    work_dir = f"{repo}/build/px4_sitl_default/rootfs/instance_{instance}"
    return (
        f"mkdir -p {shlex.quote(work_dir)} && "
        "exec env "
        f"PX4_SIM_MODEL=jsbsim_rascal PX4_SIM_HOST_ADDR={host_q} "
        f"{repo_q}/build/px4_sitl_default/bin/px4 "
        f"-i {instance} -w {shlex.quote(work_dir)} "
        f"{repo_q}/build/px4_sitl_default/etc"
    )


def start_px4(args: argparse.Namespace, instance: int) -> subprocess.Popen[str]:
    command = px4_command(args.px4_repo, args.host, instance)
    return subprocess.Popen(
        ["wsl.exe", "-d", args.distro, "--", "bash", "-lc", command],
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        bufsize=1,
    )


def start_px4_group(args: argparse.Namespace) -> subprocess.Popen[str]:
    commands = []
    for instance in range(args.count):
        prefix = f"[PX4 {instance}] "
        commands.append(
            f"({px4_command(args.px4_repo, args.host, instance)} "
            f"2>&1 | sed -u 's/^/{prefix}/') &"
        )
    command = " ".join(commands) + " wait"
    return subprocess.Popen(
        ["wsl.exe", "-d", args.distro, "--", "bash", "-lc", command],
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        bufsize=1,
    )


def monitor_group_startup(
    process: subprocess.Popen[str], count: int, timeout: float
) -> threading.Thread:
    connected: set[int] = set()
    ready = threading.Event()

    def read_output() -> None:
        assert process.stdout is not None
        for line in process.stdout:
            print(line, end="", flush=True)
            if "Simulator connected" in line:
                for instance in range(count):
                    if line.startswith(f"[PX4 {instance}]"):
                        connected.add(instance)
                        break
                if len(connected) == count:
                    ready.set()
        ready.set()

    reader = threading.Thread(target=read_output, daemon=True)
    reader.start()
    if not ready.wait(timeout):
        raise RuntimeError(
            f"Only {len(connected)} of {count} PX4 instances connected before timeout"
        )
    if len(connected) != count:
        raise RuntimeError(
            f"PX4 group exited after connecting {len(connected)} of {count} instances "
            f"(code {process.poll()})"
        )
    return reader


def monitor_startup(
    processes: list[subprocess.Popen[str]], timeout: float, start_index: int = 0
) -> list[threading.Thread]:
    events = [threading.Event() for _ in processes]
    failures: queue.Queue[str] = queue.Queue()
    readers = []

    def read_output(index: int, process: subprocess.Popen[str]) -> None:
        display_index = start_index + index
        assert process.stdout is not None
        for line in process.stdout:
            print(f"[PX4 {display_index}] {line}", end="", flush=True)
            if "Simulator connected" in line:
                events[index].set()
        if not events[index].is_set():
            failures.put(
                f"PX4 {display_index} exited with code {process.poll()}"
            )
            events[index].set()

    for index, process in enumerate(processes):
        reader = threading.Thread(
            target=read_output, args=(index, process), daemon=True
        )
        reader.start()
        readers.append(reader)

    deadline = time.monotonic() + timeout
    for index, event in enumerate(events):
        remaining = deadline - time.monotonic()
        if remaining <= 0 or not event.wait(remaining):
            detail = failures.get_nowait() if not failures.empty() else "timeout"
            raise RuntimeError(f"PX4 {index} did not connect to its simulator: {detail}")
        if not failures.empty():
            raise RuntimeError(
                f"PX4 {index} did not connect to its simulator: "
                f"{failures.get_nowait()}"
            )
    return readers


def stop_px4(processes: list[subprocess.Popen[str]]) -> None:
    for process in processes:
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=10.0)
            except subprocess.TimeoutExpired:
                process.kill()


def run(args: argparse.Namespace, output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    scene_name = generate_configs(
        args.config_dir.resolve(), output_dir, args.count, args.host, args.spacing
    )
    load_scene(output_dir, scene_name)
    processes: list[subprocess.Popen[str]] = []
    try:
        # A single WSL session avoids EventFd failures seen when multiple
        # concurrent wsl.exe clients are kept open on some WSL versions.
        process = start_px4_group(args)
        processes.append(process)
        monitor_group_startup(process, args.count, args.startup_timeout)
        print(f"PX4_MULTI_CONNECTED={args.count}", flush=True)
        print(
            f"{args.count} PX4 instance(s) are running. "
            "Press Ctrl+C to stop them.",
            flush=True,
        )
        while all(process.poll() is None for process in processes):
            time.sleep(0.5)
        failed = [
            index for index, process in enumerate(processes) if process.poll() is not None
        ]
        raise RuntimeError(f"PX4 instance(s) stopped unexpectedly: {failed}")
    finally:
        stop_px4(processes)


def main() -> None:
    args = arguments()
    if args.keep_config:
        run(args, args.keep_config.resolve())
    else:
        with tempfile.TemporaryDirectory(prefix="projectairsim_rascal_multi_") as temp:
            run(args, Path(temp))


if __name__ == "__main__":
    main()
