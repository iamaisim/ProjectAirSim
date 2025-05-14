// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#pragma once
#include "ASCDecl.h"

namespace microsoft {
namespace projectairsim {
namespace client {

// Objects of this type can only be deleted through the Release() method
// so that they are deleted on the same side of the DLL boundary as when
// they were created to avoid calling the wrong runtime instance and
// messing up the heap.
//
// Callers to a method that crosses the DLL boundary passes in an object
// of a subclass of RefCountedContainer.  The method on the other side of
// the boundary allocates an instance of a subclass of IRefCounted and
// sets it into the RefCountedContainer object.  The caller then interacts
// with the container as if it were the wrapped object and deletes the
// container like normal when done.
//
// Unlike containers, ISelfDelete objects must be allocated from the heap.
class IRefCounted {
 public:
  // Add a new reference to the object
  virtual void AddRef(void) = 0;

  // Release the object as it is no longer needed
  virtual void Release(void) = 0;

 protected:
  // Prevent direct deletion
  ASC_DECL virtual ~IRefCounted(void) {}
};  // class IRefCounted

// Objects of a subclass of this class wrap ISelfDelete objects so that
// code on one side of the DLL boundary can safely receive objects allocated
// on the other side yet when the objects are deleted they are deleted on
// the same side of the DLL boundary and avoid corrupting the heaps on
// either side.  Containers must never be allocated on one side of the DLL
// boundary and deleted on the other.
//
// Code typically allocate an object of a subclass of this class and pass
// a pointer or a reference to it across the DLL boundary.  The code on
// the other side allocates an object inheriting ISelfDelete and sets it
// into the SelfDeleteContainer object.  The caller then interacts with
// the container which typically just forwards calls to the ISelfDelete
// subclass object methods.  Deleting the container calls the
// ISelfDelete::Release() method which invokes the code on other side of
// the boundary to correctly delete the ISelfDelete object and return the
// memory to the originating heap.
//
// Containers can be on the stack or allocated from the heap.
class RefCountedContainer {
 public:
  ASC_DECL RefCountedContainer(IRefCounted* piref_counted = nullptr) noexcept;
  ASC_DECL RefCountedContainer(
      const RefCountedContainer& ref_counted_other) noexcept;
  ASC_DECL RefCountedContainer(
      RefCountedContainer&& ref_counted_other) noexcept;
  ASC_DECL virtual ~RefCountedContainer();

  ASC_DECL virtual void Clear(void);
  ASC_DECL virtual void SetPIRefCounted(IRefCounted* piref_counted);

  ASC_DECL virtual RefCountedContainer& operator=(
      const RefCountedContainer& ref_counted_container_other) noexcept;
  ASC_DECL virtual RefCountedContainer& operator=(
      RefCountedContainer&& ref_counted_container_other) noexcept;

 protected:
  IRefCounted* piref_counted_;  // Wrapped self-delete object
};                              // class SelfDeleteContainer

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
