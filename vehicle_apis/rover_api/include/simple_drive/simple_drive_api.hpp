// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef ROVER_API_INCLUDE_SIMPLE_DRIVE_API_HPP_
#define ROVER_API_INCLUDE_SIMPLE_DRIVE_API_HPP_

#include <memory>
#include <string>
#include <vector>

#include "ackermann_api_base.hpp"
#include "core_sim/actor/robot.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/service_method.hpp"
#include "core_sim/transforms/transform_tree.hpp"
#include "isimple_drive_controller.hpp"
#include "simple_drive/common.hpp"
#include "simple_drive/params.hpp"
#include "simple_drive/vehicle_state_estimator.hpp"

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

// Class for SimpleDriveApi which contains the necessary APIs for any wheeled
// ground robot, including a rover
class SimpleDriveApi : public AckermannApiBase {
 public:
  SimpleDriveApi(const Robot& robot, TransformTree* ptransformtree);

  virtual ~SimpleDriveApi() {}

 public:
  //---------------------------------------------------------------------------
  // RoverApiBase Method Overrides
  std::string GetControllerName(void) const override {
    return "SimpleDriveApi";
  }

  //---------------------------------------------------------------------------
  // IAckermannApi Method Overrides
  bool SetRoverControls(float engine, float steering_angle, float brake) override;

  //---------------------------------------------------------------------------
  // IRoverApi Method Overrides

  bool Arm(int64_t command_start_time_nanos) override;
  bool CanArm(void) const override;
  bool DisableApiControl(void) override;
  bool Disarm(void) override;
  bool EnableApiControl(void) override;
  bool IsApiControlEnabled(void) override;
  bool MoveToPosition(float x, float y, float velocity, float timeout_sec,
                      float yaw_rate_max, float lookahead,
                      float adaptive_lookahead,
                      int64_t command_start_time_nanos) override;
  bool MoveByHeading(float heading, float speed,
                     float duration, float heading_margin,
                     float yaw_rate, float timeout_sec,
                     int64_t command_start_time_nanos) override;

  //---------------------------------------------------------------------------
  // IController Method Overrides

  void BeginUpdate(void) override;
  void EndUpdate(void) override;
  void Reset(void) override;
  void SetKinematics(const Kinematics* kinematics) override;
  void Update(void) override;
  std::vector<float> GetControlSignals(const std::string& actuator_id) override;
  const IController::GimbalState& GetGimbalSignal(
      const std::string& gimbal_id) override;

 protected:
  enum class VehicleStateType {
    kUnknown,
    kInactive,
    kBeingArmed,
    kArmed,
    kActive,
    kBeingDisarmed,
    kDisarmed
  };

  // The kind of target vehicle
  enum class VehicleKind {
    Rover,
  };  // enum class VehicleKind

 protected:
  void LoadSettings(const Robot& robot);
  void OnTopicRCInput(const Topic& topic, const Message& message);

  //---------------------------------------------------------------------------
  // RoverApiBase Method Overrides
 protected:
  float GetCommandPeriod(void) const override;
  Kinematics GetKinematicsEstimated(void) const override;
  void LoadParams(
      const std::unordered_map<std::string, float>& params_map) override;

 protected:
  bool api_control_enabled_ = false;  // If true, control via API is enabled
  std::unordered_map<std::string, int> actuator_id_to_output_idx_map_;
  std::unordered_map<std::string, float> params_map_;
  std::shared_ptr<simple_drive::Params> pparams_ =
      std::make_shared<simple_drive::Params>();  // Controller parameters
  std::shared_ptr<Goals> pgoals_ =
      std::make_shared<Goals>();  // Goals for pisimple_drive_controller
  std::unique_ptr<ISimpleDriveController>
      pisimple_drive_controller_;  // Low-level vehicle controller
  std::shared_ptr<VehicleStateEstimator>
      pvehicle_state_estimator_;  // Vehicle kinematic state estimator
  std::vector<Topic> topics_;     // Topics to advertise or subscribe to
  VehicleKind vehicle_kind_ =
      VehicleKind::Rover;  // The type of vehicle being controlled
  VehicleStateType vehicle_state_ =
      VehicleStateType::kInactive;  // Overall vehicle control state
};                                  // class SimpleDriveApi

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_INCLUDE_SIMPLE_DRIVE_API_HPP_
