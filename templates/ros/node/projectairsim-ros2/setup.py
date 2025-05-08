#!/usr/bin/env python
# Project AirSim ROS 2 node bridge support package

from setuptools import setup


setup(
    name="projectairsim-ros2",
    version="{# include "client_version.txt" #}",
    description="Project AirSim ROS 2 support package",
    long_description="To be populated from a README.md",  # TODO Populate from a README.md
    package_dir={"": "src"},
    packages=["projectairsim_ros2"],
    include_package_data=True,
    package_data={"": ["schema/*.jsonc"]},
    python_requires=">=3.7, <4",
    install_requires=[
        "rclpy",
    ],
)
