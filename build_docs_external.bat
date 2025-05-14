REM Copyright (C) Microsoft Corporation. 
REM Copyright (C) IAMAI  Consulting Corporation.  

REM MIT License.

@echo off
SETLOCAL

REM python -m venv env
REM env\Scripts\activate.bat
REM python -m pip install mkdocs

if exist build_docs_external_src (
    echo.
    echo Deleting pre-existing build_docs_external_src_..
    echo.
    del /f /s /q build_docs_external_src\*
)

echo.
echo Rebuilding build_docs_external_src from docs...
robocopy docs build_docs_external_src /s /xd internal /nfl /ndl /np /njh

echo.
echo Merging external-only files to build_docs_external_src...
robocopy docs_external build_docs_external_src /s /nfl /ndl /np /njh

echo.
echo Building HTML files from docs_external...
mkdocs build -f mkdocs_external.yml

REM Preview on localhost http://127.0.0.1:8000
if "%1" == "serve" mkdocs serve -f mkdocs_external.yml
