// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_UPDATEABLE_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_UPDATEABLE_HPP_

#include <stdexcept>

namespace simple_flight {

class IUpdatable {
 public:
  virtual void Reset() {
    // todo: handle multiple reset calls without update call
    reset_called_ = true;
  }
  virtual void Update() {
    // todo: handle update called without reset
    update_called_ = true;
  }

  virtual ~IUpdatable() = default;

 protected:
  void ClearResetUpdateAsserts() {
    reset_called_ = false;
    update_called_ = false;
  }

 private:
  bool reset_called_ = false;
  bool update_called_ = false;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_UPDATEABLE_HPP_