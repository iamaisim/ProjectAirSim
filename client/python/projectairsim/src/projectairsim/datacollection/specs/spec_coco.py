"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Script for generating a coco json file based on an input `Dataset` object as well
as static information from the config
"""

import datetime
import json
import pathlib

from projectairsim.datacollection.types import COCOAnnotations, Dataset
from projectairsim.datacollection.types import Annotation


def read_json(file: pathlib.Path) -> dict:

    with open(file) as f:
        data = json.load(f)

    return data


def write_to_json(data: dict, save_path: pathlib.Path, is_populate: bool) -> str:
    if not is_populate:
        save_path = pathlib.Path(save_path, "bounding_boxes.json")
    with open(save_path, "w") as file:
        json.dump(data, file, indent=4)

    return save_path


def image(image_name: str, output_spec: dict) -> dict:
    image = {}

    image["id"] = image_name.replace(".png", "")
    image["license"] = 0
    image["file_name"] = str(pathlib.Path("images", image_name)).replace("\\", "/")
    image["coco_url"] = image_name
    image["absolute-url"] = str(
        pathlib.Path(
            "images",
            image_name,
        )
    ).replace("\\", "/")
    image["height"] = output_spec["image-spec"]["image-height"]
    image["width"] = output_spec["image-spec"]["image-width"]
    image["date-captured"] = str(datetime.date.today())
    return image


def annotation(
    img_name: str, annotation_id: int, segmentation=[], polygon=[], category_id=-1
) -> dict:
    annotation = {}

    annotation["id"] = annotation_id
    annotation["category_id"] = category_id
    annotation["image_id"] = img_name
    annotation["area"] = polygon.area
    annotation["iscrowd"] = 0
    annotation["segmentation"] = segmentation
    return annotation


def categories(category_data: dict) -> list:

    categories_list = []
    for category in category_data.keys():
        category_dict = {}
        category_dict["id"] = category_data[category]
        category_dict["name"] = category
        category_dict["supercategoty"] = "None"
        categories_list.append(category_dict)

    return categories_list


def make_base_coco_json(output_spec: dict, file_path: pathlib.Path) -> str:

    licenses = []

    info = {
        "description": output_spec["description"],
        "version": 1.0,
        "year": 2021,
        "contributor": "Project AirSim",
    }

    license = {"id": 0, "url": "/", "name": "Public Domain"}
    licenses.append(license)

    data_coco = {}
    data_coco["info"] = info
    data_coco["licenses"] = licenses
    data_coco["categories"] = []
    data_coco["images"] = []
    data_coco["annotations"] = []
    json.dump(data_coco, open(file_path, "w"), indent=4)

    return file_path


# Note - COCO Format only works for Segmentation. Use other specs for other specs
def populate_coco_json(
    dataset: Dataset, output_spec: dict, coco_file_path: pathlib.Path, data_spec: dict
) -> dict:

    if Annotation.SEGMENTATION not in data_spec.get("annotations"):
        return {}

    coco_data = read_json(coco_file_path)
    coco_images = coco_data.get("images")
    collected_images = dataset.images
    annotation_id = 0
    category_data = categories(dataset.categories)

    coco_data.get("categories").append(category_data)

    for image_data in collected_images:

        segmentation_data = image_data.segmentation_data
        image_ins = image(image_data.name, output_spec)
        coco_images.append(image_ins)

        if image_data.id in dataset.non_collected_images:
            continue

        for coco_annotation in segmentation_data:
            coco_annotation: COCOAnnotations
            annotation_dict = annotation(
                image_data.name,
                annotation_id,
                coco_annotation.segmentation,
                coco_annotation.polygon,
                coco_annotation.category_id,
            )
            coco_data["annotations"].append(annotation_dict)
            annotation_id += 1
    return coco_data
