// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#pragma once
#include <AirSimMessage/request_message.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ASCDecl.h"
#include "AsyncResult.h"
#include "Message.h"
#include "Status.h"

namespace microsoft {
namespace projectairsim {
namespace client {

// Client connection to Project AirSim server
class Client {
 public:
  // Topic callback function prototype
  typedef std::function<void(const std::string& str_topic_name,
                             const std::string& str_message)>
      TopicCallback;

 public:
  static constexpr unsigned int kPortTopicsDefault = 8989;
  static constexpr unsigned int kPortServicesDefault = 8990;

 public:
  ASC_DECL Client(void) noexcept;
  ASC_DECL ~Client();

  // Version methods
  ASC_DECL static const char* GetNNGVersion(void);
  ASC_DECL static const char* GetVersion(void);

  // Connection methods
  ASC_DECL Status Connect(const std::string& address,
                          unsigned int port_topics = kPortTopicsDefault,
                          unsigned int port_services = kPortServicesDefault);
  ASC_DECL void Disconnect(void) noexcept;

  // Topic methods
  ASC_DECL std::vector<std::string> GetTopicInfo(void);
  ASC_DECL Status Subscribe(const char* sz_topic_name,
                            TopicCallback topic_callback);
  ASC_DECL Status Unsubscribe(const char* sz_topic_name);
  ASC_DECL Status Unsubscribe(const std::vector<std::string>& vec_topic_name);
  ASC_DECL Status UnsubscribeAll(void) noexcept;

  // Service methods
  ASC_DECL void CancelAllRequests(void) noexcept;
  ASC_DECL Status Request(const std::string& str_method,
                          const nlohmann::json& json_parameters,
                          client::Message* pmessage_response_out) noexcept;
  ASC_DECL TAsyncResult<Message> RequestAsync(
      const std::string& str_method, const nlohmann::json& json_parameters,
      FnResponseCallback fnresponsecallback = nullptr) noexcept;
  ASC_DECL Status RequestLoadScene(const std::string& str_scene_config,
                                   client::Message* pmessage_response_out);
  ASC_DECL TAsyncResult<Message> RequestPriorityAsync(
      const std::string& str_method, const nlohmann::json& json_parameters,
      FnResponseCallback fnresponse_callback) noexcept;

 protected:
  class Impl;  // Client implementation

 protected:
  std::unique_ptr<Impl> pclient_impl_;  // Pointer to implementation instance
};                                      // class Client

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
