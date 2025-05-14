// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/message/request_message.hpp"

#include <sstream>

#include "common_utils.hpp"
#include "json.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {
using json = nlohmann::json;

class RequestMessage::Impl : public MessageImpl {
 public:
  Impl();
  Impl(const int& id, const std::string& method, const json& params,
       const float& version);
  ~Impl() override {}

  int GetID() const;
  std::string GetMethod() const;
  json GetParams() const;
  float GetVersion() const;

  std::string Serialize() override;
  void Deserialize(const std::string& buffer) override;
  void DeserializeFromJsonStr(const std::string& buffer);

  MSGPACK_DEFINE_MAP(id, method, params, version)

 private:
  int id;
  std::string method;
  JsonMsgpack params;
  float version;
};

RequestMessage::RequestMessage()
    : Message(std::make_shared<RequestMessage::Impl>()) {}

RequestMessage::RequestMessage(const int& id, const std::string& method,
                               const json& params, const float& version)
    : Message(std::make_shared<RequestMessage::Impl>(id, method, params,
                                                     version)) {}

RequestMessage::~RequestMessage() {}

int RequestMessage::GetID() const {
  return static_cast<RequestMessage::Impl*>(pimpl_.get())->GetID();
}

std::string RequestMessage::GetMethod() const {
  return static_cast<RequestMessage::Impl*>(pimpl_.get())->GetMethod();
}

json RequestMessage::GetParams() const {
  return static_cast<RequestMessage::Impl*>(pimpl_.get())->GetParams();
}

float RequestMessage::GetVersion() const {
  return static_cast<RequestMessage::Impl*>(pimpl_.get())->GetVersion();
}

std::string RequestMessage::Serialize() const {
  return static_cast<RequestMessage::Impl*>(pimpl_.get())->Serialize();
}

void RequestMessage::Deserialize(const std::string& buffer) {
  static_cast<RequestMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

void RequestMessage::DeserializeFromJsonStr(const std::string& buffer) {
  static_cast<RequestMessage::Impl*>(pimpl_.get())
      ->DeserializeFromJsonStr(buffer);
}

RequestMessage::Impl::Impl() : MessageImpl(MessageType::kRequest) {}

RequestMessage::Impl::Impl(const int& id, const std::string& method,
                           const json& params, const float& version)
    : MessageImpl(MessageType::kRequest),
      id(id),
      method(method),
      params(params),
      version(version) {}

int RequestMessage::Impl::GetID() const { return id; }

std::string RequestMessage::Impl::GetMethod() const { return method; }

json RequestMessage::Impl::GetParams() const { return params.ToJson(); }

float RequestMessage::Impl::GetVersion() const { return version; }

std::string RequestMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void RequestMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

void RequestMessage::Impl::DeserializeFromJsonStr(const std::string& buffer) {
  json buffer_json = json::parse(buffer);
  this->id = buffer_json.at("id");
  this->method = buffer_json.at("method");
  this->params = JsonMsgpack(buffer_json.at("params"));
  this->version = buffer_json.at("version");
}

}  // namespace projectairsim
}  // namespace microsoft
