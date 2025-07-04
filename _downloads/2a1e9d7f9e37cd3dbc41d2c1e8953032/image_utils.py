import queue
from threading import Thread
from typing import Dict, List


# Tkinter is available with python 3.x+; Setting "TkAgg" backend below takes precedence
# over other methods of choosing the backend by the user (using a . matplotlibrc file or env var).
# While not a good idea to force the use of TkAgg as the backend, this avoids reported
# crashes with GTK backend which is system default on some linux distros.
# see docs/sensors/display for more info on the MPL backend.

from projectairsim.utils import projectairsim_log, unpack_image
from projectairsim.types import Color


class ImageDisplay:
    def __init__(
        self,
        num_subwin=3,
        screen_res_x=1920,
        screen_res_y=1080,
        subwin_width=400,
        subwin_height=225,
        subwin_y_pct=0.8,
        with_bounding_box=False,
    ):
        """Display images in windows in specific positions. This runs its image
        display update loop in a separate thread independent of the client's async.io main
        loop since it's independent of the robot controls and for visualization only.

            Args:
                num_subwin (int, optional): Number of sub-windows. Defaults to 3.
                screen_res_x (int, optional): Screen window width (x-axis) in pixels. Defaults to 1920.
                screen_res_y (int, optional): Screen window height (y-axis) in pixels. Defaults to 1080.
                subwin_width (int, optional): Sub-window width in pixels. Defaults to 400.
                subwin_height (int, optional): Sub-window height in pixels. Defaults to 225.
                subwin_y_pct (float, optional): Sub-window y percentage. Defaults to 0.8.
                with_bounding_box (bool, optional): Toggle bounding box display when available
                 (see `receive(...)` method). Defaults to False.
        """
        import cv2
        self.cv2 = cv2
        self.running = False
        self.runner = None

        self.win_positions = []
        gap = (screen_res_x - num_subwin * subwin_width) / (num_subwin + 1)
        for i in range(num_subwin):
            self.win_positions.append(
                {
                    "x": int((i + 1) * gap + i * subwin_width),
                    "y": int(screen_res_y * subwin_y_pct - subwin_height / 2),
                }
            )

        self.chase_window = None
        self.named_windows = {}
        self.image_data = {}
        self.buffer_size = 5
        self.ave_loop_sec = 0.0
        self.display_bbox = with_bounding_box

    def get_subwin_info(self, subwin_idx):
        if subwin_idx < 0 or subwin_idx > len(self.win_positions) - 1:
            projectairsim_log().error("Specified subwindow index does not exist.")
            return {}
        return self.win_positions[subwin_idx]

    def start(self):
        if not self.runner:
            self.running = True

            # OpenCV doesn't always pop-up the windows properly when running with
            # multi-processing, so just use multi-threading
            self.runner = Thread(target=self.display_loop)
            self.runner.start()

    def stop(self):
        self.running = False
        if self.runner and self.runner.is_alive():
            self.runner.join()
            self.runner = None

    def add_image(self, image_name, subwin_idx=None, resize_x=None, resize_y=None):
        x = None if subwin_idx is None else self.win_positions[subwin_idx]["x"]
        y = None if subwin_idx is None else self.win_positions[subwin_idx]["y"]
        self.named_windows[image_name] = {
            "x": x,
            "y": y,
            "resize_x": resize_x,
            "resize_y": resize_y,
            "created": False,
        }
        self.image_data[image_name] = queue.SimpleQueue()

    def add_chase_cam(self, chase_name="ChaseCam", resize_x=1920, resize_y=1080):
        self.named_windows[chase_name] = {
            "x": 0,
            "y": 0,
            "resize_x": resize_x,
            "resize_y": resize_y,
            "created": False,
        }
        self.image_data[chase_name] = queue.SimpleQueue()

    def flatten_bbox2D(self, annotation):
        if annotation.get("bbox2d"):
            bb2D = annotation["bbox2d"]
            return [
                bb2D["center"]["x"],
                bb2D["center"]["y"],
                bb2D["size"]["x"],
                bb2D["size"]["y"],
            ]
        return None

    def receive(self, image, image_name):
        """Callback for receiving new image data. Keep this processing as short as
        possible to avoid blocking the callback for receiving data from NNG.

        Args:
            image (np.ndarray): Image data
            image_name (str): Name of image
        """

        if (
            self.running
            and image is not None
            and "data" in image
            and len(image["data"]) > 0
        ):
            image_q = self.image_data[image_name]
            self.dump_excess_data(image_q)
            image_q.put(image)
        elif (
            self.running
            and image is not None
            and "data_float" in image
            and len(image["data_float"]) > 0
        ):
            image_q = self.image_data[image_name]
            self.dump_excess_data(image_q)
            image_q.put(image)

    def destroy_windows(self):
        for image_name, win_info in list(self.named_windows.items()):
            if win_info["created"]:
                self.cv2.destroyWindow(image_name)
                win_info["created"] = False

    def dump_excess_data(self, image_q):
        while not image_q.empty() and image_q.qsize() > self.buffer_size:
            image_q.get()

    def display_loop(self):
        while self.running:
            for image_name, win_info in list(self.named_windows.items()):
                image_q = self.image_data[image_name]
                if not image_q.empty():
                    image = image_q.get()

                    # Create window if not already visible
                    if not win_info["created"]:
                        self.cv2.namedWindow(
                            image_name,
                            flags=self.cv2.WINDOW_GUI_NORMAL + self.cv2.WINDOW_AUTOSIZE,
                        )
                        if win_info["x"] is not None and win_info["y"] is not None:
                            self.cv2.moveWindow(image_name, win_info["x"], win_info["y"])
                        win_info["created"] = True

                    self.display_image(
                        image,
                        image_name,
                        win_info["resize_x"],
                        win_info["resize_y"],
                    )

            key = self.cv2.waitKey(1)  # expensive, can take minimum 5~15 ms
            if key == 27:  # Esc key
                self.running = False

        self.destroy_windows()

    def display_image(
        self,
        image_msg,
        win_name: str = "Image",
        resize_x: int = None,
        resize_y: int = None,
    ):
        """Display the image using OpenCV HighGUI"""
        if image_msg is None:
            return
        img_np = unpack_image(image_msg)

        if self.display_bbox:
            for annotation in image_msg["annotations"]:
                bbox_center = annotation["bbox2d"]["center"]
                bbox_size = annotation["bbox2d"]["size"]
                v1 = (
                    int(bbox_center["x"] - (bbox_size["x"] / 2.0)),
                    int(bbox_center["y"] - (bbox_size["y"] / 2.0)),
                )
                v2 = (
                    int(bbox_center["x"] + (bbox_size["x"] / 2.0)),
                    int(bbox_center["y"] + (bbox_size["y"] / 2.0)),
                )
                self.cv2.rectangle(img_np, v1, v2, (0, 255, 0), 3)
                # draw_bbox3D(img_np, annotation, thickness=1)

        # Resize image if requested
        if resize_x is not None and resize_y is not None:
            img_np = self.cv2.resize(
                img_np, (resize_x, resize_y), interpolation=self.cv2.INTER_LINEAR
            )

        # Display image
        self.cv2.imshow(win_name, img_np)


def draw_bbox3D(img_np, annotation, color=(0, 0, 255), thickness=3):
    """Draws 3D bounding boxes on top of a given image.
    The order of the bbox3d_in_image_space should be:
    (-x,-y,-z), (-x,-y,z), (-x,y,-z), (-x,y,z),
    (x,-y,-z), (x,-y,z), (x,y,-z), (x,y,z)"""
    import cv2
    if "bbox3d_in_image_space" not in annotation:
        projectairsim_log().warning(
            "No bbox3d_in_image_space in annotations, not drawing 3D bbox."
        )
        return

    try:
        points = annotation["bbox3d_in_image_space"]
        for i in range(0, 8, 2):  # draw top down lines
            a = (int(points[i]["x"]), int(points[i]["y"]))
            b = (int(points[i + 1]["x"]), int(points[i + 1]["y"]))
            cv2.line(img_np, a, b, color, thickness)

        a = (int(points[0]["x"]), int(points[0]["y"]))
        b = (int(points[2]["x"]), int(points[2]["y"]))
        cv2.line(img_np, a, b, color, thickness)
        a = (int(points[1]["x"]), int(points[1]["y"]))
        b = (int(points[3]["x"]), int(points[3]["y"]))
        cv2.line(img_np, a, b, color, thickness)
        a = (int(points[5]["x"]), int(points[5]["y"]))
        b = (int(points[7]["x"]), int(points[7]["y"]))
        cv2.line(img_np, a, b, color, thickness)
        a = (int(points[4]["x"]), int(points[4]["y"]))
        b = (int(points[6]["x"]), int(points[6]["y"]))
        cv2.line(img_np, a, b, color, thickness)

        a = (int(points[2]["x"]), int(points[2]["y"]))
        b = (int(points[6]["x"]), int(points[6]["y"]))
        cv2.line(img_np, a, b, color, thickness)
        a = (int(points[3]["x"]), int(points[3]["y"]))
        b = (int(points[7]["x"]), int(points[7]["y"]))
        cv2.line(img_np, a, b, color, thickness)
        a = (int(points[0]["x"]), int(points[0]["y"]))
        b = (int(points[4]["x"]), int(points[4]["y"]))
        cv2.line(img_np, a, b, color, thickness)
        a = (int(points[1]["x"]), int(points[1]["y"]))
        b = (int(points[5]["x"]), int(points[5]["y"]))
        cv2.line(img_np, a, b, color, thickness)
    except OverflowError as of:
        projectairsim_log().info("Overflow error. Ignoring...")


class SensorDataDisplay(object):
    def __init__(
        self,
        fig_size=(20, 10),
        title="sensor_name",
        xlabel="time",
        ylabel="data",
        wait_time=0.02,
    ):
        """2D plot/display for sensor data

        Args:
            # num_plots (int, optional): Number of plots. Defaults to 2.
        """
        import matplotlib
        matplotlib.use("TkAgg")
        import matplotlib.pyplot as plt
        import numpy as np
        self.plt = plt
        self.np = np

        self.plot_obj = None
        self.fig_size = fig_size
        self.title = title
        self.xlabel = xlabel
        self.ylabel = ylabel
        self.x_vec = []
        self.y_vec = []
        self.wait_time = wait_time

    def plot(self, x, y):
        self.x_vec.append(x)  # Use a cir buf
        self.y_vec.append(y)  # Use a cir buf
        if self.plot_obj is None:
            self.plt.ion()
            fig = self.plt.figure(figsize=self.fig_size)
            ax = fig.add_subplot(111)
            (self.plot_obj,) = ax.plot(x, y, "g-", alpha=0.9)
            self.plt.xlabel(self.xlabel)
            self.plt.ylabel(self.ylabel)
            self.plt.title(self.title)
            self.plt.show()
        self.plot_obj.set_data(self.x_vec, self.y_vec)
        self.plt.xlim(self.np.min(self.x_vec), self.np.max(self.x_vec))
        self.plt.pause(self.wait_time)


SEGMENTATION_PALLETE = [
    [55, 181, 57],
    [153, 108, 6],
    [112, 105, 191],
    [89, 121, 72],
    [190, 225, 64],
    [206, 190, 59],
    [81, 13, 36],
    [115, 176, 195],
    [161, 171, 27],
    [135, 169, 180],
    [29, 26, 199],
    [102, 16, 239],
    [242, 107, 146],
    [156, 198, 23],
    [49, 89, 160],
    [68, 218, 116],
    [11, 236, 9],
    [196, 30, 8],
    [121, 67, 28],
    [0, 53, 65],
    [146, 52, 70],
    [226, 149, 143],
    [151, 126, 171],
    [194, 39, 7],
    [205, 120, 161],
    [212, 51, 60],
    [211, 80, 208],
    [189, 135, 188],
    [54, 72, 205],
    [103, 252, 157],
    [124, 21, 123],
    [19, 132, 69],
    [195, 237, 132],
    [94, 253, 175],
    [182, 251, 87],
    [90, 162, 242],
    [199, 29, 1],
    [254, 12, 229],
    [35, 196, 244],
    [220, 163, 49],
    [86, 254, 214],
    [152, 3, 129],
    [92, 31, 106],
    [207, 229, 90],
    [125, 75, 48],
    [98, 55, 74],
    [126, 129, 238],
    [222, 153, 109],
    [85, 152, 34],
    [173, 69, 31],
    [37, 128, 125],
    [58, 19, 33],
    [134, 57, 119],
    [218, 124, 115],
    [120, 0, 200],
    [225, 131, 92],
    [246, 90, 16],
    [51, 155, 241],
    [202, 97, 155],
    [184, 145, 182],
    [96, 232, 44],
    [133, 244, 133],
    [180, 191, 29],
    [1, 222, 192],
    [99, 242, 104],
    [91, 168, 219],
    [65, 54, 217],
    [148, 66, 130],
    [203, 102, 204],
    [216, 78, 75],
    [234, 20, 250],
    [109, 206, 24],
    [164, 194, 17],
    [157, 23, 236],
    [158, 114, 88],
    [245, 22, 110],
    [67, 17, 35],
    [181, 213, 93],
    [170, 179, 42],
    [52, 187, 148],
    [247, 200, 111],
    [25, 62, 174],
    [100, 25, 240],
    [191, 195, 144],
    [252, 36, 67],
    [241, 77, 149],
    [237, 33, 141],
    [119, 230, 85],
    [28, 34, 108],
    [78, 98, 254],
    [114, 161, 30],
    [75, 50, 243],
    [66, 226, 253],
    [46, 104, 76],
    [8, 234, 216],
    [15, 241, 102],
    [93, 14, 71],
    [192, 255, 193],
    [253, 41, 164],
    [24, 175, 120],
    [185, 243, 231],
    [169, 233, 97],
    [243, 215, 145],
    [72, 137, 21],
    [160, 113, 101],
    [214, 92, 13],
    [167, 140, 147],
    [101, 109, 181],
    [53, 118, 126],
    [3, 177, 32],
    [40, 63, 99],
    [186, 139, 153],
    [88, 207, 100],
    [71, 146, 227],
    [236, 38, 187],
    [215, 4, 215],
    [18, 211, 66],
    [113, 49, 134],
    [47, 42, 63],
    [219, 103, 127],
    [57, 240, 137],
    [227, 133, 211],
    [145, 71, 201],
    [217, 173, 183],
    [250, 40, 113],
    [208, 125, 68],
    [224, 186, 249],
    [69, 148, 46],
    [239, 85, 20],
    [108, 116, 224],
    [56, 214, 26],
    [179, 147, 43],
    [48, 188, 172],
    [221, 83, 47],
    [155, 166, 218],
    [62, 217, 189],
    [198, 180, 122],
    [201, 144, 169],
    [132, 2, 14],
    [128, 189, 114],
    [163, 227, 112],
    [45, 157, 177],
    [64, 86, 142],
    [118, 193, 163],
    [14, 32, 79],
    [200, 45, 170],
    [74, 81, 2],
    [59, 37, 212],
    [73, 35, 225],
    [95, 224, 39],
    [84, 170, 220],
    [159, 58, 173],
    [17, 91, 237],
    [31, 95, 84],
    [34, 201, 248],
    [63, 73, 209],
    [129, 235, 107],
    [231, 115, 40],
    [36, 74, 95],
    [238, 228, 154],
    [61, 212, 54],
    [13, 94, 165],
    [141, 174, 0],
    [140, 167, 255],
    [117, 93, 91],
    [183, 10, 186],
    [165, 28, 61],
    [144, 238, 194],
    [12, 158, 41],
    [76, 110, 234],
    [150, 9, 121],
    [142, 1, 246],
    [230, 136, 198],
    [5, 60, 233],
    [232, 250, 80],
    [143, 112, 56],
    [187, 70, 156],
    [2, 185, 62],
    [138, 223, 226],
    [122, 183, 222],
    [166, 245, 3],
    [175, 6, 140],
    [240, 59, 210],
    [248, 44, 10],
    [83, 82, 52],
    [223, 248, 167],
    [87, 15, 150],
    [111, 178, 117],
    [197, 84, 22],
    [235, 208, 124],
    [9, 76, 45],
    [176, 24, 50],
    [154, 159, 251],
    [149, 111, 207],
    [168, 231, 15],
    [209, 247, 202],
    [80, 205, 152],
    [178, 221, 213],
    [27, 8, 38],
    [244, 117, 51],
    [107, 68, 190],
    [23, 199, 139],
    [171, 88, 168],
    [136, 202, 58],
    [6, 46, 86],
    [105, 127, 176],
    [174, 249, 197],
    [172, 172, 138],
    [228, 142, 81],
    [7, 204, 185],
    [22, 61, 247],
    [233, 100, 78],
    [127, 65, 105],
    [33, 87, 158],
    [139, 156, 252],
    [42, 7, 136],
    [20, 99, 179],
    [79, 150, 223],
    [131, 182, 184],
    [110, 123, 37],
    [60, 138, 96],
    [210, 96, 94],
    [123, 48, 18],
    [137, 197, 162],
    [188, 18, 5],
    [39, 219, 151],
    [204, 143, 135],
    [249, 79, 73],
    [77, 64, 178],
    [41, 246, 77],
    [16, 154, 4],
    [116, 134, 19],
    [4, 122, 235],
    [177, 106, 230],
    [21, 119, 12],
    [104, 5, 98],
    [50, 130, 53],
    [30, 192, 25],
    [26, 165, 166],
    [10, 160, 82],
    [106, 43, 131],
    [44, 216, 103],
    [255, 101, 221],
    [32, 151, 196],
    [213, 220, 89],
    [70, 209, 228],
    [97, 184, 83],
    [82, 239, 232],
    [251, 164, 128],
    [193, 11, 245],
    [38, 27, 159],
    [229, 141, 203],
    [130, 56, 55],
    [147, 210, 11],
    [162, 203, 118],
    [43, 47, 206],
]

COLOR_TO_ID_MAP = {
    Color(SEGMENTATION_PALLETE[i]): i for i in range(0, len(SEGMENTATION_PALLETE))
}


def segmentation_id_to_color(id: int) -> List:
    return Color(SEGMENTATION_PALLETE[id])


def segmentation_color_to_id(color: str) -> Color:
    return COLOR_TO_ID_MAP[color]