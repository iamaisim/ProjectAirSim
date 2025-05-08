from argparse import ArgumentParser
import time
import threading
import numpy as np
import cv2
import tempfile
import os

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log, unpack_image
from projectairsim.types import ImageType


cameraTypeMap = {
    "depth": ImageType.DEPTH_PLANAR, #no depth vis image type
    "segmentation": ImageType.SEGMENTATION,
    "seg": ImageType.SEGMENTATION,
    "scene": ImageType.SCENE,
    # uncomment once these image types are implemented
    #"disparity": ImageType.DISPARITY_NORMALIZED,
    #"normals": ImageType.SURFACE_NORMALS
}

CAM_NAME = "front_center"
DEBUG = False

def saveImage(response, filename):
    img = unpack_image(response)
    cv2.imwrite(os.path.normpath(filename + '.png'), img)

class ImageBenchmarker():
    def __init__(self,
            img_benchmark_type = 'simGetImages',
            viz_image_cv2 = False,
            save_images = False,
            img_type = "scene"):
        self.client = ProjectAirSimClient()
        self.client.connect()

        self.world = World(self.client, "scene_computer_vision.jsonc", delay_after_load_sec=2)
        self.drone = Drone(self.client, self.world, "Drone1")
        self.image_benchmark_num_images = 0
        self.image_benchmark_total_time = 0.0
        self.avg_fps = 0.0
        self.image_callback_thread = None
        self.viz_image_cv2 = viz_image_cv2
        self.save_images = save_images

        self.img_type = cameraTypeMap[img_type]

        if img_benchmark_type == "simGetImage":
            self.image_callback_thread = threading.Thread(target=self.repeat_timer_img, args=(self.image_callback_benchmark_simGetImage, 0.001))
        if img_benchmark_type == "simGetImages":
            self.image_callback_thread = threading.Thread(target=self.repeat_timer_img, args=(self.image_callback_benchmark_simGetImages, 0.001))
        self.is_image_thread_active = False

        if self.save_images:
            self.tmp_dir = os.path.join(tempfile.gettempdir(), "airsim_img_bm")
            projectairsim_log().info(f"Saving images to {self.tmp_dir}")
            try:
                os.makedirs(self.tmp_dir)
            except OSError:
                if not os.path.isdir(self.tmp_dir):
                    raise

    def start_img_benchmark_thread(self):
        if not self.is_image_thread_active:
            self.is_image_thread_active = True
            self.benchmark_start_time = time.time()
            self.image_callback_thread.start()
            projectairsim_log().info("Started img image_callback thread")

    def stop_img_benchmark_thread(self):
        if self.is_image_thread_active:
            self.is_image_thread_active = False
            self.image_callback_thread.join()
            projectairsim_log().info("Stopped image callback thread.")
            projectairsim_log().info(f"FPS: {self.avg_fps} for {self.image_benchmark_num_images} images")
            self.client.disconnect()

    def repeat_timer_img(self, task, period):
        while self.is_image_thread_active:
            task()
            time.sleep(period)

    def update_benchmark_results(self):
        self.image_benchmark_total_time = time.time() - self.benchmark_start_time
        self.avg_fps = self.image_benchmark_num_images / self.image_benchmark_total_time
        if self.image_benchmark_num_images % 10 == 0:
            projectairsim_log().info(f"Result: {self.avg_fps} avg_fps for {self.image_benchmark_num_images} images")

    def image_callback_benchmark_simGetImage(self):
        self.image_benchmark_num_images += 1

        responses = self.drone.get_images(CAM_NAME, [self.img_type])
        image = responses[self.img_type]
        np_arr = np.frombuffer(image, dtype=np.uint8)
        # Change the below dimensions appropriately for the camera settings
        img_rgb = np_arr.reshape(240, 512, 4)

        self.update_benchmark_results()

        if self.viz_image_cv2:
            cv2.imshow("img_rgb", img_rgb)
            cv2.waitKey(1)

    def image_callback_benchmark_simGetImages(self):
        self.image_benchmark_num_images += 1
        responses = self.drone.get_images(CAM_NAME, [self.img_type])
        response = responses[self.img_type]

        self.update_benchmark_results()

        if DEBUG:
            projectairsim_log().info("Type %s, size %d,"
                "height %d width %d", response["encoding"], len(response["data"]), response["height"], response["width"])

        if self.viz_image_cv2:
            img = unpack_image(response)
            cv2.imshow("img", img)
            cv2.waitKey(1)

        if self.save_images:
            filename = os.path.join(self.tmp_dir, str(self.image_benchmark_num_images))
            saveImage(response, filename)


def main(args):
    image_benchmarker = ImageBenchmarker(img_benchmark_type=args.img_benchmark_type, viz_image_cv2=args.viz_image_cv2,
                                      save_images=args.save_images, img_type=args.img_type)

    image_benchmarker.start_img_benchmark_thread()
    time.sleep(args.time)
    image_benchmarker.stop_img_benchmark_thread()

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument('--img_benchmark_type', type=str, choices=["simGetImage", "simGetImages"], default="simGetImages")
    parser.add_argument('--enable_viz_image_cv2', dest='viz_image_cv2', action='store_true', default=False)
    parser.add_argument('--save_images', dest='save_images', action='store_true', default=False)
    parser.add_argument('--img_type', type=str, choices=cameraTypeMap.keys(), default="scene")
    parser.add_argument('--time', help="Time in secs to run the benchmark for", type=int, default=30)

    args = parser.parse_args()
    main(args)
