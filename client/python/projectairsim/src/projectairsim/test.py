from projectairsim import World, ProjectAirSimClient, Drone
from projectairsim.types import WeatherParameter
from projectairsim.utils import projectairsim_log
from projectairsim.validate import ValidationTaskModule
from datetime import datetime
from enum import IntEnum
import numpy as np
import csv
import commentjson


class TestBench(object):
    def __init__(self, name, validation_module, fault_injection_module, scenarios=[]):
        self.name = name
        self.validation_task_file = validation_module
        self.fault_injection_file = fault_injection_module
        self.scenarios = scenarios
        self.env_variations = []
        self.results = []

    def load_test_bench_from_file(filename, name=""):
        test_bench = None
        with open(filename, "r") as csvfile:
            reader = csv.reader(csvfile)
            if name == "":
                name = filename.split(".")[0]
            row = next(reader)
            if row[0] != "ValidationModule":
                raise Exception("Invalid test bench file")
            validation_task_file = __import__(row[1])
            row = next(reader)
            if row[0] == "FaultInjectionModule":
                fault_injection_file = __import__(row[1])

            test_bench = TestBench(name, validation_task_file, fault_injection_file)

            # Skip the header row
            row = next(reader)
            scenarios = []
            env_variations = []
            for row in reader:
                scene = row[1]
                envprofile = EnvProfile.load_from_string(row[2])
                env_variations.append(envprofile)
                scenarios.append([scene, envprofile])
            test_bench.scenarios = scenarios
            test_bench.env_variations = env_variations
        return test_bench

    # If there are already scenarios added, this will append to the list
    # Does not overwrite the existing list
    def add_test_scenarios(self, scenarios):
        self.scenarios = self.scenarios + scenarios

    def set_mission_func(self, mission_func, drone_id):
        self.mission_func = mission_func
        self.drone = drone_id

    async def test_and_validate_scenarios(
        self,
        client: ProjectAirSimClient,
        mission_func,
        drone_id: str,
        run_failed_only=False,
    ):
        try:
            projectairsim_log().info(f"Starting test bench: {self.name}")
            results_summary_file = f"{self.name}-{mission_func.__name__}-test-report.csv"

            projectairsim_log().info(
                f"Results will be saved to: {results_summary_file}"
            )
            self.save_test_bench(f"{self.name}.csv")
            projectairsim_log().info(f"Saving test bench to: {self.name}.csv")

            resume_from = self.try_load_results(results_summary_file)
            if resume_from == len(self.scenarios):
                projectairsim_log().info("ALL SCENARIOS HAVE BEEN VALIDATED")
                if not run_failed_only:
                    return self.results
                projectairsim_log().info(
                    "Running failed tests only since --failed-only flag is set"
                )
                failed_indices = []
                for i, result in enumerate(self.results):
                    success = True
                    for key in result.keys():
                        success = success and result[key]
                    if not success:
                        failed_indices.append(i)

                for failed_scenario in failed_indices:
                    self.print_header(failed_scenario)
                    scene = self.scenarios[failed_scenario][0]
                    envprofile = self.scenarios[failed_scenario][1]
                    output_file = f"scenario-{failed_scenario}-validation-report-retry.xml"
                    await self.validate_scenario(
                        client,
                        scene,
                        envprofile,
                        mission_func,
                        drone_id,
                        output_file,
                    )

                results_rerun_file = (
                    f"{self.name}-{mission_func.__name__}-rerun-results.csv"
                )
                self.save_test_results_to_csv(results_rerun_file)
                return self.results

            projectairsim_log().info(
                f"Previous partial results are avaiable. Resuming from scenario: {resume_from}"
            )
            if run_failed_only:
                projectairsim_log().info(
                    f"Ignoring --failed-only flag since there are partial results available"
                )

            for i, test_case in enumerate(self.scenarios[resume_from:]):
                self.print_header(i + resume_from)
                scene = test_case[0]
                envprofile = test_case[1]
                output_file = f"scenario-{resume_from+i}-validation-report.xml"
                await self.validate_scenario(
                    client, scene, envprofile, mission_func, drone_id, output_file
                )
            self.save_test_results_to_csv(results_summary_file)
            return self.results
        # logs exception on the console
        except Exception as err:
            projectairsim_log().error(f"Exception occurred: {err}", exc_info=True)
        finally:
            self.save_test_results_to_csv(results_summary_file)
            client.unsubscribe_all()

    def print_header(self, scenario_index):
        projectairsim_log().info("-------------------------------------------------")
        projectairsim_log().info(f"       Testing SCENARIO-{scenario_index}         ")
        projectairsim_log().info("-------------------------------------------------")

    async def validate_scenario(
        self, client, scene, envprofile, mission_func, drone_id, output_file
    ):
        world = World(client, scene, delay_after_load_sec=0)
        drone = Drone(client, world, drone_id)
        envprofile.configure(drone, world)
        validation_task_module = ValidationTaskModule(drone, world)
        self.validation_task_file.inject_validation_tasks(validation_task_module)
        self.fault_injection_file.inject_faults(drone, world)
        await mission_func(drone, world, client)
        self.validation_task_file.summarize_validation_tasks(output_file)
        result = validation_task_module.get_final_results()
        self.results.append(result)
        client.unsubscribe_all()

    def save_test_bench(self, filename):
        # Save the test bench to a csv file
        # TODO: check if the file exists first
        with open(filename, "w", newline="") as csvfile:
            writer = csv.writer(csvfile)
            # Need to add validation module and fault injection module to the header
            if self.validation_task_file is None:
                writer.writerow(["ValidationModule", "None"])
            else:
                writer.writerow(["ValidationModule", self.validation_task_file.__name__])
            if self.fault_injection_file is None:
                writer.writerow(["FaultInjectionModule", "None"])
            else:
                writer.writerow(
                    ["FaultInjectionModule", self.fault_injection_file.__name__]
                )
            writer.writerow(
                [
                    "Id",
                    "scene_name",
                    "weather",
                    "light_intensity",
                    "cloud_shadow_strength",
                    "wind_velocity",
                    "time_of_day",
                ]
            )
            for i, scenario in enumerate(self.scenarios):
                scene = scenario[0]
                envprofile: EnvProfile = scenario[1]
                writer.writerow([f"scenario-{i}", scene, str(envprofile)])

    def save_test_results_to_csv(self, filename):
        # Save the test results to a csv file
        with open(filename, "w", newline="") as csvfile:
            writer = csv.writer(csvfile)
            header = ["Scenario"]
            if len(self.results) > 0:
                for key in self.results[0].keys():
                    header.append(key)
            writer.writerow(header)
            for i, result in enumerate(self.results):
                row = [f"scenario-{i}"]
                for key in result.keys():
                    row.append(str(result[key]))
                writer.writerow(row)

    def try_load_results(self, filename):
        # Try to load the results from a csv file
        # If the file does not exist, then do nothing
        # If the file exists, then load the results into the results list
        try:
            with open(filename, "r") as csvfile:
                reader = csv.reader(csvfile)
                header = next(reader)

                for row in reader:
                    result = {}
                    for i in range(1, len(header)):
                        result[header[i]] = row[i]
                    self.results.append(result)
            return len(self.results)
        except FileNotFoundError:
            return 0

    def estimate_coverage(self, json_file):

        spec = SceneRandomizationSpec(["scene_basic_drone.jsonc"])
        spec.add_variations_from_spec(json_file)
        _ = spec.generate_scenario_list()
        env_variations = spec.env_variations

        env_variations_str = set([str(x) for x in env_variations])

        # Import test bench
        tested = self.env_variations
        tested_str = set([str(x) for x in tested])

        final = tested_str.intersection(env_variations_str)

        coverage = len(final) / len(env_variations_str) * 100
        print(f"Coverage:{coverage}%")
        return coverage


def clamp(min_val, val, max_val):
    return max(min(val, max_val), min_val)


class EnvProfile(object):
    def __init__(
        self,
        weather=None,
        light_intensity=None,
        cloud_shadow_strength=None,
        wind_velocity=None,
        time_of_day=None,
    ):
        """
        Simple Env Profile. Represents all the various env based
        parameters that can adjusted to randomize the scene.
        """
        self.weather = weather
        self.light_intensity = light_intensity
        self.cloud_shadow_strength = cloud_shadow_strength
        self.wind_velocity = wind_velocity
        self.time_of_day = time_of_day

    def load_from_string(profile_str):
        """
        Loads the profile from a string. The string should be in the format:
        weather,light_intensity,cloud_shadow_strength,wind_velocity,time_of_day
        """
        profile = profile_str.split("|")
        for i, entry in enumerate(profile):
            if entry == "None":
                profile[i] = None

        light_intensity = float(profile[1]) if profile[1] is not None else None
        cloud_shadow_strength = float(profile[2]) if profile[2] is not None else None
        time_of_day = (
            datetime.strptime(profile[4], "%H:%M:%S")
            if profile[4] is not None
            else None
        )

        # These are both lists, so need to parse them separately
        weather_str = profile[0]
        weather = None
        if weather_str is not None:
            weather_str = weather_str.strip("[]")
            weather_split = weather_str.split(",")
            weather_param = WeatherParameter(
                int(weather_split[0].strip("<>").split(":")[1])
            )
            weather = [weather_param, float(weather_split[1])]

        wind_str = profile[3]
        wind_velocity = None
        if wind_str is not None:
            wind_str = wind_str.strip("[]")
            wind_velocity = [float(x) for x in wind_str.split(",")]

        return EnvProfile(
            weather, light_intensity, cloud_shadow_strength, wind_velocity, time_of_day
        )

    def configure(self, drone: Drone, world: World):
        if self.time_of_day is not None:
            world.set_sun_position_from_date_time(self.time_of_day)
        if self.cloud_shadow_strength is not None:
            world.set_cloud_shadow_strength(self.cloud_shadow_strength)
        if self.light_intensity is not None:
            world.set_sunlight_intensity(self.light_intensity)
        if self.wind_velocity is not None:
            world.set_wind_velocity(
                self.wind_velocity[0], self.wind_velocity[1], self.wind_velocity[2]
            )
        if self.weather is not None:
            world.enable_weather_visual_effects()
            world.set_weather_visual_effects_param(self.weather[0], self.weather[1])

    def __str__(self):
        if self.time_of_day is None:
            return f"{self.weather}|{self.light_intensity}|{self.cloud_shadow_strength}|{self.wind_velocity}|None"
        return f"{self.weather}|{self.light_intensity}|{self.cloud_shadow_strength}|{self.wind_velocity}|{self.time_of_day.strftime('%H')}:00:00"


class SceneParams(IntEnum):
    TIMEOFDAY = 0
    SUNLIGHT = 1
    CLOUDSHADOW = 2
    WIND = 3
    WEATHER = 4


class SceneRandomizationSpec(object):
    def __init__(self, list_of_scenes):
        self.scenes = list_of_scenes
        self.variations = {
            SceneParams.TIMEOFDAY: [None],
            SceneParams.SUNLIGHT: [None],
            SceneParams.CLOUDSHADOW: [None],
            SceneParams.WIND: [None],
            SceneParams.WEATHER: [None],
        }
        self.env_variations = []
        self.scenario_list = None

    def get_estimated_number_of_scenarios(self):
        l0 = self.scenes
        l1 = self.variations[SceneParams.TIMEOFDAY]
        l2 = self.variations[SceneParams.SUNLIGHT]
        l3 = self.variations[SceneParams.CLOUDSHADOW]
        l4 = self.variations[SceneParams.WIND]
        l5 = self.variations[SceneParams.WEATHER]
        return len(l0) * len(l1) * len(l2) * len(l3) * len(l4) * len(l5)

    def generate_scenario_list(self):
        if self.scenario_list is not None:
            return self.scenario_list
        l0 = self.scenes
        l1 = self.variations[SceneParams.TIMEOFDAY]
        l2 = self.variations[SceneParams.SUNLIGHT]
        l3 = self.variations[SceneParams.CLOUDSHADOW]
        l4 = self.variations[SceneParams.WIND]
        l5 = self.variations[SceneParams.WEATHER]
        scenario_list = []
        env_variations = []
        # Unfortunately really ugly code.
        # However,its rather simple in complexity so best to leave it rather than complicate it.
        for tod in l1:
            for sun_intensity in l2:
                for cloud in l3:
                    for wind in l4:
                        for weather in l5:
                            envprofile = EnvProfile(
                                weather=weather,
                                light_intensity=sun_intensity,
                                cloud_shadow_strength=cloud,
                                wind_velocity=wind,
                                time_of_day=tod,
                            )
                            env_variations.append(envprofile)
        for scene in l0:
            for envprofile in env_variations:
                scenario_list.append([scene, envprofile])

        self.scenario_list = scenario_list
        self.env_variations = env_variations
        return scenario_list

    def add_time_of_day_variations(self, list_of_datetimes):
        self.variations[SceneParams.TIMEOFDAY] = list_of_datetimes

    def add_wind_intensity_variation(self, list_vels):
        self.variations[SceneParams.WIND] = list_vels

    def add_wind_intensity_variations(self, init_val, final_val, step):
        for i in range(init_val, final_val, step):
            self.variations[SceneParams.WIND].append([i, 0, 0])
            self.variations[SceneParams.WIND].append([0, i, 0])
            self.variations[SceneParams.WIND].append([0, 0, i])

    def add_weather_variations(self, list_of_weather_pairs):
        # Need to add None to add the default no weather variation, because no way to specify that.
        self.variations[SceneParams.WEATHER] = list_of_weather_pairs

    def add_cloud_shadow_variation(self, init_val, final_val, step):
        init_val = clamp(0, init_val, 1.0)
        final_val = clamp(0, final_val, 1.0)
        self._add_variation(SceneParams.CLOUDSHADOW, init_val, final_val, step)

    def add_sunlight_intensity_variation(self, init_val, final_val, step):
        init_val = clamp(0, init_val, 75000)
        final_val = clamp(0, final_val, 75000)
        self._add_variation(SceneParams.SUNLIGHT, init_val, final_val, step)

    def add_time_of_day_variation(self, init_val, final_val, step):
        tod_variations = []
        for hour in range(init_val, final_val, int(step)):
            today = datetime.now()
            time_of_day = datetime(today.year, today.month, today.day, hour, 0, 0)
            self.variations[SceneParams.TIMEOFDAY].append(time_of_day)

    def add_weather_variation(
        self, weather_type: WeatherParameter, init_val, final_val, step
    ):
        intensities = list(np.arange(init_val, final_val, step))
        for i in range(len(intensities)):
            intensities[i] = round(intensities[i], 1)
        for intensity in intensities:
            self.variations[SceneParams.WEATHER].append([weather_type, intensity])

    def _add_variation(self, variation_type: SceneParams, init_val, final_val, step):
        variations = list(np.arange(init_val, final_val, step))
        for i in range(len(variations)):
            variations[i] = round(variations[i], 1)
        variations.append(None)
        self.variations[variation_type] = variations
    
    def add_variations_from_spec(self, jsonc_spec):
        jsonc_file = open(jsonc_spec, "r")
        parameters = commentjson.load(jsonc_file)
        time_of_day_values = [
            parameters["parameters"]["time-of-day"]["start"],
            parameters["parameters"]["time-of-day"]["end"],
            parameters["parameters"]["time-of-day"]["granularity"],
        ]

        weather_values = {}
        for weather_type, weather_info in parameters["parameters"]["weather"].items():
            weather_values[weather_type] = [
                weather_info["start"],
                weather_info["end"],
                weather_info["granularity"],
            ]

        wind_speed_values = [
            parameters["parameters"]["wind-speed"]["start"],
            parameters["parameters"]["wind-speed"]["end"],
            parameters["parameters"]["wind-speed"]["granularity"],
        ]

        self.add_time_of_day_variation(
            time_of_day_values[0], time_of_day_values[1], time_of_day_values[2]
        )
        self.add_wind_intensity_variations(
            wind_speed_values[0], wind_speed_values[1], wind_speed_values[2]
        )
        for weather_type, weather_info in weather_values.items():
            if weather_type == "rain":
                self.add_weather_variation(
                    WeatherParameter.RAIN,
                    weather_info[0],
                    weather_info[1],
                    weather_info[2],
                )
            elif weather_type == "snow":
                self.add_weather_variation(
                    WeatherParameter.SNOW,
                    weather_info[0],
                    weather_info[1],
                    weather_info[2],
                )
            elif weather_type == "dust":
                self.add_weather_variation(
                    WeatherParameter.DUST,
                    weather_info[0],
                    weather_info[1],
                    weather_info[2],
                )

