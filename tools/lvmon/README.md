# LVMon Debugging Facility

The Live Value Monitoring (LVMon) debugging facility allows inspection of values while Project AirSim is running.

Project AirSim's simulation can be heavily affected by timing and using a breakpoint during debugging can cause bugs to manifest differently or not at all, or other issues to appear complicating the investigation.  Even Visual Studio action breakpoints, being somewhat slow, can affect Project AirSim's timing.  Debug print statements are faster but creates a deluge of output when printing multiple rapidly updating values or can be buried among other debug output.

LVMon allows you to easily monitor quickly changing values with low overhead.

## Supported Platforms
The LVMon protocol is platform-agnostic.  The LVMon server, the portion running in the Project AirSim process, is available in both Windows and Linux builds.  LVMonitor, the data viewing application, runs only on Windows.  (Nothing prohibits a data view application like LVMonitor for Linux but one is not currently available.)

## Concepts
LVMon is an asynchronous publish/subscribe messaging system similar to uORB or ROS Topics.  LVMon differs from those systems in these ways:
- LVMon is unidirectional and not a generalized bus.  Only the LVMon server publishes values.  Subscribers (clients) only receive values and cannot publish them.
- In LVMon the publish mechanism is very lightweight, consisting of a single function call passing a value name and the data.  No setup is needed.
- In LVMon only four data types are supported: Signed 64-bit integer, unsigned 64-bit integer, 64-bit floating-point value, and UTF-1 string.

In LVMon, a "value" is a tuple containing the name, data type, and data.

## Getting Started
The LVMon server is linked into the debug build of Project AirSim and automatically starts when Project AirSim is launched.  The LVMon server requires no setup.

### Publishing a Value
Do the following to publish a value:
- To make the LVMon include directory available to your code, add `lvmon` as a source link library in the associated `CMakeLists.txt`.  For instance, in the Project AirSim core_sim lib:
    - Edit `core_sim\src\CMakeLists.txt` and in the `target_link_libraries()` statement, add `lvmon` as a source library.
    - The core_sim lib also creates a test build so the same change must be made to `core_sim\src\test\CMakeLists.txt`.
- In the source file add the include statement `#include <LVMon/lvmon.h>`.
- To publish a value, make a call of the form `LVMon::Set(name, data)`.

LVMon::Set() is an overloaded function accepting one of the supported data types and infers the data type of the value from the overload that's invoked.  C++ data type conversion rules enable LVMon::Set() to be called with 32-bit floats and non-64-bit integers--they are automatically converted to the 64-bit types supported by LVMon.  Wide strings (16-bit or 32-bit characters) are not supported.

Note that the first call to LVMon::Set() with a particular name sets the data type for that name.  Subsequent calls to LVMon::Set() with the same name but different data type are ignored.

The `name` parameter can be any string literal to uniquely identify the value.  Using a hierarchical form such as "core_sim/sensors/imu/x" is recommended but LVMon does not require a particular format nor  does it interpret the string.

**Important**: The `name` parameter _must_ be a string literal and not a variable.  LVMon relies on the address of the name string literal being unique to accelerate lookup of the value.  Using a variable defeats this and will cause value updates to fail.

Example:

In `core_sim\src\CMakeLists.txt`, add `lvmon` as a source library:

    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
            gtest_main
            nng
            ws2_32  # req by nng on Win
            mswsock  # req by nng on Win
            advapi32  # req by nng on Win
            lvmon

In `core_sim\src\sensors\imu.cpp`, include the `lvmon.h` header and the call to `LVMon::Set()`:

    #include <LVMon/lvmon.h>
    ...
    LVMon::Set("core_sim/sensors/imu/orientation/noisy/x", output_.orientation.x());

**Note**: In release builds, the LVMon library is not linked and calls to LVMon::Set() resolve to null functions and have no effect.


### LVMonitor: Monitoring Values

The client program, `LVMonitor`, views the values published through LVMon.  To launch LVMonitor, run `LVMonitor.exe` from the `tools\LVMon\App\Bin` directory.  LVMonitor is currently supported on Windows only.

To connect to a LVMon server, select the `File → Connect To...` menu item.  In the "Connect to Live Value Monitor Server" dialog, select "TCP/IP" for the connection type and enter the name of the computer where Project AirSim is running in the "Host name or IP address" edit box and click "OK".  Note that if Project AirSim is not currently running LVMonitor will automatically wait and connect when Project AirSim launches.

The "Named Pipe" connection type is implemented by the LVMon server but currently disabled pending integration in the near future.

As a convenience, `File → Connect` will connect using the server and the connection type specified last.
#### View Windows

LVMonitor uses different "view windows" to display values.  Each type of view window displays values in a different form, and different view windows may display the same value.  By default, LVMonitor starts with a "Text View" window (titled "Watch List") which display values in a generalized textual form.  To add a value to the Text View display:
1. Ensure Project AirSim is running and LVMonitor is connected to it.
2. Right-click in the Text View window and select "Add Value...".
3. Select the values to add by click on the names to highlight them.
4. Click "OK".

The Text View window should update and display the current value of the added values.

Add additional view windows by selecting the `Window → Add View...` menu item.  Add values to those windows by right-clicking in them and selected "Add Value...", although the exact procedure may different depending on the type of view window.

#### Value Updates
View windows update values when a value is added to a view window and whenever an update is received from the LVMon server.  The `Data → Freeze` menu item pauses value updates (to allow extended viewing of rapidly changing values, for instance.)  Select `Data → Freeze` again to resume value updates.

#### View Window Management

View windows can be "floating" as windows independent of the main LVMonitor window or "docked" into the main window.  Dock or undock a view window using the down-arrow button menu.

To dock a window, select "Dock" from the menu and drag the window over the main window.  A "docking indicator" will appear showing the different locations where the window can be docked.  Drag the window to move the mouse over one of the location and release the mouse button to drop the window into the location.

When multiple windows are docked in the main window, narrow gray bars will appear in between the docked windows.  These dividers can be dragged to resize the docked windows.

#### Saving and Loading Settings

The window configuration and the values shown in the view windows can be saved to and loaded from setting files which have the extension ".lvmon":
- To save the current settings select the `File → Save Settings As...` menu item.
- To load settings, select the `File → Load Settings...` menu item.
- The `File → Save Settings` menu item saves the current settings to the last settings file loaded.

When closing the program or loading new settings after settings have changed, a message box will appear offering to save the current settings before proceeding.  Note that while the window positions and sizes are saved in the settings file, moving or resizing a window is not considered a settings change and will not trigger the prompt to save settings before continuing.  Manually save the current settings to ensure  window positions and sizes are saved.

## Additional Notes

- **WARNING**: The LVMon protocol is not encrypted nor secure--do not run LVMon on unsecured networks.  LVMon is a debugging tool and should not be used in release software.
- **WARNING**: The LVMonitor client may not see every value change.  While it will always get an update for the latest change in a value, changes before the latest may be skipped.  Because the LVMon server and client operate asynchronously and may be running on different computers, it's possible that the client cannot receive or process value changes faster than the server can send them.  In that case, the server will skip intermediate updates to prevent a backlog of updates.
- The LVMon server uses TCP/IP port 2433.

## License

License info TBD.

---

Copyright (C) Microsoft Corporation.  All rights reserved.
