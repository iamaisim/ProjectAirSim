// Copyright (C) Microsoft Corporation. All rights reserved.

#include "tile_divider.hpp"

#include "bing_maps_utils.hpp"
#include "core_sim/geodetic_converter.hpp"

SCENE_BEGIN_NAMESPACE

TileDivider::TileDivider(int min_tile_lod, int max_tile_lod, Quality quality,
                         GeoLocation southwest_bound,
                         GeoLocation northeast_bound)
    : min_tile_lod_(min_tile_lod),
      max_tile_lod_(max_tile_lod),
      quality_(quality) {
  world_bounds_ =
      Box2d(Vector2d(southwest_bound.longitude, southwest_bound.latitude),
            Vector2d(northeast_bound.longitude, northeast_bound.latitude));
}

TileDivider::TileAvailable TileDivider::IsTileSufficient(
    const GeoLocation& observer_geo_loc, const TileKey& key) const {
  // Special case to stop looking for higher LOD tiles for ocean near Seattle
  // since higher LODs for them are not available.
  // TODO Delete this after the July 2022 launch demo.
  if ((key.lod == 15) &&
      (key.x >= 5245 && key.x <= 5247 && key.y >= 11443 &&
       key.y <= 11445) /* 9 tiles centered around x=5246 y=11444 */
      && !(key.x == 5247 && key.y == 11443) /* except x=5247 y=11443 */) {
    return TileAvailable::SUFFICIENT;
  }


  // TODO - assert that longitude is between -180 to 180 & latitude is between
  // -90 to 90

  double min_lat, min_long, max_lat, max_long;

  // TODO - these two functions should be used through interface or template
  // polymorphism basically have ability to pass any tile to lat/long converter
  // & viceversa
  BingMapsUtils::TileXYToLatLong(key.x, key.y, key.lod, min_lat, min_long);
  BingMapsUtils::TileXYToLatLong(key.x + 1, key.y + 1, key.lod, max_lat,
                                 max_long);

  Box2d bounds(
      Vector2d(std::min(min_long, max_long), std::min(min_lat, max_lat)),
      Vector2d(std::max(min_long, max_long), std::max(min_lat, max_lat)));

  if (!world_bounds_.intersects(bounds)) {
    return TileAvailable::OUTOFBOUNDS;
  }

  // x is long & y is lat for bounds

  // TODO - add a function which checks if lat/lon and x amount of radius (let's
  // say 2km) interests with bounds of tile, if not, then simply discard that
  // tile. Easier way, first check if lat/lon is inside bounds, if it is then no
  // need to check for radius else create another box with this lat/lon as
  // center & x radius around it, find intersection of boxes
  // TODO - do we need to take elevation into account? yes, how?
  // This also needs FOV

  double nearest_y =
      std::clamp(observer_geo_loc.latitude, bounds.min().y(), bounds.max().y());

  double nearest_x = std::clamp(observer_geo_loc.longitude, bounds.min().x(),
                                bounds.max().x());

  Vector2d nearest_point(nearest_x, nearest_y);

  microsoft::projectairsim::GeodeticConverter geo_conv;

  double geo_x, geo_y, geo_z;

  // for tiles altitude is 0? right?
  geo_conv.geodetic2Ecef(nearest_point.y(), nearest_point.x(), 0.0, &geo_x,
                         &geo_y, &geo_z);

  double observer_geo_x, observer_geo_y, observer_geo_z;
  geo_conv.geodetic2Ecef(observer_geo_loc.latitude, observer_geo_loc.longitude,
                         observer_geo_loc.altitude, &observer_geo_x,
                         &observer_geo_y, &observer_geo_z);

  Vector3d tile_geo_pos(geo_x, geo_y, geo_z);
  Vector3d observer_geo_pos(observer_geo_x, observer_geo_y, observer_geo_z);

  Vector3d dir = observer_geo_pos - tile_geo_pos;
  double dist = dir.norm();

  dir.normalize();
  tile_geo_pos.normalize();

  double angle = dir.dot(tile_geo_pos);

  // if we are at max lod, then this tile is maximum/best tile we can give
  // no point in figuring out subdividing
  if (key.lod == max_tile_lod_) {
    return TileAvailable::SUFFICIENT;
  }

  auto tile_size_occupied =
      (cos(nearest_y * M_PI / 180) * 2 * M_PI * EarthUtils::kEarthRadius) /
      std::pow(2, key.lod);

  // the further you are from the tile, the smaller tile_projected_size would
  // be (whether your flying high or because you are away from it on x/y)
  // plane
  // we use this to decide that this tile & lod is sufficient
  double tile_projected_size = tile_size_occupied / dist;

  double terminal_value = 0.0;

  if (quality_ == Quality::HIGH) {
    terminal_value = .5 + (-.4) * angle;
  } else if (quality_ == Quality::MEDIUM) {
    terminal_value = 0.3 / angle;
  } else if (quality_ == Quality::LOW) {
    terminal_value = 0.3 / (angle * angle);
  } else if (quality_ == Quality::SUPERLOW) {
    terminal_value = 0.3 / (angle * angle * angle);
  }

  if (tile_projected_size < terminal_value && key.lod >= min_tile_lod_) {
    return TileAvailable::SUFFICIENT;
  } else {
    return TileAvailable::NOTSUFFICIENT;
  }
}

// Recursive method to decide if a tile needs to be divided based on an array of
// observer locations.
void TileDivider::GetKeysImpl(
    const std::vector<GeoLocation>& observer_locations, const TileKey& cur_key,
    std::vector<std::TileKey>& out_keys) const {
  bool is_tile_sufficient_for_any_loc = false;

  for (const auto& loc : observer_locations) {
    auto tile_status = IsTileSufficient(loc, cur_key);

    if (tile_status == TileAvailable::NOTSUFFICIENT) {
      // ascii diagram to demonstrate how splitting is happening

      /*
      Parent tile: (x), (y), (lod)
      ─────────────────────────────────────────────────────────────
      |                           N(90)                           |
      |                             │                             |
      |                             │                             |
      |    (x*2), (y*2), (lod+1)    │   (x*2)+1, (y*2), (lod+1)   |
      |                             │                             |
      |                             │                             |
      |                             │                             |
      | (-180)W─────────────────────┼───────────────────── E(180) |
      |                             │                             |
      |                             │                             |
      |                             │                             |
      |   (x*2), (y*2)+1, (lod+1)   │  (x*2)+1, (y*2)+1, (lod+1)  |
      |                             │                             |
      |                             │                             |
      |                           S(-90)                          |
      ─────────────────────────────────────────────────────────────
      */
      TileKey childKeys[4];
      cur_key.GetChildTiles(&childKeys[0]);

      GetKeysImpl(observer_locations, childKeys[0], out_keys);
      GetKeysImpl(observer_locations, childKeys[1], out_keys);
      GetKeysImpl(observer_locations, childKeys[2], out_keys);
      GetKeysImpl(observer_locations, childKeys[3], out_keys);

      // This tile is getting divided due to this location, so stop checking
      // other locations and just return after the first set of recursion calls.
      // Do not add this insufficient parent tile to out_keys.
      return;
    } else if (tile_status == TileAvailable::SUFFICIENT) {
      is_tile_sufficient_for_any_loc = true;
      // This tile is sufficient for this location, but keep checking if it
      // needs to be divided for any of the other locations.
    } else {  // tile_status == TileAvailable::OUTOFBOUNDS
      // This tile is out of bounds for this location, so just keep
      // checking the other locations.
    }
  }

  if (is_tile_sufficient_for_any_loc) {
    // This tile did not need to be divided and is sufficient for at least one
    // of the locations, so add it to out_keys.
    out_keys.push_back(cur_key);
  } else {
    // This tile did not need to be divided and is out of bounds for all
    // locations, so just drop it.
  }
}

std::vector<std::TileKey> TileDivider::GetKeys(
    const std::vector<GeoLocation>& observer_locations) const {
  std::vector<std::TileKey> keys_to_render;
  TileKey root_tile(0, 0, 0);  // root lod 0 to be 256 x 256 single tile
  GetKeysImpl(observer_locations, root_tile, keys_to_render);
  return keys_to_render;
}

SCENE_END_NAMESPACE
