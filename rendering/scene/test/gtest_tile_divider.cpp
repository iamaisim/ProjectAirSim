#include "bing_maps_utils.hpp"
#include "core_sim/math_utils.hpp"
#include "gtest/gtest.h"
#include "tile_divider.hpp"

SCENE_BEGIN_NAMESPACE

using namespace projectairsim::rendering::scene;

// Testcase 1, make sure we get at least one tile whose x,y match as bingmap
// conversion will give
TEST(TileDivider, GetKeys_Test1) {
  std::vector<std::pair<double, double>> lat_lon_list = {
      {32, -117}, {32, 117}, {-32, 117}, {-32, -117}};

  for (auto [lat, lon] : lat_lon_list) {
    for (int i = 1; i < 19; i++) {
      int robot_tile_x, robot_tile_y;
      BingMapsUtils::LatLongToTileXY(lat, lon, i, &robot_tile_x, &robot_tile_y);

      TileDivider tile_divider(i, i, TileDivider::Quality::HIGH,
                               GeoLocation(lat - 0.1, lon - 0.1, 0.f),
                               GeoLocation(lat + 0.1, lon + 0.1, 0.f));
      auto keys = tile_divider.GetKeys(
          std::vector<GeoLocation>{GeoLocation(lat, lon, .1)});

      bool robot_pos_found = false;

      for (int i = 0; i < keys.size(); ++i) {
        if (keys[i].x == robot_tile_x && keys[i].y == robot_tile_y) {
          robot_pos_found = true;
          break;
        }
      }

      EXPECT_TRUE(robot_pos_found);
    }
  }
}

// Testcase 2, make sure we get tiles from divider that are in range of what
// bingmap will give for given lat/lon/lod
TEST(TileDivider, GetKeys_Test2) {
  std::vector<TileKey> robot_tiles;

  // get list of robot_xy for each lod
  for (int i = 15; i <= 19; i++) {
    int robot_tile_x, robot_tile_y;
    BingMapsUtils::LatLongToTileXY(32, -117, i, &robot_tile_x, &robot_tile_y);
    robot_tiles.push_back(TileKey(robot_tile_x, robot_tile_y, i));
  }

  TileDivider tile_divider(13, 19, TileDivider::Quality::LOW,
                           GeoLocation(32 - 0.1, -117 - 0.1, 0.f),
                           GeoLocation(32 + 0.1, -117 + 0.1, 0.f));
  auto keys =
      tile_divider.GetKeys(std::vector<GeoLocation>{GeoLocation(32, -117, 4)});
  sort(keys.begin(), keys.end());

  for (auto key : robot_tiles) {
    auto itr = std::find_if(keys.begin(), keys.end(), [key](TileKey key_in) {
      if (key_in.lod == key.lod) {
        if (key.x > key_in.x - 10 && key.x < key_in.x + 10 &&
            key.y > key_in.y - 10 && key.y < key_in.y + 10) {
          return true;
        }
      }
      return false;
    });

    EXPECT_TRUE(itr != keys.end());
  }
}

SCENE_END_NAMESPACE