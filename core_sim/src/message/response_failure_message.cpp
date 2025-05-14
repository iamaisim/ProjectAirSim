// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/message/response_failure_message.hpp"

#include <sstream>

#include "common_utils.hpp"
#include "json.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {
using json = nlohmann::json;

//! Message Spec:
//! {"id": 1, "error": { "code": ERROR_CODE, "message": “ERROR_MSG”}
class ResponseFailureMessage::Impl : public MessageImpl {
 public:
  Impl();
  Impl(const int& id, const json& result, const float& version);
  ~Impl() override {}

  int GetID() const;
  json GetError() const;
  float GetVersion() const;

  std::string Serialize() override;
  std::string SerializeToJsonStr() const;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(id, error, version)

 private:
  int id;
  JsonMsgpack error;
  float version;
};

ResponseFailureMessage::ResponseFailureMessage()
    : Message(std::make_shared<ResponseFailureMessage::Impl>()) {}

ResponseFailureMessage::ResponseFailureMessage(const int& id, const json& error,
                                               const float& version)
    : Message(
          std::make_shared<ResponseFailureMessage::Impl>(id, error, version)) {}

ResponseFailureMessage::~ResponseFailureMessage() {}

int ResponseFailureMessage::GetID() const {
  return static_cast<ResponseFailureMessage::Impl*>(pimpl_.get())->GetID();
}

json ResponseFailureMessage::GetError() const {
  return static_cast<ResponseFailureMessage::Impl*>(pimpl_.get())->GetError();
}

float ResponseFailureMessage::GetVersion() const {
  return static_cast<ResponseFailureMessage::Impl*>(pimpl_.get())->GetVersion();
}

std::string ResponseFailureMessage::Serialize() const {
  return static_cast<ResponseFailureMessage::Impl*>(pimpl_.get())->Serialize();
}

std::string ResponseFailureMessage::SerializeToJsonStr() const {
  return static_cast<ResponseFailureMessage::Impl*>(pimpl_.get())
      ->SerializeToJsonStr();
}
void ResponseFailureMessage::Deserialize(const std::string& buffer) {
  static_cast<ResponseFailureMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

ResponseFailureMessage::Impl::Impl()
    : MessageImpl(MessageType::kResponseFailure) {}

ResponseFailureMessage::Impl::Impl(const int& id, const json& error,
                                   const float& version)
    : MessageImpl(MessageType::kResponseFailure),
      id(id),
      error(error),
      version(version) {}

int ResponseFailureMessage::Impl::GetID() const { return id; }

json ResponseFailureMessage::Impl::GetError() const { return error.ToJson(); }

float ResponseFailureMessage::Impl::GetVersion() const { return version; }

std::string ResponseFailureMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

std::string ResponseFailureMessage::Impl::SerializeToJsonStr() const {
  json response = {{"id", this->id},
                   {"error", this->error.ToJson()},
                   {"version", this->version}};
  return response.dump();
}

void ResponseFailureMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
