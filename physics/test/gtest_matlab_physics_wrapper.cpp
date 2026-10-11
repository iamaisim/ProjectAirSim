// Copyright (C) Microsoft Corporation.
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include <array>
#include <chrono>
#include <future>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

#include "gtest/gtest.h"
#include "matlab_physics_model_wrapper.hpp"
#include "message/physics_model_input_message.hpp"
#include "message/physics_model_output_message.hpp"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/req.h"

namespace projectairsim = microsoft::projectairsim;

namespace {

using Manager = std::unique_ptr<void, decltype(&CleanUpRuntimeResoucesWrapper)>;

class MatlabPhysicsWrapper : public ::testing::Test {
 protected:
  void SetUp() override {
    manager_.reset(SetupRuntimeResourcesWrapper(address_, kTimeoutSec));
    Start(manager_.get());
  }

  void TearDown() override {
    if (peer_open_) nng_close(peer_);
    manager_.reset();
  }

  void OpenPeer(const std::string& address) {
    if (peer_open_) {
      nng_close(peer_);
      peer_open_ = false;
    }
    Check(nng_req0_open(&peer_));
    peer_open_ = true;
    Check(nng_socket_set_ms(peer_, NNG_OPT_RECVTIMEO, 1000));
    Check(nng_socket_set_ms(peer_, NNG_OPT_SENDTIMEO, 1000));
    Check(nng_dial(peer_, address.c_str(), nullptr, 0));
  }

  void Send(const std::string& request) {
    Check(
        nng_send(peer_, const_cast<char*>(request.data()), request.size(), 0));
  }

  void Start(void* manager) {
    StartWrapper(manager, outputs_[0].data(), outputs_[1].data(),
                 outputs_[2].data(), outputs_[3].data());
  }

  void Update(void* manager) {
    UpdateWrapper(manager, inputs_.data(), outputs_[0].data(),
                  outputs_[1].data(), outputs_[2].data(), outputs_[3].data());
  }

  std::string RequestReply(void* manager, const std::string& request) {
    Send(request);
    Update(manager);
    char* data = nullptr;
    size_t size = 0;
    Check(nng_recv(peer_, &data, &size, NNG_FLAG_ALLOC));
    auto free_message = [size](char* ptr) { nng_free(ptr, size); };
    std::unique_ptr<char, decltype(free_message)> message(data, free_message);
    return std::string(message.get(), size);
  }

  projectairsim::PhysicsModelInputMessage Request() {
    projectairsim::PhysicsModelInputMessage request;
    request.time_stamp = 123;
    return request;
  }

  static void Check(int status) {
    if (status != 0) throw std::runtime_error(nng_strerror(status));
  }

  static constexpr double kTimeoutSec = 1.0;
  const std::string address_ = "inproc://matlab-physics-wrapper";
  Manager manager_{nullptr, &CleanUpRuntimeResoucesWrapper};
  nng_socket peer_ = NNG_SOCKET_INITIALIZER;
  bool peer_open_ = false;
  std::array<double, 19> inputs_{};
  std::array<std::array<double, 72>, 4> outputs_{};
};

TEST_F(MatlabPhysicsWrapper, ListenerAvailableBeforeUpdate) {
  ASSERT_NO_THROW(OpenPeer(address_));
  EXPECT_NO_THROW(Start(manager_.get()));
}

TEST_F(MatlabPhysicsWrapper, ExchangesPortValuesWithStockMessages) {
  ASSERT_NO_THROW(OpenPeer(address_));
  auto request = Request();
  request.external_wrench_points.emplace_back(12.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                                              6.0f, 7.0f, 8.0f, 9.0f);
  request.lift_drag_control_angles = {0.25f};
  request.environment_info.air_density = 1.125f;
  request.collision_info.has_collided = true;
  inputs_[0] = 42.0;
  inputs_[6] = 1.0;
  inputs_[18] = 2.5;
  projectairsim::PhysicsModelOutputMessage response;
  ASSERT_NO_THROW(
      response.Deserialize(RequestReply(manager_.get(), request.Serialize())));
  EXPECT_DOUBLE_EQ(outputs_[0][0], 12.0);
  EXPECT_DOUBLE_EQ(outputs_[0][8], 9.0);
  EXPECT_DOUBLE_EQ(outputs_[0][9], 0.0);
  EXPECT_DOUBLE_EQ(outputs_[1][0], 0.25);
  EXPECT_DOUBLE_EQ(outputs_[1][1], 0.0);
  EXPECT_DOUBLE_EQ(outputs_[2][8], 1.125);
  EXPECT_DOUBLE_EQ(outputs_[3][0], 1.0);
  EXPECT_FLOAT_EQ(response.kinematics.pose_position_x, 42.0f);
  EXPECT_FLOAT_EQ(response.kinematics.pose_orientation_w, 1.0f);
  EXPECT_FLOAT_EQ(response.kinematics.accels_angular_z, 2.5f);
}

TEST_F(MatlabPhysicsWrapper, MissingPeerErrorsWithoutChangingOutputs) {
  manager_.reset(SetupRuntimeResourcesWrapper(address_, 0.05));
  ASSERT_NO_THROW(Start(manager_.get()));
  for (auto& port : outputs_) port.fill(123.0);
  const auto start = std::chrono::steady_clock::now();
  EXPECT_THROW(Update(manager_.get()), std::runtime_error);
  const auto elapsed = std::chrono::steady_clock::now() - start;
  EXPECT_GE(elapsed, std::chrono::milliseconds(50));
  EXPECT_LT(elapsed, std::chrono::seconds(1));
  for (const auto& port : outputs_) {
    for (double value : port) EXPECT_DOUBLE_EQ(value, 123.0);
  }
}

TEST_F(MatlabPhysicsWrapper, DefaultTimeoutWaitsUntilTermination) {
  for (double timeout_sec :
       {0.0, -1.0, std::numeric_limits<double>::infinity()}) {
    SCOPED_TRACE(timeout_sec);
    manager_.reset(SetupRuntimeResourcesWrapper(address_, timeout_sec));
    ASSERT_NO_THROW(Start(manager_.get()));
    auto update =
        std::async(std::launch::async, [this]() { Update(manager_.get()); });
    EXPECT_EQ(update.wait_for(std::chrono::milliseconds(200)),
              std::future_status::timeout);
    TerminateWrapper(manager_.get());
    ASSERT_EQ(update.wait_for(std::chrono::seconds(1)),
              std::future_status::ready);
    EXPECT_THROW(update.get(), std::runtime_error);
  }
}

TEST_F(MatlabPhysicsWrapper, CleanupPreservesOtherListener) {
  const std::string other_address = address_ + "-other";
  Manager other(SetupRuntimeResourcesWrapper(other_address, kTimeoutSec),
                &CleanUpRuntimeResoucesWrapper);
  ASSERT_NO_THROW(Start(other.get()));
  manager_.reset();
  ASSERT_NO_THROW(OpenPeer(other_address));
  EXPECT_NO_THROW(RequestReply(other.get(), Request().Serialize()));
}

TEST_F(MatlabPhysicsWrapper, TerminationClosesSocketAndAllowsRestart) {
  TerminateWrapper(manager_.get());
  EXPECT_THROW(Update(manager_.get()), std::runtime_error);
  ASSERT_NO_THROW(Start(manager_.get()));
  ASSERT_NO_THROW(OpenPeer(address_));
  EXPECT_NO_THROW(RequestReply(manager_.get(), Request().Serialize()));
}

TEST_F(MatlabPhysicsWrapper, CleanupPreservesLibraryAcrossNewRuntime) {
  ASSERT_NO_THROW(OpenPeer(address_));
  manager_.reset();
  manager_.reset(SetupRuntimeResourcesWrapper(address_, kTimeoutSec));
  ASSERT_NO_THROW(Start(manager_.get()));
  EXPECT_NO_THROW(RequestReply(manager_.get(), Request().Serialize()));
  ASSERT_NO_THROW(OpenPeer(address_));
  EXPECT_NO_THROW(RequestReply(manager_.get(), Request().Serialize()));
}

TEST_F(MatlabPhysicsWrapper, MalformedRequestErrorsWithoutChangingOutputs) {
  ASSERT_NO_THROW(OpenPeer(address_));
  for (auto& port : outputs_) port.fill(123.0);
  ASSERT_NO_THROW(Send(std::string(1, static_cast<char>(0xc1))));
  EXPECT_ANY_THROW(Update(manager_.get()));
  for (const auto& port : outputs_) {
    for (double value : port) EXPECT_DOUBLE_EQ(value, 123.0);
  }
}

TEST_F(MatlabPhysicsWrapper, DuplicateListenerErrorsPreserveOriginal) {
  Manager duplicate(SetupRuntimeResourcesWrapper(address_, kTimeoutSec),
                    &CleanUpRuntimeResoucesWrapper);
  EXPECT_THROW(Start(duplicate.get()), std::runtime_error);
  duplicate.reset();
  ASSERT_NO_THROW(OpenPeer(address_));
  EXPECT_NO_THROW(RequestReply(manager_.get(), Request().Serialize()));
}

}  // namespace
