// Copyright (C) Microsoft Corporation.  All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_LANDING_GEAR_API_BASE_HPP_
#define MULTIROTOR_API_INCLUDE_LANDING_GEAR_API_BASE_HPP_

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "ilanding_gear_api.hpp"
#include "vtolfw_api_base.hpp"

namespace microsoft {
namespace projectairsim {

class LandingGearApiBase : public VTOLFWApiBase, public ILandingGearApi {
 public:
    LandingGearApiBase() {}
    LandingGearApiBase(const Robot& robot, TransformTree* ptransformtree);

    virtual ~LandingGearApiBase() = default;

 protected:
  void RegisterServiceMethods(void) override;

  //---------------------------------------------------------------------------
  // Service Method Wrappers

    bool SetBrakesServiceMethod(float value);
};  // class LandingGearApiBase

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_LANDING_GEAR_API_BASE_HPP_