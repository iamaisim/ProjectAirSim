// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include <ProjectAirsimClient/ProjectAirsimClient.h>

#include <ProjectAirSimMessage/response_message.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "msgpack.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "projectairsim_ros2_cpp/action/move_on_path.hpp"
#include "projectairsim_ros2_cpp/msg/radar_scan.hpp"
#include "projectairsim_ros2_cpp/msg/radar_tracks.hpp"
#include "projectairsim_ros2_cpp/ros2_conversion_utils.hpp"
#include "projectairsim_ros2_cpp/srv/arm.hpp"
#include "projectairsim_ros2_cpp/srv/arm_group.hpp"
#include "projectairsim_ros2_cpp/srv/cancel_all_requests.hpp"
#include "projectairsim_ros2_cpp/srv/create_voxel_grid.hpp"
#include "projectairsim_ros2_cpp/srv/disarm.hpp"
#include "projectairsim_ros2_cpp/srv/disarm_group.hpp"
#include "projectairsim_ros2_cpp/srv/get_client_info.hpp"
#include "projectairsim_ros2_cpp/srv/get_clock.hpp"
#include "projectairsim_ros2_cpp/srv/get_drones.hpp"
#include "projectairsim_ros2_cpp/srv/get_origin_geo_point.hpp"
#include "projectairsim_ros2_cpp/srv/get_segmentation_id_by_name.hpp"
#include "projectairsim_ros2_cpp/srv/get_segmentation_id_map.hpp"
#include "projectairsim_ros2_cpp/srv/get_topic_info.hpp"
#include "projectairsim_ros2_cpp/srv/land.hpp"
#include "projectairsim_ros2_cpp/srv/land_group.hpp"
#include "projectairsim_ros2_cpp/srv/load_scene.hpp"
#include "projectairsim_ros2_cpp/srv/move_on_path.hpp"
#include "projectairsim_ros2_cpp/srv/move_to_position.hpp"
#include "projectairsim_ros2_cpp/srv/occupancy_grid.hpp"
#include "projectairsim_ros2_cpp/srv/publish.hpp"
#include "projectairsim_ros2_cpp/srv/raw_request.hpp"
#include "projectairsim_ros2_cpp/srv/reset.hpp"
#include "projectairsim_ros2_cpp/srv/set_object_material.hpp"
#include "projectairsim_ros2_cpp/srv/set_object_texture.hpp"
#include "projectairsim_ros2_cpp/srv/set_segmentation_id_by_name.hpp"
#include "projectairsim_ros2_cpp/srv/swap_object_texture.hpp"
#include "projectairsim_ros2_cpp/srv/takeoff.hpp"
#include "projectairsim_ros2_cpp/srv/takeoff_group.hpp"
#include "projectairsim_ros2_cpp/srv/unsubscribe.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rosgraph_msgs/msg/clock.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/fluid_pressure.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "sensor_msgs/msg/magnetic_field.hpp"
#include "sensor_msgs/msg/nav_sat_fix.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/point_cloud2_iterator.hpp"
#include "std_msgs/msg/header.hpp"
#include "std_msgs/msg/string.hpp"
#include "tf2_ros/transform_broadcaster.h"

namespace pasc = microsoft::projectairsim::client;
namespace mp = microsoft::projectairsim;
using nlohmann::json;
using namespace projectairsim_ros2_cpp::bridge;

namespace {

template <typename T, typename = void>
struct HasSuccessField : std::false_type {};

template <typename T>
struct HasSuccessField<T, std::void_t<decltype(std::declval<T&>().success)>>
    : std::true_type {};

template <typename T>
void SetSuccessIfPresent(T& response, bool success) {
  if constexpr (HasSuccessField<T>::value) {
    response.success = success;
  } else {
    (void)response;
    (void)success;
  }
}

std::string StatusToString(pasc::Status status) {
  char buffer[256] = {0};
  pasc::GetStatusString(status, buffer);
  return std::string(buffer);
}

std_msgs::msg::Header MakeHeader(rclcpp::Node& node,
                                 const std::string& frame_id) {
  std_msgs::msg::Header header;
  header.stamp = node.get_clock()->now();
  header.frame_id = frame_id;
  return header;
}

// Stamp from the simulator's own per-sample timestamp when the payload carries
// one, falling back to the node clock otherwise. The simulator emits
// time_stamp in nanoseconds as the first member of every sensor message map,
// for example core_sim/src/message/imu_message.cpp:39. Without this the header
// is quantised to the /clock period and consumers that drop duplicate stamps,
// discard most of the stream.
std_msgs::msg::Header MakeHeader(rclcpp::Node& node,
                                 const std::string& frame_id, const json& msg) {
    std::int64_t time_stamp = 0;
    if (msg.is_object() && msg.contains("time_stamp") &&
        JsonToInt64(msg["time_stamp"], &time_stamp) && time_stamp > 0) {
        std_msgs::msg::Header header;
        header.stamp = rclcpp::Time(time_stamp);
        header.frame_id = frame_id;
        return header;
    }
    return MakeHeader(node, frame_id);
}

}  // namespace

class ProjectAirSimROS2CppNode final : public rclcpp::Node {
 public:
  using MoveOnPathAction = projectairsim_ros2_cpp::action::MoveOnPath;
    using GoalHandleMoveOnPath =
        rclcpp_action::ServerGoalHandle<MoveOnPathAction>;

  ProjectAirSimROS2CppNode()
      : Node("projectairsim_ros2_cpp_node"),
        client_(std::make_shared<pasc::Client>()),
        world_(std::make_shared<pasc::World>()) {
    address_ = declare_parameter<std::string>("address", "127.0.0.1");
        port_topics_ = declare_parameter<int>("port_topics",
                                              pasc::Client::kPortTopicsDefault);
        port_services_ = declare_parameter<int>(
            "port_services", pasc::Client::kPortServicesDefault);
    scene_config_ = declare_parameter<std::string>("scene_config", "");
    sim_config_path_ = declare_parameter<std::string>(
        "sim_config_path", "client/python/example_user_scripts/sim_config");
        delay_after_load_sec_ =
            declare_parameter<double>("delay_after_load_sec", 2.0);
    projectairsim_topic_root_ =
        declare_parameter<std::string>("projectairsim_topic_root", "/Sim");
        ros_topic_root_ =
            declare_parameter<std::string>("ros_topic_root", "/ProjectAirsim");
    publish_unmatched_as_json_ =
        declare_parameter<bool>("publish_unmatched_as_json", false);
    publish_tf_ = declare_parameter<bool>("publish_tf", true);
        tf_world_frame_id_ =
            declare_parameter<std::string>("tf_world_frame_id", "map");
    refresh_topics_period_sec_ =
        declare_parameter<double>("refresh_topics_period_sec", 0.0);
    publish_clock_period_sec_ =
        declare_parameter<double>("publish_clock_period_sec", 0.02);
        vehicle_name_ =
            declare_parameter<std::string>("vehicle_name", "Drone1");
        service_root_ =
            declare_parameter<std::string>("service_root", "/projectairsim");

    const auto status =
        client_->Connect(address_, static_cast<unsigned int>(port_topics_),
                         static_cast<unsigned int>(port_services_));
    if (status != pasc::Status::OK) {
      throw std::runtime_error("Project AirSim connect failed: " +
                               StatusToString(status));
    }

    RCLCPP_INFO(get_logger(), "Connected to Project AirSim at %s (%d/%d)",
                address_.c_str(), port_topics_, port_services_);

    if (!scene_config_.empty()) {
      const auto scene_config_path =
          ResolveSceneConfigPath(scene_config_, sim_config_path_);
            const auto world_status =
                world_->Initialize(client_, scene_config_path, sim_config_path_,
          static_cast<float>(delay_after_load_sec_));
      if (world_status != pasc::Status::OK) {
        throw std::runtime_error("Project AirSim scene load failed: " +
                                 StatusToString(world_status));
      }
      RCLCPP_INFO(get_logger(), "Loaded Project AirSim scene %s from %s",
                  scene_config_path.c_str(), sim_config_path_.c_str());
      scene_loaded_ = true;
    } else {
      AdoptLoadedSceneIfAvailable();
    }

    topic_info_publisher_ = create_publisher<std_msgs::msg::String>(
        "/projectairsim/topic_info", rclcpp::QoS(1).transient_local());
        clock_publisher_ = create_publisher<rosgraph_msgs::msg::Clock>(
            "/clock", rclcpp::QoS(10));
    if (publish_tf_) {
            tf_broadcaster_ =
                std::make_unique<tf2_ros::TransformBroadcaster>(*this);
    }
    CreateServices(vehicle_name_);
    CreateActionServer(vehicle_name_);
    DiscoverAndSubscribeIfReady();
    StartClockTimerIfReady();
    if (refresh_topics_period_sec_ > 0.0) {
      refresh_timer_ = create_wall_timer(
          std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::duration<double>(refresh_topics_period_sec_)),
                std::bind(
                    &ProjectAirSimROS2CppNode::DiscoverAndSubscribeIfReady,
                    this));
    }
  }

  ~ProjectAirSimROS2CppNode() override {
    if (client_) {
      client_->UnsubscribeAll();
      client_->Disconnect();
    }
  }

 private:
  using TopicHandler = std::function<void(const std::string&, const json&)>;

  bool AdoptLoadedSceneIfAvailable() {
    if (scene_loaded_) return true;

    const auto status = world_->Initialize(client_);
    if (status != pasc::Status::OK) {
      if (!reported_waiting_for_scene_) {
        RCLCPP_WARN(get_logger(),
                            "No Project AirSim scene is loaded yet: %s. Topic "
                            "discovery "
                    "and /clock will start after a scene is available.",
                    StatusToString(status).c_str());
        reported_waiting_for_scene_ = true;
      }
      return false;
    }

    clock_parent_topic_ = world_->GetParentTopic();
    scene_loaded_ = true;
    reported_waiting_for_scene_ = false;
    RCLCPP_INFO(get_logger(), "Using existing Project AirSim scene %s",
                clock_parent_topic_.c_str());
    return true;
  }

  void DiscoverAndSubscribeIfReady() {
    if (!scene_loaded_ && !AdoptLoadedSceneIfAvailable()) return;
    DiscoverAndSubscribe();
    StartClockTimerIfReady();
  }

  void DiscoverAndSubscribe() {
    std::vector<std::string> topics;
    {
      std::lock_guard<std::mutex> lock(client_mutex_);
      topics = client_->GetTopicInfo();
    }
    if (!listed_projectairsim_topics_) {
            RCLCPP_INFO(get_logger(), "Project AirSim reported %zu topics",
                        topics.size());
      std_msgs::msg::String topic_info_msg;
      topic_info_msg.data = json(topics).dump();
      topic_info_publisher_->publish(topic_info_msg);
      for (const auto& topic : topics) {
                RCLCPP_INFO(get_logger(), "  Project AirSim topic: %s",
                            topic.c_str());
      }
      listed_projectairsim_topics_ = true;
    }

    for (const auto& topic : topics) {
      CacheClockParentTopic(topic);

      {
        std::lock_guard<std::mutex> lock(subscriptions_mutex_);
                if (std::find(subscribed_topics_.begin(),
                              subscribed_topics_.end(),
                              topic) != subscribed_topics_.end()) {
          continue;
        }
      }

      auto handler = CreateHandler(topic);
      if (!handler) continue;

      const auto status = client_->Subscribe(
          topic.c_str(), [this](const std::string& topic_name,
                                const std::string& payload) {
            HandleProjectAirSimMessage(topic_name, payload);
          });
      if (status != pasc::Status::OK) {
        RCLCPP_WARN(get_logger(), "Failed to subscribe to %s: %s",
                    topic.c_str(), StatusToString(status).c_str());
        continue;
      }

      {
        std::lock_guard<std::mutex> lock(subscriptions_mutex_);
                if (std::find(subscribed_topics_.begin(),
                              subscribed_topics_.end(),
                              topic) != subscribed_topics_.end()) {
          continue;
        }
        handlers_[topic] = std::move(*handler);
        subscribed_topics_.push_back(topic);
      }
            RCLCPP_INFO(get_logger(),
                        "Bridging Project AirSim topic %s -> ROS topic %s",
                  topic.c_str(), ToRosTopic(topic).c_str());
    }
  }

  std::optional<TopicHandler> CreateHandler(const std::string& topic) {
    if (EndsWith(topic, "/gps")) return CreateGpsPublisher(topic);
    if (EndsWith(topic, "/actual_pose")) return CreatePosePublisher(topic);
    if (EndsWith(topic, "/imu") || EndsWith(topic, "/imu_kinematics")) {
      return CreateImuPublisher(topic);
    }
        if (EndsWith(topic, "/barometer"))
            return CreateBarometerPublisher(topic);
        if (EndsWith(topic, "/magnetometer"))
            return CreateMagnetometerPublisher(topic);
    if (EndsWith(topic, "/lidar")) return CreateLidarPublisher(topic);
        if (EndsWith(topic, "/radar_detections"))
            return CreateRadarScanPublisher(topic);
        if (EndsWith(topic, "/radar_tracks"))
            return CreateRadarTracksPublisher(topic);
        if (EndsWith(topic, "_camera_info"))
            return CreateCameraInfoPublisher(topic);
    if (EndsWith(topic, "_camera") || EndsWith(topic, "/scene_camera") ||
            EndsWith(topic, "/depth_camera") ||
            EndsWith(topic, "/depth_planar_camera") ||
        EndsWith(topic, "/depth_vis_camera") ||
        EndsWith(topic, "/disparity_normalized_camera") ||
        EndsWith(topic, "/segmentation_camera") ||
        EndsWith(topic, "/surface_normals_camera")) {
      return CreateImagePublisher(topic);
    }
    if (publish_unmatched_as_json_) return CreateJsonPublisher(topic);
    return std::nullopt;
  }

  std::string ToRosTopic(const std::string& projectairsim_topic) const {
        if (StartsWithPathPrefix(projectairsim_topic,
                                 projectairsim_topic_root_)) {
      return ros_topic_root_ +
             projectairsim_topic.substr(projectairsim_topic_root_.size());
    }
    return projectairsim_topic;
  }

  void MaybeBroadcastTransform(const std::string& parent_frame_id,
                               const std::string& child_frame_id,
                               const json& position,
                               const json& orientation) {
    if (!tf_broadcaster_ || child_frame_id.empty()) return;

    geometry_msgs::msg::TransformStamped transform;
    transform.header.stamp = get_clock()->now();
    transform.header.frame_id = parent_frame_id;
    transform.child_frame_id = child_frame_id;
    transform.transform.translation = ToRosVector3(position);
    transform.transform.rotation = ToRosQuaternion(orientation);
    tf_broadcaster_->sendTransform(transform);
  }

  TopicHandler CreateGpsPublisher(const std::string& topic) {
    const auto ros_topic = ToRosTopic(topic);
        auto publisher = create_publisher<sensor_msgs::msg::NavSatFix>(
            ros_topic, rclcpp::SensorDataQoS());
    return [this, publisher, frame_id = TopicToFrameId(topic), topic](
               const std::string&, const json& msg) {
      sensor_msgs::msg::NavSatFix ros_msg;
            ros_msg.header = MakeHeader(*this, frame_id, msg);
      ros_msg.status.status =
          NumberOr(msg, "fix_type") >= 2.0
              ? sensor_msgs::msg::NavSatStatus::STATUS_SBAS_FIX
              : sensor_msgs::msg::NavSatStatus::STATUS_NO_FIX;
            ros_msg.status.service =
                sensor_msgs::msg::NavSatStatus::SERVICE_GPS;
      ros_msg.latitude = NumberOr(msg, "latitude");
      ros_msg.longitude = NumberOr(msg, "longitude");
      ros_msg.altitude = NumberOr(msg, "altitude");
      ros_msg.position_covariance_type =
          sensor_msgs::msg::NavSatFix::COVARIANCE_TYPE_UNKNOWN;
      publisher->publish(ros_msg);
    };
  }

  TopicHandler CreatePosePublisher(const std::string& topic) {
    const auto ros_topic = ToRosTopic(topic);
        auto publisher = create_publisher<geometry_msgs::msg::PoseStamped>(
            ros_topic, rclcpp::QoS(10));
    return [this, publisher, child_frame_id = TopicToFrameId(topic)](
               const std::string&, const json& msg) {
      geometry_msgs::msg::PoseStamped ros_msg;
            ros_msg.header = MakeHeader(*this, tf_world_frame_id_, msg);
            ros_msg.pose.position =
                ToRosPoint(msg.value("position", json::object()));
      ros_msg.pose.orientation =
          ToRosQuaternion(msg.value("orientation", json::object()));
      MaybeBroadcastTransform(tf_world_frame_id_, child_frame_id,
                              msg.value("position", json::object()),
                              msg.value("orientation", json::object()));
      publisher->publish(ros_msg);
    };
  }

  TopicHandler CreateImuPublisher(const std::string& topic) {
        const auto ros_topic =
            ReplaceSuffix(ToRosTopic(topic), "/imu_kinematics", "/imu");
        auto publisher = create_publisher<sensor_msgs::msg::Imu>(
            ros_topic, rclcpp::SensorDataQoS());
    return [this, publisher, frame_id = TopicToFrameId(topic), topic](
               const std::string&, const json& msg) {
      sensor_msgs::msg::Imu ros_msg;
            ros_msg.header = MakeHeader(*this, frame_id, msg);
      ros_msg.orientation =
          ToRosQuaternion(msg.value("orientation", json::object()));
      ros_msg.angular_velocity =
          ToRosVector3(msg.value("angular_velocity", json::array()));
      ros_msg.linear_acceleration =
          ToRosVector3(msg.value("linear_acceleration", json::array()));
      ros_msg.orientation_covariance.fill(0.0);
      ros_msg.angular_velocity_covariance.fill(0.0);
      ros_msg.linear_acceleration_covariance.fill(0.0);
      publisher->publish(ros_msg);
    };
  }

  TopicHandler CreateBarometerPublisher(const std::string& topic) {
    const auto ros_topic = ToRosTopic(topic);
    auto publisher = create_publisher<sensor_msgs::msg::FluidPressure>(
        ros_topic, rclcpp::SensorDataQoS());
    return [this, publisher, frame_id = TopicToFrameId(topic)](
               const std::string&, const json& msg) {
      sensor_msgs::msg::FluidPressure ros_msg;
            ros_msg.header = MakeHeader(*this, frame_id, msg);
      ros_msg.fluid_pressure = NumberOr(msg, "pressure");
      ros_msg.variance = 0.0;
      publisher->publish(ros_msg);
    };
  }

  TopicHandler CreateMagnetometerPublisher(const std::string& topic) {
    const auto ros_topic = ToRosTopic(topic);
    auto publisher = create_publisher<sensor_msgs::msg::MagneticField>(
        ros_topic, rclcpp::SensorDataQoS());
    return [this, publisher, frame_id = TopicToFrameId(topic)](
               const std::string&, const json& msg) {
      sensor_msgs::msg::MagneticField ros_msg;
            ros_msg.header = MakeHeader(*this, frame_id, msg);
      ros_msg.magnetic_field =
          ToRosVector3(msg.value("magnetic_field_body", json::array()));
      ros_msg.magnetic_field_covariance.fill(0.0);
            const auto covariance = ArrayNumbers(
                msg.value("magnetic_field_covariance", json::array()));
            for (size_t i = 0; i < std::min<size_t>(covariance.size(), 9);
                 ++i) {
        ros_msg.magnetic_field_covariance[i] = covariance[i];
      }
      publisher->publish(ros_msg);
    };
  }

  TopicHandler CreateLidarPublisher(const std::string& topic) {
    const auto ros_topic = ToRosTopic(topic);
    auto publisher = create_publisher<sensor_msgs::msg::PointCloud2>(
        ros_topic, rclcpp::SensorDataQoS());
    return [this, publisher, frame_id = TopicToFrameId(topic)](
               const std::string&, const json& msg) {
            const auto points =
                ArrayNumbers(msg.value("point_cloud", json::array()));
      sensor_msgs::msg::PointCloud2 cloud;
            cloud.header =
                MakeHeader(*this, msg.value("frame_id", frame_id), msg);
      cloud.height = 1;
      cloud.width = static_cast<uint32_t>(points.size() / 3);
      cloud.is_bigendian = false;
      cloud.is_dense = true;

      sensor_msgs::PointCloud2Modifier modifier(cloud);
      modifier.setPointCloud2FieldsByString(1, "xyz");
      modifier.resize(cloud.width);

      sensor_msgs::PointCloud2Iterator<float> iter_x(cloud, "x");
      sensor_msgs::PointCloud2Iterator<float> iter_y(cloud, "y");
      sensor_msgs::PointCloud2Iterator<float> iter_z(cloud, "z");
            for (size_t i = 0; i + 2 < points.size();
                 i += 3, ++iter_x, ++iter_y, ++iter_z) {
        *iter_x = static_cast<float>(points[i]);
        *iter_y = static_cast<float>(-points[i + 1]);
        *iter_z = static_cast<float>(-points[i + 2]);
      }
      publisher->publish(cloud);
    };
  }

  TopicHandler CreateRadarScanPublisher(const std::string& topic) {
    const auto ros_topic = ToRosTopic(topic);
        auto publisher =
            create_publisher<projectairsim_ros2_cpp::msg::RadarScan>(
        ros_topic, rclcpp::SensorDataQoS());
    return [this, publisher, frame_id = TopicToFrameId(topic)](
               const std::string&, const json& msg) {
      projectairsim_ros2_cpp::msg::RadarScan ros_msg;
            ros_msg.header = MakeHeader(*this, frame_id, msg);
            AppendRadarReturnsFromJson(
                msg.value("radar_detections", json::array()), &ros_msg);
      publisher->publish(ros_msg);
    };
  }

  TopicHandler CreateRadarTracksPublisher(const std::string& topic) {
    const auto ros_topic = ToRosTopic(topic);
        auto publisher =
            create_publisher<projectairsim_ros2_cpp::msg::RadarTracks>(
        ros_topic, rclcpp::SensorDataQoS());
    return [this, publisher, frame_id = TopicToFrameId(topic)](
               const std::string&, const json& msg) {
      projectairsim_ros2_cpp::msg::RadarTracks ros_msg;
            ros_msg.header = MakeHeader(*this, frame_id, msg);
            AppendRadarTracksFromJson(msg.value("radar_tracks", json::array()),
                                      &ros_msg);
      publisher->publish(ros_msg);
    };
  }

  TopicHandler CreateCameraInfoPublisher(const std::string& topic) {
        const auto image_ros_topic =
            ToRosTopic(CameraImageTopicFromInfoTopic(topic));
    const auto ros_topic = CameraInfoRosTopic(image_ros_topic);
    auto publisher = create_publisher<sensor_msgs::msg::CameraInfo>(
        ros_topic, rclcpp::SensorDataQoS());
    return [this, publisher, frame_id = CameraFrameId(topic)](
               const std::string&, const json& msg) {
      sensor_msgs::msg::CameraInfo ros_msg;
            ros_msg.header = MakeHeader(*this, frame_id, msg);
      PopulateCameraInfoFromJson(msg, &ros_msg);
      publisher->publish(ros_msg);
    };
  }

  TopicHandler CreateImagePublisher(const std::string& topic) {
    const auto ros_topic = ToRosTopic(topic);
        auto publisher = create_publisher<sensor_msgs::msg::Image>(
            ros_topic, rclcpp::SensorDataQoS());
    return [this, publisher, frame_id = CameraFrameId(topic), topic](
               const std::string&, const json& msg) {
      sensor_msgs::msg::Image ros_msg;
            ros_msg.header =
                MakeHeader(*this, msg.value("frame_id", frame_id), msg);
      if (!PopulateImagePayloadFromJson(msg, &ros_msg)) {
                RCLCPP_WARN_THROTTLE(
                    get_logger(), *get_clock(), 5000,
                    "Unsupported image encoding on %s: %s", topic.c_str(),
                             msg.value("encoding", std::string()).c_str());
        return;
      }
      if (msg.contains("position") && msg.contains("orientation")) {
                MaybeBroadcastTransform(
                    tf_world_frame_id_, ros_msg.header.frame_id,
                                msg.value("position", json::object()),
                                msg.value("orientation", json::object()));
      } else if (msg.contains("pos_x") && msg.contains("rot_w")) {
                MaybeBroadcastTransform(
                    tf_world_frame_id_, ros_msg.header.frame_id,
                                json{{"x", NumberOr(msg, "pos_x")},
                                     {"y", NumberOr(msg, "pos_y")},
                                     {"z", NumberOr(msg, "pos_z")}},
                                json{{"x", NumberOr(msg, "rot_x")},
                                     {"y", NumberOr(msg, "rot_y")},
                                     {"z", NumberOr(msg, "rot_z")},
                                     {"w", NumberOr(msg, "rot_w", 1.0)}});
      }
      publisher->publish(ros_msg);
    };
  }

  TopicHandler CreateJsonPublisher(const std::string& topic) {
    const auto ros_topic = ToRosTopic(topic);
        auto publisher =
            create_publisher<std_msgs::msg::String>(ros_topic, rclcpp::QoS(10));
    return [publisher](const std::string&, const json& msg) {
      std_msgs::msg::String ros_msg;
      ros_msg.data = msg.dump();
      publisher->publish(ros_msg);
    };
  }

    bool RequestBool(const std::string& method,
                     const json& params = json::object()) {
    std::lock_guard<std::mutex> lock(client_mutex_);
    pasc::Message message_response;
    const auto status = client_->Request(method, params, &message_response);
    if (status != pasc::Status::OK) {
      RCLCPP_ERROR(get_logger(), "%s request failed: %s", method.c_str(),
                   StatusToString(status).c_str());
      return false;
    }

    mp::ResponseMessage response;
    response.Deserialize(message_response);
    if (response.GetErrorCode() != 0) {
            RCLCPP_ERROR(get_logger(), "%s rejected by server: %s",
                         method.c_str(), response.GetResult().dump().c_str());
      return false;
    }

    const auto result = response.GetResult();
    if (result.is_boolean()) return result.get<bool>();
        if (result.is_object() && result.contains("success") &&
            result["success"].is_boolean()) {
      return result["success"].get<bool>();
    }
    return true;
  }

    bool RequestJson(const std::string& method,
                     const std::string& json_parameters, int32_t* error_code,
                     std::string* result_json, std::string* raw_response,
                     std::string* status_text) {
    json params = json::object();
    if (!json_parameters.empty()) {
      try {
        params = json::parse(json_parameters);
      } catch (const json::parse_error& error) {
        *error_code = -1;
                *status_text =
                    std::string("Invalid JSON parameters: ") + error.what();
        *result_json = json({{"error", *status_text}}).dump();
        raw_response->clear();
        return false;
      }
    }

    std::lock_guard<std::mutex> lock(client_mutex_);
    pasc::Message message_response;
    const auto status = client_->Request(method, params, &message_response);
    *status_text = StatusToString(status);
    *raw_response = static_cast<std::string>(message_response);
    if (status != pasc::Status::OK) {
      *error_code = static_cast<int32_t>(status);
      *result_json = json({{"error", *status_text}}).dump();
      RCLCPP_ERROR(get_logger(), "%s request failed: %s", method.c_str(),
                   status_text->c_str());
      return false;
    }

    mp::ResponseMessage response;
    response.Deserialize(message_response);
    *error_code = response.GetErrorCode();
    *result_json = response.GetResult().dump();
    return *error_code == 0;
  }

  bool RequestJsonObject(const std::string& method, const json& params,
                         json* result, std::string* status_text) {
    std::lock_guard<std::mutex> lock(client_mutex_);
    pasc::Message message_response;
    const auto status = client_->Request(method, params, &message_response);
    *status_text = StatusToString(status);
    if (status != pasc::Status::OK) {
      return false;
    }

    mp::ResponseMessage response;
    response.Deserialize(message_response);
    *result = response.GetResult();
    if (response.GetErrorCode() != 0) {
      *status_text = response.GetResult().dump();
      return false;
    }
    return true;
  }

  bool RequestWorldJson(const std::string& method, const json& params,
                        json* result, std::string* status_text) {
    const auto parent_topic = ResolveClockParentTopic();
    if (parent_topic.empty()) {
      *status_text = "Project AirSim scene parent topic is not resolved";
      return false;
    }
        return RequestJsonObject(parent_topic + "/" + method, params, result,
                                 status_text);
  }

  bool RequestWorldBool(const std::string& method, const json& params,
                        std::string* status_text) {
    json result;
    if (!RequestWorldJson(method, params, &result, status_text)) {
      return false;
    }

    if (!result.is_boolean()) {
            *status_text =
                "Expected boolean result from " + method + ": " + result.dump();
      return false;
    }

    if (!result.get<bool>()) {
      *status_text = method + " returned false";
      return false;
    }

    return true;
  }

    bool RequestVehicleJson(const std::string& vehicle_name,
                            const std::string& method, const json& params,
                            json* result, std::string* status_text) {
    const auto parent_topic = ResolveClockParentTopic();
    if (parent_topic.empty()) {
      *status_text = "Project AirSim scene parent topic is not resolved";
      return false;
    }
    const auto resolved_vehicle_name =
        vehicle_name.empty() ? vehicle_name_ : vehicle_name;
        return RequestJsonObject(
            parent_topic + "/robots/" + resolved_vehicle_name + "/" + method,
                             params, result, status_text);
  }

  bool PublishJson(const std::string& topic, const std::string& json_message,
                   std::string* status_text) {
    json message;
    try {
      message = json::parse(json_message);
    } catch (const json::parse_error& error) {
      *status_text = std::string("Invalid JSON message: ") + error.what();
      return false;
    }

    std::lock_guard<std::mutex> lock(client_mutex_);
    const auto status = client_->Publish(topic.c_str(), message);
    *status_text = StatusToString(status);
    return status == pasc::Status::OK;
  }

  bool UnsubscribeTopics(const std::vector<std::string>& topics, bool all,
                         std::string* status_text) {
    std::lock_guard<std::mutex> lock(client_mutex_);
        const auto status =
            all ? client_->UnsubscribeAll() : client_->Unsubscribe(topics);
    *status_text = StatusToString(status);
    if (status == pasc::Status::OK) {
            std::lock_guard<std::mutex> subscriptions_lock(
                subscriptions_mutex_);
      if (all) {
        subscribed_topics_.clear();
        handlers_.clear();
      } else {
        for (const auto& topic : topics) {
          subscribed_topics_.erase(
                        std::remove(subscribed_topics_.begin(),
                                    subscribed_topics_.end(), topic),
              subscribed_topics_.end());
          handlers_.erase(topic);
        }
      }
    }
    return status == pasc::Status::OK;
  }

  bool GetClientInfo(std::string* client_version, std::string* nng_version,
                     std::string* build_commit_hash) {
    *client_version = pasc::Client::GetVersion();
    *nng_version = pasc::Client::GetNNGVersion();
    std::lock_guard<std::mutex> lock(client_mutex_);
    const auto status = client_->GetBuildCommitHash(build_commit_hash);
    if (status != pasc::Status::OK) {
      RCLCPP_WARN(get_logger(), "GetBuildCommitHash failed: %s",
                  StatusToString(status).c_str());
    }
    return status == pasc::Status::OK;
  }

  void CacheClockParentTopic(const std::string& topic) {
    if (!clock_parent_topic_.empty()) return;
    const auto parent_topic =
        ParentTopicFromProjectAirSimTopic(topic, projectairsim_topic_root_);
    if (parent_topic.empty()) return;

    clock_parent_topic_ = parent_topic;
        RCLCPP_INFO(get_logger(),
                    "Resolved Project AirSim clock parent topic: %s",
                clock_parent_topic_.c_str());
  }

  std::string ResolveClockParentTopic() {
    if (!clock_parent_topic_.empty()) return clock_parent_topic_;

    const std::string world_parent_topic = world_->GetParentTopic();
    if (IsSceneParentTopic(world_parent_topic, projectairsim_topic_root_)) {
      clock_parent_topic_ = world_parent_topic;
      return clock_parent_topic_;
    }
    return clock_parent_topic_;
  }

  bool RequestSimTime(std::int64_t* nanosec) {
    if (!scene_loaded_) return false;
    const auto parent_topic = ResolveClockParentTopic();
    if (!parent_topic.empty()) {
      const auto method = parent_topic + "/GetSimTime";
      pasc::Message message_response;
      pasc::Status status = pasc::Status::OK;
      {
        std::lock_guard<std::mutex> lock(client_mutex_);
                status =
                    client_->Request(method, json::object(), &message_response);
      }
      if (status == pasc::Status::OK) {
        mp::ResponseMessage response;
        response.Deserialize(message_response);
        if (response.GetErrorCode() == 0 &&
            ExtractSimTimeNanos(response.GetResult(), nanosec)) {
          return true;
        }

                RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                                     "%s returned an invalid sim time "
                                     "response: error=%d result=%s",
            method.c_str(), response.GetErrorCode(),
            response.GetResult().dump().c_str());
      } else {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                             "%s request failed: %s", method.c_str(),
                             StatusToString(status).c_str());
      }
    }

    const auto status = world_->GetSimTime(nanosec);
    if (status != pasc::Status::OK) {
      RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                           "GetSimTime fallback failed: %s",
                           StatusToString(status).c_str());
      return false;
    }
    return true;
  }

  std::shared_ptr<pasc::Drone> GetDrone(const std::string& vehicle_name) {
    std::lock_guard<std::mutex> lock(drones_mutex_);
    auto it = drones_.find(vehicle_name);
    if (it != drones_.end()) return it->second;

    auto drone = std::make_shared<pasc::Drone>();
    const auto status = drone->Initialize(client_, world_, vehicle_name);
    if (status != pasc::Status::OK) {
      RCLCPP_ERROR(get_logger(), "Failed to initialize drone %s: %s",
                   vehicle_name.c_str(), StatusToString(status).c_str());
      return nullptr;
    }

    drones_[vehicle_name] = drone;
    return drone;
  }

    static pasc::Drone::YawControlMode YawModeFromDriveTrain(
        int drive_train_type) {
        return drive_train_type == 1
                   ? pasc::Drone::YawControlMode::ForwardOnly
                                 : pasc::Drone::YawControlMode::MaxDegreeOfFreedom;
  }

  static pasc::VecVector3 PathFromRos(
      const std::vector<geometry_msgs::msg::PoseStamped>& path) {
    pasc::VecVector3 result;
    result.reserve(path.size());
    for (const auto& pose : path) {
      result.push_back({static_cast<float>(pose.pose.position.x),
                        static_cast<float>(pose.pose.position.y),
                        static_cast<float>(pose.pose.position.z)});
    }
    return result;
  }

  static int CellsFromSize(double size, double resolution) {
    if (resolution <= 0.0 || size <= 0.0) return 0;
    return static_cast<int>(std::floor(size / resolution));
  }

  static pasc::Pose PoseFromPosition(double x, double y, double z) {
    pasc::Pose pose;
    pose.position = {static_cast<float>(x), static_cast<float>(y),
                     static_cast<float>(z)};
    pose.orientation = {1.0f, 0.0f, 0.0f, 0.0f};
    return pose;
  }

  bool EnableAndArm(const std::string& vehicle_name) {
    auto drone = GetDrone(vehicle_name);
    if (!drone) return false;
    bool state = false;
    auto status = drone->EnableAPIControl(&state);
    if (status != pasc::Status::OK) return false;
    status = drone->Arm(&state);
    return status == pasc::Status::OK;
  }

  bool Takeoff(const std::string& vehicle_name, bool wait_on_last_task) {
    auto drone = GetDrone(vehicle_name);
    if (!drone || !EnableAndArm(vehicle_name)) return false;
    auto result = drone->TakeoffAsync(10.0f);
    return !wait_on_last_task || result.Wait() == pasc::Status::OK;
  }

  bool Land(const std::string& vehicle_name, bool wait_on_last_task) {
    auto drone = GetDrone(vehicle_name);
    if (!drone) return false;
    auto result = drone->LandAsync();
    return !wait_on_last_task || result.Wait() == pasc::Status::OK;
  }

  bool Arm(const std::string& vehicle_name, bool /*wait_on_last_task*/) {
    return EnableAndArm(vehicle_name);
  }

  bool Disarm(const std::string& vehicle_name, bool /*wait_on_last_task*/) {
    auto drone = GetDrone(vehicle_name);
    if (!drone) return false;
    bool state = false;
    const auto disarm_status = drone->Disarm(&state);
    const auto disable_status = drone->DisableAPIControl(&state);
    if (disarm_status != pasc::Status::OK) {
      RCLCPP_ERROR(get_logger(), "Disarm failed: %s",
                   StatusToString(disarm_status).c_str());
    }
    if (disable_status != pasc::Status::OK) {
            RCLCPP_ERROR(get_logger(),
                         "DisableAPIControl after disarm failed: %s",
                   StatusToString(disable_status).c_str());
    }
        return disarm_status == pasc::Status::OK &&
               disable_status == pasc::Status::OK;
  }

    bool MoveToPosition(const std::string& vehicle_name, double x, double y,
                        double z, double velocity, float timeout_sec,
                        int drive_train_type, bool yaw_is_rate, float yaw,
                        float lookahead, float adaptive_lookahead,
                        bool wait_on_last_task) {
    auto drone = GetDrone(vehicle_name);
    if (!drone) return false;
    auto result = drone->MoveToPositionAsync(
        static_cast<float>(x), static_cast<float>(y), static_cast<float>(z),
            static_cast<float>(velocity), timeout_sec,
            YawModeFromDriveTrain(drive_train_type), yaw_is_rate, yaw,
            lookahead, adaptive_lookahead);
    return !wait_on_last_task || result.Wait() == pasc::Status::OK;
  }

  bool MoveOnPath(const std::string& vehicle_name,
                  const std::vector<geometry_msgs::msg::PoseStamped>& path,
                  double velocity, float timeout_sec, int drive_train_type,
                  bool yaw_is_rate, float yaw, float lookahead,
                  float adaptive_lookahead, bool wait_on_last_task) {
    auto drone = GetDrone(vehicle_name);
    if (!drone) return false;
    auto result = drone->MoveOnPathAsync(
        PathFromRos(path), static_cast<float>(velocity), timeout_sec,
            YawModeFromDriveTrain(drive_train_type), yaw_is_rate, yaw,
            lookahead, adaptive_lookahead);
    return !wait_on_last_task || result.Wait() == pasc::Status::OK;
  }

  bool CreateVoxelGrid(double x, double y, double z, int64_t x_size,
                       int64_t y_size, int64_t z_size, double resolution,
                       const std::string& output_file,
                       pasc::BoolArray* voxel_grid = nullptr) {
    if (x_size <= 0 || y_size <= 0 || z_size <= 0 || resolution <= 0.0) {
            RCLCPP_ERROR(
                get_logger(),
                   "CreateVoxelGrid requires positive sizes and resolution");
      return false;
    }

    pasc::BoolArray local_voxel_grid;
        auto& result_grid =
            voxel_grid != nullptr ? *voxel_grid : local_voxel_grid;
    const bool write_file = !output_file.empty();
    const auto status = world_->CreateVoxelGrid(
        PoseFromPosition(x, y, z), static_cast<int>(x_size),
        static_cast<int>(y_size), static_cast<int>(z_size),
            static_cast<float>(resolution), &result_grid, {}, write_file,
            output_file);
    if (status != pasc::Status::OK) {
      RCLCPP_ERROR(get_logger(), "CreateVoxelGrid failed: %s",
                   StatusToString(status).c_str());
      return false;
    }
    return true;
  }

  nav_msgs::msg::OccupancyGrid OccupancyGridFromVoxels(
      const pasc::BoolArray& voxel_grid, double center_x, double center_y,
      double center_z, int64_t x_size, int64_t y_size, int64_t z_size,
      double resolution) {
    (void)center_z;
    nav_msgs::msg::OccupancyGrid grid;
    grid.header = MakeHeader(*this, "map");
    grid.info.resolution = static_cast<float>(resolution);
        grid.info.width =
            static_cast<uint32_t>(CellsFromSize(x_size, resolution));
        grid.info.height =
            static_cast<uint32_t>(CellsFromSize(y_size, resolution));
        grid.info.origin.position.x =
            center_x - 0.5 * static_cast<double>(x_size);
        grid.info.origin.position.y =
            center_y - 0.5 * static_cast<double>(y_size);
    grid.info.origin.position.z = 0.0;
    grid.info.origin.orientation.w = 1.0;

    const int nx = static_cast<int>(grid.info.width);
    const int ny = static_cast<int>(grid.info.height);
    const int nz = CellsFromSize(z_size, resolution);
    grid.data.assign(static_cast<size_t>(nx * ny), -1);

    if (nx <= 0 || ny <= 0 || nz <= 0) return grid;
    const size_t expected_voxels = static_cast<size_t>(nx) *
                                   static_cast<size_t>(ny) *
                                   static_cast<size_t>(nz);
    if (voxel_grid.Cf() < expected_voxels) {
            RCLCPP_ERROR(get_logger(),
                         "Voxel grid size %zu is smaller than expected %zu",
                   voxel_grid.Cf(), expected_voxels);
      return grid;
    }

    for (int y = 0; y < ny; ++y) {
      for (int x = 0; x < nx; ++x) {
        bool occupied = false;
        for (int z = 0; z < nz; ++z) {
          const size_t voxel_index =
              static_cast<size_t>(x + nx * (z + nz * y));
          occupied = occupied || voxel_grid[voxel_index];
        }
        grid.data[static_cast<size_t>(x + nx * y)] = occupied ? 100 : 0;
      }
    }
    return grid;
  }

    bool GetClock(std::int64_t* nanosec) { return RequestSimTime(nanosec); }

  void PublishClock() {
        if (!scene_loaded_) {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *get_clock(), 5000,
                                 "Scene not loaded, not publishing clock");
            return;
        }
    std::int64_t nanosec = 0;
    if (!RequestSimTime(&nanosec)) {
            RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                                 "clock time request timeout");
      return;
    }

    rosgraph_msgs::msg::Clock clock_msg;
    const auto sec = nanosec / 1000000000LL;
    auto nsec = nanosec % 1000000000LL;
    if (nsec < 0) nsec += 1000000000LL;
    clock_msg.clock.sec = static_cast<int32_t>(sec);
    clock_msg.clock.nanosec = static_cast<uint32_t>(nsec);
    clock_publisher_->publish(clock_msg);
  }

  void StartClockTimerIfReady() {
        if (!scene_loaded_) {
            RCLCPP_WARN(get_logger(),
                        "StartClockTimerIfReady: scene not loaded yet");
            return;
        }
        if (publish_clock_period_sec_ <= 0.0) {
            RCLCPP_WARN(
                get_logger(),
                "StartClockTimerIfReady: publish_clock_period_sec (%f) <= 0.0",
                publish_clock_period_sec_);
            return;
        }
        if (clock_timer_) {
            RCLCPP_WARN(get_logger(),
                        "StartClockTimerIfReady: clock_timer_ already created");
      return;
    }
    clock_timer_ = create_wall_timer(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::duration<double>(publish_clock_period_sec_)),
        std::bind(&ProjectAirSimROS2CppNode::PublishClock, this));
        RCLCPP_INFO(
            get_logger(),
            "StartClockTimerIfReady: clock_timer_ created (period %f s)",
            publish_clock_period_sec_);
  }

    bool GetOriginGeoPoint(double* latitude, double* longitude,
                           double* altitude) {
    const auto& config = world_->GetConfiguration();
    if (!config.contains("home-geo-point") ||
        !config["home-geo-point"].is_object()) {
            RCLCPP_ERROR(get_logger(),
                         "Scene configuration has no home-geo-point");
      return false;
    }
    const auto& home_geo_point = config["home-geo-point"];
    *latitude = NumberOr(home_geo_point, "latitude");
    *longitude = NumberOr(home_geo_point, "longitude");
    *altitude = NumberOr(home_geo_point, "altitude");
    return true;
  }

    bool LoadSceneRuntime(const std::string& scene_file,
                          bool is_primary_client) {
    if (!is_primary_client) {
      return AdoptLoadedSceneIfAvailable();
    }
        const auto scene_config_path =
            ResolveSceneConfigPath(scene_file, sim_config_path_);
        const auto status =
            world_->Initialize(client_, scene_config_path, sim_config_path_,
        static_cast<float>(delay_after_load_sec_));
    if (status != pasc::Status::OK) {
            RCLCPP_ERROR(get_logger(), "Load scene failed: %s",
                         StatusToString(status).c_str());
      return false;
    }

    {
      std::lock_guard<std::mutex> lock(drones_mutex_);
      drones_.clear();
    }
    {
      std::lock_guard<std::mutex> lock(subscriptions_mutex_);
      subscribed_topics_.clear();
      handlers_.clear();
    }
    clock_parent_topic_ = world_->GetParentTopic();
    scene_loaded_ = true;
    reported_waiting_for_scene_ = false;
    listed_projectairsim_topics_ = false;
    DiscoverAndSubscribe();
    StartClockTimerIfReady();
    return true;
  }

  template <typename ServiceT, typename Fn>
  void CreateService(const std::string& name, Fn&& fn) {
    services_.push_back(create_service<ServiceT>(
            name, [this, name, fn = std::forward<Fn>(fn)](
            const std::shared_ptr<typename ServiceT::Request> request,
            std::shared_ptr<typename ServiceT::Response> response) {
          try {
            fn(request, response);
          } catch (const std::exception& error) {
            SetSuccessIfPresent(*response, false);
                    RCLCPP_ERROR(get_logger(), "%s failed: %s", name.c_str(),
                                 error.what());
          }
        }));
  }

  template <typename GroupServiceT, typename Fn>
  void CreateGroupService(const std::string& name, Fn&& fn) {
    CreateService<GroupServiceT>(
            name,
            [fn = std::forward<Fn>(fn)](const auto request, auto response) {
          response->success = true;
          for (const auto& vehicle_name : request->vehicle_names) {
            response->success =
                        fn(vehicle_name, request->wait_on_last_task) &&
                        response->success;
          }
        });
  }

  void CreateServices(const std::string& vehicle_name) {
    const std::string prefix = service_root_ + "/" + vehicle_name;

    CreateService<projectairsim_ros2_cpp::srv::RawRequest>(
            service_root_ + "/request",
            [this](const auto request, auto response) {
          response->success =
              RequestJson(request->method, request->json_parameters,
                          &response->error_code, &response->result_json,
                          &response->raw_response, &response->status);
        });
    CreateService<projectairsim_ros2_cpp::srv::GetClientInfo>(
        service_root_ + "/get_client_info",
        [this](const auto request, auto response) {
          (void)request;
                response->success = GetClientInfo(&response->client_version,
                                                  &response->nng_version,
                            &response->build_commit_hash);
        });
    CreateService<projectairsim_ros2_cpp::srv::GetTopicInfo>(
            service_root_ + "/get_topic_info",
            [this](const auto request, auto response) {
          (void)request;
          std::lock_guard<std::mutex> lock(client_mutex_);
          response->topics = client_->GetTopicInfo();
          response->topics_json = json(response->topics).dump();
          response->success = true;
        });
    CreateService<projectairsim_ros2_cpp::srv::GetDrones>(
            service_root_ + "/get_drones",
            [this](const auto request, auto response) {
          (void)request;
          const auto& drones = world_->GetDrones();
          response->drones.assign(drones.begin(), drones.end());
          response->success = true;
        });
    CreateService<projectairsim_ros2_cpp::srv::Publish>(
            service_root_ + "/publish",
            [this](const auto request, auto response) {
                response->success = PublishJson(
                    request->topic, request->json_message, &response->status);
        });
    CreateService<projectairsim_ros2_cpp::srv::Unsubscribe>(
            service_root_ + "/unsubscribe",
            [this](const auto request, auto response) {
                response->success = UnsubscribeTopics(
                    request->topics, request->all, &response->status);
        });
    CreateService<projectairsim_ros2_cpp::srv::CancelAllRequests>(
        service_root_ + "/cancel_all_requests",
        [this](const auto request, auto response) {
          (void)request;
          client_->CancelAllRequests();
          response->success = true;
        });

    CreateService<projectairsim_ros2_cpp::srv::Takeoff>(
            prefix + "/takeoff",
            [this, vehicle_name](const auto request, auto response) {
                response->success =
                    Takeoff(vehicle_name, request->wait_on_last_task);
        });
    CreateService<projectairsim_ros2_cpp::srv::Land>(
            prefix + "/land",
            [this, vehicle_name](const auto request, auto response) {
                response->success =
                    Land(vehicle_name, request->wait_on_last_task);
        });
    CreateService<projectairsim_ros2_cpp::srv::Arm>(
            prefix + "/arm",
            [this, vehicle_name](const auto request, auto response) {
                response->success =
                    Arm(vehicle_name, request->wait_on_last_task);
        });
    CreateService<projectairsim_ros2_cpp::srv::Disarm>(
            prefix + "/disarm",
            [this, vehicle_name](const auto request, auto response) {
                response->success =
                    Disarm(vehicle_name, request->wait_on_last_task);
        });
    CreateService<projectairsim_ros2_cpp::srv::MoveToPosition>(
        prefix + "/move_to_position",
        [this, vehicle_name](const auto request, auto response) {
          response->success = MoveToPosition(
                    vehicle_name, request->x, request->y, request->z,
                    request->velocity, request->timeout_sec,
                    request->drive_train_type, request->yaw_is_rate,
                    request->yaw, request->lookahead,
                    request->adaptive_lookahead, request->wait_on_last_task);
        });
    CreateService<projectairsim_ros2_cpp::srv::MoveOnPath>(
            prefix + "/move_on_path",
            [this, vehicle_name](const auto request, auto response) {
          response->success = MoveOnPath(
                    vehicle_name, request->path, request->velocity,
                    request->timeout_sec, request->drive_train_type,
                    request->yaw_is_rate, request->yaw, request->lookahead,
                    request->adaptive_lookahead, request->wait_on_last_task);
        });

    CreateGroupService<projectairsim_ros2_cpp::srv::TakeoffGroup>(
        service_root_ + "/takeoff_group",
            [this](const std::string& name, bool wait) {
                return Takeoff(name, wait);
            });
    CreateGroupService<projectairsim_ros2_cpp::srv::LandGroup>(
        service_root_ + "/land_group",
            [this](const std::string& name, bool wait) {
                return Land(name, wait);
            });
    CreateGroupService<projectairsim_ros2_cpp::srv::ArmGroup>(
        service_root_ + "/arm_group",
            [this](const std::string& name, bool wait) {
                return Arm(name, wait);
            });
    CreateGroupService<projectairsim_ros2_cpp::srv::DisarmGroup>(
        service_root_ + "/disarm_group",
            [this](const std::string& name, bool wait) {
                return Disarm(name, wait);
            });
    CreateService<projectairsim_ros2_cpp::srv::CreateVoxelGrid>(
        service_root_ + "/create_voxel_grid",
        [this](const auto request, auto response) {
          (void)request->n_z_resolution;
          response->success =
              CreateVoxelGrid(request->position_x, request->position_y,
                              request->position_z, request->ncells_x,
                              request->ncells_y, request->ncells_z,
                              request->resolution, request->output_file);
        });
    CreateService<projectairsim_ros2_cpp::srv::OccupancyGrid>(
        service_root_ + "/occupancy_grid",
        [this](const auto request, auto response) {
          (void)request->n_z_resolution;
          pasc::BoolArray voxel_grid;
                response->success = CreateVoxelGrid(
                    request->position_x, request->position_y,
                    request->position_z, request->ncells_x, request->ncells_y,
                    request->ncells_z, request->res, "", &voxel_grid);
          if (response->success) {
            response->map = OccupancyGridFromVoxels(
                voxel_grid, request->position_x, request->position_y,
                        request->position_z, request->ncells_x,
                        request->ncells_y, request->ncells_z, request->res);
          }
        });
    CreateService<projectairsim_ros2_cpp::srv::GetClock>(
            service_root_ + "/get_clock",
            [this](const auto request, auto response) {
          (void)request;
          std::int64_t nanosec = 0;
          if (GetClock(&nanosec)) response->nanosec = nanosec;
        });
    CreateService<projectairsim_ros2_cpp::srv::GetOriginGeoPoint>(
        service_root_ + "/get_origin_geo_point",
        [this](const auto request, auto response) {
          (void)request;
          double latitude = 0.0;
          double longitude = 0.0;
          double altitude = 0.0;
          if (GetOriginGeoPoint(&latitude, &longitude, &altitude)) {
            response->latitude = latitude;
            response->longitude = longitude;
            response->altitude = altitude;
          }
        });
    CreateService<projectairsim_ros2_cpp::srv::SetSegmentationIDByName>(
        service_root_ + "/set_segmentation_id_by_name",
        [this](const auto request, auto response) {
          response->success = RequestWorldBool(
              "SetSegmentationIDByName",
              json{{"mesh_name", request->mesh_name},
                   {"segmentation_id", request->segmentation_id},
                   {"is_name_regex", request->is_name_regex},
                   {"use_owner_name", request->use_owner_name}},
              &response->status);
        });
    CreateService<projectairsim_ros2_cpp::srv::GetSegmentationIDByName>(
        service_root_ + "/get_segmentation_id_by_name",
        [this](const auto request, auto response) {
          json result;
          response->success = RequestWorldJson(
              "GetSegmentationIDByName",
              json{{"mesh_name", request->mesh_name},
                   {"use_owner_name", request->use_owner_name}},
              &result, &response->status);
          if (response->success && result.is_number_integer()) {
            response->segmentation_id = result.get<int32_t>();
          }
        });
    CreateService<projectairsim_ros2_cpp::srv::GetSegmentationIDMap>(
        service_root_ + "/get_segmentation_id_map",
        [this](const auto request, auto response) {
          (void)request;
          json result;
                response->success =
                    RequestWorldJson("GetSegmentationIDMap", json::object(),
                                     &result, &response->status);
                if (response->success)
                    response->segmentation_map_json = result.dump();
        });
    CreateService<projectairsim_ros2_cpp::srv::SetObjectMaterial>(
            service_root_ + "/set_object_material",
            [this](const auto request, auto response) {
          response->success = RequestWorldBool(
              "SetObjectMaterial",
              json{{"object_name", request->object_name},
                    {"material_asset_path", request->material_asset_path}},
              &response->status);
        });
    CreateService<projectairsim_ros2_cpp::srv::SetObjectTexture>(
            service_root_ + "/set_object_texture",
            [this](const auto request, auto response) {
          std::string method;
          json params = {{"object_name", request->object_name}};
          if (request->source ==
                    projectairsim_ros2_cpp::srv::SetObjectTexture::Request::
                        SOURCE_URL) {
            method = "SetObjectTextureFromUrl";
            params["url"] = request->texture;
                } else if (request->source ==
                           projectairsim_ros2_cpp::srv::SetObjectTexture::
                               Request::SOURCE_FILE) {
            method = "SetObjectTextureFromFile";
            params["texture_file_path"] = request->texture;
                } else if (request->source ==
                           projectairsim_ros2_cpp::srv::SetObjectTexture::
                               Request::SOURCE_PACKAGED_ASSET) {
            method = "SetObjectTextureFromPackagedAsset";
            params["texture_asset_path"] = request->texture;
          } else {
            response->success = false;
            response->status = "Invalid texture source";
            return;
          }
                response->success =
                    RequestWorldBool(method, params, &response->status);
        });
    CreateService<projectairsim_ros2_cpp::srv::SwapObjectTexture>(
            service_root_ + "/swap_object_texture",
            [this](const auto request, auto response) {
          response->success = RequestWorldBool(
              "SwapObjectTexture",
              json{{"tag", request->tag}, {"tex_id", request->tex_id}},
              &response->status);
        });
    CreateService<projectairsim_ros2_cpp::srv::Reset>(
            service_root_ + "/reset",
            [this](const auto request, auto response) {
          (void)request;
          response->success = RequestBool("/Sim/Reset");
        });
    CreateService<projectairsim_ros2_cpp::srv::LoadScene>(
            service_root_ + "/load_scene",
            [this](const auto request, auto response) {
                response->success = LoadSceneRuntime(
                    request->scene_file, request->is_primary_client);
        });
  }

  void CreateActionServer(const std::string& vehicle_name) {
    action_server_ = rclcpp_action::create_server<MoveOnPathAction>(
        this, service_root_ + "/" + vehicle_name + "/move_on_path",
        [](const rclcpp_action::GoalUUID&,
           std::shared_ptr<const MoveOnPathAction::Goal>) {
          return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        },
        [](const std::shared_ptr<GoalHandleMoveOnPath>) {
          return rclcpp_action::CancelResponse::ACCEPT;
        },
            [this, vehicle_name](
                const std::shared_ptr<GoalHandleMoveOnPath> goal_handle) {
          std::thread([this, vehicle_name, goal_handle]() {
            ExecuteMoveOnPath(vehicle_name, goal_handle);
          }).detach();
        });
  }

    void ExecuteMoveOnPath(
        const std::string& vehicle_name,
                         const std::shared_ptr<GoalHandleMoveOnPath> goal_handle) {
    const auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<MoveOnPathAction::Feedback>();
    feedback->current_point_on_path = "submitted";
    goal_handle->publish_feedback(feedback);

    auto result = std::make_shared<MoveOnPathAction::Result>();
        result->success = MoveOnPath(
            vehicle_name, goal->path, goal->velocity, goal->timeout_sec,
                   goal->drive_train_type, goal->yaw_is_rate, goal->yaw,
            goal->lookahead, goal->adaptive_lookahead, goal->wait_on_last_task);

    if (result->success) {
      goal_handle->succeed(result);
    } else {
      goal_handle->abort(result);
    }
  }

  void HandleProjectAirSimMessage(const std::string& topic_name,
                                  const std::string& payload) {
    try {
      const auto msg = UnpackProjectAirSimMessage(payload);
      TopicHandler handler;
      {
        std::lock_guard<std::mutex> lock(subscriptions_mutex_);
        auto handler_it = handlers_.find(topic_name);
        if (handler_it != handlers_.end()) handler = handler_it->second;
      }
      if (handler) handler(topic_name, msg);
    } catch (const std::exception& ex) {
            RCLCPP_WARN_THROTTLE(
                get_logger(), *get_clock(), 5000,
                           "Failed to convert Project AirSim topic %s: %s",
                           topic_name.c_str(), ex.what());
    }
  }

  std::shared_ptr<pasc::Client> client_;
  std::shared_ptr<pasc::World> world_;
  std::mutex client_mutex_;
  std::mutex drones_mutex_;
  std::mutex subscriptions_mutex_;
  std::string address_;
  int port_topics_;
  int port_services_;
  std::string scene_config_;
  std::string sim_config_path_;
  double delay_after_load_sec_;
  std::string projectairsim_topic_root_;
  std::string ros_topic_root_;
  std::string vehicle_name_;
  std::string service_root_;
  std::string clock_parent_topic_;
  bool publish_unmatched_as_json_;
  bool publish_tf_;
  std::string tf_world_frame_id_;
  double refresh_topics_period_sec_;
  double publish_clock_period_sec_;
  bool listed_projectairsim_topics_ = false;
  bool scene_loaded_ = false;
  bool reported_waiting_for_scene_ = false;
  rclcpp::TimerBase::SharedPtr refresh_timer_;
  rclcpp::TimerBase::SharedPtr clock_timer_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr topic_info_publisher_;
  rclcpp::Publisher<rosgraph_msgs::msg::Clock>::SharedPtr clock_publisher_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  std::vector<rclcpp::ServiceBase::SharedPtr> services_;
  rclcpp_action::Server<MoveOnPathAction>::SharedPtr action_server_;
  std::vector<std::string> subscribed_topics_;
  std::map<std::string, TopicHandler> handlers_;
  std::map<std::string, std::shared_ptr<pasc::Drone>> drones_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  try {
    rclcpp::spin(std::make_shared<ProjectAirSimROS2CppNode>());
  } catch (const std::exception& ex) {
        RCLCPP_FATAL(rclcpp::get_logger("projectairsim_ros2_cpp_node"), "%s",
                     ex.what());
    rclcpp::shutdown();
    return 1;
  }
  rclcpp::shutdown();
  return 0;
}
