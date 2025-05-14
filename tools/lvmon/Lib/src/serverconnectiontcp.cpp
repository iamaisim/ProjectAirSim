// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "serverconnectiontcp.h"

#include <LVMon/lvmonprotocol.h>
#include <assert.h>

#ifdef _WIN32
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")  // Link with WinSock2 library
#endif                              //_WIN32

#ifdef __linux__
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#endif  //__linux__

namespace {

void DebugPrint(const char* szFmt, ...) {
  char sz[2048];

#ifdef _WIN32
  va_list vl;

  va_start(vl, szFmt);
  vsprintf_s(sz, szFmt, vl);
  va_end(vl);
  OutputDebugStringA(sz);
  OutputDebugStringA("\n");
#else   //_WIN32
  va_list vl;

  va_start(vl, szFmt);
  vsprintf(sz, szFmt, vl);
  va_end(vl);
  puts(sz);
#endif  //_WIN32
}

}  // namespace

namespace LVMon {

CServerConnectionTCP::CServerConnectionTCP(void)
    : clientidNext_(0),
      fnOnConnection_(),
      fnOnRead_(),
      fnOnWriteReady_(),
      fThreadClientIsRunning_(false),
      mutexClients_(),
      portListen_(LVMon::kPortDefault),
      socketListen_(kSocketInvalid),
      threadClients_(),
      umclientidpcliententry_(),
      uspcliententryToRemove_(),
      ussocketeventsListen_()
#ifdef _WIN32
      ,
      wsaeventSocketListen_(WSACreateEvent())
#endif  //_WIN32
{
}

CServerConnectionTCP::~CServerConnectionTCP() {
  Stop();

#ifdef _WIN32
  WSACloseEvent(wsaeventSocketListen_);
#endif  //_WIN32
}

void CServerConnectionTCP::ClientThreadProc(CThread* pthread) {
  bool fClientIOIsPending = false;
  bool fExit = false;

  // DebugPrint("LVMon::CServerConnectionTCP: ClientThreadProc start.");

#ifdef _WIN32
  bool fWinsockIsStarted = false;

  // Start Winsock
  {
    int err;
    WSAData wsadata;

    if ((err = WSAStartup(MAKEWORD(2, 2), &wsadata)) != 0) {
      DebugPrint(
          "LVMon::CserverConnectionTCP::ClientThreadProc(): WSAStartup failed "
          "with error %d",
          err);
      goto LError;
    }

    fWinsockIsStarted = true;
  }
#endif  //_WIN32

  // Create connect socketIn
  {
    Err err;

    if ((err = CreateListenSocket()) != 0) {
      DebugPrint(
          "LVMon::CServerConnectionTCP::ClientThreadProc(): Failed to create "
          "listening socketIn: error %d",
          err);
      goto LError;
    }
  }

  // Bind to our desired listening port
  {
    sockaddr_in sa = {0};

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(portListen_);

    if (bind(socketListen_, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) !=
        0) {
      DebugPrint(
          "LVMon::CServerConnectionTCP::ClientThreadProc(): Failed to bind "
          "socketIn: error %d",
          GetErr());
      goto LError;
    }
  }

  // Start listening for clients
  if (listen(socketListen_, 10) != 0) {
    DebugPrint(
        "LVMon::CServerConnectionTCP::ClientThreadProc(): Failed to listen on "
        "socketIn: error %d",
        GetErr());
    goto LError;
  }

  while (!fExit) {
    static const unsigned int kMSClientCheckTimeout = 2000;

    WaitResult waitresult =
        WaitForEvent(*static_cast<CThreadTCS*>(pthread),
                     umclientidpcliententry_.empty() ? kMSTimeoutInfinite
                                                     : kMSClientCheckTimeout);

    switch (waitresult) {
      default:
        DebugPrint(
            "LVMon::CServerConnectionTCP::ClientThreadProc(): WaitForEvent() "
            "returned unexpected value 0x%08x",
            waitresult);
        assert(false);
        break;

      case kWaitResultTimeout:
        // Probe client connections to see if they're still good since we may
        // not discover that the client has disconnected without attempting a
        // write
        {
          static const unsigned int kMSClientCheck = 500;

          uint32_t msCur = GetMSTimestamp();
          LVMon::MsgNull msgnull;

          msgnull.HToN();

          for (auto& pair : umclientidpcliententry_) {
            auto pcliententry = pair.second;

            if (((msCur - pcliententry->msLastWrite) > kMSClientCheck) &&
                !pcliententry->fWriteInProgress) {
              // DebugPrint("Pinging client %d", pcliententry->clientid);
              if (!pcliententry->Write(&msgnull, sizeof(msgnull)))
                uspcliententryToRemove_.insert(pcliententry);
            }
          }
        }
        break;

      case kWaitResultExitThread:
        fExit = true;
        break;

      case kWaitResultSocket:  // Got a network event on a socket
        if (ussocketeventsListen_.find(SocketEvents::Read) !=
            ussocketeventsListen_.end())
          HandleClientConnect();

        // Handle client events
        for (auto& pair : umclientidpcliententry_) {
          bool fNeedToRemoveClient = false;
          auto pcliententry = pair.second;

          // The connection to the client has closed
          if (pcliententry->ussocketevents.find(SocketEvents::Close) !=
              pcliententry->ussocketevents.end())
            fNeedToRemoveClient = true;
          else {
            if (pcliententry->ussocketevents.find(SocketEvents::Read) !=
                pcliententry->ussocketevents.end()) {
              if (!DoClientRead(pcliententry)) fNeedToRemoveClient = true;
            }
            if (!fNeedToRemoveClient &&
                (pcliententry->ussocketevents.find(SocketEvents::Write) !=
                 pcliententry->ussocketevents.end())) {
              if (!DoClientWrite(pcliententry)) fNeedToRemoveClient = true;
            }

            if (fNeedToRemoveClient)
              uspcliententryToRemove_.insert(pcliententry);
          }
        }
        break;

      case kWaitResultWritePending:
        // Scan for pending writes
        for (auto& pair : umclientidpcliententry_) {
          auto pcliententry = pair.second;

          DoClientWrite(pcliententry);
        }
        break;
    }

    // Remove closed clients
    for (auto pcliententry : uspcliententryToRemove_)
      RemoveClient(pcliententry);
    uspcliententryToRemove_.clear();
  }

LError:
  // Close listening socketIn, close all clients
  if (socketListen_ != kSocketInvalid) CloseSocket(&socketListen_);
  RemoveAllClients();

#ifdef _WIN32
  if (fWinsockIsStarted) WSACleanup();
#endif  //_WIN32

  DebugPrint("LVMon::CServerConnectionTCP: ClientThreadProc exiting.");
}

void CServerConnectionTCP::CloseSocket(SOCKET* psocket) {
  if (*psocket != kSocketInvalid) {
#ifdef _WIN32
    closesocket(*psocket);
#else   //_WIN32
    close(*psocket);
#endif  //_WIN32
    *psocket = kSocketInvalid;
  }
}

CServerConnectionTCP::Err CServerConnectionTCP::CreateListenSocket(void) {
  Err errRet = 0;

  CloseSocket(&socketListen_);

  if ((socketListen_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) !=
      kSocketInvalid) {
    errRet = SetSocketNonBlocking(socketListen_, true);

#ifdef _WIN32
    // Enable network event to trigger when a client connects to the listening
    // socket
    if (WSAEventSelect(socketListen_, wsaeventSocketListen_, FD_ACCEPT) != 0)
      errRet = WSAGetLastError();
#endif  //_WIN32
  }

  return (errRet);
}

bool CServerConnectionTCP::DoClientRead(ClientEntry* pcliententry) {
  // Read data from client's socketIn
  bool fOKRet = true;
  int cbRead = recv(pcliententry->socket,
                    reinterpret_cast<char*>(pcliententry->rgbBufferRead),
                    sizeof(pcliententry->rgbBufferRead), 0);

  if (cbRead == 0) {
    // DebugPrint("CServerConnectionTCP::DoClientRead(): client closed
    // connection");
    fOKRet = false;
  } else if (cbRead < 0) {
    int err = GetErr();

    if (err != kErrWouldBlock) {
#ifdef NOTDEF
      if (err == kErrConnReset)
        DebugPrint(
            "CServerConnectionTCP::DoClientRead(): client closed connection");
      else
        DebugPrint(
            "CServerConnectionTCP::DoClientRead(): ReadFileEx() returned error "
            "%d",
            err);
#endif  // NOTDEF
      uspcliententryToRemove_.insert(pcliententry);
    }
  } else {
    // Process the data from the client
    ReadEventArgs rea(pcliententry->clientid, pcliententry->rgbBufferRead,
                      cbRead);

    fnOnRead_(this, rea);
  }

  return (fOKRet);
}

bool CServerConnectionTCP::DoClientWrite(ClientEntry* pcliententry) {
  bool fOKRet = true;

  // Loop until we get a non-empty write buffer or the write queue is empty
  for (;;) {
    VecB vecb;

    // If client's write buffer is empty, get next buffer from the queue
    pcliententry->vecbWrite.swap(vecb);
    if (vecb.empty()) {
      std::lock_guard<std::recursive_mutex> lg(pcliententry->mutexWrite);

      if (pcliententry->queuevecbWrite.empty()) {
        // All done; nothing left to write
        vecb.clear();
        // DebugPrint("CServerConnectionTCP::DoClientWrite: queue empty\n");
      } else {
        pcliententry->queuevecbWrite.front().swap(vecb);
        pcliententry->queuevecbWrite.pop();
      }
    }

    // Start next write, if any
    if (vecb.empty()) {
      fnOnWriteReady_(
          this, WriteReadyEventArgs(
                    pcliententry->clientid));  // No more data, tell callback
      break;
    } else {
      int cbWritten;

      // DebugPrint("Writing %d bytes to client %p\n",
      // pcliententry->vecbWrite.size(), pcliententry);

      if (!pcliententry->Write(vecb.data(), vecb.size())) {
        uspcliententryToRemove_.insert(
            pcliententry);  // Write failed, mark client for removal
        break;
      }
    }
  }

  return (fOKRet);
}

CServerConnectionTCP::Err CServerConnectionTCP::GetErr(void) {
#ifdef _WIN32
  return (WSAGetLastError());
#else
  return (errno);
#endif  // _WIN32
}

uint32_t CServerConnectionTCP::GetMSTimestamp(void) {
#ifdef _WIN32

  return (GetTickCount());

#else  // _WIN32

  timespec ts;

  return ((timespec_get(&ts, TIME_UTC) == 0)
              ? 0
              : (ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000)));

#endif  // _WIN32
}

void CServerConnectionTCP::HandleClientConnect(void) {
  // A client has connected
  SOCKET socketClient = accept(socketListen_, nullptr, nullptr);

  if (socketClient != kSocketInvalid) {
    // Create client entry
    Err err;
    ClientEntry* pcliententry = new ClientEntry(clientidNext_);

    if ((err = pcliententry->SetSocket(socketClient)) != NO_ERROR) {
      CloseSocket(&socketClient);
      DebugPrint(
          "LVMonServer::ClientThreadProc(): Failed to set socket for new "
          "client: error %d",
          err);
    } else {
      std::pair<UMClientIDPClientEntry::iterator, bool> pair;

      pcliententry->AddRef();
      pair = umclientidpcliententry_.emplace(clientidNext_, pcliententry);
      ++clientidNext_;
      pcliententry = pair.first->second;

      // DebugPrint("LVMonServer::ClientThreadProc(): New client %d has
      // connected", umclientidpcliententry_.size());

      // Report new client
      fnOnConnection_(this,
                      ConnectionEventArgs(ConnectionEventArgs::Reason::Connect,
                                          pcliententry->clientid));
    }

    pcliententry->Release();
  }
}

void CServerConnectionTCP::RemoveAllClients(void) {
  for (auto it = umclientidpcliententry_.begin(),
            itEnd = umclientidpcliententry_.end();
       it != itEnd; ++it) {
    auto pcliententry = it->second;

    umclientidpcliententry_.erase(it);

    // Report client disconnect
    fnOnConnection_(this,
                    ConnectionEventArgs(ConnectionEventArgs::Reason::Disconnect,
                                        pcliententry->clientid));

    pcliententry->Release();
    break;
  }

  uspcliententryToRemove_.clear();
}

void CServerConnectionTCP::RemoveClient(ClientEntry* pcliententry) {
  // DebugPrint("CServerConnectionTCP::RemoveClient():  Removing client %p",
  // pcliententry);

  for (auto it = umclientidpcliententry_.begin(),
            itEnd = umclientidpcliententry_.end();
       it != itEnd; ++it) {
    if (it->second == pcliententry) {
      umclientidpcliententry_.erase(it);
      pcliententry->ussocketevents.clear();

      // Report client disconnect
      fnOnConnection_(
          this, ConnectionEventArgs(ConnectionEventArgs::Reason::Disconnect,
                                    pcliententry->clientid));

      pcliententry->Release();
      break;
    }
  }
}

void CServerConnectionTCP::Send(ClientID clientid, const uint8_t* pb,
                                size_t cb) {
  // DebugPrint("Queuing %d bytes to client %u\n", cb, clientid);

  auto pair = umclientidpcliententry_.find(clientid);

  if (pair != umclientidpcliententry_.end()) {
    auto pcliententry = pair->second;
    auto& queuevecbWrite = pcliententry->queuevecbWrite;

    {
      std::lock_guard<std::recursive_mutex> lg(pcliententry->mutexWrite);

      queuevecbWrite.emplace(pb, pb + cb);
    }

    threadClients_.SendEvent(
        kEventWritePending);  // Notify ClientThreadProc() that data is ready
                              // to be sent to a client
  }
}

void CServerConnectionTCP::SetCallbacks(FnOnConnection fnOnConnection,
                                        FnOnRead fnOnRead,
                                        FnOnWriteReady fnOnWriteReady) {
  fnOnConnection_ = fnOnConnection;
  fnOnRead_ = fnOnRead;
  fnOnWriteReady_ = fnOnWriteReady;
}

int CServerConnectionTCP::SetSocketNonBlocking(SOCKET socket,
                                               bool fNonBlocking) {
  int errRet = 0;

#ifdef _WIN32

  unsigned long iMode = fNonBlocking ? 1 : 0;

  return (ioctlsocket(socket, FIONBIO, &iMode));

#else  //_WIN32

  int flags = fcntl(socket, F_GETFL, 0);

  if (flags < 0)
    errRet = errno;
  else {
    if (fNonBlocking)
      flags |= O_NONBLOCK;
    else
      flags &= ~O_NONBLOCK;
    if (fcntl(socket, F_SETFL, flags) < 0) errRet = errno;
  }

#endif  // _WIN32

  return (errRet);
}

void CServerConnectionTCP::Start(void) {
  if (!fThreadClientIsRunning_) {
    threadClients_.Start(std::bind(&CServerConnectionTCP::ClientThreadProc,
                                   this, std::placeholders::_1));
    fThreadClientIsRunning_ = true;
  }
}

void CServerConnectionTCP::Stop(void) {
  threadClients_.StopAsync();
  threadClients_.WaitForStop();
  fThreadClientIsRunning_ = false;
}

#if defined(_WIN32)
CServerConnectionTCP::WaitResult CServerConnectionTCP::WaitForEvent(
    CThreadTCS& threadtcs, unsigned int msTimeout) {
  WaitResult waitresultRet = kWaitResultFail;
  auto& events = threadtcs.GetEvents();
  std::vector<HANDLE> vech;

  // Add entry for event event
  vech.push_back(threadtcs.GetHEvent());

  // Add entry for connect socketIn
  vech.push_back(wsaeventSocketListen_);
  ussocketeventsListen_.clear();

  // Add entries for client sockets
  for (auto pair : umclientidpcliententry_) {
    vech.push_back(pair.second->wsaevent);
    pair.second->ussocketevents.clear();
    ;
  }

  // Wait for a network event
  for (;;) {
    CEvents::Event event;

    // Check for events that are already set
    if (events.GetEventSet(&event)) {
      waitresultRet =
          (WaitResult)((unsigned int)kWaitResultEvent0 + (unsigned int)event);
      break;
    }

    // Wait for an I/O event
    DWORD dwWait = WaitForMultipleObjects(
        vech.size(), vech.data(), FALSE,
        (msTimeout == kMSTimeoutInfinite) ? INFINITE : msTimeout);

    if (dwWait == WAIT_TIMEOUT) {
      waitresultRet = kWaitResultTimeout;
      break;
    } else if (dwWait == WAIT_OBJECT_0) {
      if (!events.GetEventSet(&event))
        continue;  // Ignore spurious signal
      else {
        waitresultRet =
            (WaitResult)((unsigned int)kWaitResultEvent0 + (unsigned int)event);
      }
      break;
    } else if ((dwWait < WAIT_OBJECT_0) ||
               ((dwWait - WAIT_OBJECT_0) >= vech.size())) {
      // Got an unexpected result
      waitresultRet = kWaitResultFail;
      break;
    } else if (dwWait >= (WAIT_OBJECT_0 + 1)) {
      bool fGotEvent = false;
      int ientry =
          1;  // First socket entry is 1; entry 0 is the thread event handle
      WSAEVENT rgwsaevent[WSA_MAXIMUM_WAIT_EVENTS];
      WSANETWORKEVENTS wsanetworkevents;

      // Check for events on connect socketIn
      if ((WSAWaitForMultipleEvents(1, &vech[ientry], FALSE, 0, FALSE) ==
           WSA_WAIT_EVENT_0) &&
          (WSAEnumNetworkEvents(socketListen_, wsaeventSocketListen_,
                                &wsanetworkevents) == 0)) {
        if (wsanetworkevents.lNetworkEvents & FD_ACCEPT) {
          ussocketeventsListen_.insert(SocketEvents::Read);
          fGotEvent = true;
        }
      }
      ++ientry;

      // Check for events on client sockets
      for (auto pair : umclientidpcliententry_) {
        auto pcliententry = pair.second;

        pcliententry->ussocketevents.clear();
        if ((WSAWaitForMultipleEvents(1, &vech[ientry], FALSE, 0, FALSE) ==
             WSA_WAIT_EVENT_0) &&
            (WSAEnumNetworkEvents(pcliententry->socket, pcliententry->wsaevent,
                                  &wsanetworkevents) == 0)) {
          if (wsanetworkevents.lNetworkEvents & FD_CLOSE)
            pcliententry->ussocketevents.insert(SocketEvents::Close);
          if (wsanetworkevents.lNetworkEvents & FD_READ)
            pcliententry->ussocketevents.insert(SocketEvents::Read);
          if (wsanetworkevents.lNetworkEvents & FD_WRITE)
            pcliententry->ussocketevents.insert(SocketEvents::Write);

          fGotEvent |= !pcliententry->ussocketevents.empty();
        }

        ++ientry;
      }

      if (fGotEvent) {
        waitresultRet = kWaitResultSocket;
        break;
      }
    }
  }

  return (waitresultRet);
}

#elif defined(__linux__)

CServerConnectionTCP::WaitResult CServerConnectionTCP::WaitForEvent(
    CThreadTCS& threadtcs, unsigned int msTimeout) {
  WaitResult waitresultRet = kWaitResultFail;
  int cfd;
  auto& events = threadtcs.GetEvents();
  int fdEvent = threadtcs.GetFDEvent();
  int fdLimit = 0;
  int err;
  fd_set fdsetRead;
  fd_set fdsetWrite;

  FD_ZERO(&fdsetRead);
  FD_ZERO(&fdsetWrite);

  // Add entry for event socket
  FD_SET(fdEvent, &fdsetRead);
  fdLimit = fdEvent;

  // Add entry for connect socketIn
  FD_SET(socketListen_, &fdsetRead);
  ussocketeventsListen_.clear();
  if (socketListen_ > fdLimit) fdLimit = socketListen_;

  // Add entries for client sockets
  for (auto& pair : umclientidpcliententry_) {
    ClientEntry* pcliententry = pair.second;

    FD_SET(pcliententry->socket, &fdsetRead);
    if (pcliententry->fWriteInProgress)
      FD_SET(pcliententry->socket, &fdsetWrite);

    if (pcliententry->socket >= fdLimit) fdLimit = pcliententry->socket;

    pair.second->ussocketevents.clear();
  }

  // Wait for a network event
  for (;;) {
    CEvents::Event event;

    // Check for events that are already set
    if (events.GetEventSet(&event)) {
      waitresultRet =
          (WaitResult)((unsigned int)kWaitResultEvent0 + (unsigned int)event);
      break;
    }

    // Wait for I/O or an event signal
    {
      timeval timevalTimeout;
      timeval* ptimevalTimeout = nullptr;

      if (msTimeout != kMSTimeoutInfinite) {
        timevalTimeout.tv_sec = msTimeout / 1000;
        timevalTimeout.tv_usec =
            (msTimeout - timevalTimeout.tv_sec * 1000) * 1000;
        ptimevalTimeout = &timevalTimeout;
      }

      cfd = select(fdLimit + 1, &fdsetRead, &fdsetWrite, nullptr,
                   ptimevalTimeout);
    }
    if (cfd < 0) {
      if (errno == EINTR) continue;  // Retry the wait

      break;  // Unexpected error
    } else if (cfd == 0) {
      if (msTimeout == kMSTimeoutInfinite)
        continue;  // Not a timeout
      else {
        waitresultRet = kWaitResultTimeout;
        break;
      }
    } else if (FD_ISSET(fdEvent, &fdsetRead)) {
      uint8_t rgb[16];

      read(fdEvent, rgb, sizeof(rgb));
      if (!events.GetEventSet(&event)) {
        continue;  // Ignore spurious signal (events were probably processed
                   // previously)
      } else {
        waitresultRet =
            (WaitResult)((unsigned int)kWaitResultEvent0 + (unsigned int)event);
      }
      break;
    } else {
      bool fGotEvent = false;

      // Check for events on connect socketIn
      if (FD_ISSET(socketListen_, &fdsetRead))
        ussocketeventsListen_.insert(SocketEvents::Read);
      if (FD_ISSET(socketListen_, &fdsetWrite))
        ussocketeventsListen_.insert(SocketEvents::Write);
      fGotEvent = !ussocketeventsListen_.empty();

      // Check for events on client sockets
      for (auto pair : umclientidpcliententry_) {
        auto pcliententry = pair.second;

        if (FD_ISSET(pcliententry->socket, &fdsetRead))
          pcliententry->ussocketevents.insert(SocketEvents::Read);
        if (FD_ISSET(pcliententry->socket, &fdsetWrite))
          pcliententry->ussocketevents.insert(SocketEvents::Write);

        fGotEvent |= !pcliententry->ussocketevents.empty();
      }

      if (fGotEvent) {
        waitresultRet = kWaitResultSocket;
        break;
      }
    }
  }

  return (waitresultRet);
}
#endif  // defined(_WIN32)

CServerConnectionTCP::ClientEntry::ClientEntry(ClientID clientidIn)
    : clientid(clientidIn),
      socket(kSocketInvalid),
      fWriteInProgress(false),
      msLastWrite(0),
      mutexWrite(),
      queuevecbWrite(),
      rgbBufferRead(),
#ifdef _WIN32
      wsaevent(WSACreateEvent()),
#endif  //_WIN32
      m_cref(1) {
}

CServerConnectionTCP::ClientEntry::~ClientEntry() {
  CloseSocket();

#ifdef _WIN32
  if (this->wsaevent != NULL) WSACloseEvent(this->wsaevent);
#endif  //_WIN32
}

void CServerConnectionTCP::ClientEntry::AddRef(void) {
#ifdef _WIN32
  InterlockedIncrement(&m_cref);
#else   //_WIN32
  __sync_add_and_fetch(&m_cref, 1);
#endif  //_WIN32
}

void CServerConnectionTCP::ClientEntry::CloseSocket(void) {
  if (this->socket != kSocketInvalid) {
    // Shutdown write to server and empty pending reads
    {
#ifdef _WIN32
      shutdown(this->socket, SD_SEND);
#else   //_WIN32
      shutdown(this->socket, SHUT_WR);
#endif  //_WIN32
      int rc = 0;

      // SetSocketNonBlocking(this->socket, false);
      do {
        int cfd;
        fd_set fdsetRead;
        timeval timevalTimeout = {0};

        FD_ZERO(&fdsetRead);
        FD_SET(this->socket, &fdsetRead);
        timevalTimeout.tv_sec = 5;
        cfd = select(this->socket + 1, &fdsetRead, nullptr, nullptr,
                     &timevalTimeout);

        if (cfd < 0) {
          if (errno == EINTR) continue;
          break;
        } else if (cfd == 0)
          break;  // Timeout

        if (FD_ISSET(this->socket, &fdsetRead)) {
          char ch;

          while ((rc = recv(this->socket, &ch, 1, 0)) > 0)
            ;
          printf("CloseSocket(): recv() returned %d, errno = %d\n", rc, errno);
        }
      } while ((rc == -1) && (errno == EAGAIN));
    }

    CServerConnectionTCP::CloseSocket(&this->socket);
  }
}

void CServerConnectionTCP::ClientEntry::Release(void) {
#ifdef _WIN32
  if (InterlockedDecrement(&m_cref) == 0)
#else   //_WIN32
  if (__sync_add_and_fetch(&m_cref, -1) == 0)
#endif  //_WIN32
  {
    delete this;
  }
}

CServerConnectionTCP::Err CServerConnectionTCP::ClientEntry::SetSocket(
    SOCKET socketIn) {
  CloseSocket();

  this->socket = socketIn;
  SetSocketNonBlocking(socketIn, true);

#ifdef _WIN32
  if (((this->wsaevent = WSACreateEvent()) == NULL) ||
      (WSAEventSelect(socketIn, this->wsaevent, FD_READ | FD_WRITE) != 0))
    return (WSAGetLastError());
#endif  //_WIN32

  return (0);
}

bool CServerConnectionTCP::ClientEntry::Write(const void* pv, size_t cb) {
  bool fOKRet = true;
  int cbWritten;
  auto pb = reinterpret_cast<const char*>(pv);

  this->fWriteInProgress = true;

  // DebugPrint("Writing %d bytes to client %p\n", this->vecbWrite.size(),
  // this);

  cbWritten = send(this->socket, pb, cb, 0);
  if (cbWritten < 0) {
    int err = GetErr();

    if (err == kErrWouldBlock)
      cbWritten = 0;  // No space in output buffer, wait for write-available
                      // network event
    else {
#ifdef NOTDEF
      if (err == kErrConnReset)
        DebugPrint(
            "CServerConnectionTCP::ClientEntry::Write(): client closed "
            "connection");
      else
        DebugPrint(
            "CServerConnectionTCP::ClientEntry::Write(): send() returned error "
            "%d",
            err);
#endif  // NOTDEF

      this->fWriteInProgress = false;
      fOKRet = false;
    }
  }

  // If write succeeded, update the write timestamp and buffer
  if (fOKRet) {
    this->msLastWrite = CServerConnectionTCP::GetMSTimestamp();

    if (cbWritten == cb) {
      // Everything was written, no need for another write
      this->vecbWrite.clear();
      this->fWriteInProgress = false;
    } else {
      // Save what was not written for the next write
      this->vecbWrite.assign(pb + cbWritten, pb + cb);
    }
  }

  return (fOKRet);
}

CServerConnectionTCP::CThreadTCS::CThreadTCS(void)
    : CThread(),
#ifdef _WIN32
      hEventEvent_(CreateEvent(NULL, FALSE, FALSE, NULL))
#endif  // _WIN32
#ifdef __linux__
          fdEventReceive_(-1),
      fdEventSend_(-1)
#endif  // __linux__
{
#ifdef __linux__
  int rgfd[2];

  if (pipe2(rgfd, O_NONBLOCK) == 0) {
    fdEventReceive_ = rgfd[0];
    fdEventSend_ = rgfd[1];
  } else {
    DebugPrint("******pipe() failed, errno = %d", errno);
  }
#endif  //__linux__
}

CServerConnectionTCP::CThreadTCS::~CThreadTCS() {
  StopAsync();

#ifdef _WIN32
  if (hEventEvent_ != NULL) CloseHandle(hEventEvent_);
#endif  // _WIN32
#ifdef __linux__
  if (fdEventSend_ >= 0) close(fdEventSend_);
  if (fdEventReceive_ >= 0) close(fdEventReceive_);
#endif  // __linux__
}

void CServerConnectionTCP::CThreadTCS::SendEvent(Event event) {
  if (pthread_ == nullptr) return;

  events_.Set(event);

  // Signal thread to look at new event

#ifdef _WIN32
  SetEvent(hEventEvent_);
#else
  if (fdEventSend_ >= 0) {
    uint8_t b = 0;

    (void)write(fdEventSend_, &b, 1);
  }
#endif  // _WIN32
}

void CServerConnectionTCP::CThreadTCS::StopAsync(void) {
  SendEvent(kEventExitThread);
  CThread::StopAsync();
}

}  // namespace LVMon
