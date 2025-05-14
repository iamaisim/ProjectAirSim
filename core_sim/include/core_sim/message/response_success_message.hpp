// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_RESPONSE_SUCCESS_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_RESPONSE_SUCCESS_MESSAGE_HPP_
#include <string>

#include "core_sim/message/message.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

//! Message Spec:
//! {"id": 1, "result": { "res1": val1, …, "version": 1.0}
class ResponseSuccessMessage : public Message {
 public:
  ResponseSuccessMessage(const int& id, const json& result,
                         const float& version);
  ResponseSuccessMessage();
  ~ResponseSuccessMessage() override;

  int GetID() const;
  json GetResult() const;
  float GetVersion() const;

  std::string Serialize() const override;
  std::string SerializeToJsonStr() const;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_RESPONSE_SUCCESS_MESSAGE_HPP_
