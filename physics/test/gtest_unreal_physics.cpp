// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "gtest/gtest.h"
#include "unreal_physics.hpp"

namespace projectairsim = microsoft::projectairsim;

// -----------------------------------------------------------------------------
// class UnrealPhysicsBody

TEST(UnrealPhysicsBody, Constructor) {
  auto body = std::make_unique<projectairsim::UnrealPhysicsBody>();
  EXPECT_NE(body, nullptr);

  // Construct with a robot to initialize from
}

// TODO Add unit tests for UnrealPhysicsBody:
// TEST(UnrealPhysicsBody, InitializeUnrealPhysicsBody) {}
// TEST(UnrealPhysicsBody, CalculateExternalWrench) {}
// TEST(UnrealPhysicsBody, WriteRobotData) {}
// TEST(UnrealPhysicsBody, SetCallbackSetExternalWrench) {}

// -----------------------------------------------------------------------------
// class UnrealPhysicsModel

TEST(UnrealPhysicsModel, Constructor) {
  auto model = std::make_unique<projectairsim::UnrealPhysicsModel>();
  EXPECT_NE(model, nullptr);
}

// TODO Add unit tests for UnrealPhysicsModel
// TEST(UnrealPhysicsModel, SetWrenchesOnPhysicsBody) {}
