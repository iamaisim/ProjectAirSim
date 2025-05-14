// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_SRC_TOPIC_MANAGER_HPP_
#define CORE_SIM_SRC_TOPIC_MANAGER_HPP_

#include <functional>
#include <memory>

#include "core_sim/client_authorization.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/message/message.hpp"
#include "core_sim/topic.hpp"
#include "json.hpp"

#include "nng/nng.h"

namespace microsoft {
namespace projectairsim {

class TopicManager {
 public:
  explicit TopicManager(const Logger& logger,
                        const ClientAuthorization& client_authorization);
  explicit TopicManager(const Logger& logger);

  void Load(const nlohmann::json& json);

  bool ContainsTopic(const std::string& topic_path) const;

  void RegisterTopic(const Topic& topic,
                     std::function<void(const Topic& topic, bool is_subscribed)>
                         callback_subscribed = nullptr);

  void SubscribeTopic(const Topic& topic,
                      std::function<void(const projectairsim::Topic&,
                                         const projectairsim::Message&)>
                          callback);

  bool Unsubscribe(const std::vector<std::string>& topic_paths, nng_pipe pipe);

  void PublishTopic(const Topic& topic, const Message& message);

  void UnregisterTopic(const Topic& topic);

  void Start();

  void Stop();

  void CreateTopicList();

  void SetCallbackTopicPublished(
      const std::function<void(const std::string&, const MessageType&,
                               const std::string&)>& callback);

  void SetTopicPublishedCallbackEnabled(bool is_enabled);

  const bool IsTopicPublishedCallbackEnabled() const;

 private:
  class Impl;
  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_TOPIC_MANAGER_HPP_
