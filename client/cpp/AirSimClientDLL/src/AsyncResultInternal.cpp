// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "AsyncResultInternal.h"

#include "AirSimClient.h"
#include "pch.h"

namespace microsoft {
namespace projectairsim {
namespace client {
namespace internal {

AsyncResultMessageConverter::AsyncResultMessageConverter(
    AsyncResultMessageConverter&& armc_other)
    : AsyncResult(std::move(armc_other)) {}

AsyncResultMessageConverter::AsyncResultMessageConverter(
    const TAsyncResult<Message>& ar_message)
    : AsyncResult(new AsyncResultProviderMessageConverter(ar_message)) {}

AsyncResultMessageConverter::AsyncResultMessageConverter(
    TAsyncResult<Message>&& ar_message)
    : AsyncResult(
          new AsyncResultProviderMessageConverter(std::move(ar_message))) {}

bool AsyncResultMessageConverter::FIsDone(void) const {
  return (static_cast<AsyncResultProviderMessageConverter*>(piref_counted_)
              ->FIsDone());
}

Status AsyncResultMessageConverter::Wait(void) {
  return (static_cast<AsyncResultProviderMessageConverter*>(piref_counted_)
              ->Wait());
}

AsyncResultProviderMessageConverter::AsyncResultProviderMessageConverter(
    const TAsyncResult<Message>& ar_message)
    : TRefCounted(),
      piasyncresultprovider_(reinterpret_cast<TAsyncResultProvider<Message>*>(
          AsyncResultManager::GetPSelfDelete(ar_message))) {
  // Add a reference to the wrapped async result provider for us
  piasyncresultprovider_->AddRef();
}

AsyncResultProviderMessageConverter::AsyncResultProviderMessageConverter(
    TAsyncResult<Message>&& ar_message)
    : TRefCounted(),
      piasyncresultprovider_(reinterpret_cast<TAsyncResultProvider<Message>*>(
          AsyncResultManager::GetPSelfDelete(ar_message))) {
  // Take ownership of the wrapped async result provider
  AsyncResultManager::GetPSelfDelete(ar_message) = nullptr;
}

AsyncResultProviderMessageConverter::~AsyncResultProviderMessageConverter() {
  if (piasyncresultprovider_ != nullptr) piasyncresultprovider_->Release();
}

void AsyncResultProviderMessageConverter::Cancel(void) {
  if (piasyncresultprovider_ != nullptr) piasyncresultprovider_->Cancel();
}

bool AsyncResultProviderMessageConverter::FIsDone(void) const {
  if (piasyncresultprovider_ == nullptr) return (false);

  return (piasyncresultprovider_->FIsDone());
}

Status AsyncResultProviderMessageConverter::Wait(void) {
  Status status;

  if (piasyncresultprovider_ == nullptr) return (Status::Failed);

  status = piasyncresultprovider_->Wait();
  if (status == Status::OK) {
    auto& message = piasyncresultprovider_->GetResult();
    ResponseMessage rm;

    rm.Deserialize(message);
    if (rm.GetErrorCode() != 0) {
      auto json_result = rm.GetResult();
      json json_message = json_result["message"];

      if (json_message.is_string())
        client::log.InfoF("Error from server: %s",
                          static_cast<std::string>(json_message).c_str());

      status = Status::RejectedByServer;
    }

    return (status);
  }

  return (status);
}

void TAsyncResultProvider<Message>::InvokeCallback(const Message& message) {
  if (fn_response_callback != nullptr) {
    ResponseMessage rm;

    // Deserialize server message
    try {
      rm.Deserialize(message);
    } catch (std::exception e) {
      log.ErrorF(
          "Caught exception deserializing server's reply message to "
          "ResponseMessage: %s",
          e.what());
    }

    // Invoke async response callback
    try {
      fn_response_callback(rm.GetErrorCode(), rm.GetResult().dump());
    } catch (std::exception e) {
      log.ErrorF(
          "Caught exception invoking asynchronous response message callback: "
          "%s",
          e.what());
    }
  }
}

}  // namespace internal
}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
