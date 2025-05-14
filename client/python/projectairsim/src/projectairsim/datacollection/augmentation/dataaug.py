"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Script for augmenting data based on augmentation spec in the config/API.
Reads data from a local dir/azure and adds data to the same
"""

import argparse
import csv
import pathlib
from typing import List

from projectairsim.utils import projectairsim_log
import projectairsim.datacollection.collection.helper as helper
import projectairsim.datacollection.utils as utils
from projectairsim.datacollection.augmentation.position_aug import DataAugmentation
from projectairsim.datacollection.types import Config

parser = argparse.ArgumentParser("Dataset Augmentation")
parser.add_argument(
    "--config-dir",
    help="Path to config directory",
    default="./src/projectairsim/datacollection/configs",
)

parser.add_argument(
    "--compute-location", help="Source of data [Azure or Local]", default="Azure"
)

parser.add_argument(
    "--connection-string", help="Connection string for storage account", type=str
)

parser.add_argument(
    "--num-images", help="Number of images to augment from (0 == all)", default=100
)

parser.add_argument("--save-path", help="Path to dataset dir/save path", default="./")


def augment_data(
    augmentations: List[DataAugmentation],
    save_path,
    connection_string="",
    compute_location="local",
):
    is_Azure = True if compute_location == "aks" else False
    # num_image = Number of images from the dataset to augment
    num_images = 0
    flag = {"aug_flag": 0}
    if is_Azure:
        bbox_dict, raw_images, blob_list, container_client = utils.setup_data_azure(
            config_dir, connection_string, num_images
        )
    else:
        bbox_dict, raw_images, blob_list = utils.setup_data_local(save_path, num_images)

    augmented_imageids, bbox_2d_coords = [], []
    new_bbox_dict = {}
    if blob_list == []:
        projectairsim_log().warning(f"No images in dir to augment")
        return augmented_imageids, bbox_2d_coords

    augmentation_index = max([int(f[: f.index(".")]) for f in blob_list])
    projectairsim_log().info(f"Augmented images starting from {augmentation_index}")

    for i in range(len(raw_images)):
        for augmentation in augmentations:
            augmentation_index += 1
            image = raw_images[i]
            blob_name: str = blob_list[i]
            augmented_blob_name = (
                f"{augmentation_index}.png"  # In format {augmentation_index}.png
            )
            bbox = bbox_dict[blob_name]
            bbox_2d_coords = [bbox["x_c"], bbox["y_c"], bbox["w"], bbox["h"]]
            bbox_input_params = helper.convert_to_pascal_voc_format(bbox_2d_coords)

            augmented_image, augmented_bboxes = augmentation.augment(
                image=image,
                bboxes=[bbox_input_params],
                bbox_params={"format": "pascal_voc", "label_fields": ["labels"]},
            )
            image_dict = {
                "data": augmented_image,
                "height": augmented_image.shape[0],
                "width": augmented_image.shape[1],
            }
            augmented_bboxes_2d = helper.convert_to_airsim_format(augmented_bboxes)
            helper.save_image(image_dict, augmented_blob_name, save_path)
            augmented_imageids.append(
                {"original_image_id": i, "augmented_image_id": augmentation_index}
            )
            bbox_2d_coords.append(augmented_bboxes_2d)

            # Append bbox_dict with augmented image's bbox data
            # all the data is the same as the original image except for the bbox + imagename
            new_bbox_dict[augmented_blob_name] = bbox.copy()
            new_bbox_dict[augmented_blob_name]["ImageName"] = augmented_blob_name
            new_bbox_dict[augmented_blob_name]["x_c"] = augmented_bboxes_2d[0]
            new_bbox_dict[augmented_blob_name]["y_c"] = augmented_bboxes_2d[1]
            new_bbox_dict[augmented_blob_name]["w"] = augmented_bboxes_2d[2]
            new_bbox_dict[augmented_blob_name]["h"] = augmented_bboxes_2d[3]

    csv_path = pathlib.Path(save_path, "bounding_boxes.csv")
    csv_filename = str(csv_path)
    with open(csv_filename, mode="a", newline="") as csv_file:

        writer = csv.writer(csv_file)

        # write data rows
        for key, inner_dict in new_bbox_dict.items():
            row = {**inner_dict}
            vals = list(row.values())
            writer.writerow(vals)

    return augmented_imageids, bbox_2d_coords


if __name__ == "__main__":

    args = parser.parse_args()

    config_dir = args.config_dir
    compute_location = args.compute_location
    connection_string = args.connection_string
    num_images = args.num_images
    save_path = args.save_path

    helper.setup_save_path(save_path)

    config = Config(config_dir)

    projectairsim_log().info(f"Data Augmentation on {compute_location} is running.")
    augmented_imageids, bbox_2d_coords = augment_data(
        config.augmentation_spec, save_path, connection_string, compute_location
    )
