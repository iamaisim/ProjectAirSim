# Optional VS Code User Settings
{# include enable_internal_docs.tpp #}

Some required VS Code settings for the Project AirSim project are already included in the repo's `.vscode` folders, but there are many additional user settings that can help with development.

Here are some example optional User settings:

```
{
    "editor.tabSize": 2,
    "editor.showFoldingControls": "always",
    "editor.minimap.showSlider": "always",
    "editor.rulers": [
        80
    ],
    "files.trimTrailingWhitespace": true,
    "files.exclude": {
        "**/Intermediate": false,
        "**/Binaries": true,
        "**/Saved": true,
        "**/Content": true,
        "**/.pytest_cache": true,
        "**/__pycache__": true,
        "**/Plugins/ProjectAirSim/SimLibs": true
    },
    "debug.onTaskErrors": "abort",
    "debug.toolBarLocation": "docked",
    "debug.showBreakpointsInOverviewRuler": true,
    "terminal.integrated.scrollback": 1000000,
    "cmake.configureOnOpen": false,
    "cmake.autoSelectActiveFolder": false,
    "C_Cpp.intelliSenseEngineFallback": "Enabled",
    "C_Cpp.vcpkg.enabled": false,
    "[cpp]": {
        "editor.defaultFormatter": "ms-vscode.cpptools"
    },
    "testMate.cpp.test.executables": "build/**/Debug/**/*test*",
    "python.linting.enabled": true,
    "python.linting.pylintEnabled": false,
    "python.linting.flake8Enabled": true,
    "python.linting.flake8CategorySeverity.E": "Information",
    "python.linting.flake8Args": [
        "--max-line-length=88"
    ],
    "[python]": {
        "editor.defaultFormatter": "ms-python.python",
        "editor.rulers": [
            88
        ],
    },
    "python.formatting.provider": "black",
    "python.formatting.blackArgs": [
        "--line-length",
        "88"
    ]
}
```

---

Copyright (C) Microsoft Corporation. 
Copyright (c) 2025 IAMAI Simulations.

MIT License. All rights reserved.
