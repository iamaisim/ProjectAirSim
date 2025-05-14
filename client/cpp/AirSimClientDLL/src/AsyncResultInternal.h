// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#pragma once
#include <AirSimMessage/response_message.hpp>
#include <condition_variable>
#include <mutex>

#include "AsyncResult.h"
#include "RefCounted.h"

namespace microsoft {
namespace projectairsim {
namespace client {
namespace internal {

// Helper class that allows us access to the AsyncResult object's pointer
// to the async result provider
class AsyncResultManager {
 public:
  static IRefCounted*& GetPSelfDelete(AsyncResult& asyncresult) {
    return (asyncresult.piref_counted_);
  }
  static IRefCounted* GetPSelfDelete(const AsyncResult& asyncresult) {
    return (asyncresult.piref_counted_);
  }
};

// Base async result provider implementation that does most of the work
template <typename TAncestor>
class TAsyncResultProviderBase : public TRefCounted<TAncestor> {
 public:
  TAsyncResultProviderBase(void)
      : cv_done_(), fis_done_(false), mutex_(), status_(Status::InProgress) {}

  // Returns whether the task has been requested to cancel the operation
  bool FIsCanceled(void) const { return (fis_canceled_); }

  // Mark the task complete and set the status
  void SetDone(Status status) {
    std::unique_lock ul(mutex_);

    status_ = status;
    fis_done_ = true;
    cv_done_.notify_all();
  }

 public:
  virtual void Cancel(void) override { fis_canceled_ = true; }
  virtual bool FIsDone(void) const override { return (fis_done_); }
  virtual Status Wait(void) override {
    std::unique_lock ul(mutex_);

    while (!fis_done_) cv_done_.wait(ul);

    return (status_);
  }

 protected:
  std::condition_variable
      cv_done_;        // Conditional variable to set when task completes
  bool fis_canceled_;  // If true, the task is requested to cancel the operation
  bool fis_done_;      // If true, the task is complete
  std::mutex mutex_;   // Access guard to this object
  Status status_;      // The task return status
};                     // class TAsyncResultProviderBase

// Async result provider for a task that does not return a value
typedef TAsyncResultProviderBase<IAsyncResultProvider> AsyncResultProvider;

// Async result provider for a task that returns a value
template <typename T>
class TAsyncResultProvider
    : public TAsyncResultProviderBase<TIAsyncResultProvider<T>> {
 public:
  typedef TAsyncResultProviderBase<TIAsyncResultProvider<T>> SuperclassType;

 public:
  TAsyncResultProvider(void)
      : TAsyncResultProviderBase<TIAsyncResultProvider<T>>() {}

  // Mark the task complete and set the status and result return
  void SetDoneResult(T&& t, Status status = Status::OK) {
    t_result_ = std::move(t);
    SuperclassType::SetDone(status);
  }
  void SetDoneResult(const T& t, Status status = Status::OK) {
    t_result_ = t;
    SuperclassType::SetDone(status);
  }

 public:
  virtual const T& GetResult(void) override { return (t_result_); }

 protected:
  T t_result_;  // Result of the task
};              // class TAsyncResultProvider

template <>
class TAsyncResultProvider<Message>
    : public TAsyncResultProviderBase<TIAsyncResultProvider<Message>> {
 public:
  typedef TAsyncResultProviderBase<TIAsyncResultProvider<Message>>
      SuperclassType;

 public:
  TAsyncResultProvider(FnResponseCallback fn_response_callback_in = nullptr)
      : TAsyncResultProviderBase<TIAsyncResultProvider<Message>>(),
        fn_response_callback(fn_response_callback_in) {}

  // Mark the task complete and set the status and result return
  void SetDoneResult(Message&& t, Status status = Status::OK) {
    t_result_ = std::move(t);
    InvokeCallback(t_result_);
    SuperclassType::SetDone(status);
  }
  void SetDoneResult(const Message& t, Status status = Status::OK) {
    t_result_ = t;
    InvokeCallback(t_result_);
    SuperclassType::SetDone(status);
  }

 public:
  virtual const Message& GetResult(void) override { return (t_result_); }

 protected:
  void InvokeCallback(const Message& message);

 protected:
  Message t_result_;                        // Result of the task
  FnResponseCallback fn_response_callback;  // Asynchronous response callback
};  // class TAsyncResultProvider<Message>

// Async result provider handling the response message returned by the server
// for task with no return value.  This class wraps a TAsyncResult<Message> and
// if the result is an error, returns an error status value.
class AsyncResultProviderMessageConverter
    : public TRefCounted<IAsyncResultProvider> {
 public:
  AsyncResultProviderMessageConverter(const TAsyncResult<Message>& ar_message);
  AsyncResultProviderMessageConverter(TAsyncResult<Message>&& ar_message);

 public:
  virtual void Cancel(void) override;
  virtual bool FIsDone(void) const override;
  virtual Status Wait(void) override;

 protected:
  ~AsyncResultProviderMessageConverter();

 protected:
  TAsyncResultProvider<Message>*
      piasyncresultprovider_;  // Pointer to encapsulated result provider
};                             // class TAsyncResultProvider

// Async result provider handling the response message returned by the server
// for a task that returns a value.  This class wraps a TAsyncResult<Message>
// and if the result is an error, returns an error status value.  Otherwise the
// result is extracted from the response message from the server and returns as
// the specified type.
template <typename T>
class TAsyncResultProviderMessageConverter
    : public AsyncResultProviderMessageConverter {
 public:
  TAsyncResultProviderMessageConverter(TAsyncResult<Message>&& ar_message)
      : AsyncResultProviderMessageConverter(ar_message) {}

  // Return the result from the response message
  virtual const T& GetResult(void) const { return (t_result_); }

 public:
  virtual Status Wait(void) override {
    Status status = AsyncResultProviderMessageConverter::Wait();

    if (status == Status::OK) {
      try {
        auto& message = piasyncresultprovider_->GetResult();
        ResponseMessage rm;

        // Get the response message from the server message
        rm.Deserialize(message);
        if (rm.GetErrorCode() == 0) {
          // Save the result
          t_result_ = rm.GetResult();
        } else {
          // Server returned an error
          status = Status::RejectedByServer;
        }
      } catch (...) {
        status = Status::Failed;
      }
    }

    return (status);
  }

 protected:
  T t_result_;  // Result of the task
};              // class TAsyncResultProvider

// Async result provider handling the response message returned by the server
// for task with no return value.  This class wraps a TAsyncResult<Message> and
// if the result is an error, returns an error status value.
class AsyncResultMessageConverter : public AsyncResult {
 public:
  AsyncResultMessageConverter(const TAsyncResult<Message>& ar_message);
  AsyncResultMessageConverter(TAsyncResult<Message>&& ar_message);
  AsyncResultMessageConverter(AsyncResultMessageConverter&& armc_other);

 public:
  virtual bool FIsDone(void) const override;
  virtual Status Wait(void) override;
};  // class AsyncResultMessageConverter

// Async result provider handling the response message returned by the server
// for a task that returns a value.  This class wraps a TAsyncResult<Message>
// and if the result is an error, returns an error status value.  Otherwise the
// result is extracted from the response message from the server and returns as
// the specified type
template <typename T>
class TAsyncResultMessageConverter : public AsyncResultMessageConverter {
 public:
  TAsyncResultMessageConverter(TAsyncResult<Message>&& ar_message)
      : AsyncResultMessageConverter(ar_message) {}

  // Return the result from the response message
  virtual const T& GetResult(void) const {
    return (
        static_cast<TAsyncResultProviderMessageConverter<T>*>(piref_counted_)
            ->GetResult());
  }
};  // class TAsyncResultMessageConverter

}  // namespace internal
}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
