"""Launch a local Runtime, run its pytest partition, and reap only our process."""
import argparse
from pathlib import Path
import socket
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runtime", type=Path, required=True)
    args = parser.parse_args()
    for port in (8989, 8990):
        with socket.socket() as sock:
            if sock.connect_ex(("127.0.0.1", port)) == 0:
                parser.error(f"Port {port} is occupied; stop its simulator before running Runtime")
    with (ROOT / "runtime-regression.log").open("w") as log:
        process = subprocess.Popen([str(args.runtime.resolve())], cwd=args.runtime.resolve().parent,
                                   stdout=log, stderr=subprocess.STDOUT)
        try:
            deadline = time.monotonic() + 60
            while True:
                if process.poll() is not None:
                    raise RuntimeError("Runtime exited before readiness; see runtime-regression.log")
                with socket.socket() as sock:
                    sock.settimeout(1)
                    ready = sock.connect_ex(("127.0.0.1", 8990)) == 0
                if ready:
                    break
                if time.monotonic() >= deadline:
                    raise TimeoutError("Runtime did not open its service port within 60 seconds")
                time.sleep(0.2)
            return subprocess.call(
                [sys.executable, "-m", "pytest", "-v", "--sim-host", "runtime",
                 "--timeout=180", "--junitxml=pytest-runtime.xml"],
                cwd=ROOT / "client/python/projectairsim/tests")
        finally:
            process.terminate()
            try:
                process.wait(timeout=15)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()


if __name__ == "__main__":
    sys.exit(main())
