#!/usr/bin/env python3

import sys
import time
import argparse
import logging

from projectairsim_ros.srv import LoadScene
import rclpy
from rclpy.node import Node


class LoadSceneClientAsync(Node):

    def __init__(self, service_timeout=10.0):
        super().__init__('load_scene_client_async')
        self.service_name = '/airsim_node/load_scene'
        self.service_type = LoadScene
        self.cli = self.create_client(LoadScene, self.service_name)
        while not self.cli.wait_for_service(timeout_sec=1.0):
            self.get_logger().info(f'service {self.service_name} not available, waiting again...')
        self.req = LoadScene.Request()
        self.service_timeout = 10.0

    def send_request(self, scene_file, is_primary_client=True):
        self.req.scene_file = scene_file
        self.req.is_primary_client = is_primary_client
        self.future = self.cli.call_async(self.req)

        rclpy.spin_until_future_complete(self, self.future, None, 10.0)
        return self.future.result()


def main(args=None):
    parser = argparse.ArgumentParser(
        description="Scene file service client to trigger scene file loading",
    )

    parser.add_argument(
        "--scenefile",
        help=("the config file to load"),
        type=str,
        default="not_sepcified.jsonc",
    )

    parser.add_argument(
        "--primaryclient",
        help=("Is this client the primary client"),
        type=bool,
        default=True,
    )

    args = parser.parse_args(rclpy.utilities.remove_ros_args(sys.argv)[1:])

    rclpy.init()

    retries = 0
    while retries < 5:
        load_scene_client = LoadSceneClientAsync()
        load_scene_client.get_logger().info('Attempting to load scene %s' % args.scenefile)
        response = load_scene_client.send_request(args.scenefile, args.primaryclient)

        if response is not None and response.success:    
            load_scene_client.get_logger().info('Result of load scene %d' % response.success)
        else:
            load_scene_client.get_logger().info('Result of load scene is None' )
            
        if response is not None and response.success:
            break

        time.sleep(1)
        retries += 1

    load_scene_client.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()