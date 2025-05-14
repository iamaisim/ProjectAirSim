// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include <memory>
#include <vector>

#include "base_physics.hpp"
#include "core_sim/actor.hpp"
#include "core_sim/actor/robot.hpp"
#include "core_sim/simulator.hpp"
#include "gtest/gtest.h"
#include "physics_world.hpp"
#include "test_data/physics_test_config.hpp"  // defines physics_test_config

namespace projectairsim = microsoft::projectairsim;

TEST(PhysicsWorld, Constructor) {
  auto world = std::make_unique<projectairsim::PhysicsWorld>();
  EXPECT_NE(world, nullptr);
}

TEST(PhysicsWorld, BaseWorldAddRemoveRobots) {
  projectairsim::PhysicsWorld world;
  EXPECT_EQ(world.GetPhysicsBodies().size(), 0);

  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();
  auto actors = scene.GetActors();

  for (auto actor_ref : actors) {
    if (actor_ref.get().GetType() == projectairsim::ActorType::kRobot) {
      auto& sim_robot = dynamic_cast<projectairsim::Robot&>(actor_ref.get());
      world.AddRobot(sim_robot);
      EXPECT_EQ(world.GetPhysicsBodies().size(), 1);
    }
  }

  world.RemoveAllBodiesAndModels();
  EXPECT_EQ(world.GetPhysicsBodies().size(), 0);
}

TEST(PhysicsWorld, SetSceneTickCallbacks) {
  projectairsim::PhysicsWorld world;
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();

  world.SetSceneTickCallbacks(scene);
}

// TODO TEST(PhysicsWorld, SetWrenchesOnPhysicsBodies) {}

TEST(PhysicsWorld, StepPhysicsWorld) {
  projectairsim::PhysicsWorld world;
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();
  auto actors = scene.GetActors();

  for (auto actor_ref : actors) {
    if (actor_ref.get().GetType() == projectairsim::ActorType::kRobot) {
      auto& sim_robot = dynamic_cast<projectairsim::Robot&>(actor_ref.get());
      projectairsim::Kinematics kin = sim_robot.GetKinematics();
      EXPECT_FLOAT_EQ(kin.accels.linear.z(), 0.0);

      world.AddRobot(sim_robot);
      world.StepPhysicsWorld(1.0f);

      kin = sim_robot.GetKinematics();
      EXPECT_FLOAT_EQ(kin.accels.linear.z(), projectairsim::EarthUtils::kGravity);
    }
  }
}