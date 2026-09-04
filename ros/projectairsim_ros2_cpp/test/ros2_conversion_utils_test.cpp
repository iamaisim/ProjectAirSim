#include "projectairsim_ros2_cpp/ros2_conversion_utils.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "msgpack.hpp"
#include "projectairsim_ros2_cpp/msg/radar_scan.hpp"
#include "projectairsim_ros2_cpp/msg/radar_tracks.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/image.hpp"

namespace {

namespace bridge = projectairsim_ros2_cpp::bridge;
using bridge::json;

std::string PackNativeImage(const std::string& encoding, std::uint32_t width,
                            std::uint32_t height,
                            const std::vector<std::uint8_t>& data,
                            bool include_data = true,
                            bool include_time_stamp = true) {
  msgpack::sbuffer buffer;
  msgpack::packer<msgpack::sbuffer> packer(buffer);
  packer.pack_map(13 + static_cast<unsigned int>(include_data) +
                  static_cast<unsigned int>(include_time_stamp));
  if (include_time_stamp) {
    packer.pack("time_stamp");
    packer.pack(123U);
  }
  packer.pack("height");
  packer.pack(height);
  packer.pack("width");
  packer.pack(width);
  packer.pack("encoding");
  packer.pack(encoding);
  packer.pack("big_endian");
  packer.pack(false);
  packer.pack("step");
  packer.pack(1U);
  if (include_data) {
    packer.pack("data");
    packer.pack(data);
  }
  packer.pack("pos_x");
  packer.pack(1.0F);
  packer.pack("pos_y");
  packer.pack(2.0F);
  packer.pack("pos_z");
  packer.pack(3.0F);
  packer.pack("rot_w");
  packer.pack(0.5F);
  packer.pack("rot_x");
  packer.pack(0.1F);
  packer.pack("rot_y");
  packer.pack(0.2F);
  packer.pack("rot_z");
  packer.pack(0.3F);
  packer.pack("annotations");
  packer.pack_array(0);
  return {buffer.data(), buffer.size()};
}

TEST(Ros2ConversionUtils, TopicAndFrameHelpersMatchBridgeConventions) {
  EXPECT_TRUE(bridge::EndsWith("/Sim/Scene/robots/Drone1/imu", "/imu"));
  EXPECT_TRUE(bridge::StartsWithPathPrefix("/Sim/Scene/robots", "/Sim"));
  EXPECT_FALSE(bridge::StartsWithPathPrefix("/Simulator/Scene", "/Sim"));

  EXPECT_EQ(bridge::TopicToFrameId("/Sim/Scene/robots/Drone1/imu"), "Drone1");
  EXPECT_EQ(bridge::TopicToFrameId("/Sim/Scene/robots/Drone1/imu_kinematics"),
            "Drone1");
  EXPECT_EQ(
      bridge::CameraFrameId("/Sim/Scene/robots/Drone1/Down/scene_camera_info"),
      "Sim_Scene_robots_Drone1_Down_scene_camera");
  EXPECT_EQ(bridge::CameraImageTopicFromInfoTopic(
                "/Sim/Scene/robots/Drone1/Down/scene_camera_info"),
            "/Sim/Scene/robots/Drone1/Down/scene_camera");
  EXPECT_EQ(bridge::CameraInfoRosTopic(
                "/ProjectAirsim/Scene/robots/Drone1/Down/scene_camera"),
            "/ProjectAirsim/Scene/robots/Drone1/Down/scene_camera/camera_info");
  EXPECT_EQ(bridge::ParentTopicFromProjectAirSimTopic(
                "/Sim/Scene/robots/Drone1/imu", "/Sim"),
            "/Sim/Scene");
  EXPECT_EQ(bridge::ParentTopicFromProjectAirSimTopic("/$topics", "/"), "");
  EXPECT_TRUE(bridge::IsSceneParentTopic("/Sim/Scene", "/Sim"));
  EXPECT_FALSE(bridge::IsSceneParentTopic("/Sim", "/Sim"));
  EXPECT_FALSE(bridge::IsSceneParentTopic("/$topics", "/"));
}

TEST(Ros2ConversionUtils, SimTimeExtractionAcceptsSupportedPayloadShapes) {
  std::int64_t nanos = 0;

  ASSERT_TRUE(bridge::ExtractSimTimeNanos(1234, &nanos));
  EXPECT_EQ(nanos, 1234);

  ASSERT_TRUE(
      bridge::ExtractSimTimeNanos(json{{"sim_time_nanos", 5678}}, &nanos));
  EXPECT_EQ(nanos, 5678);

  ASSERT_TRUE(bridge::ExtractSimTimeNanos(json{{"time_stamp", 9012}}, &nanos));
  EXPECT_EQ(nanos, 9012);

  ASSERT_TRUE(
      bridge::ExtractSimTimeNanos(json{{"sec", 2}, {"nanosec", 9}}, &nanos));
  EXPECT_EQ(nanos, 2000000009LL);

  EXPECT_FALSE(bridge::ExtractSimTimeNanos(json{{"time", "bad"}}, &nanos));
}

TEST(Ros2ConversionUtils, TimestampConversionRejectsInvalidNumericValues) {
  std::int64_t value = 0;

  EXPECT_FALSE(bridge::JsonToInt64(
      std::numeric_limits<std::uint64_t>::max(), &value));
  EXPECT_FALSE(bridge::JsonToInt64(
      std::numeric_limits<double>::infinity(), &value));
}

TEST(Ros2ConversionUtils, CoordinateConversionsApplyRosAxisConvention) {
  const auto vector_from_array =
      bridge::ToRosVector3(json::array({1.0, 2.0, 3.0}));
  EXPECT_DOUBLE_EQ(vector_from_array.x, 1.0);
  EXPECT_DOUBLE_EQ(vector_from_array.y, -2.0);
  EXPECT_DOUBLE_EQ(vector_from_array.z, -3.0);

  const auto point =
      bridge::ToRosPoint(json{{"x", 4.0}, {"y", 5.0}, {"z", -6.0}});
  EXPECT_DOUBLE_EQ(point.x, 4.0);
  EXPECT_DOUBLE_EQ(point.y, -5.0);
  EXPECT_DOUBLE_EQ(point.z, 6.0);

  const auto quaternion = bridge::ToRosQuaternion(
      json{{"x", 0.1}, {"y", 0.2}, {"z", 0.3}, {"w", 0.4}});
  EXPECT_DOUBLE_EQ(quaternion.x, 0.1);
  EXPECT_DOUBLE_EQ(quaternion.y, -0.2);
  EXPECT_DOUBLE_EQ(quaternion.z, -0.3);
  EXPECT_DOUBLE_EQ(quaternion.w, 0.4);
}

TEST(Ros2ConversionUtils, CameraInfoAndImagePayloadsAreConverted) {
  sensor_msgs::msg::CameraInfo camera_info;
  bridge::PopulateCameraInfoFromJson(
      json{{"width", 640},
           {"height", 480},
           {"distortion_model", "plumb_bob"},
           {"distortion_params", json::array({0.1, 0.2})},
           {"intrinsic_camera_matrix",
            json::array({1, 0, 320, 0, 1, 240, 0, 0, 1})},
           {"rectification_matrix", json::array({1, 0, 0, 0, 1, 0, 0, 0, 1})},
           {"projection_matrix",
            json::array({1, 0, 320, 0, 0, 1, 240, 0, 0, 0, 1, 0})}},
      &camera_info);

  EXPECT_EQ(camera_info.width, 640u);
  EXPECT_EQ(camera_info.height, 480u);
  EXPECT_EQ(camera_info.distortion_model, "plumb_bob");
  EXPECT_EQ(camera_info.d.size(), 2u);
  EXPECT_DOUBLE_EQ(camera_info.k[2], 320.0);
  EXPECT_DOUBLE_EQ(camera_info.p[10], 1.0);

  sensor_msgs::msg::Image bgr_image;
  ASSERT_TRUE(bridge::PopulateImagePayloadFromJson(
      json{{"height", 2},
           {"width", 3},
           {"big_endian", 0},
           {"encoding", "BGR"},
           {"data", std::string("abcdefghijklmnopqr")}},
      &bgr_image));
  EXPECT_EQ(bgr_image.encoding, "bgr8");
  EXPECT_EQ(bgr_image.step, 9u);
  EXPECT_EQ(bgr_image.data.size(), 18u);

  sensor_msgs::msg::Image depth_image;
  ASSERT_TRUE(bridge::PopulateImagePayloadFromJson(
      json{{"height", 1},
           {"width", 2},
           {"encoding", "16UC1"},
           {"data", std::string("\x01\x00\x02\x00", 4)}},
      &depth_image));
  EXPECT_EQ(depth_image.encoding, "mono16");
  EXPECT_EQ(depth_image.step, 4u);

  sensor_msgs::msg::Image unsupported;
  EXPECT_FALSE(bridge::PopulateImagePayloadFromJson(
      json{{"height", 1}, {"width", 1}, {"encoding", "RGB"}}, &unsupported));
}

TEST(Ros2ConversionUtils, NativeBgrImagePayloadIsConvertedWithoutJson) {
  const std::vector<std::uint8_t> pixels{1, 2, 3, 4, 5, 6};
  sensor_msgs::msg::Image image;
  bridge::NativeImageMetadata metadata;

  ASSERT_TRUE(bridge::PopulateImagePayloadFromMsgpack(
      PackNativeImage("BGR", 2, 1, pixels), &image, &metadata));
  EXPECT_EQ(image.width, 2U);
  EXPECT_EQ(image.height, 1U);
  EXPECT_EQ(image.encoding, "bgr8");
  EXPECT_EQ(image.step, 6U);
  EXPECT_EQ(image.data, pixels);
  EXPECT_TRUE(metadata.has_time_stamp);
  EXPECT_EQ(metadata.time_stamp, 123U);
  EXPECT_FLOAT_EQ(metadata.pos_y, 2.0F);
  EXPECT_FLOAT_EQ(metadata.rot_w, 0.5F);
}

TEST(Ros2ConversionUtils, NativeDepthImagePayloadIsConverted) {
  const std::vector<std::uint8_t> pixels{1, 0, 2, 0};
  sensor_msgs::msg::Image image;
  bridge::NativeImageMetadata metadata;

  ASSERT_TRUE(bridge::PopulateImagePayloadFromMsgpack(
      PackNativeImage("16UC1", 2, 1, pixels), &image, &metadata));
  EXPECT_EQ(image.encoding, "mono16");
  EXPECT_EQ(image.step, 4U);
  EXPECT_EQ(image.data, pixels);
}

TEST(Ros2ConversionUtils, NativeImageMetadataTracksMissingTimestamp) {
  sensor_msgs::msg::Image image;
  bridge::NativeImageMetadata metadata;

  ASSERT_TRUE(bridge::PopulateImagePayloadFromMsgpack(
      PackNativeImage("BGR", 1, 1, {1, 2, 3}, true, false), &image,
      &metadata));
  EXPECT_FALSE(metadata.has_time_stamp);
  EXPECT_EQ(metadata.time_stamp, 0U);
}

TEST(Ros2ConversionUtils, NativeHalfDepthImagePayloadIsConvertedToFloat) {
  // IEEE 754 binary16 little-endian: 1.0 and +inf.
  const std::vector<std::uint8_t> pixels{0x00, 0x3C, 0x00, 0x7C};
  sensor_msgs::msg::Image image;
  bridge::NativeImageMetadata metadata;

  ASSERT_TRUE(bridge::PopulateImagePayloadFromMsgpack(
      PackNativeImage("16FC1", 2, 1, pixels), &image, &metadata));
  EXPECT_EQ(image.encoding, "32FC1");
  EXPECT_EQ(image.step, 8U);
  ASSERT_EQ(image.data.size(), 8U);

  std::array<float, 2> depth{};
  std::memcpy(depth.data(), image.data.data(), image.data.size());
  EXPECT_FLOAT_EQ(depth[0], 1.0F);
  EXPECT_TRUE(std::isnan(depth[1]));
  EXPECT_EQ(metadata.source_encoding, "16FC1");
}

TEST(Ros2ConversionUtils, NativeImagePayloadRejectsInvalidInputs) {
  sensor_msgs::msg::Image image;
  bridge::NativeImageMetadata metadata;

  EXPECT_FALSE(bridge::PopulateImagePayloadFromMsgpack(
      PackNativeImage("RGB", 1, 1, {1, 2, 3}), &image, &metadata));
  EXPECT_EQ(metadata.source_encoding, "RGB");
  EXPECT_THROW(bridge::PopulateImagePayloadFromMsgpack(
                   PackNativeImage("BGR", 2, 1, {1, 2, 3}), &image, &metadata),
               std::runtime_error);
  EXPECT_THROW(
      bridge::PopulateImagePayloadFromMsgpack(
          PackNativeImage("BGR", 1, 1, {1, 2, 3}, false), &image, &metadata),
      std::runtime_error);
  EXPECT_THROW(
      bridge::PopulateImagePayloadFromMsgpack("not-msgpack", &image, &metadata),
      std::exception);
}

TEST(Ros2ConversionUtils, RadarPayloadsAreConverted) {
  projectairsim_ros2_cpp::msg::RadarScan scan;
  bridge::AppendRadarReturnsFromJson(json::array({json{{"range", 10.0},
                                                       {"azimuth", 0.25},
                                                       {"elevation", -0.1},
                                                       {"velocity", 2.5},
                                                       {"rcs_sqm", 3.0}}}),
                                     &scan);

  ASSERT_EQ(scan.returns.size(), 1u);
  EXPECT_FLOAT_EQ(scan.returns[0].range, 10.0F);
  EXPECT_FLOAT_EQ(scan.returns[0].azimuth, 0.25F);
  EXPECT_FLOAT_EQ(scan.returns[0].doppler_velocity, 2.5F);
  EXPECT_FLOAT_EQ(scan.returns[0].amplitude,
                  bridge::RadarAmplitudeFromRangeAndRcs(10.0, 3.0));

  projectairsim_ros2_cpp::msg::RadarTracks tracks;
  bridge::AppendRadarTracksFromJson(
      json::array({json{{"id", 3},
                        {"position_est", json::array({1.0, 2.0, 3.0})},
                        {"velocity_est", json::array({4.0, 5.0, 6.0})},
                        {"accel_est", json::array({7.0, 8.0, 9.0})},
                        {"azimuth_est", 0.7},
                        {"elevation_est", 0.8},
                        {"range_est", 15.0}}}),
      &tracks);

  ASSERT_EQ(tracks.tracks.size(), 1u);
  EXPECT_EQ(tracks.tracks[0].id, 3);
  EXPECT_DOUBLE_EQ(tracks.tracks[0].position.y, -2.0);
  EXPECT_DOUBLE_EQ(tracks.tracks[0].velocity.z, -6.0);
  EXPECT_FLOAT_EQ(tracks.tracks[0].range, 15.0F);
}

TEST(Ros2ConversionUtils, MsgpackPayloadUnpacksToJson) {
  msgpack::sbuffer buffer;
  const std::map<std::string, int> payload{{"answer", 42}};
  msgpack::pack(buffer, payload);

  const auto unpacked = bridge::UnpackProjectAirSimMessage(
      std::string(buffer.data(), buffer.size()));

  ASSERT_TRUE(unpacked.is_object());
  EXPECT_EQ(unpacked["answer"].get<int>(), 42);
}

}  // namespace
