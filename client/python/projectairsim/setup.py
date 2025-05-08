#!/usr/bin/env python
# ProjectAirSim python client package

from setuptools import setup


setup(
    name="projectairsim",
    version="2023.9.15",
    description="ProjectAirSim client package",
    long_description="To be populated from a README.md",  # TODO Populate from a README.md
    package_dir={"": "src"},
    packages=[
        "projectairsim",
        "projectairsim.autonomy",
        "projectairsim.autonomy.models",
        "projectairsim.autonomy.models.backbone",
        "projectairsim.autonomy.gym_envs",
        "projectairsim.datacollection",
        "projectairsim.datacollection.augmentation",
        "projectairsim.datacollection.collection",
        "projectairsim.datacollection.randomization",
        "projectairsim.datacollection.trajectory",
        "projectairsim.datacollection.specs",
        "projectairsim.datacollection.validation",
    ],
    include_package_data=True,
    package_data={
        "": ["schema/*.jsonc"],
        "projectairsim.autonomy.gym_envs": ["sim_config/*.jsonc", "*.ink"],
    },
    install_requires=[
        "pynng>=0.5.0",
        "msgpack>=1.0.5",
        "opencv-python>=4.2.0.32",
        "numpy",
        "matplotlib",
        "commentjson>=0.9.0",
        "jsonschema>=4.4.0",
        "inputs",
        "pqdict",
        "cryptography",
        "pykml",
        "junit-xml",
        "jsonlines",
        "Shapely",
        "azure-storage-blob",
        "open3d"  # TODO: LidarDisplay view setting is broken from 0.17.0
    ],
    python_requires=">=3.7, <4",
    extras_require={
        "autonomy": [
            "torch>=1.8.0",
            "torchvision>=0.9.0",
            "scikit-image>=0.18.1",
            "Pillow<=8.2.0",  # Required since pip doesn't resolve gym's req ver
            "gym==0.18.3",
            "onnxruntime-gpu",
            "pydantic",
            "python-multipart",
            "fastapi",
            "uvicorn",
        ],
        "bonsai": [
            "microsoft-bonsai-api==0.1.2",
            "python-dotenv==0.13.0",
            "bonsai-cli==1.0.8",
            "msal-extensions==0.1.3",
        ],
        "datacollection": ["albumentations==1.3.0"],
    },
)
