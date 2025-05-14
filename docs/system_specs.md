# System Specifications

## Basic system specifications

Since Project AirSim uses Unreal Engine for rendering, the system specifications are mainly driven by Unreal's **[Hardware and Software Specifications](https://docs.unrealengine.com/en-US/GettingStarted/RecommendedSpecifications/index.html)**.

## Installing system prerequisites

### Windows prerequisites

Unreal binaries/projects require some system prerequisites such as DirectX End-user Runtime and Visual C++ libraries.

If you are using Unreal Editor to work with the simulation projects, installing the Unreal Engine itself will automatically install the required system prerequisites.

If you are just running packaged Unreal binaries, you can install the prerequisites using an installer program that should be included in the package folders at:

`{Binary Package Root Folder}\DebugGame\Windows\Engine\Extras\Redist\en-us\UEPrereqSetup_x64.exe`

### Linux prerequisites

For GPU rendering, Unreal requires:

1. GPU drivers that support the Vulkan interface
2. Vulkan run-time libraries

For Nvidia GPU drivers, you can confirm that they are working correctly by running:

```
nvidia-smi
```

For Ubuntu, the Vulkan libraries also need to be installed by running:

```
sudo apt update
sudo apt install libvulkan1 vulkan-utils
sudo reboot
```

If official/proprietary GPU drivers from Nvidia (or AMD) are not installed, OR if you want to use open source GPU drivers (like Nouveau on Linux), OR use an integrated GPU (like Intel), you may need to install the Mesa Vulkan driver package by running:

```
sudo apt install mesa-vulkan-drivers
```

To confirm that the Vulkan libraries are working correctly, you can run:
```
vullkaninfo
```

## Performance improvement tips

Some options to consider to help improve performance are:

- Reduce the rendering resolution
- Reduce visual quality (Low/Medium/High/Epic) using **[Scalability settings](https://docs.unrealengine.com/en-US/Engine/Performance/Scalability/ScalabilityReference/index.html)**
- Render offscreen
- Disable unused cameras/sensors in the config JSON files
- **[Optimize the Unreal environment](https://docs.unrealengine.com/en-US/Engine/Performance/index.html)**
- Disable Vsync (warning: may result in image tearing)

See **[Command Line Switches](command_line_switches.md)** for details about how to enable some of these options at runtime.

---

Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.

MIT License. All rights reserved.
