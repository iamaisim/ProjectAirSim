# Using a PX4 Controller as Hardware-In-The-Loop (HITL)

In HITL, the PX4 flight controller software is running on a dedicated hardware controller board (frequently the same as would be used in a physical remote control (RC) vehicle) connected via USB to the PC running Project AirSim.

## Supported airframes

When using PX4 HITL, Project AirSim only supports the Quadrotor airframe which corresponds to PX4's "HIL Quadcopter X" airframe.  The VTOL Fixed-Wing Tailsitter airframe is not supported in simulation by PX4 HITL (but is supported by [PX4 SITL](px4_sitl.md)).

## Supported hardware

Project AirSim has been tested with the following devices:

* Pixhawk 1 (2.4.6)
* Pixhawk 4 mini from Holybro

## Setting up PX4 Hardware-In-The-Loop

You must have one of the supported devices listed above. For manual flight simulation, you will also need a ground-based RC radio unit (aka. *RC transmitter*) and a vehicle-based RC radio unit (aka. *RC receiver*).

1. For manual flight simulation:
    1. Make sure your RC receiver and RC transmitter are bound to each other.
    2. Connect the receiver (the vehicle-based radio unit) to the PX4 flight controller's RC port.

    Refer to your RC manual and [PX4 docs](https://docs.px4.io/en/getting_started/rc_transmitter_receiver.html) for more information.

2. Do not plug the PX4 flight controller device into your computer yet.
3. Download and install [QGroundControl](http://qgroundcontrol.com/).
4. Launch *QGroundControl* and go to the Vehicle Firmware Setup panel (click on the Ground Control logo, select the "Vehicle Setup" tool, then "Firmware" tab.)
5. Connect the PX4 flight controller device to a USB port on your computer.
6. *QGroundControl* should detect the PX4 hardware and offer to setup the firmware on the device.  Select the "PX4 Pro Stable Release vx.xx.x" flight stack.
See also the [initial firmware setup video](https://docs.px4.io/master/en/config/).
7. In *QGroundControl*, configure your PX4 flight controller device for HIL simulation by selecting under "Simulation (Copter)" the "HIL Quadrocopter X" airframe, then click "Apply and Restart" at the top.  After the PX4 device reboots and *QGroundControl* reconnects to it, verify that "HIL Quadrocopter X" is still selected.

For manual flight simulation, perform these additional steps:

8. In *QGroundControl*, go to Radio tab and calibrate (make sure the transmitter is on and that the receiver indicates it is bound to it).
9. Go to the Flight Mode tab and chose one of the remote control switches as the "Mode Channel". Then set (for example) Stabilized and Attitude flight modes for two positions of the switch.
10. Go to the Tuning section of *QGroundControl* and set appropriate values. For example, for Fly Sky's FS-TH9X remote control, the following settings give a more realistic feel:

    Setting | Value
    ------- | -----
    Hover throttle | mid+1 mark
    Roll and pitch sensitivity | mid-3 mark
    Altitude and position control sensitivity | mid-2 mark

11. In the [robot configuration file](/config_robot.md), specify PX4 for your vehicle controller like this:
```jsonc
    {
        ...
        "controller": {
            "id": "PX4_Controller",
            "type": "px4-api",
            "px4-settings": {
                "use-serial": true,
                "serial-port": "serial device name",
                "qgc-host-ip": "127.0.0.1",
                "qgc-port": 14550,
                "parameters": {
                    "NAV_RCL_ACT": 0,
                    "NAV_DLL_ACT": 0,
                    "COM_OBL_ACT": 1,
                    "LPE_LAT": 47.641468,
                    "LPE_LON": -122.140165
                }
            }
        },
        ...
    }
```

The "use-serial" value indicates that Project AirSim will find the PX4 device on a serial port with the device name specified by "serial-port".  The "qgc-host-ip" and "qgc-port" configure Project AirSim to communicate with *QGroundControl*.  The "parameters" values configure PX4 to set the vehicle's local origin and set failsafe actions appropriate for Project AirSim.

You'll need to replace the "serial-port" entry's value of "serial device name" with the actual device name of the serial port to the PX4 device.
* On Windows the device name will be of the form "COM*x*" such as "COM10" which you can determine by using the Device Manager and locating the PX4 device under "Ports (COM & LPT)".
* On Linux the device name will be of the form "/dev/tty*x*"; for example, "/dev/ttyS1", "/dev/ttyUSB2", or "/dev/ttyACM3".  If the PX4 device is connected by USB, the "lsusb" command may help determine the correct device name.

After completing the above setup you should now be able to use *QGroundControl* to fly the vehicle in Project AirSim.  If an RC transmitter and receiver are connected and configured through the additional setup steps for manual flight simulation, you can manually fly using the RC transmitter. With manual flight you don't need *QGroundControl* after the initial setup. You can usually arm the vehicle by lowering and bringing the two sticks of the RC transmitter together down and inwards. Typically the Stabilized flight mode (instead of Manual) gives a better experience for beginners.  For more information see the [PX4 Basic Flying Guide](https://docs.px4.io/master/en/flying/basic_flying.html).

You can also control the drone with a Python script using [the Python APIs](/apis.md).

---

Copyright (C) Microsoft Corporation. 
Copyright (c) 2025 IAMAI Simulations.

MIT License. All rights reserved.
