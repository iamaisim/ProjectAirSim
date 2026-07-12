#include <gtest/gtest.h>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "projectairsim_ros2_cpp/action/move_on_path.hpp"
#include "projectairsim_ros2_cpp/msg/radar_return.hpp"
#include "projectairsim_ros2_cpp/msg/radar_scan.hpp"
#include "projectairsim_ros2_cpp/msg/radar_track.hpp"
#include "projectairsim_ros2_cpp/msg/radar_tracks.hpp"
#include "projectairsim_ros2_cpp/srv/get_clock.hpp"
#include "projectairsim_ros2_cpp/srv/get_segmentation_id_by_name.hpp"
#include "projectairsim_ros2_cpp/srv/get_segmentation_id_map.hpp"
#include "projectairsim_ros2_cpp/srv/raw_request.hpp"
#include "projectairsim_ros2_cpp/srv/set_object_material.hpp"
#include "projectairsim_ros2_cpp/srv/set_object_texture.hpp"
#include "projectairsim_ros2_cpp/srv/set_segmentation_id_by_name.hpp"
#include "projectairsim_ros2_cpp/srv/swap_object_texture.hpp"

namespace {

TEST(Ros2GeneratedInterfacesRegression, RadarMessageInterfacesMatchExpectedFields) {
  projectairsim_ros2_cpp::msg::RadarReturn radar_return;
  radar_return.range = 10.0F;
  radar_return.azimuth = 0.25F;
  radar_return.elevation = -0.1F;
  radar_return.doppler_velocity = 2.5F;
  radar_return.amplitude = 12.0F;

  projectairsim_ros2_cpp::msg::RadarScan scan;
  scan.returns.push_back(radar_return);

  projectairsim_ros2_cpp::msg::RadarTrack track;
  track.id = 7;
  track.position.x = 1.0;
  track.velocity.y = 2.0;
  track.acceleration.z = -3.0;
  track.azimuth = 0.1F;
  track.elevation = 0.2F;
  track.range = 30.0F;

  projectairsim_ros2_cpp::msg::RadarTracks tracks;
  tracks.tracks.push_back(track);

  EXPECT_FLOAT_EQ(scan.returns[0].range, 10.0F);
  EXPECT_EQ(tracks.tracks[0].id, 7);
  EXPECT_DOUBLE_EQ(tracks.tracks[0].acceleration.z, -3.0);
}

TEST(Ros2GeneratedInterfacesRegression, GenericAndClockServicesAreUsable) {
  projectairsim_ros2_cpp::srv::RawRequest::Request raw_request;
  raw_request.method = "/Sim/GetBuildCommitHash";
  raw_request.json_parameters = "{}";

  projectairsim_ros2_cpp::srv::RawRequest::Response raw_response;
  raw_response.success = true;
  raw_response.error_code = 0;
  raw_response.result_json = "\"abc123\"";

  projectairsim_ros2_cpp::srv::GetClock::Response clock_response;
  clock_response.nanosec = 123456789;

  EXPECT_EQ(raw_request.method, "/Sim/GetBuildCommitHash");
  EXPECT_TRUE(raw_response.success);
  EXPECT_EQ(clock_response.nanosec, 123456789);
}

TEST(Ros2GeneratedInterfacesRegression,
     SegmentationMaterialAndTextureServicesMatchBridgeContract) {
  projectairsim_ros2_cpp::srv::SetSegmentationIDByName::Request set_seg;
  set_seg.mesh_name = "Car.*";
  set_seg.segmentation_id = 42;
  set_seg.is_name_regex = true;
  set_seg.use_owner_name = false;

  projectairsim_ros2_cpp::srv::GetSegmentationIDByName::Response get_seg;
  get_seg.success = true;
  get_seg.segmentation_id = 42;

  projectairsim_ros2_cpp::srv::GetSegmentationIDMap::Response seg_map;
  seg_map.success = true;
  seg_map.segmentation_map_json = "{}";

  projectairsim_ros2_cpp::srv::SetObjectMaterial::Request material;
  material.object_name = "Cube";
  material.material_asset_path = "/Game/Materials/M_Test";

  projectairsim_ros2_cpp::srv::SetObjectTexture::Request texture;
  texture.object_name = "Cube";
  texture.source =
      projectairsim_ros2_cpp::srv::SetObjectTexture::Request::SOURCE_URL;
  texture.texture = "https://example.invalid/texture.png";

  projectairsim_ros2_cpp::srv::SwapObjectTexture::Request swap;
  swap.tag = "target";
  swap.tex_id = 2;

  EXPECT_TRUE(set_seg.is_name_regex);
  EXPECT_EQ(get_seg.segmentation_id, 42);
  EXPECT_EQ(seg_map.segmentation_map_json, "{}");
  EXPECT_EQ(material.material_asset_path, "/Game/Materials/M_Test");
  EXPECT_EQ(texture.source,
            projectairsim_ros2_cpp::srv::SetObjectTexture::Request::SOURCE_URL);
  EXPECT_EQ(swap.tex_id, 2);
}

TEST(Ros2GeneratedInterfacesRegression, MoveOnPathActionIsGenerated) {
  projectairsim_ros2_cpp::action::MoveOnPath::Goal goal;
  geometry_msgs::msg::PoseStamped waypoint;
  waypoint.pose.position.x = 5.0;
  waypoint.pose.position.y = 5.0;
  waypoint.pose.position.z = -3.0;
  waypoint.pose.orientation.w = 1.0;
  goal.path.push_back(waypoint);
  goal.velocity = 2.0;
  goal.timeout_sec = 20.0F;
  goal.lookahead = -1.0F;
  goal.adaptive_lookahead = 1.0F;
  goal.drive_train_type = 0;
  goal.yaw_is_rate = true;
  goal.yaw = 0.0F;
  goal.wait_on_last_task = true;

  projectairsim_ros2_cpp::action::MoveOnPath::Feedback feedback;
  feedback.current_point_on_path = "submitted";

  projectairsim_ros2_cpp::action::MoveOnPath::Result result;
  result.success = true;

  EXPECT_EQ(goal.path.size(), 1u);
  EXPECT_EQ(feedback.current_point_on_path, "submitted");
  EXPECT_TRUE(result.success);
}

}  // namespace
