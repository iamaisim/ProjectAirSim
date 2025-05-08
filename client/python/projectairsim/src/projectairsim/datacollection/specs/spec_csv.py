"""
Copyright (C) Microsoft Corporation. All rights reserved.

Script for generating a csv file based on an input `Dataset` object as well
as static information from the config
The format of the csv is:
[ImageName, pose_x, psoe_y, pose_z, roll, pitch, yaw, 3d_bbox params, 2d_bbox params]
The 3d bbox is formatted as what the sim returns - 8 pixel coordinate frame points: x0,y0....x7,y7
The 2d bbox is formatted as [x_c, y_c, w, h] where (x_c, y_c) is the center of the bbox
"""

import csv
from typing import List

from projectairsim.datacollection.types import Annotation, Dataset


def read_csv(file):
    output = []
    with open(file) as f:
        reader = csv.reader(f)
        for row in reader:
            output.append(row)

    return output


def write_to_csv(save_path, bbox_data):

    with open(save_path, "w", newline="") as csvfile:
        writer = csv.writer(csvfile)
        for row in bbox_data:
            writer.writerow(row)
        return save_path


def make_bbox_csv(data_spec):

    headers = [
        "ImageName",
        "pose_x",
        "pose_y",
        "pose_z",
        "roll",
        "pitch",
        "yaw",
        "category",
    ]
    annotations = data_spec.get("annotations")
    if Annotation.THREE_D_BBOX in annotations:
        headers.extend(
            [
                "x0",
                "y0",
                "x1",
                "y1",
                "x2",
                "y2",
                "x3",
                "y3",
                "x4",
                "y4",
                "x5",
                "y5",
                "x6",
                "y6",
                "x7",
                "y7",
            ]
        )
    if Annotation.TWO_D_BBOX in annotations:
        headers.extend(
            [
                "x_c",
                "y_c",
                "w",
                "h",
            ]
        )

    return [headers]


def populate_csv(csv_data: List, dataset: Dataset):

    images = dataset.images
    for image in images:
        image_id = image.id
        if image_id in dataset.non_collected_images:
            continue
        row = [image.name]
        row.extend(image.pose)
        if image.negative_sample:
            row.append("")
        else:
            row.append(image.bbox_data.category)

        if image.bbox_data.bbox_3d != []:
            for point in image.bbox_data.bbox_3d:
                row.extend(point)
        if image.bbox_data.bbox_2d != []:
            row.extend(image.bbox_data.bbox_2d)
        csv_data.append(row)

    return csv_data
