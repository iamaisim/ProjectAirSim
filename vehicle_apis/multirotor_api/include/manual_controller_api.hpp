// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_MANUAL_CONTROLLER_API_HPP_
#define MULTIROTOR_API_INCLUDE_MANUAL_CONTROLLER_API_HPP_

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "core_sim/actor/robot.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/service_method.hpp"

namespace microsoft {
namespace projectairsim {

// ManualControllerApi
class ManualControllerApi : public IController {
 public:
  ManualControllerApi() {}
  ManualControllerApi(const Robot& robot);
  virtual ~ManualControllerApi() {}

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

  //---------------------------------------------------------------------------
  // ManualController

  bool SetControlSignals(
      const std::unordered_map<std::string, float>& control_signal_map);

 protected:
  Logger GetLogger() { return sim_robot_.GetLogger(); }

  void LoadSettings(const Robot& robot);

  void RegisterServiceMethods();

 private:
  Robot sim_robot_;  // The robot we control
  std::mutex update_lock_;
  std::vector<float> motor_output_;
  std::unordered_map<std::string, int> actuator_id_to_output_idx_map_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_MANUAL_CONTROLLER_API_HPP_
