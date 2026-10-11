// Copyright (C) Microsoft Corporation.
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include <array>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>

#include "gtest/gtest.h"
#include "matlab_control_model_wrapper.hpp"
#include "message/control_model_input_message.hpp"
#include "message/control_model_output_message.hpp"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/req.h"

namespace projectairsim = microsoft::projectairsim;

namespace {

using Manager = std::unique_ptr<void, decltype(&CleanUpRuntimeResoucesWrapper)>;

class MatlabControlWrapper : public ::testing::Test {
 protected:
  void SetUp() override {
    manager_.reset(SetupRuntimeResourcesWrapper(address_));
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
    Check(nng_send(peer_, const_cast<char*>(request.data()), request.size(), 0));
  }

  void Start(void* manager) {
    StartWrapper(manager, outputs_[0].data(), outputs_[1].data(),
                 outputs_[2].data(), outputs_[3].data(), outputs_[4].data(),
                 outputs_[5].data(), outputs_[6].data());
  }

  void Update(void* manager) {
    UpdateWrapper(manager, inputs_.data(), outputs_[0].data(),
                  outputs_[1].data(), outputs_[2].data(),
                  outputs_[3].data(), outputs_[4].data(),
                  outputs_[5].data(), outputs_[6].data());
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

  projectairsim::ControlModelInputMessage Request() {
    projectairsim::ControlModelInputMessage request;
    request.time_stamp = 123;
    return request;
  }

  static void Check(int status) {
    if (status != 0) throw std::runtime_error(nng_strerror(status));
  }

  const std::string address_ = "inproc://matlab-control-wrapper";
  Manager manager_{nullptr, &CleanUpRuntimeResoucesWrapper};
  nng_socket peer_ = NNG_SOCKET_INITIALIZER;
  bool peer_open_ = false;
  std::array<double, 16> inputs_{};
  std::array<std::array<double, 72>, 7> outputs_{};
};

TEST_F(MatlabControlWrapper, ListenerAvailableBeforeUpdate) {
  EXPECT_DOUBLE_EQ(outputs_[1][0], 0.0);
  EXPECT_DOUBLE_EQ(outputs_[5][0], 0.0);
  ASSERT_NO_THROW(OpenPeer(address_));
  EXPECT_NO_THROW(Start(manager_.get()));
}

TEST_F(MatlabControlWrapper, ExchangesPortValuesWithStockMessages) {
  ASSERT_NO_THROW(OpenPeer(address_));
  auto request = Request();
  request.kinematics.pose_position_x = 42.0f;
  request.airspeed = 12.0f;
  request.barometer.altitude = 30.0f;
  request.imu.orientation_w = 1.0f;
  request.magnetometer.z = 2.5f;
  request.distance = 6.0f;
  request.gps.longitude = -71.0;
  inputs_[0] = 0.25;
  inputs_[15] = 0.75;
  projectairsim::ControlModelOutputMessage response;
  ASSERT_NO_THROW(response.Deserialize(
      RequestReply(manager_.get(), request.Serialize())));
  EXPECT_DOUBLE_EQ(outputs_[0][0], 42.0);
  EXPECT_DOUBLE_EQ(outputs_[1][0], 12.0);
  EXPECT_DOUBLE_EQ(outputs_[2][0], 30.0);
  EXPECT_DOUBLE_EQ(outputs_[3][3], 1.0);
  EXPECT_DOUBLE_EQ(outputs_[4][2], 2.5);
  EXPECT_DOUBLE_EQ(outputs_[5][0], 6.0);
  EXPECT_DOUBLE_EQ(outputs_[6][1], -71.0);
  ASSERT_EQ(response.control_values.size(), 16);
  EXPECT_FLOAT_EQ(response.control_values[0], 0.25f);
  EXPECT_FLOAT_EQ(response.control_values[15], 0.75f);
}

TEST_F(MatlabControlWrapper, MissingPeerErrorsWithoutChangingOutputs) {
  for (auto& port : outputs_) port.fill(123.0);
  const auto start = std::chrono::steady_clock::now();
  EXPECT_THROW(Update(manager_.get()), std::runtime_error);
  EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::seconds(7));
  for (const auto& port : outputs_) {
    for (double value : port) EXPECT_DOUBLE_EQ(value, 123.0);
  }
}

TEST_F(MatlabControlWrapper, CleanupPreservesOtherListener) {
  const std::string other_address = address_ + "-other";
  Manager other(SetupRuntimeResourcesWrapper(other_address),
                &CleanUpRuntimeResoucesWrapper);
  ASSERT_NO_THROW(Start(other.get()));
  manager_.reset();
  ASSERT_NO_THROW(OpenPeer(other_address));
  EXPECT_NO_THROW(RequestReply(other.get(), Request().Serialize()));
}

TEST_F(MatlabControlWrapper, TerminationClosesSocketAndAllowsRestart) {
  TerminateWrapper(manager_.get());
  EXPECT_THROW(Update(manager_.get()), std::runtime_error);
  ASSERT_NO_THROW(Start(manager_.get()));
  ASSERT_NO_THROW(OpenPeer(address_));
  EXPECT_NO_THROW(RequestReply(manager_.get(), Request().Serialize()));
}

TEST_F(MatlabControlWrapper, CleanupPreservesLibraryAcrossNewRuntime) {
  ASSERT_NO_THROW(OpenPeer(address_));
  manager_.reset();
  manager_.reset(SetupRuntimeResourcesWrapper(address_));
  ASSERT_NO_THROW(Start(manager_.get()));
  EXPECT_NO_THROW(RequestReply(manager_.get(), Request().Serialize()));
  ASSERT_NO_THROW(OpenPeer(address_));
  EXPECT_NO_THROW(RequestReply(manager_.get(), Request().Serialize()));
}

TEST_F(MatlabControlWrapper, MalformedRequestErrorsWithoutChangingOutputs) {
  ASSERT_NO_THROW(OpenPeer(address_));
  for (auto& port : outputs_) port.fill(123.0);
  ASSERT_NO_THROW(Send(std::string(1, static_cast<char>(0xc1))));
  EXPECT_ANY_THROW(Update(manager_.get()));
  for (const auto& port : outputs_) {
    for (double value : port) EXPECT_DOUBLE_EQ(value, 123.0);
  }
}

TEST_F(MatlabControlWrapper, DuplicateListenerErrorsPreserveOriginal) {
  Manager duplicate(SetupRuntimeResourcesWrapper(address_),
                    &CleanUpRuntimeResoucesWrapper);
  EXPECT_THROW(Start(duplicate.get()), std::runtime_error);
  duplicate.reset();
  ASSERT_NO_THROW(OpenPeer(address_));
  EXPECT_NO_THROW(RequestReply(manager_.get(), Request().Serialize()));
}

}  // namespace
