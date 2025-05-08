// Copyright (C) Microsoft Corporation. All rights reserved.

#include "simple_drive/simple_drive_api.hpp"

#include "json.hpp"
#include "simple_drive/ackermann_controller.hpp"
#include "simple_drive/utils.hpp"

#ifdef LVMON_REPORTING
#include <LVMon/lvmon.h>
#endif  // LVMON_REPORTING

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

// -----------------------------------------------------------------------------
// class SimpleDriveApi

SimpleDriveApi::SimpleDriveApi(const Robot& robot,
                               TransformTree* ptransformtree)
    : AckermannApiBase(robot, ptransformtree) {
  LoadSettings(robot);
}

float SimpleDriveApi::GetCommandPeriod(void) const {
  return 1.0f / 50;  // 50hz
}

void SimpleDriveApi::LoadSettings(const Robot& robot) {
  const std::string controller_settings = robot.GetControllerSettings();
  const json& controller_settings_json = json::parse(controller_settings);

  const std::string vehicle_setup =
      controller_settings_json.value("vehicle-setup", "");

  if (vehicle_setup == "ackermann") {
    int64_t wheel = 4;
    vehicle_kind_ = VehicleKind::Rover;
    pisimple_drive_controller_ = std::make_unique<AckermannController>(
        pparams_, SimClock::Get(), GetLogger());
  }

  // GetJsonObject
  const json& simple_drive_api_settings_json =
      controller_settings_json.value("simple-drive-api-settings", "{ }"_json);

  // Get SimpleDrive parameters and store in map
  const json& params_json =
      simple_drive_api_settings_json.value("parameters", "{ }"_json);
  for (json::const_iterator it = params_json.begin(); it != params_json.end();
       ++it) {
    params_map_[it.key()] = it.value();
  }

  // GetArray
  const json& actuator_order_json =
      simple_drive_api_settings_json.value("actuator-order", "[ ]"_json);

  try {
    int output_idx = 0;
    for (auto& actuator_json : actuator_order_json) {
      std::string id = actuator_json.value("id", "");
      actuator_id_to_output_idx_map_.insert({id, output_idx});
      output_idx++;
    }
  } catch (...) {
    throw;
  }
}

//---------------------------------------------------------------------------
// IController overrides

void SimpleDriveApi::BeginUpdate(void) {
  RegisterServiceMethods();

  LoadParams(params_map_);

  pvehicle_state_estimator_ = std::make_shared<VehicleStateEstimator>();

  pisimple_drive_controller_->Initialize(pgoals_, pvehicle_state_estimator_);
}

void SimpleDriveApi::EndUpdate(void) {
  pisimple_drive_controller_.reset();
  pvehicle_state_estimator_.reset();
}

void SimpleDriveApi::Reset(void) {
  api_control_enabled_ = false;
  if (pisimple_drive_controller_ != nullptr)
    pisimple_drive_controller_->Reset();
}

void SimpleDriveApi::LoadParams(
    const std::unordered_map<std::string, float>& params_map) {
  AckermannApiBase::LoadParams(params_map);
  if (!params_map_.empty()) {
    pparams_->Load(params_map_);
  }
}

void SimpleDriveApi::SetKinematics(const Kinematics* kinematics) {
  pvehicle_state_estimator_->SetGroundTruthKinematics(kinematics);
}

void SimpleDriveApi::Update(void) {
  if (pisimple_drive_controller_ != nullptr)
    pisimple_drive_controller_->Update();
}

std::vector<float> SimpleDriveApi::GetControlSignals(
    const std::string& actuator_id) {
  // auto actuator_map_itr = actuator_id_to_output_idx_map_.find(actuator_id);

  if ((vehicle_state_ != VehicleStateType::kArmed) ||
      (pisimple_drive_controller_ == nullptr))
    return std::vector<float>{0, 0, 0};
  else {
    static int kNumValueReturn =
        3;  // Number of values to return: throttle, steering, and brake

    std::vector<float> vecrRet;

    auto& vecr = pisimple_drive_controller_->GetOutput();
    auto cr = vecr.size();

    if (cr == kNumValueReturn)
      return (vecr);
    else if (cr < kNumValueReturn) {
      vecrRet = vecr;
      vecrRet.resize(kNumValueReturn);
    } else
      vecrRet.assign(vecr.begin(), vecr.begin() + kNumValueReturn);

    return vecrRet;
  }
}

const projectairsim::IController::GimbalState& SimpleDriveApi::GetGimbalSignal(
    const std::string& gimbal_id) {
  throw std::runtime_error(
      "This controller does not support following gimbal devices");
}

bool SimpleDriveApi::SetRoverControls(float engine, float steering_angle,
                                      float brake) {
  if (!api_control_enabled_) {
    GetLogger().LogError(GetControllerName(),
                         "Vehicle cannot be commanded via API because API has "
                         "not been given control");
    return false;
  }

  auto& goals = *pgoals_;

  goals[GoalTargetIndex::kX].mode = Goal::Mode::kNone;
  goals[GoalTargetIndex::kY].mode = Goal::Mode::kNone;
  goals[GoalTargetIndex::kZ].Set(Goal::Mode::kPassthrough, steering_angle);
  goals[GoalTargetIndex::kThrottle].Set(Goal::Mode::kPassthrough, engine);
  goals[GoalTargetIndex::kBrake].Set(Goal::Mode::kPassthrough, brake);

  return true;
}

//---------------------------------------------------------------------------
// IRoverApi overrides

bool SimpleDriveApi::EnableApiControl(void) {
  api_control_enabled_ = true;
  return true;
}

bool SimpleDriveApi::DisableApiControl(void) {
  api_control_enabled_ = false;
  return true;
}

bool SimpleDriveApi::IsApiControlEnabled(void) { return api_control_enabled_; }

bool SimpleDriveApi::Arm(int64_t /*command_start_time_nanos*/) {
  if (api_control_enabled_) {
    if (vehicle_state_ == VehicleStateType::kArmed) {
      GetLogger().LogError(GetControllerName(), "Vehicle is already armed");
      return true;
    } else if ((vehicle_state_ == VehicleStateType::kInactive ||
                vehicle_state_ == VehicleStateType::kDisarmed ||
                vehicle_state_ == VehicleStateType::kBeingDisarmed)) {
      vehicle_state_ = VehicleStateType::kArmed;
      GetLogger().LogTrace(GetControllerName(), "Vehicle is armed");
      return true;
    } else {
      GetLogger().LogError(GetControllerName(),
                           "Vehicle cannot be armed because it is not in "
                           "Inactive, Disarmed or BeingDisarmed state");
      return false;
    }
  } else {
    GetLogger().LogError(GetControllerName(),
                         "Vehicle cannot be armed via API because API has not "
                         "been given control");
    return false;
  }
}

bool SimpleDriveApi::Disarm(void) {
  if (api_control_enabled_ &&
      (vehicle_state_ == VehicleStateType::kActive ||
       vehicle_state_ == VehicleStateType::kArmed ||
       vehicle_state_ == VehicleStateType::kBeingArmed)) {
    vehicle_state_ = VehicleStateType::kDisarmed;

    GetLogger().LogTrace(GetControllerName(), "Vehicle is disarmed");
    return true;
  } else {
    GetLogger().LogError(GetControllerName(),
                         "Vehicle cannot be disarmed because it is not in "
                         "Active, Armed or BeingArmed state");
    return false;
  }
}

bool SimpleDriveApi::CanArm(void) const { return true; }

Kinematics SimpleDriveApi::GetKinematicsEstimated(void) const {
  return Utils::ToKinematicsState3r(
      pvehicle_state_estimator_->GetKinematicsEstimated());
}

bool SimpleDriveApi::MoveToPosition(float x, float y, float velocity,
                                    float timeout_sec, float yaw_rate_max,
                                    float /*lookahead*/,
                                    float /*adaptive_lookahead*/,
                                    int64_t command_start_time_nanos) {
  if (!api_control_enabled_) {
    GetLogger().LogError(GetControllerName(),
                         "Vehicle cannot be commanded via API because API has "
                         "not been given control");
    return false;
  }

  static const float kMeterToleranceSq = 10.0f;

  bool f_result;
  auto& goals = *pgoals_;
  float yaw_max_limit_sav;

  goals[GoalTargetIndex::kX].Set(Goal::Mode::kPositionWorld, x);
  goals[GoalTargetIndex::kY].Set(Goal::Mode::kPositionWorld, y);
  goals[GoalTargetIndex::kZ].mode =
      Goal::Mode::kNone;  // Enable automatic steering
  goals[GoalTargetIndex::kThrottle].Set(Goal::Mode::kVelocityWorld, velocity);
  goals[GoalTargetIndex::kBrake].Set(Goal::Mode::kPassthrough, 0);

  if (yaw_rate_max >= 0) {
    yaw_max_limit_sav = pparams_->angle_rate_pid.max_limit.Z();
    pparams_->angle_rate_pid.max_limit.Z() = yaw_rate_max;
  }

  f_result = RunTimedCommand(
                 [&]() {
                   auto position = GetPosition();
                   auto x_cur = position.x();
                   auto y_cur = position.y();
                   auto dx = x_cur - x;
                   auto dy = y_cur - y;

                   return ((dx * dx + dy * dy) <= kMeterToleranceSq);
                 },
                 timeout_sec, command_start_time_nanos)
                 .IsComplete();

  if (yaw_rate_max >= 0)
    pparams_->angle_rate_pid.max_limit.Z() = yaw_max_limit_sav;

  return f_result;
}

bool SimpleDriveApi::MoveByHeading(float heading, float speed, float duration,
                                   float heading_margin, float yaw_rate,
                                   float timeout_sec,
                                   int64_t command_start_time_nanos) {
  if (!api_control_enabled_) {
    GetLogger().LogError(GetControllerName(),
                         "Vehicle cannot be commanded via API because API has "
                         "not been given control");
    return false;
  }

  bool f_result;
  auto& goals = *pgoals_;
  float yaw_max_limit_sav;

  goals[GoalTargetIndex::kX].mode = Goal::Mode::kNone;
  goals[GoalTargetIndex::kY].mode = Goal::Mode::kNone;
  goals[GoalTargetIndex::kZ].Set(Goal::Mode::kAngleLevel,
                                 heading);  // Enable automatic steering
  goals[GoalTargetIndex::kThrottle].Set(Goal::Mode::kVelocityWorld, speed);
  goals[GoalTargetIndex::kBrake].Set(Goal::Mode::kPassthrough, 0);

  if (yaw_rate >= 0) {
    yaw_max_limit_sav = pparams_->angle_rate_pid.max_limit.Z();
    pparams_->angle_rate_pid.max_limit.Z() = yaw_rate;
  }

  // Turn to heading
  f_result = RunTimedCommand(
                 [&]() {
                   auto angles = GetAngles();
                   auto heading_cur = angles.z();
                   auto dh = heading_cur - heading;

                   return (std::abs(dh) <= heading_margin);
                 },
                 timeout_sec, command_start_time_nanos)
                 .IsComplete();

  // Move forward for duration, continue correcting steering if needed
  f_result = RunTimedCommand([&]() { return false; }, duration,
                             command_start_time_nanos)
                 .IsComplete();

  if (yaw_rate >= 0) pparams_->angle_rate_pid.max_limit.Z() = yaw_max_limit_sav;

  return f_result;
}

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft
