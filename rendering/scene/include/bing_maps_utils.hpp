// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef RENDERING_SCENE_INCLUDE_BING_MAP_UTILS_HPP_
#define RENDERING_SCENE_INCLUDE_BING_MAP_UTILS_HPP_

#include "core_sim/math_utils.hpp"
#include "scene_global.hpp"

SCENE_BEGIN_NAMESPACE

class BingMapsUtils {
 public:
  // Static utility methods
  static void LatLongToTileXY(double lat, double lon, int lod, int* tile_x_out,
                              int* tile_y_out);

  static void PixelXYToLatLong(int pixel_x, int pixel_y, int lod,
                               double& latitude, double& longitude);

  static void TileXYToLatLong(int tile_x, int tile_y, int lod, double& latitude,
                              double& longitude);

  static std::string TileXYToQuadkey(int tile_x, int tile_y, int lod);

  static uint64_t TileIdFromQuadkey(const std::string& tile_id_str);

  static bool TryCreateTilePosition(uint64_t quadKeyValue, int* x, int* y,
                                    int* levelOfDetail);
};

SCENE_END_NAMESPACE

#endif  // RENDERING_SCENE_INCLUDE_BING_MAP_UTILS_HPP_