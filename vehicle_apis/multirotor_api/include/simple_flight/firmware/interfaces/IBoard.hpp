// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARD_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARD_HPP_

#include "IBoardClock.hpp"
#include "IBoardInputPins.hpp"
#include "IBoardOutputPins.hpp"
#include "IBoardSensors.hpp"
#include "IUpdatable.hpp"

namespace simple_flight {

class IBoard : public IUpdatable,
               public IBoardClock,
               public IBoardInputPins,
               public IBoardOutputPins,
               public IBoardSensors {};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_INTERFACES_IBOARD_HPP_