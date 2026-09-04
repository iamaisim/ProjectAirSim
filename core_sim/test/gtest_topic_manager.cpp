// Copyright (C) Microsoft Corporation.
// Copyright (C) 2026 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include <chrono>
#include <string>

#include "gtest/gtest.h"
#include "msgpack.hpp"
#include "nng/nng.h"
#include "nng/protocol/pair0/pair.h"
#include "topic_manager.hpp"

namespace projectairsim = microsoft::projectairsim;

namespace {

struct LegacyTopicFrame {
  projectairsim::FrameType type;
  std::string topic;
  std::string body;

  MSGPACK_DEFINE(type, topic, body);
};

std::string PackLegacyTopicFrame(projectairsim::FrameType type,
                                 const std::string& topic,
                                 const std::string& body) {
  LegacyTopicFrame frame{type, topic, body};
  msgpack::sbuffer buffer;
  msgpack::pack(buffer, frame);
  return std::string(buffer.data(), buffer.size());
}

std::string PackTopicFrame(projectairsim::FrameType type,
                           const std::string& topic, const std::string& body) {
  msgpack::sbuffer buffer;
  projectairsim::detail::PackTopicFrame(buffer, type, topic, body);
  return std::string(buffer.data(), buffer.size());
}

}  // namespace

TEST(TopicManager, TopicFrameViewMatchesLegacyFrameBytes) {
  const std::string topic =
      "/Sim/SceneBasicDrone/robots/Drone1/sensors/DownCamera/scene_camera";
  const std::string body{"\x82\xa5image\xc4\x03\x01\x02\x03", 11};

  EXPECT_EQ(
      PackTopicFrame(projectairsim::FrameType::kMessage, topic, body),
      PackLegacyTopicFrame(projectairsim::FrameType::kMessage, topic, body));
}

TEST(TopicManager, PackedTopicFrameSizeMatchesWireBytes) {
  const std::string topic =
      "/Sim/SceneBasicDrone/robots/Drone1/sensors/DownCamera/scene_camera";
  const std::string body(1024, '\x7f');

  const std::string packed =
      PackTopicFrame(projectairsim::FrameType::kMessage, topic, body);

  EXPECT_EQ(projectairsim::detail::GetPackedTopicFrameSize(
                projectairsim::FrameType::kMessage, topic, body),
            packed.size());
}

TEST(TopicManager, NngTopicFrameMessageMatchesLegacyFrameBytes) {
  const std::string topic =
      "/Sim/SceneBasicDrone/robots/Drone1/sensors/DownCamera/scene_camera";
  const std::string body(1024 * 1024, '\x5a');

  nng_msg* message = nullptr;
  ASSERT_EQ(projectairsim::detail::AllocPackedTopicFrameMessage(
                &message, projectairsim::FrameType::kMessage, topic, body),
            0);
  ASSERT_NE(message, nullptr);

  const std::string packed =
      PackLegacyTopicFrame(projectairsim::FrameType::kMessage, topic, body);
  EXPECT_EQ(nng_msg_len(message), packed.size());
  EXPECT_EQ(std::string(static_cast<const char*>(nng_msg_body(message)),
                        nng_msg_len(message)),
            packed);

  nng_msg_free(message);
}

TEST(TopicManager, NngTopicFrameMessageRejectsNullOutputPointer) {
  const std::string topic = "/topic";
  const std::string body = "body";

  EXPECT_EQ(projectairsim::detail::AllocPackedTopicFrameMessage(
                nullptr, projectairsim::FrameType::kMessage, topic, body),
            NNG_EINVAL);
}

TEST(TopicManager, NngSendmsgTransfersPackedTopicFrameOnSuccess) {
  nng_socket sender = NNG_SOCKET_INITIALIZER;
  nng_socket receiver = NNG_SOCKET_INITIALIZER;
  ASSERT_EQ(nng_pair0_open(&sender), 0);
  ASSERT_EQ(nng_pair0_open(&receiver), 0);

  const char* url = "inproc://topic-manager-send-success";
  ASSERT_EQ(nng_listen(receiver, url, nullptr, 0), 0);
  ASSERT_EQ(nng_dial(sender, url, nullptr, 0), 0);

  const std::string topic = "/topic";
  const std::string body{"\xc4\x03\x01\x02\x03", 5};
  const std::string packed =
      PackLegacyTopicFrame(projectairsim::FrameType::kMessage, topic, body);

  nng_msg* message = nullptr;
  ASSERT_EQ(projectairsim::detail::AllocPackedTopicFrameMessage(
                &message, projectairsim::FrameType::kMessage, topic, body),
            0);
  ASSERT_NE(message, nullptr);
  ASSERT_EQ(nng_sendmsg(sender, message, 0), 0);
  message = nullptr;

  void* received_data = nullptr;
  size_t received_size = 0;
  ASSERT_EQ(nng_recv(receiver, &received_data, &received_size, NNG_FLAG_ALLOC),
            0);
  EXPECT_EQ(std::string(static_cast<const char*>(received_data), received_size),
            packed);
  nng_free(received_data, received_size);

  nng_close(sender);
  nng_close(receiver);
}

TEST(TopicManager, NngSendmsgFailureLeavesPackedTopicFrameAvailableForCleanup) {
  const std::string topic = "/topic";
  const std::string body = "body";

  nng_msg* message = nullptr;
  ASSERT_EQ(projectairsim::detail::AllocPackedTopicFrameMessage(
                &message, projectairsim::FrameType::kMessage, topic, body),
            0);
  ASSERT_NE(message, nullptr);

  nng_socket invalid_socket = NNG_SOCKET_INITIALIZER;
  EXPECT_NE(nng_sendmsg(invalid_socket, message, 0), 0);
  nng_msg_free(message);
}

TEST(TopicManager, BlockingReceiveAllowsBoundedStopWithoutTraffic) {
  auto logger_callback = [](const std::string&, projectairsim::LogLevel,
                            const std::string&) {};
  projectairsim::Logger logger(logger_callback);
  projectairsim::TopicManager manager(logger);
  manager.Load(nlohmann::json{{"ip", "127.0.0.1"}, {"port", 18989}});

  manager.Start();
  const auto start = std::chrono::steady_clock::now();
  manager.Stop();
  const auto elapsed = std::chrono::steady_clock::now() - start;

  EXPECT_LT(elapsed, std::chrono::seconds(1));
}
