// Copyright (C) Microsoft Corporation.  All rights reserved.

#include <NNGI/NNGI.h>

#include <AirSimMessage/request_message.hpp>
#include <AirSimMessage/response_message.hpp>
#include <chrono>
#include <sstream>
#include <string>

#include "AirSimClient.h"
#include "AsyncResultInternal.h"
#include "IMessageBuffer.h"
#include "RefCounted.h"
#include "Topics.h"
#include "Utils.h"
#include "json.hpp"
#include "pch.h"

namespace microsoft {
namespace projectairsim {
namespace client {

// Message buffer allocated and freed by NNG
class NNGMessageBuffer : public internal::TRefCounted<IMessageBuffer> {
 public:
  NNGMessageBuffer(uint8_t* rgb_in, size_t cb_in)
      : TRefCounted(), rgb_(rgb_in), cb_(cb_in) {}

  void Clear(void) {
    if (rgb_ != nullptr) {
      nngi::nng_free(rgb_, cb_);
      rgb_ = nullptr;
    }
  }

 public:
  virtual size_t Cb(void) const override { return (cb_); }
  virtual const std::uint8_t* Data(void) const override { return (rgb_); }

 protected:
  virtual ~NNGMessageBuffer() override { Clear(); }

 protected:
  uint8_t* rgb_;  // Message bytes
  size_t cb_;     // Number of bytes in rgb_
};                // struct NNGMessageBuffer

class NNGChannel : public internal::ICommChannel {
 public:
  NNGChannel(nngi::nng_socket* pnng_socket) : pnng_socket_(pnng_socket) {}

  void Close(void) { pnng_socket_ = nullptr; }

 public:
  virtual Status Receive(Message* pmessage_inout,
                         unsigned long ms_timeout) override {
    size_t cb;
    int err;
    uint8_t* rgb;

    if (pnng_socket_ == nullptr) return (Status::Closed);

    // Update time-out if necessary
    if (ms_timeout != ms_timeout_) {
      ms_timeout_ = ms_timeout;
      nngi::nng_socket_set_ms(*pnng_socket_, NNG_OPT_RECVTIMEO, ms_timeout_);
    }

    // Read from the socket
    if ((err = nngi::nng_recv(*pnng_socket_, &rgb, &cb, NNG_FLAG_ALLOC)) != 0) {
      if (err == nngi::NNG_ETIMEDOUT) return (Status::TimedOut);

      return (internal::ErrNNGToStatus(err));
    }

    // Return the message
    pmessage_inout->SetMessageBuffer(new NNGMessageBuffer(rgb, cb));
    return (Status::OK);
  }

  virtual Status Send(const uint8_t* rgb, size_t cb) override {
    if (pnng_socket_ == nullptr) return (Status::Closed);

    int err = nngi::nng_send(
        *pnng_socket_, const_cast<char*>(reinterpret_cast<const char*>(rgb)),
        cb, 0);

    return (internal::ErrNNGToStatus(err));
  }

 protected:
  nngi::nng_socket* pnng_socket_ = nullptr;  // NNG socket to server
  unsigned long ms_timeout_ =
      kMSTimeoutInfinite;  // Current timeout duration set
};                         // class NNGServerChannel

class RequestMessageModifier : public RequestMessage {
 public:
  RequestMessageModifier(const RequestMessage& request_message,
                         unsigned int id_new)
      : RequestMessage(id_new, request_message.GetMethod(),
                       request_message.GetParams(),
                       request_message.GetVersion()) {}
};

class Client::Impl {
 public:
  // Service method request ID
  typedef unsigned int RequestID;

 public:
  Impl(void);
  ~Impl(void);

  void CancelAllRequests(bool fNoLock = false) noexcept;
  Status Connect(const std::string& address, unsigned int port_topics,
                 unsigned int port_services);
  void Disconnect(void) noexcept;
  std::vector<std::string> GetTopicInfo(void);
  Status Request(const std::string& str_method,
                 const nlohmann::json& json_parameters,
                 client::Message* pmessage_response_out) noexcept;
  TAsyncResult<Message> RequestAsync(
      const std::string& str_method, const nlohmann::json& json_parameters,
      FnResponseCallback fnresponse_callback = nullptr,
      bool fbypass_connected_check = false) noexcept;
  Status RequestLoadScene(const std::string& str_scene_config,
                          client::Message* pmessage_response_out);
  TAsyncResult<Message> RequestPriorityAsync(
      const std::string& str_method,
      const nlohmann::json& json_parameters = json(),
      FnResponseCallback fnresponse_callback = nullptr) noexcept;
  Status Subscribe(const char* sz_topic_name, TopicCallback topic_callback);
  Status Unsubscribe(const char* sz_topic_name);
  Status Unsubscribe(const std::vector<std::string>& vec_topic_name,
                     bool fbypass_connected_check = false);
  Status UnsubscribeAll(bool fbypass_connected_check = false) noexcept;

 protected:
  typedef std::vector<std::unique_ptr<internal::Topics::SubscriptionToken>>
      VecPSubscriptionToken;  // Vector of subscription token pointers
  typedef std::unordered_map<std::string, VecPSubscriptionToken>
      UMStrVecPSubscriptionToken;  // Mapping from topic name to subscription
                                   // token

  // Entry for an expected response waiting to be received from the server
  struct PendingResponseEntry {
    RequestID request_id;  // Unique ID of the request
    internal::TAsyncResultProvider<Message>*
        pasyncresultprovider_message;  // Pointer to async result provider to
                                       // receive the result

    PendingResponseEntry(RequestID request_id_in,
                         internal::TAsyncResultProvider<Message>*
                             pasyncresultprovider_message_in)
        : request_id(request_id_in),
          pasyncresultprovider_message(pasyncresultprovider_message_in) {
      pasyncresultprovider_message->AddRef();
    }
    ~PendingResponseEntry(void) {
      if (pasyncresultprovider_message != nullptr)
        pasyncresultprovider_message->Release();
    }
  };  // struct PendingResponseEntry

  typedef std::list<PendingResponseEntry> ListPendingResponseEntry;

  // Entry for a request waiting to be sent to the server
  struct PendingRequestEntry {
    json json_params;  // Method parameters
    internal::TAsyncResultProvider<Message>*
        pasyncresultprovider_message;  // Pointer to async result provider to
                                       // receive the result
    std::string str_method;            // Server method to invoke

    PendingRequestEntry(const std::string& str_method_in,
                        const json& json_params_in,
                        internal::TAsyncResultProvider<Message>*
                            pasyncresultprovider_message_in)
        : json_params(json_params_in),
          str_method(str_method_in),
          pasyncresultprovider_message(pasyncresultprovider_message_in) {
      pasyncresultprovider_message->AddRef();
    }
    PendingRequestEntry(std::string&& str_method_in, json&& json_params_in,
                        internal::TAsyncResultProvider<Message>*
                            pasyncresultprovider_message_in)
        : json_params(std::move(json_params_in)),
          str_method(std::move(str_method_in)),
          pasyncresultprovider_message(pasyncresultprovider_message_in) {
      pasyncresultprovider_message->AddRef();
    }
    ~PendingRequestEntry(void) {
      if (pasyncresultprovider_message != nullptr)
        pasyncresultprovider_message->Release();
    }
  };  // struct PendingRequestEntry

  typedef std::list<PendingRequestEntry> ListPendingRequestEntry;

  // Entry for a request waiting a response from the server
  struct WaitingRequestEntry {
    internal::TAsyncResultProvider<Message>*
        pasyncresultprovider_message;  // Pointer to async result provider to
                                       // receive the result
    RequestID request_id;              // ID of this request

    WaitingRequestEntry(internal::TAsyncResultProvider<Message>*
                            pasyncresultprovider_message_in,
                        RequestID request_id_in)
        : pasyncresultprovider_message(pasyncresultprovider_message_in),
          request_id(request_id_in) {
      pasyncresultprovider_message->AddRef();
    }
    ~WaitingRequestEntry(void) {
      if (pasyncresultprovider_message != nullptr)
        pasyncresultprovider_message->Release();
    }
  };  // struct WaitingRequestEntry

  typedef std::list<WaitingRequestEntry> ListWaitingRequestEntry;

 protected:
  void DisconnectCore(void) noexcept;
  std::string GetUrl(const char* address, unsigned int port);
  void RequestSendingThreadProc(void);
  void ResponseReceivingThreadProc(void);
  Status SendRequest(const std::string& str_method,
                     const nlohmann::json& json_parameters,
                     internal::TAsyncResultProvider<Message>* parp = nullptr);

 protected:
  static constexpr nngi::nng_duration kSecTopicSendTimeout =
      1000;  // Pub-sub topic send timeout (milliseconds)
  static constexpr nngi::nng_duration kSecResponseReceiveTimeout =
      300000;  // Service response read timeout (milliseconds)
  static constexpr nngi::nng_duration kSecRequestSendTimeout =
      1000;  // Service request send timeout (milliseconds)
  static constexpr nngi::nng_duration kSecRequestRetryTime =
      -1;  // Service request resend time (milliseconds), set to disabled

 protected:
  std::condition_variable
      cv_pending_request_;  // Condition variable to signal when a request is
  std::condition_variable
      cv_pending_response_;  // Condition variable to signal when a response is
                             // added to list_pending_response_entry_
  bool is_connected_;        // We're connected to a client
  ListPendingResponseEntry
      list_pending_response_entry_;  // List of expected responses from the
                                     // server
  ListPendingRequestEntry
      list_pending_request_entry_;  // List of pending requests to the server
  std::mutex
      mutex_message_queues_;  // Access guard to list_pending_request_entry_
  std::mutex
      mutex_message_send_;  // Access guard to list_pending_response_entry_
  nngi::nng_socket nng_socket_services_;  // NNG socket to Project AirSim server
                                          // services interface
  nngi::nng_socket nng_socket_topics_;    // NNG socket to Project AirSim server
                                          // topics interface
  std::shared_ptr<NNGChannel>
      pnng_server_channel_topics_;  // Server channel to topics interface
  std::unique_ptr<std::thread> pthread_receiving_;  // Response receiving thread
  std::unique_ptr<std::thread> pthread_sending_;    // Request sending thread
  RequestID request_id_next_;                       // Next service request ID
  bool run_request_threads_;  // If true, the sending & receiving threads should
                              // keep running
  internal::Topics topics_;   // Topic handler
  UMStrVecPSubscriptionToken
      um_str_vecpsubscription_token_;  // Mapping from topic name to
                                       // subscription tokens
};                                     // class Client::Impl

ASC_DECL Client::Client(void) noexcept : pclient_impl_(new Impl()) {}

ASC_DECL Client::~Client() { Disconnect(); }

ASC_DECL TAsyncResult<Message> Client::RequestPriorityAsync(
    const std::string& str_method, const nlohmann::json& json_parameters,
    FnResponseCallback fnresponse_callback) noexcept {
  return (pclient_impl_->RequestPriorityAsync(str_method, json_parameters,
                                              fnresponse_callback));
}

ASC_DECL void Client::CancelAllRequests(void) noexcept {
  pclient_impl_->CancelAllRequests();
}

ASC_DECL Status Client::Connect(const std::string& address,
                                unsigned int port_topics,
                                unsigned int port_services) {
  return (pclient_impl_->Connect(address, port_topics, port_services));
}

ASC_DECL void Client::Disconnect(void) noexcept { pclient_impl_->Disconnect(); }

ASC_DECL std::vector<std::string> Client::GetTopicInfo(void) {
  return (pclient_impl_->GetTopicInfo());
}

ASC_DECL const char* Client::GetNNGVersion(void) {
  return (nngi::nng_version());
}

ASC_DECL const char* Client::GetVersion(void) {
  return (internal::v_str_version.c_str());
}

ASC_DECL Status Client::Request(
    const std::string& str_method, const nlohmann::json& json_parameters,
    client::Message* pmessage_response_out) noexcept {
  return (pclient_impl_->Request(str_method, json_parameters,
                                 pmessage_response_out));
}

ASC_DECL TAsyncResult<Message> Client::RequestAsync(
    const std::string& str_method, const nlohmann::json& json_parameters,
    FnResponseCallback fnresponse_callback) noexcept {
  return (pclient_impl_->RequestAsync(str_method, json_parameters,
                                      fnresponse_callback));
}

ASC_DECL Status
Client::RequestLoadScene(const std::string& str_scene_config,
                         client::Message* pmessage_response_out) {
  return (
      pclient_impl_->RequestLoadScene(str_scene_config, pmessage_response_out));
}

ASC_DECL Status Client::Subscribe(const char* sz_topic_name,
                                  TopicCallback topic_callback) {
  return (pclient_impl_->Subscribe(sz_topic_name, topic_callback));
}

ASC_DECL Status Client::Unsubscribe(const char* sz_topic_name) {
  return (pclient_impl_->Unsubscribe(sz_topic_name));
}

ASC_DECL Status
Client::Unsubscribe(const std::vector<std::string>& vec_topic_name) {
  return (pclient_impl_->Unsubscribe(vec_topic_name));
}

ASC_DECL Status Client::UnsubscribeAll(void) noexcept {
  return (pclient_impl_->UnsubscribeAll());
}

Client::Impl::Impl(void)
    : cv_pending_request_(),
      cv_pending_response_(),
      is_connected_(false),
      list_pending_request_entry_(),
      list_pending_response_entry_(),
      mutex_message_queues_(),
      mutex_message_send_(),
      nng_socket_services_(NNG_SOCKET_INITIALIZER),
      nng_socket_topics_(NNG_SOCKET_INITIALIZER),
      pnng_server_channel_topics_(
          std::make_shared<NNGChannel>(&nng_socket_topics_)),
      pthread_receiving_(),
      pthread_sending_(),
      request_id_next_(0),
      run_request_threads_(true),
      topics_(std::static_pointer_cast<internal::ICommChannel>(
          pnng_server_channel_topics_)),
      um_str_vecpsubscription_token_() {}

Client::Impl::~Impl() { DisconnectCore(); }

void Client::Impl::CancelAllRequests(bool fNoLock) noexcept {
  std::unique_lock ul(mutex_message_queues_, std::defer_lock);

  // Lock pending request list to prevent new requests while we're trying
  // to cancel the existing ones
  if (!fNoLock) ul.lock();

  // Cancel all pending requests
  for (auto& entry : list_pending_request_entry_)
    entry.pasyncresultprovider_message->SetDone(Status::Canceled);
  list_pending_request_entry_.clear();

  // Cancel request in progress, if any, in case the request is taking a long
  // time
  if (is_connected_) {
    // Cancel the current request by sending another (no-op, but
    // error-returning) request
    RequestPriorityAsync("CancelRequest").Wait();
  }
}

Status Client::Impl::Connect(const std::string& address,
                             unsigned int port_topics,
                             unsigned int port_services) {
  Status statusRet = Status::OK;
  uint32_t err;

  DisconnectCore();

  log.InfoF("Connecting to simulation server at %s", address.c_str());

  // Open socket for the Project AirSim request-response services
  if ((err = nngi::nng_req0_open(&nng_socket_services_)) != 0) goto LErrorNNG;

  // Set request-response socket options
  nngi::nng_socket_set_ms(nng_socket_services_, NNG_OPT_RECVTIMEO,
                          kSecResponseReceiveTimeout);
  nngi::nng_socket_set_ms(nng_socket_services_, NNG_OPT_SENDTIMEO,
                          kSecRequestSendTimeout);
  nngi::nng_socket_set_ms(nng_socket_services_, NNG_OPT_REQ_RESENDTIME,
                          kSecRequestRetryTime);

  // Connect the services socket
  if ((err = nngi::nng_dial(nng_socket_services_,
                            GetUrl(address.c_str(), port_services).c_str(),
                            nullptr, 0)) != 0)
    goto LErrorNNG;

  // Open socket for the Project AirSim topics
  if ((err = nngi::nng_pair0_open(&nng_socket_topics_)) != 0) goto LErrorNNG;

  // Set pub-sub socket options
  nngi::nng_socket_set_ms(nng_socket_topics_, NNG_OPT_SENDTIMEO,
                          kSecTopicSendTimeout);

  // Connect the topics socket
  if ((err = nngi::nng_dial(nng_socket_topics_,
                            GetUrl(address.c_str(), port_topics).c_str(),
                            nullptr, 0)) != 0)
    goto LErrorNNG;

  // Start topic receiving and dispatching thread
  topics_.StartReceiving();

  // Indicate that we're connected
  is_connected_ = true;

  // Start service request sending and response receiving threads
  run_request_threads_ = true;
  pthread_sending_.reset(
      new std::thread([this]() { RequestSendingThreadProc(); }));
  pthread_receiving_.reset(
      new std::thread([this]() { ResponseReceivingThreadProc(); }));

LDone:
  if (statusRet != Status::OK)
    DisconnectCore();
  else
    log.Info("Connection opened.");

  return (statusRet);

LErrorNNG:
  statusRet = internal::ErrNNGToStatus(err);
  goto LDone;
}

void Client::Impl::Disconnect(void) noexcept {
  if (is_connected_) log.Info("Disconnecting from simulation server...");

  DisconnectCore();

  log.Info("Disconnected.");
}

void Client::Impl::DisconnectCore(void) noexcept {
  bool fsockets_are_connected = is_connected_;

  // Flag that we're about to be disconnected to prevent any new requests
  // and cancel current requests.  We need to keep the send thread running
  // until after unsubscribing our topics.
  {
    std::unique_lock lg(mutex_message_queues_);

    CancelAllRequests(true);
    is_connected_ = false;  // Fail any new external requests
  }

  // Unsubcribe from all topics
  UnsubscribeAll(true);

  // Stop topics object
  topics_.StopReceiving();

  // Close the sockets
  if (fsockets_are_connected) {
    (void)nng_close(nng_socket_services_);
    (void)nng_close(nng_socket_topics_);
    fsockets_are_connected = false;
  }

  // Stop and cleanup sending thread
  run_request_threads_ = false;
  if (pthread_sending_ != nullptr) {
    pthread_sending_->join();
    pthread_sending_.reset();
  }
  if (pthread_receiving_ != nullptr) {
    pthread_receiving_->join();
    pthread_receiving_.reset();
  }

  // Cancel any pending requests again in case any slipped through before we
  // managed to disable external requests
  CancelAllRequests();
}

VecStr Client::Impl::GetTopicInfo(void) {
  VecStr vec_str;

  log.Info("Getting the list of available topic info from the sim server...");
  vec_str = topics_.GetAll();
  log.Info("Successfully received topic info list.");

  return (vec_str);
}

std::string Client::Impl::GetUrl(const char* address, unsigned int port) {
  std::ostringstream oss;

  oss << "tcp://" << address << ":" << std::to_string(port);
  return (oss.str());
}

Status Client::Impl::Request(const std::string& str_method,
                             const nlohmann::json& json_parameters,
                             client::Message* pmessage_response_out) noexcept {
  auto async_result = RequestAsync(str_method, json_parameters);
  Status status;

  try {
    status = async_result.Wait();
    if (status == Status::OK) *pmessage_response_out = async_result.GetResult();
  } catch (...) {
    status = Status::Failed;
  }

  return (status);
}

TAsyncResult<Message> Client::Impl::RequestAsync(
    const std::string& str_method, const nlohmann::json& json_parameters,
    FnResponseCallback fnresponse_callback,
    bool fbypass_connected_check) noexcept {
  auto* parp = new internal::TAsyncResultProvider<Message>(fnresponse_callback);
  TAsyncResult<Message> ar(parp);

  try {
    // If we're connected, complete the task with an error
    if (!fbypass_connected_check && !is_connected_)
      parp->SetDone(Status::NotConnected);
    else {
      // Queue the request
      std::lock_guard lg(mutex_message_queues_);

      list_pending_request_entry_.emplace_back(str_method, json_parameters,
                                               parp);
      cv_pending_request_.notify_all();
    }
  } catch (...) {
    parp->SetDone(Status::Failed);
  }

  return (ar);
}

Status Client::Impl::RequestLoadScene(const std::string& str_scene_config,
                                      client::Message* pmessage_response_out) {
  Status status;
  json json_params = {{"scene_config", str_scene_config}};

  CancelAllRequests();     // Cancel all other pending requests
  (void)UnsubscribeAll();  // Remove all subscriptions to the current scene that
                           // we're about to replace
  topics_.StopReceiving();  // Stop topics processing and reset everything

  if ((status = Request("/Sim/LoadScene", json_params,
                        pmessage_response_out)) == Status::OK) {
    ResponseMessage rm;

    rm.Deserialize(*pmessage_response_out);
    if (rm.GetErrorCode() == 0)
      status = topics_.StartReceiving();  // Start topics processing
  }

  return (status);
}

TAsyncResult<Message> Client::Impl::RequestPriorityAsync(
    const std::string& str_method, const nlohmann::json& json_parameters,
    FnResponseCallback fnresponse_callback) noexcept {
  auto* parp = new internal::TAsyncResultProvider<Message>(fnresponse_callback);
  TAsyncResult<Message> ar(parp);
  Status status;

  // Send request directly, jumping the line
  if ((status = SendRequest(str_method, json_parameters, parp)) != Status::OK)
    parp->SetDone(status);

  return (ar);
}

void Client::Impl::RequestSendingThreadProc(void) {
  while (run_request_threads_) {
    json json_params;  // Method parameters
    internal::TAsyncResultProvider<Message>* pasyncresultprovider_message =
        nullptr;  // Pointer to async result provider to receive the result
    std::string str_method;  // Server method to invoke

    // Get the next request to send
    {
      std::unique_lock ul(mutex_message_queues_);

      while (run_request_threads_) {
        if (!list_pending_request_entry_.empty()) {
          auto& pending_request_entry = list_pending_request_entry_.front();

          json_params = std::move(pending_request_entry.json_params);
          str_method = std::move(pending_request_entry.str_method);
          pasyncresultprovider_message =
              pending_request_entry.pasyncresultprovider_message;
          pending_request_entry.pasyncresultprovider_message =
              nullptr;  // Shortcut: take ownership of the reference count and
                        // prevent entry from releasing it

          list_pending_request_entry_.pop_front();
          break;
        }

        // If there's no request, go to sleep for a bit
        (void)cv_pending_request_.wait_until(
            ul, std::chrono::steady_clock::now() +
                    std::chrono::duration<float>(0.5f));
      }
    }

    // Exit if we're not supposed to run
    if (!run_request_threads_) {
      // If we retrieved a pending request already, cancel and release it
      if (pasyncresultprovider_message != nullptr) {
        pasyncresultprovider_message->SetDone(Status::Canceled);
        pasyncresultprovider_message->Release();
      }
      break;
    }

    // If the request has been canceled, ignore it and go to the next
    if (!pasyncresultprovider_message->FIsCanceled()) {
      // Send the request to the server and queue entry to wait for response
      Status status =
          SendRequest(str_method, json_params, pasyncresultprovider_message);

      // If the send failed, complete the async result with the error
      if (status != Status::OK) pasyncresultprovider_message->SetDone(status);
    }

    // Done with request in this method
    pasyncresultprovider_message->Release();

    // Advance request ID
    ++request_id_next_;
  }
}

void Client::Impl::ResponseReceivingThreadProc(void) {
  while (run_request_threads_) {
    json json_params;          // Method parameters
    RequestID request_id = 0;  // Request ID to for which to get response
    internal::TAsyncResultProvider<Message>* pasyncresultprovider_message =
        nullptr;  // Pointer to async result provider to receive the result

    // Get the next request for which we need to wait for response
    {
      std::unique_lock ul(mutex_message_send_);

      while (run_request_threads_) {
        if (!list_pending_response_entry_.empty()) {
          auto& pending_response_entry = list_pending_response_entry_.front();

          request_id = pending_response_entry.request_id;
          pasyncresultprovider_message =
              pending_response_entry.pasyncresultprovider_message;
          pending_response_entry.pasyncresultprovider_message =
              nullptr;  // Shortcut: take ownership of the reference count and
                        // prevent entry from releasing it

          list_pending_response_entry_.pop_front();
          break;
        }

        // If there's no request, go to sleep for a bit
        (void)cv_pending_response_.wait_until(
            ul, std::chrono::steady_clock::now() +
                    std::chrono::duration<float>(0.5f));
      }
    }

    // Exit if we're not supposed to run
    if (!run_request_threads_) {
      // If we retrieved a pending response already, cancel and release it
      if (pasyncresultprovider_message != nullptr) {
        pasyncresultprovider_message->SetDone(Status::Canceled);
        pasyncresultprovider_message->Release();
      }
      break;
    }

    // If the response has been canceled, ignore it and go to the next
    if (!pasyncresultprovider_message->FIsCanceled()) {
      int err;

      // Wait for the reply from the server
      for (;;) {
        size_t cb_response;
        uint8_t* rgb_response;

        err = nngi::nng_recv(nng_socket_services_, &rgb_response, &cb_response,
                             NNG_FLAG_ALLOC);
        if (err == nngi::NNG_ETIMEDOUT) {
          // Check to see whether we should exit
          if (run_request_threads_)
            continue;  // Nope, keep going
          else {
            // Yes, cancel the request and exit
            pasyncresultprovider_message->SetDone(Status::Canceled);
          }
        } else if ((err == nngi::NNG_ECANCELED) || (err == nngi::NNG_ECLOSED)) {
          // Request was canceled or socket was closed--cancel the request
          pasyncresultprovider_message->SetDone(Status::Canceled);
        } else if (err != 0) {
          log.ErrorF(
              "Client: error reading reply to request #%d from server: %s",
              request_id, nngi::nng_strerror(err));
          pasyncresultprovider_message->SetDone(internal::ErrNNGToStatus(err));
        } else {
          // Complete the async result with the response from the server
          Message message;

          // Found it, set the reply to the async result provider and complete
          // the task
          message.SetMessageBuffer(
              new NNGMessageBuffer(rgb_response, cb_response));
          pasyncresultprovider_message->SetDoneResult(std::move(message));
        }
        break;
      }
    }

    // Done processing response
    pasyncresultprovider_message->Release();
  }
}

// Note: This method bypasses the request queue and is intended for direct use
// only by the send process thread or by methods like CancelAllTasks() that
// send requests outside the queue.
Status Client::Impl::SendRequest(
    const std::string& str_method, const nlohmann::json& json_parameters,
    internal::TAsyncResultProvider<Message>* parp) {
  Status status = Status::OK;

  try {
    int err;
    std::lock_guard lg(mutex_message_send_);
    RequestMessage request_message(request_id_next_, str_method,
                                   json_parameters, client::kClientAPIVersion);
    std::string str = request_message.Serialize();

    if ((err = nngi::nng_send(nng_socket_services_, str.data(), str.size(),
                              0)) != 0)
      status = internal::ErrNNGToStatus(err);
    else {
      // Queue entry to be handled by response receiving thread
      list_pending_response_entry_.emplace_back(request_id_next_, parp);
      cv_pending_response_.notify_all();
      ++request_id_next_;
    }
  } catch (...) {
    status = Status::Failed;
  }

  return (status);
}

Status Client::Impl::Subscribe(const char* sz_topic_name,
                               TopicCallback topic_callback) {
  Status status = Status::OK;
  std::unique_ptr<internal::Topics::SubscriptionToken> psubscription_token;
  VecPSubscriptionToken* pvec_psubscription_token;
  auto it = um_str_vecpsubscription_token_.find(sz_topic_name);

  // Get or create the subscription token list for the topic
  if (it == um_str_vecpsubscription_token_.end()) {
    auto pair = um_str_vecpsubscription_token_.insert(
        std::make_pair(sz_topic_name, VecPSubscriptionToken()));

    it = pair.first;
  }
  pvec_psubscription_token = &it->second;

  // Subscribe to the topic
  if ((status = topics_.Subscribe(sz_topic_name, topic_callback,
                                  &psubscription_token)) == Status::OK)
    pvec_psubscription_token->emplace_back(std::move(psubscription_token));
  else if (pvec_psubscription_token->empty())
    um_str_vecpsubscription_token_.erase(it);

  return (status);
}

Status Client::Impl::Unsubscribe(const char* sz_topic_name) {
  um_str_vecpsubscription_token_.erase(sz_topic_name);

  return (Status::OK);
}

Status Client::Impl::Unsubscribe(const std::vector<std::string>& vec_topic_name,
                                 bool fbypass_connected_check) {
  Status status;
  json jsonParams = {{"topic_paths", vec_topic_name}};
  TAsyncResult<Message> ar_message;

  // We'll use the service method to unsubscribe multiple topics at once instead
  // of the topics channel and unsubscribe each topic individually.
  ar_message = RequestAsync("/Sim/Unsubscribe", jsonParams, nullptr,
                            fbypass_connected_check);
  status = ar_message.Wait();
  if (status != Status::OK) {
    char sz_msg[256];

    GetStatusString(status, sz_msg);
    log.ErrorF("Failed to unsubscribe topics: %s", sz_msg);
  } else {
    const projectairsim::client::Message& message_reply =
        ar_message.GetResult();

    try {
      ResponseMessage response_message;
      int err;

      response_message.Deserialize(message_reply);
      err = response_message.GetErrorCode();
      if (err != 0) {
        auto json_error = response_message.GetResult();
        std::string message = json_error["message"];

        log.ErrorF("Failed to unsubscribe topics: error %d - %s", err,
                   message.c_str());
        return (Status::RejectedByServer);
      }
    } catch (std::exception e) {
      log.ErrorF("Failed to unsubscribe topics--internal error: %s", e.what());
    } catch (...) {
      log.Error("Failed to unsubscribe topics--unknown internal error");
    }
  }

  // Since we directly canceled the subscriptions instead of going through the
  // topic object, invalidate the associated subscription tokens to avoid
  // unnecessary requests to the server to unsubscribe
  topics_.InvalidateTopics(vec_topic_name);

  // Now get rid of the relevant subscription tokens we hold
  {
    auto it_end = um_str_vecpsubscription_token_.end();

    // Collect affected subscription tokens
    for (auto& str : vec_topic_name) {
      auto it = um_str_vecpsubscription_token_.find(str);

      if (it != it_end) um_str_vecpsubscription_token_.erase(it);
    }
  }

  return (status);
}

Status Client::Impl::UnsubscribeAll(bool fbypass_connected_check) noexcept {
  // Not subscribed to anything?
  if (um_str_vecpsubscription_token_.empty()) return (Status::OK);

  std::vector<std::string> vec_topics;

  vec_topics.reserve(um_str_vecpsubscription_token_.size());
  for (auto& pair : um_str_vecpsubscription_token_)
    vec_topics.push_back(pair.first);

  return (Unsubscribe(vec_topics, fbypass_connected_check));
}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
