// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/message/response_success_message.hpp"

// #include <iostream>
#include <sstream>

#include "common_utils.hpp"
#include "json.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {
using json = nlohmann::json;

//! Message Spec:
//! {"id": 1, "results": { "res1": val1, …}, "version": 1.0}
class ResponseSuccessMessage::Impl : public MessageImpl {
 public:
  Impl();
  Impl(const int& id, const json& result, const float& version);
  ~Impl() override {}

  int GetID() const;
  json GetResult() const;
  float GetVersion() const;

  std::string Serialize() override;
  std::string SerializeToJsonStr() const;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(id, result, version)

 private:
  int id;
  JsonMsgpack result;
  float version;
};

ResponseSuccessMessage::ResponseSuccessMessage()
    : Message(std::make_shared<ResponseSuccessMessage::Impl>()) {}

ResponseSuccessMessage::ResponseSuccessMessage(const int& id,
                                               const json& result,
                                               const float& version)
    : Message(std::make_shared<ResponseSuccessMessage::Impl>(id, result,
                                                             version)) {}

ResponseSuccessMessage::~ResponseSuccessMessage() {}

int ResponseSuccessMessage::GetID() const {
  return static_cast<ResponseSuccessMessage::Impl*>(pimpl_.get())->GetID();
}

json ResponseSuccessMessage::GetResult() const {
  return static_cast<ResponseSuccessMessage::Impl*>(pimpl_.get())->GetResult();
}

float ResponseSuccessMessage::GetVersion() const {
  return static_cast<ResponseSuccessMessage::Impl*>(pimpl_.get())->GetVersion();
}

std::string ResponseSuccessMessage::Serialize() const {
  return static_cast<ResponseSuccessMessage::Impl*>(pimpl_.get())->Serialize();
}

std::string ResponseSuccessMessage::SerializeToJsonStr() const {
  return static_cast<ResponseSuccessMessage::Impl*>(pimpl_.get())
      ->SerializeToJsonStr();
}

void ResponseSuccessMessage::Deserialize(const std::string& buffer) {
  static_cast<ResponseSuccessMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

ResponseSuccessMessage::Impl::Impl()
    : MessageImpl(MessageType::kResponseSuccess) {}

ResponseSuccessMessage::Impl::Impl(const int& id, const json& result,
                                   const float& version)
    : MessageImpl(MessageType::kResponseSuccess),
      id(id),
      result(result),
      version(version) {}

int ResponseSuccessMessage::Impl::GetID() const { return id; }

json ResponseSuccessMessage::Impl::GetResult() const { return result.ToJson(); }

float ResponseSuccessMessage::Impl::GetVersion() const { return version; }

std::string ResponseSuccessMessage::Impl::Serialize() {
  // auto t1 = std::chrono::steady_clock::now();
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  std::string response_str = stream.str();
  // auto t2 = std::chrono::steady_clock::now();
  // auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  // std::cout << "Serialize() dur = " << dur.count()
  //           << " ms, len = " << response_str.length() << std::endl;
  return response_str;
}

std::string ResponseSuccessMessage::Impl::SerializeToJsonStr() const {
  // auto t1 = std::chrono::steady_clock::now();
  json response = {{"id", this->id},
                   {"result", this->result.ToJson()},
                   {"version", this->version}};
  std::string response_str = response.dump();
  // auto t2 = std::chrono::steady_clock::now();
  // auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  // std::cout << "SerializeToJsonStr() dur = " << dur.count()
  //           << " ms, len = " << response_str.length() << std::endl;
  return response_str;
}

void ResponseSuccessMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
