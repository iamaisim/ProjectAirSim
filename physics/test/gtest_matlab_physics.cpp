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
#include "ground_collision_host.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "matlab_physics.hpp"
#include "message/physics_model_output_message.hpp"
#include "nng/protocol/reqrep0/rep.h"
#include "test_data/physics_test_config.hpp"  // defines physics_test_config
#include "topic_manager.hpp"

namespace projectairsim = microsoft::projectairsim;

// Custom test subclass to expose protected members for testing
namespace microsoft {
namespace projectairsim {
class TestMatlabPhysicsBody : public MatlabPhysicsBody {
 public:
  // Expose parent constructors for testing
  using MatlabPhysicsBody::collision_info_;
  using MatlabPhysicsBody::connection_string_;
  using MatlabPhysicsBody::kinematics_;
  using MatlabPhysicsBody::MatlabPhysicsBody;
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

TEST(MatlabPhysicsBody, RuntimeGroundContactReachesPhysicsInput) {
  for (const char* physics_type : {"fast-physics", "matlab-physics"}) {
    SCOPED_TRACE(physics_type);
    auto config = nlohmann::json::parse(physics_test_config);
    config["actors"][0]["robot-config"]["physics-type"] = physics_type;
    projectairsim::Simulator simulator;
    simulator.LoadSceneWithJSON(config.dump());
    auto& scene = simulator.GetScene();
    auto& robot =
        static_cast<projectairsim::Robot&>(scene.GetActors().front().get());
    projectairsim::GroundCollisionHost host({});
    host.LoadScene(scene);
    robot.DrainStepEvents();

    projectairsim::TestMatlabPhysicsBody body(robot);
    projectairsim::Kinematics kinematics;
    kinematics.pose.position = {0.0f, 0.0f, 0.0f};
    auto wait_for_contact = [&](bool has_collided,
                                TimeNano time_stamp) {
      const auto deadline =
          std::chrono::steady_clock::now() + std::chrono::seconds(2);
      while (std::chrono::steady_clock::now() < deadline) {
        body.ReadRobotData();
        if (body.collision_info_.has_collided == has_collided &&
            (!has_collided || body.collision_info_.time_stamp == time_stamp)) {
          return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      return false;
    };

    robot.UpdateKinematics(kinematics, 100);
    ASSERT_TRUE(wait_for_contact(true, 100));
    EXPECT_FLOAT_EQ(body.collision_info_.normal.z(), -1.0f);
    EXPECT_FLOAT_EQ(body.collision_info_.impact_point.z(), 0.0f);
    EXPECT_GT(body.collision_info_.penetration_depth, 0.0f);
    bool reported_contact = false;
    for (const auto& event : robot.DrainStepEvents()) {
      if (event.at("type") == "collision" && event.at("sim_time_ns") == 100) {
        reported_contact = true;
      }
    }
    EXPECT_TRUE(reported_contact);

    kinematics.pose.position.z() = -10.0f;
    robot.UpdateKinematics(kinematics, 200);
    ASSERT_TRUE(wait_for_contact(false, 200));

    kinematics.pose.position.z() = 0.0f;
    robot.UpdateKinematics(kinematics, 300);
    ASSERT_TRUE(wait_for_contact(true, 300));
    EXPECT_FLOAT_EQ(body.collision_info_.normal.z(), -1.0f);
    EXPECT_FLOAT_EQ(body.collision_info_.impact_point.z(), 0.0f);
    EXPECT_GT(body.collision_info_.penetration_depth, 0.0f);
  }
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

TEST(MatlabPhysicsModel, StopInterruptsUnavailablePeerConnection) {
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& robot = static_cast<projectairsim::Robot&>(
      simulator.GetScene().GetActors().front().get());
  auto body = std::make_shared<projectairsim::TestMatlabPhysicsBody>(robot);
  body->connection_string_ = "inproc://matlab_physics_unavailable_peer";
  projectairsim::MatlabPhysicsModel model;
  model.Start(body);
  auto step = std::async(std::launch::async, [&]() {
    model.StepPhysicsBody(16666667, body);
  });
  EXPECT_EQ(step.wait_for(std::chrono::milliseconds(100)),
            std::future_status::timeout);
  model.Stop(body);
  EXPECT_EQ(step.wait_for(std::chrono::seconds(1)), std::future_status::ready);
  step.get();
}

TEST(MatlabPhysicsModel, StopInterruptsPendingPhysicsReply) {
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& robot = static_cast<projectairsim::Robot&>(
      simulator.GetScene().GetActors().front().get());
  auto body = std::make_shared<projectairsim::TestMatlabPhysicsBody>(robot);
  body->connection_string_ = "inproc://matlab_physics_pending_reply";
  nng_socket peer = NNG_SOCKET_INITIALIZER;
  ASSERT_EQ(nng_rep0_open(&peer), 0);
  ASSERT_EQ(nng_setopt_ms(peer, NNG_OPT_RECVTIMEO, 1000), 0);
  ASSERT_EQ(nng_listen(peer, body->connection_string_.c_str(), nullptr, 0), 0);
  projectairsim::MatlabPhysicsModel model;
  model.Start(body);
  auto step = std::async(std::launch::async, [&]() {
    model.StepPhysicsBody(16666667, body);
  });
  char* request = nullptr;
  size_t size = 0;
  const int received = nng_recv(peer, &request, &size, NNG_FLAG_ALLOC);
  if (request != nullptr) nng_free(request, size);
  // The peer deliberately receives a request without answering it.
  model.Stop(body);
  EXPECT_EQ(step.wait_for(std::chrono::seconds(1)), std::future_status::ready);
  step.get();
  nng_close(peer);
  EXPECT_EQ(received, 0);
}

TEST(MatlabPhysicsModel, RepeatedRepliesUpdateCompleteKinematics) {
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& robot = static_cast<projectairsim::Robot&>(
      simulator.GetScene().GetActors().front().get());
  auto body = std::make_shared<projectairsim::TestMatlabPhysicsBody>(robot);
  body->connection_string_ = "inproc://matlab_physics_repeated_replies";
  nng_socket peer = NNG_SOCKET_INITIALIZER;
  ASSERT_EQ(nng_rep0_open(&peer), 0);
  ASSERT_EQ(nng_setopt_ms(peer, NNG_OPT_RECVTIMEO, 1000), 0);
  ASSERT_EQ(nng_listen(peer, body->connection_string_.c_str(), nullptr, 0), 0);
  projectairsim::MatlabPhysicsModel model;
  model.Start(body);
  for (int index = 0; index < 4; ++index) {
    auto step = std::async(std::launch::async, [&]() {
      model.StepPhysicsBody(16666667, body);
    });
    char* request = nullptr;
    size_t size = 0;
    const int received = nng_recv(peer, &request, &size, NNG_FLAG_ALLOC);
    if (request != nullptr) nng_free(request, size);
    EXPECT_EQ(received, 0);
    if (received != 0) {
      model.Stop(body);
      step.get();
      break;
    }
    if (index % 2) std::this_thread::sleep_for(std::chrono::milliseconds(20));
    projectairsim::PhysicsModelOutputMessage reply(
        0, 5.0f + index, 6.0f, -7.0f, 0, 0, 0, 1,
        1, 2, 3, .1f, .2f, .3f, .4f, .5f, .6f, .7f, .8f, .9f);
    auto bytes = reply.Serialize();
    EXPECT_EQ(nng_send(peer, bytes.data(), bytes.size(), 0), 0);
    const auto status = step.wait_for(std::chrono::seconds(1));
    EXPECT_EQ(status, std::future_status::ready);
    if (status != std::future_status::ready) model.Stop(body);
    step.get();
    const auto& kinematics = robot.GetKinematics();
    EXPECT_FLOAT_EQ(kinematics.pose.position.x(), 5.0f + index);
    EXPECT_FLOAT_EQ(kinematics.pose.position.y(), 6.0f);
    EXPECT_FLOAT_EQ(kinematics.pose.position.z(), -7.0f);
    EXPECT_FLOAT_EQ(kinematics.pose.orientation.x(), 0);
    EXPECT_FLOAT_EQ(kinematics.pose.orientation.y(), 0);
    EXPECT_FLOAT_EQ(kinematics.pose.orientation.z(), 0);
    EXPECT_FLOAT_EQ(kinematics.pose.orientation.w(), 1);
    EXPECT_FLOAT_EQ(kinematics.twist.linear.x(), 1);
    EXPECT_FLOAT_EQ(kinematics.twist.linear.y(), 2);
    EXPECT_FLOAT_EQ(kinematics.twist.linear.z(), 3);
    EXPECT_FLOAT_EQ(kinematics.twist.angular.x(), .1f);
    EXPECT_FLOAT_EQ(kinematics.twist.angular.y(), .2f);
    EXPECT_FLOAT_EQ(kinematics.twist.angular.z(), .3f);
    EXPECT_FLOAT_EQ(kinematics.accels.linear.x(), .4f);
    EXPECT_FLOAT_EQ(kinematics.accels.linear.y(), .5f);
    EXPECT_FLOAT_EQ(kinematics.accels.linear.z(), .6f);
    EXPECT_FLOAT_EQ(kinematics.accels.angular.x(), .7f);
    EXPECT_FLOAT_EQ(kinematics.accels.angular.y(), .8f);
    EXPECT_FLOAT_EQ(kinematics.accels.angular.z(), .9f);
    if (status != std::future_status::ready) break;
  }
  model.Stop(body);
  nng_close(peer);
}

TEST(MatlabPhysicsModel, DelayedReplyDoesNotCompleteThePendingStep) {
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& robot = static_cast<projectairsim::Robot&>(
      simulator.GetScene().GetActors().front().get());
  auto body = std::make_shared<projectairsim::TestMatlabPhysicsBody>(robot);
  body->connection_string_ = "inproc://matlab_physics_delayed_reply";
  nng_socket peer = NNG_SOCKET_INITIALIZER;
  ASSERT_EQ(nng_rep0_open(&peer), 0);
  ASSERT_EQ(nng_setopt_ms(peer, NNG_OPT_RECVTIMEO, 1000), 0);
  ASSERT_EQ(nng_listen(peer, body->connection_string_.c_str(), nullptr, 0), 0);
  projectairsim::MatlabPhysicsModel model;
  model.Start(body);
  auto step = std::async(std::launch::async, [&]() {
    model.StepPhysicsBody(16666667, body);
  });
  char* request = nullptr;
  size_t size = 0;
  const int received = nng_recv(peer, &request, &size, NNG_FLAG_ALLOC);
  if (request != nullptr) nng_free(request, size);
  EXPECT_EQ(received, 0);
  EXPECT_EQ(step.wait_for(std::chrono::milliseconds(5100)),
            std::future_status::timeout);
  projectairsim::PhysicsModelOutputMessage reply(
      0, 25, 0, -10, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  auto bytes = reply.Serialize();
  EXPECT_EQ(nng_send(peer, bytes.data(), bytes.size(), 0), 0);
  const auto status = step.wait_for(std::chrono::seconds(1));
  EXPECT_EQ(status, std::future_status::ready);
  if (status != std::future_status::ready) model.Stop(body);
  step.get();
  EXPECT_FLOAT_EQ(robot.GetKinematics().pose.position.x(), 25);
  model.Stop(body);
  nng_close(peer);
}
