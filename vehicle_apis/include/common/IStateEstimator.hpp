// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef VEHICLEAPIS_COMMON_ISTATEESTIMATOR_HPP_
#define VEHICLEAPIS_COMMON_ISTATEESTIMATOR_HPP_

#include "common.hpp"

namespace microsoft {
namespace projectairsim {
namespace vehicle_apis {

class IStateEstimator {
 public:
  virtual Axis3r GetAngles() const = 0;
  virtual Axis3r GetAngularVelocity() const = 0;
  virtual Axis3r GetPosition() const = 0;
  virtual Axis3r GetLinearVelocity() const = 0;
  virtual Axis4r GetOrientation() const = 0;
  virtual KinematicsState GetKinematicsEstimated() const = 0;
  virtual Axis3r TransformVectorToBodyFrame(
      const Axis3r& world_frame_val) const = 0;
  virtual Axis3r TransformVectorToBodyFromHorizontalPlaneFrame(
      const Axis3r& body_frame_val) const = 0;

  virtual ~IStateEstimator() = default;
};

}  // namespace vehicle_apis
}  // namespace projectairsim
}  // namespace microsoft

#endif  // VEHICLEAPIS_COMMON_ISTATEESTIMATOR_HPP_
