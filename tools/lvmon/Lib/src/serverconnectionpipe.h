// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef TOOLS_LVMON_LIB_SRC_PIPECONNECTIONSERVER_H_
#define TOOLS_LVMON_LIB_SRC_PIPECONNECTIONSERVER_H_

#include <Windows.h>
#include <stdint.h>

#include <functional>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>

#include "Thread.h"
#include "serverconnection_i.h"

namespace LVMon {

class CServerConnectionPipe : public IServerConnection {
 public:
  CServerConnectionPipe(void);
  virtual ~CServerConnectionPipe() override;

  virtual void Send(ClientID clientid, const uint8_t *pb, size_t cb) override;
  virtual void SetCallbacks(FnOnConnection fnOnConnection, FnOnRead fnOnRead,
                            FnOnWriteReady fnOnWriteReady) override;
  virtual void Start(void) override;
  virtual void Stop(void) override;

 protected:
  typedef std::vector<uint8_t> VecB;

  struct ClientEntry {
    ClientID clientid;      // ID of this entry
    bool fWriteInProgress;  // If true, a write is in progress
    HANDLE hPipe;           // Pipe to and from client
    std::recursive_mutex
        mutexWrite;  // Access guard to fWriteInProgress and queuevecbWrite
    OVERLAPPED overlappedRead;   // Overlapped I/O state for reading from pipe
    OVERLAPPED overlappedWrite;  // Overlapped I/O state for writing to pipe
    CServerConnectionPipe *pserverconnectionpipe;  // Pointer to parent
    std::queue<VecB> queuevecbWrite;  // Queue of data to write to client
    uint8_t rgbBufferRead[257];       // Read buffer
    VecB vecbWrite;                   // Write buffer

   public:
    ClientEntry(ClientID clientidIn,
                CServerConnectionPipe *pserverconnectionpipeIn, HANDLE hPipeIn);

    void AddRef(void);
    void Release(void);

   protected:
    unsigned int m_cref;  // Reference count

   private:
    ~ClientEntry();
  };  // struct ClientEntry
  friend struct ClientEntry;

  typedef std::unordered_map<ClientID, ClientEntry *>
      UMClientIDPClientEntry;  // Mapping from client ID to entry

 protected:
  static void WINAPI ClientCompletedReadCB(DWORD dwErr, DWORD cbRead,
                                           LPOVERLAPPED poverlapped);
  static void WINAPI ClientCompletedWriteCB(DWORD dwErr, DWORD cbWritten,
                                            LPOVERLAPPED poverlapped);

 protected:
  void ClientThreadProc(CThread *pthread);
  void RemoveClient(ClientEntry *pcliententry);
  bool ConnectClient(OVERLAPPED &overlapped);

 protected:
  ClientID clientidNext_;          // Next available client ID
  FnOnConnection fnOnConnection_;  // Connection callback
  FnOnRead fnOnRead_;              // Read callback
  FnOnWriteReady fnOnWriteReady_;  // Write ready callback
  bool fThreadClientIsRunning_;    // True if threadClients_ is running
  HANDLE hEventExitThread_;    // Event to set to request threadClients_ exit
  HANDLE hEventWritePending_;  // Event to set a write buffer has been queued
  HANDLE hPipeClientNew_;      // Pipe to the next new client
  std::recursive_mutex mutexClients_;  // Access guard to m_listcliententry
  CThread threadClients_;              // Thread for handling client connection
  UMClientIDPClientEntry
      umclientidpcliententry_;  // Mapping from client ID to entry for
                                // currently connected clients
};                              // class CServerConnectionPipe

}  // namespace LVMon

#endif  // TOOLS_LVMON_LIB_SRC_PIPECONNECTIONSERVER_H_
