// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <atomic>
#include <chrono>
#include <future>
#include <limits>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

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

class HandoffClock : public SteppableClock {
 public:
  using SteppableClock::Step;
  using SteppableClock::SteppableClock;
};

class ControllerCommands : public MatlabVTOLControllerApi {
 public:
  using MatlabVTOLControllerApi::AfterTask;
  using MatlabVTOLControllerApi::BeforeTask;
  using MatlabVTOLControllerApi::CommandHeading;
  using MatlabVTOLControllerApi::CommandPosition;
  using MatlabVTOLControllerApi::CommandVelocity;
  using MatlabVTOLControllerApi::CommandVelocityBody;
  using MatlabVTOLControllerApi::CommandVelocityZ;
  using MatlabVTOLControllerApi::MatlabVTOLControllerApi;
};

class HookCountingController : public ControllerCommands {
 public:
  using ControllerCommands::ControllerCommands;

  int LockDepth() { return GetCancelToken().GetRecursionCount(); }

  std::atomic<int> before_tasks{0};
  std::atomic<int> after_tasks{0};

 protected:
  void BeforeTask() override {
    ++before_tasks;
    ControllerCommands::BeforeTask();
  }

  void AfterTask() override {
    ++after_tasks;
    ControllerCommands::AfterTask();
  }
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
    Listen("tcp://127.0.0.1:0");
    robot_ = Scene::MakeRobot(
        "external_controller_test",
        Logger(
            [this](const std::string& component, LogLevel level,
                   const std::string& message) {
              if (component != "MatlabVTOLControllerApi") return;
              std::lock_guard<std::mutex> lock(log_lock_);
              logs_.push_back({level, message});
            },
            LogLevel::kTrace));
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

  void Listen(const std::string& url) {
    ASSERT_EQ(nng_rep0_open(&socket_), 0);
    ASSERT_EQ(nng_setopt_ms(socket_, NNG_OPT_RECVTIMEO, 1000), 0);
    ASSERT_EQ(nng_setopt_ms(socket_, NNG_OPT_SENDTIMEO, 1000), 0);
    nng_listener listener;
    ASSERT_EQ(nng_listener_create(&listener, socket_, url.c_str()), 0);
    ASSERT_EQ(nng_listener_start(listener, 0), 0);
    ASSERT_EQ(nng_listener_getopt_int(listener, NNG_OPT_TCP_BOUND_PORT, &port_),
              0);
  }

  std::vector<std::string> Logs(LogLevel level) {
    std::lock_guard<std::mutex> lock(log_lock_);
    std::vector<std::string> messages;
    for (const auto& log : logs_)
      if (log.first == level) messages.push_back(log.second);
    return messages;
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

  void Reload(const json& config) {
    api_->EndUpdate();
    api_.reset();
    Scene::LoadRobot(robot_, config);
    Replace(new ControllerCommands(robot_, nullptr));
  }

  void Replace(ControllerCommands* api) {
    if (api_) api_->EndUpdate();
    api_.reset(api);
    robot_.UpdateKinematics(kinematics_);
    api_->SetKinematics(&kinematics_);
    api_->BeginUpdate();
  }

  void ConfigureHandoff(double seconds) {
    auto config = Config(20);
    config["controller"]["matlab-vtol-controller-api-settings"]
          ["command-handoff-grace-period"] = seconds;
    Reload(config);
  }

  void CompleteTimedCommand(const std::shared_ptr<HandoffClock>& clock,
                            uint32_t mode = ExternalControlSetpoint::Velocity,
                            bool body = false) {
    const auto start = clock->NowSimNanos();
    auto task = std::async(std::launch::async, [&]() {
      if (mode == ExternalControlSetpoint::Heading)
        return api_->MoveByHeading(0, 2, 0, 0.02, 0.1, 0, 1, start);
      if (mode == ExternalControlSetpoint::VelocityZ) {
        if (body)
          return api_->MoveByVelocityBodyFrameZ(
              2, 0, -3, 0.02, DrivetrainType::MaxDegreeOfFreedom, false, 0,
              start);
        return api_->MoveByVelocityZ(2, 0, -3, 0.02,
                                     DrivetrainType::MaxDegreeOfFreedom, false,
                                     0, start);
      }
      if (body)
        return api_->MoveByVelocityBodyFrame(
            2, 0, 0, 0.02, DrivetrainType::MaxDegreeOfFreedom, false, 0, start);
      return api_->MoveByVelocity(
          2, 0, 0, 0.02, DrivetrainType::MaxDegreeOfFreedom, false, 0, start);
    });
    VTOLControlModelInputMessage request;
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(1);
    do {
      request = Exchange();
    } while (request.setpoint.mode != mode &&
             std::chrono::steady_clock::now() < deadline);
    EXPECT_EQ(request.setpoint.mode, mode);
    clock->SetNextSimTime(start + 30'000'000);
    clock->Step();
    const auto status = task.wait_for(std::chrono::seconds(1));
    if (status != std::future_status::ready) api_->CancelLastTask();
    ASSERT_EQ(status, std::future_status::ready);
    EXPECT_TRUE(task.get());
  }

  void Arm() {
    ASSERT_TRUE(api_->EnableApiControl());
    Exchange();
    ASSERT_TRUE(api_->Arm(SimClock::Get()->NowSimNanos()));
  }

  Robot robot_;
  std::mutex log_lock_;
  std::vector<std::pair<LogLevel, std::string>> logs_;
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

TEST_F(ExternalController, IndexedOutputsAndSnapshotsPreserveEffectorOrder) {
  IController& controller = *api_;
  EXPECT_EQ(controller.GetControlSignalIndex("effector-0"), 0);
  EXPECT_EQ(controller.GetControlSignalIndex("effector-19"), 19);
  EXPECT_EQ(controller.GetControlSignalIndex("unknown"), -1);
  EXPECT_EQ(controller.GetControlSignalIndex("effector-0", 1), -1);
  std::vector<float> outputs;
  controller.GetControlSignalSnapshot(outputs);
  EXPECT_EQ(outputs, std::vector<float>(20, 0.25f));
  Arm();
  Exchange();
  controller.GetControlSignalSnapshot(outputs);
  ASSERT_EQ(outputs.size(), 20);
  EXPECT_EQ(outputs.front(), 1);
  EXPECT_EQ(outputs.back(), 20);
  EXPECT_EQ(controller.GetControlSignals(19), std::vector<float>{20});
  EXPECT_EQ(controller.GetControlSignals(-1), std::vector<float>{0});
  EXPECT_EQ(controller.GetControlSignals(20), std::vector<float>{0});
  api_->Disarm();
  controller.GetControlSignalSnapshot(outputs);
  EXPECT_EQ(outputs, std::vector<float>(20, 0.25f));
  EXPECT_EQ(controller.GetControlSignals(19), std::vector<float>{0.25f});
  api_->Reset();
  controller.GetControlSignalSnapshot(outputs);
  EXPECT_EQ(outputs, std::vector<float>(20, 0.25f));
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
       BodyFrameCommandKeepsInitialWorldVelocityWhileYawChanges) {
  auto clock = std::make_shared<HandoffClock>(0, 1'000'000'000);
  SimClock::Get(clock);
  Arm();
  kinematics_.pose.orientation =
      Quaternion(Eigen::AngleAxisf(float(M_PI / 2), Vector3::UnitZ()));
  robot_.UpdateKinematics(kinematics_);
  api_->SetKinematics(&kinematics_);
  auto command = std::async(std::launch::async, [&]() {
    return api_->MoveByVelocityBodyFrame(
        2, 0, 0, 0.02, DrivetrainType::MaxDegreeOfFreedom, false, 0.2,
        SimClock::Get()->NowSimNanos());
  });
  VTOLControlModelInputMessage request;
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
  do {
    request = Exchange();
  } while (request.setpoint.mode == ExternalControlSetpoint::Idle &&
           std::chrono::steady_clock::now() < deadline);
  EXPECT_EQ(request.setpoint.frame, ExternalControlSetpoint::WorldNED);
  EXPECT_NEAR(request.setpoint.x, 0, 1e-6);
  EXPECT_NEAR(request.setpoint.y, 2, 1e-6);
  EXPECT_NEAR(request.setpoint.yaw, float(M_PI / 2) + 0.2f, 1e-6);
  kinematics_.pose.orientation = Quaternion::Identity();
  robot_.UpdateKinematics(kinematics_);
  request = Exchange();
  EXPECT_EQ(request.setpoint.frame, ExternalControlSetpoint::WorldNED);
  EXPECT_NEAR(request.setpoint.x, 0, 1e-6);
  EXPECT_NEAR(request.setpoint.y, 2, 1e-6);
  clock->SetNextSimTime(1'030'000'000);
  clock->Step();
  if (command.wait_for(std::chrono::seconds(1)) != std::future_status::ready)
    api_->CancelLastTask();
  EXPECT_TRUE(command.get());
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Idle);
}

TEST_F(ExternalController, NonpositiveDurationMatchesStandardVelocityServices) {
  Arm();
  const auto start = SimClock::Get()->NowSimNanos();
  for (float duration : {0.0f, -1.0f}) {
    EXPECT_TRUE(api_->MoveByVelocityBodyFrame(
        2, 0, 0, duration, DrivetrainType::MaxDegreeOfFreedom, true, 0, start));
    EXPECT_FALSE(api_->MoveByVelocityBodyFrameZ(
        2, 0, -3, duration, DrivetrainType::MaxDegreeOfFreedom, true, 0,
        start));
    EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Idle);
  }
}

TEST_F(ExternalController, BodyVelocityZPreservesWorldAltitudeAndYawRate) {
  auto clock = std::make_shared<HandoffClock>(0, 1'000'000'000);
  SimClock::Get(clock);
  Arm();
  kinematics_.pose.orientation =
      Quaternion(Eigen::AngleAxisf(float(M_PI / 2), Vector3::UnitZ()));
  robot_.UpdateKinematics(kinematics_);
  api_->SetKinematics(&kinematics_);
  auto command = std::async(std::launch::async, [&]() {
    return api_->MoveByVelocityBodyFrameZ(2, 1, -6, 0.02,
                                          DrivetrainType::MaxDegreeOfFreedom,
                                          true, 0.3, clock->NowSimNanos());
  });
  VTOLControlModelInputMessage request;
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(1);
  do {
    request = Exchange();
  } while (request.setpoint.mode == ExternalControlSetpoint::Idle &&
           std::chrono::steady_clock::now() < deadline);
  EXPECT_EQ(request.setpoint.mode, ExternalControlSetpoint::VelocityZ);
  EXPECT_EQ(request.setpoint.frame, ExternalControlSetpoint::WorldNED);
  EXPECT_NEAR(request.setpoint.x, -1, 1e-6);
  EXPECT_NEAR(request.setpoint.y, 2, 1e-6);
  EXPECT_EQ(request.setpoint.z, -6);
  EXPECT_TRUE(request.setpoint.yaw_is_rate);
  EXPECT_FLOAT_EQ(request.setpoint.yaw, 0.3f);
  clock->SetNextSimTime(1'030'000'000);
  clock->Step();
  if (command.wait_for(std::chrono::seconds(1)) != std::future_status::ready)
    api_->CancelLastTask();
  EXPECT_TRUE(command.get());
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

TEST_F(ExternalController, HandoffRetainsVelocityUntilSimulatedDeadline) {
  ConfigureHandoff(0.05);
  auto clock = std::make_shared<HandoffClock>(0, 1'000'000'000);
  SimClock::Get(clock);
  Arm();
  CompleteTimedCommand(clock);
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Velocity);
  clock->SetNextSimTime(1'079'999'999);
  clock->Step();
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Velocity);
  clock->SetNextSimTime(1'080'000'000);
  clock->Step();
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Idle);
}

TEST_F(ExternalController, HandoffReplacementAndCancellationClearDeadline) {
  ConfigureHandoff(0.05);
  auto clock = std::make_shared<HandoffClock>(0, 1'000'000'000);
  SimClock::Get(clock);
  Arm();
  CompleteTimedCommand(clock);
  api_->BeforeTask();
  api_->CommandVelocity(3, 1, 0, false, 0);
  clock->SetNextSimTime(1'100'000'000);
  clock->Step();
  EXPECT_EQ(Exchange().setpoint.x, 3);
  api_->AfterTask();
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Idle);
  CompleteTimedCommand(clock, ExternalControlSetpoint::Heading);
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Heading);
  EXPECT_TRUE(api_->CancelLastTask());
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Idle);
}

TEST_F(ExternalController, HandoffDisarmAndInvalidSettingsFailClosed) {
  ConfigureHandoff(0.05);
  auto clock = std::make_shared<HandoffClock>(0, 1'000'000'000);
  SimClock::Get(clock);
  Arm();
  CompleteTimedCommand(clock, ExternalControlSetpoint::VelocityZ);
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::VelocityZ);
  EXPECT_TRUE(api_->Disarm());
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Idle);
  for (double value : {-0.1, std::numeric_limits<double>::infinity()}) {
    auto config = Config(20);
    config["controller"]["matlab-vtol-controller-api-settings"]
          ["command-handoff-grace-period"] = value;
    Robot robot = Scene::MakeRobot("invalid_handoff");
    Scene::LoadRobot(robot, config);
    EXPECT_THROW(ControllerCommands rejected(robot, nullptr),
                 std::invalid_argument);
  }
}

TEST_F(ExternalController, HandoffSupportsBodyFrameTimedRequests) {
  ConfigureHandoff(0.05);
  auto clock = std::make_shared<HandoffClock>(0, 1'000'000'000);
  SimClock::Get(clock);
  Arm();
  CompleteTimedCommand(clock, ExternalControlSetpoint::Velocity, true);
  EXPECT_EQ(Exchange().setpoint.frame, ExternalControlSetpoint::WorldNED);
  EXPECT_TRUE(api_->CancelLastTask());
  CompleteTimedCommand(clock, ExternalControlSetpoint::VelocityZ, true);
  EXPECT_EQ(Exchange().setpoint.frame, ExternalControlSetpoint::WorldNED);
}

TEST_F(ExternalController, HandoffRejectsIncompleteAndPositionTasks) {
  ConfigureHandoff(0.05);
  auto clock = std::make_shared<HandoffClock>(0, 1'000'000'000);
  SimClock::Get(clock);
  Arm();
  const auto start = SimClock::Get()->NowSimNanos();
  EXPECT_FALSE(
      api_->MoveByHeading(1, 2, 0, 0.02, 0.01, 0, 0.02, start - 30'000'000));
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Idle);
  EXPECT_FALSE(api_->MoveToPosition(
      10, 2, -3, 1, 0.02, DrivetrainType::MaxDegreeOfFreedom, false, 0, -1, 0,
      SimClock::Get()->NowSimNanos() - 30'000'000));
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Idle);
  CompleteTimedCommand(clock);
  EXPECT_TRUE(api_->MoveByVelocity(3, 0, 0, 0,
                                   DrivetrainType::MaxDegreeOfFreedom, false, 0,
                                   SimClock::Get()->NowSimNanos()));
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Idle);
  CompleteTimedCommand(clock);
  EXPECT_THROW(api_->MoveByVelocity(std::numeric_limits<float>::quiet_NaN(), 0,
                                    0, 0.02, DrivetrainType::MaxDegreeOfFreedom,
                                    false, 0, SimClock::Get()->NowSimNanos()),
               std::exception);
  EXPECT_EQ(Exchange().setpoint.mode, ExternalControlSetpoint::Idle);
}

TEST_F(ExternalController, MalformedReplyFailsClosedAndShutdownIsBounded) {
  Arm();
  std::vector<float> outputs;
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
    static_cast<IController&>(*api_).GetControlSignalSnapshot(outputs);
    EXPECT_EQ(outputs, std::vector<float>(20, 0.25f));
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

TEST_F(ExternalController, UnavailableControllerRetriesEachUpdateAndWarnsOnce) {
  Arm();
  nng_close(socket_);  // The external controller stops listening.
  api_->Update();      // Waits for the request timeout, then fails closed.
  EXPECT_FALSE(api_->CanArm());
  EXPECT_EQ(api_->GetControlSignals("effector-19"), std::vector<float>{0.25f});
  ASSERT_EQ(Logs(LogLevel::kWarning).size(), 1);
  // Each later update waits for the controller again instead of letting
  // simulation time advance without control, but does not log again.
  for (int i = 0; i < 2; ++i) {
    const auto start = std::chrono::steady_clock::now();
    api_->Update();
    EXPECT_GE(std::chrono::steady_clock::now() - start,
              std::chrono::milliseconds(100));
  }
  EXPECT_EQ(Logs(LogLevel::kWarning).size(), 1);
  Listen("tcp://127.0.0.1:" + std::to_string(port_));
  EXPECT_FALSE(Exchange().setpoint.armed);
  EXPECT_TRUE(api_->GetReadyState().ReadyVal);
  EXPECT_EQ(Logs(LogLevel::kTrace),
            std::vector<std::string>{"External controller available"});
  EXPECT_EQ(Logs(LogLevel::kWarning).size(), 1);
  ASSERT_TRUE(api_->Arm(0));
  EXPECT_EQ(api_->GetControlSignals("effector-19"), std::vector<float>{0.25f});
  Exchange();
  EXPECT_EQ(api_->GetControlSignals("effector-19"), std::vector<float>{20});
}

TEST_F(ExternalController, WarnsAboutActuatorsMissingFromActuatorOrder) {
  auto config = Config(1);
  config["actuators"] = json::array();
  for (const auto* id : {"effector-0", "unlisted-a", "unlisted-b"}) {
    config["actuators"].push_back({{"name", id},
                                   {"type", "rotor"},
                                   {"enabled", true},
                                   {"parent-link", "Frame"},
                                   {"child-link", "Frame"}});
  }
  Reload(config);
  EXPECT_EQ(Logs(LogLevel::kWarning),
            std::vector<std::string>{
                "Actuators not listed in actuator-order receive 0: unlisted-a, "
                "unlisted-b"});
}

TEST_F(ExternalController, TaskHooksRunOncePerRootTaskAndAfterPreemption) {
  auto* hooks = new HookCountingController(robot_, nullptr);
  Replace(hooks);
  Arm();
  // The controller task wraps the nested shared-implementation task.
  api_->MoveByVelocity(1, 0, 0, 0, DrivetrainType::MaxDegreeOfFreedom, false, 0,
                       SimClock::Get()->NowSimNanos());
  EXPECT_EQ(hooks->before_tasks, 1);
  EXPECT_EQ(hooks->after_tasks, 1);
  EXPECT_EQ(hooks->LockDepth(), 0);
  auto first = std::async(std::launch::async, [&]() {
    return api_->MoveByVelocity(2, 0, 0, 10, DrivetrainType::MaxDegreeOfFreedom,
                                false, 0, SimClock::Get()->NowSimNanos());
  });
  VTOLControlModelInputMessage request;
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(1);
  do {
    request = Exchange();
  } while (request.setpoint.mode == ExternalControlSetpoint::Idle &&
           std::chrono::steady_clock::now() < deadline);
  EXPECT_EQ(request.setpoint.mode, ExternalControlSetpoint::Velocity);
  // A new task cancels the running task and waits for its lock.
  api_->MoveByVelocity(1, 0, 0, 0, DrivetrainType::MaxDegreeOfFreedom, false, 0,
                       SimClock::Get()->NowSimNanos());
  EXPECT_FALSE(first.get());
  EXPECT_EQ(hooks->before_tasks, 3);
  EXPECT_EQ(hooks->after_tasks, 3);
  EXPECT_EQ(hooks->LockDepth(), 0);
  api_->MoveByVelocity(1, 0, 0, 0, DrivetrainType::MaxDegreeOfFreedom, false, 0,
                       SimClock::Get()->NowSimNanos());
  EXPECT_EQ(hooks->before_tasks, 4);
  EXPECT_EQ(hooks->after_tasks, 4);
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
