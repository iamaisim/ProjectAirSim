// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef RENDERING_SCENE_INCLUDE_GLTF_DATA_PROVIDER_HPP_
#define RENDERING_SCENE_INCLUDE_GLTF_DATA_PROVIDER_HPP_

#include <string>

#include "mesh_data_provider.hpp"
#include "scene_global.hpp"

SCENE_BEGIN_NAMESPACE

class FileReaderInterface {
 public:
  virtual ~FileReaderInterface() {}
  virtual std::string ReadFile(const std::string& file_name) const = 0;
};

class DefaultFileReader : public FileReaderInterface {
 public:
  DefaultFileReader(const std::string& gltf_dir);
  std::string ReadFile(const std::string& file_name) const override;

 private:
  std::string gltf_dir_;
};

class GLTFDataProvider : public MeshDataProvider {
 public:
  GLTFDataProvider(std::unique_ptr<FileReaderInterface> file_reader);

  MeshData GetTileMeshData(const TileKey& tile_key) const override;

 private:
  std::string TileKeyToQuadkey(const TileKey& tile_key) const;

  std::unique_ptr<FileReaderInterface> file_reader_;
};

SCENE_END_NAMESPACE

#endif  // RENDERING_SCENE_INCLUDE_GLTF_DATA_PROVIDER_HPP_
