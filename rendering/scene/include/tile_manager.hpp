// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef RENDERING_SCENE_INCLUDE_TILE_MANAGER_HPP_
#define RENDERING_SCENE_INCLUDE_TILE_MANAGER_HPP_

#include <vector>

#include "core_sim/geodetic_converter.hpp"
#include "mesh_data_provider.hpp"
#include "scene_global.hpp"
#include "tile_cache.hpp"
#include "tile_divider.hpp"

namespace microsoft {
namespace projectairsim {

class Robot;

}  // namespace projectairsim
}  // namespace microsoft

SCENE_BEGIN_NAMESPACE

class TileManager {
 public:
  TileManager(const GeoLocation& center_geo_point, int min_lod, 
              int max_lod,
              TileDivider::Quality quality = TileDivider::Quality::HIGH);

  ~TileManager();

  void AddRobot(const Robot& sim_robot);

  void RemoveAllRobots();

  std::vector<TileKey> GetTileKeysToRender() const;

  int min_tile_lod_;
  int max_tile_lod_;

 private:
  GeoLocation CalculateSouthWestBounds(const GeoLocation& home_geo_point);

  GeoLocation CalculateNorthEastBounds(const GeoLocation& home_geo_point);

  TileDivider tile_divider_;
  GeodeticConverter geo_converter_;
  std::vector<std::reference_wrapper<const Robot>> sim_robots_;

  // 200 sq km = 14 x 14 km square region
  static constexpr float kBoundsRadiusInMeters = 14.0 * 1000;
};

SCENE_END_NAMESPACE

#endif  // RENDERING_SCENE_INCLUDE_TILE_MANAGER_HPP_