# Developer Initial Setup for Windows
{# include enable_internal_docs.tpp #}

On Windows, Project AirSim can be developed with either Visual Studio 2019 or VS Code. VS Code is convenient because it provides a lighter-weight, cross-platform common experience between Windows and Linux.

## Setup

1.  A) To develop with **VS Code**:

    First, install the **[VS 2022 C++ build tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/)** with:

    - `Desktop development with C++` workload
    - `.NET Framework 4.8 SDK` individual component
    - (no)`.NET Core SDK` individual component

    Second, install **[VS Code](https://code.visualstudio.com/)** with the following extensions:

    *Note: If not manually installed, these extensions will be recommended for automatic install on opening the Project AirSim project workspace*

    - **[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)**
    - **[C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)** (includes clang-format functionality, use **Alt-Shift-F** to auto-format the current code file using the repo's .clang-format settings)
    - **[CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb)**
    - **[C#](https://marketplace.visualstudio.com/items?itemName=ms-dotnettools.csharp)**
    - **[Mono Debug](https://marketplace.visualstudio.com/items?itemName=ms-vscode.mono-debug)**
    - **[C++ TestMate](https://marketplace.visualstudio.com/items?itemName=matepek.vscode-catch2-test-adapter)** (also automatically installs **[Test Explorer UI](https://marketplace.visualstudio.com/items?itemName=hbenl.vscode-test-explorer)**)
    - **[Python](https://marketplace.visualstudio.com/items?itemName=ms-python.python)**
    - **[Pylance](https://marketplace.visualstudio.com/items?itemName=ms-python.vscode-pylance)**

    Third, see **[Optional VS Code User Settings](vscode_user_settings.md)** for some example customized user settings that can help with Project AirSim development.

    B) To develop with **Visual Studio 2019**:

    Install **[Visual Studio 2019](https://visualstudio.microsoft.com/vs/)** with:

    - `Desktop development with C++` workload
    - `.NET Framework 4.8 SDK` individual component
    - `.NET Core SDK` individual component

2. Install the **[Epic Games Launcher](https://www.unrealengine.com/en-US/)** and install Unreal Engine 5.2 binary (requires Epic account log-in). While selecting the engine version to install, there is also an `Options` section where you can enable downloading `Editor symbols for debugging` (~30 GB) if desired. **Note**: Installing the engine can take a long time **(~1 hour)**.

3. Set a Windows environment variable for `UE_ROOT` to the installed folder, either through the Control Panel section `Edit environment variables for your account`, or by using the command line:

    ```
    setx UE_ROOT "C:\Program Files\Epic Games\UE_5.2"

    <restart the command prompt to refresh environment variables>
    ```

4. Unreal Engine on Windows defaults to using DirectX 11 for rendering, so you may need to install the latest GPU driver and DirectX updates. However, if you want to use Vulkan for rendering instead, you may need to install the **[Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)**.

5. Install **[Git for Windows](https://gitforwindows.org/)** (including Git Credential Manager).

6. Clone the Project AirSim repo (HTTPS authentication is recommended for using Git Credential Manager):

    `git clone https://github.com/microsoft/ProjectAirSim.git`

7. Do the **[Project AirSim Client Setup](../client_setup.md#setting-up-the-client-on-windows)**.

Now you're ready to start **[Developing Project AirSim Sim Libs](use_source.md#developing-projectairsim-libs)**

---

Copyright (C) Microsoft Corporation.  All rights reserved.
