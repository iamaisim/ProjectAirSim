// Copyright (C) Microsoft Corporation.
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include "MavLinkConnection.hpp"

using namespace mavlinkcom;

std::vector<SerialPortInfo> MavLinkConnection::findSerialPorts(int vid,
                                                               int pid) {
  (void)vid;
  (void)pid;
  return {};
}
