"""
Copyright (C) Microsoft Corporation. All rights reserved.

Script for aggregating information in the `Dataset` object as well
as static information from the config

The script generates four files - [csv, json, jsonl, coco_json]

These files are either written locally or uploaded to a container on Azure

On Azure, the assumption is that the datacollection load would have been distributed over multiple
compute instances and thus information from each of these need to be aggregated into one final file. For this
reason, a system is implemented that avoids concurrent writes to the same file by multiple instances. This is
achieved through the `lease` function that locks a file from reads and writes until the lease is broken.
If a file is trying to write to a csv file for example, it will check if the file on the container is leased. If it is,
it will wait till it isn't. If it isn't, it will lease it, write to it, upload it back to the container and then break the lease.
"""

import os
import pathlib
from typing import Dict
import time

from azure.storage.blob import BlobLeaseClient, ContainerClient, BlobClient
from projectairsim.utils import projectairsim_log

from projectairsim.datacollection.specs import (
    spec_coco,
    spec_csv,
    spec_json,
    spec_jsonl,
)
from projectairsim.datacollection.types import Dataset

BLOB_BASE_PATH = os.path.join("Common", "Output")


def populate_specs(
    dataset: Dataset,
    output_spec: Dict,
    save_path: pathlib.Path,
    data_spec: Dict,
    compute_location: str,
    container_client: str = ContainerClient,
) -> None:
    if compute_location == "aks":
        save_path = pathlib.Path(save_path, "..", "specs")
        os.makedirs(save_path, exist_ok=True)

    specs_file_names = {
        "jsonl": "bounding_boxes.jsonl",
        "json": "bounding_boxes.json",
        "csv": "bounding_boxes.csv",
        "coco": "bounding_boxes_coco.json",
    }

    projectairsim_log().info("Generating Base Specs")
    make_base_specs(
        output_spec, specs_file_names, save_path, data_spec, container_client
    )

    projectairsim_log().info("Populating Specs")
    if compute_location == "local":
        populate_specs_local(
            dataset, save_path, specs_file_names, output_spec, data_spec
        )
    elif compute_location == "aks":
        populate_specs_aks(
            output_spec,
            save_path,
            specs_file_names,
            dataset,
            data_spec,
            container_client,
        )
    else:
        projectairsim_log().info(
            "Compute Location not identified. Can not populate specs"
        )


def upload_blob(
    container_client: ContainerClient,
    blob_name: str,
    file_path: pathlib.Path,
    lease_id=None,
) -> None:
    if lease_id is None:
        container_client.upload_blob(
            name=blob_name,
            data=open(file_path, "rb").read(),
            overwrite=True,
        )
    else:
        container_client.upload_blob(
            name=blob_name,
            data=open(file_path, "rb").read(),
            overwrite=True,
            lease=lease_id,
        )
    return


def make_base_specs(
    output_spec: Dict,
    specs_file_names: Dict[str, str],
    save_path: pathlib.Path,
    data_spec: Dict,
    container_client: ContainerClient,
) -> None:
    # TODO: Upload each of these files to container
    blob_list = [blob.name for blob in container_client.list_blobs()]

    for key in specs_file_names.keys():
        if specs_file_names.get(key) not in blob_list:
            file_save_path = pathlib.Path(save_path, specs_file_names.get(key))
            if key == "jsonl":
                jsonl_data = spec_jsonl.make_base_jsonl()
                spec_jsonl.write_to_jsonl(jsonl_data, file_save_path)

            if key == "json":
                json_data = spec_json.make_base_json(output_spec)
                spec_json.write_to_json(json_data, file_save_path)

            if key == "csv":
                csv_temp_data = spec_csv.make_bbox_csv(data_spec)
                spec_csv.write_to_csv(file_save_path, csv_temp_data)

            if key == "coco":
                spec_coco.make_base_coco_json(output_spec, file_save_path)

            upload_blob(
                container_client,
                blob_name=os.path.join(BLOB_BASE_PATH, specs_file_names[key]),
                file_path=file_save_path,
            )

    return


def populate_specs_local(
    dataset: Dataset,
    save_path: pathlib.Path,
    spec_file_names: Dict,
    output_spec: Dict,
    data_spec: Dict,
) -> None:
    for file_name in spec_file_names.keys():
        file_save_path = pathlib.Path(save_path, spec_file_names.get(file_name))
        if file_name == "json":
            json_data = spec_json.read_json(file_save_path)
            json_data = spec_json.populate_json(json_data, dataset, output_spec)
            spec_json.write_to_json(json_data, file_save_path)

        elif file_name == "jsonl":
            jsonl_data = spec_jsonl.read_jsonl(file_save_path)
            jsonl_data = spec_jsonl.populate_jsonl(jsonl_data, dataset, output_spec)
            spec_jsonl.write_to_jsonl(jsonl_data, file_save_path)

        elif file_name == "csv":
            csv_data = spec_csv.read_csv(file_save_path)
            csv_data = spec_csv.populate_csv(csv_data, dataset)
            spec_csv.write_to_csv(file_save_path, csv_data)

        elif file_name == "coco":
            spec_coco.make_base_coco_json(output_spec, file_save_path)
            coco_data = spec_coco.populate_coco_json(
                dataset, output_spec, file_save_path, data_spec
            )
            spec_json.write_to_json(coco_data, file_save_path)


def populate_specs_aks(
    output_spec: Dict,
    save_path: pathlib.Path,
    specs_file_names: Dict[str, str],
    dataset: Dataset,
    data_spec: Dict,
    container_client: ContainerClient,
    sim_id: int = 0,
) -> None:
    extra_path = pathlib.Path(save_path, "extra")
    os.makedirs(extra_path, exist_ok=True)
    parsed = []

    while len(parsed) != len(specs_file_names):
        for spec in specs_file_names:
            if spec in parsed:
                continue
            file_name = specs_file_names[spec]

            # Get BlobClient
            blob_client = container_client.get_blob_client(
                os.path.join(BLOB_BASE_PATH, file_name)
            )
            blob_prop = blob_client.get_blob_properties()

            # Check if the file is leased. If it is, wait till it isn't
            if blob_prop["lease"]["status"] == "locked":
                projectairsim_log().info(
                    f"{specs_file_names[spec]} locked. Will try again later"
                )
                continue

            # If it isn't, lease it
            projectairsim_log().info(f"{specs_file_names[spec]} not leased. Leasing it")
            lease_client = blob_client.acquire_lease()
            lease_id = lease_client.id

            projectairsim_log().info(f"Populating {specs_file_names[spec]}")
            file_save_path = pathlib.Path(save_path, file_name)
            file_write_path = pathlib.Path(extra_path, file_name)
            # Download whats on the blob container
            with open(file_save_path, "wb") as download_file:
                download_file.write(blob_client.download_blob(lease=lease_id).readall())

            if spec == "csv":
                csv_data = spec_csv.read_csv(file_save_path)
                csv_data = spec_csv.populate_csv(csv_data, dataset)
                spec_csv.write_to_csv(file_write_path, csv_data)

            elif spec == "json":
                json_data = spec_json.read_json(file_save_path)
                json_data = spec_json.populate_json(json_data, dataset, output_spec)
                spec_json.write_to_json(json_data, file_write_path)

            elif spec == "jsonl":
                jsonl_data = spec_jsonl.read_jsonl(file_save_path)
                jsonl_data = spec_jsonl.populate_jsonl(jsonl_data, dataset, output_spec)
                spec_jsonl.write_to_jsonl(jsonl_data, file_write_path)
            elif spec == "coco":
                coco_data = spec_coco.populate_coco_json(
                    dataset, output_spec, file_save_path, data_spec
                )
                spec_json.write_to_json(coco_data, file_write_path)

            projectairsim_log().info(f"Uploading {specs_file_names[spec]}")
            container_path = os.path.join(BLOB_BASE_PATH, specs_file_names[spec])
            upload_blob(container_client, container_path, file_write_path, lease_id)
            lease_client.release()
            parsed.append(spec)

        projectairsim_log().info(f"Sleeping")
        time.sleep(5)
