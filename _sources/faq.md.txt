# Frequently Asked Questions (FAQ)

## General

### What vehicle types are supported?

Currently, only quadrotor, hexarotor, VTOL fixed-wing quad-x tailsitter, and VTOL fixed-wing quad tiltrotor aircraft are supported.

## Troubleshooting

### Something went wrong in my simulation. How do I get log files to help see what happened?

The Python client saves a log file in the same folder as the client script that's run:

`<folder of client script>\projectairsim_client.log`

To add log output lines in your script, use the `projectairsim_log()` logger:

```python
from projectairsim.utils import projectairsim_log

projectairsim_log().info(f"Message to log: {my_var}")
projectairsim_log().error("Error to log with stack trace", exc_info=True)
projectairsim_log().exception("Another way to log an error with stack trace")
```

For the simulation server, a log file is saved in the UE project folder:

`<{UE project folder}>\projectairsim_server.log`

For `DebugGame` and `Development` Unreal builds, the Unreal native log file will also be saved to a `Saved\Logs` subfolder:

`DebugGame\WindowsNoEditor\Blocks\Saved\Logs\Blocks.log`

On Linux, this file has the same contents as what is output to the stdout console. For Windows, if you want to see the log file contents on the stdout console as well, you can run the environment with the `-log` switch:

`Blocks.exe -log`

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI Consulting Corp.

MIT License. All rights reserved.
