// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once
#include <functional>
#include <memory>
#include <msgpack.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Client.h"
#include "ICommChannel.h"
#include "Status.h"

namespace microsoft {
namespace projectairsim {
namespace client {
namespace internal {

class Topics {
 protected:
  typedef unsigned int SubscriptionID;
  class SubscriptionHub;

 public:
  // Subscribers hold onto this token as long as they want the subscription.
  // Destroying this object unsubscribes the subscriber.
  class SubscriptionToken {
   public:
    ~SubscriptionToken(void);

   protected:
    friend class Topics;

   protected:
    SubscriptionToken(
        std::shared_ptr<SubscriptionHub>& psubscription_hub) noexcept;

   protected:
    void Invalidate(void) noexcept;

   protected:
    std::shared_ptr<SubscriptionHub>
        psubscription_hub_;  // Subscription hub that handles our request to
                             // unsubscribe
  };  // class SubscriptionToken

 public:
  Topics(std::shared_ptr<ICommChannel> pICommChannel) noexcept;
  ~Topics();

  std::vector<std::string> GetAll(void);
  void InvalidateAllTopics(void);
  void InvalidateTopics(const std::vector<std::string>& vec_topic_name);
  Status StartReceiving(void);
  void StopReceiving(void);
  Status Subscribe(const char* sz_topic_name,
                   Client::TopicCallback topic_callback,
                   std::unique_ptr<SubscriptionToken>* ppsubscriptiontokenOut);
  Status Unsubscribe(const char* sz_topic_name);

 protected:
  // This class decouples the topics object and subscription tokens.
  // Subscription tokens have a shared pointer to an instance of this class
  // and they work through it when the token is destroyed.  The parent
  // topics object can be destroyed independently of the tokens.
  class SubscriptionHub {
   public:
    SubscriptionHub(Topics* ptopics_parent) noexcept;

    void AddToken(SubscriptionToken* psubscription_token,
                  const std::string& topic_name,
                  SubscriptionID subscription_id);
    void Disconnect(void);
    void InvalidateAllTokens(void);
    void InvalidateTokensForTopic(const std::string& topic_name);
    void Unsubscribe(SubscriptionToken* psubscriptiontoken);

   protected:
    struct TokenEntry {
      SubscriptionID subscription_id;  // This subscription's ID
      std::string topic_name;          // This subscription's topic name

      TokenEntry(const std::string& topic_name_in,
                 SubscriptionID subscription_id_in)
          : subscription_id(subscription_id_in), topic_name(topic_name_in) {}
    };  // struct TokenEntry

   protected:
    Topics* ptopics_parent_;  // Topics object that manages the subscription
    std::unordered_map<SubscriptionToken*, TokenEntry>
        um_psubscription_token_token_entry_;  // The list of active tokens
  };                                          // class SubscriptionHub

  friend SubscriptionHub;

  // Entry for each callback registered for a topic
  struct CallbackEntry {
    Client::TopicCallback topic_callback;  // Callback for this subscription
    SubscriptionID
        subscription_id;  // Unique ID withing the topic of this subscription

    CallbackEntry(Client::TopicCallback topic_callback_in,
                  SubscriptionID subscription_id_in)
        : topic_callback(topic_callback_in),
          subscription_id(subscription_id_in) {}
  };  // struct CallbackEntry

  // Entry for each topic for which there is at least one callback
  struct TopicEntry {
    std::vector<CallbackEntry>
        vec_callback;  // Callbacks registered for this topic
    SubscriptionID subscription_id_next = 0;  // Next unassigned subscription ID
  };                                          // struct TopicEntry

  typedef std::unordered_map<std::string, TopicEntry> UMStrTopicEntry;

 protected:
  // Name of topic that returns all published topics
  static const std::string kStrTopicNameTopicList;

 protected:
  void CleanupReceiveThread(void);
  void ProcessTopicList(const std::string& str_message);
  Status RequestSubscribe(const std::string& str_topic_name);
  Status RequestUnsubscribe(const std::string& str_topic_name);
  void TopicReceiveThreadProc(void);
  void Unsubscribe(const std::string& topic_name,
                   SubscriptionID subscription_id);

 protected:
  std::condition_variable
      cv_topic_info_;  // Condition variable for topic_info_is_received_
  bool fis_running_;  // If true, we're receiving topic messages from the server
  bool ftopic_info_is_received_;  // If true, the topic list has been received
  std::mutex mutex_topic_info_;   // Access guard to topic_info_is_received
  std::mutex mutex_um_topic_;     // Access guard to um_str_topic_entry
  std::shared_ptr<ICommChannel>
      piserver_channel_;  // Object for communicating with server over the
                          // topics channel
  std::shared_ptr<SubscriptionHub> psubscription_hub_;  // Subscription token
                                                        // hub
  std::unique_ptr<SubscriptionToken>
      psubscription_token_topic_list_;  // Subscription token for topic list
  std::unique_ptr<std::thread>
      pthread_topic_receive_;  // Topic receive and dispatch thread
  UMStrTopicEntry
      um_str_topic_entry_;  // Mapping from topic name to entry for the topic
  std::vector<std::string>
      vec_string_topics_;  // List of all topics published by server
};                         // class Topics

}  // namespace internal
}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
