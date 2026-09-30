// Copyright (C) 2026 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#pragma once

#include <memory>
#include <string>

#include "ASCDecl.h"
#include "Client.h"
#include "Status.h"
#include "Types.h"
#include "World.h"

namespace microsoft {
namespace projectairsim {
namespace client {

// Client for an Unreal AWheeledVehiclePawn robot with native control RPCs.
class WheeledVehicle {
 public:
  ASC_DECL WheeledVehicle(void) noexcept;
  ASC_DECL ~WheeledVehicle();

  ASC_DECL Status Initialize(std::shared_ptr<Client>& pclient,
                             std::shared_ptr<World>& pworld,
                             const std::string& vehicle_name);

  // The simulator rejects non-finite values and clamps finite inputs.
  ASC_DECL Status SetThrottle(float value, bool* pf_success_out);  // [-1, 1]
  ASC_DECL Status SetSteering(float value, bool* pf_success_out);  // [-1, 1]
  ASC_DECL Status SetBrakes(float value, bool* pf_success_out);    // [0, 1]
  ASC_DECL Status GetKinematics(json* pjson_out) const;
  ASC_DECL Status GetGroundTruthKinematics(json* pjson_out) const;

 protected:
  class Impl;
  std::unique_ptr<Impl> pimpl_;
};  // class WheeledVehicle

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
