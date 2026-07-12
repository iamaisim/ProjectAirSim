#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::filesystem::path PackageRoot() {
  return std::filesystem::path(__FILE__).parent_path().parent_path();
}

std::string ReadFile(const std::filesystem::path& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    ADD_FAILURE() << "Unable to open " << path;
    return {};
  }

  std::ostringstream contents;
  contents << file.rdbuf();
  return contents.str();
}

void ExpectContains(const std::string& haystack, const std::string& needle) {
  EXPECT_NE(haystack.find(needle), std::string::npos)
      << "Missing expected text: " << needle;
}

TEST(Ros2BridgeSurfaceRegression, InterfacesAreDeclaredAndRegistered) {
  const auto package_root = PackageRoot();
  const auto cmake = ReadFile(package_root / "CMakeLists.txt");

  const std::vector<std::string> expected_interfaces = {
      "msg/RadarReturn.msg",
      "msg/RadarScan.msg",
      "msg/RadarTrack.msg",
      "msg/RadarTracks.msg",
      "srv/GetClock.srv",
      "srv/GetSegmentationIDByName.srv",
      "srv/GetSegmentationIDMap.srv",
      "srv/SetObjectMaterial.srv",
      "srv/SetObjectTexture.srv",
      "srv/SetSegmentationIDByName.srv",
      "srv/SwapObjectTexture.srv",
  };

  for (const auto& interface : expected_interfaces) {
    EXPECT_TRUE(std::filesystem::is_regular_file(package_root / interface))
        << "Missing " << interface;
    ExpectContains(cmake, "\"" + interface + "\"");
  }
}

TEST(Ros2BridgeSurfaceRegression, BridgeSourceKeepsConversionSurface) {
  const auto source =
      ReadFile(PackageRoot() / "src" / "projectairsim_ros2_cpp_node.cpp");

  const std::vector<std::string> expected_tokens = {
      "create_publisher<rosgraph_msgs::msg::Clock>(\"/clock\"",
      "std::make_unique<tf2_ros::TransformBroadcaster>",
      "EndsWith(topic, \"/radar_detections\")",
      "CreateRadarScanPublisher(topic)",
      "EndsWith(topic, \"/radar_tracks\")",
      "CreateRadarTracksPublisher(topic)",
      "EndsWith(topic, \"_camera_info\")",
      "CreateCameraInfoPublisher(topic)",
      "CameraInfoRosTopic(image_ros_topic)",
      "AdoptLoadedSceneIfAvailable()",
      "world_->Initialize(client_)",
      "if (!is_primary_client)",
      "subscriptions_mutex_",
      "TopicHandler handler;",
      "\"SetSegmentationIDByName\"",
      "\"GetSegmentationIDByName\"",
      "\"GetSegmentationIDMap\"",
      "\"SetObjectMaterial\"",
      "\"SetObjectTextureFromUrl\"",
      "\"SetObjectTextureFromFile\"",
      "\"SetObjectTextureFromPackagedAsset\"",
      "\"SwapObjectTexture\"",
  };

  for (const auto& token : expected_tokens) {
    ExpectContains(source, token);
  }

  const std::vector<std::string> expected_service_names = {
      "/projectairsim/request",
      "/projectairsim/get_client_info",
      "/projectairsim/get_topic_info",
      "/projectairsim/get_drones",
      "/projectairsim/publish",
      "/projectairsim/unsubscribe",
      "/projectairsim/cancel_all_requests",
      "/projectairsim/get_clock",
      "/projectairsim/set_segmentation_id_by_name",
      "/projectairsim/get_segmentation_id_by_name",
      "/projectairsim/get_segmentation_id_map",
      "/projectairsim/set_object_material",
      "/projectairsim/set_object_texture",
      "/projectairsim/swap_object_texture",
  };

  for (const auto& service : expected_service_names) {
    const auto service_suffix = service.substr(std::string("/projectairsim").size());
    const auto direct = source.find(service) != std::string::npos;
    const auto rooted =
        source.find("service_root_ + \"" + service_suffix + "\"") !=
        std::string::npos;
    EXPECT_TRUE(direct || rooted) << "Missing service " << service;
  }
}

}  // namespace
