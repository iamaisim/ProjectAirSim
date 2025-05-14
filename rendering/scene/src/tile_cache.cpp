// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "tile_cache.hpp"

#include <cassert>

SCENE_BEGIN_NAMESPACE

TileCache::TileCache(int max_num_of_tiles)
    : tile_mesh_map_(),
      lru_tile_keys_(),
      lru_index_map_(),
      max_num_tiles_(max_num_of_tiles) {}

void TileCache::AddTile(const TileKey& tile_key, MeshData&& mesh_data) {
  // If not in cache already, add it to the front
  if (tile_mesh_map_.find(tile_key) == tile_mesh_map_.end()) {
    assert(lru_index_map_.find(tile_key) == lru_index_map_.end());
    tile_mesh_map_.emplace(tile_key, mesh_data);
    lru_tile_keys_.push_front(tile_key);
    lru_index_map_[tile_key] = lru_tile_keys_.begin();
  } else {
    BumpToFront(tile_key);
  }

  if (lru_tile_keys_.size() > max_num_tiles_) {
    auto oldest_key = lru_tile_keys_.back();
    lru_tile_keys_.pop_back();
    lru_index_map_.erase(oldest_key);
    tile_mesh_map_.erase(oldest_key);
  }

  assert(tile_mesh_map_.size() <= max_num_tiles_);
  assert(lru_index_map_.size() <= max_num_tiles_);
}

MeshData* TileCache::GetMesh(const TileKey& tile_key) {
  BumpToFront(tile_key);
  return &tile_mesh_map_[tile_key];
}

void TileCache::BumpToFront(const TileKey& tile_key) {
  if (tile_mesh_map_.find(tile_key) != tile_mesh_map_.end()) {
    assert(lru_index_map_.find(tile_key) != lru_index_map_.end());
    lru_tile_keys_.erase(lru_index_map_[tile_key]);
    lru_tile_keys_.push_front(tile_key);
    lru_index_map_[tile_key] = lru_tile_keys_.begin();
    return;
  }
}

SCENE_END_NAMESPACE