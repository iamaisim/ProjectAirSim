// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/client_authorization.hpp"

#include <chrono>
#include <string>

#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/simulator.hpp"

namespace microsoft {
namespace projectairsim {

class ClientAuthorization::Impl {
 public:
  Impl(void);
  virtual ~Impl() {}

  virtual bool IsAuthorized(void) const;
  virtual bool IsAuthorizedMethod(const std::string& method_name) const;
  virtual void RegisterServiceMethods(
      const std::string& topic_name,
      FnRegisterServiceMethod fnregisterservicemethod);
  virtual bool SetPublicKey(const char* pch, size_t cch) = 0;
  void SetLogger(const Logger& logger);
  virtual uint64_t SetToken(const char* pch, size_t cch) = 0;

 protected:
  static constexpr std::chrono::system_clock::time_point
      kTimePointAlwaysAuthorized = std::chrono::system_clock::time_point::
          max();  // Set in time_point_when_expired_ to indicate any client is authorized without needing a token
  static constexpr std::chrono::system_clock::time_point
      kTimePointNotAuthorized = std::chrono::system_clock::time_point::
          min();  // Set in time_point_when_expired_ to indicate clients are not currently authorized
  static const std::chrono::system_clock::time_point
      kTimePointEpochStart;  // The timepoint corresponding to token timestamp 0

 protected:
  virtual std::string GetAuthorizationTokenPublicKey(void) = 0;
  virtual uint64_t SetAuthorizationToken(
      const std::string& str_token_encrypted_base64);

 protected:
  Logger logger_;  // Logging object
  std::string
      str_base64_token_encryption_public_key_;  // Public key for encrypting
                                                // authorization token
  std::string str_component_name_;              // Component name for logging
  std::chrono::system_clock::time_point
      time_point_when_expired_;  // Timestamp when the authorization expires
  std::string
      str_sm_get_authorization_token_public_key_;  // Full path name of
                                                   // get_authorization_token_public_key
                                                   // service method
  std::string
      str_sm_set_authorization_token_;  // Full path name of
                                        // set_authorization_token_public_key
                                        // service method
  std::string str_sm_unsubscribe_;      // Full path name of
                                        // unsubscribe service method
};                                      // class ClientAuthorization::Impl

bool ClientAuthorization::IsAuthorized(void) const {
  return (pimpl_->IsAuthorized());
}

bool ClientAuthorization::IsAuthorizedMethod(
    const std::string& method_name) const {
  return (pimpl_->IsAuthorizedMethod(method_name));
}

void ClientAuthorization::RegisterServiceMethods(
    const std::string& topic_name,
    FnRegisterServiceMethod fnregisterservicemethod) {
  pimpl_->RegisterServiceMethods(topic_name, fnregisterservicemethod);
}

void ClientAuthorization::SetLogger(const Logger& logger) {
  pimpl_->SetLogger(logger);
}

bool ClientAuthorization::SetPublicKey(const char* pch, size_t cch) {
  return (pimpl_->SetPublicKey(pch, cch));
}

uint64_t ClientAuthorization::SetToken(const char* pch, size_t cch) {
  return (pimpl_->SetToken(pch, cch));
}

const std::chrono::system_clock::time_point
    ClientAuthorization::Impl::kTimePointEpochStart =
        std::chrono::system_clock::from_time_t(0);

ClientAuthorization::Impl::Impl(void)
    : logger_([](const std::string&, LogLevel, const std::string&) {},
              LogLevel::kError),
      str_base64_token_encryption_public_key_(),
      str_component_name_(Constant::Component::client_authorization),
      time_point_when_expired_(kTimePointAlwaysAuthorized) {}

bool ClientAuthorization::Impl::IsAuthorized(void) const {
  bool fIsAuthorized = false;

  if (time_point_when_expired_ == kTimePointAlwaysAuthorized)
    fIsAuthorized = true;
  else if (time_point_when_expired_ != kTimePointNotAuthorized) {
    auto time_point_now = std::chrono::system_clock::now();

    fIsAuthorized = (time_point_now < time_point_when_expired_);
  }

  return (fIsAuthorized);
}

bool ClientAuthorization::Impl::IsAuthorizedMethod(
    const std::string& method_name) const {
  // Method is authorized when client is authorized or when the method is always
  // allowed.
  return (IsAuthorized() ||
          (method_name == str_sm_get_authorization_token_public_key_) ||
          (method_name == str_sm_set_authorization_token_) ||
          (strncmp(method_name.c_str(), str_sm_unsubscribe_.c_str(),
                   str_sm_unsubscribe_.size()) == 0));
}

void ClientAuthorization::Impl::RegisterServiceMethods(
    const std::string& topic_name,
    FnRegisterServiceMethod fnregisterservicemethod) {
  // Register GetAuthorizationTokenPublicKey
  {
    auto method = ServiceMethod("GetAuthorizationTokenPublicKey", {""});
    auto method_handler = method.CreateMethodHandler(
        &Impl::GetAuthorizationTokenPublicKey, *this);
    fnregisterservicemethod(method, method_handler);
    str_sm_get_authorization_token_public_key_ =
        topic_name + "/" + "GetAuthorizationTokenPublicKey";
  }

  // Register SetAuthorizationToken
  {
    auto method =
        ServiceMethod("SetAuthorizationToken", {"token_encrypted_base64"});
    auto method_handler =
        method.CreateMethodHandler(&Impl::SetAuthorizationToken, *this);
    fnregisterservicemethod(method, method_handler);
    str_sm_set_authorization_token_ =
        topic_name + "/" + "SetAuthorizationToken";
  }

  str_sm_unsubscribe_ = topic_name + "/" + "Unsubscribe";
}

uint64_t ClientAuthorization::Impl::SetAuthorizationToken(
    const std::string& token_encrypted_base64) {
  return (
      SetToken(token_encrypted_base64.data(), token_encrypted_base64.size()));
}

void ClientAuthorization::Impl::SetLogger(const Logger& logger) {
  logger_ = logger;
}

bool ClientAuthorization::Impl::SetPublicKey(const char* sz, size_t cch) {
  time_point_when_expired_ =
      (cch == 0) ? kTimePointAlwaysAuthorized : kTimePointNotAuthorized;
  return (true);
}

}  // namespace projectairsim
}  // namespace microsoft

#ifdef _WIN32

#include "client_authorization_win.cpp"

microsoft::projectairsim::ClientAuthorization::ClientAuthorization(void)
    : pimpl_(std::make_shared<microsoft::projectairsim::ImplWin>()) {}
microsoft::projectairsim::ClientAuthorization::ClientAuthorization(
    const Logger& logger)
    : pimpl_(std::make_shared<microsoft::projectairsim::ImplWin>()) {
  SetLogger(logger);
}

#elif defined(__linux__)

#include "client_authorization_linux.cpp"

microsoft::projectairsim::ClientAuthorization::ClientAuthorization(void)
    : pimpl_(std::make_shared<microsoft::projectairsim::ImplLinux>()) {}
microsoft::projectairsim::ClientAuthorization::ClientAuthorization(
    const Logger& logger)
    : pimpl_(std::make_shared<microsoft::projectairsim::ImplLinux>()) {
  SetLogger(logger);
}

#endif  //_WIN32
