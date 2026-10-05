// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <chrono>
#include <future>
#include <limits>

#include "gtest/gtest.h"
#include "json.hpp"
#include "matlab_controller_api.hpp"
#include "matlab_vtol_controller_api.hpp"
#include "message/control_model_output_message.hpp"
#include "nng/protocol/reqrep0/rep.h"
#include "robot_test_utils.hpp"

namespace projectairsim = microsoft::projectairsim;
using json = nlohmann::json;
using namespace projectairsim;

namespace {

class ControllerCommands : public MatlabVTOLControllerApi {
 public:
  using MatlabVTOLControllerApi::CommandHeading;
  using MatlabVTOLControllerApi::CommandPosition;
  using MatlabVTOLControllerApi::CommandVelocity;
  using MatlabVTOLControllerApi::CommandVelocityBody;
  using MatlabVTOLControllerApi::CommandVelocityZ;
  using MatlabVTOLControllerApi::MatlabVTOLControllerApi;
};

class ShutdownGateController : public ControllerCommands {
 public:
  ShutdownGateController(const Robot& robot, std::promise<void>& entered,
                         std::shared_future<void> release)
      : ControllerCommands(robot, nullptr),
        entered_(entered),
        release_(release) {}

  void EndUpdate() override {
    entered_.set_value();
    release_.wait();
    ControllerCommands::EndUpdate();
  }

 private:
  std::promise<void>& entered_;
  std::shared_future<void> release_;
};

class ExternalController : public ::testing::Test {
 protected:
  void SetUp() override {
    SimClock::Get(std::make_shared<RealTimeClock>());
    ASSERT_EQ(nng_rep0_open(&socket_), 0);
    ASSERT_EQ(nng_setopt_ms(socket_, NNG_OPT_RECVTIMEO, 1000), 0);
    ASSERT_EQ(nng_setopt_ms(socket_, NNG_OPT_SENDTIMEO, 1000), 0);
    nng_listener listener;
    ASSERT_EQ(nng_listener_create(&listener, socket_, "tcp://127.0.0.1:0"), 0);
    ASSERT_EQ(nng_listener_start(listener, 0), 0);
    ASSERT_EQ(nng_listener_getopt_int(listener, NNG_OPT_TCP_BOUND_PORT, &port_),
              0);
    robot_ = Scene::MakeRobot("external_controller_test");
    auto config = Config(20);
    Scene::LoadRobot(robot_, config);
    api_.reset(new ControllerCommands(robot_, nullptr));
    kinematics_.pose.position = Vector3(1, 2, -3);
    robot_.UpdateKinematics(kinematics_);
    api_->SetKinematics(&kinematics_);
    api_->BeginUpdate();
  }

  void TearDown() override {
    if (api_) api_->EndUpdate();
    nng_close(socket_);
    SimClock::Get(std::make_shared<RealTimeClock>());
  }

  json Config(size_t count) {
    json order = json::array();
    for (size_t i = 0; i < count; ++i) {
      order.push_back({{"id", "effector-" + std::to_string(i)},
                       {"kind", i == 0 ? "rotor" : "surface"},
                       {"units", i == 0 ? "rpm" : "rad"},
                       {"initial-value", 0.25f}});
    }
    return {
        {"physics-type", "non-physics"},
        {"links", {{{"name", "Frame"}}}},
        {"controller",
         {{"id", "external"},
          {"type", "matlab-vtol-controller-api"},
          {"matlab-vtol-controller-api-settings",
           {{"actuator-order", order},
            {"request-timeout-ms", 200},
            {"control-connection", {{"ip", "127.0.0.1"}, {"port", port_}}}}}}}};
  }

  VTOLControlModelInputMessage Receive() {
    char* data = nullptr;
    size_t size = 0;
    int rv = nng_recv(socket_, &data, &size, NNG_FLAG_ALLOC);
    if (rv != 0) throw std::runtime_error(nng_strerror(rv));
    std::string bytes(data, size);
    nng_free(data, size);
    VTOLControlModelInputMessage request;
    request.Deserialize(bytes);
    return request;
  }

  void Send(VTOLControlModelOutputMessage response) {
    auto bytes = response.Serialize();
    ASSERT_EQ(nng_send(socket_, &bytes[0], bytes.size(), 0), 0);
  }

  VTOLControlModelInputMessage Exchange() {
    auto update = std::async(std::launch::async, [&]() { api_->Update(); });
    auto request = Receive();
    VTOLControlModelOutputMessage response;
    response.time_stamp = request.state.time_stamp;
    response.ready = true;
    response.landed = false;
    response.control_values.resize(request.effectors.size());
    for (size_t i = 0; i < response.control_values.size(); ++i)
      response.control_values[i] = float(i + 1);
    Send(response);
    update.get();
    return request;
  }

  void Arm() {
    ASSERT_TRUE(api_->EnableApiControl());
    Exchange();
    ASSERT_TRUE(api_->Arm(SimClock::Get()->NowSimNanos()));
  }

  Robot robot_;
  Kinematics kinematics_;
  std::unique_ptr<ControllerCommands> api_;
  nng_socket socket_ = NNG_SOCKET_INITIALIZER;
  int port_ = 0;
};

TEST_F(ExternalController, MapsMoreThanSixteenEffectorsAndReportsModelState) {
  EXPECT_FALSE(api_->CanArm());
  EXPECT_FALSE(api_->Arm(0));
  Arm();
  auto request = Exchange();
  ASSERT_EQ(request.effectors.size(), 20);
  EXPECT_EQ(request.effectors[0].id, "effector-0");
  EXPECT_EQ(request.effectors[0].kind, "rotor");
  EXPECT_EQ(request.effectors[0].units, "rpm");
  EXPECT_EQ(request.effectors[19].id, "effector-19");
  EXPECT_EQ(request.effectors[19].units, "rad");
  EXPECT_EQ(api_->GetControlSignals("effector-19"), std::vector<float>{20});
  EXPECT_EQ(api_->GetControlSignals("effector-0"), std::vector<float>{1});
  EXPECT_EQ(api_->GetControlSignals("unknown"), std::vector<float>{0});
  EXPECT_EQ(request.state.kinematics.pose_position_z, -3);
  EXPECT_EQ(api_->GetKinematicsEstimated().pose.position.x(), 1);
  EXPECT_TRUE(api_->GetReadyState().ReadyVal);
  EXPECT_EQ(api_->GetLandedState(), LandedState::Flying);
  api_->Reset();
  EXPECT_FALSE(api_->IsApiControlEnabled());
  EXPECT_FALSE(api_->CanArm());
  EXPECT_EQ(api_->GetControlSignals("effector-19"), std::vector<float>{0.25f});
}

TEST_F(ExternalController, PublishesKinematicsSnapshotsAtControllerUpdates) {
  // Physics can advance its storage before the next controller update.
  kinematics_.pose.position.x() = 99;
  robot_.UpdateKinematics(kinematics_);
  EXPECT_EQ(api_->GetKinematicsEstimated().pose.position.x(), 1);
  auto request = Exchange();
  EXPECT_EQ(request.state.kinematics.pose_position_x, 99);
  EXPECT_EQ(api_->GetKinematicsEstimated().pose.position.x(), 99);
}

TEST_F(ExternalController, TransportsModesFramesYawAndVTOLRequest) {
  Arm();
  api_->CommandVelocity(1, -2, -0.5, true, 0.3);
  auto request = Exchange();
  EXPECT_EQ(request.setpoint.mode, ExternalControlSetpoint::Velocity);
  EXPECT_EQ(request.setpoint.frame, ExternalControlSetpoint::WorldNED);
  EXPECT_EQ(request.setpoint.y, -2);
  EXPECT_TRUE(request.setpoint.yaw_is_rate);
  EXPECT_FLOAT_EQ(request.setpoint.yaw, 0.3f);
  api_->CommandVelocityBody(2, 1, 0.5, false, 1.2);
  request = Exchange();
  EXPECT_EQ(request.setpoint.frame, ExternalControlSetpoint::BodyFRD);
  EXPECT_FALSE(request.setpoint.yaw_is_rate);
  EXPECT_FLOAT_EQ(request.setpoint.yaw, 1.2f);
  api_->CommandVelocityZ(2, 1, -6, true, 0);
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::VelocityZ);
  api_->CommandPosition(4, 5, -6, false, 0.2);
  request = Exchange();
  EXPECT_EQ(request.setpoint.mode, ExternalControlSetpoint::Position);
  EXPECT_EQ(request.setpoint.z, -6);
  ASSERT_TRUE(api_->SetVTOLMode(VTOLMode::FixedWing));
  api_->CommandHeading(0.5, 12, -0.2);
  request = Exchange();
  EXPECT_EQ(request.setpoint.mode, ExternalControlSetpoint::Heading);
  EXPECT_FLOAT_EQ(request.setpoint.x, 0.5f);
  EXPECT_EQ(request.setpoint.y, 12);
  EXPECT_EQ(request.setpoint.vtol_mode, 1);
  EXPECT_THROW(api_->CommandVelocity(std::numeric_limits<float>::quiet_NaN(), 0,
                                     0, true, 0),
               std::invalid_argument);
  EXPECT_THROW(
      api_->MoveByMotorPWMs(0, 0, 0, 0, 0.01, SimClock::Get()->NowSimNanos()),
      std::runtime_error);
}

TEST_F(ExternalController, HoverObjectiveSurvivesCompletionBeforeNextUpdate) {
  Arm();
  ASSERT_TRUE(api_->Hover(SimClock::Get()->NowSimNanos()));
  auto request = Exchange();
  EXPECT_EQ(request.setpoint.request, ExternalControlSetpoint::Hover);
  EXPECT_EQ(request.setpoint.mode, ExternalControlSetpoint::Position);
  EXPECT_EQ(request.setpoint.x, 1);
  EXPECT_EQ(request.setpoint.y, 2);
  EXPECT_EQ(request.setpoint.z, -3);
  api_->CancelLastTask();
  request = Exchange();
  EXPECT_EQ(request.setpoint.mode, ExternalControlSetpoint::Idle);
  EXPECT_EQ(request.setpoint.request, ExternalControlSetpoint::None);
}

TEST_F(ExternalController,
       BodyFrameCommandKeepsBodyValuesAndCompletesAtDuration) {
  Arm();
  kinematics_.pose.orientation =
      Quaternion(Eigen::AngleAxisf(float(M_PI / 2), Vector3::UnitZ()));
  robot_.UpdateKinematics(kinematics_);
  api_->SetKinematics(&kinematics_);
  auto command = std::async(std::launch::async, [&]() {
    return api_->MoveByVelocityBodyFrame(
        2, 0, 0, 0.08, DrivetrainType::MaxDegreeOfFreedom, false, 0.2,
        SimClock::Get()->NowSimNanos());
  });
  VTOLControlModelInputMessage request;
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
  do {
    request = Exchange();
  } while (request.setpoint.mode == ExternalControlSetpoint::Idle &&
           std::chrono::steady_clock::now() < deadline);
  EXPECT_EQ(request.setpoint.frame, ExternalControlSetpoint::BodyFRD);
  EXPECT_EQ(request.setpoint.x, 2);
  EXPECT_EQ(request.setpoint.y, 0);
  EXPECT_NEAR(request.setpoint.yaw, float(M_PI / 2) + 0.2f, 1e-6);
  EXPECT_TRUE(command.get());
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Idle);
}

TEST_F(ExternalController,
       CancellationStopsTaskAndDisarmRejectsInFlightOutput) {
  Arm();
  auto command = std::async(std::launch::async, [&]() {
    return api_->MoveByVelocity(2, 0, 0, 10, DrivetrainType::MaxDegreeOfFreedom,
                                true, 0, SimClock::Get()->NowSimNanos());
  });
  VTOLControlModelInputMessage request;
  do {
    request = Exchange();
  } while (request.setpoint.mode == ExternalControlSetpoint::Idle);
  EXPECT_TRUE(api_->CancelLastTask());
  EXPECT_FALSE(command.get());
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Idle);
  auto update = std::async(std::launch::async, [&]() { api_->Update(); });
  request = Receive();
  EXPECT_TRUE(api_->Disarm());
  VTOLControlModelOutputMessage response;
  response.time_stamp = request.state.time_stamp;
  response.ready = true;
  response.control_values.assign(20, 99);
  Send(response);
  update.get();
  EXPECT_EQ(api_->GetControlSignals("effector-19"), std::vector<float>{0.25f});
  EXPECT_FALSE(Exchange().setpoint.armed);
}

TEST_F(ExternalController, MalformedReplyFailsClosedAndShutdownIsBounded) {
  Arm();
  for (int kind = 0; kind < 3; ++kind) {
    auto update = std::async(std::launch::async, [&]() { api_->Update(); });
    auto request = Receive();
    VTOLControlModelOutputMessage response;
    response.time_stamp = request.state.time_stamp;
    response.ready = true;
    response.control_values.assign(20, 99);
    if (kind == 0) response.control_values.pop_back();
    if (kind == 1) ++response.time_stamp;
    if (kind == 2)
      response.control_values[0] = std::numeric_limits<float>::infinity();
    Send(response);
    update.get();
    EXPECT_FALSE(api_->CanArm());
    EXPECT_EQ(api_->GetControlSignals("effector-19"),
              std::vector<float>{0.25f});
    Exchange();
    ASSERT_TRUE(api_->Arm(0));
  }
  auto update = std::async(std::launch::async, [&]() { api_->Update(); });
  Receive();  // Deliberately do not reply.
  api_->EndUpdate();
  EXPECT_EQ(update.wait_for(std::chrono::seconds(1)),
            std::future_status::ready);
  update.get();
  EXPECT_FALSE(api_->GetReadyState().ReadyVal);
}

TEST_F(ExternalController, RobotShutdownDoesNotDeadlockWithPendingSnapshot) {
  api_->EndUpdate();
  api_.reset();
  std::promise<void> shutdown_entered, release_shutdown, update_entered;
  auto controller =
      std::unique_ptr<ShutdownGateController>(new ShutdownGateController(
          robot_, shutdown_entered, release_shutdown.get_future().share()));
  auto* external = controller.get();
  robot_.SetController(std::move(controller));
  robot_.BeginUpdate();
  auto shutdown = std::async(std::launch::async, [&]() { robot_.EndUpdate(); });
  shutdown_entered.get_future().wait();
  auto update = std::async(std::launch::async, [&]() {
    update_entered.set_value();
    external->Update();
  });
  update_entered.get_future().wait();
  // Robot shutdown owns its mutex until the controller callback returns.
  EXPECT_EQ(update.wait_for(std::chrono::milliseconds(50)),
            std::future_status::timeout);
  release_shutdown.set_value();
  EXPECT_EQ(shutdown.wait_for(std::chrono::seconds(1)),
            std::future_status::ready);
  EXPECT_EQ(update.wait_for(std::chrono::seconds(1)),
            std::future_status::ready);
  shutdown.get();
  update.get();
  EXPECT_FALSE(external->GetReadyState().ReadyVal);
}

TEST_F(ExternalController, RejectsDuplicateMetadataAndPreservesLegacyMapping) {
  auto config = Config(2);
  config["controller"]["matlab-vtol-controller-api-settings"]["actuator-order"]
        [1]["id"] = "effector-0";
  auto robot = Scene::MakeRobot("invalid_external_controller");
  Scene::LoadRobot(robot, config);
  EXPECT_THROW(MatlabVTOLControllerApi(robot, nullptr), std::invalid_argument);
  config["controller"]["type"] = "matlab-controller-api";
  config["controller"]["matlab-controller-api-settings"]["actuator-order"] = {
      {{"id", "a"}, {"initial-value", 0.4}},
      {{"id", "b"}, {"initial-value", -0.2}}};
  Scene::LoadRobot(robot, config);
  robot.SetControlConnectionSettings("{\"ip\":\"127.0.0.1\",\"port\":18998}");
  MatlabControllerApi legacy(robot);
  EXPECT_EQ(legacy.GetControlSignals("b"), std::vector<float>{-0.2f});
  EXPECT_EQ(legacy.GetControlSignals("a"), std::vector<float>{0.4f});
}

TEST(ExternalControllerProtocol, RejectsRepliesWithMissingStatusOrTimestamp) {
  for (bool omit_timestamp : {false, true}) {
    std::stringstream stream;
    msgpack::packer<std::stringstream> packer(stream);
    packer.pack_map(4);
    packer.pack("version");
    packer.pack(1);
    packer.pack("control_values");
    packer.pack(std::vector<float>{1});
    packer.pack("ready");
    packer.pack(true);
    if (omit_timestamp) {
      packer.pack("landed");
      packer.pack(false);
    } else {
      packer.pack("time_stamp");
      packer.pack(0);
    }
    VTOLControlModelOutputMessage output;
    EXPECT_THROW(output.Deserialize(stream.str()), std::invalid_argument);
  }
}

TEST(ExternalControllerProtocol, RejectsLegacyAndUnsupportedVersions) {
  ControlModelOutputMessage legacy;
  legacy.time_stamp = 42;
  legacy.control_values = {1, 2};
  VTOLControlModelOutputMessage output;
  EXPECT_THROW(output.Deserialize(legacy.Serialize()), std::invalid_argument);
  output.version = 2;
  output.control_values = {1};
  EXPECT_THROW(output.Deserialize(output.Serialize()), std::invalid_argument);
}

}  // namespace
