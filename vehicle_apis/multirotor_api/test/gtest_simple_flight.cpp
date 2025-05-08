// Copyright (C) Microsoft Corporation. All rights reserved.

#include <memory>

#include "gtest/gtest.h"
#include "simple_flight/AirSimSimpleFlightBoard.hpp"
#include "simple_flight/AirSimSimpleFlightCommLink.hpp"
#include "simple_flight/AirSimSimpleFlightEstimator.hpp"
#include "simple_flight/AirSimSimpleFlightEstimatorFW.hpp"
#include "simple_flight/firmware/Firmware.hpp"
#include "simple_flight/firmware/Params.hpp"

namespace projectairsim = microsoft::projectairsim;

TEST(SimpleFlight, Constructor) {
  auto params = std::make_unique<simple_flight::Params>();
  auto board =
      std::make_unique<projectairsim::AirSimSimpleFlightBoard>(params.get());
  auto comm_link = std::make_unique<projectairsim::AirSimSimpleFlightCommLink>();
  auto state_estimator =
      std::make_unique<projectairsim::AirSimSimpleFlightEstimator>();
  auto state_estimator_fw =
      std::make_unique<projectairsim::AirSimSimpleFlightEstimatorFW>();
  auto firmware = std::make_unique<simple_flight::Firmware>(
      params.get(), board.get(), comm_link.get(), state_estimator.get(),
      state_estimator_fw.get());
  EXPECT_NE(firmware, nullptr);
}
