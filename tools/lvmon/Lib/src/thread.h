// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef TOOLS_LVMON_LIB_SRC_THREAD_H_
#define TOOLS_LVMON_LIB_SRC_THREAD_H_

#include <functional>
#include <thread>

#include "events.h"

namespace LVMon {

class CThread {
 public:
  static const CEvents::Event kEventExitThread =
      0x8000;  // Event to signal thread to exit

 public:
  CThread(void);
  virtual ~CThread();

  virtual bool FShouldStop(void) const { return (fShouldStop_); }
  virtual CEvents &GetEvents(void) { return (events_); }
  virtual void Start(std::function<void(CThread *)> fnThreadProc);
  virtual void StopAsync(void);
  virtual void WaitForStop(void);

 protected:
  static void ThreadProcProxy(CThread *pthread,
                              std::function<void(CThread *)> fnThreadProc);

 protected:
  CEvents events_;    // Events for the thread
  bool fShouldStop_;  // If true, the thread procedure should exit as soon as
                      // possible
  std::thread *pthread_;  // Thread object
};                        // class CThread

}  // namespace LVMon

#endif  // TOOLS_LVMON_LIB_SRC_THREAD_H_
