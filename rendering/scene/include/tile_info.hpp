// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef RENDERING_SCENE_INCLUDE_TILE_INFO_HPP_
#define RENDERING_SCENE_INCLUDE_TILE_INFO_HPP_

#include <functional>
#include <tuple>

#include "core_sim/earth_utils.hpp"
#include "core_sim/math_utils.hpp"
#include "scene_global.hpp"

SCENE_BEGIN_NAMESPACE

using GeoLocation = microsoft::projectairsim::GeoPoint;

// Potentially will be useful for Quad (or KD) Tree impl, hence adding operators
struct TileKey {
  int x;
  int y;
  int lod;

  TileKey() : TileKey(0, 0, 0) {}
  TileKey(int in_x, int in_y, int in_lod) : x(in_x), y(in_y), lod(in_lod) {}

  bool operator<(const TileKey& r) const {
    return std::tie(lod, x, y) < std::tie(r.lod, r.x, r.y);
  }

  bool operator>(const TileKey& r) const {
    return std::tie(lod, x, y) > std::tie(r.lod, r.x, r.y);
  }

  bool operator==(const TileKey& r) const {
    return std::tie(lod, x, y) == std::tie(r.lod, r.x, r.y);
  }

  // expects an array of size 4
  void GetChildTiles(TileKey* output) const {
    int new_x = x * 2;
    int new_y = y * 2;
    int new_lod = lod + 1;
    output[0] = TileKey(new_x, new_y, new_lod);
    output[1] = TileKey(new_x + 1, new_y, new_lod);
    output[2] = TileKey(new_x, new_y + 1, new_lod);
    output[3] = TileKey(new_x + 1, new_y + 1, new_lod);
  }
};

SCENE_END_NAMESPACE

namespace std {

using TileKey = microsoft::projectairsim::rendering::scene::TileKey;

template <>
struct hash<TileKey> {
  using argument_type = TileKey;
  using result_type = size_t;

  size_t operator()(const TileKey& r) const {
    const std::hash<int> int_hash_fn;

    const size_t result =
        int_hash_fn(r.x) ^ int_hash_fn(r.y) ^ int_hash_fn(r.lod);

    return result;
  }
};
};  // namespace std

#endif  // RENDERING_SCENE_INCLUDE_TILE_INFO_HPP_