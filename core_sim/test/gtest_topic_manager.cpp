// Copyright (C) Microsoft Corporation.
// Copyright (C) 2026 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <future>
#include <string>
#include <thread>

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

namespace {

void RequireNngSuccess(int result) {
  if (result != 0) {
    std::fprintf(stderr, "NNG setup failed: %s\n", nng_strerror(result));
    std::_Exit(2);
  }
}

void CheckConnectedShutdown(bool pending_frames) {
  // A deadlock must fail in a bounded child process, not hang the test runner.
  std::thread([] {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    std::fprintf(stderr, "TopicManager shutdown timed out\n");
    std::_Exit(3);
  }).detach();

  projectairsim::Logger logger(
      [](const std::string&, projectairsim::LogLevel, const std::string&) {});
  {
    projectairsim::TopicManager manager(logger);
    const int port = pending_frames ? 18992 : 18991;
    manager.Load(nlohmann::json{{"ip", "127.0.0.1"}, {"port", port}});
    const std::string url = "tcp://127.0.0.1:" + std::to_string(port);

    for (int iteration = 0; iteration < 3; ++iteration) {
      manager.Start();
      nng_socket peer = NNG_SOCKET_INITIALIZER;
      RequireNngSuccess(nng_pair0_open(&peer));
      RequireNngSuccess(nng_socket_set_ms(peer, NNG_OPT_SENDTIMEO, 1000));
      RequireNngSuccess(nng_dial(peer, url.c_str(), nullptr, 0));

      if (pending_frames) {
        // Even an unknown topic takes manager_lock_ in RecvLoop. Queue frames
        // while stopping to exercise the receive path as well as disconnect.
        const std::string frame = PackTopicFrame(
            projectairsim::FrameType::kSubscribe, "/shutdown-test", "");
        for (int frame_index = 0; frame_index < 16; ++frame_index) {
          RequireNngSuccess(nng_send(peer, const_cast<char*>(frame.data()),
                                     frame.size(), 0));
        }
        std::promise<void> start;
        const auto ready = start.get_future().share();
        std::thread first([&] { ready.wait(); manager.Stop(); });
        std::thread second([&] { ready.wait(); manager.Stop(); });
        start.set_value();
        first.join();
        second.join();
      } else {
        manager.Stop();
      }

      // Keep the peer open until Stop returns: closing it beforehand hides
      // the disconnect callback's lock cycle. Repeated Stop must be harmless.
      manager.Stop();
      RequireNngSuccess(nng_close(peer));
    }
  }
  std::_Exit(0);
}

}  // namespace

TEST(TopicManagerDeathTest, ConnectedClientCanStopAndRestart) {
  ::testing::GTEST_FLAG(death_test_style) = "threadsafe";
  ASSERT_EXIT(CheckConnectedShutdown(false), ::testing::ExitedWithCode(0), "");
}

TEST(TopicManagerDeathTest, PendingFramesAndConcurrentStopsCanRestart) {
  ::testing::GTEST_FLAG(death_test_style) = "threadsafe";
  ASSERT_EXIT(CheckConnectedShutdown(true), ::testing::ExitedWithCode(0), "");
}
