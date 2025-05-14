// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#pragma once
#include <core_sim/message/message.hpp>



namespace microsoft {
namespace projectairsim {


// Project AirSim service method calls return a ResponseSuccessMessage on success
// and a ResponseFailureMessage on failure.  There a catch-22 in that you can only
// deserialize the packed reply stream with the correct message class but can't
// determine which message was returned without deserializing it first.  Project
// AirSim clients can use this class to handle that issue, which deserializes
// either message and returns the response information.
class ResponseMessage : public Message {
 public:
  ResponseMessage(const int& id, const nlohmann::json& result, const float& version);
  ResponseMessage(const int& id, int error_code, const std::string& error_msg,
                         const float& version);
  ResponseMessage(void);
  ~ResponseMessage() override;

  //The ID of the original request that prompted this response
  int GetID(void) const;

  //The error code.  This returns zero if the response is success
  int GetErrorCode(void) const;

  //Response if GetErrorCode() returns zero, error info otherwise
  nlohmann::json GetResult(void) const;

  //Project AirSim client protocol version
  float GetVersion(void) const;

  //Deserialize from transmission stream
  void Deserialize(const std::string& buffer) override;
  
  //Serialize to transmission stream or JSON string
  std::string Serialize(void) const override;
  std::string SerializeToJsonStr(void) const;


 private:
  class Impl;
}; // class ResponseMessage

}  // namespace projectairsim
}  // namespace microsoft

