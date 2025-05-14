// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "gltf_data_provider.hpp"

#include "bing_maps_utils.hpp"

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_STATIC
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
// Force little endian for Windows
#ifndef __ORDER_LITTLE_ENDIAN__
#define __ORDER_LITTLE_ENDIAN__ 1
#endif
#ifndef __BYTE_ORDER__
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#endif
#include "tiny_gltf.h"

SCENE_BEGIN_NAMESPACE

DefaultFileReader::DefaultFileReader(const std::string& gltf_dir)
    : gltf_dir_(gltf_dir) {}

std::string DefaultFileReader::ReadFile(const std::string& file_name) const {
  std::string gltf_file_local = gltf_dir_ + file_name;
  std::ifstream file(gltf_file_local, std::ifstream::binary);
  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

// Note: in an Azure instance, gltf_dir needs to be mounted.
GLTFDataProvider::GLTFDataProvider(
    std::unique_ptr<FileReaderInterface> file_reader)
    : file_reader_(std::move(file_reader)) {}

void FillVertexData(tinygltf::Model& model, std::vector<Vertex>& outRef) {
  tinygltf::Mesh& gltfmesh = model.meshes[0];
  tinygltf::Primitive& prim = gltfmesh.primitives[0];

  tinygltf::Accessor& pos_accessor =
      model.accessors[prim.attributes["POSITION"]];
  tinygltf::BufferView& pos_bview = model.bufferViews[pos_accessor.bufferView];
  tinygltf::Buffer& pos_bfer = model.buffers[pos_bview.buffer];

  // update vertex data
  outRef.resize(pos_accessor.count);

  auto ptr = reinterpret_cast<float*>(
      &(pos_bfer.data[pos_bview.byteOffset + pos_accessor.byteOffset]));

  for (size_t i = 0; i < pos_accessor.count; ++i) {
    int start_index = i * 3;
    outRef[i] = Vertex(ptr[start_index + 0], ptr[start_index + 1],
                       ptr[start_index + 2]);
  }
}

void FillUVData(tinygltf::Model& model, std::vector<UV>& outRef) {
  tinygltf::Mesh& gltfmesh = model.meshes[0];
  tinygltf::Primitive& prim = gltfmesh.primitives[0];

  tinygltf::Accessor& uv0_accessor =
      model.accessors[prim.attributes["TEXCOORD_0"]];
  tinygltf::BufferView& uv0_bview = model.bufferViews[uv0_accessor.bufferView];
  tinygltf::Buffer& uv0_bfer = model.buffers[uv0_bview.buffer];

  // update uv coord
  outRef.resize(uv0_accessor.count);

  auto ptr = reinterpret_cast<float*>(
      &(uv0_bfer.data[uv0_bview.byteOffset + uv0_accessor.byteOffset]));

  for (size_t i = 0; i < uv0_accessor.count; ++i) {
    int start_index = i * 2;
    outRef[i] = UV(ptr[start_index + 0], ptr[start_index + 1]);
  }
}

template <typename T>
void FillIndicesForType(tinygltf::Model& model,
                        std::vector<unsigned int>& outRef) {
  tinygltf::Mesh& gltfmesh = model.meshes[0];
  tinygltf::Primitive& prim = gltfmesh.primitives[0];

  int indices_mode = prim.mode;
  tinygltf::Accessor& indices_accessor = model.accessors[prim.indices];
  tinygltf::BufferView& indices_bview =
      model.bufferViews[indices_accessor.bufferView];
  tinygltf::Buffer& indices_bfer = model.buffers[indices_bview.buffer];

  size_t n_indices = indices_accessor.count;

  size_t loopstartIndex = 0;

  if (indices_mode == TINYGLTF_MODE_TRIANGLES) {
    outRef.resize(n_indices);
  } else if (indices_mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
    loopstartIndex = 2;
    outRef.resize((n_indices - 2) * 3);
  } else {
    throw std::runtime_error("unsupported indices type.");
  }

  int triangleStripIndex = 0;
  auto ptr = reinterpret_cast<T*>(
      &(indices_bfer
            .data[indices_bview.byteOffset + indices_accessor.byteOffset]));

  for (; loopstartIndex < n_indices; ++loopstartIndex) {
    if (indices_mode == TINYGLTF_MODE_TRIANGLES) {
      outRef[loopstartIndex] = ptr[loopstartIndex];
    } else if (indices_mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
      // Triangle Strip (with alternating winding order)
      if (loopstartIndex % 2) {
        outRef[triangleStripIndex++] = ptr[loopstartIndex];
        outRef[triangleStripIndex++] = ptr[loopstartIndex - 1];
        outRef[triangleStripIndex++] = ptr[loopstartIndex - 2];
      } else {
        outRef[triangleStripIndex++] = ptr[loopstartIndex - 2];
        outRef[triangleStripIndex++] = ptr[loopstartIndex - 1];
        outRef[triangleStripIndex++] = ptr[loopstartIndex];
      }
    } else {
      throw std::runtime_error("unsupported indices mode.");
    }
  }
}

void FillIndices(tinygltf::Model& model, std::vector<unsigned int>& outRef) {
  tinygltf::Mesh& gltfmesh = model.meshes[0];
  tinygltf::Primitive& prim = gltfmesh.primitives[0];
  tinygltf::Accessor& indices_accessor = model.accessors[prim.indices];

  switch (indices_accessor.componentType) {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
      FillIndicesForType<uint8_t>(model, outRef);
      break;
    case TINYGLTF_COMPONENT_TYPE_BYTE:
      FillIndicesForType<int8_t>(model, outRef);
      break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
      FillIndicesForType<uint16_t>(model, outRef);
      break;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
      FillIndicesForType<int16_t>(model, outRef);
      break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
      FillIndicesForType<uint32_t>(model, outRef);
      break;
    case TINYGLTF_COMPONENT_TYPE_INT:
      FillIndicesForType<int32_t>(model, outRef);
      break;
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
      FillIndicesForType<float>(model, outRef);
      break;
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
      FillIndicesForType<double>(model, outRef);
      break;
    default:
      throw std::runtime_error("Unsupported index component type.");
      break;
  }
}

void FillTextureData(tinygltf::Model& model, TexutureData& outRef) {
  if (model.textures.size() > 0) {
    tinygltf::Texture& tex = model.textures[0];
    if (tex.source > -1) {
      tinygltf::Image& image = model.images[tex.source];
      outRef.width = image.width;
      outRef.height = image.height;
      outRef.buffer = std::move(image.image);
    }
  }
}

MeshData GLTFDataProvider::GetTileMeshData(const TileKey& tile_key) const {
  auto quad_key = TileKeyToQuadkey(tile_key);
  std::string gltf_file_local = quad_key + ".glb";

  auto file_contents = file_reader_->ReadFile(gltf_file_local);

  MeshData outMesh;

  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  bool ret = loader.LoadBinaryFromMemory(
      &model, &err, &warn,
      reinterpret_cast<const unsigned char*>(file_contents.c_str()),
      file_contents.size());

  if (ret) {
    FillVertexData(model, outMesh.vertices);
    FillUVData(model, outMesh.uvs);
    FillIndices(model, outMesh.triangle_indices);
    FillTextureData(model, outMesh.texture);
  }

  return outMesh;
}

std::string GLTFDataProvider::TileKeyToQuadkey(const TileKey& tile_key) const {
  return BingMapsUtils::TileXYToQuadkey(tile_key.x, tile_key.y, tile_key.lod);
}

SCENE_END_NAMESPACE