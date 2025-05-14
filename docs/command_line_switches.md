# Command Line Switches

{# include enable_internal_docs.tpp #}
Project AirSim supports the following command line switches.  Since Project AirSim environments are built as Unreal executables, there are some Unreal command line switch options that can be useful:

| Switch | Description |
| ------ | ----------- |
| <code>&#x2011;RenderOffScreen</code> | Enable headless rendering (same for both Windows and Linux) |
| <code>&#x2011;nullrhi</code> | Disable rendering completely |
| <code>&#x2011;log</code> | For Windows, this switch will launch an additional command prompt that displays the log output as stdout (default behavior in Linux) |
| <code>&#x2011;vulkan</code> | Force Vulkan rendering instead of DirectX for Windows (Vulkan is already default for Linux)
| <code>&#x2011;ResX=</code>640 <code>&#x2011;ResY=</code>480 | Set the main viewport rendering resolution (ex. 640x480) |
| <code>&#x2011;windowed</code> | Run in windowed mode |
| <code>&#x2011;fullscreen</code> | Run in full-screen mode |
| <code>&#x2011;NoVSync</code> | Disable VSync to prevent capping FPS at the monitor's refresh rate |
| <code>&#x2011;nosound</code> | Disable any sound output |
{# ifdef INTERNAL_DOCS #}
| <code>&#x2011;benchmark</code> | Enabled Unreal's benchmark mode that seems to remove any sleeps in between rendering/tick loops to run without limiting to any real-time execution rate FPS target (this mode needs further testing to confirm any side-effects and how it could be used properly) |
{# endif INTERNAL_DOCS #}
| <code>&#x2011;gltfDir=</code><i>dir/containing/tiles/</i> | If you'll be using a GIS scene, you can specify the directory to read the tiles from. Alternatively, if you're trying CesiumForUnreal you can provide the root tileset json file. |
| <code>&#x2011;clientauthpubkey=</code><i>public_key</i> | Specify a client authorization public key to require clients to present a client authorization token before being allowed to use the client API.  Overrides the key set by the [PROJECTAIRSIM_CLIENT_AUTH_PUBKEY](#environment_variables) environment variable. |

The below simulation parameters can also be changed through the comand line:

| Switch | Value | Default | Description |
| ------ | ----- | ----- | ----- |
| <code>&#x2011;topicsport</code> | integer | 8989 | TCP port for pub-sub client connection (e.g. <code>&#x2011;topicsport=8989</code>)|
| <code>&#x2011;servicesport</code> | integer | 8990 | TCP port for req-resp client connection (e.g. <code>&#x2011;servicesport=8990</code>)|

**Note**: The port values must be different from one another.


# Environment Variables

Project AirSim supports the following environment variables:

| Environment Variable | Description |
| -------------------- | ----------- |
| <code>PROJECTAIRSIM_CLIENT_AUTH_PUBKEY</code> | Client authorization token public key

---

Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.

MIT License. All rights reserved.
