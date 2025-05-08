import asyncio
import json
import logging
import numpy as np
import websockets
import time
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
from aiortc import RTCPeerConnection, RTCSessionDescription, MediaStreamTrack

logging.basicConfig(level=logging.INFO)

# WebRTC signaling server configuration
SIGNALING_SERVER_URL = "ws://localhost:8080"


class WebRTCImagePublisher(Node):
    """
    ROS2 node that publishes images received via WebRTC to the 'airsim_streaming_camera/image_raw' topic.
    """
    def __init__(self):
        super().__init__('webrtc_image_publisher')
        self.publisher_ = self.create_publisher(Image, 'airsim_streaming_camera/image_raw', 10)
        self.br = CvBridge()
        self.get_logger().info("WebRTC Image Publisher node started.")

    def publish_image(self, img):
        """Converts a NumPy image to a ROS2 Image message and publishes it."""
        ros_image = self.br.cv2_to_imgmsg(img, encoding="rgb8")
        ros_image.header.stamp = self.get_clock().now().to_msg()
        self.publisher_.publish(ros_image)


class VideoDisplayTrack(MediaStreamTrack):
    """
    WebRTC video track handler that processes incoming frames and publishes them to ROS2.
    """
    kind = "video"

    def __init__(self, track, image_publisher):
        super().__init__()
        self.track = track
        self.image_publisher = image_publisher
        self.frame_counter = 0
        self.last_time = time.time()
        self.current_fps = 0.0

    async def recv(self):
        """Receives a video frame, converts it to a NumPy array, and publishes it."""
        frame = await self.track.recv()
        img = frame.to_ndarray(format="rgb24")
        self.image_publisher.publish_image(img)
        
        return frame


async def run(image_publisher):
    """Handles WebRTC communication with the signaling server and subscribes to the video stream."""
    print(f"Connecting to signaling server at {SIGNALING_SERVER_URL}...")
    try:
        async with websockets.connect(SIGNALING_SERVER_URL) as ws:
            await ws.send(json.dumps({"type": "listStreamers"}))
            print("Sent 'listStreamers' request.")

            pc = RTCPeerConnection()

            @pc.on("icecandidate")
            async def on_icecandidate(event):
                if event.candidate is not None:
                    await ws.send(json.dumps({"type": "candidate", "candidate": event.candidate.to_dict()}))
                    logging.info("Sent ICE candidate.")

            @pc.on("track")
            async def on_track(track):
                print("Receiving video...")
                if track.kind == "video":
                    video_display = VideoDisplayTrack(track, image_publisher)
                    while True:
                        await video_display.recv()

            async for message in ws:
                try:
                    data = json.loads(message)
                except json.JSONDecodeError:
                    print("Error decoding JSON message:", message)
                    continue

                print("Message received:", data)

                if data.get("type") == "streamerList":
                    ids = data.get("ids", [])
                    streamer_id = ids[0] if ids else None
                    if streamer_id:
                        print(f"Streamer '{streamer_id}' found. Subscribing...")
                        await ws.send(json.dumps({"type": "subscribe", "streamerId": streamer_id}))

                elif data.get("type") == "offer":
                    print("Offer received. Creating answer...")
                    offer = RTCSessionDescription(sdp=data["sdp"], type=data["type"])
                    await pc.setRemoteDescription(offer)
                    answer = await pc.createAnswer()
                    await pc.setLocalDescription(answer)
                    await ws.send(json.dumps({"type": pc.localDescription.type, "sdp": pc.localDescription.sdp}))
                    print("Answer sent.")

                elif data.get("type") == "candidate":
                    print("❄ ICE candidate received.")
                    candidate = data.get("candidate")
                    if candidate:
                        await pc.addIceCandidate(candidate)

            await pc.close()
    except Exception as e:
        print(f"Failed to connect to the signaling server: {e}")
        return  # Exit WebRTC thread if the connection fails