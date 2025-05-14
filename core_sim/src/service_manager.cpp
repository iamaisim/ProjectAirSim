// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/service_manager.hpp"

#include <exception>
#include <map>
#include <shared_mutex>
#include <string>
#include <utility>

#include "constant.hpp"
#include "core_sim/error.hpp"
#include "core_sim/json_utils.hpp"
#include "core_sim/message/request_message.hpp"
#include "core_sim/message/response_failure_message.hpp"
#include "core_sim/message/response_success_message.hpp"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"
#include "nng/supplemental/util/platform.h"

namespace microsoft {
namespace projectairsim {

// ----------------------------------------------------------------------------

class ServiceManager::Impl {
 public:
  explicit Impl(const Logger& logger,
                const ClientAuthorization& client_authorization);

  void Load(const json& config_json);

  void RegisterMethod(const ServiceMethod& method,
                      MethodHandler method_handler);

  void UnregisterMethod(const ServiceMethod& method);

  void UnregisterAllMethods();

  void Start();

  void Stop();

  enum class JobState { kListening = 0, kProcessing = 1 };

  enum class SerializationType { kMsgpackJSON = 0, kPureJSON = 1 };

  struct Job {
    // Job state
    nng_aio* aio;
    nng_ctx ctx;
    const std::map<std::string, MethodHandler>* methods_registry;
    const std::map<std::string, ParamsList>* method_parameters_registry;
    const std::string* parent_name;
    const std::atomic<bool>* parent_is_alive;
    const Logger* logger;
    const ClientAuthorization* client_authorization;
    JobState state;

    // Request data
    int request_id;
    std::string request_method;
    json request_params;
  };

 private:
  static std::string RequestResponse(TimeNano service_method_start_time,
                                     const Job* job);

  static std::string ProcessRequest(
      TimeNano service_method_start_time, int id, std::string method_name,
      MethodHandler method_handler, json params,
      const std::map<std::string, ParamsList> method_params_registry,
      const std::string name, const Logger logger,
      const ClientAuthorization* p_client_authorization);

  static void HandleNNGPipeEventProxy(nng_pipe pipe, nng_pipe_ev ev,
                                      void* arg) {
    reinterpret_cast<ServiceManager::Impl*>(arg)->HandleNNGPipeEvent(pipe, ev);
  }

  template <typename ResponseMessageType>
  static std::string SerializeResponse(const ResponseMessageType& response);

  struct ServiceComparator {
    bool operator()(const ServiceMethod& a, const ServiceMethod& b) const {
      return a.GetName().length() < b.GetName().length();
    }
  };

  void CreateJob(nng_socket sock, const ServiceManager::Impl* manager,
                 Job* new_job);

  static void ResetJob(Job* job);

  static void RunJob(Job* job);

  static void JobRunner(void* job);

  void HandleNNGPipeEvent(nng_pipe pipe, nng_pipe_ev ev);

  //! Default configs to make services work "out-of-the-box";
  static constexpr const char* kDefaultLocalAddress = "*";  // ANY
  static constexpr const int kDefaultServicePort = 8990;
  nng_socket default_service_socket_ = NNG_SOCKET_INITIALIZER;

  // Set a maximum limit to the number of concurrent requests that can be
  // queued. Takes a few KBs per request-response loop; But, memory (not CPU
  // thread) limits apply; Also limited by number of available file discriptors
  // TODO: Expose as config
  static constexpr const int kMaxConcurrentJobs = 64;

  // TODO make this a global extern string
  static constexpr float kApiVersion = 1.0f;

  static constexpr SerializationType kSerializationType =
      SerializationType::kMsgpackJSON;

  std::string name_;
  Logger logger_;
  std::string service_address_;
  int service_port_;
  nng_socket service_socket_;
  ClientAuthorization client_authorization_;
  std::atomic<bool> is_alive_;
  std::map<std::string, MethodHandler> methods_registry_;
  std::map<std::string, ParamsList> method_parameters_registry_;
  mutable std::shared_timed_mutex service_manager_lock_;
  Job jobs_[kMaxConcurrentJobs];
};

// ----------------------------------------------------------------------------

ServiceManager::ServiceManager(const Logger& logger,
                               const ClientAuthorization& client_authorization)
    : pimpl_(std::make_shared<Impl>(logger, client_authorization)) {}

ServiceManager::ServiceManager(const Logger& logger)
    : pimpl_(std::make_shared<Impl>(logger, ClientAuthorization())) {}

void ServiceManager::Load(const json& config_json) {
  pimpl_->Load(config_json);
}

void ServiceManager::RegisterMethod(const ServiceMethod& method,
                                    MethodHandler method_handler) {
  pimpl_->RegisterMethod(method, method_handler);
}

void ServiceManager::UnregisterMethod(const ServiceMethod& method) {
  pimpl_->UnregisterMethod(method);
}

void ServiceManager::UnregisterAllMethods() { pimpl_->UnregisterAllMethods(); }

void ServiceManager::Start() { pimpl_->Start(); }

void ServiceManager::Stop() { pimpl_->Stop(); }

// ----------------------------------------------------------------------------

// Construct using default configs
ServiceManager::Impl::Impl(const Logger& logger,
                           const ClientAuthorization& client_authorization)
    : logger_(logger),
      name_(Constant::Component::service_manager),
      service_port_(kDefaultServicePort),
      service_socket_(default_service_socket_),
      client_authorization_(client_authorization) {}

void ServiceManager::Impl::Load(const json& config_json) {
  service_address_ = JsonUtils::GetString(config_json, Constant::Config::ip,
                                          kDefaultLocalAddress);

  service_port_ = JsonUtils::GetInteger(config_json, Constant::Config::port,
                                        kDefaultServicePort);
}

void ServiceManager::Impl::Start() {
  std::unique_lock<std::shared_timed_mutex> exclusive_lock(
      service_manager_lock_);
  logger_.LogTrace(name_, "Starting ServiceManager");

  int rv = nng_rep0_open(&service_socket_);
  if (rv != 0) {
    auto errno_str = nng_strerror(rv);
    logger_.LogError(name_, "nng_rep0_open failed with '%s'.", errno_str);
    throw Error("Error initializing services socket for serving requests.");
  }
  logger_.LogTrace(name_, "nng socket for services created '%d'.",
                   service_socket_);

  // Initialize jobs and bind their AIO callbacks to JobRunner() state machine
  for (int job_num = 0; job_num < kMaxConcurrentJobs; job_num++) {
    CreateJob(service_socket_, this, &jobs_[job_num]);
  }

  rv = nng_pipe_notify(service_socket_, NNG_PIPE_EV_REM_POST,
                       &ServiceManager::Impl::HandleNNGPipeEventProxy, this);
  if (rv != 0) {
    auto errno_str = nng_strerror(rv);
    logger_.LogError(name_, "nng_pipe_notify failed with '%s'.", errno_str);
    throw Error(
        "Error installing event listener on services socket for serving "
        "requests.");
  }

  std::stringstream network_address_sstream;
  network_address_sstream << "tcp://" << service_address_ << ":"
                          << service_port_;
  if ((rv = nng_listen(service_socket_, network_address_sstream.str().c_str(),
                       NULL, 0)) != 0) {
    std::string error_msg =
        "nng_listen failed with failure value:" + std::to_string(rv);
    logger_.LogError(name_, error_msg.c_str());
    throw Error(error_msg);
  }

  is_alive_ = true;

  // Kick-start the async job runners to start listening for requests
  for (int job_num = 0; job_num < kMaxConcurrentJobs; job_num++) {
    JobRunner(&jobs_[job_num]);
  }

  logger_.LogTrace(name_, "Services active at: '%s'.",
                   network_address_sstream.str().c_str());
}

void ServiceManager::Impl::Stop() {
  std::unique_lock<std::shared_timed_mutex> exclusive_lock(
      service_manager_lock_);
  logger_.LogTrace(name_, "Stopping ServiceManager");

  is_alive_ = false;

  auto error = nng_close(service_socket_);
  if (error != 0) {
    auto errno_str = nng_strerror(error);
    logger_.LogError(name_, "nng_close for services failed with '%s'.",
                     errno_str);
  }

  // Stop all AIO Runners & free the async I/O handle
  for (int job_num = 0; job_num < kMaxConcurrentJobs; job_num++) {
    // cancel -> wait -> stop -> free
    nng_aio_free(jobs_[job_num].aio);
  }

  service_socket_ = default_service_socket_;
}

void ServiceManager::Impl::RegisterMethod(const ServiceMethod& method,
                                          MethodHandler method_handler) {
  // TODO: Assert method.ParamsList === method_handler's ParamsList
  methods_registry_.insert(std::make_pair(method.GetName(), method_handler));

  method_parameters_registry_.insert(
      std::make_pair(method.GetName(), method.GetParamsList()));
}

void ServiceManager::Impl::UnregisterMethod(const ServiceMethod& method) {
  const std::string& method_name = method.GetName();
  auto iter = methods_registry_.find(method_name);
  if (iter == methods_registry_.end()) {
    logger_.LogWarning(name_,
                       "Cannot Unregister ServiceMethod '%s'; Not found in the "
                       "methods registry.",
                       method_name.c_str());
    return;
  }
  methods_registry_.erase(iter->first);

  logger_.LogVerbose(name_, "Unregistered ServiceMethod '%s'",
                     method_name.c_str());
}

void ServiceManager::Impl::UnregisterAllMethods() {
  methods_registry_.clear();
  logger_.LogVerbose(name_, "Unregistered ALL ServiceMethods");
}

std::string ServiceManager::Impl::RequestResponse(
    TimeNano service_method_start_time, const Job* job) {
  int request_id = job->request_id;
  //! Find the handler for the requested method
  auto requested_method = job->request_method;
  auto iter = job->methods_registry->find(requested_method);
  if (iter == job->methods_registry->end()) {
    std::string error_msg = "Requested ServiceMethod " + requested_method +
                            " is not supported by Request-Response Service";
    // TODO: Use a local logger
    // logger_.LogError(name_, error_msg.c_str());
    json error = {{"code", 1.0}, {"message", error_msg}};

    return SerializeResponse(
        ResponseFailureMessage(request_id, error, kApiVersion));
  }

  MethodHandler method_handler = iter->second;
  std::string job_name = *(job->parent_name) + ":::" + requested_method;
  // TODO: Need to sanitize request_msg or it's fields (params)?
  json parameters = job->request_params;

  //! Launch the handler with the provided parameters
  std::string response = ServiceManager::Impl::ProcessRequest(
      service_method_start_time, request_id, requested_method, method_handler,
      parameters, *(job->method_parameters_registry), job_name, *(job->logger),
      &*(job->client_authorization));

  return response;
}

std::string ServiceManager::Impl::ProcessRequest(
    TimeNano service_method_start_time, int id, std::string method_name,
    MethodHandler method_handler, json params,
    const std::map<std::string, ParamsList> method_parameters_registry,
    const std::string name, const Logger logger,
    const ClientAuthorization* client_authorization) {
  // A vector to store the positional args to be supplied to the method_handler
  std::vector<json> args;
  bool request_params_valid = true;

  const auto& params_list = method_parameters_registry.at(method_name);
  std::string error_msg;
  // Preserve ordering of the args based on the ordering of the service
  // method's ParamsList provided during service method registration
  // NOTE: Unsupported arguments that were provided with the request will be
  // ignored silently
  // TODO: Break the silence and warn the dev about unused args?
  for (const auto& param_name : params_list) {
    if (param_name == "_service_method_start_time") {
      args.push_back(json(service_method_start_time));
      continue;
    }

    if (!param_name.empty()) {
      if (!params.contains(param_name)) {
        // Required parameter(s) doesn't exist in the provided parameters.
        error_msg = "Required parameter '" + param_name +
                    +"' not provided when calling:" + method_name +
                    +". Provided parameters:" + params.dump();
        request_params_valid = false;
        break;  // or throw Error(error_msg);
      }
      // Required parameter exists in the provided parameters. Add to args in
      // the expected position
      args.push_back(json(params.at(param_name)));
    }
  }

  if (request_params_valid) {
    // Verify the client has authorization
    if (!client_authorization->IsAuthorizedMethod(method_name)) {
      json error;
      error["code"] =
          4.0;  // Make this more meaningful; Using an error registry?
      error["message"] = "Client is not authorized";
      return SerializeResponse(ResponseFailureMessage(id, error, kApiVersion));
    }

    try {
      auto result_json = method_handler(args);
      // TODO: validate the field(s) in the results returned;
      // Follow-up TODO: Should result fields list also be added to
      // Message decl?
      return SerializeResponse(
          ResponseSuccessMessage(id, result_json, kApiVersion));
    } catch (Error e) {
      json error;
      error["code"] =
          1.0;  // Make this more meaningful; Using an error registry?
      error["message"] = e.what();
      return SerializeResponse(ResponseFailureMessage(id, error, kApiVersion));
    } catch (std::exception e) {
      json error;
      error["code"] =
          2.0;  // Make this more meaningful; Using an error registry?
      error["message"] = e.what();
      return SerializeResponse(ResponseFailureMessage(id, error, kApiVersion));
    } catch (...) {
      json error;
      error["code"] =
          2.0;  // Make this more meaningful; Using an error registry?
      error["message"] = "[ServiceManager] Unknown exception";
      return SerializeResponse(ResponseFailureMessage(id, error, kApiVersion));
    }
  } else {
    // Required parameters doesn't exist in the provided parameters.
    // TODO: Use local logger
    // logger.LogError(name, error_msg.c_str());
    json error = {{"code", 3.0}, {"message", error_msg}};
    return SerializeResponse(ResponseFailureMessage(id, error, kApiVersion));
  }
}

void ServiceManager::Impl::JobRunner(void* job) {
  // A wrapper to RunJob with a NNG compatible callback interface (void*)
  RunJob(static_cast<struct ServiceManager::Impl::Job*>(job));
}

void ServiceManager::Impl::RunJob(Job* job) {
  if (job->parent_is_alive->load() == false) return;

  //! State Machine to handle async requests job
  RequestMessage request_msg;
  const char* request_buffer;
  std::string request_buffer_str;
  std::string response;
  nng_msg* request_nng_msg;
  nng_msg* response_nng_msg;
  int rv;
  TimeNano service_method_start_time = 0;

  switch (job->state) {
    case JobState::kListening:
      //! 0. Check if a prev response was not sent successfully
      if ((rv = nng_aio_result(job->aio)) != 0) {
        // job->logger->LogError(*(job->parent_name), "nng_ctx_send: %d", rv);
      }

      //! 1. Get ready to start listening to receive a new request
      ResetJob(job);

      // Move state to be processing for when it comes back to this state
      // machine by the AIO callback after completing receiving a new request by
      // the following NNG receive call.
      job->state = JobState::kProcessing;
      // Start NNG async receive loop. When a request is received, it will come
      // back by callback to re-enter this state machine method.
      nng_ctx_recv(job->ctx, job->aio);
      // No further processing should be done here to avoid overlapping state
      // for this job with the callback called by nng_ctx_recv() above.
      return;

    case JobState::kProcessing:
      service_method_start_time = SimClock::Get()->NowSimNanos();

      //! 0. Check if prev request was not received successfully
      if ((rv = nng_aio_result(job->aio)) != 0) {
        // job->logger->LogError(*(job->parent_name), "nng_ctx_recv: %d", rv);
      }

      //! 1. Retrieve the request message
      request_nng_msg = nng_aio_get_msg(job->aio);

      if (request_nng_msg == nullptr) {
        // Handle bad or null request message
        // job->logger->LogError(*(job->parent_name),
        //                       "nng_aio_get_msg: Got bad request");
        response = "Error: Got bad request message.";
      } else {
        //! 2. Extract the request body
        request_buffer =
            static_cast<const char*>(nng_msg_body(request_nng_msg));
        std::string request_buffer_str;
        size_t request_len = nng_msg_len(request_nng_msg);
        request_buffer_str.resize(request_len);
        request_buffer_str.assign(request_buffer, request_len);

        if (request_len < 1)
          response = "Error: Got bad request message.";
        else {
          try {
            //! 3. Deserialize the request body
            switch (kSerializationType) {
              case (SerializationType::kPureJSON):
                request_msg.DeserializeFromJsonStr(request_buffer_str);
                break;

              case (SerializationType::kMsgpackJSON):
              default:
                request_msg.Deserialize(request_buffer_str);
                break;
            }

            //! 4. Get the request data
            job->request_id = request_msg.GetID();
            job->request_method = request_msg.GetMethod();
            job->request_params = request_msg.GetParams();

            //! 5. Process requested method and get response
            response = ServiceManager::Impl::RequestResponse(
                service_method_start_time, job);
          } catch (...) {
            response = "Error: Got bad request message.";
          }
        }
      }

      //! 6. Pack response into NNG message to send back
      // Alloc mem for job->response_nng_msg.body
      if ((rv = nng_msg_alloc(&response_nng_msg, response.length())) != 0) {
        // job->logger->LogError(*(job->parent_name),
        //                       "While preparing response, nng_msg_alloc:
        //                       %d", rv);
        nng_msg_free(response_nng_msg);
      } else {
        // Set response data
        std::memcpy(nng_msg_body(response_nng_msg), response.c_str(),
                    response.length());
      }

      //! 7. Set the response message to be sent
      nng_aio_set_msg(job->aio, response_nng_msg);

      //! 8. Send the response
      // Move state to be listening for when it comes back to this state
      // machine by the AIO callback after completing sending the response by
      // the following NNG send call.
      job->state = JobState::kListening;
      // Send NNG async response. When the send is completed, it will come
      // back by callback to re-enter this state machine method.
      nng_ctx_send(job->ctx, job->aio);
      // No further processing should be done here to avoid overlapping state
      // for this job with the callback called by nng_ctx_send() above.
      return;

    default: {
      // job->logger->LogError(*(job->parent_name), "RunJob() invalid job
      // state.");
      return;
    }
  }
}

void ServiceManager::Impl::CreateJob(nng_socket sock,
                                     const ServiceManager::Impl* manager,
                                     Job* new_job) {
  if (new_job == nullptr) return;

  // Create AIO object with completion callback as the JobRunner state machine.
  // After each async AIO call completes, it will come back to this callback so
  // we can process the next state and start the next AIO call.
  int rv;
  if ((rv = nng_aio_alloc(&new_job->aio, &ServiceManager::Impl::JobRunner,
                          new_job)) != 0) {
    logger_.LogError(name_, "nng_aio_alloc: %d", rv);
  }

  // Create CTX socket context object
  if ((rv = nng_ctx_open(&new_job->ctx, sock)) != 0) {
    logger_.LogError(name_, "nng_ctx_open: %d", rv);
  }

  // Save management pointers
  new_job->methods_registry = &manager->methods_registry_;
  new_job->method_parameters_registry = &manager->method_parameters_registry_;
  new_job->parent_name = &manager->name_;
  new_job->parent_is_alive = &manager->is_alive_;
  new_job->logger = &manager->logger_;
  new_job->client_authorization = &manager->client_authorization_;

  // Initialize state and job data
  new_job->state = JobState::kListening;
  ResetJob(new_job);
}

void ServiceManager::Impl::ResetJob(Job* job) {
  // Initialize request data
  job->request_id = -1;
  job->request_method = "";
  job->request_params = json();
}

void ServiceManager::Impl::HandleNNGPipeEvent(nng_pipe pipe, nng_pipe_ev ev) {
  if (ev == NNG_PIPE_EV_REM_POST) {
    // Client disconnected--reset client authorization
    logger_.LogVerbose(
        name_, "Req-resp client disconnected--resetting client authorization",
        ev);
    client_authorization_.SetToken(nullptr, 0);
  }
}

template <typename ResponseMessageType>
std::string ServiceManager::Impl::SerializeResponse(
    const ResponseMessageType& msg) {
  switch (kSerializationType) {
    case (SerializationType::kPureJSON):
      return msg.SerializeToJsonStr();

    case (SerializationType::kMsgpackJSON):
    default:
      return msg.Serialize();
  }
}

}  // namespace projectairsim
}  // namespace microsoft
