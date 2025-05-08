// Copyright (C) Microsoft Corporation. All rights reserved.

#include "thread.h"

namespace LVMon {

CThread::CThread(void) : events_(), fShouldStop_(false), pthread_(nullptr) {}

CThread::~CThread() {
  StopAsync();
  WaitForStop();
}

void CThread::Start(std::function<void(CThread *)> fnThreadProc) {
  fShouldStop_ = false;
  events_.Reset(kEventExitThread);
  pthread_ = new std::thread(ThreadProcProxy, this, fnThreadProc);
}

void CThread::StopAsync(void) {
  fShouldStop_ = true;
  events_.Set(kEventExitThread);
}

void CThread::ThreadProcProxy(CThread *pthread,
                              std::function<void(CThread *)> fnThreadProc) {
  fnThreadProc(pthread);
}

void CThread::WaitForStop(void) {
  if (pthread_ != nullptr) {
    auto pthread = pthread_;

    pthread_ = nullptr;
    pthread->join();
    delete pthread;
  }
}

}  // namespace LVMon
