REM Copyright (C) Microsoft Corporation. All rights reserved.

@echo off
SETLOCAL

REM python -m venv env
REM env\Scripts\activate.bat
REM python -m pip install mkdocs

copy README.md docs\README.md

REM Strip 'docs/' out of link/image paths in the README.md that was moved into
REM the docs subfolder for mkdocs.
SET find=(docs/
SET replace=(

REM Loop through lines but prepend [line #] to each line so the for loop doesn't
REM skip blank lines, then delimit on the ] to set s to the line contents. For
REM blank lines, echo a blank line to the output.
REM
REM To prevent the command processor from handling special characters like the
REM stream redirection operators, the line contents is quoted, processed, then
REM stripped of the outer quotes before appending to the file.
for /f "delims=] tokens=1*" %%a in ('type docs\README.md ^| find /v /n ""') do (
    SET s=%%b
    if defined s (
        set s=" %%b "
        call SET s=%%s:%find%=%replace%%%

        SETLOCAL ENABLEDELAYEDEXPANSION
        echo !s:~2,-2! >> docs\temp_README.md
        ENDLOCAL
    ) else (
        echo: >> docs\temp_README.md
    )
)
move docs\temp_README.md docs\README.md

mkdocs build

copy docs\web.config build_docs

REM Preview on localhost http://127.0.0.1:8000

if "%1" == "serve" mkdocs serve
