#!/usr/bin/env python
# Project AirSim ROS node bridge package

from setuptools import setup


setup(
    name="projectairsim-rosbridge",
    version="{# include "client_version.txt" #}",
    description="Project AirSim ROS bridge core package",
    long_description="To be populated from a README.md",  # TODO Populate from a README.md
    package_dir={"": "src"},
    packages=["projectairsim_rosbridge"],
    include_package_data=True,
    package_data={"": ["schema/*.jsonc"]},
    python_requires=">=3.7, <4",
    install_requires=[
        "projectairsim",
        "numpy",
    ],
)
