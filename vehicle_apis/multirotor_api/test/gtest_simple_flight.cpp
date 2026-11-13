// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include <atomic>
#include <memory>
#include <thread>

#include "common/canceltoken.hpp"
#include "gtest/gtest.h"
#include "simple_flight/AirSimSimpleFlightBoard.hpp"
#include "simple_flight/AirSimSimpleFlightCommLink.hpp"
#include "simple_flight/AirSimSimpleFlightEstimator.hpp"
#include "simple_flight/AirSimSimpleFlightEstimatorFW.hpp"
#include "simple_flight/firmware/Firmware.hpp"
#include "simple_flight/firmware/Params.hpp"

namespace projectairsim = microsoft::projectairsim;

TEST(SimpleFlight, Constructor) {
  // General description:
  // Verifies constructor for SimpleFlight.
  // Arrange: prepare context for `auto params = std::make_unique<simple_flight::Params>();`.
  auto params = std::make_unique<simple_flight::Params>();
  auto board =
      std::make_unique<projectairsim::AirSimSimpleFlightBoard>(params.get());
  auto comm_link = std::make_unique<projectairsim::AirSimSimpleFlightCommLink>();
  auto state_estimator =
      std::make_unique<projectairsim::AirSimSimpleFlightEstimator>();
  auto state_estimator_fw =
      std::make_unique<projectairsim::AirSimSimpleFlightEstimatorFW>();
  auto firmware = std::make_unique<simple_flight::Firmware>(
      // Act: run `params.get(), board.get(), comm_link.get(), state_estimator.get(),`.
      params.get(), board.get(), comm_link.get(), state_estimator.get(),
      state_estimator_fw.get());
  // Assert: check result from `EXPECT_NE(firmware, nullptr);`.
  EXPECT_NE(firmware, nullptr);
}

TEST(CancelToken, ResetPreservesNestedTaskLocks) {
  projectairsim::CancelToken token;
  ASSERT_TRUE(token.Try_lock());
  token.Cancel();
  token.Reset();
  EXPECT_FALSE(token.IsCancelled());
  EXPECT_EQ(token.GetRecursionCount(), 1);
  ASSERT_TRUE(token.Try_lock());
  EXPECT_EQ(token.GetRecursionCount(), 2);
  token.Unlock();
  EXPECT_EQ(token.GetRecursionCount(), 1);
  token.Unlock();
  EXPECT_EQ(token.GetRecursionCount(), 0);
}

TEST(CancelToken, CompetingTasksObserveRootLockDepth) {
  projectairsim::CancelToken token;
  std::atomic<int> incorrect_depth{0};
  auto acquire = [&]() {
    for (int i = 0; i < 100000; ++i) {
      token.Lock();
      if (token.GetRecursionCount() != 1) ++incorrect_depth;
      token.Unlock();
    }
  };
  std::thread first(acquire);
  std::thread second(acquire);
  first.join();
  second.join();
  EXPECT_EQ(incorrect_depth.load(), 0);
  EXPECT_EQ(token.GetRecursionCount(), 0);
}
