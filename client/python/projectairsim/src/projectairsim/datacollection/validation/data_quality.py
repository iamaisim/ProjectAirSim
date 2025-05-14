"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Script for computing quantitative metrics about the data as well as running
various validation checks on it. Also allows for the generation of data cards
that summarize the input params of the dataset as well as quantitative and
qualitative aspects of the output dataset
"""
import os
import pathlib
from datetime import datetime
from typing import Dict, List

from projectairsim.types import WeatherParameter
from projectairsim.utils import projectairsim_log

import projectairsim.datacollection.utils as utils
from projectairsim.datacollection import __airsim_client_version__, __version__
from projectairsim.datacollection.specs.spec_json import write_to_json
from projectairsim.datacollection.types import COCOAnnotations, Dataset, ImageData


class DataQualityValidator:
    def __init__(
        self,
        dataset: Dataset,
        save_path: pathlib.Path,
        data_spec: Dict,
        config: Dict,
        sim_id: int = 0,
    ) -> None:
        self.dataset = dataset
        self.save_path = save_path
        self.data_spec = data_spec
        self.config = config
        self.sim_id = sim_id

    # Validate and clean dataset
    def validate_data(self):

        temp = self.dataset.images.copy()
        discarded_images = 0

        for i, image in enumerate(temp):
            image: ImageData

            # Remove data point if not valid
            if not self.check_image_file_integrity(image):
                self.dataset.images.remove(image)
                discarded_images += 1
                continue

            # Remove data point if not valid
            if not self.check_annotation_validity(image):
                self.dataset.images.remove(image)
                discarded_images += 1
                continue

            if self.check_negative_sample(image):
                self.dataset.images[i].negative_sample = True

            # Check metadata validity
            self.check_metadata_validity(image)

        projectairsim_log().info(f"{discarded_images} images discarded")

    def generate_data_card(self) -> Dict:

        self.compute_quantitative_stats()

        metadata_dict = {
            "core-sim-version": __airsim_client_version__,
            "data-infra-version": __version__,
            "num-samples": len(self.dataset.images),
            "collection-date-time": str(datetime.now()),
        }

        quantitative_dict = {
            "composition": self.data_composition,
            "data-quality": {},
        }

        self.config["output-dataset-spec"] = {
            "metadata": metadata_dict,
            "quantitative-specs": quantitative_dict,
            "configs": self.dataset.configs,
        }

        return self.config

    def write_data_card(self) -> bool:

        try:
            extra_path = pathlib.Path(self.save_path, "data_cards")
            os.makedirs(extra_path, exist_ok=True)

            file_name = "datacollector_config_output" + str(self.sim_id) + ".jsonc"
            file_path = pathlib.Path(extra_path, file_name)
            write_to_json(self.config, file_path)
        except Exception as e:
            projectairsim_log().warning(f"Writing data card to file failed with {e}")
            return False

        return True

    def compute_quantitative_stats(self):

        self.data_composition = {}

        for image in self.dataset.images:
            image: ImageData

            # Note weather, time and location composition
            self.get_composition(image)

    # Check if image file exists
    def check_image_file_integrity(self, image: ImageData):

        name = image.name
        image_path = pathlib.Path(self.save_path, "images", name)

        if image_path.is_file():
            return True

        return False

    # Check if 2d bbox GT is valid
    def check_2DBBox_validity(self, image: ImageData):

        bbox_2d = image.bbox_data.bbox_2d
        height = image.height
        width = image.width

        x_vals = bbox_2d[::2]
        y_vals = bbox_2d[1::2]

        if len(bbox_2d) != 4:  # 2d bbox format - [x_c, y_c, w, h]
            return False

        # all values in the array should be between 0 and height/width
        if not (
            utils.check_range(x_vals, 0, width) and utils.check_range(y_vals, 0, height)
        ):
            return False

        return True

    def check_negative_sample(self, image: ImageData):

        bbox_2d = image.bbox_data.bbox_2d  # 2d bbox format - [x_c, y_c, w, h]
        bbox_width = bbox_2d[2]
        bbox_height = bbox_2d[3]

        if bbox_width == bbox_height == 0:
            return True

        return False

    # Check if 3d bbox GT is valid
    def check_3DBBox_validity(self, image: ImageData):

        bbox_3d = image.bbox_data.bbox_3d
        height = image.height
        width = image.width

        x_vals = bbox_3d[::2]
        y_vals = bbox_3d[1::2]

        if len(bbox_3d) != 16:  # 2d bbox format - [x0,y0,x1,y1...x7,y7]
            return False

        # all values in the array should be between 0 and height/width
        if not (
            utils.check_range(x_vals, 0, width) and utils.check_range(y_vals, 0, height)
        ):
            return False

        return True

    # Check if segmentation GT is valid
    def check_segmentation_validity(self, image: ImageData):

        segmentation_data: List[COCOAnnotations]
        segmentation_data = image.segmentation_data

        for annotation in segmentation_data:

            # TODO: Add more tests for segmentation data
            if type(annotation) != COCOAnnotations:
                return False

        return True

    # Check GT data validity. In case of failure, data point is removed from dataset
    def check_annotation_validity(self, image: ImageData):

        for spec in self.data_spec:

            if spec == "2DBBox" and not self.check_2DBBox_validity(image):
                return False

            if spec == "3DBBox" and not self.check_3DBBox_validity(image):
                return False

            if spec == "segmentation" and not self.check_segmentation_validity(image):
                return False

        return True

    # Check f associated data is valid. In case of failure, a warning is logged
    def check_metadata_validity(self, image: ImageData):

        # Check weather validity
        if image.weather not in WeatherParameter._member_names_:
            projectairsim_log().info(f"{image.name}: Invalid weather {image.weather}")

        # Check pose validity
        if len(image.pose) != 6 or not (
            all(isinstance(x, (float, int)) for x in image.pose)
        ):
            projectairsim_log().info(f"{image.name}: Invalid pose {image.pose}")

        return True

    def get_composition(self, image: ImageData):

        location = image.location
        weather = image.weather
        time = image.time_of_day
        point_of_interest = image.point_of_interest

        if location not in self.data_composition:
            self.data_composition[location] = {}

        if point_of_interest not in self.data_composition.get(location):
            self.data_composition[location][point_of_interest] = {}

        if weather not in self.data_composition.get(location).get(point_of_interest):
            self.data_composition[location][point_of_interest][weather] = {}

        if time not in self.data_composition.get(location).get(point_of_interest).get(
            weather
        ):
            self.data_composition[location][point_of_interest][weather][time] = 0

        self.data_composition[location][point_of_interest][weather][time] += 1
