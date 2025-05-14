"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

This script serves as a convenient way to setup the azure blob container for data collection.
USAGE:
python setup_blob_container.py --sas-url <container-sas-url> --config-dir <path/to/config/dir> --script <path/to/script> --asset-dir <path/to/asset/dir> --pas-whl <path/to/projectairsim/whl> 

For more information, refer to the `docs/datacollection` folder

"""

import argparse
from azure.storage.blob import BlobServiceClient, BlobClient, ContainerClient
import os


parser = argparse.ArgumentParser("Blob Container Setup")

parser.add_argument(
    "--sas-url",
    help="Sas token for the azure blob container",
    required=True,
)

parser.add_argument(
    "--config-dir",
    help="Path to config directory",
    required=True,
)

parser.add_argument(
    "--script",
    help="Path to the collection script",
    required=True,
)

parser.add_argument(
    "--asset-dir",
    help="Path to asset directory",
    required=True,
)

parser.add_argument(
    "--pas-whl",
    help="Path to the projectairisim client whl file",
    required=True,
)


args = parser.parse_args()

config_dir = args.config_dir
asset_dir = args.asset_dir
script = args.script
sas_url = args.sas_url
pas_whl = args.pas_whl

COMMON_DIR = "Common"
CONTAINER_CONFIG_DIR = "Config"
CONTAINER_DATAGEN_CONFIG_DIR = "datagen_configs"
CONTAINER_ASSET_DIR = "assets"


def get_container_client(sas_url) -> ContainerClient:
    try:
        container_url, sas_token = sas_url.split("?")
        # Find the index of the last forward slash
        last_slash_index = container_url.rfind("/")
        account_url = container_url[:last_slash_index]
        container_name = container_url.split("/")[-1]
        blob_service_client = BlobServiceClient(
            account_url=account_url, credential=sas_token
        )
        container_client = blob_service_client.get_container_client(container_name)
        return container_client
    except Exception as e:
        print(f"Getting container client failed with error: {e}")
        return None


def upload_folder_to_blob_container(
    container_client: ContainerClient, blob_dir, folder_path
):
    # Upload files in the folder while maintaining the directory structure
    for root, _, files in os.walk(folder_path):
        for file in files:
            file_path = os.path.join(root, file)
            blob_path = file_path[
                len(folder_path) + 1 :
            ]  # Remove the root folder path from blob name
            blob_path = os.path.join(blob_dir, blob_path)

            # Upload file to given blob path
            container_client.upload_blob(
                name=blob_path,
                data=open(file_path, "rb").read(),
                overwrite=True,
            )
            print(f"Uploaded to container path {blob_path}")


def upload_file(container_client: ContainerClient, blob_dir, blob_path, file_path):
    # Upload file to given blob path
    blob_path = os.path.join(blob_dir, blob_path)
    container_client.upload_blob(
        name=blob_path,
        data=open(file_path, "rb").read(),
        overwrite=True,
    )
    print(f"Uploaded to container path {blob_path}")


def create_shell_script(file_path, commands):
    with open(file_path, "w") as f:
        # Write the shebang to specify the shell to be used for execution
        f.write("#!/bin/bash\n")

        # Write the commands to the file
        for command in commands:
            f.write(command + "\n")

    # Convert from CRLF TO LF TODO: Figure out why this is needed
    with open(file_path, "rb") as open_file:
        content = open_file.read()

    WINDOWS_LINE_ENDING = b"\r\n"
    UNIX_LINE_ENDING = b"\n"
    content = content.replace(WINDOWS_LINE_ENDING, UNIX_LINE_ENDING)

    with open(file_path, "wb") as open_file:
        open_file.write(content)


if __name__ == "__main__":
    container_client: ContainerClient = get_container_client(sas_url)
    # Setup paths
    container_config_dir = os.path.join(COMMON_DIR, CONTAINER_CONFIG_DIR)
    container_datagen_config_dir = os.path.join(
        container_config_dir, CONTAINER_DATAGEN_CONFIG_DIR
    )
    container_asset_dir = os.path.join(container_config_dir, CONTAINER_ASSET_DIR)
    # Upload Config Dir
    upload_folder_to_blob_container(
        container_client, container_datagen_config_dir, config_dir
    )
    # Upload Asset Dir
    upload_folder_to_blob_container(container_client, container_asset_dir, asset_dir)
    # Upload Script
    upload_file(
        container_client, container_config_dir, os.path.basename(script), script
    )
    # Upload PAS whl
    upload_file(
        container_client, container_config_dir, os.path.basename(pas_whl), pas_whl
    )

    # Create and upload shell script to run the collection script
    sh_file_name = "User.sh"
    tmp_folder = "./tmp"
    os.makedirs(tmp_folder, exist_ok=True)
    sh_file_path = os.path.join(tmp_folder, sh_file_name)
    commands = [
        "export DEBIAN_FRONTEND=noninteractive",
        "echo 'Running user commands...'",
        "# Update package list and install Python 3",
        "sudo -E apt update",
        "sudo -E add-apt-repository ppa:deadsnakes/ppa",
        "sudo -E apt update",
        "sudo -E apt install python3",
        "sudo -E apt-get install -y python3-pip",
        "# Install Packages",
        "sudo -H -E pip3 install 'numpy>=1.16.5,<1.23.0'",
        "sudo -E apt install -y python3-opencv",
        "sudo -H -E pip3 install projectairsim-2023.6.30-py3-none-any.whl",
        "sudo -H -E pip3 install -U albumentations --no-binary qudida,albumentations",
        "# Run script",
        f"python3 ./{os.path.basename(script)} --save-path $DCW_OUTPUT_DIR --sim-id $DCW_INSTANCE_NUM --num-sims $DCW_INSTANCE_COUNT --sas-url $DCW_SAS_URI",
    ]

    create_shell_script(sh_file_path, commands)
    upload_file(container_client, container_config_dir, sh_file_name, sh_file_path)
