import json
import pathlib

import jsonlines
import numpy as np

from projectairsim.datacollection.types import Dataset


def read_jsonl(file):

    data = []
    with open(file) as f:
        for line in f:
            data.append(json.loads(line))

    return data


"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Script for generating a jsonl file based on an input `Dataset` object as well
as static information from the config
The jsonl holds each image as entry in the global list. Each entry holds 2d/3d bbox data
"""


def write_to_jsonl(data, save_path):
    with jsonlines.open(save_path, "w") as writer:
        writer.write_all(data)

    return save_path


def make_base_jsonl():
    jsonl_data = []

    return jsonl_data


def populate_jsonl(jsonl_data: list, dataset: Dataset, output_spec: dict):

    if jsonl_data == None:
        jsonl_data = []
    image_spec: dict = output_spec.get("image-spec", {})
    image_width: int = image_spec["image-width"]
    image_height: int = image_spec["image-height"]
    azureml_spec: dict = output_spec.get("azureml-spec", {})
    images = dataset.images

    for image in images:
        image_dict = {}
        image_url = pathlib.Path(azureml_spec.get("datastore", ""), image.name)
        image_dict["image_url"] = str(image_url).replace("\\", "/")
        image_dict["image-width"] = image_width
        image_dict["image-height"] = image_height
        if image.id in dataset.non_collected_images:
            continue
        if image.bbox_data.bbox_2d != []:
            x_c, y_c, w, h = image.bbox_data.bbox_2d
            bbox = [x_c - w / 2, y_c - h / 2, x_c + w / 2, y_c + h / 2]
            bbox = list(
                np.divide(bbox, [image_width, image_height, image_width, image_height])
            )

            label_dict = {}
            label_dict["label"] = image.bbox_data.category
            label_dict["isCrowd"] = False
            label_dict["label_confidence"] = [1.0]
            label_dict["topX"] = bbox[0]
            label_dict["topY"] = bbox[1]
            label_dict["bottomX"] = bbox[2]
            label_dict["bottomY"] = bbox[3]

        if image.bbox_data.bbox_3d != []:
            label_dict["polygon"] = image.bbox_data.bbox_3d

        image_dict["label"] = label_dict
        jsonl_data.append(image_dict)

    return jsonl_data
