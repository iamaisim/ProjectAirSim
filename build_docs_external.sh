#!/bin/bash
# Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

## Install mkdocs
# python3.7 -m venv env
# source env/bin/activate
# python3.7 -m pip install mkdocs

echo Deleting pre-existing build_docs_external_src...
if [ -d build_docs_external_src ]; then
    rm -rf build_docs_external_src
fi

echo Rebuilding build_docs_external_src from docs...
cp -r docs build_docs_external_src
if [ -d build_docs_external_src/internal ]; then
    rm -rf build_docs_external_src/internal
fi

echo Merging docs_external files to build_docs_external_src...
if [ -d docs_external ]; then
    cp -r docs_external/* build_docs_external_src/
fi

echo Building HTML files from build_docs_external_src...
mkdocs build -f mkdocs_external.yml

## Preview on localhost http://127.0.0.1:8000
# mkdocs serve -f mkdocs_external.yml
