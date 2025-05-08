// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef TOOLS_LVMON_LIB_SRC_SERVERCONNECTION_I_H_
#define TOOLS_LVMON_LIB_SRC_SERVERCONNECTION_I_H_

#include <functional>

namespace LVMon {

class IServerConnection {
 public:
  // Unique ID for each client
  typedef unsigned int ClientID;

  // This event is triggered on a client connection change
  struct ConnectionEventArgs {
    enum class Reason {
      Connect,     // A new client has connected
      Disconnect,  // A client has disconnected
    };             // enum class Reason

    ClientID clientid;  // ID of client
    Reason reason;      // Reason for the event

    ConnectionEventArgs(Reason reasonIn, ClientID clientid)
        : clientid(clientid), reason(reasonIn) {}
  };  // struct ConnectionEventArgs

  typedef std::function<void(IServerConnection *piserverconnection,
                             const ConnectionEventArgs &args)>
      FnOnConnection;

  // This event is triggered when data is received from a client
  struct ReadEventArgs {
    ClientID clientid;  // ID of client from which this data was received
    uint8_t *pb;        // Data buffer
    size_t cb;          // Number of bytes in pb

    ReadEventArgs(ClientID clientidIn, uint8_t *pbIn, size_t cbIn)
        : clientid(clientidIn), pb(pbIn), cb(cbIn) {}
  };  // struct ReadEventArgs

  typedef std::function<void(IServerConnection *piserverconnection,
                             ReadEventArgs &args)>
      FnOnRead;

  // This event is triggered when a client's write buffer is empty
  struct WriteReadyEventArgs {
    ClientID clientid;  // ID of client whose write buffer is empty

    WriteReadyEventArgs(ClientID clientidIn) : clientid(clientidIn) {}
  };  // struct WriteReadyEventArgs

  typedef std::function<void(IServerConnection *piserverconnection,
                             const WriteReadyEventArgs &args)>
      FnOnWriteReady;

 public:
  virtual ~IServerConnection() {}

  virtual void Send(ClientID clientid, const uint8_t *pb, size_t cb) = 0;
  virtual void SetCallbacks(FnOnConnection fnOnConnection, FnOnRead fnOnRead,
                            FnOnWriteReady fnOnWriteReady) = 0;
  virtual void Start(void) = 0;
  virtual void Stop(void) = 0;
};  // class IServerConnection

}  // namespace LVMon

#endif  // TOOLS_LVMON_LIB_SRC_SERVERCONNECTION_I_H_
