import logging
import queue
from threading import Thread
from concurrent.futures import ThreadPoolExecutor
from functools import partial
from typing import List
import cv2
import numpy as np
import requests
import asyncio
import json


logger = logging.getLogger()
handler = logging.StreamHandler()
formatter = logging.Formatter("[%(asctime)s] [%(name)s]  [%(levelname)s]  %(message)s")
handler.setFormatter(formatter)
logger.addHandler(handler)
logger.setLevel(logging.INFO)


class BytesEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, bytes):
            return obj.decode("utf-8")
        return json.JSONEncoder.default(self, obj)


class PredictionsDisplayAsync:
    def __init__(
        self,
        model_endpoint=None,
        bb_normalized=True,
        bb_3D=False,
        num_subwin=1,
        screen_res_x=1920,
        screen_res_y=1080,
        subwin_width=400,
        subwin_height=225,
        subwin_y_pct=0.8,
    ):
        """Display predictions from autonomy models on images in windows at specific positions. The
        autonomy blocks server is assumed to be running and serving predictions at the `model_endpoint`.
        The image display update loop is run in a separate thread independent of the client's async.io main.

            Args:
                model_endpoint (str) : URL of the autonomy model server
                bb_normalized (bool) : Whether the bounding box coordinates are normalized or not
                bb_3D (bool) : Whether the bounding box is 3D or 2D
                num_subwin (int, optional): Number of sub-windows. Defaults to 3.
                screen_res_x (int, optional): Screen window width (x-axis) in pixels. Defaults to 1920.
                screen_res_y (int, optional): Screen window height (y-axis) in pixels. Defaults to 1080.
                subwin_width (int, optional): Sub-window width in pixels. Defaults to 400.
                subwin_height (int, optional): Sub-window height in pixels. Defaults to 225.
                subwin_y_pct (float, optional): Sub-window y percentage. Defaults to 0.8.
        """

        self.running = False
        # OpenCV doesn't always pop-up the windows properly when running with
        # multi-processing, so just use multi-threading
        self.runner = None
        self.prediction_runner = None
        self.executor = ThreadPoolExecutor(10)

        self.win_positions = []
        gap = (screen_res_x - num_subwin * subwin_width) / (num_subwin + 1)
        for i in range(num_subwin):
            self.win_positions.append(
                {
                    "x": int((i + 1) * gap + i * subwin_width),
                    "y": int(screen_res_y * subwin_y_pct - subwin_height / 2),
                }
            )

        self.named_windows = {}
        self.image_data = {}
        self.prediction_data = {}
        self.buffer_size = 5
        self.prediction_buffer = 3
        self.num_channels_img = 3
        self.display_bb = True
        self.bb_3D: bool = bb_3D
        self.bb_coords = np.zeros(16) if self.bb_3D else np.zeros(4)
        self.bb_normalized: bool = bb_normalized

        self.model_endpoint = model_endpoint

    def start(self):
        if not self.runner:
            self.running = True
            # OpenCV doesn't always pop-up the windows properly when running with
            # multi-processing, so just use multi-threading
            self.runner = Thread(target=self.display_loop)
            self.runner.start()
        if not self.prediction_runner:
            self.prediction_runner = Thread(
                target=asyncio.run, args=(self.prediction_loop(),)
            )
            self.prediction_runner.start()

    def stop(self):
        self.running = False
        if self.prediction_runner and self.prediction_runner.is_alive():
            self.prediction_runner.join()
            self.prediction_runner = None

        if self.runner and self.runner.is_alive():
            self.runner.join()
            self.runner = None

    def receive(self, image, image_name):
        """Callback for receiving new image data. Keep this processing as short as
        possible to avoid blocking the callback for receiving data from NNG.
        """
        if image is not None and "data" in image and len(image["data"]) > 0:
            self.image_data[image_name].put(image)

    def add_image(self, image_name, subwin_idx=None):
        x = None if subwin_idx is None else self.win_positions[subwin_idx]["x"]
        y = None if subwin_idx is None else self.win_positions[subwin_idx]["y"]
        self.named_windows[image_name] = {
            "x": x,
            "y": y,
            "resize_x": None,
            "resize_y": None,
            "created": False,
        }
        self.image_data[image_name] = queue.SimpleQueue()
        self.prediction_data[image_name] = queue.SimpleQueue()

    def transform_image_data(self, image: dict):
        image["data"] = np.frombuffer(image["data"], dtype="uint8").tolist()

        return image

    async def get_predictions(self, image_packed: dict, image_name: str):
        loop = asyncio.get_event_loop()
        image_packed = self.transform_image_data(image_packed)
        output = None
        w = image_packed["width"]
        h = image_packed["height"]

        if self.display_bb:
            response = await loop.run_in_executor(
                self.executor,
                partial(
                    requests.post,
                    self.model_endpoint,
                    files={"input_image": json.dumps(image_packed, cls=BytesEncoder)},
                ),
            )
            output = response.json()["prediction"]

            # Scale outputs;
            if self.bb_normalized:
                if self.bb_3D:
                    output = list(np.multiply(output, 8 * [w, h]).astype(int))
                else:
                    output = list(np.multiply(output, [w, h, w, h]))

        self.prediction_data[image_name].put(
            {"image": image_packed, "prediction": output}
        )

    def get_predicted_bb(self):
        return self.bb_coords

    def destroy_windows(self):
        for image_name, win_info in list(self.named_windows.items()):
            cv2.destroyWindow(image_name)
            win_info["created"] = False

    def dump_excess_data(self, image_q: queue.SimpleQueue):
        while not image_q.empty() and image_q.qsize() > self.buffer_size:
            image_q.get()

    async def prediction_loop(self):
        while self.running:
            for image_name, win_info in list(self.named_windows.items()):
                image_q: queue.SimpleQueue = self.image_data[image_name]
                if not image_q.empty():
                    self.dump_excess_data(image_q)
                    image = image_q.get()
                    await self.get_predictions(image, image_name)
            await asyncio.sleep(0.001)  # don't clog up other async tasks

    def display_loop(self):
        while self.running:
            for image_name, win_info in list(self.named_windows.items()):
                prediction_q: queue.SimpleQueue = self.prediction_data[image_name]
                if (not prediction_q.empty()) and prediction_q.qsize() > 3:
                    pred_dict = prediction_q.get()
                    image = pred_dict["image"]
                    prediction = pred_dict["prediction"]

                    # Create window if not already visible
                    if not win_info["created"]:
                        cv2.namedWindow(
                            image_name,
                            flags=cv2.WINDOW_GUI_NORMAL + cv2.WINDOW_AUTOSIZE,
                        )
                        if win_info["x"] is not None and win_info["y"] is not None:
                            cv2.moveWindow(image_name, win_info["x"], win_info["y"])
                        win_info["created"] = True

                    self.display_image_with_predictions(
                        image,
                        prediction,
                        image_name,
                        win_info["resize_x"],
                        win_info["resize_y"],
                    )

            cv2.waitKey(1)  # Allow OpenCV to handle window events

        self.destroy_windows()

    def display_image_with_predictions(
        self,
        image: dict,
        prediction,
        win_name: str,
        resize_x: int = -1,
        resize_y: int = -1,
    ):
        """Display the image using OpenCV HighGUI"""

        if image is None:
            return

        # Unpack image data
        img_cv = np.array(image.get("data"), dtype=np.uint8).reshape(
            ((image.get("height"), image.get("width"), self.num_channels_img))
        )  # Format into np array of correct size

        if self.display_bb and (prediction is not None):
            self.bb_coords = prediction
            if self.bb_3D:
                self.draw_bbox3D(img_cv, self.bb_coords)
            else:
                self.draw_bbox2D(img_cv, self.bb_coords)

        # Resize image if requested
        if resize_x is not None and resize_y is not None:
            img_cv = cv2.resize(
                img_cv, (resize_x, resize_y), interpolation=cv2.INTER_LINEAR
            )

        # Display image
        cv2.imshow(win_name, img_cv)

    @staticmethod
    def draw_bbox3D(
        img_np, prediction: List[int], color=(0, 0, 255), thickness=3, w=400, h=225
    ):
        """Draws 3D bounding boxes on top of a given image. The `prediction`
        is expected to be a List[int]=[x0, y0, x1, y1, ... x7,y7] representing
        the pixel coordinates corresponding to the 3D points (in order):
        (-x,-y,-z), (-x,-y,z), (-x,y,-z), (-x,y,z),
        (x,-y,-z), (x,-y,z), (x,y,-z), (x,y,z).
        The indices maps to the 3DBB vertices in the following way:
        3DBB front face vertices (clockwise from top left):[0, 2, 3, 1]
        3DBB back face vertices (clockwise from top left): [4, 6, 7, 5]

        Args:
            img_np (np.array): w x h x 3 RGB image
            prediction (List[int]): BBox3D: [x0, y0, x1, y1, ... x7,y7]
            color (tuple, optional): BBox color. Defaults to (0, 0, 255).
            thickness (int, optional): BBox thickness. Defaults to 3.
            w (int, optional): Image Width. Defaults to 400.
            h (int, optional): Image Height. Defaults to 225.
        """

        if len(prediction) == 0 or len(prediction) != 16:
            # logger.warning("Invalid Predictions")
            print(f"Invalid prediction")
            return

        try:
            coordinates = np.array_split(prediction, 8)
            # print(f"drawing:{coordinates}", end="\r")
            # 3DBB front face vertices (clockwise from top left):[0, 2, 3, 1]
            # 3DBB back face vertices (clockwise from top left): [4, 6, 7, 5]

            # Join vertices front to back
            for v in [0, 1, 2]:  # , 3]:
                front = (coordinates[v][0], coordinates[v][1])
                back = (coordinates[v + 4][0], coordinates[v + 4][1])
                cv2.line(img_np, front, back, (0, 0, 255), thickness)

            # Join vertices top to bottom
            for v in [0, 2, 4]:  # , 6]:  # draw top down lines
                top = (coordinates[v][0], coordinates[v][1])
                bottom = (coordinates[v + 1][0], coordinates[v + 1][1])
                cv2.line(img_np, top, bottom, (0, 255, 0), thickness)

            # # Join vertices left to right
            # for v in [0, 1, 4, 5]:  # Left vertices
            #    left = (coordinates[v][0], coordinates[v][1])
            #    right = (coordinates[v + 2][0], coordinates[v + 2][0])  # Right vertices
            #    cv2.line(img_np, left, right, (255, 0, 0), thickness)

        except OverflowError as of:
            # logger.info("Overflow error. Ignoring...")
            print(f"Overflow Error occured. Skipping.{of}")

    @staticmethod
    def draw_bbox2D(
        img_np, prediction: List[int], color=(0, 0, 255), thickness=3, w=400, h=225
    ):
        """Draw 2D bounding box on image

        Args:
            img_np (np.array): w x h x 3 RGB image
            prediction (List[int]): BBox2D: [x_center, y_center, width, height]
            color (tuple, optional): BBox color. Defaults to (0, 0, 255).
            thickness (int, optional): BBox thickness. Defaults to 3.
            w (int, optional): Width. Defaults to 400.
            h (int, optional): Height. Defaults to 225.
        """
        x_c, y_c, w, h = list(prediction)
        cv2.rectangle(
            img_np,
            (int(x_c - w / 2), int(y_c - h / 2)),
            (int(x_c + w / 2), int(y_c + h / 2)),
            color=color,
            thickness=thickness,
        )
