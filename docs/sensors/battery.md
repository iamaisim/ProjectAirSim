# Battery sensor

The battery sensor is implemented as a collection of APIs to help plan and execute mission planning and perception loops through estimation of state of charge and flight times. It is not meant to help with battery modeling.

## Battery usage

Battery information is available through the Pub/Sub channel. The following information is published as a part
of this channel.

1. Timestamp with simtime
2. Battery remaining (percentage from 0-100)
3. Estimated time remaining (expected battery time in seconds to empty)
4. Battery charge state (specifies if battery is ok, low, critical or unhealthy)


Example info published on the Pub/Sub channel
```
{
  'time_stamp': 39000000, 
  'battery_pct_remaining': 83.33333587646484, 
  'estimated_time_remaining': 7500, 
  'battery_charge_state': 'BATTERY_CHARGE_STATE_OK'
}
```

## Battery Simulation Modes
There are two different modes in the battery sensor each with its specific use cases.

1. Simple Discharge Mode:
    - Configurable maximum capacity through JSON
    - Configurable charge level at start of flight/simulation
    - Charge level depletes at the rate specified through discharge rate. 
    - Discharge rate adjustable through API calls and config.

This is useful for those who want complete control over the discharge rate. It is possible to set an intitial charge and adjust the discharge for various segments of flight. i.e. start with 0 discharge rate, increase to high for takeoff/landing, set it to a medium rate for landing.

Or you can simply set the discharge rate to the average of your observed value and let it deplete until you hit your minimum acceptable charge level.

### Sample Config for Simple Discharge Mode

```json
{
  "sensors": [
    {
      "id": "Battery",
      "type": "battery",
      "enabled": true,
      "parent-link": "Frame",

      "battery-mode":"simple-discharge-mode",

      "total-battery-capacity": 36000,
      "battery-capacity-on-start": 30000,
      // The drain rate can be adjusted at runtime using client api calls.
      "battery-drain-rate-on-start": 1
    }
  ]
}
```

2. Energy Consumption Model:

    - Configurable max and initial capacity like the simple discharge mode. 
    - Estimates discharge rate through a physics model for energy consumption of brushless motors onboard.
    - Discharge rate adjustable to a linear scaling factor.

This model uses a simplified energy equation to estimate energy consumption of brushless motors. It depletes power proportional to number of rotors x torque x angular_velocity.

```
Power Consumed = rotor-power-coefficient x number_of_rotors x torque x ang_vel + constant
```
 This can be scaled using a linear coefficient in the config referred to as "rotor-power-coefficient"  to match real usage. The constant accounts for any other onboard sensor/compute power usage. This can be used as a tool to estimate energy efficient routes during mission planning.


### Sample config for Energy Consumption Mode
```json
{
  "sensors": [
    {
      "id": "Battery",
      "type": "battery",
      "enabled": true,
      "parent-link": "Frame",

      "battery-mode": "rotor-power-discharge-mode",

      // Use joules as unit for capacity.
      //  1 Amp hour = 3600 * Joules, 1 milli Amp hour = 3.6 Joules
      "total-battery-capacity": 36000,
      "battery-capacity-on-start": 30000,
      // Use this to scale linearly
      "rotor-power-coefficient": 1
    }
  ]
}
```

# Battery sensor settings

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `id` | string | Name of battery sensor. |
| `type` | `battery` | Sensor type specifying this as a battery sensor. |
| `enabled` | bool | Whether sensor is enabled. |
| `parent-link` | string | Name of the link that the sensor is attached to. |
| `battery-mode` | string | Mode of battery simulator. "rotor-power-discharge-mode" or  "simple-discharge-mode" |
| `total-battery-capacity` | float | Total capacity of the battery aka Max charge it can hold. For rotor power discharge, the units need to be in joules.|
|`battery-capacity-on-start`| float | Capacity/charge of the battery at the beginning of flight/simulation.Needs to the same unit as total capacity |
|`battery-drain-rate-on-start`| float | Required for simple-discharge-mode. The rate at which the battery depletes per second at the beginning of flight. Units needs to match with capacity.  Modifiable during runtime.|
| `rotor-power-coefficient`| float| Required for rotor-power-discharge-mode. Used to scale the rotor-power estimates by a constant, default is one.|

---
Copyright (C) Microsoft Corporation. 
Copyright (c) 2025 IAMAI Simulations.

MIT License. All rights reserved.
