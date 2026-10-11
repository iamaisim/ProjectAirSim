// Copyright (C) Microsoft Corporation.
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include <chrono>
#include <future>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "core_sim/actor/robot.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "matlab_controller_api.hpp"
#include "message/control_model_input_message.hpp"
#include "message/control_model_output_message.hpp"
#include "nng/protocol/reqrep0/rep.h"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class Scene {
 public:
  static Robot MakeMatlabControllerRobot() {
    Transform origin = {{0, 0, 0}, {1, 0, 0, 0}};
    auto callback = [](const std::string&, LogLevel, const std::string&) {};
    Logger logger(callback);
    return Robot("matlab_controller_test", origin, logger, TopicManager(logger),
                 "", ServiceManager(logger), StateManager(logger), "");
  }

  static void LoadRobot(Robot& robot, ConfigJson config_json) {
    robot.Load(config_json);
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;
using json = nlohmann::json;

namespace {

class MatlabController : public ::testing::Test {
 protected:
  void SetUp() override {
    ASSERT_EQ(nng_rep0_open(&peer_), 0);
    peer_open_ = true;
    ASSERT_EQ(nng_socket_set_ms(peer_, NNG_OPT_RECVTIMEO, 1000), 0);
    ASSERT_EQ(nng_socket_set_ms(peer_, NNG_OPT_SENDTIMEO, 1000), 0);
    nng_listener listener;
    ASSERT_EQ(nng_listener_create(&listener, peer_, "tcp://127.0.0.1:0"), 0);
    ASSERT_EQ(nng_listener_start(listener, 0), 0);
    int port = 0;
    ASSERT_EQ(nng_listener_get_int(listener, NNG_OPT_TCP_BOUND_PORT, &port), 0);
    robot_ = projectairsim::Scene::MakeMatlabControllerRobot();
    json config{
        {"physics-type", "non-physics"},
        {"links", {{{"name", "Frame"}}}},
        {"controller",
         {{"type", "matlab-controller-api"},
          {"matlab-controller-api-settings",
           {{"control-connection", {{"ip", "127.0.0.1"}, {"port", port}}},
            {"actuator-order",
             {{{"id", "first"}, {"initial-value", 0.25f}},
              {{"id", "second"}, {"initial-value", 0.75f}}}}}}}}};
    projectairsim::Scene::LoadRobot(robot_, config);
    robot_.SetControlConnectionSettings("{}");
    api_ = std::make_unique<projectairsim::MatlabControllerApi>(robot_);
    kinematics_.pose.position = {42.0f, 2.0f, -3.0f};
    api_->SetKinematics(&kinematics_);
    api_->BeginUpdate();
  }

  void TearDown() override {
    if (api_) api_->EndUpdate();
    if (peer_open_) nng_close(peer_);
    if (update_.valid()) update_.get();
  }

  void StartUpdate() {
    update_ = std::async(std::launch::async, [&]() { api_->Update(); });
  }

  projectairsim::ControlModelInputMessage Receive() {
    char* data = nullptr;
    size_t size = 0;
    int status = nng_recv(peer_, &data, &size, NNG_FLAG_ALLOC);
    if (status != 0) throw std::runtime_error(nng_strerror(status));
    auto free_message = [size](char* ptr) { nng_free(ptr, size); };
    std::unique_ptr<char, decltype(free_message)> message(data, free_message);
    projectairsim::ControlModelInputMessage request;
    request.Deserialize(std::string(message.get(), size));
    return request;
  }

  nng_socket peer_ = NNG_SOCKET_INITIALIZER;
  bool peer_open_ = false;
  projectairsim::Robot robot_;
  projectairsim::Kinematics kinematics_;
  std::unique_ptr<projectairsim::MatlabControllerApi> api_;
  std::future<void> update_;
};

TEST_F(MatlabController, MissingPeerUpdateStopsWithoutChangingOutputs) {
  ASSERT_EQ(nng_close(peer_), 0);
  peer_open_ = false;
  StartUpdate();
  EXPECT_EQ(update_.wait_for(std::chrono::milliseconds(50)),
            std::future_status::timeout);
  api_->EndUpdate();
  ASSERT_EQ(update_.wait_for(std::chrono::seconds(1)),
            std::future_status::ready);
  EXPECT_NO_THROW(update_.get());
  EXPECT_EQ(api_->GetControlSignals("first"), std::vector<float>({0.25f}));
  EXPECT_EQ(api_->GetControlSignals("second"), std::vector<float>({0.75f}));
}

TEST_F(MatlabController, PendingReplyStopsWithoutChangingOutputs) {
  StartUpdate();
  ASSERT_NO_THROW(Receive());
  EXPECT_EQ(update_.wait_for(std::chrono::milliseconds(20)),
            std::future_status::timeout);
  api_->EndUpdate();
  ASSERT_EQ(update_.wait_for(std::chrono::seconds(1)),
            std::future_status::ready);
  EXPECT_NO_THROW(update_.get());
  EXPECT_EQ(api_->GetControlSignals("first"), std::vector<float>({0.25f}));
  EXPECT_EQ(api_->GetControlSignals("second"), std::vector<float>({0.75f}));
}

TEST_F(MatlabController, ExchangesKinematicsAndPreservesUnspecifiedOutputs) {
  StartUpdate();
  projectairsim::ControlModelInputMessage request;
  ASSERT_NO_THROW(request = Receive());
  EXPECT_FLOAT_EQ(request.kinematics.pose_position_x, 42.0f);
  EXPECT_FLOAT_EQ(request.kinematics.pose_position_y, 2.0f);
  EXPECT_FLOAT_EQ(request.kinematics.pose_position_z, -3.0f);
  EXPECT_FLOAT_EQ(request.airspeed, 0.0f);
  EXPECT_FLOAT_EQ(request.distance, 0.0f);
  projectairsim::ControlModelOutputMessage response(request.time_stamp, {0.6f});
  auto bytes = response.Serialize();
  ASSERT_EQ(nng_send(peer_, &bytes[0], bytes.size(), 0), 0);
  ASSERT_EQ(update_.wait_for(std::chrono::seconds(1)),
            std::future_status::ready);
  EXPECT_NO_THROW(update_.get());
  std::vector<float> outputs;
  api_->GetControlSignalSnapshot(outputs);
  EXPECT_EQ(outputs, std::vector<float>({0.6f, 0.75f}));
}

}  // namespace
