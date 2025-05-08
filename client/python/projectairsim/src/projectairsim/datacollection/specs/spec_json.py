import json

import numpy as np

"""
Copyright (C) Microsoft Corporation. All rights reserved.

Script for generating a json file based on an input `Dataset` object as well
as static information from the config
This json file holds information about the dataset as well as metadata about each image. The metadata
includes annotations like 2d/3d bbox as well as the lat-lon, time, weather etc
"""

from projectairsim.datacollection.types import Dataset


def read_json(file):

    with open(file) as f:
        data = json.load(f)

    return data


def write_to_json(data, save_path):
    with open(save_path, "w") as file:
        json.dump(data, file, indent=4)

    return save_path


def make_base_json(output_spec: dict):

    azure_spec = output_spec.get("azure-spec", {})

    json_data = {}

    json_data["baseuri"] = (
        azure_spec.get("account-url", "") + "/" + azure_spec.get("container-name", "")
    )
    json_data["categories"] = []
    json_data["dataset-name"] = output_spec["dataset-name"]
    json_data["images"] = []

    return json_data


def populate_json(json_data, dataset: Dataset, output_spec):
    image_spec = output_spec.get("image-spec", {})
    image_width = image_spec["image-width"]
    image_height = image_spec["image-height"]
    images = dataset.images
    for image in images:

        image_dict = {}
        image_dict["image-width"] = image_width
        image_dict["image-height"] = image_height
        file = {
            "hash": "",
            "key": "",
            "object-type": "File",
            "storage-type": "azure-blob",
            "uri": image.name,
        }

        image_dict["file"] = file
        image_dict["format"] = image_spec["image-format"]
        image_dict["split"] = "validate"

        if image.id in dataset.non_collected_images:
            continue
        if image.bbox_data.category not in json_data.get("categories"):
            json_data.get("categories").append(image.bbox_data.category)
        if image.bbox_data.bbox_2d != []:
            x_c, y_c, w, h = image.bbox_data.bbox_2d
            box = [x_c - w / 2, y_c - h / 2, w, h]
            box = list(
                np.divide(box, [image_width, image_height, image_width, image_height])
            )
            box_dict = {
                "height": box[3],
                "left": box[0],
                "score": 1.0,
                "tag": image.bbox_data.category,
                "top": box[1],
                "width": box[2],
            }
        if image.bbox_data.bbox_3d != []:
            image_dict["polygon"] = image.bbox_data.bbox_3d

        image_dict["boxes"] = [box_dict]
        image_dict["weather"] = image.weather
        image_dict["time"] = image.time_of_day
        image_dict["geo-location"] = image.location
        image_dict["lat-lon"] = image.lat_lon

        json_data["images"].append(image_dict)

    return json_data
