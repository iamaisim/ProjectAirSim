# Developer Initial Setup for Linux

On Linux, Project AirSim can be developed with VS Code which provides a light-weight, cross-platform common experience between Linux and Windows.

1. Clone the Project AirSim repo (**[SSH authentication](https://docs.microsoft.com/en-us/azure/devops/repos/git/use-ssh-keys-to-authenticate)** is recommended for Linux)

    ```
    git clone bizair@vs-ssh.visualstudio.com:v3/bizair/Project%20AirSim/projectairsim
    ```

2. From the `projectairsim/` folder, install the prerequisites:

    ```
    ./setup_linux_dev_tools.sh
    ```

    This will install:

    - **make** - used by build scripts to drive CMake commands
    - **cmake** - used to build sim lib components
    - **clang 13, libc++ 13** - used to build sim lib components and match the bundled toolchain of Unreal Engine 4.25
    - **ninja** - used as the preferred CMake generator/build tool
    - **vulkan loader library** - for UE rendering on Linux (OpenGL has been deprecated)
    - **vulkan utils** - utilities like `vulkaninfo` for checking for graphics support

    You may also need to install the latest drivers for your GPU to support the Vulkan interface.

3. Install **[VS Code](https://code.visualstudio.com/)** and the following extensions:

    *Note: If not manually installed, these extensions will be recommended for automatic install on opening the Project AirSim project workspace*

    - **[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)**
    - **[C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)** (includes clang-format functionality, use **Alt-Shift-F** to auto-format the current code file using the repo's .clang-format settings)
    - **[CodeLLDB](https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb)**
    - **[C#](https://marketplace.visualstudio.com/items?itemName=ms-dotnettools.csharp)**
    - **[Mono Debug](https://marketplace.visualstudio.com/items?itemName=ms-vscode.mono-debug)**
    - **[C++ TestMate](https://marketplace.visualstudio.com/items?itemName=matepek.vscode-catch2-test-adapter)** (also automatically installs **[Test Explorer UI](https://marketplace.visualstudio.com/items?itemName=hbenl.vscode-test-explorer)**)
    - **[Python](https://marketplace.visualstudio.com/items?itemName=ms-python.python)**
    - **[Pylance](https://marketplace.visualstudio.com/items?itemName=ms-python.vscode-pylance)**

    The above can also be done by the following bash command:
    ```
    code --install-extension ms-vscode.cmake-tools
    code --install-extension ms-vscode.cpptools
    code --install-extension vadimcn.vscode-lldb
    code --install-extension ms-dotnettools.csharp
    code --install-extension ms-vscode.mono-debug
    code --install-extension matepek.vscode-catch2-test-adapter
    code --install-extension ms-python.python
    code --install-extension ms-python.vscode-pylance
    ```

4. *(Optional)* Customize VS Code User settings

    See **[Optional VS Code User Settings](vscode_user_settings.md)** for some example customized user settings that can help with Project AirSim development.

5. Install Unreal Engine 4.25

    - Get UE source from **[Unreal Engine's private GitHub repo](https://github.com/EpicGames/UnrealEngine)** (requires **[registering with Epic](https://docs.unrealengine.com/en-US/GettingStarted/DownloadingUnrealEngine/index.html)**)

    - You can run the engine as built from source following **[Unreal's Linux native build process](https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Build/BatchFiles/Linux/README.md)**, but to prevent the engine from recompiling itself during Project AirSim development, **it is recommended to make an [installed build](https://docs.unrealengine.com/en-US/Programming/Deployment/UsinganInstalledBuild/index.html) of the engine instead**. To make an installed build, run these steps:

        ```
        cd <UE source path>

        ./Setup.sh

        rm -rf Engine/Platforms/XXX

        ./Engine/Build/BatchFiles/RunUAT.sh BuildGraph \
            -target="Make Installed Build Linux" \
            -script=Engine/Build/InstalledEngineBuild.xml \
            -set:HostPlatformOnly=true \
            -set:WithLinuxAArch64=false \
            -set:WithFullDebugInfo=false \
            -set:WithDDC=true \
            -set:GameConfigurations="DebugGame;Development"
        ```

    **Note:** Making an installed build will take a **lot of disk space (~200 GB)** and a **lot of time (~4+ hours)** because it will build the engine and precompile the engine's **[DDC](https://docs.unrealengine.com/en-US/Engine/Basics/DerivedDataCache/index.html)** content. However, using the installed build for Project AirSim development can save a lot of developer iteration time since the engine will never recompile and the bulk of the initial shader compilation will already be complete. This is similar to using a Windows binary engine downloaded from Epic's Launcher. Most of the original source package and build files can also be deleted to just keep the installed build version which is ~40 GB.

    - After the installed build completes, the binary version of the engine will be located in `<UE source path>/LocalBuilds/Engine/Linux/` and the contents of this folder can be moved to wherever you plan to use the engine from. The other contents of the `<UE source path>` can be deleted since the installed build engine will not need to rebuild again.

        ```
        mv ./LocalBuilds/Engine/Linux ~/UE_installed_build
        cd ~/UE_installed_build
        rm -rf <UE source path>
        ```

6. Set a `UE_ROOT` environment variable to wherever you put the built Unreal Engine (you can add this to `~/.bashrc` to persist)

    ```
    export UE_ROOT=/path/to/UE_installed_build
    ```

    **Note:** When using the installed build engine for VS Code debugging, it will look for the source files in the original `<UE source path>` location that it was built from, so you can either put the installed build engine in place of the original source package, or you can add a source mapping configuration to the VS Code project's `launch.json` after the project is generated, like this:

    ```
    "sourceFileMap": { "<UE source path>": "${env:UE_ROOT}" }
    ```

7. Do the **[Project AirSim Client Setup](../client_setup.md#setting-up-the-client-on-linux)**.

Now you're ready to start **[Developing Project AirSim Libs](use_source.md#developing-projectairsim-libs)**

---

Copyright (C) Microsoft Corporation. 
Copyright (c) 2025 IAMAI Simulations.

MIT License. All rights reserved.