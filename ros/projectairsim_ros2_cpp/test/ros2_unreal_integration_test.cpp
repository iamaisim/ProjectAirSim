#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "projectairsim_ros2_cpp/srv/get_clock.hpp"
#include "projectairsim_ros2_cpp/srv/get_drones.hpp"
#include "projectairsim_ros2_cpp/srv/land_group.hpp"
#include "projectairsim_ros2_cpp/srv/load_scene.hpp"
#include "projectairsim_ros2_cpp/srv/move_on_path.hpp"
#include "projectairsim_ros2_cpp/srv/takeoff_group.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rosgraph_msgs/msg/clock.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/image.hpp"

using namespace std::chrono_literals;

namespace {

constexpr const char* kServiceRoot = "/projectairsim";
constexpr const char* kRosTopicRoot = "/ProjectAirsim";

class RclcppEnvironment : public ::testing::Environment {
 public:
  void SetUp() override {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
  }

  void TearDown() override {
    if (rclcpp::ok()) {
      rclcpp::shutdown();
    }
  }
};

template <typename ServiceT>
typename ServiceT::Response::SharedPtr CallService(
    const rclcpp::Node::SharedPtr& node, const std::string& service_name,
    const typename ServiceT::Request& request, std::chrono::seconds timeout = 30s) {
  auto client = node->create_client<ServiceT>(service_name);
  if (!client->wait_for_service(timeout)) {
    ADD_FAILURE() << "Service not available: " << service_name
                  << ". Start Unreal and projectairsim_ros2_cpp_node first.";
    return nullptr;
  }

  auto future =
      client->async_send_request(std::make_shared<typename ServiceT::Request>(request));
  const auto result =
      rclcpp::spin_until_future_complete(node, future, timeout);
  if (result != rclcpp::FutureReturnCode::SUCCESS) {
    ADD_FAILURE() << "Timed out calling service: " << service_name;
    return nullptr;
  }
  return future.get();
}

bool LoadScene(const rclcpp::Node::SharedPtr& node, const std::string& scene_file) {
  projectairsim_ros2_cpp::srv::LoadScene::Request request;
  request.scene_file = scene_file;
  request.is_primary_client = true;

  for (int attempt = 0; attempt < 5; ++attempt) {
    const auto response = CallService<projectairsim_ros2_cpp::srv::LoadScene>(
        node, std::string(kServiceRoot) + "/load_scene", request, 60s);
    if (response && response->success) return true;
    std::this_thread::sleep_for(1s);
  }
  return false;
}

std::map<std::string, std::vector<std::string>> Topics(
    const rclcpp::Node::SharedPtr& node) {
  return node->get_topic_names_and_types();
}

bool HasTopicType(const rclcpp::Node::SharedPtr& node, const std::string& topic,
                  const std::string& type) {
  for (const auto& [topic_name, topic_types] : Topics(node)) {
    if (topic_name != topic) continue;
    return std::find(topic_types.begin(), topic_types.end(), type) !=
           topic_types.end();
  }
  return false;
}

bool HasTopicEndingWithType(const rclcpp::Node::SharedPtr& node,
                            const std::string& suffix, const std::string& type,
                            std::string* matched_topic = nullptr) {
  for (const auto& [topic_name, topic_types] : Topics(node)) {
    if (topic_name.size() < suffix.size() ||
        topic_name.compare(topic_name.size() - suffix.size(), suffix.size(),
                           suffix) != 0) {
      continue;
    }
    if (std::find(topic_types.begin(), topic_types.end(), type) ==
        topic_types.end()) {
      continue;
    }
    if (matched_topic) *matched_topic = topic_name;
    return true;
  }
  return false;
}

bool WaitForTopicEndingWithType(const rclcpp::Node::SharedPtr& node,
                                const std::string& suffix,
                                const std::string& type,
                                std::string* matched_topic = nullptr,
                                std::chrono::seconds timeout = 30s) {
  const auto deadline = std::chrono::steady_clock::now() + timeout;
  while (std::chrono::steady_clock::now() < deadline) {
    if (HasTopicEndingWithType(node, suffix, type, matched_topic)) return true;
    rclcpp::spin_some(node);
    std::this_thread::sleep_for(250ms);
  }
  return false;
}

template <typename MsgT>
std::vector<typename MsgT::SharedPtr> CaptureMessages(
    const rclcpp::Node::SharedPtr& node, const std::string& topic,
    size_t min_count, std::chrono::seconds timeout = 30s) {
  std::vector<typename MsgT::SharedPtr> messages;
  auto subscription = node->create_subscription<MsgT>(
      topic, rclcpp::SensorDataQoS(),
      [&messages](typename MsgT::SharedPtr msg) { messages.push_back(msg); });

  const auto deadline = std::chrono::steady_clock::now() + timeout;
  while (messages.size() < min_count && std::chrono::steady_clock::now() < deadline) {
    rclcpp::spin_some(node);
    std::this_thread::sleep_for(100ms);
  }
  return messages;
}

geometry_msgs::msg::PoseStamped::SharedPtr CapturePose(
    const rclcpp::Node::SharedPtr& node, const std::string& pose_topic) {
  const auto poses =
      CaptureMessages<geometry_msgs::msg::PoseStamped>(node, pose_topic, 1, 20s);
  if (poses.empty()) return nullptr;
  return poses.front();
}

std::vector<std::string> TopicsContaining(const rclcpp::Node::SharedPtr& node,
                                          const std::vector<std::string>& needles,
                                          const std::string& type) {
  std::vector<std::string> matches;
  for (const auto& [topic_name, topic_types] : Topics(node)) {
    const auto has_type =
        std::find(topic_types.begin(), topic_types.end(), type) != topic_types.end();
    if (!has_type) continue;

    for (const auto& needle : needles) {
      if (topic_name.find(needle) != std::string::npos) {
        matches.push_back(topic_name);
        break;
      }
    }
  }
  std::sort(matches.begin(), matches.end());
  matches.erase(std::unique(matches.begin(), matches.end()), matches.end());
  return matches;
}

std::vector<int64_t> MessageTimes(const std::vector<sensor_msgs::msg::Image::SharedPtr>& messages) {
  std::vector<int64_t> times;
  times.reserve(messages.size());
  for (const auto& msg : messages) {
    times.push_back(static_cast<int64_t>(msg->header.stamp.sec) * 1000000000LL +
                    static_cast<int64_t>(msg->header.stamp.nanosec));
  }
  return times;
}

geometry_msgs::msg::PoseStamped Waypoint(double x, double y, double z) {
  geometry_msgs::msg::PoseStamped waypoint;
  waypoint.header.frame_id = "world";
  waypoint.pose.position.x = x;
  waypoint.pose.position.y = y;
  waypoint.pose.position.z = z;
  waypoint.pose.orientation.w = 1.0;
  return waypoint;
}

class UnrealIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    node_ = rclcpp::Node::make_shared("projectairsim_ros2_cpp_unreal_test");
  }

  void TearDown() override { node_.reset(); }

  rclcpp::Node::SharedPtr node_;
};

TEST_F(UnrealIntegrationTest, BasicScenePublishesStandardTopicsAndClock) {
  ASSERT_TRUE(LoadScene(node_, "scene_basic_drone.jsonc"));

  std::string actual_pose_topic;
  ASSERT_TRUE(WaitForTopicEndingWithType(
      node_, "/robots/Drone1/actual_pose", "geometry_msgs/msg/PoseStamped",
      &actual_pose_topic, 45s));

  EXPECT_TRUE(HasTopicType(node_, "/clock", "rosgraph_msgs/msg/Clock"));
  EXPECT_TRUE(HasTopicEndingWithType(node_, "/gps", "sensor_msgs/msg/NavSatFix"));
  EXPECT_TRUE(HasTopicEndingWithType(node_, "/imu", "sensor_msgs/msg/Imu"));

  const auto clock_messages =
      CaptureMessages<rosgraph_msgs::msg::Clock>(node_, "/clock", 2, 20s);
  ASSERT_GE(clock_messages.size(), 2u);
  const auto first_time = static_cast<int64_t>(clock_messages.front()->clock.sec) *
                              1000000000LL +
                          clock_messages.front()->clock.nanosec;
  const auto last_time = static_cast<int64_t>(clock_messages.back()->clock.sec) *
                             1000000000LL +
                         clock_messages.back()->clock.nanosec;
  EXPECT_GT(last_time, first_time);

  const auto pose = CapturePose(node_, actual_pose_topic);
  ASSERT_NE(pose, nullptr);
  EXPECT_TRUE(std::isfinite(pose->pose.position.x));
  EXPECT_TRUE(std::isfinite(pose->pose.position.y));
  EXPECT_TRUE(std::isfinite(pose->pose.position.z));
}

TEST_F(UnrealIntegrationTest, DroneSensorScenePublishesImageAndCameraInfoTopics) {
  ASSERT_TRUE(LoadScene(node_, "scene_drone_sensors.jsonc"));

  const std::vector<std::string> image_needles = {
      "DownCamera/depth_camera",
      "DownCamera/scene_camera",
      "DownCamera/segmentation_camera",
  };

  std::vector<std::string> image_topics;
  const auto deadline = std::chrono::steady_clock::now() + 45s;
  while (std::chrono::steady_clock::now() < deadline) {
    image_topics =
        TopicsContaining(node_, image_needles, "sensor_msgs/msg/Image");
    if (image_topics.size() >= 3) break;
    rclcpp::spin_some(node_);
    std::this_thread::sleep_for(500ms);
  }

  ASSERT_GE(image_topics.size(), 3u);
  for (const auto& image_topic : image_topics) {
    EXPECT_TRUE(HasTopicType(node_, image_topic + "/camera_info",
                             "sensor_msgs/msg/CameraInfo"))
        << "Missing camera_info for " << image_topic;
  }

  for (const auto& image_topic : image_topics) {
    const auto messages =
        CaptureMessages<sensor_msgs::msg::Image>(node_, image_topic, 3, 45s);
    ASSERT_GE(messages.size(), 1u) << "No image messages on " << image_topic;
    EXPECT_GT(messages.front()->width, 0u);
    EXPECT_GT(messages.front()->height, 0u);
  }
}

TEST_F(UnrealIntegrationTest, TakeoffChangesActualPose) {
  ASSERT_TRUE(LoadScene(node_, "scene_basic_drone.jsonc"));

  std::string actual_pose_topic;
  ASSERT_TRUE(WaitForTopicEndingWithType(
      node_, "/robots/Drone1/actual_pose", "geometry_msgs/msg/PoseStamped",
      &actual_pose_topic, 45s));

  const auto before = CapturePose(node_, actual_pose_topic);
  ASSERT_NE(before, nullptr);
  const auto z_before = before->pose.position.z;

  projectairsim_ros2_cpp::srv::TakeoffGroup::Request request;
  request.vehicle_names = {"Drone1"};
  request.wait_on_last_task = false;
  const auto response = CallService<projectairsim_ros2_cpp::srv::TakeoffGroup>(
      node_, std::string(kServiceRoot) + "/takeoff_group", request, 60s);
  ASSERT_NE(response, nullptr);
  ASSERT_TRUE(response->success);

  std::this_thread::sleep_for(5s);
  const auto after = CapturePose(node_, actual_pose_topic);
  ASSERT_NE(after, nullptr);
  EXPECT_NE(after->pose.position.z, z_before);
  EXPECT_GT(after->pose.position.z, z_before);

  projectairsim_ros2_cpp::srv::LandGroup::Request land_request;
  land_request.vehicle_names = {"Drone1"};
  land_request.wait_on_last_task = false;
  (void)CallService<projectairsim_ros2_cpp::srv::LandGroup>(
      node_, std::string(kServiceRoot) + "/land_group", land_request, 60s);
}

TEST_F(UnrealIntegrationTest, TwoDroneSceneSupportsGroupTakeoffMoveAndLand) {
  ASSERT_TRUE(LoadScene(node_, "scene_two_drones.jsonc"));

  projectairsim_ros2_cpp::srv::GetDrones::Request drones_request;
  const auto drones_response = CallService<projectairsim_ros2_cpp::srv::GetDrones>(
      node_, std::string(kServiceRoot) + "/get_drones", drones_request, 30s);
  ASSERT_NE(drones_response, nullptr);
  ASSERT_TRUE(drones_response->success);
  const std::set<std::string> drones(drones_response->drones.begin(),
                                     drones_response->drones.end());
  EXPECT_TRUE(drones.count("Drone1"));
  EXPECT_TRUE(drones.count("Drone2"));

  projectairsim_ros2_cpp::srv::TakeoffGroup::Request takeoff_request;
  takeoff_request.vehicle_names = {"Drone1", "Drone2"};
  takeoff_request.wait_on_last_task = true;
  const auto takeoff_response =
      CallService<projectairsim_ros2_cpp::srv::TakeoffGroup>(
          node_, std::string(kServiceRoot) + "/takeoff_group", takeoff_request, 90s);
  ASSERT_NE(takeoff_response, nullptr);
  ASSERT_TRUE(takeoff_response->success);

  projectairsim_ros2_cpp::srv::MoveOnPath::Request move_request;
  move_request.path = {Waypoint(30.0, 0.0, -30.0), Waypoint(0.0, 0.0, -8.0)};
  move_request.velocity = 10.0;
  move_request.timeout_sec = 45.0F;
  move_request.drive_train_type = 0;
  move_request.yaw_is_rate = true;
  move_request.yaw = 0.0F;
  move_request.lookahead = -1.0F;
  move_request.adaptive_lookahead = 1.0F;
  move_request.wait_on_last_task = false;
  const auto move_response = CallService<projectairsim_ros2_cpp::srv::MoveOnPath>(
      node_, std::string(kServiceRoot) + "/Drone1/move_on_path", move_request, 60s);
  ASSERT_NE(move_response, nullptr);
  EXPECT_TRUE(move_response->success);

  projectairsim_ros2_cpp::srv::LandGroup::Request land_request;
  land_request.vehicle_names = {"Drone1", "Drone2"};
  land_request.wait_on_last_task = false;
  const auto land_response = CallService<projectairsim_ros2_cpp::srv::LandGroup>(
      node_, std::string(kServiceRoot) + "/land_group", land_request, 30s);
  ASSERT_NE(land_response, nullptr);
  EXPECT_TRUE(land_response->success);
}

}  // namespace

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new RclcppEnvironment());
  return RUN_ALL_TESTS();
}
