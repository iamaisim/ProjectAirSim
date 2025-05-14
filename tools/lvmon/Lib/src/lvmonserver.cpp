// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "serverconnectiontcp.h"
#ifdef _WIN32
#include "serverconnectionpipe.h"
#endif  // _WIN32
#include <assert.h>
#include <stdarg.h>
#include <time.h>

#include <functional>

#include "lvmonprotocol.h"
#include "lvmonserver.h"

namespace stdph = std::placeholders;

namespace {

void DebugPrint(const char* szFmt, ...) {
  char sz[2048];
  va_list vl;

#ifdef WIN32
  size_t cch;

  va_start(vl, szFmt);
  cch = vsprintf_s(sz, sizeof(sz) - 1, szFmt, vl);
  va_end(vl);
  if ((cch < 1) || (sz[cch - 1] != '\n')) {
    sz[cch++] = '\n';
    sz[cch] = '\0';
  }
  OutputDebugStringA(sz);
#else   // WIN32
  va_start(vl, szFmt);
  vsprintf(sz, szFmt, vl);
  va_end(vl);
  puts(sz);
#endif  // WIN32
}

}  // namespace

namespace LVMon {

CLVMonServer::CLVMonServer(void)
    : mutexClients_(),
      mutexValues_(),
      mutexValuesAdded_(),
      mutexValuesChanged_()
#ifdef WINDOWS_PIPES
      ,
      piserverconnection_(new CServerConnectionPipe())
#else
      ,
      piserverconnection_(new CServerConnectionTCP())
#endif  // WINDOWS_PIPES
      ,
      threadNotify_(),
      umclientidcliententry_(),
      umpchpvalueentry_(),
      umstrvalueentry_(),
      umvalueidpvalueentry_(),
      uspvalueentryAdded_(),
      uspvalueentryChanged_(),
      valueidNext_(0) {
  piserverconnection_->SetCallbacks(
      std::bind(&CLVMonServer::OnPCS_Connection, this, stdph::_1, stdph::_2),
      std::bind(&CLVMonServer::OnPCS_Read, this, stdph::_1, stdph::_2),
      std::bind(&CLVMonServer::OnPCS_WriteReady, this, stdph::_1, stdph::_2));
  piserverconnection_->Start();
  threadNotify_.Start(
      std::bind(&CLVMonServer::NotifyThreadProc, this, stdph::_1));
}

CLVMonServer::~CLVMonServer(void) {
  threadNotify_.StopAsync();
  threadNotify_.WaitForStop();
  piserverconnection_->Stop();
  delete piserverconnection_;
  DebugPrint("*****LVMonServer Stopped");
}

CLVMonServer::ValueEntry* CLVMonServer::EnsureValueEntry(const char* szName,
                                                         ValueType valuetype) {
  bool fIsNewEntry = false;
  ValueEntry* pvalueentryRet = nullptr;
  std::unique_lock<std::recursive_mutex> ulValues(mutexValues_);
  auto itPch = umpchpvalueentry_.find(szName);

  // Try finding the entry by the name pointer
  if (itPch != umpchpvalueentry_.end()) {
    pvalueentryRet = itPch->second;
    ulValues.unlock();  // We're done reading from the value entry maps
  } else {
    // Not found by that pointer--look up by name and create a new one if needed
    auto itValueEntry = umstrvalueentry_.find(szName);

    if (itValueEntry != umstrvalueentry_.end())
      pvalueentryRet = &itValueEntry->second;
    else {
      std::string strName(szName);

      // Not found by name?  It's a new entry.
      fIsNewEntry = true;
      pvalueentryRet =
          &umstrvalueentry_
               .emplace(std::make_pair(
                   szName, ValueEntry(valueidNext_++, valuetype, szName)))
               .first->second;
      umvalueidpvalueentry_.emplace(pvalueentryRet->valueid, pvalueentryRet);
      umpchpvalueentry_.emplace(szName, pvalueentryRet);
      ulValues.unlock();  // We're done reading from and modifying the value
                          // entry maps

      // DebugPrint("LVMonServer: Adding new value \"%s\", type %d\n", szName,
      // valuetype);

      // Tell notification thread about the new value
      {
        std::unique_lock<std::mutex> ul(mutexValuesAdded_);

        uspvalueentryAdded_.insert(pvalueentryRet);
        threadNotify_.GetEvents().Set(kEventValueAdded);
      }
    }
  }

  // A value's type must never change once set
  if (!fIsNewEntry && (pvalueentryRet != nullptr)) {
    assert(pvalueentryRet->valuetype == valuetype);
    if (pvalueentryRet->valuetype != valuetype) pvalueentryRet = nullptr;
  }

  return (pvalueentryRet);
}

LVMon::MsgValueUpdate* CLVMonServer::CreateUpdateMessageForValue(
    ValueEntry* pvalueentry, VecB* pvecbRet) {
  LVMon::MsgValueUpdate* pmvuRet = nullptr;

  // Construct value change message
  switch (pvalueentry->valuetype) {
    default:
      assert(false);  // Unhandled value type?
      break;

    case ValueType::Double:
      pmvuRet = LVMon::MsgValueUpdate::Create(
          pvalueentry->valuetype, pvalueentry->valueid,
          pvalueentry->microseconds, pvalueentry->r, pvecbRet);
      break;

    case ValueType::Int64:
      pmvuRet = LVMon::MsgValueUpdate::Create(
          pvalueentry->valuetype, pvalueentry->valueid,
          pvalueentry->microseconds, pvalueentry->i64, pvecbRet);
      break;

    case ValueType::String:
      pmvuRet = LVMon::MsgValueUpdate::Create(
          pvalueentry->valuetype, pvalueentry->valueid,
          pvalueentry->microseconds, pvalueentry->str, pvecbRet);
      break;

    case ValueType::UInt64:
      pmvuRet = LVMon::MsgValueUpdate::Create(
          pvalueentry->valuetype, pvalueentry->valueid,
          pvalueentry->microseconds, pvalueentry->ui64, pvecbRet);
      break;
  }

  return (pmvuRet);
}

template <>
void CLVMonServer::Get(const char* szName, int64_t* pi64Ret) {
  ValueEntry* pvalueentry;

  if ((pvalueentry = EnsureValueEntry(szName, ValueType::Int64)) != nullptr) {
    std::unique_lock<std::mutex> ul(pvalueentry->mutex);

    *pi64Ret = pvalueentry->i64;
  }
}

template <>
void CLVMonServer::Get(const char* szName, double* prRet) {
  ValueEntry* pvalueentry;

  if ((pvalueentry = EnsureValueEntry(szName, ValueType::Double)) != nullptr) {
    std::unique_lock<std::mutex> ul(pvalueentry->mutex);

    *prRet = pvalueentry->r;
  }
}

template <>
void CLVMonServer::Get(const char* szName, uint64_t* pui64Ret) {
  ValueEntry* pvalueentry;

  if ((pvalueentry = EnsureValueEntry(szName, ValueType::UInt64)) != nullptr) {
    std::unique_lock<std::mutex> ul(pvalueentry->mutex);

    *pui64Ret = pvalueentry->ui64;
  }
}

template <>
void CLVMonServer::Get(const char* szName, std::string* pstrRet) {
  ValueEntry* pvalueentry;

  if ((pvalueentry = EnsureValueEntry(szName, ValueType::String)) != nullptr) {
    std::unique_lock<std::mutex> ul(pvalueentry->mutex);

    *pstrRet = pvalueentry->str;
  }
}

CLVMonServer::MicroSeconds CLVMonServer::GetTimestampCur(void) {
  timespec ts;

  return ((timespec_get(&ts, TIME_UTC) == 0)
              ? 0
              : (ts.tv_sec * 1000 * 1000 + ts.tv_nsec / 1000));
}

void CLVMonServer::NotifyChange(ValueEntry* pvalueentry) {
  if (!pvalueentry->listclientidSubscribers.empty()) {
    std::unique_lock<std::mutex> ul(mutexValuesChanged_);

    uspvalueentryChanged_.insert(pvalueentry);
    threadNotify_.GetEvents().Set(kEventValueChanged);
  }
}

void CLVMonServer::NotifyThreadProc(CThread* pthread) {
  auto& events = pthread->GetEvents();
  bool fExit = false;
  CEvents::Event rgevent[] = {CThread::kEventExitThread, kEventValueAdded,
                              kEventValueChanged};

  DebugPrint("******LVMon Server started");
  while (!fExit) {
    auto waitresult =
        events.WaitForMultiple(sizeof(rgevent) / sizeof(*rgevent), rgevent);

    switch (waitresult) {
      default:
      case CEvents::kWaitResultFail:
        assert(false);  // Unexpected result from events.WaitForMultiple()
        break;

      case CEvents::kWaitResultEvent0:  // Request for thread to exit
        fExit = true;
        break;

      case CEvents::kWaitResultEvent0 + 1:  // New value added
      {
        USPValueEntry uspvalueentryAdded;

        // Get new value entries
        {
          std::unique_lock<std::mutex> ul(mutexValuesAdded_);

          uspvalueentryAdded.swap(uspvalueentryAdded_);
        }

        // Tell all the clients about the new value
        for (auto pvalueentry : uspvalueentryAdded) {
          LVMon::MsgNewName* pmnn;
          VecB vecbMsg;

          // Create new name message
          pmnn = LVMon::MsgNewName::Create(
              pvalueentry->valuetype, pvalueentry->valueid,
              pvalueentry->strName.c_str(), &vecbMsg);
          if (pmnn != nullptr) {
            pmnn->HToN();

            {
              std::unique_lock<std::mutex> ul(mutexClients_);

              for (auto& pair : umclientidcliententry_) {
                auto clientid = pair.second.clientid;

                // DebugPrint("Sending new name \"%s\" (0x%x) to client %d\n",
                // pvalueentry->strName.c_str(), pvalueentry->valueid,
                // clientid);
                SendToClient(clientid, pmnn);
              }
            }
          }
        }
      } break;

      case CEvents::kWaitResultEvent0 + 2:  // Value(s) has been changed
      {
        USPValueEntry uspvalueentry;

        // Get values that have changed
        {
          std::unique_lock<std::mutex> ul(mutexValuesChanged_);

          uspvalueentry.swap(uspvalueentryChanged_);
        }

        // Notify subscribers of changes
        for (auto pvalueentry : uspvalueentry) {
          std::unique_lock<std::mutex> ul(pvalueentry->mutex);

          if (!pvalueentry->listclientidSubscribers.empty()) {
            auto listclientidSubscribers = pvalueentry->listclientidSubscribers;
            LVMon::MsgValueUpdate* pmvu;
            auto valueid = pvalueentry->valueid;
            VecB vecb;

            // Create value update message
            pmvu = CreateUpdateMessageForValue(pvalueentry, &vecb);
            ul.unlock();

            if (pmvu != nullptr) {
              pmvu->HToN();

              // Send message to all subscribing clients
              for (auto clientid : listclientidSubscribers) {
                std::lock_guard<std::mutex> lgClient(mutexClients_);
                auto it = umclientidcliententry_.find(clientid);

                if (it != umclientidcliententry_.end()) {
                  auto& cliententry = it->second;
                  std::lock_guard<std::mutex> lgClientEntry(
                      cliententry.mutexValue);

                  // To handle a slow client, we'll send the update message if
                  // the client doesn't already have a update message for this
                  // value already queued to the client.  If itPending does,
                  // we'll just note that another needs to be sent.
                  if (cliententry.usvalueidChangeSent.find(valueid) !=
                      cliententry.usvalueidChangeSent.end()) {
                    // Already queued, add to pending list
                    cliententry.usvalueidChangePending.insert(valueid);
                  } else {
                    // Not queued, send the message to the client
                    cliententry.usvalueidChangeSent.insert(valueid);
                    SendToClient(clientid, pmvu);
                  }
                }
              }
            }
          }
        }
      } break;
    }  // switch (waitresult)
  }    // while (!fExit)
}

void CLVMonServer::OnPCS_Connection(
    IServerConnection* /*piserverconnection*/,
    const IServerConnection::ConnectionEventArgs& args) {
  if (args.reason == IServerConnection::ConnectionEventArgs::Reason::Connect) {
    std::unique_lock<std::mutex> ul(mutexClients_);

    umclientidcliententry_.emplace(args.clientid, args.clientid);
    DebugPrint("LVMonServer: New client %d connected", args.clientid);
  } else if (args.reason ==
             IServerConnection::ConnectionEventArgs::Reason::Disconnect) {
    RemoveClient(args.clientid);
    DebugPrint("LVMonServer: Client %d disconnected", args.clientid);
  }
}

void CLVMonServer::OnPCS_Read(IServerConnection* /*piserverconnection*/,
                              const IServerConnection::ReadEventArgs& args) {
  ProcessMessageFromClient(args.clientid, args.pb, args.cb);
}

void CLVMonServer::OnPCS_WriteReady(
    IServerConnection* /*piserverconnection*/,
    const IServerConnection::WriteReadyEventArgs& args) {
  // Determine whether we need to send another update for a value for which we
  // recently sent an update
  {
    std::lock_guard<std::mutex> lgClients(mutexClients_);
    auto it = umclientidcliententry_.find(args.clientid);

    if (it != umclientidcliententry_.end()) {
      auto& cliententry = it->second;
      std::lock_guard<std::mutex> lgClientEntry(cliententry.mutexValue);
      USValueID usvalueidChangeSent = cliententry.usvalueidChangeSent;

      // Note that this callback means the client's write buffer is empty so all
      // values noted in the sent queue have actually been sent
      for (auto valueid : usvalueidChangeSent) {
        USValueID::iterator itPending;

        // See if there's another update for this value
        itPending = cliententry.usvalueidChangePending.find(valueid);
        if (itPending == cliententry.usvalueidChangePending.end()) {
          // Nope, we're done updating this value for the client
          cliententry.usvalueidChangeSent.erase(valueid);
        } else {
          // Yup, we need to send another update
          auto itValue = umvalueidpvalueentry_.find(valueid);
          LVMon::MsgValueUpdate* pmvu;
          VecB vecb;

          cliententry.usvalueidChangePending.erase(itPending);

          {
            std::lock_guard<std::mutex> lg(itValue->second->mutex);

            pmvu = CreateUpdateMessageForValue(itValue->second, &vecb);
          }
          if (pmvu != nullptr) {
            pmvu->HToN();
            SendToClient(cliententry.clientid, pmvu);
          }
        }
      }  // while (!cliententry.queuevalueidSent.empty())
    }
  }
}

void CLVMonServer::ProcessMessageFromClient(ClientID clientid, uint8_t* rgb,
                                            size_t cb) {
  assert(cb >= sizeof(LVMon::MsgHeader));
  if (cb < sizeof(LVMon::MsgHeader)) return;

  ClientEntry* pcliententry;

  // Get client entry
  {
    auto it = umclientidcliententry_.find(clientid);

    assert(it != umclientidcliententry_.end());
    if (it == umclientidcliententry_.end())
      return;  // Couldn't find the
               // client

    pcliententry = &it->second;
  }

  // Extract and process each message from the client
  for (auto *pbNew = rgb, *pbNewEnd = rgb + cb; pbNew < pbNewEnd;) {
    size_t cbMessage;
    uint8_t* pbMessage;
    LVMon::MsgHeader* pmsgheader;

    // Get the next message
    if (!pcliententry->lvmonmessage.Process(&pbNew, pbNewEnd, &pbMessage,
                                            &cbMessage))
      break;

    pmsgheader = reinterpret_cast<LVMon::MsgHeader*>(
        pbMessage);  // Note that LVMonMessage::Process() has already arranged
                     // the message header bytes in host order
    // DebugPrint("Got message %d, %d data bytes from client %d\n",
    // pmsgheader->messagetype, pmsgheader->cbData, clientid);

    switch ((LVMon::MessageType)pmsgheader->messagetype) {
      default:
        // Ignore unknown message
        break;

      case LVMon::MessageType::GetNames: {
        VecB vecbMsg;
        std::unique_lock<std::recursive_mutex> ul(mutexValues_);

        for (auto& pair : umstrvalueentry_) {
          LVMon::MsgNewName* pmsgnewname;

          // DebugPrint("Sending new name \"%s\" (0x%x) to client %d\n",
          // pair.first.c_str(), pair.second.valueid, clientid);
          pmsgnewname = LVMon::MsgNewName::Create(pair.second.valuetype,
                                                  pair.second.valueid,
                                                  pair.first.c_str(), &vecbMsg);
          pmsgnewname->HToN();
          SendToClient(clientid, pmsgnewname);
        }
      } break;

      case LVMon::MessageType::NewName:
        // Ignore--to clients only
        break;

      case LVMon::MessageType::Null:
        // Always ignore this message
        break;

      case LVMon::MessageType::SetValue: {
        auto pmsgsetvalue = static_cast<LVMon::MsgSetValue*>(pmsgheader);
        ValueEntry* pvalueentry = nullptr;
        MicroSeconds usCur = GetTimestampCur();

        pmsgsetvalue->NToH();

        // Get the value, if any
        {
          std::unique_lock<std::recursive_mutex> ul(mutexValues_);
          auto it = umvalueidpvalueentry_.find((ValueID)pmsgsetvalue->valueid);

          if (it != umvalueidpvalueentry_.end()) pvalueentry = it->second;
        }

        // If we found the value entry, set the value
        if ((pvalueentry != nullptr) &&
            ((ValueType)pmsgsetvalue->valuetype == pvalueentry->valuetype)) {
          switch (pvalueentry->valuetype) {
            default:
              break;

            case ValueType::Double:
              SetValue(pvalueentry, usCur, pmsgsetvalue->value.r);
              break;

            case ValueType::Int64:
              SetValue(pvalueentry, usCur, pmsgsetvalue->value.i64);
              break;

            case ValueType::String:
              SetValue(pvalueentry, usCur, pmsgsetvalue->value.str.rgch,
                       pmsgsetvalue->value.str.cch);
              break;

            case ValueType::UInt64:
              SetValue(pvalueentry, usCur, pmsgsetvalue->value.ui64);
              break;
          }
        }
      } break;

      case LVMon::MessageType::Subscribe: {
        auto pmsgsubscribe = static_cast<LVMon::MsgSubscribe*>(pmsgheader);
        ValueEntry* pvalueentry = nullptr;

        pmsgsubscribe->NToH();

        // DebugPrint("Client %d subscribing to value 0x%x\n", clientid,
        // pmsgsubscribe->valueid);

        // Get value's entry
        {
          std::unique_lock<std::recursive_mutex> ul(mutexValues_);
          auto it = umvalueidpvalueentry_.find((ValueID)pmsgsubscribe->valueid);

          if (it != umvalueidpvalueentry_.end()) pvalueentry = it->second;
        }

        // If we found the value entry, add the client to the list of
        // subscribers
        if (pvalueentry != nullptr) {
          LVMon::MsgValueUpdate* pmvu;
          VecB vecb;

          // Add client to list of subscribers
          {
            std::unique_lock<std::mutex> ul(pvalueentry->mutex);

            pvalueentry->listclientidSubscribers.push_back(clientid);
          }

          // Send current value to client
          pmvu = CreateUpdateMessageForValue(pvalueentry, &vecb);
          pmvu->HToN();
          SendToClient(clientid, pmvu);
        }
      } break;

      case LVMon::MessageType::Unsubscribe: {
        auto pmsgunsubscribe = static_cast<LVMon::MsgUnsubscribe*>(pmsgheader);
        ValueEntry* pvalueentry = nullptr;

        pmsgunsubscribe->NToH();
        // DebugPrint("Client %d unsubscribing from value 0x%x\n", clientid,
        // pmsgunsubscribe->valueid);

        // Find the value entry
        {
          std::unique_lock<std::recursive_mutex> ul(mutexValues_);
          auto it =
              umvalueidpvalueentry_.find((ValueID)pmsgunsubscribe->valueid);

          if (it != umvalueidpvalueentry_.end()) pvalueentry = it->second;
        }

        // Remove client from list of subscribers
        {
          std::unique_lock<std::mutex> ul(pvalueentry->mutex);
          auto& listclientidSubscribers = pvalueentry->listclientidSubscribers;

          for (auto itClientID = listclientidSubscribers.begin(),
                    itClientIDEnd = listclientidSubscribers.end();
               itClientID != itClientIDEnd; ++itClientID) {
            if (*itClientID == clientid) {
              listclientidSubscribers.erase(itClientID);
              break;
            }
          }
        }
      } break;

      case LVMon::MessageType::ValueUpdate:
        // Ignore--to clients only
        break;
    }  // switch (pmsgheader->messagetype)
  }    // for (pbNew)
}

void CLVMonServer::RemoveClient(ClientID clientid) {
  // DebugPrint("CLVMonServer::RemoveClient():  Removing client %d\n",
  // clientid);

  // Remove client from all subscriptions
  {
    std::unique_lock<std::recursive_mutex> ulValues(mutexValues_);

    for (auto& pair : umstrvalueentry_) {
      auto& valueentry = pair.second;
      std::unique_lock<std::mutex> ul(valueentry.mutex);
      auto& listclientidSubscribers = valueentry.listclientidSubscribers;

      for (auto it = listclientidSubscribers.begin(),
                itEnd = listclientidSubscribers.end();
           it != itEnd; ++it) {
        if (*it == clientid) {
          listclientidSubscribers.erase(it);
          break;
        }
      }
    }
  }

  // Remove client entry
  {
    std::unique_lock<std::mutex> ul(mutexClients_);
    auto it = umclientidcliententry_.find(clientid);

    if (it != umclientidcliententry_.end()) umclientidcliententry_.erase(it);
  }
}

int CLVMonServer::SendToClient(ClientID clientid,
                               const LVMon::MsgHeader* pmsgheader) {
  int errRet = 0;
  size_t cbMsg = sizeof(*pmsgheader) + pmsgheader->cbData;
  auto pbMsg = reinterpret_cast<const uint8_t*>(pmsgheader);

  // DebugPrint("CLVMonServer::SendToClient:: Sending %d bytes to client %p\n",
  // cbMsg, pcliententry);

  piserverconnection_->Send(clientid, pbMsg, cbMsg);

  return (errRet);
}

void CLVMonServer::Set(const char* szName, int64_t i64) {
  ValueEntry* pvalueentry;
  MicroSeconds us = GetTimestampCur();

  if ((pvalueentry = EnsureValueEntry(szName, ValueType::Int64)) != nullptr) {
    SetValue(pvalueentry, us, i64);
  }
}

void CLVMonServer::Set(const char* szName, uint64_t ui64) {
  ValueEntry* pvalueentry;
  MicroSeconds us = GetTimestampCur();

  if ((pvalueentry = EnsureValueEntry(szName, ValueType::UInt64)) != nullptr) {
    SetValue(pvalueentry, us, ui64);
  }
}

void CLVMonServer::Set(const char* szName, double r) {
  ValueEntry* pvalueentry;
  MicroSeconds us = GetTimestampCur();

  if ((pvalueentry = EnsureValueEntry(szName, ValueType::Double)) != nullptr) {
    SetValue(pvalueentry, us, r);
  }
}

void CLVMonServer::Set(const char* szName, const std::string& str) {
  ValueEntry* pvalueentry;
  MicroSeconds us = GetTimestampCur();

  if ((pvalueentry = EnsureValueEntry(szName, ValueType::String)) != nullptr) {
    SetValue(pvalueentry, us, str);
  }
}

void CLVMonServer::Set(const char* szName, const char* sz) {
  ValueEntry* pvalueentry;
  MicroSeconds us = GetTimestampCur();

  if ((pvalueentry = EnsureValueEntry(szName, ValueType::String)) != nullptr) {
    SetValue(pvalueentry, us, sz);
  }
}

void CLVMonServer::SetValue(ValueEntry* pvalueentry, MicroSeconds us,
                            double r) {
  std::unique_lock<std::mutex> ul(pvalueentry->mutex);

  pvalueentry->microseconds = us;
  pvalueentry->r = r;
  NotifyChange(pvalueentry);
}

void CLVMonServer::SetValue(ValueEntry* pvalueentry, MicroSeconds us,
                            int64_t i64) {
  std::unique_lock<std::mutex> ul(pvalueentry->mutex);

  pvalueentry->microseconds = us;
  pvalueentry->i64 = i64;
  NotifyChange(pvalueentry);
}

void CLVMonServer::SetValue(ValueEntry* pvalueentry, MicroSeconds us,
                            uint64_t ui64) {
  std::unique_lock<std::mutex> ul(pvalueentry->mutex);

  pvalueentry->microseconds = us;
  pvalueentry->ui64 = ui64;
  NotifyChange(pvalueentry);
}

void CLVMonServer::SetValue(ValueEntry* pvalueentry, MicroSeconds us,
                            const std::string& str) {
  std::unique_lock<std::mutex> ul(pvalueentry->mutex);

  pvalueentry->microseconds = us;
  pvalueentry->str = str;
  NotifyChange(pvalueentry);
}

void CLVMonServer::SetValue(ValueEntry* pvalueentry, MicroSeconds us,
                            const char* sz) {
  std::unique_lock<std::mutex> ul(pvalueentry->mutex);

  pvalueentry->microseconds = us;
  pvalueentry->str = sz;
  NotifyChange(pvalueentry);
}

void CLVMonServer::SetValue(ValueEntry* pvalueentry, MicroSeconds us,
                            const char* rgch, size_t cch) {
  std::unique_lock<std::mutex> ul(pvalueentry->mutex);

  pvalueentry->microseconds = us;
  pvalueentry->str.assign(rgch, cch);
  NotifyChange(pvalueentry);
}

CLVMonServer::ValueEntry::ValueEntry(ValueID valueidIn, ValueType valuetypeIn,
                                     std::string strNameIn)
    : listclientidSubscribers(),
      microseconds(0),
      mutex(),
      strName(strNameIn),
      valueid(valueidIn),
      valuetype(valuetypeIn),
      i64(0),
      ui64(0),
      r(0),
      str() {}

CLVMonServer::ValueEntry::ValueEntry(ValueEntry&& valueentryOther)
    : listclientidSubscribers(
          std::move(valueentryOther.listclientidSubscribers)),
      microseconds(valueentryOther.microseconds),
      mutex(),
      strName(std::move(valueentryOther.strName)),
      valueid(valueentryOther.valueid),
      valuetype(valueentryOther.valuetype),
      i64(valueentryOther.i64),
      ui64(valueentryOther.ui64),
      r(valueentryOther.r),
      str(std::move(valueentryOther.str)) {}

}  // namespace LVMon
