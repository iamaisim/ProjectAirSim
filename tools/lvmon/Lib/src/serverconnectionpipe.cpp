// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "serverconnectionpipe.h"

#include <assert.h>

namespace {

void DebugPrint(const char* szFmt, ...) {
  char sz[2048];
  va_list vl;

  va_start(vl, szFmt);
  vsprintf_s(sz, szFmt, vl);
  va_end(vl);
  OutputDebugStringA(sz);
}

}  // namespace

namespace LVMon {

CServerConnectionPipe::CServerConnectionPipe(void)
    : clientidNext_(0),
      fnOnConnection_(),
      fnOnRead_(),
      fnOnWriteReady_(),
      fThreadClientIsRunning_(false),
      hEventExitThread_(CreateEvent(NULL, TRUE, FALSE, NULL)),
      hEventWritePending_(CreateEvent(NULL, FALSE, FALSE, NULL)),
      hPipeClientNew_(INVALID_HANDLE_VALUE),
      mutexClients_(),
      threadClients_(),
      umclientidpcliententry_() {}

CServerConnectionPipe::~CServerConnectionPipe() {
  Stop();
  if (hEventExitThread_ != NULL) CloseHandle(hEventExitThread_);
  if (hEventWritePending_ != NULL) CloseHandle(hEventWritePending_);
}

void CServerConnectionPipe::ClientThreadProc(CThread* pthread) {
  bool fClientIOIsPending = false;
  bool fExit = false;
  HANDLE rgh[] = {hEventExitThread_, nullptr, hEventWritePending_};
  OVERLAPPED overlappedConnect;

  // DebugPrint("LVMon::CServerConnectionPipe: ClientThreadProc start.\n");

  // Asynchronously connect to first client
  overlappedConnect.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
  if (overlappedConnect.hEvent == NULL) {
    DebugPrint(
        "LVMon::CServerConnectionPipe::ClientThreadProc(): failed to create "
        "overlappedConnect.hEvent (error %d)\n",
        GetLastError());
    goto LError;
  }
  rgh[1] = overlappedConnect.hEvent;
  fClientIOIsPending = ConnectClient(overlappedConnect);

  while (!fExit) {
    DWORD dwWait =
        WaitForMultipleObjectsEx(_countof(rgh), rgh, FALSE, INFINITE, TRUE);

    switch (dwWait) {
      default:
        printf(
            "LVMon::CServerConnectionPipe::ClientThreadProc(): "
            "WaitForMultipleObjectsEx() returned unexpected value 0x%08x\n",
            dwWait);
        DebugPrint(
            "LVMon::CServerConnectionPipe::ClientThreadProc(): "
            "WaitForMultipleObjectsEx() returned unexpected value 0x%08x\n",
            dwWait);
        assert(false);
        break;

      case WAIT_IO_COMPLETION:
        // No action needed
        // DebugPrint("LVMon::CServerConnectionPipe::ClientThreadProc():
        // WAIT_IO_COMPLETION\n");
        break;

      case WAIT_OBJECT_0:
        fExit = true;
        break;

      case WAIT_OBJECT_0 + 1:
        // A new client has connected--if itValueEntry was an overlapped
        // notification, complete the overlapped I/O
        if (fClientIOIsPending) {
          DWORD cbFromClient = 0;

          if (!GetOverlappedResult(hPipeClientNew_, &overlappedConnect,
                                   &cbFromClient, FALSE)) {
            DebugPrint(
                "LVMonServer::ClientThreadProc(): failed to create "
                "overlappedConnect.hEvent (error %d)\n",
                GetLastError());
            assert(false);
            goto LError;
          }
        }

        {
          // Create client entry
          std::unique_lock<std::recursive_mutex> ul(mutexClients_);
          ClientEntry* pcliententry;
          std::pair<UMClientIDPClientEntry::iterator, bool> pair;

          pair = umclientidpcliententry_.emplace(
              clientidNext_,
              new ClientEntry(clientidNext_, this, hPipeClientNew_));
          ++clientidNext_;
          pcliententry = pair.first->second;

          hPipeClientNew_ = INVALID_HANDLE_VALUE;

          // DebugPrint("LVMon::CServerConnectionPipe::ClientThreadProc(): New
          // client %d has connected\n", umclientidpcliententry_.size());

          // Report new client
          fnOnConnection_(
              this, ConnectionEventArgs(ConnectionEventArgs::Reason::Connect,
                                        pcliententry->clientid));

          // Start a read for this client
          pcliententry->AddRef();  // AddRef for ClientCompletedReadCB()
          ClientCompletedReadCB(0, 0, &pcliententry->overlappedRead);
        }

        // Asynchronously connect to next client
        fClientIOIsPending = ConnectClient(overlappedConnect);
        break;

      case WAIT_OBJECT_0 + 2:
        // Scan for pending writes
        {
          std::unique_lock<std::recursive_mutex> ul(mutexClients_);

          for (auto pair : umclientidpcliententry_) {
            auto pcliententry = pair.second;

            if (!pcliententry->fWriteInProgress &&
                !pcliententry->queuevecbWrite.empty()) {
              pcliententry->AddRef();  // AddRef for ClientCompeltedWriteCB()
              ClientCompletedWriteCB(0, 0, &pcliententry->overlappedWrite);
            }
          }
        }
        break;
    }
  }

LError:
    // DebugPrint("LVMon::CServerConnectionPipe: ClientThreadProc exiting.\n");
    ;
}

void CServerConnectionPipe::ClientCompletedReadCB(DWORD /*dwErr*/, DWORD cbRead,
                                                  LPOVERLAPPED poverlapped) {
  ClientEntry* pcliententry =
      reinterpret_cast<ClientEntry*>(reinterpret_cast<uint8_t*>(poverlapped) -
                                     offsetof(ClientEntry, overlappedRead));

  // Process the data from the client
  if (cbRead > 0) {
    ReadEventArgs args(pcliententry->clientid, pcliententry->rgbBufferRead,
                       cbRead);
    pcliententry->pserverconnectionpipe->fnOnRead_(
        pcliententry->pserverconnectionpipe, args);
  }

  // Start next read
  pcliententry->AddRef();  // AddRef next read
  if (!ReadFileEx(pcliententry->hPipe, pcliententry->rgbBufferRead,
                  sizeof(pcliententry->rgbBufferRead), poverlapped,
                  (LPOVERLAPPED_COMPLETION_ROUTINE)
                      CServerConnectionPipe::ClientCompletedReadCB)) {
    DWORD err = GetLastError();

#ifdef NOTDEF
    if ((err == ERROR_BROKEN_PIPE) || (err == ERROR_NO_DATA))
      DebugPrint(
          "CServerConnectionPipe::ClientCompletedReadCB(): client closed "
          "connection\n");
    else
      DebugPrint(
          "CServerConnectionPipe::ClientCompletedReadCB(): ReadFileEx() "
          "returned error %d\n",
          err);
#endif  // NOTDEF
    pcliententry->pserverconnectionpipe->RemoveClient(pcliententry);
    pcliententry->Release();  // Release for new read didn't happen
  }

  pcliententry->Release();  // Release this completed read
}

void CServerConnectionPipe::ClientCompletedWriteCB(DWORD dwErr,
                                                   DWORD /*cbWrite*/,
                                                   LPOVERLAPPED poverlapped) {
  if (dwErr != 0)
    DebugPrint("CServerConnectionPipe::ClientCompletedWriteCB(): err %d\n",
               dwErr);

  ClientEntry* pcliententry =
      reinterpret_cast<ClientEntry*>(reinterpret_cast<uint8_t*>(poverlapped) -
                                     offsetof(ClientEntry, overlappedWrite));
  std::unique_lock<std::recursive_mutex> ul(pcliententry->mutexWrite);

  // Start next write, if any
  if (pcliententry->queuevecbWrite.empty()) {
    pcliententry->vecbWrite.clear();
    pcliententry->fWriteInProgress = false;
    ResetEvent(poverlapped->hEvent);

    // DebugPrint("CServerConnectionPipe::ClientCompletedWriteCB: queue
    // empty\n");
    pcliententry->pserverconnectionpipe->fnOnWriteReady_(
        pcliententry->pserverconnectionpipe,
        WriteReadyEventArgs(
            pcliententry->clientid));  // No more data, tell callback
  } else {
    pcliententry->fWriteInProgress = true;
    pcliententry->vecbWrite.swap(pcliententry->queuevecbWrite.front());
    pcliententry->queuevecbWrite.pop();
    ul.unlock();

    // DebugPrint("Writing %d bytes to client %p\n",
    // pcliententry->vecbWrite.size(), pcliententry);

    pcliententry->AddRef();  // AddRef() for new write
    if (!WriteFileEx(pcliententry->hPipe, pcliententry->vecbWrite.data(),
                     pcliententry->vecbWrite.size(), poverlapped,
                     (LPOVERLAPPED_COMPLETION_ROUTINE)
                         CServerConnectionPipe::ClientCompletedWriteCB)) {
      DWORD err = GetLastError();

#ifdef NOTDEF
      if ((err == ERROR_BROKEN_PIPE) || (err == ERROR_NO_DATA))
        DebugPrint(
            "CServerConnectionPipe::ClientCompletedWriteCB(): client closed "
            "connection\n");
      else
        DebugPrint(
            "CServerConnectionPipe::ClientCompletedWriteCB(): WriteFileEx() "
            "returned error %d\n",
            err);
#endif  // NOTDEF
      pcliententry->pserverconnectionpipe->RemoveClient(pcliententry);
      pcliententry->Release();  // Release since New write didn't happen
    }
  }

  pcliententry->Release();  // Release this completed write
}

void CServerConnectionPipe::RemoveClient(ClientEntry* pcliententry) {
  // DebugPrint("CServerConnectionPipe::RemoveClient():  Removing client %p\n",
  // pcliententry);

  std::unique_lock<std::recursive_mutex> ul(mutexClients_);

  for (auto it = umclientidpcliententry_.begin(),
            itEnd = umclientidpcliententry_.end();
       it != itEnd; ++it) {
    if (it->second == pcliententry) {
      auto clientid = it->first;

      // Delete client entry
      umclientidpcliententry_.erase(it);
      pcliententry->Release();

      // Report client disconnect
      fnOnConnection_(
          this, ConnectionEventArgs(ConnectionEventArgs::Reason::Disconnect,
                                    clientid));
      break;
    }
  }
}

bool CServerConnectionPipe::ConnectClient(OVERLAPPED& overlapped) {
  bool fIOIsPendingRet = false;
  const wchar_t* wzPipename = L"\\\\.\\pipe\\lvmonserver";

  hPipeClientNew_ =
      CreateNamedPipeW(wzPipename,                  // pipe name
                       PIPE_ACCESS_DUPLEX |         // read/write access
                           FILE_FLAG_OVERLAPPED,    // overlapped mode
                       PIPE_TYPE_MESSAGE |          // message-type pipe
                           PIPE_READMODE_MESSAGE |  // message read mode
                           PIPE_WAIT,               // blocking mode
                       PIPE_UNLIMITED_INSTANCES,    // unlimited instances
                       256,                         // output buffer size
                       sizeof(ClientEntry::rgbBufferRead),  // input buffer size
                       5000,                                // client time-out
                       NULL);  // default security attributes

  if (hPipeClientNew_ == INVALID_HANDLE_VALUE) {
    DebugPrint(
        "CServerConnectionPipe::ConnectClient(): CreateNamedPipe failed with "
        "%d.\n",
        GetLastError());
  }
  // Start an overlapped connection for this pipe instance.
  // Overlapped ConnectNamedPipe should return zero.
  else if (ConnectNamedPipe(hPipeClientNew_, &overlapped))
    DebugPrint("ConnectNamedPipe failed with %d.\n", GetLastError());
  else {
    switch (GetLastError()) {
      default:  // An error occurs during the connect operation...
        DebugPrint("ConnectNamedPipe failed with %d.\n", GetLastError());
        break;

      case ERROR_IO_PENDING:  // The overlapped connection is in progress
        fIOIsPendingRet = TRUE;
        break;

      case ERROR_PIPE_CONNECTED:  // Client is already connected, so signal the
                                  // event
        if (SetEvent(overlapped.hEvent)) break;
    }
  }

  return (fIOIsPendingRet);
}

void CServerConnectionPipe::Send(ClientID clientid, const uint8_t* pb,
                                 size_t cb) {
  // DebugPrint("Queuing %d bytes to client %u\n", cb, clientid);

  auto pair = umclientidpcliententry_.find(clientid);

  if (pair != umclientidpcliententry_.end()) {
    auto pcliententry = pair->second;
    auto& queuevecbWrite = pcliententry->queuevecbWrite;

    {
      std::unique_lock<std::recursive_mutex> ul(pcliententry->mutexWrite);

      queuevecbWrite.emplace(pb, pb + cb);
    }

    SetEvent(hEventWritePending_);
  }
}

void CServerConnectionPipe::SetCallbacks(FnOnConnection fnOnConnection,
                                         FnOnRead fnOnRead,
                                         FnOnWriteReady fnOnWriteReady) {
  fnOnConnection_ = fnOnConnection;
  fnOnRead_ = fnOnRead;
  fnOnWriteReady_ = fnOnWriteReady;
}

void CServerConnectionPipe::Start(void) {
  if (!fThreadClientIsRunning_) {
    ResetEvent(hEventExitThread_);
    threadClients_.Start(std::bind(&CServerConnectionPipe::ClientThreadProc,
                                   this, std::placeholders::_1));
    fThreadClientIsRunning_ = true;
  }
}

void CServerConnectionPipe::Stop(void) {
  SetEvent(hEventExitThread_);
  threadClients_.StopAsync();
  threadClients_.WaitForStop();
  fThreadClientIsRunning_ = false;
}

CServerConnectionPipe::ClientEntry::ClientEntry(
    ClientID clientidIn, CServerConnectionPipe* pserverconnectionpipeIn,
    HANDLE hPipeIn)
    : clientid(clientidIn),
      fWriteInProgress(false),
      hPipe(hPipeIn),
      mutexWrite(),
      overlappedRead{0},
      overlappedWrite{0},
      pserverconnectionpipe(pserverconnectionpipeIn),
      queuevecbWrite(),
      rgbBufferRead(),
      vecbWrite(),
      m_cref(1) {
  // Initialize superclass data
  overlappedRead.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
  overlappedWrite.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
}

CServerConnectionPipe::ClientEntry::~ClientEntry() {
  if (overlappedRead.hEvent != INVALID_HANDLE_VALUE)
    CloseHandle(overlappedRead.hEvent);
  if (overlappedWrite.hEvent != INVALID_HANDLE_VALUE)
    CloseHandle(overlappedWrite.hEvent);
  if (hPipe != INVALID_HANDLE_VALUE) CloseHandle(hPipe);
}

void CServerConnectionPipe::ClientEntry::AddRef(void) {
  InterlockedIncrement(&m_cref);
}

void CServerConnectionPipe::ClientEntry::Release(void) {
  if (InterlockedDecrement(&m_cref) == 0) delete this;
}

}  // namespace LVMon
