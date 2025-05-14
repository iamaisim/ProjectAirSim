"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
Python API class for ProjectAirSim Validation Suite. 
Gives customers ability to define and enforce validation on a sim run.
"""
from collections import deque
from enum import IntEnum
from datetime import datetime
import math, inspect, os
import bisect

from projectairsim import Drone, World
from projectairsim.utils import geo_to_ned_coordinates, projectairsim_log
from junit_xml import TestCase, TestSuite


class ValidationParam:
    def __init__(self, topic, param_path: list):
        """
        Constructor not intended for customer usage, see "create_sensor_validation_param()"
        or "create_robot_info_validation_param()" for usage outside this file.
        ValidationParam type that can be used to create a range task on them.
        Args:
            topic (string): topic path as
            param_path (list): list of arguments to index/obtain the param value for topic callback.
        """
        self.topic = topic
        self.param_path = param_path


class Strictness(IntEnum):
    """Task strictness enum:
    ALWAYS,
    ATLEAST_ONCE,
    NEVER
    """

    ALWAYS = 0
    ATLEAST_ONCE = 1
    NEVER = 2


class ReachTargetType(IntEnum):
    """Supported formats for targets in Reach tasks
    GPS_DICT,
    GPS_LIST,
    STATIC_SCENE_OBJECT
    """

    GPS_DICT = 0
    GPS_LIST = 1
    NED_LIST = 2
    STATIC_SCENE_OBJECT = 3


class Task:
    def __init__(self, task_id, name, strictness):
        """
        Basic task class to help us keep state on each task
        More specialized tasks can inherit from here.

        Args:
            task_id (int): id for the task
            name (str): Name of the task, used as human-readable identifier in summary
            strictness(Strictness): Success/Failure criteria for a task
        """
        self.strictness: Strictness = strictness
        self.name = name
        self.id = task_id
        self.total_evals = 0
        self.num_occurences = 0
        self.recent_occurrences = deque([], maxlen=5)
        self.recent_absences = deque([], maxlen=5)

    def report_occurence(self, details):
        self.total_evals += 1
        self.num_occurences += 1
        self.recent_occurrences.append(details)

    def report_absence(self, details):
        self.total_evals += 1
        self.recent_absences.append(details)

    def summarize(self, print_output=True):
        desired_min_occurences = 0
        desired_max_occurences = self.total_evals
        is_failure = False
        output = ""

        if self.strictness == Strictness.ALWAYS:
            desired_min_occurences = self.total_evals
        elif self.strictness == Strictness.ATLEAST_ONCE:
            desired_min_occurences = 1
        elif self.strictness == Strictness.NEVER:
            desired_max_occurences = 0

        if desired_min_occurences <= self.num_occurences <= desired_max_occurences:
            output = f"Task Successful: {self.name} Successful"
        else:
            is_failure = True
            output = f"Task Failed: {self.name} Failed because {self.num_occurences} occurences out of {self.total_evals} evals were observed with strictness {self.strictness.name} specified."

        if print_output:
            projectairsim_log().info(output)

        return is_failure, output


class ValidationTaskModule(object):
    def __init__(self, drone: Drone, world: World):
        """
        Main class for clients to define ValidationParams, ValidationTasks
        to write their own validation modules

        Args:
            drone (Drone): ProjectAirsim Drone object which we want to define tasks on.
        """
        self.drone = drone
        self.world = world
        self.enabled = True
        self.validation_tasks = []
        self.num_tasks = 0
        self.results_summary = {}

    def summarize(self, filename=""):
        """Stop evaluation loops and Summarize all the tasks defined in the suite."""
        self.write_to_xml_file(filename)

    def write_to_xml_file(self, filename):
        results = []
        main_script_name = os.path.basename(inspect.stack()[3].filename)
        module_name = os.path.basename(inspect.stack()[2].filename)
        if filename == "":
            filename = f"{main_script_name.removesuffix('.py')}_validation_report.xml"
        task: Task
        for task in self.validation_tasks:
            test = TestCase(task.name, main_script_name)
            is_failure, output = task.summarize()
            self.results_summary[task.name] = "Failed" if is_failure else "Passed"
            err_out = None
            if is_failure:
                test.add_failure_info(output)
            results.append(test)
        xml = TestSuite(module_name, results)
        with open(filename, "w") as f:
            TestSuite.to_file(f, [xml])

    def get_final_results(self):
        return self.results_summary

    def init_task(self, name, strictness):
        """Helper to initialize a task"""
        task_id = self.num_tasks
        self.num_tasks += 1
        task_data = Task(task_id, name, strictness)
        self.validation_tasks.append(task_data)
        return task_data.id

    # Generic task helper
    def add_task(self, param: ValidationParam, func):
        """Helper to add a task to the validation suite and start evaluation"""

        def unpack_and_call(topic, val):
            for path in param.param_path:
                val = val[path]
            func(val)

        self.drone.client.subscribe(param.topic, unpack_and_call)

    def create_sensor_validation_param(self, sensor_name, sensor_type, variable_path):
        """
        Helper to create validation params from sensor topics
        Args:
            sensor_name (str): Name of the sensor (defined in robot_config usually)
            name (str): Type of the sensor (defined in robot_config usually)
            variable_path(str): Path of the variable in style of file paths (e.g. )
        """
        if not sensor_name in self.drone.sensors:
            return None
        topic = self.drone.sensors[sensor_name][sensor_type]
        if not topic:
            return None
        return ValidationParam(topic, variable_path.split("/"))

    def create_robot_info_validation_param(self, topic, variable_path):
        """
        Helper to create validation params from robot info topics
        Args:
            topic (str): Name of the robot_info topic
            variable_path(str): Path of the variable in style of file paths e.g. position/z or position/x
        """
        if not topic in self.drone.robot_info:
            return None
        topic = self.drone.robot_info[topic]
        if not topic:
            raise IndexError("Invalid Topic Path")
        return ValidationParam(topic, variable_path.split("/"))

    # General template for new tasks: one customer facing method, one wrapper and one callback

    def add_range_validation_task(
        self, name, param: ValidationParam, min_val, max_val, strictness: Strictness
    ):
        """
        Method to create a range validation task
        Args:
            param (ValidationParam): ValidationParam created from sensor_topic or robot_info topic
            min_val(num): Min value that the param can take
            max_val(num): Max value that the param can take
            strictness(Strictness): Enum to specify the evaluation criteria for the task
        """
        task_id = self.init_task(name, strictness)
        range_func = self.range_wrapper(task_id, min_val, max_val)
        self.add_task(param, range_func)

    def range_wrapper(self, task_id, min_val, max_val):
        """Wrapper to create task/target overloaded callback"""

        def callback(val):
            self.range_callback(task_id, val, min_val, max_val)

        return callback

    def range_callback(self, task_id, val, min_val, max_val):
        """Callback for Range type task"""
        if not self.enabled:
            return
        task: Task = self.validation_tasks[task_id]
        if min_val <= val <= max_val:
            task.report_occurence([datetime.now(), val])
        else:
            task.report_absence([datetime.now(), val])

    def add_reach_validation_task(
        self,
        name,
        destination_type: ReachTargetType,
        destination,
        error_thresh,
        strictness: Strictness,
    ):
        """
        Method to create a reach validation task
        Args:
            param (ValidationParam): ValidationParam created from sensor_topic or robot_info topic
            destination_type (ReachTargetType): Enum that specifies the format of the destination,
            destination (destination_type): input varies depending on type of destination
            error_thresh (num): error threshold to specify what's considering reaching in meters
            strictness(Strictness): Enum to specify the evaluation criteria for the task
        """
        task_id = self.init_task(name, strictness)

        target_ned = self.get_ned_from_target(destination_type, destination)
        param = self.create_robot_info_validation_param("actual_pose", "position")
        reach_func = self.reach_wrapper(task_id, target_ned, error_thresh)

        self.add_task(param, reach_func)

    def reach_wrapper(self, task_id, target, thresh):
        """Wrapper to create task/target overloaded callback"""

        def callback(val):
            self.reach_callback(task_id, val, target, thresh)

        return callback

    def reach_callback(self, task_id, val, target, thresh):
        """Callback for Reach type task"""
        if not self.enabled:
            return
        task: Task = self.validation_tasks[task_id]
        ned = [val["x"], val["y"], val["z"]]
        if math.dist(ned, target) <= thresh:
            task.report_occurence([datetime.now(), val])
        else:
            task.report_absence([datetime.now(), val])

    def get_ned_from_target(self, destination_type: ReachTargetType, destination):
        """Helper method to extract NED for each ReachTargetType"""
        gps_list = destination
        if destination_type == ReachTargetType.GPS_DICT:
            gps_list = [
                destination["latitude"],
                destination["longitude"],
                destination["altitude"],
            ]
        if destination_type == ReachTargetType.NED_LIST:
            return destination
        if destination_type == ReachTargetType.STATIC_SCENE_OBJECT:
            pose = self.world.get_object_pose(destination)["translation"]
            ned = pose.values()
            return ned
        else:
            return geo_to_ned_coordinates(self.drone.home_geo_point, gps_list)


class FaultInjectionModule(object):
    def __init__(self, drone: Drone, world: World):
        """
        Main class for clients to inject faults

        Args:
            drone (Drone): ProjectAirsim Drone object which we want to define tasks on.
        """
        self.drone = drone
        self.world = world
        self.timestamps = []
        self.faults = {}

    def start(self):
        self._setup_trigger_loop()

    def _setup_trigger_loop(self):
        def trigger_fault(topic, val):
            if self.timestamps:
                if val["time_stamp"] > self.timestamps[0]:
                    timestamp = self.timestamps[0]
                    self.timestamps = self.timestamps[1:]
                    faults = self.faults[timestamp]
                    for fault in faults:
                        fault()

        self.drone.client.subscribe(
            self.drone.robot_info["actual_pose"], callback=trigger_fault
        )

    def add_fault_injection_at_simtime(self, lambda_task, simtime):
        fault_list = self.faults.get(simtime, [])
        fault_list.append(lambda_task)
        self.faults[simtime] = fault_list
        bisect.insort(self.timestamps, simtime)
