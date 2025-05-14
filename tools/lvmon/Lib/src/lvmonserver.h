// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef LVMON_LIB_SRC_LVMONSERVER_H_
#define LVMON_LIB_SRC_LVMONSERVER_H_

#ifdef _WIN32
#include <Windows.h>
#endif  // WIN32
#include <stdint.h>

#include <functional>
#include <list>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "events.h"
#include "lvmonmessage.h"
#include "lvmonprotocol.h"
#include "serverconnection_i.h"
#include "thread.h"

namespace LVMon {

class CLVMonServer {
 public:
  CLVMonServer(void);
  ~CLVMonServer();

  template <typename T>
  void Get(const char* szName, T t);

  void Set(const char* szName, int64_t i64);
  void Set(const char* szName, uint64_t ui64);
  void Set(const char* szName, double r);
  void Set(const char* szName, const std::string& str);
  void Set(const char* szName, const char* sz);

 protected:
  // Unique ID for each client assigned by IServerConnection object
  typedef IServerConnection::ClientID ClientID;
  typedef std::list<ClientID> ListClientID;

  typedef std::vector<uint8_t> VecB;  // Array of bytes

  typedef uint64_t MicroSeconds;  // Timestamp in microseconds

  typedef LVMon::ValueID ValueID;                 // Unique ID for a value
  typedef std::unordered_set<ValueID> USValueID;  // Set of value ID's
  typedef LVMon::ValueType ValueType;             // Data type of a value

  // One entry per connected client
  //
  // Value Update Backlog Prevention:
  //--------------------------------
  //  To prevent a slow client from building up a backlog of value updates until
  //  we run out of memory,
  // we limit the number of updates messages actively being sent to a client to
  // one per value.  When a value is updated, a LVMon::MsgValueUpdate message is
  // sent to the client and the value's ID is noted in usvalueidChangeSent.  If
  // the value is updated again and its ID is still in usvalueidChangeSent, we
  // hold off sending another message and instead add the ID to
  // usvalueidChangePending, preventing more than one in-progress update message
  // for a value at a time.
  //  When the client connection invokes our OnPCS_WriteReady() for the client,
  //  we know that all update
  // messages in usvalueidChangeSent have been sent to the client (or at least
  // to the network stack.) For each value in usvalueidChangeSent, we check to
  // see if the ID is in usvalueidChangePending.  If not, we remove the ID from
  // usvalueidChangeSent; if so, we remove it from usvalueidChangePending
  //(leaving the ID in usvalueidChangeSent) and send another update message for
  // the value to the client thus ensuring the client will get another update
  // message for values that have changed since.
  struct ClientEntry {
    ClientID clientid;                 // ServerConnection's ID for this client
    bool fLastMessageWasValueUpdate;   // If true, the last message sent was an
                                       // update message for the value
                                       // valueidLastMessage
    CLVMonMessage lvmonmessage;        // Message reassembler
    std::mutex mutexValue;             // Access guard to usvalueid*
    USValueID usvalueidChangePending;  // Values for a which a change message
                                       // should be queued
    USValueID usvalueidChangeSent;     // Values for which a change message has
                                       // been sent asynchronously to the client
    ValueID valueidLastMessage;        // Value ID of the last update message if
                                       // fLastMessageWasValueUpdate is true

    ClientEntry(ClientID clientidIn)
        : clientid(clientidIn),
          fLastMessageWasValueUpdate(false),
          lvmonmessage(),
          mutexValue(),
          usvalueidChangePending(),
          usvalueidChangeSent(),
          valueidLastMessage(0) {}
  };  // struct ClientEntry
  typedef std::unordered_map<ClientID, ClientEntry> UMClientIDClientEntry;

  // One entry per value name
  struct ValueEntry {
    ListClientID listclientidSubscribers;  // Clients subscribing to this value
    MicroSeconds microseconds;  // Timestamp of when entry was updated last
    std::mutex mutex;           // Access guard to this entry
    std::string strName;        // This entry's name
    ValueID valueid;            // Unique ID for this entry
    ValueType valuetype;        // The type of data in this entry

    // Values
    int64_t i64;      // Int64 value
    uint64_t ui64;    // UInt64 value
    double r;         // Double value
    std::string str;  // String value

    ValueEntry(ValueID valueidIn, ValueType valuetypeIn, std::string strNameIn);
    ValueEntry(ValueEntry&& valueentryOther);

    template <typename T>
    void Set(const T& t);
  };  // struct ValueEntry

  typedef std::unordered_set<ValueEntry*>
      USPValueEntry;  // Set of value entries

  typedef std::unordered_map<std::string, ValueEntry> UMStrValueEntry;
  typedef std::unordered_map<const char*, ValueEntry*> UMPChPValueEntry;
  typedef std::unordered_map<ValueID, ValueEntry*> UMValueIDPValueEntry;

 protected:
  static const CEvents::Event kEventValueAdded =
      0;  // Event to indicate a new value has been added
  static const CEvents::Event kEventValueChanged =
      1;  // Event to indicate a value has been changed

 protected:
  static MicroSeconds GetTimestampCur(void);

 protected:
  ValueEntry* EnsureValueEntry(const char* szName, ValueType valuetype);
  LVMon::MsgValueUpdate* CreateUpdateMessageForValue(ValueEntry* pvalueentry,
                                                     VecB* pvecbRet);
  void NotifyChange(ValueEntry* pvalueentry);
  void NotifyThreadProc(CThread* pthread);
  void OnPCS_Connection(IServerConnection* piserverconnection,
                        const IServerConnection::ConnectionEventArgs& args);
  void OnPCS_Read(IServerConnection* piserverconnection,
                  const IServerConnection::ReadEventArgs& args);
  void OnPCS_WriteReady(IServerConnection* piserverconnection,
                        const IServerConnection::WriteReadyEventArgs& args);
  void ProcessMessageFromClient(ClientID clientid, uint8_t* rgb, size_t cb);
  void RemoveClient(ClientID clientid);
  int SendToClient(ClientID clientid, const LVMon::MsgHeader* pmsgheader);

  void SetValue(ValueEntry* pvalueentry, MicroSeconds us, double r);
  void SetValue(ValueEntry* pvalueentry, MicroSeconds us, int64_t i64);
  void SetValue(ValueEntry* pvalueentry, MicroSeconds us, uint64_t ui64);
  void SetValue(ValueEntry* pvalueentry, MicroSeconds us,
                const std::string& str);
  void SetValue(ValueEntry* pvalueentry, MicroSeconds us, const char* sz);
  void SetValue(ValueEntry* pvalueentry, MicroSeconds us, const char* rgch,
                size_t cch);

 protected:
  std::mutex mutexClients_;  // Access guard to m_listclientid
  std::recursive_mutex
      mutexValues_;  // Access guard to umpchpvalueentry_, umstrvalueentry_,
                     // and umvalueidpvalueentry_
  std::mutex mutexValuesAdded_;    // Access guard to uspvalueentryAdded_
  std::mutex mutexValuesChanged_;  // Access guard to uspvalueentryChanged_
  IServerConnection*
      piserverconnection_;  // Server connection for clients via pipe
  CThread threadNotify_;    // Thread for handling notifications
  UMClientIDClientEntry
      umclientidcliententry_;  // Mapping from client ID to client entry for
                               // currently connected clients
  UMPChPValueEntry umpchpvalueentry_;  // Cache to quickly map value name via
                                       // string pointer to value entry
  UMStrValueEntry umstrvalueentry_;    // Map from topic name to value entry
  UMValueIDPValueEntry
      umvalueidpvalueentry_;            // Map from value ID to value entry
  USPValueEntry uspvalueentryAdded_;    // Values that have been added and
                                        // pending notification of clients
  USPValueEntry uspvalueentryChanged_;  // Values that have been changed and
                                        // pending notification of subscribers
  ValueID valueidNext_;                 // Next available value ID
};                                      // class CLVMonServer

}  // namespace LVMon

#endif  // LVMON_LIB_SRC_LVMONSERVER_H_
