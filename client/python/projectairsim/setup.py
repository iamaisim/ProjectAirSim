#!/usr/bin/env python
from setuptools import setup

# Compatibility shim: all metadata is defined in pyproject.toml (PEP 621).
# Keeping this file allows legacy workflows like:
#   python setup.py sdist bdist_wheel
if __name__ == "__main__":
    setup()
