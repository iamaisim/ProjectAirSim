"""
Copyright (C) Microsoft Corporation. All rights reserved.
Update client version in repo files listed in the template directory
"""
import argparse
import pathlib

import projectairsimpp


def get_template_placeholder_files(path_root):
    # Walk directory subtree and note each file
    return path_root.resolve().glob("**/*")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Apply the client version specification to the repo."
    )
    parser.add_argument(
        "template_directory",
        help="directory with files to be preprocessed to create the updated versions of the corresponding repo files",
        type=str,
        nargs="?",
        default="templates",
    )
    args = parser.parse_args()

    # Identify the template files
    path_root = pathlib.Path(args.template_directory).resolve()
    template_paths = get_template_placeholder_files(path_root)
    template_files = []
    for path in template_paths:
        if path.is_file():
            template_files.append(path)

    # Identify the target files
    path_cwd = pathlib.Path.cwd()
    target_files = {}
    for path in template_files:
        target_files[path] = path_cwd / path.relative_to(path_root)

    # Preprocess the template files to the target files
    print(f'Processing template directory "{path_root}":')
    for template_path, target_path in target_files.items():
        target_exists = target_path.exists()
        print(f"    {template_path}", end="", flush=True)
        projectairsimpp.preprocess_file(
            filename_input=str(template_path),
            filename_output=str(target_path),
            log_level="info",
        )
        print(f" --> {target_path} ({'replaced' if target_exists else 'new'})")
