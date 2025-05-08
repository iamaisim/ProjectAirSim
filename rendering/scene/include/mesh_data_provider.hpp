// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef RENDERING_SCENE_INCLUDE_MESH_DATA_PROVIDER_HPP_
#define RENDERING_SCENE_INCLUDE_MESH_DATA_PROVIDER_HPP_

#include "mesh.hpp"
#include "scene_global.hpp"
#include "tile_info.hpp"

SCENE_BEGIN_NAMESPACE

class MeshDataProvider {
 public:
  virtual ~MeshDataProvider(){};
  virtual MeshData GetTileMeshData(const TileKey& tile_key) const = 0;
};

SCENE_END_NAMESPACE

#endif  // RENDERING_SCENE_INCLUDE_MESH_DATA_PROVIDER_HPP_