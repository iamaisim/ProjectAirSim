// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARDSENSORS_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARDSENSORS_HPP_

namespace simple_flight {

class IBoardSensors {
 public:
  virtual void ReadAccel(float accel[3]) const = 0;  // accel in m/s^2
  virtual void ReadGyro(
      float gyro[3]) const = 0;  // angular velocity vector rad/sec

  virtual ~IBoardSensors() = default;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARDSENSORS_HPP_