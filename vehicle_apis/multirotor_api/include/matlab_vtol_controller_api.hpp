// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_MATLAB_VTOL_CONTROLLER_API_HPP_
#define MULTIROTOR_API_INCLUDE_MATLAB_VTOL_CONTROLLER_API_HPP_

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "message/vtol_control_model_message.hpp"
#include "nng/nng.h"
#include "vtolfw_api_base.hpp"

namespace microsoft {
namespace projectairsim {

class MatlabVTOLControllerApi : public VTOLFWApiBase {
 public:
  MatlabVTOLControllerApi(const Robot& robot, TransformTree* ptransformtree);
  ~MatlabVTOLControllerApi() override;

  void BeginUpdate() override;
  void EndUpdate() override;
  void Reset() override;
  void SetKinematics(const Kinematics* kinematics) override;
  void Update() override;
  int GetControlSignalIndex(const std::string& actuator_id) override;
  void GetControlSignalSnapshot(std::vector<float>& control_signals) override;
  std::vector<float> GetControlSignals(int signal_index) override;
  std::vector<float> GetControlSignals(const std::string& actuator_id) override;

  bool EnableApiControl() override;
  bool DisableApiControl() override;
  bool IsApiControlEnabled() override;
  bool Arm(int64_t command_start_time_nanos) override;
  bool Disarm() override;
  bool CanArm() const override;
  ReadyState GetReadyState() const override;
  Kinematics GetKinematicsEstimated() const override;
  LandedState GetLandedState() const override;
  bool CancelLastTask() override;
  bool SetVTOLMode(VTOLMode vtolmode) override;
  bool Takeoff(float timeout_sec, int64_t command_start_time_nanos) override;
  bool Land(float timeout_sec, int64_t command_start_time_nanos) override;
  bool Hover(int64_t command_start_time_nanos) override;
  bool MoveByVelocity(float vx, float vy, float vz, float duration,
                      DrivetrainType drivetrain, bool yaw_is_rate, float yaw,
                      int64_t command_start_time_nanos) override;
  bool MoveByVelocityZ(float vx, float vy, float z, float duration,
                       DrivetrainType drivetrain, bool yaw_is_rate, float yaw,
                       int64_t command_start_time_nanos) override;
  bool MoveByHeading(float heading, float speed, float vz, float duration,
                     float heading_margin, float yaw_rate, float timeout_sec,
                     int64_t command_start_time_nanos) override;
  bool MoveByVelocityBodyFrame(float vx, float vy, float vz, float duration,
                               DrivetrainType drivetrain, bool yaw_is_rate,
                               float yaw,
                               int64_t command_start_time_nanos) override;
  bool MoveByVelocityBodyFrameZ(float vx, float vy, float z, float duration,
                                DrivetrainType drivetrain, bool yaw_is_rate,
                                float yaw,
                                int64_t command_start_time_nanos) override;

 protected:
  void CommandMotorPWMs(float, float, float, float) override;
  void CommandRollPitchYawrateThrottle(float, float, float, float) override;
  void CommandRollPitchYawZ(float, float, float, float) override;
  void CommandRollPitchYawThrottle(float, float, float, float) override;
  void CommandRollPitchYawrateZ(float, float, float, float) override;
  void CommandAngleRatesZ(float, float, float, float) override;
  void CommandAngleRatesThrottle(float, float, float, float) override;
  void CommandVelocity(float vx, float vy, float vz, bool yaw_is_rate,
                       float yaw) override;
  void CommandVelocityZ(float vx, float vy, float z, bool yaw_is_rate,
                        float yaw) override;
  void CommandVelocityBody(float vx, float vy, float vz, bool yaw_is_rate,
                           float yaw) override;
  void CommandVelocityZBody(float vx, float vy, float z, bool yaw_is_rate,
                            float yaw) override;
  void CommandPosition(float x, float y, float z, bool yaw_is_rate,
                       float yaw) override;
  void CommandHeading(float heading, float speed, float vz) override;
  void SetControllerGains(uint8_t, const std::vector<float>&,
                          const std::vector<float>&,
                          const std::vector<float>&) override;
  std::string GetControllerName() const override {
    return "MatlabVTOLControllerApi";
  }
  GeoPoint GetGpsLocationEstimated() const override;
  const MultirotorApiParams& GetMultirotorApiParams() const override;
  float GetCommandPeriod() const override;
  float GetTakeoffZ() const override;
  float GetDistanceAccuracy() const override;
  void BeforeTask() override;
  void AfterTask() override;

 private:
  void LoadSettings();
  ControlModelInputMessage GetState() const;
  void SetCommand(uint32_t mode, uint32_t frame, float x, float y, float z,
                  bool yaw_is_rate, float yaw);
  void SetRequest(uint32_t request);
  void SetIdle();
  void MarkHandoffCompletion(bool completed);
  void RestoreInitialOutputs();
  void CloseConnection();

  mutable std::mutex update_lock_;
  std::mutex socket_lock_;
  bool has_kinematics_ = false;
  Kinematics kinematics_estimated_;
  std::vector<ExternalEffector> effectors_;
  std::unordered_map<std::string, size_t> actuator_indices_;
  std::vector<float> outputs_;
  ExternalControlSetpoint setpoint_;
  MultirotorApiParams params_;
  nng_socket socket_ = NNG_SOCKET_INITIALIZER;
  bool socket_open_ = false;
  // Whether the current outage was already reported; guarded by socket_lock_.
  bool transport_failed_ = false;
  bool running_ = false;
  bool ready_ = false;
  bool landed_ = true;
  uint64_t lifecycle_generation_ = 0;
  std::string connection_string_;
  int request_timeout_ms_ = 1000;
  float command_period_ = 0.02f;
  double command_handoff_grace_period_ = 0;
  TimeNano handoff_deadline_nanos_ = 0;
  bool handoff_command_issued_ = false;
  bool handoff_task_completed_ = false;
  float takeoff_z_ = -3;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_MATLAB_VTOL_CONTROLLER_API_HPP_
