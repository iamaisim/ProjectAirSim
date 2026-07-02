// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "Rover.h"

#include <exception>
#include <limits>

#include "ProjectAirsimClient.h"
#include "ProjectAirSimMessage/response_message.hpp"
#include "AsyncResultInternal.h"
#include "JSONUtils.h"
#include "pch.h"

namespace microsoft {
namespace projectairsim {
namespace client {

class Rover::Impl {
 public:
  Impl(void) noexcept;
  ~Impl();

  Status Arm(bool* pf_is_armed_out);
  Status CanArm(bool* pf_can_be_armed_out) const;
  Status CancelLastTask(bool* pf_task_is_canceled_out);
  Status DisableAPIControl(bool* pf_is_disabled_out);
  Status Disarm(bool* pf_is_disarmed_out);
  Status EnableAPIControl(bool* pf_is_enabled_out);
  Status IsAPIControlEnabled(bool* pf_is_enabled_out) const;
  Status GetGroundTruthKinematics(json* pjson_out) const;
  Status SetPose(const Transform& transform, bool reset_kinematics);
  Status Initialize(std::shared_ptr<Client>& pclient,
                    std::shared_ptr<World>& pworld,
                    const std::string& rover_name);
  AsyncResult MoveToPositionAsync(float north, float east, float velocity,
                                  float timeout_sec, float yaw_rate_max,
                                  float lookahead, float adaptive_lookahead,
                                  FnResponseCallback fnresponse_callback);
  AsyncResult MoveByHeadingAsync(float heading, float speed, float sec_duration,
                                 float heading_margin, float yaw_rate,
                                 float timeout_sec,
                                 FnResponseCallback fnresponse_callback);
  Status SetRoverControls(float engine, float steering_angle, float brake);

 protected:
  Status RequestRover(const std::string& str_method,
                      const json& json_params = json()) const;
  Status RequestRover(const std::string& str_method, bool* pf_out) const;
  Status RequestRover(const std::string& str_method, json* pjson_out) const;
  Status RequestRover(const std::string& str_method, const json& json_params,
                      bool* pf_out) const;
  Status RequestRover(const std::string& str_method, const json& json_params,
                      json* pjson_out) const;
  AsyncResult RequestRoverAsync(const std::string& str_method,
                                const json& json_params,
                                FnResponseCallback fnresponse_callback) const;

 protected:
  std::shared_ptr<Client> pclient_;
  std::shared_ptr<World> pworld_;
  std::string str_rover_name_;
  std::string str_parent_topic_;
  std::string str_world_parent_topic_;
};  // class Rover::Impl

// ---------------------------------------------------------------------------
// Rover public API
// ---------------------------------------------------------------------------

ASC_DECL const float Rover::kNoTimeout = std::numeric_limits<float>::max();

ASC_DECL Rover::Rover(void) noexcept : pimpl_(new Impl()) {}

ASC_DECL Rover::~Rover() {}

ASC_DECL Status Rover::Initialize(std::shared_ptr<Client>& pclient,
                                   std::shared_ptr<World>& pworld,
                                   const std::string& rover_name) {
  RETURN_CATCH_STATUS(pimpl_->Initialize(pclient, pworld, rover_name));
}

ASC_DECL Status Rover::Arm(bool* pf_is_armed_out) {
  RETURN_CATCH_STATUS(pimpl_->Arm(pf_is_armed_out));
}

ASC_DECL Status Rover::CanArm(bool* pf_can_be_armed_out) const {
  RETURN_CATCH_STATUS(pimpl_->CanArm(pf_can_be_armed_out));
}

ASC_DECL Status Rover::CancelLastTask(bool* pf_task_is_canceled_out) {
  RETURN_CATCH_STATUS(pimpl_->CancelLastTask(pf_task_is_canceled_out));
}

ASC_DECL Status Rover::DisableAPIControl(bool* pf_is_disabled_out) {
  RETURN_CATCH_STATUS(pimpl_->DisableAPIControl(pf_is_disabled_out));
}

ASC_DECL Status Rover::Disarm(bool* pf_is_disarmed_out) {
  RETURN_CATCH_STATUS(pimpl_->Disarm(pf_is_disarmed_out));
}

ASC_DECL Status Rover::EnableAPIControl(bool* pf_is_enabled_out) {
  RETURN_CATCH_STATUS(pimpl_->EnableAPIControl(pf_is_enabled_out));
}

ASC_DECL Status Rover::IsAPIControlEnabled(bool* pf_is_enabled_out) const {
  RETURN_CATCH_STATUS(pimpl_->IsAPIControlEnabled(pf_is_enabled_out));
}

ASC_DECL Status Rover::GetGroundTruthKinematics(json* pjson_out) const {
  RETURN_CATCH_STATUS(pimpl_->GetGroundTruthKinematics(pjson_out));
}

ASC_DECL Status Rover::SetPose(const Transform& transform,
                                bool reset_kinematics) {
  RETURN_CATCH_STATUS(pimpl_->SetPose(transform, reset_kinematics));
}

ASC_DECL AsyncResult Rover::MoveToPositionAsync(
    float north, float east, float velocity, float timeout_sec,
    float yaw_rate_max, float lookahead, float adaptive_lookahead,
    FnResponseCallback fnresponse_callback) {
  return (pimpl_->MoveToPositionAsync(north, east, velocity, timeout_sec,
                                      yaw_rate_max, lookahead,
                                      adaptive_lookahead, fnresponse_callback));
}

ASC_DECL AsyncResult Rover::MoveByHeadingAsync(
    float heading, float speed, float sec_duration, float heading_margin,
    float yaw_rate, float timeout_sec, FnResponseCallback fnresponse_callback) {
  return (pimpl_->MoveByHeadingAsync(heading, speed, sec_duration,
                                     heading_margin, yaw_rate, timeout_sec,
                                     fnresponse_callback));
}

ASC_DECL Status Rover::SetRoverControls(float engine, float steering_angle,
                                         float brake) {
  RETURN_CATCH_STATUS(pimpl_->SetRoverControls(engine, steering_angle, brake));
}

// ---------------------------------------------------------------------------
// Rover::Impl
// ---------------------------------------------------------------------------

Rover::Impl::Impl() noexcept
    : pclient_(),
      pworld_(),
      str_rover_name_(),
      str_parent_topic_(),
      str_world_parent_topic_() {}

Rover::Impl::~Impl() {}

Status Rover::Impl::Initialize(std::shared_ptr<Client>& pclient,
                                std::shared_ptr<World>& pworld,
                                const std::string& rover_name) {
  log.InfoF("Initalizing Rover '%s'...", rover_name.c_str());

  pclient_ = pclient;
  pworld_ = pworld;
  str_rover_name_ = rover_name;
  str_world_parent_topic_ = pworld->GetParentTopic();
  str_parent_topic_ = str_world_parent_topic_ + "/robots/" + str_rover_name_;

  log.InfoF("Rover '%s' initialized for World scene '%s'",
            str_rover_name_.c_str(), str_world_parent_topic_.c_str());

  return (Status::OK);
}

Status Rover::Impl::Arm(bool* pf_is_armed_out) {
  return (RequestRover("Arm", pf_is_armed_out));
}

Status Rover::Impl::CanArm(bool* pf_can_be_armed_out) const {
  return (RequestRover("CanArm", pf_can_be_armed_out));
}

Status Rover::Impl::CancelLastTask(bool* pf_task_is_canceled_out) {
  TAsyncResult<Message> ar;
  Status status;

  ar = pclient_->RequestPriorityAsync(str_parent_topic_ + "/CancelLastTask",
                                      json(), nullptr);
  if ((status = ar.Wait()) == Status::OK) {
    const Message& message = ar.GetResult();
    ResponseMessage response_message;

    response_message.Deserialize(message);
    if (response_message.GetErrorCode() == 0)
      *pf_task_is_canceled_out =
          response_message.GetResult().is_boolean() &&
          static_cast<bool>(response_message.GetResult());
    else {
      status = Status::RejectedByServer;
      log.ErrorF("Server method \"CancelLastTask\" failed: error %d: %s",
                 response_message.GetErrorCode(),
                 static_cast<std::string>(
                     response_message.GetResult()["message"])
                     .c_str());
    }
  }

  return (status);
}

Status Rover::Impl::DisableAPIControl(bool* pf_is_disabled_out) {
  return (RequestRover("DisableApiControl", pf_is_disabled_out));
}

Status Rover::Impl::Disarm(bool* pf_is_disarmed_out) {
  return (RequestRover("Disarm", pf_is_disarmed_out));
}

Status Rover::Impl::EnableAPIControl(bool* pf_is_enabled_out) {
  return (RequestRover("EnableApiControl", pf_is_enabled_out));
}

Status Rover::Impl::IsAPIControlEnabled(bool* pf_is_enabled_out) const {
  return (RequestRover("IsApiControlEnabled", pf_is_enabled_out));
}

Status Rover::Impl::GetGroundTruthKinematics(json* pjson_out) const {
  return (RequestRover("GetGroundTruthKinematics", pjson_out));
}

Status Rover::Impl::SetPose(const Transform& transform, bool reset_kinematics) {
  json json_params = {{"pose", transform},
                      {"reset_kinematics", reset_kinematics}};
  return (RequestRover("SetPose", json_params));
}

AsyncResult Rover::Impl::MoveToPositionAsync(
    float north, float east, float velocity, float timeout_sec,
    float yaw_rate_max, float lookahead, float adaptive_lookahead,
    FnResponseCallback fnresponse_callback) {
  json json_params = {
      {"x", north},
      {"y", east},
      {"velocity", velocity},
      {"timeout_sec", timeout_sec},
      {"yaw_rate_max", yaw_rate_max},
      {"lookahead", lookahead},
      {"adaptive_lookahead", adaptive_lookahead},
  };

  return (RequestRoverAsync("MoveToPosition", json_params, fnresponse_callback));
}

AsyncResult Rover::Impl::MoveByHeadingAsync(float heading, float speed,
                                             float sec_duration,
                                             float heading_margin,
                                             float yaw_rate, float timeout_sec,
                                             FnResponseCallback
                                                 fnresponse_callback) {
  json json_params = {
      {"heading", heading},
      {"speed", speed},
      {"duration", sec_duration},
      {"heading_margin", heading_margin},
      {"yaw_rate", yaw_rate},
      {"timeout_sec", timeout_sec},
  };

  return (RequestRoverAsync("MoveByHeading", json_params, fnresponse_callback));
}

Status Rover::Impl::SetRoverControls(float engine, float steering_angle,
                                      float brake) {
  json json_params = {
      {"engine", engine},
      {"steering_angle", steering_angle},
      {"brake", brake},
  };
  return (RequestRover("SetRoverControls", json_params));
}

// ---------------------------------------------------------------------------
// Request helpers
// ---------------------------------------------------------------------------

Status Rover::Impl::RequestRover(const std::string& str_method,
                                  const json& json_params) const {
  Status status;
  Message message_response;

  status = pclient_->Request(str_parent_topic_ + "/" + str_method, json_params,
                             &message_response);
  if (status == Status::OK) {
    ResponseMessage response_message;

    response_message.Deserialize(message_response);
    if (response_message.GetErrorCode() != 0) {
      status = Status::RejectedByServer;
      log.ErrorF(
          "Server method \"%s\" failed: error %d: %s", str_method.c_str(),
          response_message.GetErrorCode(),
          static_cast<std::string>(response_message.GetResult()["message"])
              .c_str());
    }
  }

  return (status);
}

Status Rover::Impl::RequestRover(const std::string& str_method,
                                  bool* pf_out) const {
  return (RequestRover(str_method, json(), pf_out));
}

Status Rover::Impl::RequestRover(const std::string& str_method,
                                  json* pjson_out) const {
  return (RequestRover(str_method, json(), pjson_out));
}

Status Rover::Impl::RequestRover(const std::string& str_method,
                                  const json& json_params, bool* pf_out) const {
  json json_response;
  Status status;

  status = RequestRover(str_method, json_params, &json_response);
  if (status == Status::OK) *pf_out = json_response;

  return (status);
}

Status Rover::Impl::RequestRover(const std::string& str_method,
                                  const json& json_params,
                                  json* pjson_out) const {
  Message message_response;
  Status status;

  status = pclient_->Request(str_parent_topic_ + "/" + str_method, json_params,
                             &message_response);
  if (status == Status::OK) {
    ResponseMessage response_message;

    response_message.Deserialize(message_response);
    if (response_message.GetErrorCode() == 0)
      *pjson_out = response_message.GetResult();
    else {
      status = Status::RejectedByServer;
      log.ErrorF(
          "Server method \"%s\" failed: error %d: %s", str_method.c_str(),
          response_message.GetErrorCode(),
          static_cast<std::string>(response_message.GetResult()["message"])
              .c_str());
    }
  }

  return (status);
}

AsyncResult Rover::Impl::RequestRoverAsync(
    const std::string& str_method, const json& json_params,
    FnResponseCallback fnresponse_callback) const {
  return (internal::AsyncResultMessageConverter(pclient_->RequestAsync(
      str_parent_topic_ + "/" + str_method, json_params, fnresponse_callback)));
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
