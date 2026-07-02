// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "EnvActor.h"

#include <map>
#include <string>

#include "ProjectAirsimClient.h"
#include "ProjectAirSimMessage/response_message.hpp"
#include "pch.h"

namespace microsoft {
namespace projectairsim {
namespace client {

class EnvActor::Impl {
 public:
  Impl(void) noexcept;
  ~Impl();

  Status Initialize(std::shared_ptr<Client>& pclient,
                    std::shared_ptr<World>& pworld,
                    const std::string& actor_name);
  Status SetTrajectory(const std::string& traj_name, bool to_loop,
                       float time_offset, float x_offset, float y_offset,
                       float z_offset, float roll_offset, float pitch_offset,
                       float yaw_offset);
  Status SetLinkRotationAngle(const std::string& link_name, float angle_deg);
  Status SetLinkRotationRate(const std::string& link_name,
                             float rotation_deg_per_sec);
  Status SetLinkRotationAngles(
      const std::map<std::string, float>& rotation_angle_map);
  Status SetLinkRotationRates(
      const std::map<std::string, float>& rotation_rate_map);

 protected:
  Status RequestActor(const std::string& str_method_path,
                      const json& json_params = json()) const;
  Status RequestActor(const std::string& str_method_path,
                      const json& json_params, json* pjson_out) const;
  Status RequestActor(const std::string& str_method_path,
                      const json& json_params, bool* pf_out) const;

 protected:
  std::shared_ptr<Client> pclient_;
  std::string str_actor_name_;
  std::string str_parent_topic_;        // {world}/env_actors/{name}
  std::string str_world_parent_topic_;  // {world}
};  // class EnvActor::Impl

// ---------------------------------------------------------------------------
// EnvActor public API
// ---------------------------------------------------------------------------

ASC_DECL EnvActor::EnvActor(void) noexcept : pimpl_(new Impl()) {}

ASC_DECL EnvActor::~EnvActor() {}

ASC_DECL Status EnvActor::Initialize(std::shared_ptr<Client>& pclient,
                                      std::shared_ptr<World>& pworld,
                                      const std::string& actor_name) {
  RETURN_CATCH_STATUS(pimpl_->Initialize(pclient, pworld, actor_name));
}

ASC_DECL Status EnvActor::SetTrajectory(const std::string& traj_name,
                                         bool to_loop, float time_offset,
                                         float x_offset, float y_offset,
                                         float z_offset, float roll_offset,
                                         float pitch_offset,
                                         float yaw_offset) {
  RETURN_CATCH_STATUS(pimpl_->SetTrajectory(traj_name, to_loop, time_offset,
                                            x_offset, y_offset, z_offset,
                                            roll_offset, pitch_offset,
                                            yaw_offset));
}

ASC_DECL Status EnvActor::SetLinkRotationAngle(const std::string& link_name,
                                                float angle_deg) {
  RETURN_CATCH_STATUS(pimpl_->SetLinkRotationAngle(link_name, angle_deg));
}

ASC_DECL Status EnvActor::SetLinkRotationRate(
    const std::string& link_name, float rotation_deg_per_sec) {
  RETURN_CATCH_STATUS(
      pimpl_->SetLinkRotationRate(link_name, rotation_deg_per_sec));
}

ASC_DECL Status EnvActor::SetLinkRotationAngles(
    const std::map<std::string, float>& rotation_angle_map) {
  RETURN_CATCH_STATUS(pimpl_->SetLinkRotationAngles(rotation_angle_map));
}

ASC_DECL Status EnvActor::SetLinkRotationRates(
    const std::map<std::string, float>& rotation_rate_map) {
  RETURN_CATCH_STATUS(pimpl_->SetLinkRotationRates(rotation_rate_map));
}

// ---------------------------------------------------------------------------
// EnvActor::Impl
// ---------------------------------------------------------------------------

EnvActor::Impl::Impl() noexcept
    : pclient_(),
      str_actor_name_(),
      str_parent_topic_(),
      str_world_parent_topic_() {}

EnvActor::Impl::~Impl() {}

Status EnvActor::Impl::Initialize(std::shared_ptr<Client>& pclient,
                                   std::shared_ptr<World>& pworld,
                                   const std::string& actor_name) {
  log.InfoF("Initalizing EnvActor '%s'...", actor_name.c_str());

  pclient_ = pclient;
  str_actor_name_ = actor_name;
  str_world_parent_topic_ = pworld->GetParentTopic();
  str_parent_topic_ =
      str_world_parent_topic_ + "/env_actors/" + str_actor_name_;

  log.InfoF("EnvActor '%s' initialized for World scene '%s'",
            str_actor_name_.c_str(), str_world_parent_topic_.c_str());

  return (Status::OK);
}

Status EnvActor::Impl::SetTrajectory(const std::string& traj_name,
                                      bool to_loop, float time_offset,
                                      float x_offset, float y_offset,
                                      float z_offset, float roll_offset,
                                      float pitch_offset, float yaw_offset) {
  // SetEnvActorTrajectory is dispatched on the world topic, not the actor topic
  json json_params = {
      {"env_actor_name", str_actor_name_},
      {"traj_name", traj_name},
      {"time_offset", time_offset},
      {"x_offset", x_offset},
      {"y_offset", y_offset},
      {"z_offset", z_offset},
      {"roll_offset", roll_offset},
      {"pitch_offset", pitch_offset},
      {"yaw_offset", yaw_offset},
      {"to_loop", to_loop},
  };

  return (RequestActor(str_world_parent_topic_ + "/SetEnvActorTrajectory",
                       json_params));
}

Status EnvActor::Impl::SetLinkRotationAngle(const std::string& link_name,
                                             float angle_deg) {
  json json_params = {
      {"env_actor_name", str_actor_name_},
      {"link_name", link_name},
      {"angle_deg", angle_deg},
  };
  return (RequestActor(str_world_parent_topic_ + "/SetEnvActorLinkRotAngle",
                       json_params));
}

Status EnvActor::Impl::SetLinkRotationRate(const std::string& link_name,
                                            float rotation_deg_per_sec) {
  json json_params = {
      {"env_actor_name", str_actor_name_},
      {"link_name", link_name},
      {"rotation_deg_per_sec", rotation_deg_per_sec},
  };
  return (RequestActor(str_world_parent_topic_ + "/SetEnvActorLinkRotRate",
                       json_params));
}

Status EnvActor::Impl::SetLinkRotationAngles(
    const std::map<std::string, float>& rotation_angle_map) {
  for (const auto& kv : rotation_angle_map) {
    Status status = SetLinkRotationAngle(kv.first, kv.second);
    if (status != Status::OK) return status;
  }
  return Status::OK;
}

Status EnvActor::Impl::SetLinkRotationRates(
    const std::map<std::string, float>& rotation_rate_map) {
  for (const auto& kv : rotation_rate_map) {
    Status status = SetLinkRotationRate(kv.first, kv.second);
    if (status != Status::OK) return status;
  }
  return Status::OK;
}

// ---------------------------------------------------------------------------
// Request helpers
// ---------------------------------------------------------------------------

Status EnvActor::Impl::RequestActor(const std::string& str_method_path,
                                     const json& json_params) const {
  Status status;
  Message message_response;

  status = pclient_->Request(str_method_path, json_params, &message_response);
  if (status == Status::OK) {
    ResponseMessage response_message;

    response_message.Deserialize(message_response);
    if (response_message.GetErrorCode() != 0) {
      status = Status::RejectedByServer;
      log.ErrorF(
          "Server method \"%s\" failed: error %d: %s",
          str_method_path.c_str(), response_message.GetErrorCode(),
          static_cast<std::string>(response_message.GetResult()["message"])
              .c_str());
    }
  }

  return (status);
}

Status EnvActor::Impl::RequestActor(const std::string& str_method_path,
                                     const json& json_params,
                                     json* pjson_out) const {
  Status status;
  Message message_response;

  status = pclient_->Request(str_method_path, json_params, &message_response);
  if (status == Status::OK) {
    ResponseMessage response_message;

    response_message.Deserialize(message_response);
    if (response_message.GetErrorCode() == 0)
      *pjson_out = response_message.GetResult();
    else {
      status = Status::RejectedByServer;
      log.ErrorF(
          "Server method \"%s\" failed: error %d: %s",
          str_method_path.c_str(), response_message.GetErrorCode(),
          static_cast<std::string>(response_message.GetResult()["message"])
              .c_str());
    }
  }

  return (status);
}

Status EnvActor::Impl::RequestActor(const std::string& str_method_path,
                                     const json& json_params,
                                     bool* pf_out) const {
  json json_response;
  Status status;

  status = RequestActor(str_method_path, json_params, &json_response);
  if (status == Status::OK) *pf_out = json_response;

  return (status);
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
