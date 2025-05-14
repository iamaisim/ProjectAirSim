// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_RESPONSE_FAILURE_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_RESPONSE_FAILURE_MESSAGE_HPP_
#include <string>

#include "core_sim/message/message.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

//! Message Spec:
//! {"id": 1, "error": { "code": ERRO_CODE, "message": “ERROR_MSG”}
class ResponseFailureMessage : public Message {
 public:
  ResponseFailureMessage(const int& id, const json& error,
                         const float& version);
  ResponseFailureMessage();
  ~ResponseFailureMessage() override;

  int GetID() const;
  json GetError() const;
  float GetVersion() const;

  std::string Serialize() const override;
  std::string SerializeToJsonStr() const;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_RESPONSE_FAILURE_MESSAGE_HPP_
