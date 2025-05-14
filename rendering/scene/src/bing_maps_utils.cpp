// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "bing_maps_utils.hpp"

#include <algorithm>

#include "core_sim/math_utils.hpp"

SCENE_BEGIN_NAMESPACE

void BingMapsUtils::LatLongToTileXY(double lat, double lon, int lod,
                                    int* tile_x_out, int* tile_y_out) {
  if (tile_x_out == nullptr || tile_y_out == nullptr) return;

  // Bing Maps value limits
  static constexpr double kMinLatitude = -85.05112878;
  static constexpr double kMaxLatitude = 85.05112878;
  static constexpr double kMinLongitude = -180.0;
  static constexpr double kMaxLongitude = 180.0;
  static constexpr int kMinLOD = 1;
  static constexpr int kMaxLOD = 23;
  // is max LOD actually 19?
  // https://docs.microsoft.com/en-us/bingmaps/articles/understanding-scale-and-resolution#calculating-resolution

  const double lat_clip = std::min(std::max(lat, kMinLatitude), kMaxLatitude);
  const double lon_clip = std::min(std::max(lon, kMinLongitude), kMaxLongitude);
  const int lod_clip = std::min(std::max(lod, kMinLOD), kMaxLOD);

  const double x = (lon_clip + 180) / 360;
  const double sin_lat = std::sin(lat_clip * M_PI / 180);
  const double y = 0.5 - std::log((1 + sin_lat) / (1 - sin_lat)) / (4 * M_PI);

  // Map width/height pixels = 256 * 2^lod
  const int map_size_pixels = 256 << lod_clip;
  const auto pixel_x = static_cast<int>(x * map_size_pixels + 0.5);
  const auto pixel_y = static_cast<int>(y * map_size_pixels + 0.5);

  const int pixel_x_clip = std::min(std::max(pixel_x, 0), map_size_pixels - 1);
  const int pixel_y_clip = std::min(std::max(pixel_y, 0), map_size_pixels - 1);

  *tile_x_out = pixel_x_clip / 256;
  *tile_y_out = pixel_y_clip / 256;
}

std::string BingMapsUtils::TileXYToQuadkey(int tile_x, int tile_y, int lod) {
  std::string quadkey = "";
  for (int i_lod = lod; i_lod > 0; --i_lod) {
    int digit = 0;
    const int mask = 1 << (i_lod - 1);
    if ((tile_x & mask) != 0) digit += 1;
    if ((tile_y & mask) != 0) digit += 2;
    quadkey += std::to_string(digit);
  }

  return quadkey;
}

uint64_t BingMapsUtils::TileIdFromQuadkey(const std::string& quadkey) {
  uint64_t result = 1;  // leader bit
  for (int i = 0; i < quadkey.length(); ++i) {
    const int quad_val = quadkey[i] - '0';
    if (quad_val < 0 || quad_val > 3) {
      throw std::invalid_argument("Invalid quadkey.");
    }

    result <<= 2;
    result |= static_cast<uint64_t>(quad_val);
  }

  return result;
}

void BingMapsUtils::PixelXYToLatLong(int pixel_x, int pixel_y, int lod,
                                     double& latitude, double& longitude) {
  unsigned int map_size = 256 << lod;
  double x = (std::clamp<double>(pixel_x, 0, map_size) / map_size) - 0.5;
  double y = 0.5 - (std::clamp<double>(pixel_y, 0, map_size) / map_size);

  latitude = 90 - 360 * std::atan(std::exp(-y * 2 * M_PI)) / M_PI;
  longitude = 360 * x;
}

void BingMapsUtils::TileXYToLatLong(int tile_x, int tile_y, int lod,
                                    double& latitude, double& longitude) {
  PixelXYToLatLong(tile_x * 256, tile_y * 256, lod, latitude, longitude);
}

bool BingMapsUtils::TryCreateTilePosition(uint64_t quadKeyValue, int* x, int* y,
                           int* levelOfDetail) {
  // Calculate the level of detail based on the marker bit.
  uint64_t tempQuadKeyValue = quadKeyValue;
  int internalLevelOfDetail = 0;
  while (tempQuadKeyValue != 1) {
    if (tempQuadKeyValue == 0) {
      return false;
    }

    tempQuadKeyValue >>= 2;
    ++internalLevelOfDetail;
  }

  // Calculate X and Y from the bits.
  int internalX = 0;
  int internalY = 0;
  for (int i = 0; i < internalLevelOfDetail; ++i) {
    internalX <<= 1;
    internalX |= static_cast<int>(
        (quadKeyValue >> ((internalLevelOfDetail - i - 1) * 2)) & 1);
    internalY <<= 1;
    internalY |= static_cast<int>(
        (quadKeyValue >> ((internalLevelOfDetail - i - 1) * 2 + 1)) & 1);
  }

  *levelOfDetail = internalLevelOfDetail;
  *x = internalX;
  *y = internalY;
  return true;
}

SCENE_END_NAMESPACE