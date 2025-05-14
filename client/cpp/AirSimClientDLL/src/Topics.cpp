// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "Topics.h"

#include <chrono>

#include "AirSimClient.h"
#include "pch.h"

using namespace std::chrono_literals;

namespace microsoft {
namespace projectairsim {
namespace client {
namespace internal {

// Topic message frame type
enum class FrameType : int { kSubscribe = 0, kUnsubscribe = 1, kMessage = 2 };

// Topic message frame
struct Frame {
  FrameType type;
  std::string topic;
  std::string body;

  MSGPACK_DEFINE(type, topic, body);
};  // struct Frame

// Topic info--one entry per topic being published by the server via "/$Topic"
// topic
struct TopicInfo {
  std::string path;
  std::string type;
  std::string message_type;
  int frequency;

  MSGPACK_DEFINE_MAP(path, type, message_type, frequency);
};  // struct TopicInfo

const std::string Topics::kStrTopicNameTopicList = "/$topics";

Topics::Topics(std::shared_ptr<ICommChannel> piserver_channel) noexcept
    : cv_topic_info_(),
      fis_running_(false),
      ftopic_info_is_received_(false),
      mutex_topic_info_(),
      mutex_um_topic_(),
      piserver_channel_(piserver_channel),
      psubscription_hub_(new SubscriptionHub(this)),
      psubscription_token_topic_list_(),
      pthread_topic_receive_(),
      um_str_topic_entry_(),
      vec_string_topics_() {}

Topics::~Topics() {
  StopReceiving();

  if (psubscription_hub_ != nullptr) {
    psubscription_hub_->Disconnect();
    psubscription_hub_.reset();
  }

  CleanupReceiveThread();
}

void Topics::CleanupReceiveThread(void) {
  // Wait for receive thread to exit
  if (pthread_topic_receive_ != nullptr) {
    pthread_topic_receive_->join();
    pthread_topic_receive_.reset();
  }
}

std::vector<std::string> Topics::GetAll(void) {
  static const auto duration_timeout = std::chrono::duration<int>(1);

  std::unique_lock ul(mutex_topic_info_);
  auto fn_can_proceed = [this](void) {
    return (ftopic_info_is_received_ || !fis_running_);
  };

  while (!fn_can_proceed()) {
    auto time_now = std::chrono::system_clock::now();

    // Wait until the topics info have been received.  Periodically
    // wait up to check whether we're still running.
    (void)cv_topic_info_.wait_until(ul, time_now + 1s, fn_can_proceed);
  }

  return (vec_string_topics_);
}

void Topics::InvalidateAllTopics(void) {
  std::lock_guard ul(mutex_um_topic_);

  psubscription_hub_->InvalidateAllTokens();
  um_str_topic_entry_.clear();
}

void Topics::InvalidateTopics(const std::vector<std::string>& vec_topic_name) {
  std::lock_guard ul(mutex_um_topic_);

  for (auto& str_topic : vec_topic_name) {
    // Invalidate the associated tokens
    psubscription_hub_->InvalidateTokensForTopic(str_topic);

    // Remove the entry for the topic
    {
      auto it = um_str_topic_entry_.find(str_topic);

      if (it != um_str_topic_entry_.end()) um_str_topic_entry_.erase(it);
    }
  }
}

void Topics::ProcessTopicList(const std::string& str_message) {
  auto object_handle_topicinfos =
      msgpack::unpack(str_message.data(), str_message.size());
  msgpack::object objtopicinfos = object_handle_topicinfos.get();
  std::vector<TopicInfo> vectopicinfo;

  objtopicinfos.convert(vectopicinfo);

  // log.InfoF("There are %d topics:", vectopicinfo.size());
  vec_string_topics_.clear();
  for (auto& topicinfo : vectopicinfo) {
    // log.InfoF("  %s", topicinfo.path.c_str());
    vec_string_topics_.push_back(topicinfo.path);
  }

  {
    std::lock_guard ul(mutex_topic_info_);

    ftopic_info_is_received_ = true;
    cv_topic_info_.notify_all();
  }
}

Status Topics::RequestSubscribe(const std::string& str_topic_name) {
  Frame frame;
  msgpack::sbuffer sbuffer;

  // Create frame to subscribe to the topic
  frame.type = FrameType::kSubscribe;
  frame.topic = str_topic_name;

  // Send the frame to the server
  msgpack::pack(sbuffer, frame);
  return (piserver_channel_->Send(reinterpret_cast<uint8_t*>(sbuffer.data()),
                                  sbuffer.size()));
}

Status Topics::RequestUnsubscribe(const std::string& str_topic_name) {
  if (piserver_channel_ == nullptr) return (Status::Closed);

  Frame frame;
  msgpack::sbuffer sbuffer;

  // Create frame for unsubscribing from a topic
  frame.type = FrameType::kUnsubscribe;
  frame.topic = str_topic_name;

  // Send the frame to the server
  msgpack::pack(sbuffer, frame);

  return (piserver_channel_->Send(reinterpret_cast<uint8_t*>(sbuffer.data()),
                                  sbuffer.size()));
}

Status Topics::StartReceiving(void) {
  if (!fis_running_) {
    CleanupReceiveThread();

    fis_running_ = true;
    ftopic_info_is_received_ = false;
    pthread_topic_receive_ =
        std::make_unique<std::thread>([this]() { TopicReceiveThreadProc(); });

    RequestSubscribe(kStrTopicNameTopicList);
  }

  return (Status::OK);
}

void Topics::StopReceiving(void) {
  if (fis_running_) {
    fis_running_ = false;

    // For expediency, we'll request unsubscribing for each topic and
    // clear the subscriptions en-masse
    RequestUnsubscribe(kStrTopicNameTopicList);

    // Clear all subscriptions
    {
      std::lock_guard<std::mutex> lg(mutex_um_topic_);

      for (auto& pair : um_str_topic_entry_) RequestUnsubscribe(pair.first);
      um_str_topic_entry_.clear();
    }

    psubscription_hub_->InvalidateAllTokens();
    psubscription_token_topic_list_.reset();

    // Release anybody waiting
    cv_topic_info_.notify_all();
  }
}

Status Topics::Subscribe(
    const char* sz_topic_name, Client::TopicCallback topic_callback,
    std::unique_ptr<SubscriptionToken>* ppsubscriptiontokenOut) {
  bool fis_first = true;
  UMStrTopicEntry::iterator it_topic_entry;
  Status status = Status::OK;
  std::string str_topic_name(sz_topic_name);
  SubscriptionID subscriptionid;

  // Add the topic callback to the topic
  {
    TopicEntry* ptopic_entry;
    std::lock_guard<std::mutex> lg(mutex_um_topic_);

    it_topic_entry = um_str_topic_entry_.find(str_topic_name);
    if (it_topic_entry == um_str_topic_entry_.end()) {
      auto pair = um_str_topic_entry_.emplace(
          std::make_pair(std::string(sz_topic_name), TopicEntry()));

      it_topic_entry = pair.first;
      fis_first = true;
    }

    ptopic_entry = &it_topic_entry->second;
    subscriptionid = ptopic_entry->subscription_id_next++;
    ptopic_entry->vec_callback.push_back(
        CallbackEntry(topic_callback, subscriptionid));
  }

  // If it's the first subscriber, request subscription from server
  if (fis_first) status = RequestSubscribe(sz_topic_name);

  if (status != Status::OK) {
    // The subscription request failed--remove the topic callback
    auto ptopic_entry = &it_topic_entry->second;
    std::lock_guard<std::mutex> lg(mutex_um_topic_);

    ptopic_entry->vec_callback.pop_back();
    if (ptopic_entry->vec_callback.empty())
      um_str_topic_entry_.erase(it_topic_entry);
  } else {
    // Create subscription token
    ppsubscriptiontokenOut->reset(new SubscriptionToken(psubscription_hub_));
    psubscription_hub_->AddToken(ppsubscriptiontokenOut->get(), str_topic_name,
                                 subscriptionid);
  }

  return (status);
}

void Topics::TopicReceiveThreadProc(void) {
  // log.Info("TopicReceiveThreadProc() starting.");

  while (fis_running_) {
    client::Message message;
    Status status = Status::OK;

    // Read the next message.  The timeout allows us to check whether
    // we should keep running.
    status = piserver_channel_->Receive(&message, 1000);

    if (!fis_running_) {
      // Note that Receive() may have returned an error that we're ignoring
      // because it was interrupted when the channel was deliberately closed
      // or the read timed-out.
      break;
    }

    if (status == Status::TimedOut)
      continue;
    else if (status != Status::OK) {
      char sz[256];

      GetStatusString(status, sz);
      log.ErrorF(
          "TopicReceiveThreadProc(): ICommChannel::Receive() returned error: "
          "%s",
          sz);
    } else {
      Frame frame;
      auto object_handle = msgpack::unpack(
          reinterpret_cast<const char*>(message.Data()), message.Cb());
      msgpack::object object = object_handle.get();

      // log.InfoF("Topic received: %u bytes, root object is type %d",
      // message.Cb(), object.type);

      object.convert(frame);
      // log.InfoF("Topic received: Frame type is %d, topic is \"%s\"",
      // (int)frame.type, frame.topic.c_str());

      // Do special processing for the available topic list
      if (frame.topic == kStrTopicNameTopicList) ProcessTopicList(frame.body);

      // Invoke associated topic callbacks
      {
        std::lock_guard<std::mutex> lg(mutex_um_topic_);
        auto it = um_str_topic_entry_.find(frame.topic);

        if (it != um_str_topic_entry_.end()) {
          for (auto& callback_entry : it->second.vec_callback)
            callback_entry.topic_callback(frame.topic, frame.body);
        }
      }
    }
  }

  // log.Info("TopicReceiveThreadProc() exiting.");
}

Status Topics::Unsubscribe(const char* sz_topic_name) {
  Status status = Status::OK;

  // Invalidate any subscription tokens for the topic
  if (psubscription_hub_ != nullptr)
    psubscription_hub_->InvalidateTokensForTopic(sz_topic_name);

  if (kStrTopicNameTopicList != sz_topic_name) {
    // Remove the topic entry
    {
      std::lock_guard<std::mutex> lg(mutex_um_topic_);
      um_str_topic_entry_.erase(sz_topic_name);
    }

    // Remove the subscription from the server
    status = RequestUnsubscribe(sz_topic_name);
  }

  return (status);
}

void Topics::Unsubscribe(const std::string& topic_name,
                         SubscriptionID subscription_id) {
  // Note that this is called when the subscription token is deleted so we don't
  // need to invalidate the token.

  std::lock_guard<std::mutex> lg(mutex_um_topic_);

  // Find the topic entry
  auto it = um_str_topic_entry_.find(topic_name);

  if (it != um_str_topic_entry_.end()) {
    auto& vec_callback = it->second.vec_callback;

    // Find the matching callback entry
    for (auto it_callback = vec_callback.begin(),
              it_callback_end = vec_callback.end();
         it_callback != it_callback_end; ++it_callback) {
      if (it_callback->subscription_id == subscription_id) {
        // Remove the entry
        vec_callback.erase(it_callback);
        break;
      }
    }

    // If there's no more callbacks, unsubscribe from the topic
    if (vec_callback.empty() && (topic_name != kStrTopicNameTopicList)) {
      um_str_topic_entry_.erase(it);
      RequestUnsubscribe(topic_name);
    }
  }
}

Topics::SubscriptionHub::SubscriptionHub(Topics* ptopics_parent) noexcept
    : ptopics_parent_(ptopics_parent), um_psubscription_token_token_entry_() {}

void Topics::SubscriptionHub::AddToken(SubscriptionToken* psubscription_token,
                                       const std::string& topic_name,
                                       SubscriptionID subscription_id) {
  um_psubscription_token_token_entry_.insert(std::make_pair(
      psubscription_token, TokenEntry(topic_name, subscription_id)));
}

void Topics::SubscriptionHub::Disconnect(void) {
  InvalidateAllTokens();
  ptopics_parent_ = nullptr;
}

void Topics::SubscriptionHub::InvalidateAllTokens(void) {
  for (auto& pair : um_psubscription_token_token_entry_)
    pair.first->Invalidate();

  um_psubscription_token_token_entry_.clear();
}

void Topics::SubscriptionHub::InvalidateTokensForTopic(
    const std::string& topic_name) {
  std::vector<SubscriptionToken*> vec_psubscription_token;

  // Find tokens for the topic and invalidate them
  for (auto& pair : um_psubscription_token_token_entry_) {
    if (pair.second.topic_name == topic_name) {
      pair.first->Invalidate();
      vec_psubscription_token.push_back(pair.first);
    }
  }

  // Remove the matching tokens from the map
  for (auto psubscription_token : vec_psubscription_token)
    um_psubscription_token_token_entry_.erase(psubscription_token);
}

void Topics::SubscriptionHub::Unsubscribe(
    SubscriptionToken* psubscriptiontoken) {
  if (ptopics_parent_ != nullptr) {
    auto it = um_psubscription_token_token_entry_.find(psubscriptiontoken);

    if (it != um_psubscription_token_token_entry_.end()) {
      // Remove the token's subscription and remove the token's entry
      ptopics_parent_->Unsubscribe(it->second.topic_name,
                                   it->second.subscription_id);
      um_psubscription_token_token_entry_.erase(it);
    }
  }
}

Topics::SubscriptionToken::~SubscriptionToken(void) {
  if (psubscription_hub_ != nullptr) {
    psubscription_hub_->Unsubscribe(this);
    psubscription_hub_.reset();
  }
}

Topics::SubscriptionToken::SubscriptionToken(
    std::shared_ptr<SubscriptionHub>& psubscription_hub) noexcept
    : psubscription_hub_(psubscription_hub) {}

void Topics::SubscriptionToken::Invalidate(void) noexcept {
  psubscription_hub_.reset();
}

}  // namespace internal
}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft

// Add enum adapters to msgpack (these statements must be in the global
// namespace)
MSGPACK_ADD_ENUM(microsoft::projectairsim::client::internal::FrameType);
