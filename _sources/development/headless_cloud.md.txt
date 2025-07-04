# Running Headless (Docker)

## Using headless mode with VS Code Debugging

To use the `-RenderOffScreen` or `-nullrhi` headless arguments for debugging in VS Code, edit the `launch.json` section for your debugging run target and add them to the `"args"` array:

```
{
    "name": "Blocks (DebugGame)",
    "request": "launch",
    ...
    "args": [ "-RenderOffScreen" ],
    ...
},
```

For an **Editor** target, you can also add the `-game` argument to automatically start the simulation Play on launch:

```
{
    "name": "BlocksEditor (DebugGame)",
    "request": "launch",
    ...
    "args": [
      "C:\\dev\\projectairsim\\unreal\\Blocks\\Blocks.uproject", "-game", "-RenderOffScreen"
    ],
    ...
},
```


## Using headless mode with Visual Studio 2019 Debugging

To use the `-RenderOffScreen` or `-nullrhi` headless arguments for debugging in Visual Studio 2019, right-click on the Blocks project in the Solution Explorer and open the **Properties** window, then add the arguments to `Configuration Properties -> Debugging -> Command Arguments`:

| DebugGame | |
| ----------------  | ------------------ |
| Command Arguments | `-RenderOffScreen` |

For an **Editor** target, you can also add the `-game` argument to automatically start the simulation Play on launch:

| DebugGame Editor | |
| ----------------- | --------------------------------------------------------------------- |
| Command Arguments | `"$(SolutionDir)Blocks.uproject" -skipcompile -game -RenderOffScreen` |

This can also be done by directly modifying the `Blocks/Intermediate/ProjectFiles/Blocks.vcxproj.user` file to add the arguments:

```
<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='DebugGame|x64'">
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
    <LocalDebuggerCommandArguments>-RenderOffScreen</LocalDebuggerCommandArguments>
</PropertyGroup>
```

```
<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='DebugGame_Editor|x64'">
    <LocalDebuggerCommandArguments>"$(SolutionDir)Blocks.uproject" -skipcompile -game -RenderOffScreen</LocalDebuggerCommandArguments>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
</PropertyGroup>
```

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI Consulting Corp.

MIT License. All rights reserved.
