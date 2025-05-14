// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_REQUEST_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_REQUEST_MESSAGE_HPP_
#include <string>

#include "message.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class RequestMessage : public Message {
 public:
  RequestMessage(const int& id, const std::string& method, const json& params,
                 const float& version);
  RequestMessage();
  ~RequestMessage() override;

  int GetID() const;
  std::string GetMethod() const;
  json GetParams() const;
  float GetVersion() const;

  std::string Serialize() const override;
  void Deserialize(const std::string& buffer) override;

  void DeserializeFromJsonStr(const std::string& buffer);

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_REQUEST_MESSAGE_HPP_
