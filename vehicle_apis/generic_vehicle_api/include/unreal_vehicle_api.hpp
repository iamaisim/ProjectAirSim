// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#ifndef GENERIC_VEHICLE_API_INCLUDE_UNREAL_VEHICLE_API_HPP_
#define GENERIC_VEHICLE_API_INCLUDE_UNREAL_VEHICLE_API_HPP_

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "core_sim/actor/robot.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/service_method.hpp"

namespace microsoft {
namespace projectairsim {

// Controller for unreal vehicle physics. This controller does not compute
// any control signals itself. It exposes SetParameter service methods so
// clients can send named control parameters directly to the Unreal vehicle
// actor that manages its own physics.
class UnrealVehicleApi : public IController {
 public:
  explicit UnrealVehicleApi(const Robot& robot);
  virtual ~UnrealVehicleApi() = default;

  // IController overrides
  void BeginUpdate() override;
  void EndUpdate() override;
  void Reset() override;
  void SetKinematics(const Kinematics* kinematics) override;
  void Update() override;
  std::vector<float> GetControlSignals(
      const std::string& actuator_id) override;
  const GimbalState& GetGimbalSignal(const std::string& gimbal_id) override;

  // Actuator signal accessors (thread-safe)
  void SetActuatorSignal(const std::string& name, float signal);
  float GetActuatorSignal(const std::string& name) const;
  std::unordered_map<std::string, float> GetAllActuatorSignals() const;
  std::vector<float> GetOrderedSignals() const;

 protected:
  Logger GetLogger() { return sim_robot_.GetLogger(); }

 private:
  void RegisterServiceMethods();

  // Service method handlers
  bool SetParameter(const std::string& name, float value);
  bool SetActuator(const std::string& name, float signal);

  Robot sim_robot_;
  const Kinematics* kinematics_ = nullptr;
  mutable std::mutex actuator_mutex_;
  std::unordered_map<std::string, float> actuator_signals_;
  std::vector<std::string> actuator_order_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // GENERIC_VEHICLE_API_INCLUDE_UNREAL_VEHICLE_API_HPP_
