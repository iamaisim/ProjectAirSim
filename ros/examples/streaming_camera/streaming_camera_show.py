import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
import cv2

class ImageSubscriber(Node):
    """
    A ROS2 node that subscribes to the "/airsim_streaming_camera/image_raw" topic,
    converts the received ROS2 Image messages into OpenCV format, and displays the video stream.
    """

    def __init__(self):
        super().__init__('image_subscriber')

        # Create a subscriber for the "/airsim_streaming_camera/image_raw" topic
        self.subscription = self.create_subscription(
            Image,
            '/airsim_streaming_camera/image_raw',
            self.image_callback,
            10)
        self.subscription  # Prevent unused variable warning

        # Initialize the bridge between ROS2 and OpenCV
        self.br = CvBridge()

        self.get_logger().info("Image Subscriber Node initialized. Listening to images on /airsim_streaming_camera/image_raw")

    def image_callback(self, msg):
        """
        Callback function to process the received image message.
        Converts ROS2 Image message to OpenCV format and displays the image.
        """
        # Convert ROS2 Image message to OpenCV format (RGB)
        img_rgb = self.br.imgmsg_to_cv2(msg, desired_encoding="rgb8")

        # Convert RGB to BGR for correct OpenCV display
        img_bgr = cv2.cvtColor(img_rgb, cv2.COLOR_RGB2BGR)

        # Display the image in an OpenCV window
        cv2.imshow("WebRTC Stream", img_bgr)
        cv2.waitKey(1)  # Necessary to update the OpenCV window

def main():
    """
    Main function to initialize the ROS2 node and start the subscriber.
    """
    rclpy.init()
    node = ImageSubscriber()
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()
        cv2.destroyAllWindows()  # Close OpenCV windows

if __name__ == '__main__':
    main()
