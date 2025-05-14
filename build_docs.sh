#!/bin/bash
# Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

## Install mkdocs
# python3.7 -m venv env
# source env/bin/activate
# python3.7 -m pip install mkdocs

cp README.md docs/

# find and replace "](docs/" by "]("
sed -i 's/](docs\//](/g' docs/README.md

mkdocs build

cp docs/web.config build_docs/

## Preview on localhost http://127.0.0.1:8000
# mkdocs serve
