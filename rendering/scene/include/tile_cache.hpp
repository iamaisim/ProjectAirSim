// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef RENDERING_SCENE_INCLUDE_TILE_CACHE_HPP_
#define RENDERING_SCENE_INCLUDE_TILE_CACHE_HPP_

#include <list>
#include <unordered_map>

#include "mesh.hpp"
#include "scene_global.hpp"
#include "tile_info.hpp"

SCENE_BEGIN_NAMESPACE

class TileCache {
 public:
  TileCache(int max_num_of_tiles = 1000);

  void AddTile(const TileKey& tile_key, MeshData&& mesh_data);
  MeshData* GetMesh(const TileKey& tile_key);

  inline bool IsCached(const TileKey& tile_key) const {
    return tile_mesh_map_.find(tile_key) != tile_mesh_map_.end();
  }

 private:
  void BumpToFront(const TileKey& tileKey);

  // Holds all loaded tiles (whether or not they're rendered)
  std::unordered_map<TileKey, MeshData> tile_mesh_map_;

  // Manages LRU cache for tiles. All of these are already loaded in memory (in
  // tile_mesh_map_)
  std::list<TileKey>
      lru_tile_keys_;  // the front is the most recently queried tile
  std::unordered_map<TileKey, std::list<TileKey>::iterator>
      lru_index_map_;  // for efficient deletion

  int max_num_tiles_;
};

SCENE_END_NAMESPACE

#endif  // RENDERING_SCENE_INCLUDE_TILE_CACHE_HPP_