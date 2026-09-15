// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include <chrono>
#include <future>
#include <memory>
#include <thread>

#include "core_sim/actor/robot.hpp"
#include "core_sim/message/pose_stamped_message.hpp"
#include "core_sim/simulator.hpp"
#include "gtest/gtest.h"
#include "matlab_physics.hpp"
#include "test_data/physics_test_config.hpp"  // defines physics_test_config
#include "topic_manager.hpp"

namespace projectairsim = microsoft::projectairsim;

// Custom test subclass to expose protected members for testing
namespace microsoft {
namespace projectairsim {
class TestMatlabPhysicsBody : public MatlabPhysicsBody {
 public:
  // Expose parent constructors for testing
  using MatlabPhysicsBody::MatlabPhysicsBody;
  using MatlabPhysicsBody::kinematics_;
};
}  // namespace projectairsim
}  // namespace microsoft

// -----------------------------------------------------------------------------
// class MatlabPhysicsBody

TEST(MatlabPhysicsBody, Constructor) {
  // General description:
  // Ensures MatlabPhysicsBody default construction succeeds and returns a
  // valid object instance.
  // Arrange: prepare context for default constructor.
  // Act: allocate MatlabPhysicsBody with std::make_unique.
  auto body = std::make_unique<projectairsim::MatlabPhysicsBody>();
  // Assert: allocation produced a non-null pointer.
  EXPECT_NE(body, nullptr);
}

TEST(MatlabPhysicsBody, ConstructorFromRobot) {
  // General description:
  // Verifies MatlabPhysicsBody construction from Robot sets up kinematics
  // from robot state without connecting to Matlab service.
  // Arrange: load simulator with test robot.
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();
  auto actors = scene.GetActors();

  // Arrange: find robot in scene.
  projectairsim::Robot* sim_robot = nullptr;
  for (auto actor_ref : actors) {
    if (actor_ref.get().GetType() == projectairsim::ActorType::kRobot) {
      sim_robot = &dynamic_cast<projectairsim::Robot&>(actor_ref.get());
      break;
    }
  }
  EXPECT_NE(sim_robot, nullptr);

  // Act: construct MatlabPhysicsBody from robot.
  auto body = std::make_unique<projectairsim::MatlabPhysicsBody>(*sim_robot);

  // Assert: object was created successfully.
  EXPECT_NE(body, nullptr);
}

TEST(MatlabPhysicsBody, WriteRobotData) {
  // General description:
  // Verifies WriteRobotData propagates kinematics from Matlab response to the
  // robot's UpdateKinematics method, allowing response data to update the
  // simulation robot state.
  // Arrange: load simulator and create MatlabPhysicsBody from robot.
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();
  auto actors = scene.GetActors();

  projectairsim::Robot* sim_robot = nullptr;
  for (auto actor_ref : actors) {
    if (actor_ref.get().GetType() == projectairsim::ActorType::kRobot) {
      sim_robot = &dynamic_cast<projectairsim::Robot&>(actor_ref.get());
      break;
    }
  }
  EXPECT_NE(sim_robot, nullptr);

  // Arrange: construct body.
  auto body = std::make_shared<projectairsim::MatlabPhysicsBody>(*sim_robot);

  // Arrange: create new kinematics response from Matlab.
  projectairsim::Kinematics matlab_kin;
  matlab_kin.pose.position = {5.0, 6.0, 7.0};
  matlab_kin.pose.orientation =
      projectairsim::Quaternion(0.707, 0.707, 0, 0).normalized();
  matlab_kin.twist.linear = {1.0, 1.5, 2.0};
  matlab_kin.twist.angular = {0.1, 0.2, 0.3};

  // Act: write Matlab kinematics to robot via body.
  body->WriteRobotData(matlab_kin);

  // Assert: robot kinematics updated to reflect Matlab response.
  auto updated_kin = sim_robot->GetKinematics();
  EXPECT_FLOAT_EQ(updated_kin.pose.position.x(), 5.0);
  EXPECT_FLOAT_EQ(updated_kin.pose.position.y(), 6.0);
  EXPECT_FLOAT_EQ(updated_kin.pose.position.z(), 7.0);
  EXPECT_FLOAT_EQ(updated_kin.twist.linear.x(), 1.0);
  EXPECT_FLOAT_EQ(updated_kin.twist.linear.y(), 1.5);
  EXPECT_FLOAT_EQ(updated_kin.twist.linear.z(), 2.0);
}

TEST(PhysicsPublication, DestructionWaitsForCallback) {
  std::promise<void> callback_started;
  auto started = callback_started.get_future();
  std::promise<void> release_callback;
  auto release = release_callback.get_future().share();
  std::promise<void> callback_destroyed;
  auto destroyed = callback_destroyed.get_future();
  auto lifetime = std::shared_ptr<int>(new int(0), [&](int* value) {
    delete value;
    callback_destroyed.set_value();
  });

  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& robot = dynamic_cast<projectairsim::Robot&>(
      simulator.GetScene().GetActors().front().get());
  auto topic = robot.CreateTopic("teardown", projectairsim::TopicType::kPublished,
                                 0, projectairsim::MessageType::kPosestamped,
                                 nullptr);
  projectairsim::Logger logger([](const std::string&, projectairsim::LogLevel,
                                  const std::string&) {});
  auto manager = std::make_unique<projectairsim::TopicManager>(logger);
  manager->RegisterTopic(topic);
  manager->SetTopicPublishedCallbackEnabled(true);
  manager->SetCallbackTopicPublished(
      [lifetime, release, &callback_started](const std::string&,
                                           const projectairsim::MessageType&,
                                           const std::string&) {
        // Keep a local copy of the gate so the failing implementation can
        // release its callback storage without invalidating the wait itself.
        auto gate = release;
        callback_started.set_value();
        gate.wait();
      });
  lifetime.reset();

  manager->PublishTopic(
      topic, projectairsim::PoseStampedMessage(
                 0, projectairsim::Vector3::Zero(),
                 projectairsim::Quaternion::Identity()));
  const auto started_status = started.wait_for(std::chrono::seconds(5));
  if (started_status != std::future_status::ready) {
    release_callback.set_value();
    FAIL() << "Publication callback did not start";
  }

  std::promise<void> destruction_started;
  auto destroying = destruction_started.get_future();
  std::thread destroyer([&]() {
    destruction_started.set_value();
    manager.reset();
  });
  destroying.wait();

  // The callback's storage must remain alive until its worker finishes,
  // even when manager destruction runs concurrently with publication.
  EXPECT_EQ(destroyed.wait_for(std::chrono::milliseconds(100)),
            std::future_status::timeout);
  release_callback.set_value();
  destroyer.join();
  EXPECT_EQ(destroyed.wait_for(std::chrono::seconds(1)),
            std::future_status::ready);
}

TEST(MatlabPhysicsBody, ReadRobotData) {
  // General description:
  // Verifies ReadRobotData extracts current robot state for transmission to
  // Matlab service. Validates that position, orientation, velocities, and
  // accelerations are correctly packaged.
  // Arrange: load simulator and create body.
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();
  auto actors = scene.GetActors();

  projectairsim::Robot* sim_robot = nullptr;
  for (auto actor_ref : actors) {
    if (actor_ref.get().GetType() == projectairsim::ActorType::kRobot) {
      sim_robot = &dynamic_cast<projectairsim::Robot&>(actor_ref.get());
      break;
    }
  }
  EXPECT_NE(sim_robot, nullptr);

  // Arrange: construct body.
  projectairsim::MatlabPhysicsBody body(*sim_robot);

  // Act: read robot state into body for Matlab serialization.
  body.ReadRobotData();

  // Assert: body has captured robot state.
  auto body_kin = body.GetKinematics();
  EXPECT_TRUE(std::isfinite(body_kin.pose.position.x()));
  EXPECT_TRUE(std::isfinite(body_kin.pose.position.y()));
  EXPECT_TRUE(std::isfinite(body_kin.pose.position.z()));
  EXPECT_TRUE(std::isfinite(body_kin.twist.linear.x()));
  EXPECT_TRUE(std::isfinite(body_kin.twist.linear.y()));
  EXPECT_TRUE(std::isfinite(body_kin.twist.linear.z()));
}

TEST(MatlabPhysicsBody, CalculateExternalWrench) {
  // General description:
  // Verifies CalculateExternalWrench aggregates wrench from all rotor
  // actuator points and aggregates into total external wrench.
  // Arrange: load simulator and create body.
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();
  auto actors = scene.GetActors();

  projectairsim::Robot* sim_robot = nullptr;
  for (auto actor_ref : actors) {
    if (actor_ref.get().GetType() == projectairsim::ActorType::kRobot) {
      sim_robot = &dynamic_cast<projectairsim::Robot&>(actor_ref.get());
      break;
    }
  }
  EXPECT_NE(sim_robot, nullptr);

  // Act: construct body and calculate external wrench.
  auto body = std::make_shared<projectairsim::MatlabPhysicsBody>(*sim_robot);
  body->CalculateExternalWrench();

  // Assert: external wrench was computed.
  const auto& external_wrench = body->GetExternalWrench();
  EXPECT_TRUE(std::isfinite(external_wrench.force.x()));
  EXPECT_TRUE(std::isfinite(external_wrench.force.y()));
  EXPECT_TRUE(std::isfinite(external_wrench.force.z()));
  EXPECT_TRUE(std::isfinite(external_wrench.torque.x()));
  EXPECT_TRUE(std::isfinite(external_wrench.torque.y()));
  EXPECT_TRUE(std::isfinite(external_wrench.torque.z()));
}

// TODO Add integration tests:
// TEST(MatlabPhysicsBody, StepPhysicsBodyWithMatlabConnection) {
//   // Would require mocking NNG connection or live Matlab service
// }

// TODO Add error handling tests:
// TEST(MatlabPhysicsBody, ConnectToMatlabFailure) {
//   // Would require testing connection retry logic on NNG dial failure
// }

// TODO Add serialization tests:
// TEST(MatlabPhysicsBody, RobotStateSerializationFormat) {
//   // Would require testing NNG message format matches Matlab service spec
// }

// TODO Add deserialization tests:
// TEST(MatlabPhysicsBody, KinematicsDeserializationFormat) {
//   // Would require testing response parsing from Matlab service
// }

// TODO Add performance tests:
// TEST(MatlabPhysicsBody, StepPhysicsBodyLatency) {
//   // Would require measuring round-trip latency to Matlab service
// }

// -----------------------------------------------------------------------------
// class MatlabPhysicsModel

TEST(MatlabPhysicsModel, Constructor) {
  // General description:
  // Ensures MatlabPhysicsModel default construction succeeds and returns a
  // valid object instance.
  // Arrange: prepare context for default constructor.
  // Act: allocate MatlabPhysicsModel with std::make_unique.
  auto model = std::make_unique<projectairsim::MatlabPhysicsModel>();
  // Assert: allocation produced a non-null pointer.
  EXPECT_NE(model, nullptr);
}

TEST(MatlabPhysicsModel, SetWrenchesOnPhysicsBody) {
  // General description:
  // Verifies SetWrenchesOnPhysicsBody calculates external wrench on supplied
  // MatlabPhysicsBody without requiring active Matlab connection.
  // Arrange: create model.
  auto model = std::make_unique<projectairsim::MatlabPhysicsModel>();

  // Arrange: load simulator and create body.
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();
  auto actors = scene.GetActors();

  projectairsim::Robot* sim_robot = nullptr;
  for (auto actor_ref : actors) {
    if (actor_ref.get().GetType() == projectairsim::ActorType::kRobot) {
      sim_robot = &dynamic_cast<projectairsim::Robot&>(actor_ref.get());
      break;
    }
  }
  EXPECT_NE(sim_robot, nullptr);

  // Arrange: create body.
  auto body = std::make_shared<projectairsim::MatlabPhysicsBody>(*sim_robot);
  std::shared_ptr<projectairsim::BasePhysicsBody> base_body =
      std::static_pointer_cast<projectairsim::BasePhysicsBody>(body);

  // Act: dispatch wrench calculation via model.
  model->SetWrenchesOnPhysicsBody(base_body);

  // Assert: body has valid external wrench after dispatch.
  auto wrench = body->GetExternalWrench();
  EXPECT_TRUE(std::isfinite(wrench.force.x()));
  EXPECT_TRUE(std::isfinite(wrench.force.y()));
  EXPECT_TRUE(std::isfinite(wrench.force.z()));
}
