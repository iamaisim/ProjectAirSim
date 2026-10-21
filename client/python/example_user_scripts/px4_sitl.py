"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Starts and stops PX4 SITL for one simulator scene. PX4 SITL can only connect to
one scene, so moving to a new location means loading the new scene and then
starting a fresh PX4 for it.

Settings come from the environment (set by run_cesium_sim.sh):
  PX4_DIR, PX4_MODEL       PX4-Autopilot checkout and SITL model [none_iris]
  PX4_PARAM_<NAME>=value   parameters PX4 applies at boot
  PI_IP, PI_PORTS          optional companion-computer MAVLink links
  LOG_DIR                  where px4.log and px4.pid go
"""

import os
import subprocess
import threading
import time

from projectairsim.utils import projectairsim_log


class Px4Sitl:
    def __init__(self):
        self.px4_dir = os.path.expanduser(os.environ.get("PX4_DIR", "~/Documents/PX4-Autopilot"))
        self.model = os.environ.get("PX4_MODEL", "none_iris")
        self.pi_ip = os.environ.get("PI_IP", "")
        self.pi_ports = [p for p in os.environ.get("PI_PORTS", "15540,15541").split(",") if p]
        self.log_dir = os.environ.get("LOG_DIR", "/tmp/projectairsim-run")
        self.rootfs = os.path.join(self.px4_dir, "build", "px4_sitl_default", "rootfs")
        self.log_path = os.path.join(self.log_dir, "px4.log")
        self.proc = None

    def start(self, timeout=30):
        """Start PX4 and wait until it is trying to reach the simulator."""
        os.makedirs(self.log_dir, exist_ok=True)
        log = open(self.log_path, "w")
        env = dict(os.environ, PX4_SIM_MODEL=self.model)
        self.proc = subprocess.Popen(
            ["../bin/px4", "-d", "../etc", "-s", "etc/init.d-posix/rcS"],
            cwd=self.rootfs, env=env, stdout=log, stderr=subprocess.STDOUT,
        )
        with open(os.path.join(self.log_dir, "px4.pid"), "w") as f:
            f.write(str(self.proc.pid))

        deadline = time.monotonic() + timeout
        while not self._log_has("Waiting for simulator"):
            if self.proc.poll() is not None:
                raise RuntimeError(f"PX4 exited during startup; see {self.log_path}")
            if time.monotonic() > deadline:
                raise RuntimeError(f"PX4 did not start within {timeout}s; see {self.log_path}")
            time.sleep(0.5)
        projectairsim_log().info(f"PX4 SITL started ({self.model}); log: {self.log_path}")

        if self.pi_ip:
            threading.Thread(target=self._add_companion_links, daemon=True).start()

    def stop(self):
        if self.proc and self.proc.poll() is None:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=10)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait()
        self.proc = None

    def _log_has(self, text):
        try:
            with open(self.log_path) as f:
                return text in f.read()
        except FileNotFoundError:
            return False

    def _add_companion_links(self):
        # PX4 starts its own MAVLink links once the sim connects; add the
        # companion-computer links after the last of them (the gimbal link)
        proc = self.proc
        while not self._log_has("mode: Gimbal"):
            if proc is not self.proc or proc.poll() is not None:
                return
            time.sleep(0.5)
        with open(self.log_path, "a") as log:
            for i, port in enumerate(self.pi_ports):
                subprocess.run(
                    ["../bin/px4-mavlink", "start", "-x", "-u", str(14590 + i),
                     "-r", "200000", "-m", "onboard", "-t", self.pi_ip, "-o", port, "-f"],
                    cwd=self.rootfs, stdout=log, stderr=subprocess.STDOUT,
                )
        projectairsim_log().info(
            f"MAVLink to companion computer {self.pi_ip} on UDP {', '.join(self.pi_ports)}"
        )
