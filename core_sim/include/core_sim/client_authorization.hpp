// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_CLIENT_AUTHORIZATION_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_CLIENT_AUTHORIZATION_HPP_

#include <stdint.h>

#include <functional>
#include <memory>

#include "core_sim/logger.hpp"
#include "core_sim/service_method.hpp"

namespace microsoft {
namespace projectairsim {

// This class defines the client authorization token structure
// and implements verifying and processing tokens.
class ClientAuthorization {
 public:
#pragma pack(push)
#pragma pack(1)
  // Binary structure of client authorization token
  struct TokenV1 {
    uint8_t version;                // Token version
    uint64_t timestamp_expiration;  // Expiration timestamp in UNIX epoch
                                    // seconds (seconds since 12:00:00 midnight,
                                    // January 1, 1970 UTC)
    uint8_t rgb_signature[1];       // Start of variable-length signature

    static const uint64_t kTimestampExpirationNone =
        0xffffffffffffffff;  // Essentially no expiration
    static const int kRSAKeySizeTokenTransport =
        2048;  // Size of RSA key used for transporting the token
  };           // struct TokenV1

#pragma pack(pop)

  using FnRegisterServiceMethod =
      std::function<void(const ServiceMethod&, MethodHandler)>;

 public:
  explicit ClientAuthorization(void);
  explicit ClientAuthorization(const Logger& logger);

  bool IsAuthorized(void) const;
  bool IsAuthorizedMethod(const std::string& method_name) const;
  void RegisterServiceMethods(const std::string& topic_name,
                              FnRegisterServiceMethod fnregisterservicemethod);
  void SetLogger(const Logger& logger);
  bool SetPublicKey(const char* pch, size_t cch);
  uint64_t SetToken(const char* pch, size_t cch);

 private:
  class Impl;              // Implementation class
  friend class ImplWin;    // Implementation for Windows
  friend class ImplLinux;  // Implementation for Linux

 private:
  std::shared_ptr<Impl> pimpl_;  // Implementation object
};                               // class ClientAuthorization

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_CLIENT_AUTHORIZATION_HPP_
