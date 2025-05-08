// Copyright (C) Microsoft Corporation. All rights reserved.

#include "response_message.hpp"

#include <core_sim/message/response_failure_message.hpp>
#include <core_sim/message/response_success_message.hpp>
#include <core_sim/src/message/common_utils.hpp>
#include <core_sim/src/message/message_impl.hpp>
#include <sstream>

#include "json.hpp"
#include "msgpack.hpp"
#include "pch.h"

using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {

//! Message Spec success:
//! {"id": 1, "results": { "res1": val1, …}, "version": 1.0}
//! Message Spec failure:
//! {"id": 1, "error": { "code": ERROR_CODE, "message": “ERROR_MSG”}}
class ResponseMessage::Impl : public MessageImpl {
 public:
  Impl();
  Impl(const int& id, const json& result, const float& version);
  Impl(const int& id, int error_code, const std::string& error_msg,
       const float& version);
  ~Impl() override {}

  int GetID() const;
  int GetErrorCode() const;
  json GetResult() const;
  float GetVersion() const;

  std::string Serialize() override;
  std::string SerializeToJsonStr() const;

  void Deserialize(const std::string& buffer) override;

 private:
  int id;              // ID of initiating request
  int error_code;      // Error code, 0 if success response
  JsonMsgpack result;  // If error_code == 0, result, error info otherwise
  float version;       // Client API version
};                     // class ResponseMessage::Impl

ResponseMessage::ResponseMessage()
    : Message(std::make_shared<ResponseMessage::Impl>()) {}

ResponseMessage::ResponseMessage(const int& id, const json& error,
                                 const float& version)
    : Message(std::make_shared<ResponseMessage::Impl>(id, error, version)) {}

ResponseMessage::ResponseMessage(const int& id, int error_code,
                                 const std::string& error_msg,
                                 const float& version)
    : Message(std::make_shared<ResponseMessage::Impl>(id, error_code, error_msg,
                                                      version)) {}

ResponseMessage::~ResponseMessage() {}

void ResponseMessage::Deserialize(const std::string& buffer) {
  static_cast<ResponseMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

int ResponseMessage::GetID(void) const {
  return static_cast<ResponseMessage::Impl*>(pimpl_.get())->GetID();
}

int ResponseMessage::GetErrorCode(void) const {
  return static_cast<ResponseMessage::Impl*>(pimpl_.get())->GetErrorCode();
}

json ResponseMessage::GetResult(void) const {
  return static_cast<ResponseMessage::Impl*>(pimpl_.get())->GetResult();
}

float ResponseMessage::GetVersion(void) const {
  return static_cast<ResponseMessage::Impl*>(pimpl_.get())->GetVersion();
}

std::string ResponseMessage::Serialize(void) const {
  return static_cast<ResponseMessage::Impl*>(pimpl_.get())->Serialize();
}

std::string ResponseMessage::SerializeToJsonStr(void) const {
  return static_cast<ResponseMessage::Impl*>(pimpl_.get())
      ->SerializeToJsonStr();
}

ResponseMessage::Impl::Impl() : MessageImpl(MessageType::kResponseFailure) {}

ResponseMessage::Impl::Impl(const int& id, const json& result,
                            const float& version)
    : MessageImpl(MessageType::kResponseFailure),
      id(id),
      error_code(0),
      result(result),
      version(version) {}

ResponseMessage::Impl::Impl(const int& id, int error_code,
                            const std::string& error_msg, const float& version)
    : MessageImpl(MessageType::kResponseFailure),
      id(id),
      error_code(error_code),
      result({{"code", (float)error_code}, {"message", error_msg}}),
      version(version) {}

int ResponseMessage::Impl::GetID(void) const { return id; }

int ResponseMessage::Impl::GetErrorCode(void) const { return error_code; }

json ResponseMessage::Impl::GetResult(void) const { return result.ToJson(); }

float ResponseMessage::Impl::GetVersion(void) const { return version; }

std::string ResponseMessage::Impl::Serialize(void) {
  if (error_code == 0)
    return ResponseSuccessMessage(id, result.ToJson(), version).Serialize();
  else
    return ResponseFailureMessage(id, result.ToJson(), version).Serialize();
}

std::string ResponseMessage::Impl::SerializeToJsonStr(void) const {
  if (error_code == 0)
    return ResponseSuccessMessage(id, result.ToJson(), version)
        .SerializeToJsonStr();
  else
    return ResponseFailureMessage(id, result.ToJson(), version)
        .SerializeToJsonStr();
}

void ResponseMessage::Impl::Deserialize(const std::string& buffer) {
  bool fIsSuccess = false;

  // We expect the buffer to contain a ResponseSuccessMessage or
  // ResponseFailureMessage.  Unfortunately we can't tell which from the
  // buffer so we'll just try to deserialize as one, and if that fails,
  // as the other.

  // Try to deserialize as a success message
  try {
    projectairsim::ResponseSuccessMessage rsm;

    rsm.Deserialize(buffer);
    id = rsm.GetID();
    error_code = 0;
    result = JsonMsgpack(rsm.GetResult());
    version = rsm.GetVersion();

    fIsSuccess = true;
  } catch (...) {
  }

  // Didn't work?  Try to deserialize as a failure message
  if (!fIsSuccess) {
    projectairsim::ResponseFailureMessage rfm;

    rfm.Deserialize(buffer);
    id = rfm.GetID();
    version = rfm.GetVersion();

    {
      auto json_error = rfm.GetError();

      error_code = json_error.contains("code") ? (int)json_error["code"] : 1;
      result = JsonMsgpack(json_error);
    }
  }
}

}  // namespace projectairsim
}  // namespace microsoft
