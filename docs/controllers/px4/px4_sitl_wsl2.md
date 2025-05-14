# PX4 Software-in-the-Loop with WSL 2

The [Windows Subsystem for Linux version 2](https://docs.microsoft.com/en-us/windows/wsl/install-win10) uses a virtual machine with its own IP address separate from that of your Windows host machine. This means PX4 will not find Project AirSim on the "localhost" which is the default behavior for PX4.

You will notice that the Windows command `ipconfig` returns a new Ethernet adapter for WSL like this (notice that the Ethernet adapter has `(WSL)` in the name):

```plain
Ethernet adapter vEthernet (WSL):

   Connection-specific DNS Suffix  . :
   Link-local IPv6 Address . . . . . : aaaa::bbbb:cccc:cccc:eeee%ff
   IPv4 Address. . . . . . . . . . . : www.xxx.yyy.zzz
   Subnet Mask . . . . . . . . . . . : mmm.nnn.ooo.ppp
   Default Gateway . . . . . . . . . :
```

`www.xxx.yyy.zzz` is the address of the Windows host machine on the virtual network also connected to WSL 2, and WSL 2 can reach your Windows host machine through this address.

**Note**:  If Windows has been rebooted this virtual Ethernet adapter may not be created until the first run of WSL 2 terminal and the IP address may change.

Beginning with this [PX4 change request](https://github.com/PX4/PX4-Autopilot/commit/1719ff9892f3c3d034f2b44e94d15527ab09cec6) (which correlates to version v1.12.0-beta1 or newer) PX4 in SITL mode can now connect to Project AirSim at a remote IP address.  To use this feature, make sure you have a version of PX4 containing this fix and set the following environment variable in the WSL 2 Linux instance running  PX4:

```bash
export PX4_SIM_HOST_ADDR=www.xxx.yyy.zzz
```

**Note:** Be sure to change `www.xxx.yyy.zzz` to match the IP address reported by the `ipconfig` command.

In your network firewall configuration, allow incoming TCP/IP port 4560 and incoming UDP/IP port 14540.

Edit your [robot config](../../config_robot.md#px4_settings) file and add the `local-host-ip` setting to tell Project AirSim to use the WSL ethernet adapter address instead of the default `localhost`.  Project AirSim will then listen on a TCP port on that adapter through which PX4 will connect.  Set `control-ip-address` to the special value `remote` so that Project AirSim will automatically connect the API/Offboard communication channel to PX4 at the WSL 2 remote IP address obtained when PX4 connects via the TCP/IP socket.

``` json
"px4-settings": {
  "lock-step" : true,
  "use-serial": false,
  "use-tcp": true,
  "tcp-port": 4560,
  "local-host-ip": "www.xxx.yyy.zzz",
  "control-ip-address": "remote",
  "control-port-remote": 14580,
  "qgc-host-ip": "", //Set only when enabling GCS proxy
  "qgc-port": 14550, //Set only when enabling GCS proxy
  "parameters": {
    "NAV_RCL_ACT": 0,
    "NAV_DLL_ACT": 0,
    "COM_OBL_ACT": 1,
    "LPE_LAT": 47.641468,
    "LPE_LON": -122.140165
  },
  ...
}
```

Usually Lockstep mode is enabled for PX4 SITL.  See [PX4 Lockstep Mode](px4_lockstep.md) for more information.

If your local repo does not include [this PX4 commit](https://github.com/PX4/PX4-Autopilot/commit/292a66ce417c9769e1a7845fbc9b8d5e68e1cf0b), please edit the Linux file in `ROMFS/px4fmu_common/init.d-posix/rcS`.  Make sure it is looking for the `PX4_SIM_HOST_ADDR` environment variable and passing that through to the PX4 simulator like this:

```bash
# If PX4_SIM_HOST_ADDR environment variable is empty use localhost.
if [ -z "${PX4_SIM_HOST_ADDR}" ]; then
    echo "PX4 SIM HOST: localhost"
    simulator start -c $simulator_tcp_port
else
    echo "PX4 SIM HOST: $PX4_SIM_HOST_ADDR"
    simulator start -t $PX4_SIM_HOST_ADDR $simulator_tcp_port
fi
```

**Note:** This code might already be there depending on the version of PX4 you are using.

**Note:** Please be patient waiting for the message:

```
INFO  [simulator] Simulator connected on TCP port 4560.
```

It can take a little longer to establish the remote connection than it does when PX4 and Project AirSim connect via `localhost`.

To complete setting up PX4 SITL, perform the steps given in [Setting up PX4 Software-in-Loop](px4_sitl.md#setting_up_px4_software_in_the_loop).


## Using a ground control station

Normally when using ground control station software with PX4 (like [*QGroundControl*](http://qgroundcontrol.com/)) the ground control station software will connect to PX4 directly.  In this configuration, ground control station software running in Windows can't connnect to PX4 directly since PX4 is running in WSL 2.  To connect the ground control station software to PX4, enable Project AirSim's ground control proxy by setting `qgc-host-ip` to `127.0.0.1`.  The ground control station software can then communicate with PX4 via Project AirSim.

---

Copyright (C) Microsoft Corporation. 
Copyright (c) 2025 IAMAI Simulations.

MIT License. All rights reserved.