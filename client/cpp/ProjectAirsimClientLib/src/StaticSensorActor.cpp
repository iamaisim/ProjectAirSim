// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "StaticSensorActor.h"

#include <string>
#include <vector>

#include "ProjectAirsimClient.h"
#include "ProjectAirSimMessage/response_message.hpp"
#include "pch.h"

namespace microsoft {
namespace projectairsim {
namespace client {

class StaticSensorActor::Impl {
 public:
  Impl(void) noexcept;
  ~Impl();

  Status Initialize(std::shared_ptr<Client>& pclient,
                    std::shared_ptr<World>& pworld,
                    const std::string& actor_name);
  Status GetImages(const std::string& str_camera_id,
                   const std::vector<int>& veci_image_type_id,
                   json* pjson_out) const;

 protected:
  Status RequestMethod(const std::string& str_method_path,
                       const json& json_params, json* pjson_out) const;

 protected:
  std::shared_ptr<Client> pclient_;
  std::string str_actor_name_;
  std::string str_parent_topic_;   // {world}/robots/{name}
  std::string str_sensor_topic_;   // {world}/robots/{name}/sensors
};  // class StaticSensorActor::Impl

// ---------------------------------------------------------------------------
// StaticSensorActor public API
// ---------------------------------------------------------------------------

ASC_DECL StaticSensorActor::StaticSensorActor(void) noexcept
    : pimpl_(new Impl()) {}

ASC_DECL StaticSensorActor::~StaticSensorActor() {}

ASC_DECL Status StaticSensorActor::Initialize(
    std::shared_ptr<Client>& pclient, std::shared_ptr<World>& pworld,
    const std::string& actor_name) {
  RETURN_CATCH_STATUS(pimpl_->Initialize(pclient, pworld, actor_name));
}

ASC_DECL Status StaticSensorActor::GetImages(
    const std::string& str_camera_id,
    const std::vector<int>& veci_image_type_id, json* pjson_out) const {
  RETURN_CATCH_STATUS(
      pimpl_->GetImages(str_camera_id, veci_image_type_id, pjson_out));
}

// ---------------------------------------------------------------------------
// StaticSensorActor::Impl
// ---------------------------------------------------------------------------

StaticSensorActor::Impl::Impl() noexcept
    : pclient_(),
      str_actor_name_(),
      str_parent_topic_(),
      str_sensor_topic_() {}

StaticSensorActor::Impl::~Impl() {}

Status StaticSensorActor::Impl::Initialize(std::shared_ptr<Client>& pclient,
                                            std::shared_ptr<World>& pworld,
                                            const std::string& actor_name) {
  log.InfoF("Initalizing StaticSensorActor '%s'...", actor_name.c_str());

  pclient_ = pclient;
  str_actor_name_ = actor_name;
  const std::string world_topic = pworld->GetParentTopic();
  str_parent_topic_ = world_topic + "/robots/" + str_actor_name_;
  str_sensor_topic_ = str_parent_topic_ + "/sensors";

  log.InfoF("StaticSensorActor '%s' initialized for World scene '%s'",
            str_actor_name_.c_str(), world_topic.c_str());

  return (Status::OK);
}

Status StaticSensorActor::Impl::GetImages(
    const std::string& str_camera_id,
    const std::vector<int>& veci_image_type_id, json* pjson_out) const {
  json json_params = {{"image_type_ids", veci_image_type_id}};

  return (RequestMethod(
      str_sensor_topic_ + "/" + str_camera_id + "/GetImages", json_params,
      pjson_out));
}

Status StaticSensorActor::Impl::RequestMethod(const std::string& str_method_path,
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

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
