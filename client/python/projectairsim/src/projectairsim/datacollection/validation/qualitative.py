"""
Copyright (C) Microsoft Corporation. All rights reserved.

Bounding Box data validator. Summarizes dataset into a video to
enable visual validation of images and bounding box annotations

Takes in datacollector_config.jsonc and creates video from data on 
the specified blob container

"""

import argparse
import os
from typing import Dict, List
import cv2

import projectairsim.datacollection.utils as utils

parser = argparse.ArgumentParser("Qualitative Analysis")

parser.add_argument(
    "--config-dir",
    help="Path to config directory",
    default="./src/projectairsim/datacollection/configs",
)

parser.add_argument(
    "--source",
    help="Source of data [azure or local]",
    default="azure",
)

parser.add_argument(
    "--connection-string",
    help="Connection string for storage account",
    type=str,
)

parser.add_argument(
    "--num-images",
    help="Number of images to make video from (0 == all)",
    default=100,
)

parser.add_argument("--save-path", help="Path to dataset dir/save path", default="./")

parser.add_argument(
    "-pd",
    "--pose-dim",
    default=16,
    type=int,
    metavar="P",
    help="pose dimension (default: 16 | for 3D BBox: 16)",
)


class VideoGenerator:
    """Generates videos from data on azure/local dir"""

    def __init__(
        self,
        config_dir: str,
        source: str = "local",
        num_images: int = 0,
        save_path: str = "./",
        pose_dim: int = 16,
        connection_string: str = "",
    ) -> None:
        self.config_dir = config_dir
        self.num_images = int(num_images)
        self.save_path = save_path
        self.pose_dim = int(pose_dim)
        self.connection_string = connection_string
        self.is_azure = True
        if source == "local":
            self.is_azure = False

        if self.is_azure and self.connection_string == "":
            raise ValueError(
                "Please provide a valid connection string for your storage account"
            )

        self.setup_data()

    def setup_data(self) -> None:

        if self.is_azure:
            (
                bbox_dict,
                video_images,
                blob_list,
                container_client,
            ) = utils.setup_data_azure(
                self.config_dir, self.connection_string, self.num_images
            )
            self.container_client = container_client
        else:
            bbox_dict, video_images, blob_list = utils.setup_data_local(
                self.save_path, self.num_images
            )

        self.bbox_dict = bbox_dict
        self.video_images = video_images
        self.blob_list = blob_list

    def setup_bbox_3d(self, bbox: Dict) -> List[float]:

        try:
            p0 = [int(float(bbox["x0"])), int(float(bbox["y0"]))]
            p1 = [int(float(bbox["x1"])), int(float(bbox["y1"]))]
            p2 = [int(float(bbox["x2"])), int(float(bbox["y2"]))]
            p3 = [int(float(bbox["x3"])), int(float(bbox["y3"]))]
            p4 = [int(float(bbox["x4"])), int(float(bbox["y4"]))]
            p5 = [int(float(bbox["x5"])), int(float(bbox["y5"]))]
            p6 = [int(float(bbox["x6"])), int(float(bbox["y6"]))]
            p7 = [int(float(bbox["x7"])), int(float(bbox["y7"]))]
        except:
            print(bbox)

        bbox_3d = [p0, p1, p2, p3, p4, p5, p6, p7]

        return bbox_3d

    def generate_video(self, color=(0, 0, 233), thickness=2) -> bool:

        height, width, layers = self.video_images[0].shape
        out_path = os.path.join(self.save_path, f"Validation_{self.pose_dim}.avi")
        video = cv2.VideoWriter(
            out_path, cv2.VideoWriter_fourcc(*"DIVX"), 15, (width, height)
        )

        for i, blob_name in enumerate(self.blob_list):
            image_name = os.path.split(blob_name)[1]
            image = self.video_images[i]
            bbox = self.bbox_dict.get(image_name)

            if bbox == None:
                continue

            cv2.putText(
                image, str(image_name), (100, 100), cv2.FONT_HERSHEY_COMPLEX, 1, color
            )

            if self.pose_dim == 4:
                x_c = float(bbox["x_c"])
                y_c = float(bbox["y_c"])
                w = float(bbox["w"])
                h = float(bbox["h"])
                cv2.rectangle(
                    image,
                    (int(x_c - w / 2), int(y_c - h / 2)),
                    (int(x_c + w / 2), int(y_c + h / 2)),
                    color,
                    thickness,
                )
            elif self.pose_dim == 16:

                bbox_3d = self.setup_bbox_3d(bbox)

                for i in range(0, 8, 2):  # draw top down lines
                    a = (int(bbox_3d[i][0]), int(bbox_3d[i][1]))
                    b = (int(bbox_3d[i + 1][0]), int(bbox_3d[i + 1][1]))
                    cv2.line(image, a, b, color, thickness)

                a = (int(bbox_3d[0][0]), int(bbox_3d[0][1]))
                b = (int(bbox_3d[2][0]), int(bbox_3d[2][1]))
                cv2.line(image, a, b, color, thickness)
                a = (int(bbox_3d[1][0]), int(bbox_3d[1][1]))
                b = (int(bbox_3d[3][0]), int(bbox_3d[3][1]))
                cv2.line(image, a, b, color, thickness)
                a = (int(bbox_3d[5][0]), int(bbox_3d[5][1]))
                b = (int(bbox_3d[7][0]), int(bbox_3d[7][1]))
                cv2.line(image, a, b, color, thickness)
                a = (int(bbox_3d[4][0]), int(bbox_3d[4][1]))
                b = (int(bbox_3d[6][0]), int(bbox_3d[6][1]))
                cv2.line(image, a, b, color, thickness)

                a = (int(bbox_3d[2][0]), int(bbox_3d[2][1]))
                b = (int(bbox_3d[6][0]), int(bbox_3d[6][1]))
                cv2.line(image, a, b, color, thickness)
                a = (int(bbox_3d[3][0]), int(bbox_3d[3][1]))
                b = (int(bbox_3d[7][0]), int(bbox_3d[7][1]))
                cv2.line(image, a, b, color, thickness)
                a = (int(bbox_3d[0][0]), int(bbox_3d[0][1]))
                b = (int(bbox_3d[4][0]), int(bbox_3d[4][1]))
                cv2.line(image, a, b, color, thickness)
                a = (int(bbox_3d[1][0]), int(bbox_3d[1][1]))
                b = (int(bbox_3d[5][0]), int(bbox_3d[5][1]))
                cv2.line(image, a, b, color, thickness)
            video.write(image)

        cv2.destroyAllWindows()
        video.release()

        if self.is_azure:
            print(f"Uploading video to container")
            self.container_client.upload_blob(
                name="Validation.avi",
                data=open(out_path, "rb").read(),
                overwrite=True,
            )
            print(f"Video uploaded")

        return True


if __name__ == "__main__":

    args = parser.parse_args()

    config_dir = args.config_dir
    source = args.source
    connection_string = args.connection_string
    num_images = args.num_images
    save_path = args.save_path
    pose_dim = args.pose_dim

    video_generator = VideoGenerator(
        config_dir=config_dir,
        source=source,
        num_images=num_images,
        save_path=save_path,
        pose_dim=pose_dim,
        connection_string=connection_string,
    )

    video_generator.generate_video()
