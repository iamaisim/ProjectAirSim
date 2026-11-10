// Copyright (C) 2025 IAMAI CONSULTING CORP
// MIT License. All rights reserved.

#include <chrono>
#include <future>

#include "core_sim/actor/robot.hpp"
#include "core_sim/clock.hpp"
#include "core_sim/simulator.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"
#include "physics_world.hpp"
#include "simple_flight_api.hpp"
#include "test_data/physics_test_config.hpp"

namespace projectairsim = microsoft::projectairsim;

namespace {
// Observe task entry without replacing real command or cancellation behavior.
class ObservedSimpleFlight : public projectairsim::SimpleFlightApi {
 public:
  using SimpleFlightApi::SimpleFlightApi;
  std::promise<void> command_started;

 protected:
  void CommandMotorPWMs(float a, float b, float c, float d) override {
    SimpleFlightApi::CommandMotorPWMs(a, b, c, d);
    if (!notified_) {
      notified_ = true;
      command_started.set_value();
    }
  }

 private:
  bool notified_ = false;
};

struct ReplySocket {
  nng_socket socket = NNG_SOCKET_INITIALIZER;
  ~ReplySocket() { nng_close(socket); }
};
}  // namespace

TEST(MatlabShutdown, SceneStopsWithActiveCommandAndPendingPhysicsReply) {
  ReplySocket peer;
  ASSERT_EQ(nng_rep0_open(&peer.socket), 0);
  ASSERT_EQ(nng_socket_set_ms(peer.socket, NNG_OPT_RECVTIMEO, 2000), 0);
  nng_listener listener;
  ASSERT_EQ(nng_listener_create(&listener, peer.socket, "tcp://127.0.0.1:0"), 0);
  ASSERT_EQ(nng_listener_start(listener, 0), 0);
  int port = 0;
  ASSERT_EQ(nng_listener_get_int(listener, NNG_OPT_TCP_BOUND_PORT, &port), 0);

  // The physics callback owner must outlive the simulator.
  projectairsim::PhysicsWorld world;
  projectairsim::Simulator simulator;
  auto config = nlohmann::json::parse(physics_test_config);
  auto& actor = config["actors"][0];
  actor["robot-config"]["physics-type"] = "matlab-physics";
  actor["robot-config"]["controller"]["airframe-setup"] = "quadrotor-x";
  actor["physics-connection"] = {{"ip", "127.0.0.1"}, {"port", port}};
  simulator.LoadSceneWithJSON(config.dump());
  auto& scene = simulator.GetScene();
  auto& robot = static_cast<projectairsim::Robot&>(scene.GetActors()[0].get());
  auto controller = std::make_unique<ObservedSimpleFlight>(
      robot, scene.GetTransformTree());
  auto* api = controller.get();
  auto command_started = api->command_started.get_future();
  robot.SetController(std::move(controller));
  world.AddRobot(robot);
  world.SetSceneTickCallbacks(scene);
  simulator.StartScene();

  char* request = nullptr;
  size_t size = 0;
  const int received = nng_recv(peer.socket, &request, &size, NNG_FLAG_ALLOC);
  if (request != nullptr) nng_free(request, size);
  EXPECT_EQ(received, 0);
  if (received != 0) {
    world.Stop();
    simulator.StopScene();
    return;
  }
  // Withhold the reply so simulation time cannot advance to finish the task.
  const auto sim_time = projectairsim::SimClock::Get()->NowSimNanos();
  auto command = std::async(std::launch::async, [&]() {
    return api->MoveByMotorPWMs(0.25f, 0.25f, 0.25f, 0.25f, 60.0f, sim_time);
  });
  const auto entered = command_started.wait_for(std::chrono::seconds(2));
  EXPECT_EQ(entered, std::future_status::ready);
  EXPECT_EQ(command.wait_for(std::chrono::milliseconds(20)),
            std::future_status::timeout);
  EXPECT_EQ(projectairsim::SimClock::Get()->NowSimNanos(), sim_time);

  auto stop = std::async(std::launch::async, [&]() { simulator.StopScene(); });
  const auto stopped = stop.wait_for(std::chrono::seconds(2));
  EXPECT_EQ(stopped, std::future_status::ready);
  // Release dependencies on failure so a regression can report without hanging.
  if (stopped != std::future_status::ready) {
    api->CancelLastTask();
    world.Stop();
  }
  stop.get();
  EXPECT_EQ(command.wait_for(std::chrono::seconds(1)), std::future_status::ready);
  EXPECT_FALSE(command.get());
}
