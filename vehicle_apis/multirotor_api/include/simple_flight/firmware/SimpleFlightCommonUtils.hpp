// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_SIMPLEFLIGHTCOMMONUTILS_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_SIMPLEFLIGHTCOMMONUTILS_HPP_

// Call this on a function parameter to suppress the unused paramter warning
template <class T>
inline void unused_simpleflight(T const& result) {
  static_cast<void>(result);
}

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_SIMPLEFLIGHTCOMMONUTILS_HPP_