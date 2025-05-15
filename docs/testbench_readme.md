
# Project AirSim - Test Bench Framework

The **Test Bench** system is a component of Project AirSim designed to automate the process of testing drone behavior in simulation under a wide variety of environmental and operational conditions.

It allows users to:
- Create combinations of scenes and environmental settings (e.g. time of day, weather, wind)
- Inject faults to test robustness
- Run a user-defined mission function
- Validate the mission using predefined criteria
- Export results and measure test coverage

---

## Core Components and Concepts

### `TestBench`
`TestBench` is the orchestrator of simulation-based testing. It manages:
- **Validation Modules**: These are custom Python modules that define validation tasks (e.g., checking that the drone reaches its goal).
- **Fault Injection Modules**: Optional modules that simulate failures or disturbances (e.g., wind gusts, sensor noise).
- **Scenarios**: Each scenario includes a scene (e.g., a `.jsonc` file in AirSim) and an `EnvProfile` describing environmental parameters.

It provides methods to run all scenarios, validate outcomes, retry failed tests, save/load test definitions, and export results.

---

### `EnvProfile`
`EnvProfile` defines the environmental conditions for each scenario. It can include:
- **Weather**: Type and intensity (rain, snow, dust)
- **Wind Velocity**: As a 3D vector
- **Sunlight Intensity**: Simulates daylight changes
- **Cloud Shadow Strength**: Affects scene lighting
- **Time of Day**: Affects sun position

These settings are applied before running a test to simulate real-world conditions.

---

### `SceneRandomizationSpec`
This class allows users to:
- Define a list of scenes to use
- Specify the range and granularity of environmental variations
- Generate the full list of scenario combinations automatically

For example, if you specify 3 times of day and 2 weather types, this will generate 6 scenarios (3 x 2 combinations).

---

## File Descriptions

| File | Description |
|------|-------------|
| `testbench_generate_new.py` | Automatically generates a test bench based on a variation spec (`.jsonc` file). |
| `testbench_create_custom.py` | Manually defines a few specific test scenarios and appends them to the test bench. |
| `testbench_example.csv` | CSV file listing the generated test scenarios. Each row is a unique environmental configuration. |
| `testbench_execute.py` | Loads a test bench, connects to the simulation, and runs the mission function over each scenario. |
| `testbench_estimate_coverage.py` | Computes how many scenarios from a full spec were covered by the current test bench. |
| `testbench_variaton_spec.jsonc` | A JSONC file with ranges for parameters like time-of-day, wind, and weather used for randomization. |
| `testbench_complete_variations.jsonc` | A superset spec used to measure test coverage against all possible environment combinations. |

---

## ⚙️ Step-by-Step Usage

### 1. Generate a Test Bench
This creates a CSV file with all combinations of environmental parameters:

```bash
python testbench_generate_new.py     --output-filename testbench_example.csv     --scenes scene_basic_drone.jsonc     --variation-spec testbench_variaton_spec.jsonc
```

This will combine the selected scenes and the environmental variations into a test matrix.

---

### 2. Add Custom Scenarios (Optional)
You can manually define and add specific scenarios, for example:

```bash
python testbench_create_custom.py
```

This adds hand-picked test cases, like a windy rainy noon, into the existing bench.

---

### 3. 🚁 Run the Tests
Execute each scenario using a mission function:

```bash
python testbench_execute.py     --test-bench testbench_example.csv     --mission-script mission_testing_example.py     --drone-id Drone1
```

- `--failed-only` runs only scenarios that previously failed.
- Results are saved automatically in CSV format.

---

### 4. Estimate Coverage
This tells you how much of your total intended scenario space is covered by the current test bench:

```bash
python testbench_estimate_coverage.py
```

This compares your test bench against `testbench_complete_variations.jsonc`.

---

## Example Mission Script

Each mission script must define a coroutine like this:

```python
async def mission_script(drone, world, client):
    await drone.takeoff()
    await drone.fly_to((10, 0, -5))
    await drone.land()
```

This is executed for every test scenario. The validation module then checks if the mission succeeded.

---

## CSV Output Format

The test bench CSV (`testbench_example.csv`) looks like:

```csv
Id,scene_name,weather,light_intensity,cloud_shadow_strength,wind_velocity,time_of_day
scenario-0,scene_basic_drone.jsonc,[<WeatherParameter.RAIN: 0>,0.3],30000,0.5,[2,0,0],10:00:00
```

Each row defines one test configuration.

---

## Extra Notes

- `TestBench` supports resuming from where it left off.
- You can retry only failed scenarios.
- Results are stored in summary CSV files.
- All scenarios are reproducible and serializable.

---

## Summary

This system allows automated, repeatable, and systematic testing of drone missions across diverse conditions. You can generate tests programmatically, execute them in simulation, and validate results—all in a reproducible pipeline.


---

## What Happens During a Rerun with `--failed-only`

When you execute the test bench with the `--failed-only` flag, it behaves as follows:

### Step-by-Step Behavior

1. **Loads the existing test result CSV file**, typically named:
   ```
   Hello Drone Test Bench-mission_script-test-report.csv
   ```

2. **Identifies all failed scenarios**:
   - Any scenario with at least one validation condition returning `False` is marked as failed.
   - Scenarios that passed all conditions are skipped.

3. **Reruns only the failed scenarios**:
   - Loads the scene again
   - Applies the environmental profile
   - Re-runs the mission script
   - Re-applies the validation logic

4. **Saves the rerun results to a new file**:
   ```
   Hello Drone Test Bench-mission_script-rerun-results.csv
   ```
   This ensures that original results remain unchanged for traceability and comparison.

5. **Optionally updates scenario-specific XML reports**:
   Each scenario rerun generates a fresh XML file with validation summary, e.g.:
   ```
   scenario-12-validation-report-retry.xml
   ```

### Advantages of This Approach

- Keeps the original test results intact
- Allows iterative testing without data loss
- Helps in identifying improvements or regressions in mission logic
- Enables targeted debugging on failed cases only



---

## Mission Scripts – Structure and Integration

Mission scripts define the autonomous behavior the drone should execute in each test scenario.

### Required Structure

Each script must include:
- An `async def mission_script(drone, world, client)` function.
- No top-level execution code unless it is protected with:

```python
if __name__ == "__main__":
    asyncio.run(mission_script(...))
```

This prevents unintended execution when the mission script is imported dynamically by `testbench_execute.py`.

---

## Validation Module (`mission_validation_module.py`)

This module defines what constitutes a successful mission by adding validation tasks to a `ValidationTaskModule`.

### Example tasks added:
- **Altitude limits**: Drone must stay within ±100 meters of its spawn altitude.
- **Proximity constraints**: 
  - `NEVER` get closer than 10m to a wind turbine object.
  - `ATLEAST_ONCE` reach within 50m of a GPS-defined target.

These are injected using:

```python
inject_validation_tasks(validation_task_module)
```

After the mission, results are summarized and saved with:

```python
summarize_validation_tasks("validation-output.xml")
```

---

## Output Validation File (`*.xml`)

Each mission run produces a validation report XML file (e.g., `scenario-0-validation-report.xml`). It contains:

- Number of tests run
- Names of each validation task
- Pass/fail status
- Failure messages explaining why a condition was violated

### Example excerpt:
```xml
<testcase name="Never be closer than 10m of Wind Turbine object">
  <failure type="failure" message="...2286 occurences..."/>
</testcase>
```

These outputs are compatible with CI tools or can be manually reviewed for debugging or safety verification.

---
