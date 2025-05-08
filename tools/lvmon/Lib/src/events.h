// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef TOOLS_LVMON_LIB_SRC_EVENTS_H_
#define TOOLS_LVMON_LIB_SRC_EVENTS_H_

#include <condition_variable>
#include <mutex>
#include <unordered_set>

namespace LVMon {

struct CEvents {
 public:
  typedef unsigned int Event;       // ID of an event
  typedef unsigned int WaitResult;  // Result from WaitForMultiple()

 public:
  static const WaitResult kWaitResultEvent0 = 0;  // Wait result for first
                                                  // event
  static const WaitResult kWaitResultFail =
      ~(WaitResult)0;  // Wait result when wait fails

 public:
  CEvents(void);
  ~CEvents();

  // Return a set event
  bool GetEventSet(Event *peventRet);

  // Set an event
  void Set(Event event);

  // Reset an event
  void Reset(Event event);

  // Block until the event is set
  WaitResult Wait(Event event);

  // Block until at least one event is set
  WaitResult WaitForMultiple(size_t cevent, Event *rgevent);

 protected:
  std::condition_variable
      conditionvariable_;  // Condition variable to signal when an event is set
  std::mutex mutex_;       // Mutex protected conditionvariable_
  std::unordered_set<Event> useventSet_;  // Events that are set
};                                        // struct CEvents

}  // namespace LVMon

#endif  // TOOLS_LVMON_LIB_SRC_EVENTS_H_
