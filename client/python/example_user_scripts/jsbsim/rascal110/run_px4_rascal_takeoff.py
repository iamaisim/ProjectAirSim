"""Run PX4 SITL in WSL2 and command a Rascal takeoff non-interactively."""

import argparse
import queue
import shlex
import subprocess
import threading
import time
from pathlib import Path


DEFAULT_PX4_REPO = "/home/devcontainers/PX4-Autopilot"
DEFAULT_WINDOWS_HOST = "172.18.144.1"


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--distro", default="ubuntu_ros2")
    parser.add_argument("--px4-repo", default=DEFAULT_PX4_REPO)
    parser.add_argument("--windows-host", default=DEFAULT_WINDOWS_HOST)
    parser.add_argument("--takeoff-altitude", type=float, default=100.0)
    parser.add_argument("--takeoff-distance", type=float, default=1500.0)
    parser.add_argument("--flight-seconds", type=float, default=90.0)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    build_path = f"{args.px4_repo}/build/px4_sitl_default"
    command = (
        f"cd {shlex.quote(args.px4_repo)} && "
        f"PX4_SIM_HOST_ADDR={shlex.quote(args.windows_host)} "
        "Tools/sitl_run.sh "
        f"{shlex.quote(f'{build_path}/bin/px4')} "
        "none none rascal-electric none "
        f"{shlex.quote(args.px4_repo)} {shlex.quote(build_path)}"
    )
    if args.dry_run:
        print(
            "WSL_COMMAND="
            + subprocess.list2cmdline(
                ["wsl.exe", "-d", args.distro, "--", "bash", "-lc", command]
            )
        )
        return

    process = subprocess.Popen(
        ["wsl.exe", "-d", args.distro, "--", "bash", "-lc", command],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )
    output_queue: queue.Queue[str | None] = queue.Queue()
    output_lines: list[str] = []

    def read_output() -> None:
        assert process.stdout is not None
        for line in process.stdout:
            output_lines.append(line)
            output_queue.put(line)
            print(line, end="", flush=True)
        output_queue.put(None)

    reader = threading.Thread(target=read_output, daemon=True)
    reader.start()

    def wait_for(pattern: str, timeout_sec: float) -> None:
        deadline = time.monotonic() + timeout_sec
        checked = 0
        while time.monotonic() < deadline:
            while checked < len(output_lines):
                if pattern.lower() in output_lines[checked].lower():
                    return
                checked += 1
            if process.poll() is not None:
                raise RuntimeError(
                    f"PX4 exited with code {process.returncode} before '{pattern}'"
                )
            try:
                output_queue.get(timeout=0.2)
            except queue.Empty:
                pass
        raise TimeoutError(f"PX4 output did not contain '{pattern}'")

    def send(command_text: str) -> None:
        if process.stdin is None:
            raise RuntimeError("PX4 stdin is unavailable")
        print(f"PX4_COMMAND={command_text}", flush=True)
        process.stdin.write(command_text + "\n")
        process.stdin.flush()

    try:
        wait_for("Simulator connected on TCP port 4560", 60.0)
        wait_for("home set", 60.0)
        send(f"param set MIS_TAKEOFF_ALT {args.takeoff_altitude:g}")
        time.sleep(1.0)
        helper_path = Path(__file__).with_name(
            "command_px4_fixed_wing_takeoff.py"
        ).resolve()
        drive, relative_path = helper_path.as_posix().split(":/", 1)
        helper_wsl_path = f"/mnt/{drive.lower()}/{relative_path}"
        helper = subprocess.run(
            [
                "wsl.exe",
                "-d",
                args.distro,
                "--",
                "python3",
                helper_wsl_path,
                "--altitude-gain",
                f"{args.takeoff_altitude:g}",
                "--distance",
                f"{args.takeoff_distance:g}",
            ],
            text=True,
            capture_output=True,
            timeout=60.0,
        )
        print(helper.stdout, end="", flush=True)
        if helper.returncode != 0:
            raise RuntimeError(
                "PX4 takeoff command failed: " + helper.stderr.strip()
            )
        time.sleep(min(25.0, args.flight_seconds))
        send("commander status")
        time.sleep(max(0.0, args.flight_seconds - 25.0))
        send("listener vehicle_local_position -n 1")
        send("listener vehicle_attitude -n 1")
        send("listener airspeed -n 1")
        send("commander status")
        time.sleep(3.0)
    finally:
        if process.poll() is None:
            try:
                send("commander disarm -f")
                time.sleep(1.0)
                send("shutdown")
                process.wait(timeout=15.0)
            except Exception:
                process.terminate()
                try:
                    process.wait(timeout=5.0)
                except subprocess.TimeoutExpired:
                    process.kill()
        reader.join(timeout=2.0)

    if process.returncode not in (0, None):
        raise RuntimeError(f"PX4 exited with code {process.returncode}")


if __name__ == "__main__":
    main()
