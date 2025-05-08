// Copyright (C) Microsoft Corporation. All rights reserved.

#include "events.h"

namespace LVMon {

CEvents::CEvents(void) : conditionvariable_(), mutex_(), useventSet_() {}

CEvents::~CEvents() {}

bool CEvents::GetEventSet(CEvents::Event *peventRet) {
  bool fGotEvent = false;
  std::unique_lock<std::mutex> ul(mutex_);

  if (!useventSet_.empty()) {
    auto it = useventSet_.begin();

    *peventRet = *it;
    useventSet_.erase(it);
    ul.unlock();

    fGotEvent = true;
  }

  return (fGotEvent);
}

void CEvents::Set(Event event) {
  std::unique_lock<std::mutex> ul(mutex_);

  useventSet_.insert(event);
  conditionvariable_.notify_one();
}

void CEvents::Reset(Event event) {
  std::unique_lock<std::mutex> ul(mutex_);

  useventSet_.erase(event);
}

CEvents::WaitResult CEvents::Wait(Event event) {
  WaitResult waitresultRet = kWaitResultFail;
  std::unique_lock<std::mutex> ul(mutex_);

  conditionvariable_.wait(ul, [&]() {
    auto it = useventSet_.find(event);

    if (it == useventSet_.end())
      return (false);
    else {
      useventSet_.erase(event);
      waitresultRet = kWaitResultEvent0;
      return (true);
    }
  });

  return (waitresultRet);
}

CEvents::WaitResult CEvents::WaitForMultiple(size_t cevent, Event *rgevent) {
  WaitResult waitresultRet = kWaitResultFail;
  std::unique_lock<std::mutex> ul(mutex_);

  conditionvariable_.wait(ul, [&]() {
    for (auto *pevent = rgevent, *peventMax = rgevent + cevent;
         pevent < peventMax; ++pevent) {
      auto it = useventSet_.find(*pevent);

      if (it != useventSet_.end()) {
        useventSet_.erase(it);
        waitresultRet = kWaitResultEvent0 + (pevent - rgevent);
        return (true);
      }
    }

    return (false);
  });

  return (waitresultRet);
}

}  // namespace LVMon
