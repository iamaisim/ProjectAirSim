# PX4 Lockstep Mode

Recent versions of PX4 support a new [lockstep feature](https://docs.px4.io/master/en/simulation/#lockstep-simulation) when communicating with a simulator over TCP.  `Lockstep` decouples the internal clocks of PX4 and the simulator from real-time and synchronizes them to each other so that both internal clocks advance together.  This allows PX4 to behave normally even during unusually long delays in simulator updates.

It is recommended that when you are running a `lockstep`-enabled version of PX4 in SITL mode that you specify the controller settings within the robot configuration of Project AirSim to set `lock-step` to `true` and set `use-tcp` to `true`.

```
{
        ...
        "controller": {
            "id": "PX4_Controller",
            "type": "px4-api",
            "px4-settings": {
                "lock-step": true,
                "use-tcp": true,
            ...
}
```

This configures Project AirSim to use a "non-realtime" clock that advances in step with each sensor update sent to PX4.  PX4 thinks time is progressing smoothly no matter how long
it takes Project AirSim to actually process each loop update.

This has the following advantages:

- Project AirSim can be used on slow machines that cannot update the simulation fast enough for real-time.
- You can debug Project AirSim, hit a breakpoint, and PX4 will behave normally after you resume.
- You can use very slow sensors (such as LIDAR with a large number of simulated points) and PX4 will still behave normally.

There are some side effects to `lockstep`, primarily slower update loops.  Running Project AirSim on an underpowered machine or with processing-heavy sensors (like LIDAR) will cause slow updates or show some visible jerkiness in the simulated flight when you watch the screen update in real-time.

---

Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.

MIT License. All rights reserved.
