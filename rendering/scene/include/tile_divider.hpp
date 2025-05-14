// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef RENDERING_SCENE_INCLUDE_TILE_DIVIDER_HPP_
#define RENDERING_SCENE_INCLUDE_TILE_DIVIDER_HPP_

#include <functional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "core_sim/math_utils.hpp"
#include "tile_info.hpp"

SCENE_BEGIN_NAMESPACE

using Box2d = microsoft::projectairsim::Box2d;
using Vector2d = microsoft::projectairsim::Vector2d;
using Vector3d = microsoft::projectairsim::Vector3d;

class TileDivider {
 public:
  enum class Quality { HIGH, MEDIUM, LOW, SUPERLOW };

  TileDivider(int min_tile_lod = 0, int max_tile_lod = 0,
              Quality quality = Quality::HIGH,
              GeoLocation southwest_bound = GeoLocation(-85.0, -180.0, 0.f),
              GeoLocation northeast_bound = GeoLocation(85.0, 180.0, 0.f));

  std::vector<TileKey> GetKeys(
      const std::vector<GeoLocation>& observer_locations) const;

 private:
  enum class TileAvailable { SUFFICIENT, NOTSUFFICIENT, OUTOFBOUNDS };

  void GetKeysImpl(const std::vector<GeoLocation>& observer_locations,
                   const TileKey& cur_key,
                   std::vector<std::TileKey>& out_keys) const;

  TileAvailable IsTileSufficient(const GeoLocation& observer_geo_loc,
                                 const TileKey& key) const;

  int min_tile_lod_;
  int max_tile_lod_;
  Quality quality_;
  Box2d world_bounds_;
};

SCENE_END_NAMESPACE

#endif  // RENDERING_SCENE_INCLUDE_TILE_DIVIDER_HPP_