// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MavLinkCom_MavLinkTcpServer_hpp
#define MavLinkCom_MavLinkTcpServer_hpp

#include <memory>
#include <string>

#include "MavLinkConnection.hpp"

namespace mavlinkcom_impl {
class MavLinkTcpServerImpl;
}

namespace mavlinkcom {

class MavLinkTcpServer {
 public:
  MavLinkTcpServer(const std::string& local_addr, int local_port);
  ~MavLinkTcpServer();

  // This method accepts a new connection from a remote machine and gives that
  // connection the given name. This is how you can build a TCP server by
  // calling this method in a loop as long as you want to continue receiving new
  // incoming connections.
  std::shared_ptr<MavLinkConnection> acceptTcp(const std::string& nodeName);

 public:
  // needed for piml pattern
  MavLinkTcpServer();
  // MavLinkTcpServer(MavLinkTcpServer&&);
  // MavLinkTcpServer& operator=(MavLinkTcpServer&&);
 private:
  std::shared_ptr<mavlinkcom_impl::MavLinkTcpServerImpl> impl_;
};
}  // namespace mavlinkcom

#endif
