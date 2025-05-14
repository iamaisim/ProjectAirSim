"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
This script is used to run a test bench interactively.
You can specify the test bench file, mission script and drone ID to use.
"""

from projectairsim.test import TestBench
from projectairsim import ProjectAirSimClient
import argparse, asyncio, os



parser = argparse.ArgumentParser(description="Project AirSim Test Bench")
parser.add_argument("--test-bench", type=str, default="testbench_example.csv", help="Path to test bench file")
parser.add_argument("--mission-script", type=str, default="mission_testing_example.py", help="Python file with the mission script")
parser.add_argument("--drone-id", type=str, default="Drone1", help="Drone ID to use for the test bench")

## Add a simple flag to specify if we want to run the test bench to run only failed tests
parser.add_argument("--failed-only", action="store_true", help="Run only failed tests from the test bench")
    
args = parser.parse_args()
print(args)

async def main():
    mission_script_module = os.path.basename(args.mission_script).strip(".py")
    print(mission_script_module)
    mission_script = __import__(mission_script_module)
    test_bench_name = os.path.basename(args.test_bench).strip(".csv")
    
    test_bench = TestBench.load_test_bench_from_file(args.test_bench, f"testbench-{test_bench_name}-{mission_script_module}")
    try:
        client = ProjectAirSimClient()
        client.connect()
        await test_bench.test_and_validate_scenarios(client, mission_script.mission_script, args.drone_id, run_failed_only=args.failed_only)
    finally:
        client.disconnect()

if __name__ == "__main__":
    asyncio.run(main())