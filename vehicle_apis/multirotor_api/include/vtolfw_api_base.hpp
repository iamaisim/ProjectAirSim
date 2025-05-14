// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_VTOLFW_API_BASE_HPP_
#define MULTIROTOR_API_INCLUDE_VTOLFW_API_BASE_HPP_

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "ivtolfw_api.hpp"
#include "multirotor_api_base.hpp"

namespace microsoft {
namespace projectairsim {

class VTOLFWApiBase : public MultirotorApiBase, public IVTOLFWApi {
 public:
  VTOLFWApiBase() {}
  VTOLFWApiBase(const Robot& robot, TransformTree* ptransformtree);

  virtual ~VTOLFWApiBase() = default;

  //---------------------------------------------------------------------------
  // IVTOLFWApi

  bool MoveByHeading(float heading, float speed, float vz, float duration,
                     float heading_margin, float yaw_rate, float timeout_sec,
                     int64_t command_start_time_nanos) override;

  bool SetVTOLMode(VTOLMode vtolmode) override;

 protected:  // Must be implemented by subclass
  virtual void CommandHeading(float heading, float speed, float vz) = 0;

 protected:
  void RegisterServiceMethods(void) override;

  //---------------------------------------------------------------------------
  // Service Method Wrappers

  bool MoveByHeadingServiceMethod(float heading, float speed, float vz,
                                  float duration, float heading_margin,
                                  float yaw_rate, float timeout_sec,
                                  TimeNano _service_method_start_time);
};  // class VTOLFWApiBase

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_VTOLFW_API_BASE_HPP_
