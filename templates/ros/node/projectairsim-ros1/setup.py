#!/usr/bin/env python
# Project AirSim ROS 1 node bridge support package

from setuptools import setup


setup(
    name="projectairsim-ros1",
    version="{# include "client_version.txt" #}",
    description="Project AirSim ROS 1 support package",
    long_description="To be populated from a README.md",  # TODO Populate from a README.md
    package_dir={"": "src"},
    packages=["projectairsim_ros1"],
    include_package_data=True,
    package_data={"": ["schema/*.jsonc"]},
    python_requires=">=3.7, <4",
    install_requires=[
        "rospy",
    ],
)
