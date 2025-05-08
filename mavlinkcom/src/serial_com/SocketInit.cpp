// Copyright (c) Microsoft Corporation. All rights reserved.

#include "SocketInit.hpp"

#include <stdexcept>

#include "Utils.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace mavlink_utils;
bool SocketInit::socket_initialized_ = false;

SocketInit::SocketInit() {
  if (!socket_initialized_) {
    socket_initialized_ = true;
#ifdef _WIN32
    WSADATA wsaData;
    // Initialize Winsock
    int rc = WSAStartup(MAKEWORD(2, 2), (LPWSADATA)&wsaData);
    if (rc != 0) {
      throw std::runtime_error(
          Utils::stringf("WSAStartup failed with error : %d\n", rc));
    }
#endif
  }
}