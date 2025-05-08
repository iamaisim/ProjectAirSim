// Copyright (C) Microsoft Corporation. All rights reserved.

#include "lvmonmessage.h"

#include "lvmonprotocol.h"

namespace LVMon {

void CLVMonMessage::Clear(void) { vecbBuffer_.clear(); }

bool CLVMonMessage::Process(uint8_t **ppbReceivedInOut, uint8_t *pbReceivedMax,
                            uint8_t **ppbMessageRet, size_t *pcbMessageRet) {
  bool fGotMessageRet = false;
  uint8_t *&pbReceivedInOut = *ppbReceivedInOut;

  if (pbReceivedInOut < pbReceivedMax) {
    size_t cbReceived = pbReceivedMax - pbReceivedInOut;

    if (vecbBuffer_.empty()) {
      // No partial message received yet; data is start of a new message
      if (cbReceived < sizeof(LVMon::MsgHeader)) {
        // Message is shorter than the header--save the fragment and return
        vecbBuffer_.assign(pbReceivedInOut, pbReceivedMax);
        pbReceivedInOut = pbReceivedMax;  // Used all of the new data
      } else {
        // Get the header and the total message length
        {
          auto pmsgheader =
              reinterpret_cast<LVMon::MsgHeader *>(pbReceivedInOut);

          pmsgheader->NToH();  // Note: We're modifying the incoming buffer and
                               // whoever gets the message should not call
                               // NToH() on the header again
          cbMessage_ = pmsgheader->cbData + sizeof(*pmsgheader);
        }

        // Optimization: If we've received the whole message, return it now
        if (cbReceived >= cbMessage_) {
          *ppbMessageRet =
              pbReceivedInOut;  // Return pointer into the new data buffer and
                                // skip copying into vecbBuffer_
          *pcbMessageRet = cbMessage_;
          fGotMessageRet = true;

          pbReceivedInOut += cbMessage_;  // Advance past the message
        } else {
          // Haven't got the whole message yet, buffer what we've received so
          // far
          vecbBuffer_.assign(pbReceivedInOut, pbReceivedMax);
          pbReceivedInOut = pbReceivedMax;  // Used all of the new data
        }
      }
    } else {
      // We have a partial message so far.  Complete the header to get the
      // message size if needed.
      if (vecbBuffer_.size() < sizeof(LVMon::MsgHeader)) {
        size_t cbMissing = sizeof(LVMon::MsgHeader) - vecbBuffer_.size();

        // Got a partial message header, see if we've received the entire header
        // now
        if (cbReceived < cbMissing) {
          // Nope, add the new fragment to the buffer
          vecbBuffer_.insert(vecbBuffer_.end(), pbReceivedInOut, pbReceivedMax);
          pbReceivedInOut = pbReceivedMax;  // Used all of the new data
          cbReceived = 0;
        } else {
          LVMon::MsgHeader *pmsgheader;

          // Complete the header and get the message data size
          vecbBuffer_.insert(vecbBuffer_.end(), pbReceivedInOut,
                             pbReceivedInOut + cbMissing);
          pbReceivedInOut += cbMissing;  // Advance past end of header
          cbReceived -= cbMissing;

          pmsgheader = reinterpret_cast<LVMon::MsgHeader *>(vecbBuffer_.data());
          pmsgheader->NToH();  // Note: We're calling NToH() on the
                               // header--whoever gets this message should not
                               // call NToH() on the header again
          if (pmsgheader->cbData > 0)
            cbMessage_ =
                pmsgheader->cbData +
                sizeof(LVMon::MsgHeader);  // Get the total message length
          else {
            // The message has no data payload--return the message now
            vecbMessage_.swap(vecbBuffer_);
            vecbBuffer_.clear();
            *ppbMessageRet = vecbMessage_.data();
            *pcbMessageRet = vecbMessage_.size();
            fGotMessageRet = true;
          }
        }
      }

      // Append new data to the message data payload
      if (!fGotMessageRet && (cbReceived > 0)) {
        size_t cbMissing = cbMessage_ - vecbBuffer_.size();

        if (cbReceived < cbMissing) {
          // New data still doesn't complete the message; just append the new
          // data to the buffer
          vecbBuffer_.insert(vecbBuffer_.end(), pbReceivedInOut, pbReceivedMax);
          pbReceivedInOut = pbReceivedMax;
        } else {
          // We've got enough data to complete the message
          auto pbEnd = pbReceivedInOut + cbMissing;

          vecbBuffer_.insert(vecbBuffer_.end(), pbReceivedInOut, pbEnd);
          pbReceivedInOut = pbEnd;  // Advance past end of message

          // Return the message
          vecbMessage_.swap(vecbBuffer_);
          vecbBuffer_.clear();
          *ppbMessageRet = vecbMessage_.data();
          *pcbMessageRet = cbMessage_;
          fGotMessageRet = true;
        }
      }
    }
  }

  return (fGotMessageRet);
}

}  // namespace LVMon
