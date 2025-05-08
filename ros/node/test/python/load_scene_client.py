import rclpy
from rclpy.node import Node

from projectairsim_ros.srv import *

import time
import logging

LOGGER = logging.getLogger(__name__)

class ServiceClient(Node):
    #
    #  delay added to address intermittent failures in service calls, always a comms timeout.
    #  The delay fix suggests async setup somewhere in rclpy.
    #
    #  Repeated test suites also show intermittent wait_for_service() failures, which the retry
    #  loop in the constructor addresses.
    #
    def __init__(self, service_name, service_type, delay=0.5):
        super().__init__(f'test_{service_name.__class__.__name__}_client')
        self.service_name = service_name
        self.service_type = service_type
        self.client = self.create_client(service_type, service_name)
        while not self.client.wait_for_service(timeout_sec=1.0):
            LOGGER.warning(f'Service {self.service_name} {self.service_type} not available, waiting again...\n')
        self.req = service_type.Request()
        time.sleep(delay)

    #
    #  self.future.result() is None if the call fails
    #
    def call(self, timeout=30.0):
        self.future = self.client.call_async(self.req)
        rclpy.spin_until_future_complete(self, self.future, None, timeout)
        return self.future.result()


class LoadSceneClientAsync(Node):
    #
    # ROS2 node to run the load_scene service invocation
    #
    def __init__(self, node_name, service_timeout=10.0):
        super().__init__(node_name)
        self.service_client = ServiceClient('/airsim_node/load_scene', LoadScene)
        self.service_timeout = service_timeout

    def send_request(self, scene_file, is_primary_client=True):
        self.service_client.req.scene_file = scene_file
        self.service_client.req.is_primary_client = is_primary_client

        response = self.service_client.call(self.service_timeout)
        self.service_client.destroy_node()

        return response
