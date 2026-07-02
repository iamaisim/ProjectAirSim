// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#pragma once
#include <memory>
#include <string>
#include <vector>

#include "ASCDecl.h"
#include "Client.h"
#include "Status.h"
#include "Types.h"
#include "World.h"

namespace microsoft {
namespace projectairsim {
namespace client {

// A static (non-moving, non-controlled) sensor platform.
// Supports sensor data queries but no movement or control methods.
class StaticSensorActor {
 public:
  ASC_DECL StaticSensorActor(void) noexcept;
  ASC_DECL ~StaticSensorActor();

  // Initialize the static sensor actor object.
  //
  // Arguments:
  //   pclient     Pointer to client object
  //   pworld      Pointer to world object
  //   actor_name  Name of the static sensor actor in the scene
  //
  // Returns:
  //   (Return)    Initialization status
  ASC_DECL Status Initialize(std::shared_ptr<Client>& pclient,
                             std::shared_ptr<World>& pworld,
                             const std::string& actor_name);

  // Retrieve images from a camera sensor.
  //
  // Arguments:
  //   str_camera_id      Camera sensor ID
  //   veci_image_type_id List of image type IDs to request
  //   pjson_out          Output JSON with image data
  ASC_DECL Status GetImages(const std::string& str_camera_id,
                            const std::vector<int>& veci_image_type_id,
                            json* pjson_out) const;

 protected:
  class Impl;

 protected:
  std::unique_ptr<Impl> pimpl_;
};  // class StaticSensorActor

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
