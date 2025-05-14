// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "tile_manager.hpp"

#include "core_sim/actor/robot.hpp"

SCENE_BEGIN_NAMESPACE

TileManager::TileManager(const GeoLocation& center_geo_point, int min_lod,
                         int max_lod, TileDivider::Quality quality) {
  geo_converter_ =
      GeodeticConverter(center_geo_point.latitude, center_geo_point.longitude,
                        center_geo_point.altitude);

  GeoLocation southwest_bound = CalculateSouthWestBounds(center_geo_point);
  GeoLocation northeast_bound = CalculateNorthEastBounds(center_geo_point);

  tile_divider_ =
      TileDivider(min_lod, max_lod, quality, southwest_bound, northeast_bound);

  min_tile_lod_ = min_lod;
  max_tile_lod_ = max_lod;
}

std::vector<TileKey> TileManager::GetTileKeysToRender() const {
  // Calculate geolocations for all of the robot positions
  std::vector<GeoLocation> robot_geo_locations;
  for (const auto& robot : sim_robots_) {
    auto robot_loc = robot.get().GetKinematics().pose.position;
    double robot_lat, robot_lon;
    float robot_alt;
    geo_converter_.ned2Geodetic(robot_loc.x(), robot_loc.y(), robot_loc.z(),
                                &robot_lat, &robot_lon, &robot_alt);
    robot_geo_locations.emplace_back(robot_lat, robot_lon, robot_alt);
  }

  std::vector<TileKey> keys = tile_divider_.GetKeys(robot_geo_locations);

  // sort mayn't be the best idea here but I would say decent enough
  // we could also use min heap (heapify)
  std::sort(keys.begin(), keys.end(), std::greater<TileKey>());

  return keys;
}

TileManager::~TileManager() {}

void TileManager::AddRobot(const Robot& sim_robot) {
  sim_robots_.emplace_back(sim_robot);
}

void TileManager::RemoveAllRobots() { sim_robots_.clear(); }

GeoLocation TileManager::CalculateSouthWestBounds(
    const GeoLocation& home_geo_point) {
  GeoLocation p;
  p.latitude = home_geo_point.latitude -
               TransformUtils::ToDegrees(kBoundsRadiusInMeters /
                                         EarthUtils::kEarthRadius);

  p.longitude =
      home_geo_point.longitude -
      TransformUtils::ToDegrees(kBoundsRadiusInMeters /
                                EarthUtils::kEarthRadius) /
          std::cos(TransformUtils::ToRadians(home_geo_point.latitude));

  return p;
}

GeoLocation TileManager::CalculateNorthEastBounds(
    const GeoLocation& home_geo_point) {
  GeoLocation p;
  p.latitude = home_geo_point.latitude +
               TransformUtils::ToDegrees(kBoundsRadiusInMeters /
                                         EarthUtils::kEarthRadius);

  p.longitude =
      home_geo_point.longitude +
      TransformUtils::ToDegrees(kBoundsRadiusInMeters /
                                EarthUtils::kEarthRadius) /
          std::cos(TransformUtils::ToRadians(home_geo_point.latitude));

  return p;
}

SCENE_END_NAMESPACE