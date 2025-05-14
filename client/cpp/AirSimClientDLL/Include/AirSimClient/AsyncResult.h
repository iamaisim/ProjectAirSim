// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#pragma once
#include <functional>
#include <string>

#include "ASCDecl.h"
#include "IRefCounted.h"
#include "Status.h"

namespace microsoft {
namespace projectairsim {
namespace client {

namespace internal {
class AsyncResultManager;
}  // namespace internal

// Asynchronous function to invoke upon receiving a response to a request
// from the server.  This function is called from an arbitrary thread and
// may not (likely not) from a UI thread.
typedef std::function<void(int errorcode, const std::string& str_json_result)>
    FnResponseCallback;

// AsyncResult provider interface
struct IAsyncResultProvider : public IRefCounted {
  // Cancel the asynchronous task.  If the task was successfully canceled
  // before completion Wait() will return Status::Canceled and otherwise
  // return the operation's completion status.
  virtual void Cancel(void) = 0;

  // Returns whether the task has completed (or was canceled.)  If true,
  // the completion status is available via Wait() which will return
  // immediately.
  virtual bool FIsDone(void) const = 0;

  // Wait until the task has completed and return the completion status.
  // This method may be called multiple times.
  virtual Status Wait(void) = 0;
};  // interface IAsyncResultProvider

// AsyncResult provider interface that also returns a result return value
template <typename T>
struct TIAsyncResultProvider : public IAsyncResultProvider {
  // Returns the result of the operation.  Wait() must have beenn called
  // and returned a success status first otherwise the result of calling
  // this method is undefined.
  virtual const T& GetResult(void) = 0;
};  // struct TIAsyncResultProvider

// Basic asynchronous result with no return value from the task
//(such as a method returning void.)  A function returns an object
// of this class and runs the task asynchronously.  When FIsDone()
// returns true the task is complete and calling Wait() returns the
// status of the task.  A thread can also just call Wait() which
// will block until the task is complete and a status is available.
class AsyncResult : public RefCountedContainer {
 public:
  ASC_DECL AsyncResult(IAsyncResultProvider* piasyncresultprovider = nullptr);

  // Cancel the asynchronous task.  Wait() will return Status::Canceled
  // if the task was canceled before completion and the completion status
  // otherwise.
  ASC_DECL virtual void Cancel(void);

  // If true, the task is complete (or canceled) and a status is available
  // via Wait() which will return immediately.
  ASC_DECL virtual bool FIsDone(void) const;

  // Returns the status of the task.  If the task is still in
  // progress, this method blocks until the task is complete.  This method
  // may be called multiple times.
  ASC_DECL virtual Status Wait(void);

 protected:
  friend class internal::AsyncResultManager;
};  // class AsyncResult

// Asynchronous result with typed result return value
template <typename T>
class TAsyncResult : public AsyncResult {
 public:
  TAsyncResult(TIAsyncResultProvider<T>* piasyncresultprovider = nullptr)
      : AsyncResult(piasyncresultprovider) {}

  // Returns the result of the operation.  Wait() must have been called and
  // returned a success status first otherwise the result of calling this
  // method is undefined.
  const T& GetResult(void) {
    return (
        static_cast<TIAsyncResultProvider<T>*>(piref_counted_)->GetResult());
  }
};  // class TAsyncResult

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
