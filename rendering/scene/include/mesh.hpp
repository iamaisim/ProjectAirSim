// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef RENDERING_SCENE_INCLUDE_MESH_HPP_
#define RENDERING_SCENE_INCLUDE_MESH_HPP_

#include <vector>

#include "core_sim/math_utils.hpp"
#include "scene_global.hpp"

SCENE_BEGIN_NAMESPACE

using Vertex = microsoft::projectairsim::Vector3f;
using UV = microsoft::projectairsim::Vector2f;

struct TexutureData {
  unsigned int width;
  unsigned int height;
  std::vector<unsigned char> buffer;
  // may need texture encodings? (such as DXT1 - may be)
};

struct MeshData {
  std::vector<Vertex> vertices;
  std::vector<UV> uvs;
  std::vector<unsigned int> triangle_indices;

  TexutureData texture;

  // Todo - Add normal buffer, face buffer, tangent buffer as necessary
};

SCENE_END_NAMESPACE

#endif  // RENDERING_SCENE_INCLUDE_MESH_HPP_