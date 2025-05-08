// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_MATLAB_CONTROLLER_API_HPP_
#define MULTIROTOR_API_INCLUDE_MATLAB_CONTROLLER_API_HPP_

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "core_sim/actor/robot.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/sensors/airspeed.hpp"
#include "core_sim/sensors/barometer.hpp"
#include "core_sim/sensors/distance_sensor.hpp"
#include "core_sim/sensors/gps.hpp"
#include "core_sim/sensors/imu.hpp"
#include "core_sim/sensors/magnetometer.hpp"
#include "core_sim/service_method.hpp"
#include "nng/nng.h"

namespace microsoft {
namespace projectairsim {

// MatlabControllerApi
class MatlabControllerApi : public IController {
 public:
  MatlabControllerApi() {}
  MatlabControllerApi(const Robot& robot);
  virtual ~MatlabControllerApi() {}

  //---------------------------------------------------------------------------
  // IController overrides

  void BeginUpdate() override;
  void EndUpdate() override;
  void Reset() override;
  void SetKinematics(const Kinematics* kinematics) override;
  void Update() override;
  std::vector<float> GetControlSignals(const std::string& actuator_id) override;
  const IController::GimbalState& GetGimbalSignal(
      const std::string& gimbal_id) override;

 protected:
  Logger GetLogger() { return sim_robot_.GetLogger(); }

  void LoadSettings(const Robot& robot);

 private:
   void ConnectToMatlab();
   void GetSensors(const Robot& robot);

  Robot sim_robot_;  // The robot we control
  std::mutex update_lock_;
  std::vector<float> motor_output_;
  std::unordered_map<std::string, int> actuator_id_to_output_idx_map_;
  bool connected_ = false;
  std::atomic<bool> running_ = false;
  nng_socket nng_socket_ = NNG_SOCKET_INITIALIZER;
  std::string connection_string_;

  AirspeedSensor* airspeed_sensor_ = nullptr;
  Imu* imu_sensor_ = nullptr;
  Magnetometer* magnetometer_sensor_ = nullptr;
  Barometer* barometer_sensor_ = nullptr;
  DistanceSensor* distance_sensor_ = nullptr;
  Gps* gps_sensor_ = nullptr;
  const Kinematics* kinematics_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_MATLAB_CONTROLLER_API_HPP_
