# PX4 with Multiple Robots

The [PX4 SITL stack](px4_sitl.md) comes with a `sitl_multiple_run.sh` shell script that runs multiple instances of the PX4 binary. This  allows the SITL stack to listen to connections from multiple Project AirSim vehicles on multiple TCP ports starting from 4560.
However, the provided script does not let us view the PX4 console. If you want to run the instances manually while being able to view each instance's console (**recommended**) see [this section](px4_multi_vehicle.md#starting-sitl-instances-with-px4-console)

## Setting up multiple instances of PX4 Software-in-Loop

**Note**: You must build PX4 with `make px4_sitl_default none_iris` as shown [here](px4_sitl.md#setting-up-px4-software-in-loop) before trying to run multiple PX4 instances.

1. From your bash (or Cygwin) terminal go to the PX4 Firmware directory and run the `sitl_multiple_run.sh` script while specifying the number of vehicles you need:

        cd PX4-Autopilot
        ./Tools/sitl_multiple_run.sh 2    # 2 here is the number of vehicles/instances

    This starts multiple instances that listen to TCP ports 4560 through 4560+<i>i</i>-1 where <i>i</i> is the number of vehicles/instances specified.

2. You should get a confirmation message that says that old instances have been stopped and new instances have been started:

        killing running instances
        starting instance 0 in /cygdrive/c/PX4/home/PX4/Firmware/build/px4_sitl_default/instance_0
        starting instance 1 in /cygdrive/c/PX4/home/PX4/Firmware/build/px4_sitl_default/instance_1

3. Now edit each [robot config](../../config_robot.md) file to make sure you have matching TCP port settings and to make sure that both vehicles do not spawn on the same point.

    For example, these settings would spawn two PX4Multirotors where one of them would try to connect to PX4 SITL at port `4560` and the other at port `4561`.


    `robot1.jsonc`:

        {
            ...
            "controller": {
                "id": "PX4_Controller",
                "type": "px4-api",
                "px4-settings": {
                    "lock-step": true,
                    "use-tcp": true,
                    "tcp-port": 4560,
                    "control-ip-address": "127.0.0.1",
                    "control-port": 14540,
                    "parameters": {
                        "NAV_RCL_ACT": 0,
                        "NAV_DLL_ACT": 0,
                        "COM_OBL_ACT": 1,
                    }
                }
            },
            ...
        }

    `robot2.jsonc`:

        {
            ...
            "controller": {
                "id": "PX4_Controller",
                "type": "px4-api",
                "px4-settings": {
                    "lock-step": true,
                    "use-tcp": true,
                    "tcp-port": 4561,
                    "control-ip-address": "127.0.0.1",
                    "control-port": 14541,
                    "parameters": {
                        "NAV_RCL_ACT": 0,
                        "NAV_DLL_ACT": 0,
                        "COM_OBL_ACT": 1,
                    }
                }
            },
            ...
        }

    You can add more than two vehicles but you will need to make sure you adjust the TCP port for each (ie: vehicle 3's port would be `4562` and so on..) and adjust the spawn point.

4. Now run your client script loading the simulation scene and Project AirSim should connect to PX4 SITL instances via TCP.  If you are running the instances with the [PX4 console visible](px4_multi_vehicle.md#Starting-sitl-instances-with-px4-console), you should see a bunch of messages from each PX4 SITL window.  Specifically, the following messages tell you that AirSim is connected properly and GPS fusion is stable:

        INFO  [simulator] Simulator connected on UDP port 14560
        INFO  [mavlink] partner IP: 127.0.0.1
        INFO  [ecl/EKF] EKF GPS checks passed (WGS-84 origin set)
        INFO  [ecl/EKF] EKF commencing GPS fusion

    If you do not see these messages then check your port settings.

5. You should also be able to use QGroundControl with SITL mode.  Make sure there is no Pixhawk hardware plugged in, otherwise QGroundControl will connect to that instead.  Note that as we don't have a physical board, a remote control cannot be connected directly to it. So the alternatives are either use XBox 360 Controller or connect your remote control using USB (for example, in case of FrSky Taranis X9D Plus) or  a trainer USB cable to your PC. This makes your remote control look like a joystick. You will need to do extra set up in QGroundControl to use a virtual joystick for RC control.  You do not need to do this unless you plan to fly a drone manually in Project AirSim.  Autonomous flight using the Python API does not require RC, see [`no remote control`](px4_sitl.md#No-Remote-Control) in [Using a PX4 Controller as Software-In-The-Loop (SITL)](px4_sitl.md).

## Starting SITL instances with PX4 console

If you want to start your SITL instances while being able to view the PX4 console, you will need to run the shell scripts found [here](https://github.com/microsoft/AirSim/tree/master/PX4Scripts) rather than `sitl_multiple_run.sh`.
Here is how you would do so:

**Note**: This script also assumes PX4 is built with `make px4_sitl_default none_iris` as shown [here](px4_sitl.md#setting-up-px4-software-in-loop) before trying to run multiple PX4 instances.

1. From your bash (or Cygwin) terminal go to the PX4 directory and get the scripts (place them in a subdirectory called Scripts win the PX4 directory as shown)

        cd PX4
        mkdir -p Scripts
        cd Scripts
        wget https://github.com/microsoft/AirSim/raw/master/PX4Scripts/sitl_kill.sh
        wget https://github.com/microsoft/AirSim/raw/master/PX4Scripts/run_airsim_sitl.sh

    **Note** the shell scripts expect the `Scripts` and `Firmware` directories to be within the same parent directory. Also, you may need to make the scripts executable by running `chmod +x sitl_kill.sh` and `chmod +x run_airsim_sitl.sh`.
2. Run the `sitl_kill.sh` script to kill all active PX4 SITL instances

        ./sitl_kill.sh

3. Run the `run_airsim_sitl.sh` script while specifying which instance you would like to run in the current terminal window (the first instance would be numbered 0)

        ./run_airsim_sitl.sh 0 # first instance = 0


    You should see the PX4 instance starting and waiting for AirSim's connection as it would with a single instance.

        ______  __   __    ___
        | ___ \ \ \ / /   /   |
        | |_/ /  \ V /   / /| |
        |  __/   /   \  / /_| |
        | |     / /^\ \ \___  |
        \_|     \/   \/     |_/

        px4 starting.
        INFO  [px4] Calling startup script: /bin/sh /cygdrive/c/PX4/home/PX4/Firmware/etc/init.d-posix/rcS 0
        INFO  [dataman] Unknown restart, data manager file './dataman' size is 11798680 bytes
        INFO  [simulator] Waiting for simulator to connect on TCP port 4560


4. Open a new terminal and go to the Scripts directory and start the next instance

        cd PX4
        cd Scripts
        ./run_airsim_sitl.sh 1  # ,2,3,4,..,etc


5. Repeat step 4 for as many instances as you would like to start

6. Run your client script to load the simulation scene and Project AirSim should connect to PX4 SITL via TCP (assuming your robot config files have the correct ports.)

---

Copyright (C) Microsoft Corporation.  
Copyright (C) 2025 IAMAI Consulting Corp.

MIT License. All rights reserved.
