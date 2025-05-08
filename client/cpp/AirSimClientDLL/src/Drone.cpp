// Copyright (C) Microsoft Corporation.  All rights reserved.

#include "Drone.h"

#include <exception>
#include <limits>
#include <regex>
#include <unordered_set>

#include "AirSimClient.h"
#include "AirSimMessage/response_message.hpp"
#include "AsyncResultInternal.h"
#include "Common.h"
#include "JSONUtils.h"
#include "pch.h"

namespace microsoft {
namespace projectairsim {
namespace client {

class Drone::Impl {
 public:
  Impl(void) noexcept;
  ~Impl();

  Status Arm(bool* pf_is_armed_out);
  Status CanArm(bool* pf_can_be_armed_out) const;
  Status CancelLastTask(bool* pf_task_is_canceled_out);
  Status DisableAPIControl(bool* pf_is_disabled_out);
  Status Disarm(bool* pf_is_disarmed_out);
  Status EnableAPIControl(bool* pf_is_enabled_out);
  Status GetCameraRay(const std::string& str_camera_name, ImageType image_type,
                      int x, int y, Pose* ppose_out);
  Status GetGroundTruthPose(Transform* ptransform_out);
  size_t GetRobotInfoCount(void) const;
  const char* GetRobotInfoName(int iname) const;
  const char* GetRobotTopic(const std::string& str_info_name) const;
  size_t GetSensorCount(void) const;
  size_t GetSensorInfoCount(const std::string& str_sensor_name) const;
  const char* GetSensorInfoName(const std::string& str_sensor_name,
                                int iinfo) const;
  const char* GetSensorName(int iname) const;
  const char* GetSensorTopic(const std::string& str_sensor_name,
                             const std::string& str_info_name) const;
  AsyncResult GoHomeAsync(float sec_timeout, float velocity,
                          FnResponseCallback fnresponse_callback);
  AsyncResult HoverAsync(FnResponseCallback fnresponse_callback);
  Status Initialize(std::shared_ptr<Client>& pclient,
                    std::shared_ptr<World>& pworld,
                    const std::string& str_drone_name);
  Status IsAPIControlEnabled(bool* pf_is_enabled_out) const;
  AsyncResult LandAsync(float sec_timeout,
                        FnResponseCallback fnresponse_callback);
  AsyncResult MoveByVelocityAsync(float v_north, float v_east, float v_down,
                                  float sec_duration,
                                  YawControlMode yaw_control_mode,
                                  bool yaw_is_rate, float yaw,
                                  FnResponseCallback fnresponse_callback);
  ASC_DECL AsyncResult MoveByVelocityBodyFrameAsync(
      float v_forward, float v_right, float v_down, float sec_duration,
      YawControlMode yaw_control_mode, bool yaw_is_rate, float yaw,
      FnResponseCallback fnresponse_callback);
  ASC_DECL AsyncResult MoveOnPathAsync(const VecVector3& path, float velocity,
                                       float timeout_sec,
                                       YawControlMode yaw_control_mode,
                                       bool yaw_is_rate, float yaw,
                                       float lookahead,
                                       float adaptive_lookahead,
                                       FnResponseCallback fnresponse_callback);
  ASC_DECL AsyncResult MoveToPositionAsync(
      float north, float east, float down, float velocity, float timeout_sec,
      YawControlMode yaw_control_mode, bool yaw_is_rate, float yaw,
      float lookahead, float adaptive_lookahead,
      FnResponseCallback fnresponse_callback);
  Status SetPose(const Transform& transform, bool reset_kinematics);
  AsyncResult TakeoffAsync(float sec_timeout,
                           FnResponseCallback fnresponse_callback);

 protected:
  // Mapping from a string to another
  typedef std::unordered_map<std::string, std::string> UMStrStr;

  // Mapping from a string to a string-to-string map
  typedef std::unordered_map<std::string, UMStrStr> UMStrUMStr;

  // Mapping from a string to vector of strings
  typedef std::unordered_map<std::string, std::vector<std::string>> UMStrVecStr;

 protected:
  void LogTopics(void) const;
  void SetRobotInfoTopics(void);
  void SetSensorTopics(World* pworld);
  void SetTopics(void);
  Status RequestDrone(const std::string str_method,
                      const json& json_params = json()) const;
  Status RequestDrone(const std::string str_method, bool* pf_out) const;
  Status RequestDrone(const std::string str_method, json* pjson_out) const;
  Status RequestDrone(const std::string str_method, const json& json_params,
                      bool* pf_out) const;
  Status RequestDrone(const std::string str_method, const json& json_params,
                      json* pjson_out) const;
  AsyncResult RequestDroneAsync(const std::string str_method,
                                const json& json_params,
                                FnResponseCallback fnresponse_callback) const;

 protected:
  std::shared_ptr<Client> pclient_;     // Pointer to client connection object
  std::shared_ptr<World> pworld_;       // Pointer to scene object
  std::string str_drone_name_;          // Name of the drone
  std::string str_parent_topic_;        // Topic root of the drone
  std::string str_sensor_topic_;        // Topic root of the sensors
  std::string str_world_parent_topic_;  // Topic root of the scene
  UMStrStr umrobot_name_topic_;  // Mapping from robot info name to topic name
  UMStrUMStr umsensor_name_uminfo_topic_;  // Mapping from sensor name to
                                           // mapping from info to topic name
  UMStrVecStr umsensor_name_vecinfo_;  // Mapping from sensor name to list of
                                       // sensor info names
  VecStr vecstr_robot_info_name_;      // List of robot info names
  VecStr vecstr_sensor_name_;          // List of sensor names
};                                     // class Drone::Impl

ASC_DECL const float Drone::kNoTimeout = std::numeric_limits<float>::max();

ASC_DECL Drone::Drone(void) noexcept : pimpl_(new Impl()) {}

ASC_DECL Drone::~Drone() {}

Status Drone::Initialize(std::shared_ptr<Client>& pclient,
                         std::shared_ptr<World>& pworld,
                         const std::string& str_drone_name) {
  RETURN_CATCH_STATUS(pimpl_->Initialize(pclient, pworld, str_drone_name));
}

ASC_DECL Status Drone::Arm(bool* pf_is_armed_out) {
  RETURN_CATCH_STATUS(pimpl_->Arm(pf_is_armed_out));
}

ASC_DECL Status Drone::Disarm(bool* pf_is_disarmed_out) {
  RETURN_CATCH_STATUS(pimpl_->Disarm(pf_is_disarmed_out));
}

ASC_DECL Status Drone::CanArm(bool* pf_can_be_armed_out) const {
  RETURN_CATCH_STATUS(pimpl_->CanArm(pf_can_be_armed_out));
}

ASC_DECL Status Drone::CancelLastTask(bool* pf_task_is_canceled_out) {
  RETURN_CATCH_STATUS(pimpl_->CancelLastTask(pf_task_is_canceled_out));
}

ASC_DECL Status Drone::DisableAPIControl(bool* pf_is_disabled_out) {
  RETURN_CATCH_STATUS(pimpl_->DisableAPIControl(pf_is_disabled_out));
}

ASC_DECL Status Drone::EnableAPIControl(bool* pf_is_enabled_out) {
  RETURN_CATCH_STATUS(pimpl_->EnableAPIControl(pf_is_enabled_out));
}

ASC_DECL Status Drone::GetCameraRay(const std::string& str_camera_name,
                                    ImageType image_type, int x, int y,
                                    Pose* ppose_out) {
  RETURN_CATCH_STATUS(
      pimpl_->GetCameraRay(str_camera_name, image_type, x, y, ppose_out));
}

ASC_DECL Status Drone::GetGroundTruthPose(Transform* ptransform_out) {
  RETURN_CATCH_STATUS(pimpl_->GetGroundTruthPose(ptransform_out));
}

ASC_DECL size_t Drone::GetRobotInfoCount(void) const {
  return (pimpl_->GetRobotInfoCount());
}

ASC_DECL const char* Drone::GetRobotInfoName(int iname) const {
  return (pimpl_->GetRobotInfoName(iname));
}

ASC_DECL const char* Drone::GetRobotTopic(
    const std::string& str_info_name) const {
  return (pimpl_->GetRobotTopic(str_info_name));
}

ASC_DECL size_t Drone::GetSensorCount(void) const {
  return (pimpl_->GetSensorCount());
}

ASC_DECL size_t
Drone::GetSensorInfoCount(const std::string& str_sensor_name) const {
  return (pimpl_->GetSensorInfoCount(str_sensor_name));
}

ASC_DECL const char* Drone::GetSensorInfoName(
    const std::string& str_sensor_name, int iinfo) const {
  return (pimpl_->GetSensorInfoName(str_sensor_name, iinfo));
}

ASC_DECL const char* Drone::GetSensorName(int iname) const {
  return (pimpl_->GetSensorName(iname));
}

ASC_DECL const char* Drone::GetSensorTopic(
    const std::string& str_sensor_name,
    const std::string& str_info_name) const {
  return (pimpl_->GetSensorTopic(str_sensor_name, str_info_name));
}

ASC_DECL AsyncResult Drone::GoHomeAsync(
    float sec_timeout, float velocity, FnResponseCallback fnresponse_callback) {
  return (pimpl_->GoHomeAsync(sec_timeout, velocity, fnresponse_callback));
}

ASC_DECL AsyncResult Drone::HoverAsync(FnResponseCallback fnresponse_callback) {
  return (pimpl_->HoverAsync(fnresponse_callback));
}

ASC_DECL Status Drone::IsAPIControlEnabled(bool* pf_is_enabled_out) const {
  RETURN_CATCH_STATUS(pimpl_->IsAPIControlEnabled(pf_is_enabled_out));
}

ASC_DECL AsyncResult Drone::LandAsync(float sec_timeout,
                                      FnResponseCallback fnresponse_callback) {
  return (pimpl_->LandAsync(sec_timeout, fnresponse_callback));
}

ASC_DECL AsyncResult Drone::MoveByVelocityAsync(
    float v_north, float v_east, float v_down, float sec_duration,
    YawControlMode yaw_control_mode, bool yaw_is_rate, float yaw,
    FnResponseCallback fnresponse_callback) {
  return (pimpl_->MoveByVelocityAsync(v_north, v_east, v_down, sec_duration,
                                      yaw_control_mode, yaw_is_rate, yaw,
                                      fnresponse_callback));
}

ASC_DECL AsyncResult Drone::MoveByVelocityBodyFrameAsync(
    float v_forward, float v_right, float v_down, float sec_duration,
    YawControlMode yaw_control_mode, bool yaw_is_rate, float yaw,
    FnResponseCallback fnresponse_callback) {
  return (pimpl_->MoveByVelocityBodyFrameAsync(
      v_forward, v_right, v_down, sec_duration, yaw_control_mode, yaw_is_rate,
      yaw, fnresponse_callback));
}

ASC_DECL AsyncResult Drone::MoveOnPathAsync(
    const VecVector3& path, float velocity, float timeout_sec,
    YawControlMode yaw_control_mode, bool yaw_is_rate, float yaw,
    float lookahead, float adaptive_lookahead,
    FnResponseCallback fnresponse_callback) {
  return (pimpl_->MoveOnPathAsync(path, velocity, timeout_sec, yaw_control_mode,
                                  yaw_is_rate, yaw, lookahead,
                                  adaptive_lookahead, fnresponse_callback));
}

ASC_DECL AsyncResult Drone::MoveToPositionAsync(
    float north, float east, float down, float velocity, float timeout_sec,
    YawControlMode yaw_control_mode, bool yaw_is_rate, float yaw,
    float lookahead, float adaptive_lookahead,
    FnResponseCallback fnresponse_callback) {
  return (pimpl_->MoveToPositionAsync(
      north, east, down, velocity, timeout_sec, yaw_control_mode, yaw_is_rate,
      yaw, lookahead, adaptive_lookahead, fnresponse_callback));
}

ASC_DECL Status Drone::SetPose(const Transform& transform,
                               bool reset_kinematics) {
  RETURN_CATCH_STATUS(pimpl_->SetPose(transform, reset_kinematics));
}

ASC_DECL AsyncResult
Drone::TakeoffAsync(float sec_timeout, FnResponseCallback fnresponse_callback) {
  return (pimpl_->TakeoffAsync(sec_timeout, fnresponse_callback));
}

Drone::Impl::Impl() noexcept
    : pclient_(),
      pworld_(),
      str_drone_name_(),
      str_parent_topic_(),
      str_sensor_topic_(),
      str_world_parent_topic_(),
      umrobot_name_topic_(),
      umsensor_name_uminfo_topic_(),
      umsensor_name_vecinfo_(),
      vecstr_robot_info_name_(),
      vecstr_sensor_name_() {}

Drone::Impl::~Impl() {}

Status Drone::Impl::Arm(bool* pf_is_armed_out) {
  return (RequestDrone("Arm", pf_is_armed_out));
}

Status Drone::Impl::CanArm(bool* pf_can_be_armed_out) const {
  return (RequestDrone("CanArm", pf_can_be_armed_out));
}

Status Drone::Impl::CancelLastTask(bool* pf_task_is_canceled_out) {
  TAsyncResult<Message> ar;
  Status status;

  ar = pclient_->RequestPriorityAsync(str_parent_topic_ + "/CancelLastTask", json(), nullptr);
  if ((status = ar.Wait()) == Status::OK) {
    const Message& message = ar.GetResult();
    ResponseMessage response_message;

    response_message.Deserialize(message);
    if (response_message.GetErrorCode() == 0)
        *pf_task_is_canceled_out = response_message.GetResult().is_boolean() && static_cast<bool>(response_message.GetResult());
    else
    {
      status = Status::RejectedByServer;

      log.ErrorF(
          "Server method \"CancelLastTask\" failed: error %d: %s",
          response_message.GetErrorCode(),
          static_cast<std::string>(response_message.GetResult()["message"])
              .c_str());
    }
  }

  return (status);
}

Status Drone::Impl::DisableAPIControl(bool* pf_is_disabled_out) {
  return (RequestDrone("DisableApiControl", pf_is_disabled_out));
}

Status Drone::Impl::Disarm(bool* pf_is_disarmed_out) {
  return (RequestDrone("Disarm", pf_is_disarmed_out));
}

Status Drone::Impl::EnableAPIControl(bool* pf_is_enabled_out) {
  return (RequestDrone("EnableApiControl", pf_is_enabled_out));
}

Status Drone::Impl::GetCameraRay(const std::string& str_camera_name,
                                 ImageType image_type, int x, int y,
                                 Pose* ppose_out) {
  Status status;
  json json_params = {{"camera_id", str_camera_name},
                      {"image_type", (int)image_type},
                      {"x", x},
                      {"y", y}};
  json json_response;

  if ((status = RequestDrone("GetCameraRay", json_params, &json_response)) ==
      Status::OK)
    *ppose_out = json_response;

  return (status);
}

Status Drone::Impl::GetGroundTruthPose(Transform* ptransform_out) {
  Status status;
  json json_response;

  if ((status = RequestDrone("GetGroundTruthPose", &json_response)) ==
      Status::OK)
    *ptransform_out = json_response;

  return (status);
}

size_t Drone::Impl::GetRobotInfoCount(void) const {
  return (vecstr_robot_info_name_.size());
}

const char* Drone::Impl::GetRobotInfoName(int iname) const {
  const char* sz_ret = nullptr;

  if ((iname >= 0) && (iname < vecstr_robot_info_name_.size()))
    sz_ret = vecstr_robot_info_name_[iname].c_str();

  return (sz_ret);
}

const char* Drone::Impl::GetRobotTopic(const std::string& str_info_name) const {
  const char* sz_ret = nullptr;
  auto it_robot = umrobot_name_topic_.find(str_info_name);

  if (it_robot != umrobot_name_topic_.end()) sz_ret = it_robot->second.c_str();

  return (sz_ret);
}

size_t Drone::Impl::GetSensorCount(void) const {
  return (vecstr_sensor_name_.size());
}

size_t Drone::Impl::GetSensorInfoCount(
    const std::string& str_sensor_name) const {
  size_t cinfo = 0;
  auto it = umsensor_name_vecinfo_.find(str_sensor_name);

  if (it != umsensor_name_vecinfo_.end()) cinfo = it->second.size();

  return (cinfo);
}

const char* Drone::Impl::GetSensorInfoName(const std::string& str_sensor_name,
                                           int iinfo) const {
  const char* sz_ret = nullptr;
  auto it = umsensor_name_vecinfo_.find(str_sensor_name);

  if ((it != umsensor_name_vecinfo_.end()) && (iinfo >= 0) &&
      (iinfo < it->second.size())) {
    sz_ret = it->second[iinfo].c_str();
  }

  return (sz_ret);
}

const char* Drone::Impl::GetSensorName(int iname) const {
  const char* sz_ret = nullptr;

  if ((iname >= 0) && (iname < vecstr_sensor_name_.size()))
    sz_ret = vecstr_sensor_name_[iname].c_str();

  return (sz_ret);
}

const char* Drone::Impl::GetSensorTopic(
    const std::string& str_sensor_name,
    const std::string& str_info_name) const {
  const char* sz_ret = nullptr;
  auto it_sensor = umsensor_name_uminfo_topic_.find(str_sensor_name);

  if (it_sensor != umsensor_name_uminfo_topic_.end()) {
    auto& uminfo_topic = it_sensor->second;
    auto it_info = uminfo_topic.find(str_info_name);

    if (it_info != uminfo_topic.end()) sz_ret = it_info->second.c_str();
  }

  return (sz_ret);
}

AsyncResult Drone::Impl::GoHomeAsync(float sec_timeout, float velocity,
                                     FnResponseCallback fnresponse_callback) {
  json json_params = {
      {"timeout_sec", sec_timeout},
      {"velocity", velocity},
  };

  return (RequestDroneAsync("GoHome", json_params, fnresponse_callback));
}

AsyncResult Drone::Impl::HoverAsync(FnResponseCallback fnresponse_callback) {
  return (RequestDroneAsync("Hover", json(), fnresponse_callback));
}

Status Drone::Impl::Initialize(std::shared_ptr<Client>& pclient,
                               std::shared_ptr<World>& pworld,
                               const std::string& str_drone_name) {
  Status status = Status::OK;

  log.InfoF("Initalizing Drone '%s'...", str_drone_name.c_str());

  pclient_ = pclient;
  pworld_ = pworld;
  str_drone_name_ = str_drone_name;
  str_world_parent_topic_ = pworld->GetParentTopic();

  try {
    SetTopics();
    LogTopics();

    log.InfoF("Drone '%s' initialized for  World scene '%s'",
              str_drone_name_.c_str(), str_world_parent_topic_.c_str());
  } catch (Status status_caught) {
    status = status_caught;
  }

  if (status != Status::OK) {
    char sz_error[512];

    GetStatusString(status, sz_error);
    log.ErrorF("Failed to initialize drone '%s': %s", str_drone_name.c_str(),
               sz_error);
  }

  return (status);
}

Status Drone::Impl::IsAPIControlEnabled(bool* pf_is_enabled_out) const {
  return (RequestDrone("IsApiControlEnabled", pf_is_enabled_out));
}

AsyncResult Drone::Impl::LandAsync(float sec_timeout,
                                   FnResponseCallback fnresponse_callback) {
  json json_params = {{"timeout_sec", sec_timeout}};

  return (RequestDroneAsync("Land", json_params, fnresponse_callback));
}

AsyncResult Drone::Impl::MoveByVelocityAsync(
    float v_north, float v_east, float v_down, float sec_duration,
    YawControlMode yaw_control_mode, bool yaw_is_rate, float yaw,
    FnResponseCallback fnresponse_callback) {
  json json_params = {
      {"vx", v_north},
      {"vy", v_east},
      {"vz", v_down},
      {"duration", sec_duration},
      {"drivetrain", yaw_control_mode},
      {"yaw_is_rate", yaw_is_rate},
      {"yaw", yaw},
  };

  return (
      RequestDroneAsync("MoveByVelocity", json_params, fnresponse_callback));
}

AsyncResult Drone::Impl::MoveByVelocityBodyFrameAsync(
    float v_forward, float v_right, float v_down, float sec_duration,
    YawControlMode yaw_control_mode, bool yaw_is_rate, float yaw,
    FnResponseCallback fnresponse_callback) {
  json json_params = {
      {"vx", v_forward},
      {"vy", v_right},
      {"vz", v_down},
      {"duration", sec_duration},
      {"drivetrain", yaw_control_mode},
      {"yaw_is_rate", yaw_is_rate},
      {"yaw", yaw},
  };

  return (RequestDroneAsync("MoveByVelocityBodyFrame", json_params,
                            fnresponse_callback));
}

AsyncResult Drone::Impl::MoveOnPathAsync(
    const VecVector3& path, float velocity, float timeout_sec,
    YawControlMode yaw_control_mode, bool yaw_is_rate, float yaw,
    float lookahead, float adaptive_lookahead,
    FnResponseCallback fnresponse_callback) {
  json json_params = {
      {"path", path},
      {"velocity", velocity},
      {"timeout_sec", timeout_sec},
      {"drivetrain", yaw_control_mode},
      {"yaw_is_rate", yaw_is_rate},
      {"yaw", yaw},
      {"lookahead", lookahead},
      {"adaptive_lookahead", adaptive_lookahead},
  };

  return (RequestDroneAsync("MoveOnPath", json_params, fnresponse_callback));
}

AsyncResult Drone::Impl::MoveToPositionAsync(
    float north, float east, float down, float velocity, float timeout_sec,
    YawControlMode yaw_control_mode, bool yaw_is_rate, float yaw,
    float lookahead, float adaptive_lookahead,
    FnResponseCallback fnresponse_callback) {
  json json_params = {
      {"x", north},
      {"y", east},
      {"z", down},
      {"velocity", velocity},
      {"timeout_sec", timeout_sec},
      {"drivetrain", yaw_control_mode},
      {"yaw_is_rate", yaw_is_rate},
      {"yaw", yaw},
      {"lookahead", lookahead},
      {"adaptive_lookahead", adaptive_lookahead},
  };

  return (
      RequestDroneAsync("MoveToPosition", json_params, fnresponse_callback));
}

AsyncResult Drone::Impl::TakeoffAsync(float sec_timeout,
                                      FnResponseCallback fnresponse_callback) {
  json json_params = {{"timeout_sec", sec_timeout}};

  return (RequestDroneAsync("Takeoff", json_params, fnresponse_callback));
}

void Drone::Impl::LogTopics(void) const {
  log.Info("-------------------------------------------------");
  log.InfoF("The following topics can be subscribed to for robot '%s':",
            str_drone_name_.c_str());
  for (auto& pair_sensor : umsensor_name_uminfo_topic_) {
    for (auto& pair_info : pair_sensor.second)
      log.InfoF("    sensors[\"%s\"][\"%s\"]", pair_sensor.first.c_str(),
                pair_info.first.c_str());
  }
  for (auto& pair_robot : umrobot_name_topic_)
    log.InfoF("    robot_info[\"%s\"]", pair_robot.first.c_str());
  log.Info("-------------------------------------------------");
}

Status Drone::Impl::RequestDrone(const std::string str_method,
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

Status Drone::Impl::RequestDrone(const std::string str_method,
                                 bool* pf_out) const {
  return (RequestDrone(str_method, json(), pf_out));
}

Status Drone::Impl::RequestDrone(const std::string str_method,
                                 json* pjson_out) const {
  return (RequestDrone(str_method, json(), pjson_out));
}

Status Drone::Impl::RequestDrone(const std::string str_method,
                                 const json& json_params, bool* pf_out) const {
  json json_response;
  Status status;

  status = RequestDrone(str_method, json_params, &json_response);
  if (status == Status::OK) *pf_out = json_response;

  return (status);
}

Status Drone::Impl::RequestDrone(const std::string str_method,
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

AsyncResult Drone::Impl::RequestDroneAsync(
    const std::string str_method, const json& json_params,
    FnResponseCallback fnresponse_callback) const {
  return (internal::AsyncResultMessageConverter(pclient_->RequestAsync(
      str_parent_topic_ + "/" + str_method, json_params, fnresponse_callback)));
}

Status Drone::Impl::SetPose(const Transform& transform, bool reset_kinematics) {
  json json_params = {{"pose", transform},
                      {"reset_kinematics", reset_kinematics}};

  return (RequestDrone("SetPose", json_params));
}

void Drone::Impl::SetRobotInfoTopics(void) {
  // TODO: Change this to construct this info from the published topics rather
  // than
  //  hard coding it.

  const char* c_sz_robot_info_name[3] = {
      "actual_pose",
      "collision_info",
      "rotor_info",
  };

  umrobot_name_topic_.clear();
  vecstr_robot_info_name_.clear();
  for (auto& sz : c_sz_robot_info_name) {
    umrobot_name_topic_.insert(
        std::make_pair(sz, str_parent_topic_ + "/" + sz));
    vecstr_robot_info_name_.push_back(sz);
  }
}

#ifdef ORIG_IMPL  // Implementation as done in Drone.py
void Drone::Impl::SetSensorTopics(World* pworld) {
  // TODO: Change this to construct this info from the published topics rather
  // than
  //  extracting it from the configuration.

  static const std::unordered_map<int, std::string> c_umcapture_name_setting = {
      {0, "scene_camera"},  // TODO: rename scene_camera topic to rgb_camera
      {1, "depth_camera"},
      {2, "depth_planar_camera"},
      {3, "segmentation_camera"},
      {4, "depth_vis_camera"},
      {5, "disparity_normalized_camera"},
      {6, "surface_normals_camera"},
  };

  auto& json_config = pworld->GetConfiguration();
  json json_actor_config;

  auto fnAddSensorInfo = [this](std::string sensor_name, std::string info_name,
                                std::string topic_root_name) {
    auto it = umsensor_name_uminfo_topic_.find(sensor_name);

    if (it == umsensor_name_uminfo_topic_.end()) {
      auto pair = umsensor_name_uminfo_topic_.emplace(sensor_name, UMStrStr());

      it = pair.first;
      vecstr_sensor_name_.push_back(sensor_name);
      umsensor_name_vecinfo_.emplace(sensor_name, VecStr());
    }

    it->second.insert(
        std::make_pair(info_name, topic_root_name + "/" + info_name));
    umsensor_name_vecinfo_[sensor_name].push_back(info_name);
  };

  umsensor_name_uminfo_topic_.clear();
  umsensor_name_vecinfo_.clear();
  vecstr_sensor_name_.clear();

  // Get the config for this drone
  if (!json_config.is_null() && json_config["actors"].is_array()) {
    for (auto& json_actor : json_config["actors"]) {
      if (json_actor["name"] == str_drone_name_) {
        json_actor_config = json_actor["robot-config"];
        break;
      }
    }
  }

  if (json_actor_config.is_null()) {
    log.ErrorF("Actor \"%s\" not found in the config", str_drone_name_.c_str());
    throw Status::NotFound;
  }

  // Loop through the robot's sensors
  if (!json_actor_config["sensors"].is_array()) return;

  for (auto& json_sensor : json_actor_config["sensors"]) {
    // Add topic entries for each recognized sensor type
    std::string str_name = json_sensor["id"];
    std::string str_sensor_type = json_sensor["type"];
    std::string str_sensor_topic_root = str_sensor_topic_ + "/" + str_name;

    if (str_sensor_type == "airspeed")
      fnAddSensorInfo(str_name, "airspeed", str_sensor_topic_root);
    else if (str_sensor_type == "barometer")
      fnAddSensorInfo(str_name, "barometer", str_sensor_topic_root);
    else if (str_sensor_type == "battery")
      fnAddSensorInfo(str_name, "battery", str_sensor_topic_root);
    else if (str_sensor_type == "camera") {
      auto json_subcameras = json_sensor["capture-settings"];

      if (json_subcameras.is_array()) {
        for (auto& json_subcamera : json_subcameras) {
          auto i_image_type = json_subcamera["image-type"];
          auto it = c_umcapture_name_setting.find(i_image_type);

          if (it != c_umcapture_name_setting.end()) {
            auto str_image_type = it->second;

            fnAddSensorInfo(str_name, str_image_type, str_sensor_topic_root);
            fnAddSensorInfo(str_name, str_image_type + "_info",
                            str_sensor_topic_root);
          }
        }
      }
    } else if (str_sensor_type == "distance-sensor")
      fnAddSensorInfo(str_name, "distance-sensor", str_sensor_topic_root);
    else if (str_sensor_type == "gps")
      fnAddSensorInfo(str_name, "gps", str_sensor_topic_root);
    else if (str_sensor_type == "imu")
      fnAddSensorInfo(str_name, "imu_kinematics", str_sensor_topic_root);
    else if (str_sensor_type == "lidar")
      fnAddSensorInfo(str_name, "lidar", str_sensor_topic_root);
    else if (str_sensor_type == "magnetometer")
      fnAddSensorInfo(str_name, "magnetometer", str_sensor_topic_root);
    else if (str_sensor_type == "radar") {
      fnAddSensorInfo(str_name, "radar_detections", str_sensor_topic_root);
      fnAddSensorInfo(str_name, "radar_tracks", str_sensor_topic_root);
    } else {
      log.ErrorF("Unknown sensor type \"%s\" found in config for sensor \"%s\"",
                 str_sensor_type.c_str(), str_name.c_str());
      throw Status::NotFound;
    }
  }
}
#endif  // ORIG_IMPL

void Drone::Impl::SetSensorTopics(World* /*pworld*/) {
  bool fdrone_found = false;
  std::regex re_drone("^/Sim/.+/robots/([^/]+)/.+$");
  std::regex re_sensor("^/Sim/.+/sensors/([^/]+)/([^/]+)$");
  auto vec_str_topic = pclient_->GetTopicInfo();
  std::unordered_set<std::string> us_sensor;

  // Clear existing sensor lists
  umsensor_name_uminfo_topic_.clear();
  umsensor_name_vecinfo_.clear();
  vecstr_sensor_name_.clear();

  // Loop through the robot's sensor topics
  for (auto& str_topic : vec_str_topic) {
    std::smatch smatch_drone;

    // Ignore topics except those for this drone
    if (std::regex_match(str_topic, smatch_drone, re_drone)) {
      std::ssub_match ssub_match_drone = smatch_drone[1];

      if (ssub_match_drone.str() == str_drone_name_) {
        std::smatch smatch_sensor;

        fdrone_found = true;

        // Ignore topics except those for the sensors
        if (std::regex_match(str_topic, smatch_sensor, re_sensor)) {
          // Add topic entries for each recognized sensor type
          std::string str_sensor_name = smatch_sensor[1].str();
          std::string str_sensor_info_name = smatch_sensor[2].str();

          // Add sensor name to list
          us_sensor.insert(str_sensor_name);

          // Add sensor info name to sensor's list
          {
            auto it_info = umsensor_name_vecinfo_.find(str_sensor_name);

            if (it_info == umsensor_name_vecinfo_.end()) {
              auto pair =
                  umsensor_name_vecinfo_.emplace(str_sensor_name, VecStr());

              it_info = pair.first;
            }
            it_info->second.push_back(str_sensor_info_name);
          }

          // Add sensor info topic to sensor's list
          {
            auto it_um_info = umsensor_name_uminfo_topic_.find(str_sensor_name);

            if (it_um_info == umsensor_name_uminfo_topic_.end()) {
              auto pair = umsensor_name_uminfo_topic_.insert(
                  std::make_pair(str_sensor_name, UMStrStr()));

              it_um_info = pair.first;
            }
            it_um_info->second.insert(
                std::make_pair(str_sensor_info_name, str_topic));
          }
        }
      }
    }
  }

  if (!fdrone_found) {
    log.ErrorF("Actor \"%s\" not found in the config", str_drone_name_.c_str());
    throw Status::NotFound;
  }
}

void Drone::Impl::SetTopics(void) {
  str_parent_topic_ = str_world_parent_topic_ + "/robots/" + str_drone_name_;
  str_sensor_topic_ = str_parent_topic_ + "/sensors";
  SetSensorTopics(pworld_.get());
  SetRobotInfoTopics();
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
