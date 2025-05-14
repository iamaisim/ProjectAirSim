// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#ifndef MULTIWHEEL_API_INCLUDE_VTOLFW_API_BASE_HPP_
#define MULTIWHEEL_API_INCLUDE_VTOLFW_API_BASE_HPP_

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "iackermann_api.hpp"
#include "rover_api_base.hpp"

namespace microsoft {
namespace projectairsim {

class AckermannApiBase : public RoverApiBase, public AckermannApi {
 public:
  AckermannApiBase(const Robot& robot, TransformTree* ptransformtree);

  virtual ~AckermannApiBase(void) = default;


  //---------------------------------------------------------------------------
  // IAckermannApi Methods
  // (Redeclared here so we can attach service methods)

  virtual bool SetRoverControls(float engine, float steering_angle,
                                float brake) override = 0;

 protected:
  virtual void RegisterServiceMethods(void) override;

};  // class AckermannApiBase

}  // namespace projectairsim
}  // namespace microsoft

#endif