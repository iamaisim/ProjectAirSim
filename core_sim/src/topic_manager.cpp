// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "topic_manager.hpp"

#include <array>
#include <list>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <thread>

#include "constant.hpp"
#include "core_sim/error.hpp"
#include "core_sim/json_utils.hpp"
#include "core_sim/logger.hpp"
#include "dispatcher.hpp"
#include "json.hpp"
#include "message/message_utils.hpp"
#include "msgpack.hpp"
#include "nng/nng.h"
#include "nng/protocol/pair1/pair.h"

namespace microsoft {
namespace projectairsim {

// forward declarations

enum class FrameType : int { kSubscribe = 0, kUnsubscribe = 1, kMessage = 2, kUnsubscribeAll = 3 };

class TopicManager::Impl {
 public:
  explicit Impl(const Logger& logger,
                const ClientAuthorization& client_authorization);

  void Load(const json& config_json);

  bool ContainsTopic(const std::string& topic_path) const;

  void RegisterTopic(const Topic& topic,
                     std::function<void(const Topic&, bool is_subscribed)>
                         callback_subscribed);

  void SubscribeTopic(
      const Topic& topic,
      std::function<void(const Topic&, const Message&)> callback);

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
  static void HandleNNGPipeEventProxy(nng_pipe pipe, nng_pipe_ev ev,
                                      void* arg) {
    reinterpret_cast<TopicManager::Impl*>(arg)->HandleNNGPipeEvent(pipe, ev);
  }

  struct TopicBlock {
    Topic topic;
    bool is_subscribed = false;
    int message_count = 0;
    std::string last_value;
    std::list<std::shared_ptr< nng_pipe > > subscribers_;
    std::function<void(const Topic&, const Message&)> callback;
    std::function<void(const Topic&, bool is_subscribed)> callback_subscribed;

    TopicBlock(const Topic &topic, std::function<void(const Topic&, bool is_subscribed)>
                   callback_subscribed_in = nullptr)
        : topic(topic), callback_subscribed(callback_subscribed_in) {}
  };

  struct Frame {
    FrameType type;
    std::string topic;
    std::string body;

    MSGPACK_DEFINE(type, topic, body);
  };

  struct TopicInfo {
    std::string path;
    std::string type;
    std::string message_type;
    int frequency;

    MSGPACK_DEFINE_MAP(path, type, message_type, frequency);
  };

  void HandleNNGPipeEvent(nng_pipe pipe, nng_pipe_ev ev);

  void RecvLoop();
  void Subscribe(const Topic& topic, nng_pipe pipe);
  void Unsubscribe(const Topic& topic, nng_pipe pipe);
  bool Unsubscribe(const std::string& topic_path, nng_pipe pipe);
  void UnsubscribeAll(nng_pipe pipe);
  void Send(const Topic& topic, const Message& message);
  void Send(const std::shared_ptr<TopicBlock>& block, nng_pipe pipe);
  void Recv(const Topic& topic, const Message& message);

  static constexpr const int recv_buffer_size = 64 * 1024;
  static constexpr const int default_port = 8999;
  static constexpr const char* default_local_address = "*";

  ClientAuthorization client_authorization_;
  std::string local_address_;
  Logger log_;
  mutable std::shared_timed_mutex manager_lock_;
  std::string name_;
  int port_;
  char* recv_buffer_;
  size_t recv_buffer_size_;
  Dispatcher recv_dispatcher_;
  std::thread recv_thread_;
  Dispatcher send_dispatcher_;
  nng_socket topic_socket_;
  std::atomic<bool> state_;
  std::map<std::string, std::shared_ptr<TopicBlock>> topic_table_;
  std::list<std::shared_ptr< nng_pipe > >  subscribers_;
  std::function<void(const std::string&, const MessageType&,
                     const std::string&)>
      topic_published_callback_;
  // TODO Configure a set of topic paths to have the topic callback enabled for
  bool enable_topic_published_callback_;
};

// class TopicManager

TopicManager::TopicManager(const Logger& logger,
                           const ClientAuthorization& client_authorization)
    : pimpl_(std::make_shared<Impl>(logger, client_authorization)) {}

TopicManager::TopicManager(const Logger& logger)
    : pimpl_(std::make_shared<Impl>(logger, ClientAuthorization())) {}

void TopicManager::Load(const json& config_json) { pimpl_->Load(config_json); }

bool TopicManager::ContainsTopic(const std::string& topic_path) const {
  return pimpl_->ContainsTopic(topic_path);
}

void TopicManager::RegisterTopic(
    const Topic& topic,
    std::function<void(const Topic&, bool is_subscribed)> callback_subscribed) {
  pimpl_->RegisterTopic(topic, callback_subscribed);
}

void TopicManager::SubscribeTopic(
    const Topic& topic,
    std::function<void(const Topic&, const Message&)> callback) {
  pimpl_->SubscribeTopic(topic, callback);
}

bool TopicManager::Unsubscribe(const std::vector<std::string>& topic_paths, nng_pipe pipe) {
  return pimpl_->Unsubscribe(topic_paths, pipe);
}

void TopicManager::PublishTopic(const Topic& topic, const Message& message) {
  pimpl_->PublishTopic(topic, message);
}

void TopicManager::UnregisterTopic(const Topic& topic) {
  pimpl_->UnregisterTopic(topic);
}

void TopicManager::Start() { pimpl_->Start(); }

void TopicManager::Stop() { pimpl_->Stop(); }

void TopicManager::CreateTopicList() { pimpl_->CreateTopicList(); }

void TopicManager::SetCallbackTopicPublished(
    const std::function<void(const std::string&, const MessageType&,
                             const std::string&)>& callback) {
  pimpl_->SetCallbackTopicPublished(callback);
}

void TopicManager::SetTopicPublishedCallbackEnabled(bool is_enabled) {
  pimpl_->SetTopicPublishedCallbackEnabled(is_enabled);
}

const bool TopicManager::IsTopicPublishedCallbackEnabled() const {
  return pimpl_->IsTopicPublishedCallbackEnabled();
}

// class TopicManager::impl

TopicManager::Impl::Impl(const Logger& logger,
                         const ClientAuthorization& client_authorization)
    : client_authorization_(client_authorization),
      local_address_(default_local_address),
      log_(logger),
      manager_lock_(),
      name_(Constant::Component::topic_manager),
      port_(default_port),
      recv_buffer_(nullptr),
      recv_buffer_size_(0),
      recv_dispatcher_("recv_dispatcher", logger),
      recv_thread_(),
      send_dispatcher_("send_dispatcher", logger),
      topic_socket_(NNG_SOCKET_INITIALIZER),
      state_(false),
      topic_table_(),
      topic_published_callback_(nullptr),
      enable_topic_published_callback_(false) {}

void TopicManager::Impl::Load(const json& config_json) {
  local_address_ = JsonUtils::GetString(config_json, Constant::Config::ip,
                                        default_local_address);
  port_ =
      JsonUtils::GetInteger(config_json, Constant::Config::port, default_port);
}

void TopicManager::Impl::HandleNNGPipeEvent(nng_pipe pipe, nng_pipe_ev ev) {
  if (ev == NNG_PIPE_EV_REM_POST) {
    // Client disconnected--reset client authorization
    log_.LogVerbose(
        name_, "Pub-sub client disconnected--resetting client authorization",
        ev);

    //
    // TODO: investigate per client auth
    //
    std::unique_lock<std::shared_timed_mutex> exclusive_lock(manager_lock_);

    for( auto it = subscribers_.begin(); it != subscribers_.end(); ++it ) {
        if( (*it).get()->id == pipe.id ) {
            subscribers_.erase( it );
            break;
        }
    }
 
    if( subscribers_.empty() ) {
      client_authorization_.SetToken(nullptr, 0);

      // Clear topics queues and unsubscribe all topics
      send_dispatcher_.clear();
      recv_dispatcher_.clear();
    }

    exclusive_lock.unlock();
    UnsubscribeAll(pipe);
  }
  else if (ev == NNG_PIPE_EV_ADD_POST) {
    // Client connected
    log_.LogVerbose(
        name_, "Pub-sub client connected--saving pipe id %d",
        pipe.id);
    //
    //  Add pipe to array of connections
    //
    std::unique_lock<std::shared_timed_mutex> exclusive_lock(manager_lock_);
    subscribers_.push_back( std::shared_ptr< nng_pipe >( new nng_pipe( pipe ) ) );
   }
}

void TopicManager::Impl::Start() {
  std::unique_lock<std::shared_timed_mutex> exclusive_lock(manager_lock_);

  int rv = nng_pair1_open_poly(&topic_socket_);
  if (rv != 0) {
    auto errno_str = nng_strerror(rv);
    log_.LogError(Constant::Component::topic_manager,
                  "nng_Pair1_open failed with '%s'.", errno_str);
    throw Error("Error initializing topic socket.");
  }

  log_.LogVerbose(
      name_, "Pub-sub connected with pair1 protocol");
 
  log_.LogVerbose(
      name_, "Pub-sub polyamourous mode on");

  // A send call can get held up by the receiving callback if it tries to do any
  // processing other than just copying the data, so if it takes too long, just
  // drop the send after this specified send_timeout to prevent the dispatcher
  // queue from stacking up
  const int send_timeout = 1000 / 60;  // 16.7 ms = 60 FPS
  rv = nng_setopt_ms(topic_socket_, NNG_OPT_SENDTIMEO, send_timeout);
  if (rv != 0) {
    auto errno_str = nng_strerror(rv);
    log_.LogError(Constant::Component::topic_manager,
                  "nng_setopt_ms failed with '%s'.", errno_str);
    throw Error("Error setting send timeout on server socket.");
  }

  // Previously with Nanomsg the send buffer size NN_SNDBUF was set to 4 MB,
  // but NNG's send buffer size parameter NNG_OPT_SENDBUF is in number of
  // messages rather than bytes, so instead of choosing some arbitrary number
  // of messages, just leave it unset to use the default value.

  // Register callback for client disconnects
  rv = nng_pipe_notify(topic_socket_, NNG_PIPE_EV_REM_POST,
                       &TopicManager::Impl::HandleNNGPipeEventProxy, this);
  if (rv != 0) {
    auto errno_str = nng_strerror(rv);
    log_.LogError(name_, "nng_pipe_notify failed with '%s'.", errno_str);
    throw Error(
        "Error installing event listener on topic socket for serving "
        "requests.");
  }
  rv = nng_pipe_notify(topic_socket_, NNG_PIPE_EV_ADD_POST,
                       &TopicManager::Impl::HandleNNGPipeEventProxy, this);
  if (rv != 0) {
    auto errno_str = nng_strerror(rv);
    log_.LogError(name_, "nng_pipe_notify add failed with '%s'.", errno_str);
    throw Error(
        "Error installing event listener on topic socket for serving "
        "requests.");
  }

  log_.LogTrace(name_, "Topic socket created with id: '%d'.",
                nng_socket_id(topic_socket_));

  std::stringstream binding_str;
  binding_str << "tcp://" << local_address_ << ":" << port_;

  rv = nng_listen(topic_socket_, binding_str.str().c_str(),
                  NULL /* no need to save pointer to the created listener */,
                  0 /*no special flags*/);
  if (rv != 0) {
    auto errno_str = nng_strerror(rv);
    log_.LogError(Constant::Component::topic_manager,
                  "nng_listen failed with '%s'.", errno_str);
    throw Error("Error binding topic socket.");
  }

  log_.LogTrace(name_, "nng_listen success '%s'.", binding_str.str().c_str());

  send_dispatcher_.start();
  recv_dispatcher_.start();

  state_ = true;
  recv_thread_ = std::thread(&TopicManager::Impl::RecvLoop, this);
}

void TopicManager::Impl::Stop() {
  std::unique_lock<std::shared_timed_mutex> exclusive_lock(manager_lock_);

  send_dispatcher_.stop();
  recv_dispatcher_.stop();

  state_ = false;

  if (recv_thread_.joinable()) {
    try {
      recv_thread_.join();
    } catch (std::exception& exception) {
      log_.LogError(name_, "recv_thread_.join() failed with '%s'.",
                    exception.what());
    } catch (...) {
      log_.LogError(name_,
                    "recv_thread_.join() failed with unknown exception.");
    }
  }

  int rv = nng_close(topic_socket_);
  if (rv != 0) {
    auto errno_str = nng_strerror(rv);
    log_.LogError(name_, "nng_close failed with '%s'.", errno_str);
  }

  topic_socket_ = NNG_SOCKET_INITIALIZER;

  enable_topic_published_callback_ = false;
  topic_published_callback_ = nullptr;
}

void TopicManager::Impl::RegisterTopic(
    const Topic& topic,
    std::function<void(const Topic&, bool is_subscribed)> callback_subscribed) {
  std::unique_lock<std::shared_timed_mutex> exclusive_lock(manager_lock_);

  auto topic_path = topic.GetPath();
  auto iter = topic_table_.find(topic_path);
  if (iter != topic_table_.end()) {
    log_.LogWarning(name_, "Topic '%s' already registered.",
                    topic.GetPath().c_str());
    return;
  }

  auto block = std::make_shared<TopicBlock>(topic, callback_subscribed);

  topic_table_.insert(std::make_pair(topic_path, block));
}

void TopicManager::Impl::SubscribeTopic(
    const Topic& topic,
    std::function<void(const Topic&, const Message&)> callback) {
  std::unique_lock<std::shared_timed_mutex> exclusive_lock(manager_lock_);

  auto topic_path = topic.GetPath();
  auto iter = topic_table_.find(topic_path);
  if (iter == topic_table_.end()) {
    log_.LogWarning(name_, "Subscribe failed as topic '%s' not registered.",
                    topic.GetPath().c_str());
    return;
  }

  auto block = iter->second;
  block->is_subscribed = true;
  block->callback = callback;

}

void TopicManager::Impl::PublishTopic(const Topic& topic,
                                      const Message& message) {
  if (client_authorization_.IsAuthorized())
    send_dispatcher_.dispatch(
        [this, topic, message]() { Send(topic, message); });
}

bool TopicManager::Impl::ContainsTopic(const std::string& topic_path) const {
  std::shared_lock<std::shared_timed_mutex> shared_lock(manager_lock_);
  auto iter = topic_table_.find(topic_path);
  return iter != topic_table_.end();
}

void TopicManager::Impl::UnregisterTopic(const Topic& topic) {
  std::unique_lock<std::shared_timed_mutex> exclusive_lock(manager_lock_);

  auto topic_path = topic.GetPath();
  auto iter = topic_table_.find(topic_path);
  if (iter == topic_table_.end()) {
    log_.LogWarning(name_,
                    "Cannot unregister topic '%s' that is not registered.",
                    topic.GetPath().c_str());
    return;
  }

  auto block = iter->second;
  block->is_subscribed = false;
  block->callback = nullptr;

  topic_table_.erase(iter->first);

}

void TopicManager::Impl::RecvLoop() {
  while (state_.load()) {
    nng_msg* recv_msg_;

    int rv = nng_recvmsg(topic_socket_, &recv_msg_,
                      NNG_FLAG_ALLOC | NNG_FLAG_NONBLOCK);

    if (rv != 0) {
      if (rv == NNG_EAGAIN) {
        // No data is ready yet, spin to retry
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        continue;
      } else {
        auto errno_str = nng_strerror(rv);
        log_.LogWarning(name_, "nng_recv failed with '%s'.", errno_str);
        break;
      }
    }

    nng_pipe client_pipe = nng_msg_get_pipe(recv_msg_);
    recv_buffer_ = static_cast<char *>( nng_msg_body(recv_msg_) );
    recv_buffer_size_ = nng_msg_len(recv_msg_);

    auto object_handle = msgpack::unpack(recv_buffer_, recv_buffer_size_);
    auto object = object_handle.get();
    Frame frame;
    object.convert(frame);

    Topic topic;

    if (frame.type != FrameType::kUnsubscribeAll)
    {
      std::shared_lock<std::shared_timed_mutex> shared_lock(manager_lock_);
      auto topic_path = frame.topic;
      auto iter = topic_table_.find(topic_path);
      if (iter == topic_table_.end()) {
        log_.LogError(
            name_, "Cannot recv message message for not registered topic '%s'",
            topic_path.c_str());

        nng_msg_free(recv_msg_);
        continue;
      }

      topic = iter->second->topic;
    }

    if (frame.type == FrameType::kSubscribe) {
      send_dispatcher_.dispatch([this, topic, client_pipe]() { Subscribe(topic, client_pipe); });
    } else if (frame.type == FrameType::kUnsubscribe) {
      send_dispatcher_.dispatch([this, topic, client_pipe]() { Unsubscribe(topic, client_pipe); });
    } else if (frame.type == FrameType::kUnsubscribeAll) {
      send_dispatcher_.dispatch([this, topic, client_pipe]() { UnsubscribeAll(client_pipe); });
    } else if (frame.type == FrameType::kMessage) {
      auto message = MessageUtils::ToMessage(topic, frame.body);
      recv_dispatcher_.dispatch(
          [this, topic, message]() { Recv(topic, message); });
    } else {
      log_.LogError(name_, "Invalid frame type.");
    }

    nng_msg_free(recv_msg_);
  }
}

void TopicManager::Impl::Send(const Topic& topic, const Message& message) {
  std::shared_lock<std::shared_timed_mutex> shared_lock(manager_lock_);

  auto topic_path = topic.GetPath();
  auto iter = topic_table_.find(topic_path);
  if (iter == topic_table_.end()) {
    log_.LogError(name_, "Cannot send message to unregistered topic '%s'",
                  topic_path.c_str());
    return;
  }

  auto block = iter->second;

  // Cache the value to serve the latest value when client subscribes to the
  // topic.
  block->last_value = message.Serialize();

  for( auto pipe : block->subscribers_) {
    Send(block, *(pipe.get()));
  }

  // Also pass message to external callback (e.g. UnrealSimTopicData actor)
  if (topic_published_callback_ && enable_topic_published_callback_) {
    topic_published_callback_(topic_path, topic.GetMessageType(),
                              message.Serialize());
  }
}

void TopicManager::Impl::Subscribe(const Topic& topic, nng_pipe pipe) {
  std::shared_lock<std::shared_timed_mutex> shared_lock(manager_lock_);

  auto topic_path = topic.GetPath();
  auto iter = topic_table_.find(topic_path);
  if (iter == topic_table_.end()) {
    log_.LogError(name_, "Client cannot subscribe to unregistered topic '%s'",
                  topic_path.c_str());
    return;
  }

  log_.LogTrace(name_, "Client subscribing to topic %s, %d", topic_path.c_str(), pipe.id);

  {
    auto block = iter->second;
    auto callback_subscribed = block->callback_subscribed;

    block->is_subscribed = true;
    block->subscribers_.push_back( std::shared_ptr< nng_pipe >( new nng_pipe( pipe ) ) );

    if (!block->last_value.empty()) {
      Send(block, pipe);
    }

    // Notify callbacks of change in subscription
    if (callback_subscribed != nullptr) callback_subscribed(topic, true);
  }
}

bool TopicManager::Impl::Unsubscribe(const std::string& topic_path, nng_pipe pipe) {
  std::shared_lock<std::shared_timed_mutex> shared_lock(manager_lock_);
  auto iter = topic_table_.find(topic_path);
  if (iter == topic_table_.end()) {
    log_.LogError(name_, "Client cannot unsubscribe from unregistered topic '%s'",
                  topic_path.c_str());
    return false;
  }

  log_.LogTrace(name_, "Client unsubscribing from topic %s", topic_path.c_str());

  {
    auto block = iter->second;
    auto callback_subscribed = block->callback_subscribed;

    for( auto it = block->subscribers_.begin(); it != block->subscribers_.end(); ++it ) {
        if( (*it).get()->id == pipe.id ) {
            block->subscribers_.erase( it );
            break;
        }
    }
 
    if( block->subscribers_.empty() ) {
        block->is_subscribed = false;
    }

    // Notify callbacks of change in subscription
    if (callback_subscribed != nullptr) callback_subscribed(block->topic, false);
  }

  return true;
}

bool TopicManager::Impl::Unsubscribe(
    const std::vector<std::string>& topic_paths, nng_pipe pipe) {
  auto success = true;
  for (const auto& topic_path : topic_paths) {
    success = Unsubscribe(topic_path, pipe) && success;
  }
  return success;
}

void TopicManager::Impl::Unsubscribe(const Topic& topic, nng_pipe pipe) {
  auto topic_path = topic.GetPath();
  Unsubscribe(topic_path, pipe);
}

void TopicManager::Impl::UnsubscribeAll(nng_pipe pipe) {
  std::shared_lock<std::shared_timed_mutex> shared_lock(manager_lock_);
  log_.LogTrace(name_, "Unsubscribing all topics.");
  for (auto [topic, topic_block] : topic_table_) {

    for( auto it = topic_block->subscribers_.begin(); it != topic_block->subscribers_.end(); ++it ) {
        if( (*it).get()->id == pipe.id ) {
            topic_block->subscribers_.erase( it );
            break;
        }
    }
 
    if( topic_block->subscribers_.empty() ) {
        topic_block->is_subscribed = false;
    }
  }
}

void TopicManager::Impl::Send(const std::shared_ptr<TopicBlock>& block, nng_pipe pipe) {
  auto topic_path = block->topic.GetPath();
  if (block->last_value.empty()) {
    // No published data to send yet
    return;
  }

  if (block->is_subscribed) {
    Frame frame;
    frame.type = FrameType::kMessage;
    frame.topic = topic_path;
    frame.body = block->last_value;

    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, frame);

    nng_msg* send_msg_;
    int rv = nng_msg_alloc(&send_msg_, 0);
    if (rv != 0) {
        auto errno_str = nng_strerror(rv);
        log_.LogWarning(name_, "nng_msg_alloc for topic '%s' failed with error '%s'.",
                        frame.topic.c_str(), errno_str);
        return;
      }

    nng_msg_append( send_msg_, sbuf.data(), sbuf.size() );
    nng_msg_set_pipe( send_msg_, pipe );

    rv = nng_sendmsg( topic_socket_, send_msg_, 0 );

    //nng_msg_free(send_msg_);  // docs say socket will free msg when sent

    if (rv != 0) {
      auto errno_str = nng_strerror(rv);
      log_.LogWarning(name_, "nng_send for topic '%s' failed with error '%s'.",
                      frame.topic.c_str(), errno_str);
      return;
    }
  }
}

void TopicManager::Impl::Recv(const Topic& topic, const Message& message) {
  std::shared_lock<std::shared_timed_mutex> shared_lock(manager_lock_);

  auto topic_path = topic.GetPath();
  auto iter = topic_table_.find(topic_path);
  if (iter == topic_table_.end()) {
    log_.LogError(name_, "Cannot recv message from not registered topic '%s'",
                  topic_path.c_str());
    return;
  }

  auto block = iter->second;
  if (!block->is_subscribed) {
    log_.LogWarning(name_, "Cannot recv message for unsubscribed topic '%s'",
                    topic_path.c_str());
    return;
  }

  auto callback = block->callback;
  if (callback == nullptr) {
    log_.LogError(name_, "Callback null for topic '%s'", topic_path.c_str());
    return;
  }

  callback(topic, message);
}

void TopicManager::Impl::CreateTopicList() {
  std::vector<TopicInfo> topic_infos;

  // In case the Topic List topic hasn't been created yet, add in a placeholder
  // before creating the topic info data so it is included in the list of info
  auto topic_name = "$topics";
  Topic topic_list_topic(topic_name, "", TopicType::kPublished, 0,
                         MessageType::kTopicList);
  auto block = std::make_shared<TopicBlock>(topic_list_topic);

  topic_table_[topic_list_topic.GetPath()] = block;  // insert or overwrite

  // Create list of topic info data for the Topic List topic
  std::transform(
      topic_table_.cbegin(), topic_table_.cend(),
      std::back_inserter(topic_infos),
      [topic_infos](std::pair<std::string, std::shared_ptr<TopicBlock>> pair) {
        TopicInfo topic_info;
        auto topic = pair.second->topic;

        topic_info.path = topic.GetPath();

        switch (topic.GetType()) {
          case TopicType::kPublished:
            topic_info.type = "published";
            break;
          case TopicType::kSubscribed:
            topic_info.type = "subscribed";
            break;
          default:
            break;
        }

        switch (topic.GetMessageType()) {
          case MessageType::kTopicList:
            topic_info.message_type = "topic-list";
            break;
          case MessageType::kInt8:
            topic_info.message_type = "int8";
            break;
          case MessageType::kJointState:
            topic_info.message_type = "joint-state";
            break;
          case MessageType::kImage:
            topic_info.message_type = "image";
            break;
          case MessageType::kCameraInfo:
            topic_info.message_type = "camera-info";
            break;
          case MessageType::kFlightControlSetpoint:
            topic_info.message_type = "flight-control-setpoint";
            break;
          case MessageType::kImu:
            topic_info.message_type = "imu";
            break;
          case MessageType::kTransform:
            topic_info.message_type = "transform";
            break;
          case MessageType::kBarometer:
            topic_info.message_type = "barometer";
            break;
          case MessageType::kCollisionInfo:
            topic_info.message_type = "collision-info";
            break;
          case MessageType::kMagnetometer:
            topic_info.message_type = "magnetometer";
            break;
          case MessageType::kLidar:
            topic_info.message_type = "lidar";
            break;
          case MessageType::kDistanceSensor:
            topic_info.message_type = "distance-sensor";
            break;
          default:
            break;
        }

        topic_info.frequency = topic.GetFrequency();

        return topic_info;
      });

  std::stringstream stream;
  msgpack::pack(stream, topic_infos);
  stream.seekg(0);

  // Update Topic List info data to its block that is already in the topic table
  block->last_value = stream.str();
}

void TopicManager::Impl::SetCallbackTopicPublished(
    const std::function<void(const std::string&, const MessageType&,
                             const std::string&)>& callback) {
  topic_published_callback_ = callback;
}

void TopicManager::Impl::SetTopicPublishedCallbackEnabled(bool is_enabled) {
  enable_topic_published_callback_ = is_enabled;
}

const bool TopicManager::Impl::IsTopicPublishedCallbackEnabled() const {
  return enable_topic_published_callback_;
}

}  // namespace projectairsim
}  // namespace microsoft

MSGPACK_ADD_ENUM(microsoft::projectairsim::FrameType);
