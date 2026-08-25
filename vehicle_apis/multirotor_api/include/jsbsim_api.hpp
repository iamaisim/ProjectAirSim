// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_JSBSIM_API_HPP_
#define MULTIROTOR_API_INCLUDE_JSBSIM_API_HPP_

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "core_sim/actor/robot.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/service_method.hpp"
#include "core_sim/transforms/transform_tree.hpp"


namespace microsoft {
namespace projectairsim {

class JSBSimApiTestPeer;

// todo "firmware" / "firmware wrapper api" or "api" type (wrt px4 / mavlink)
//enum class MultirotorApiType { kJSBSim = 0 };

// JSBSimApi
// TODO: Should we use pimpl or some other pattern to hide the implementation?
class JSBSimApi : public IController {
 public:
  JSBSimApi() {}
  JSBSimApi(const Robot& robot, TransformTree* ptransformtree);

  ~JSBSimApi() override = default;

  //---------------------------------------------------------------------------
  // IController overrides

  void BeginUpdate() override;
  void EndUpdate() override;
  void Reset() override;
  void SetKinematics(const Kinematics* kinematics) override;
  void Update() override;
  std::vector<float> GetControlSignals(const std::string& actuator_id) override;
  const GimbalState& GetGimbalSignal(const std::string& gimbal_id) override;

  float GetJSBSimProperty(const std::string& property);
  bool SetJSBSimProperty(const std::string& property, float value);

 private:
  friend class JSBSimApiTestPeer;

  void LoadSettings(const Robot& robot);
  void RegisterServiceMethods();
  float GetJSBSimPropertyUnlocked(const std::string& property);
  bool SetJSBSimPropertyUnlocked(const std::string& property, float value);

 private:
  Robot sim_robot_;
  std::unordered_map<std::string, int> actuator_id_to_output_idx_map_;
  std::mutex jsbsim_property_mutex_;
  GimbalState gimbal_state_{};
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_JSBSIM_API_HPP_
