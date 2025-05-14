// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef TOOLS_LVMON_LIB_SRC_TCPCONNECTIONSERVER_H_
#define TOOLS_LVMON_LIB_SRC_TCPCONNECTIONSERVER_H_

#include <stdint.h>

#include <functional>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>

#include "events.h"
#include "serverconnection_i.h"
#include "thread.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#endif  //_WIN32

#ifdef __linux__
#include <sys/signalfd.h>
#endif  //__linux__

namespace LVMon {

class CServerConnectionTCP : public IServerConnection {
 public:
  CServerConnectionTCP(void);
  virtual ~CServerConnectionTCP() override;

  virtual void Send(ClientID clientid, const uint8_t* pb, size_t cb) override;
  virtual void SetCallbacks(FnOnConnection fnOnConnection, FnOnRead fnOnRead,
                            FnOnWriteReady fnOnWriteReady) override;
  virtual void Start(void) override;
  virtual void Stop(void) override;

 protected:
#ifdef __linux__
  typedef int SOCKET;
#endif  //__linux__

  typedef int Err;  // Error code

  typedef std::vector<uint8_t> VecB;      // Vector of bytes
  typedef std::vector<SOCKET> VecSocket;  // Vector of sockets

  // Flags indicating events on a socket
  enum class SocketEvents {
    Close,  // Socket was closed
    Read,   // Socket is ready to read
    Write,  // Socket is ready to write
  };        // enum class SocketEvents

  typedef std::unordered_set<SocketEvents> USSocketEvents;

  // Each client is associated with one of the following
  struct ClientEntry {
    ClientID clientid;      // ID of this entry
    bool fWriteInProgress;  // If true, a write is in progress and we want the
                            // "write ready" network event
    uint32_t msLastWrite;  // The timestamp of when the last message was sent to
                           // this client (milliseconds)
    std::recursive_mutex
        mutexWrite;  // Access guard to fWriteInProgress and queuevecbWrite
    std::queue<VecB> queuevecbWrite;  // Queue of data to write to client
    uint8_t rgbBufferRead[257];       // Read buffer
    SOCKET socket;                    // Socket to and from client
    VecB vecbWrite;                   // Write buffer
    USSocketEvents ussocketevents;    // Socket events, set by WaitForEvent()

#ifdef _WIN32
    WSAEVENT wsaevent;  // Event that's set when the socket has data to read or
                        // is ready to write
#endif                  //_WIN32

   public:
    ClientEntry(ClientID clientidIn);

    void AddRef(void);
    void Release(void);

    void CloseSocket(void);
    Err SetSocket(SOCKET socketIn);
    bool Write(const void* pv, size_t cb);

   protected:
    unsigned int m_cref;  // Reference count

   private:
    ~ClientEntry();
  };  // struct ClientEntry
  friend struct ClientEntry;

  typedef std::unordered_map<ClientID, ClientEntry*>
      UMClientIDPClientEntry;  // Mapping from client ID to entry

  // Our version of the thread class adding an async event mechanism compatible
  // with select()
  class CThreadTCS : public CThread {
   public:
    typedef CEvents::Event Event;

   public:
    CThreadTCS(void);
    virtual ~CThreadTCS() override;

    void SendEvent(Event event);

#ifdef _WIN32
    HANDLE GetHEvent(void) const { return (hEventEvent_); }
#endif  //_WIN32
#ifdef __linux__
    int GetFDEvent(void) const { return (fdEventReceive_); }
#endif  //__linux__

   public:
    virtual void StopAsync(void) override;

   protected:
#ifdef _WIN32
    HANDLE hEventEvent_;  // Event has been queued to m_queueevent
#endif                    //_WIN32
#ifdef __linux__
    int fdEventReceive_;  // File descriptor for receiving event signals
    int fdEventSend_;     // File descriptor for sending event signals
#endif                    //__linux__
  };                      // class CThreadTCS

  typedef unsigned int WaitResult;  // Return values from WaitForEvent()

 protected:
  static const CThreadTCS::Event kEventWritePending =
      0;  // Writes to one or more clients are pending
  static const unsigned int kMSTimeoutInfinite =
      ~(unsigned int)0;  // Timeout value for no timeout
  static const WaitResult kWaitResultFail = ~(WaitResult)0;  // Wait failed
  static const WaitResult kWaitResultEvent0 = 0;  // First event value
  static const WaitResult kWaitResultExitThread =
      kWaitResultEvent0 +
      CThreadTCS::kEventExitThread;  // Thread requested to exit
  static const WaitResult kWaitResultWritePending =
      kWaitResultEvent0 +
      kEventWritePending;  // One or more clients have new write buffers queued
  static const WaitResult kWaitResultSocket =
      0x10000;  // Events have occurred on listening socket and/or client
                // sockets--check ussocketeventsListen_ and
                // ClientEntry.m_ussocketevents
  static const WaitResult kWaitResultTimeout =
      0x10001;  // Wait timed out--no events before specified time elapsed

#ifdef _WIN32
  static const SOCKET kSocketInvalid = INVALID_SOCKET;
  static const int kErrWouldBlock = WSAEWOULDBLOCK;
  static const int kErrConnReset = WSAECONNRESET;
#endif  //_WIN32

#ifdef __linux__
  static const SOCKET kSocketInvalid = -1;  // Invalid socket value
  static const int NO_ERROR = 0;            // No error code
  static const int kErrWouldBlock =
      EWOULDBLOCK;  // errno code for "socket is non-blocking and call can't
                    // proceed"
  static const int kErrConnReset =
      ECONNRESET;  // errno code for "connect has been reset"
#endif             //__linux__

 protected:
  static void CloseSocket(SOCKET* psocket);
  static Err GetErr(void);
  static uint32_t GetMSTimestamp(void);
  static int SetSocketNonBlocking(SOCKET socket, bool fNonBlocking);

 protected:
  void ClientThreadProc(CThread* pthread);
  Err CreateListenSocket(void);
  bool DoClientRead(ClientEntry* pcliententry);
  bool DoClientWrite(ClientEntry* pcliententry);
  void HandleClientConnect(void);
  void RemoveAllClients(void);
  void RemoveClient(ClientEntry* pcliententry);
  WaitResult WaitForEvent(CThreadTCS& threadtcs, unsigned int msTimeout);

 protected:
  ClientID clientidNext_;              // Next available client ID
  FnOnConnection fnOnConnection_;      // Connection callback
  FnOnRead fnOnRead_;                  // Read callback
  FnOnWriteReady fnOnWriteReady_;      // Write ready callback
  bool fThreadClientIsRunning_;        // True if threadClients_ is running
  std::recursive_mutex mutexClients_;  // Access guard to m_listcliententry
  int portListen_;                     // TCP port we listen on
  SOCKET socketListen_;       // Our listening socket clients use to initiate a
                              // connection
  CThreadTCS threadClients_;  // Thread for handling client connections
  UMClientIDPClientEntry
      umclientidpcliententry_;  // Mapping from client ID to entry for
                                // currently connected clients
  std::unordered_set<ClientEntry*>
      uspcliententryToRemove_;           // Clients to remove
  USSocketEvents ussocketeventsListen_;  // Socket events for socketListen_

#ifdef _WIN32
  WSAEVENT wsaeventSocketListen_;  // Event that's set when a client connects
                                   // to the connect socket
#endif                             //_WIN32
};                                 // class CServerConnectionTCP

}  // namespace LVMon

#endif  // TOOLS_LVMON_LIB_SRC_TCPCONNECTIONSERVER_H_
