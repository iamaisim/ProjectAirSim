#!/usr/bin/env python3
import asyncio

import rclpy
from projectairsim_rosbridge.webrtc_image_publisher import WebRTCImagePublisher, run


def run_webrtc_node():
    """Initializes the ROS2 node and starts WebRTC communication."""
    rclpy.init()
    webrtc_node = WebRTCImagePublisher()

    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)

    try:
        loop.run_until_complete(run(webrtc_node))
    except KeyboardInterrupt:
        print("Shutting down WebRTC...")
    finally:
        webrtc_node.destroy_node()
        rclpy.shutdown()

if __name__ == "__main__":
    run_webrtc_node()
