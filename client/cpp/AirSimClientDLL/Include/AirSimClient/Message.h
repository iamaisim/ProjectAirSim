// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#pragma once
#include <cstdint>
#include <string>

#include "ASCDecl.h"
#include "IRefCounted.h"

namespace microsoft {
namespace projectairsim {
namespace client {

// Forward declarations
class IMessageBuffer;

// Message class for passing a received message back to the callers or
// callback functions.  This class handles the details of buffer
// memory management across the DLL boundary.  The Message object is
// created by whoever wishes to receive the message and the object is
// eventually deleted by them on the same side of the boundary.  The
// Message buffer is allocated by the side returning the message and
// when the message object is deleted, the buffer is deleted by the
// side that allocated it even when the message object was deleted on
// the other side of the boundary.
class Message : protected RefCountedContainer {
 public:
  ASC_DECL Message(void) noexcept;
  ASC_DECL Message(const Message& message_other) noexcept;
  ASC_DECL Message(Message&& message_other) noexcept;
  ASC_DECL virtual ~Message();

  // Delete the attached buffer
  ASC_DECL void Clear(void);

  // Return the message data
  ASC_DECL const std::uint8_t* Data(void) const;

  // Return the number of bytes returned by Data()
  ASC_DECL size_t Cb(void) const;

  // Set the message buffer
  ASC_DECL void SetMessageBuffer(IMessageBuffer* pimessage_buffer);

  // Convert to a string
  // Make this inline so we don't have allocated objects crossing the DLL
  // boundary
  inline operator std::string(void) const {
    std::string strRet;

    if (piref_counted_ != nullptr)
      strRet.assign(reinterpret_cast<const char*>(Data()), Cb());

    return (strRet);
  }

  inline Message& operator=(const Message& message_other) noexcept {
    RefCountedContainer::operator=(message_other);
    return (*this);
  }

  inline Message& operator=(Message&& message_other) noexcept {
    RefCountedContainer::operator=(std::move(message_other));
    return (*this);
  }
};  // struct Message

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
