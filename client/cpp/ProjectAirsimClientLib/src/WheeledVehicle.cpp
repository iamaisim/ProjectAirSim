// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "WheeledVehicle.h"

#include "ProjectAirSimMessage/response_message.hpp"
#include "ProjectAirsimClient.h"
#include "pch.h"

namespace microsoft {
namespace projectairsim {
namespace client {

class WheeledVehicle::Impl {
 public:
  Impl(void) noexcept;
  ~Impl();

  Status Initialize(std::shared_ptr<Client>& pclient,
                    std::shared_ptr<World>& pworld,
                    const std::string& vehicle_name);
  Status SetThrottle(float value, bool* pf_success_out);
  Status SetSteering(float value, bool* pf_success_out);
  Status SetBrakes(float value, bool* pf_success_out);
  Status GetKinematics(json* pjson_out) const;
  Status GetGroundTruthKinematics(json* pjson_out) const;

 private:
  Status RequestVehicle(const std::string& method, const json& params,
                        bool* pf_out) const;
  Status RequestVehicle(const std::string& method, const json& params,
                        json* pjson_out) const;

  std::shared_ptr<Client> pclient_;
  std::shared_ptr<World> pworld_;
  std::string vehicle_name_;
  std::string parent_topic_;
};  // class WheeledVehicle::Impl

ASC_DECL WheeledVehicle::WheeledVehicle(void) noexcept : pimpl_(new Impl()) {}

ASC_DECL WheeledVehicle::~WheeledVehicle() {}

ASC_DECL Status WheeledVehicle::Initialize(std::shared_ptr<Client>& pclient,
                                           std::shared_ptr<World>& pworld,
                                           const std::string& vehicle_name) {
  RETURN_CATCH_STATUS(pimpl_->Initialize(pclient, pworld, vehicle_name));
}

ASC_DECL Status WheeledVehicle::SetThrottle(float value,
                                            bool* pf_success_out) {
  RETURN_CATCH_STATUS(pimpl_->SetThrottle(value, pf_success_out));
}

ASC_DECL Status WheeledVehicle::SetSteering(float value,
                                            bool* pf_success_out) {
  RETURN_CATCH_STATUS(pimpl_->SetSteering(value, pf_success_out));
}

ASC_DECL Status WheeledVehicle::SetBrakes(float value,
                                          bool* pf_success_out) {
  RETURN_CATCH_STATUS(pimpl_->SetBrakes(value, pf_success_out));
}

ASC_DECL Status WheeledVehicle::GetKinematics(json* pjson_out) const {
  RETURN_CATCH_STATUS(pimpl_->GetKinematics(pjson_out));
}

ASC_DECL Status
WheeledVehicle::GetGroundTruthKinematics(json* pjson_out) const {
  RETURN_CATCH_STATUS(pimpl_->GetGroundTruthKinematics(pjson_out));
}

WheeledVehicle::Impl::Impl(void) noexcept = default;

WheeledVehicle::Impl::~Impl() = default;

Status WheeledVehicle::Impl::Initialize(std::shared_ptr<Client>& pclient,
                                        std::shared_ptr<World>& pworld,
                                        const std::string& vehicle_name) {
  log.InfoF("Initializing WheeledVehicle '%s'...", vehicle_name.c_str());

  pclient_ = pclient;
  pworld_ = pworld;
  vehicle_name_ = vehicle_name;
  parent_topic_ =
      std::string(pworld_->GetParentTopic()) + "/robots/" + vehicle_name_;

  log.InfoF("WheeledVehicle '%s' initialized for World scene '%s'",
            vehicle_name_.c_str(), pworld_->GetParentTopic());
  return Status::OK;
}

Status WheeledVehicle::Impl::SetThrottle(float value, bool* pf_success_out) {
  return RequestVehicle("SetThrottle", {{"value", value}}, pf_success_out);
}

Status WheeledVehicle::Impl::SetSteering(float value, bool* pf_success_out) {
  return RequestVehicle("SetSteering", {{"value", value}}, pf_success_out);
}

Status WheeledVehicle::Impl::SetBrakes(float value, bool* pf_success_out) {
  return RequestVehicle("SetBrakes", {{"value", value}}, pf_success_out);
}

Status WheeledVehicle::Impl::GetKinematics(json* pjson_out) const {
  return GetGroundTruthKinematics(pjson_out);
}

Status WheeledVehicle::Impl::GetGroundTruthKinematics(json* pjson_out) const {
  return RequestVehicle("GetGroundTruthKinematics", json(), pjson_out);
}

Status WheeledVehicle::Impl::RequestVehicle(const std::string& method,
                                            const json& params,
                                            bool* pf_out) const {
  json response;
  const Status status = RequestVehicle(method, params, &response);
  if (status == Status::OK) *pf_out = response;
  return status;
}

Status WheeledVehicle::Impl::RequestVehicle(const std::string& method,
                                            const json& params,
                                            json* pjson_out) const {
  Message message;
  Status status =
      pclient_->Request(parent_topic_ + "/" + method, params, &message);
  if (status != Status::OK) return status;

  ResponseMessage response;
  response.Deserialize(message);
  if (response.GetErrorCode() == 0) {
    *pjson_out = response.GetResult();
    return Status::OK;
  }

  log.ErrorF("Server method \"%s\" failed: error %d: %s", method.c_str(),
             response.GetErrorCode(),
             static_cast<std::string>(response.GetResult()["message"]).c_str());
  return Status::RejectedByServer;
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
