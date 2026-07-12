#include <gtest/gtest.h>

#include <string>
#include <type_traits>
#include <vector>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "projectairsim_ros2_cpp/action/move_on_path.hpp"
#include "projectairsim_ros2_cpp/msg/radar_scan.hpp"
#include "projectairsim_ros2_cpp/msg/radar_tracks.hpp"
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
#include "rosgraph_msgs/msg/clock.hpp"
#include "rosidl_runtime_cpp/traits.hpp"
#include "sensor_msgs/msg/camera_info.hpp"
#include "sensor_msgs/msg/fluid_pressure.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "sensor_msgs/msg/magnetic_field.hpp"
#include "sensor_msgs/msg/nav_sat_fix.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "std_msgs/msg/string.hpp"

namespace {

template <typename T>
std::string RosTypeName() {
  return rosidl_generator_traits::name<T>();
}

TEST(Ros2Contracts, StandardTypedTopicMessageContracts) {
  EXPECT_EQ(RosTypeName<std_msgs::msg::String>(), "std_msgs/msg/String");
  EXPECT_EQ(RosTypeName<geometry_msgs::msg::PoseStamped>(),
            "geometry_msgs/msg/PoseStamped");
  EXPECT_EQ(RosTypeName<sensor_msgs::msg::NavSatFix>(),
            "sensor_msgs/msg/NavSatFix");
  EXPECT_EQ(RosTypeName<sensor_msgs::msg::Imu>(), "sensor_msgs/msg/Imu");
  EXPECT_EQ(RosTypeName<sensor_msgs::msg::FluidPressure>(),
            "sensor_msgs/msg/FluidPressure");
  EXPECT_EQ(RosTypeName<sensor_msgs::msg::MagneticField>(),
            "sensor_msgs/msg/MagneticField");
  EXPECT_EQ(RosTypeName<sensor_msgs::msg::PointCloud2>(),
            "sensor_msgs/msg/PointCloud2");
  EXPECT_EQ(RosTypeName<sensor_msgs::msg::Image>(), "sensor_msgs/msg/Image");
  EXPECT_EQ(RosTypeName<sensor_msgs::msg::CameraInfo>(),
            "sensor_msgs/msg/CameraInfo");
  EXPECT_EQ(RosTypeName<projectairsim_ros2_cpp::msg::RadarScan>(),
            "projectairsim_ros2_cpp/msg/RadarScan");
  EXPECT_EQ(RosTypeName<projectairsim_ros2_cpp::msg::RadarTracks>(),
            "projectairsim_ros2_cpp/msg/RadarTracks");
  EXPECT_EQ(RosTypeName<rosgraph_msgs::msg::Clock>(),
            "rosgraph_msgs/msg/Clock");
}

TEST(Ros2Contracts, BridgeTopicNamesMatchExpectedSurface) {
  const std::vector<std::string> required_topics = {
      "/projectairsim/topic_info",
      "/clock",
      "/ProjectAirsim/<scene>/robots/<vehicle>/actual_pose",
      "/ProjectAirsim/<scene>/robots/<vehicle>/sensors/<sensor>/gps",
      "/ProjectAirsim/<scene>/robots/<vehicle>/sensors/<sensor>/imu",
      "/ProjectAirsim/<scene>/robots/<vehicle>/sensors/<sensor>/barometer",
      "/ProjectAirsim/<scene>/robots/<vehicle>/sensors/<sensor>/magnetometer",
      "/ProjectAirsim/<scene>/robots/<vehicle>/sensors/<sensor>/lidar",
      "/ProjectAirsim/<scene>/robots/<vehicle>/sensors/<sensor>/radar_detections",
      "/ProjectAirsim/<scene>/robots/<vehicle>/sensors/<sensor>/radar_tracks",
      "/ProjectAirsim/<scene>/robots/<vehicle>/sensors/<camera>/<image>/camera_info",
  };

  EXPECT_EQ(required_topics.size(), 11u);
  EXPECT_EQ(required_topics.front(), "/projectairsim/topic_info");
  EXPECT_EQ(required_topics[1], "/clock");
}

TEST(Ros2Contracts, SingleVehicleServiceContracts) {
  using projectairsim_ros2_cpp::srv::Arm;
  using projectairsim_ros2_cpp::srv::Disarm;
  using projectairsim_ros2_cpp::srv::Land;
  using projectairsim_ros2_cpp::srv::MoveOnPath;
  using projectairsim_ros2_cpp::srv::MoveToPosition;
  using projectairsim_ros2_cpp::srv::Takeoff;

  Takeoff::Request takeoff_request;
  takeoff_request.wait_on_last_task = true;
  Takeoff::Response takeoff_response;
  takeoff_response.success = true;
  EXPECT_TRUE(takeoff_request.wait_on_last_task);
  EXPECT_TRUE(takeoff_response.success);

  Land::Request land_request;
  land_request.wait_on_last_task = true;
  Land::Response land_response;
  land_response.success = true;
  EXPECT_TRUE(land_request.wait_on_last_task);
  EXPECT_TRUE(land_response.success);

  Arm::Request arm_request;
  arm_request.wait_on_last_task = true;
  Arm::Response arm_response;
  arm_response.success = true;
  EXPECT_TRUE(arm_request.wait_on_last_task);
  EXPECT_TRUE(arm_response.success);

  Disarm::Request disarm_request;
  disarm_request.wait_on_last_task = true;
  Disarm::Response disarm_response;
  disarm_response.success = true;
  EXPECT_TRUE(disarm_request.wait_on_last_task);
  EXPECT_TRUE(disarm_response.success);

  MoveToPosition::Request move_to_position_request;
  move_to_position_request.x = 5.0;
  move_to_position_request.y = 0.0;
  move_to_position_request.z = -3.0;
  move_to_position_request.velocity = 2.0;
  move_to_position_request.timeout_sec = 10.0F;
  move_to_position_request.drive_train_type = 0;
  move_to_position_request.yaw_is_rate = true;
  move_to_position_request.yaw = 0.0F;
  move_to_position_request.lookahead = -1.0F;
  move_to_position_request.adaptive_lookahead = 1.0F;
  move_to_position_request.wait_on_last_task = true;
  EXPECT_DOUBLE_EQ(move_to_position_request.x, 5.0);
  EXPECT_FLOAT_EQ(move_to_position_request.timeout_sec, 10.0F);

  MoveOnPath::Request move_on_path_request;
  geometry_msgs::msg::PoseStamped waypoint;
  waypoint.pose.position.x = 5.0;
  waypoint.pose.position.y = 0.0;
  waypoint.pose.position.z = -3.0;
  waypoint.pose.orientation.w = 1.0;
  move_on_path_request.path.push_back(waypoint);
  move_on_path_request.velocity = 2.0;
  move_on_path_request.timeout_sec = 20.0F;
  move_on_path_request.wait_on_last_task = true;
  EXPECT_EQ(move_on_path_request.path.size(), 1u);
  EXPECT_DOUBLE_EQ(move_on_path_request.path[0].pose.position.x, 5.0);
}

TEST(Ros2Contracts, GroupServiceContracts) {
  using projectairsim_ros2_cpp::srv::ArmGroup;
  using projectairsim_ros2_cpp::srv::DisarmGroup;
  using projectairsim_ros2_cpp::srv::LandGroup;
  using projectairsim_ros2_cpp::srv::TakeoffGroup;

  TakeoffGroup::Request takeoff_request;
  takeoff_request.vehicle_names = {"Drone1", "Drone2"};
  takeoff_request.wait_on_last_task = true;
  EXPECT_EQ(takeoff_request.vehicle_names.size(), 2u);

  LandGroup::Request land_request;
  land_request.vehicle_names = {"Drone1", "Drone2"};
  land_request.wait_on_last_task = true;
  EXPECT_EQ(land_request.vehicle_names[1], "Drone2");

  ArmGroup::Request arm_request;
  arm_request.vehicle_names = {"Drone1"};
  arm_request.wait_on_last_task = true;
  EXPECT_EQ(arm_request.vehicle_names[0], "Drone1");

  DisarmGroup::Request disarm_request;
  disarm_request.vehicle_names = {"Drone1"};
  disarm_request.wait_on_last_task = true;
  EXPECT_EQ(disarm_request.vehicle_names[0], "Drone1");
}

TEST(Ros2Contracts, WorldAndSceneServiceContracts) {
  using projectairsim_ros2_cpp::srv::CreateVoxelGrid;
  using projectairsim_ros2_cpp::srv::GetClock;
  using projectairsim_ros2_cpp::srv::GetOriginGeoPoint;
  using projectairsim_ros2_cpp::srv::LoadScene;
  using projectairsim_ros2_cpp::srv::OccupancyGrid;
  using projectairsim_ros2_cpp::srv::Reset;

  LoadScene::Request load_scene_request;
  load_scene_request.scene_file = "scene_drone_sensors.jsonc";
  load_scene_request.is_primary_client = true;
  EXPECT_EQ(load_scene_request.scene_file, "scene_drone_sensors.jsonc");

  Reset::Request reset_request;
  reset_request.wait_on_last_task = true;
  EXPECT_TRUE(reset_request.wait_on_last_task);

  CreateVoxelGrid::Request voxel_request;
  voxel_request.position_x = 0.0;
  voxel_request.position_y = 0.0;
  voxel_request.position_z = -4.0;
  voxel_request.ncells_x = 20;
  voxel_request.ncells_y = 20;
  voxel_request.ncells_z = 10;
  voxel_request.resolution = 1.0;
  voxel_request.n_z_resolution = 10;
  voxel_request.output_file = "/tmp/projectairsim_voxel_grid.binvox";
  EXPECT_EQ(voxel_request.ncells_x, 20);
  EXPECT_EQ(voxel_request.output_file, "/tmp/projectairsim_voxel_grid.binvox");

  OccupancyGrid::Request occupancy_request;
  occupancy_request.position_z = -4.0;
  occupancy_request.ncells_x = 20;
  occupancy_request.ncells_y = 20;
  occupancy_request.ncells_z = 10;
  occupancy_request.res = 1.0;
  OccupancyGrid::Response occupancy_response;
  occupancy_response.success = true;
  occupancy_response.map.info.resolution = 1.0F;
  occupancy_response.map.info.width = 20;
  occupancy_response.map.info.height = 20;
  EXPECT_TRUE(occupancy_response.success);
  EXPECT_EQ(occupancy_response.map.info.width, 20u);

  GetClock::Response clock_response;
  clock_response.nanosec = 123456789;
  EXPECT_EQ(clock_response.nanosec, 123456789);

  GetOriginGeoPoint::Response origin_response;
  origin_response.latitude = 47.641468;
  origin_response.longitude = -122.140165;
  origin_response.altitude = 122.0;
  EXPECT_NEAR(origin_response.latitude, 47.641468, 1.0e-9);
}

TEST(Ros2Contracts, GenericClientServiceContracts) {
  using projectairsim_ros2_cpp::srv::CancelAllRequests;
  using projectairsim_ros2_cpp::srv::GetClientInfo;
  using projectairsim_ros2_cpp::srv::GetDrones;
  using projectairsim_ros2_cpp::srv::GetTopicInfo;
  using projectairsim_ros2_cpp::srv::Publish;
  using projectairsim_ros2_cpp::srv::RawRequest;
  using projectairsim_ros2_cpp::srv::Unsubscribe;

  RawRequest::Request request;
  request.method = "/Sim/GetBuildCommitHash";
  request.json_parameters = "{}";
  RawRequest::Response response;
  response.success = true;
  response.error_code = 0;
  response.result_json = "\"abc123\"";
  EXPECT_EQ(request.method, "/Sim/GetBuildCommitHash");
  EXPECT_EQ(response.error_code, 0);

  GetClientInfo::Response client_info;
  client_info.client_version = "1.0";
  client_info.nng_version = "1.5";
  client_info.build_commit_hash = "abc123";
  client_info.success = true;
  EXPECT_TRUE(client_info.success);
  EXPECT_EQ(client_info.build_commit_hash, "abc123");

  GetTopicInfo::Response topic_info;
  topic_info.topics = {"/Sim/topics/example"};
  topic_info.topics_json = "[\"/Sim/topics/example\"]";
  topic_info.success = true;
  EXPECT_EQ(topic_info.topics.size(), 1u);

  GetDrones::Response get_drones_response;
  get_drones_response.drones = {"Drone1", "Drone2"};
  get_drones_response.success = true;
  EXPECT_EQ(get_drones_response.drones[0], "Drone1");

  Publish::Request publish_request;
  publish_request.topic = "/Sim/topics/example";
  publish_request.json_message = "{\"value\":1}";
  Publish::Response publish_response;
  publish_response.success = true;
  publish_response.status = "OK";
  EXPECT_EQ(publish_request.topic, "/Sim/topics/example");
  EXPECT_TRUE(publish_response.success);

  Unsubscribe::Request unsubscribe_request;
  unsubscribe_request.topics = {"/Sim/topics/example"};
  unsubscribe_request.all = false;
  Unsubscribe::Response unsubscribe_response;
  unsubscribe_response.success = true;
  unsubscribe_response.status = "OK";
  EXPECT_FALSE(unsubscribe_request.all);
  EXPECT_TRUE(unsubscribe_response.success);

  CancelAllRequests::Response cancel_response;
  cancel_response.success = true;
  EXPECT_TRUE(cancel_response.success);
}

TEST(Ros2Contracts, WorldTypedExtensionServiceContracts) {
  using projectairsim_ros2_cpp::srv::GetSegmentationIDByName;
  using projectairsim_ros2_cpp::srv::GetSegmentationIDMap;
  using projectairsim_ros2_cpp::srv::SetObjectMaterial;
  using projectairsim_ros2_cpp::srv::SetObjectTexture;
  using projectairsim_ros2_cpp::srv::SetSegmentationIDByName;
  using projectairsim_ros2_cpp::srv::SwapObjectTexture;

  SetSegmentationIDByName::Request set_seg;
  set_seg.mesh_name = "Car.*";
  set_seg.segmentation_id = 42;
  set_seg.is_name_regex = true;
  set_seg.use_owner_name = false;
  EXPECT_TRUE(set_seg.is_name_regex);

  GetSegmentationIDByName::Response get_seg;
  get_seg.success = true;
  get_seg.segmentation_id = 42;
  EXPECT_EQ(get_seg.segmentation_id, 42);

  GetSegmentationIDMap::Response seg_map;
  seg_map.success = true;
  seg_map.segmentation_map_json = "{}";
  EXPECT_EQ(seg_map.segmentation_map_json, "{}");

  SetObjectMaterial::Request material;
  material.object_name = "Cube";
  material.material_asset_path = "/Game/Materials/M_Test";
  EXPECT_EQ(material.object_name, "Cube");

  SetObjectTexture::Request texture;
  texture.object_name = "Cube";
  texture.source = SetObjectTexture::Request::SOURCE_URL;
  texture.texture = "https://example.invalid/texture.png";
  EXPECT_EQ(texture.source, SetObjectTexture::Request::SOURCE_URL);

  SwapObjectTexture::Request swap;
  swap.tag = "target";
  swap.tex_id = 2;
  EXPECT_EQ(swap.tex_id, 2);
}

TEST(Ros2Contracts, MoveOnPathActionContract) {
  using projectairsim_ros2_cpp::action::MoveOnPath;

  MoveOnPath::Goal goal;
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

  MoveOnPath::Feedback feedback;
  feedback.current_point_on_path = "submitted";

  MoveOnPath::Result result;
  result.success = true;

  EXPECT_EQ(goal.path.size(), 1u);
  EXPECT_EQ(feedback.current_point_on_path, "submitted");
  EXPECT_TRUE(result.success);
}

TEST(Ros2Contracts, ServiceNameSurfaceMatchesBridgeDefaults) {
  const std::vector<std::string> services = {
      "/projectairsim/request",
      "/projectairsim/get_client_info",
      "/projectairsim/get_topic_info",
      "/projectairsim/get_drones",
      "/projectairsim/publish",
      "/projectairsim/unsubscribe",
      "/projectairsim/cancel_all_requests",
      "/projectairsim/Drone1/takeoff",
      "/projectairsim/Drone1/land",
      "/projectairsim/Drone1/arm",
      "/projectairsim/Drone1/disarm",
      "/projectairsim/Drone1/move_to_position",
      "/projectairsim/Drone1/move_on_path",
      "/projectairsim/takeoff_group",
      "/projectairsim/land_group",
      "/projectairsim/arm_group",
      "/projectairsim/disarm_group",
      "/projectairsim/reset",
      "/projectairsim/load_scene",
      "/projectairsim/create_voxel_grid",
      "/projectairsim/occupancy_grid",
      "/projectairsim/get_clock",
      "/projectairsim/get_origin_geo_point",
      "/projectairsim/set_segmentation_id_by_name",
      "/projectairsim/get_segmentation_id_by_name",
      "/projectairsim/get_segmentation_id_map",
      "/projectairsim/set_object_material",
      "/projectairsim/set_object_texture",
      "/projectairsim/swap_object_texture",
  };

  EXPECT_EQ(services.size(), 29u);
  EXPECT_EQ(services.front(), "/projectairsim/request");
  EXPECT_EQ(services.back(), "/projectairsim/swap_object_texture");
}

}  // namespace
