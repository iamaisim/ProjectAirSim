// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTESTIMATORFW_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTESTIMATORFW_HPP_

#include "AirSimSimpleFlightCommon.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/physics_common_utils.hpp"
#include "firmware/interfaces/CommonStructs.hpp"
#include "firmware/interfaces/IStateEstimator.hpp"

namespace microsoft {
namespace projectairsim {

// Wrapper around standard state estimator except angles are appropriate for
// fixed-wing flight where 0 pitch is horizontal and roll, pitch and yaw angles
// are not Euler angles but ground-relative attitude angles.
class AirSimSimpleFlightEstimatorFW : public simple_flight::IStateEstimator {
 public:
  AirSimSimpleFlightEstimatorFW(void)
      : angles_last_(),
        micros_last_(0),
        pistateestimator_(nullptr),
        velocity_angular_() {}
  virtual ~AirSimSimpleFlightEstimatorFW(void) {}

  void Initialize(const simple_flight::IStateEstimator* pistateestimator) {
    pistateestimator_ = pistateestimator;
  }

  simple_flight::Axis3r GetAngles() const override {
    // Instead of returning the normal Euler angles (roll, pitch, and yaw) from
    // the orientation quaternion, return the ground-relative vehicle attitude
    // RPY angles.
    static const float c_dxyzMin =
        1.0f - 0.000001f;  // Threshold for to detect vehicle pointing straight
                           // up or down

    auto matRotation = AirSimSimpleFlightCommon::ToQuaternion(
                           pistateestimator_->GetOrientation())
                           .toRotationMatrix();
    microsoft::projectairsim::Vector3f vec3X =
        matRotation * microsoft::projectairsim::Vector3(1.0f, 0.0f, 0.0f);
    microsoft::projectairsim::Vector3f vec3Y =
        matRotation * microsoft::projectairsim::Vector3(0.0f, 1.0f, 0.0f);
    microsoft::projectairsim::Vector3f vec3Z =
        matRotation * microsoft::projectairsim::Vector3(0.0f, 0.0f, 1.0f);

    auto rollAttitude = 0.0f;
    auto pitchAttitude = 0.0f;
    auto yawAttitude = 0.0f;

    if (vec3Z.z() > c_dxyzMin) {
      // Pointing straight up
      pitchAttitude = simple_flight::kPI_2;
      yawAttitude = 0.0f;
      rollAttitude = atan2(
          vec3Y.x(), vec3Y.y());  // Arbitrarily pick Y-Z plane as zero roll
    } else if (vec3Z.z() < -c_dxyzMin) {
      // Pointing straight down
      pitchAttitude = -simple_flight::kPI_2;
      yawAttitude = 0.0f;
      rollAttitude = -atan2(
          vec3Y.x(), vec3Y.y());  // Arbitrarily pick Y-Z plane as zero roll
    } else {
      microsoft::projectairsim::AngleAxis matCounterRot;

      // Calculate yaw from Y or Z, depending on who's closer to horizontal
      if (fabs(vec3Z.z()) < fabs(vec3Y.z()))
        yawAttitude = atan2(-vec3Z.y(), -vec3Z.x());
      else
        yawAttitude = atan2(-vec3Y.x(), vec3Y.y());

      // Align vehicle nose to positive X-axis in X-Z plane and get pitch
      matCounterRot = microsoft::projectairsim::AngleAxis(
          -yawAttitude, microsoft::projectairsim::Vector3f::UnitZ());
      vec3X = matCounterRot * vec3X;
      vec3Y = matCounterRot * vec3Y;
      vec3Z = matCounterRot * vec3Z;

      // Calculate pitch from X or Z, depending on who's closer to X-Z plane
      if (fabs(vec3Z.y()) < fabs(vec3X.y()))
        pitchAttitude = atan2(vec3Z.z(), -vec3Z.x());
      else
        pitchAttitude = atan2(vec3X.x(), vec3X.z());
      if (vec3X.z() < 0.0f) {
        // When the vehicle is inverted, undoing the yaw causes the calculated
        // pitch to be reflected from the expected value
        pitchAttitude = ((pitchAttitude < 0.0f) ? -simple_flight::kPI
                                                : simple_flight::kPI) -
                        pitchAttitude;
      }

      // Align vehicle nose to X-axis and get roll
      matCounterRot = microsoft::projectairsim::AngleAxis(
          -pitchAttitude, microsoft::projectairsim::Vector3f::UnitY());
      vec3Y = matCounterRot * vec3Y;

      rollAttitude = atan2(vec3Y.z(), vec3Y.y());
      pitchAttitude -=
          simple_flight::kPI_2;  // We calculate with pitch 0 = horizontal, but
                                 // normal estimator pitch 0 = vertical
    }

    return simple_flight::Axis3r(rollAttitude, pitchAttitude, yawAttitude);
  }

  simple_flight::Axis3r GetAngularVelocity() const override {
    return (velocity_angular_);
  }

  simple_flight::Axis3r GetPosition() const override {
    return (pistateestimator_->GetPosition());
  }

  simple_flight::Axis3r TransformVectorToBodyFrame(
      const simple_flight::Axis3r& world_frame_val) const override {
    return (pistateestimator_->TransformVectorToBodyFrame(world_frame_val));
  }

  simple_flight::Axis3r TransformVectorToBodyFromHorizontalPlaneFrame(
      const simple_flight::Axis3r& body_frame_val) const override {
    return (pistateestimator_->TransformVectorToBodyFromHorizontalPlaneFrame(
        body_frame_val));
  }

  simple_flight::Axis3r GetLinearVelocity() const override {
    return (pistateestimator_->GetLinearVelocity());
  }

  simple_flight::Axis4r GetOrientation() const override {
    simple_flight::Axis4r axis4r = pistateestimator_->GetOrientation();

    //
    return (axis4r);
  }

  simple_flight::KinematicsState GetKinematicsEstimated() const override {
    return (pistateestimator_->GetKinematicsEstimated());
  }

  void Update(int64_t micros) {
    uint64_t micros_cur = *(uint64_t*)&micros;
    float dsec = (micros_cur - micros_last_) / 1.0e6;
    auto angles_cur = GetAngles();

    for (int i = 0, c = simple_flight::Axis3r::AxisCount(); i < c; ++i) {
      auto radian_cur = angles_cur[i];
      auto radian_last = angles_last_[i];
      auto dradian = radian_cur - radian_last;

      if (dradian < -simple_flight::kPI_2)
        dradian += simple_flight::k2PI;
      else if (dradian > simple_flight::kPI_2)
        dradian -= simple_flight::k2PI;

      velocity_angular_[i] = dradian / dsec;
    }

    micros_last_ = micros_cur;
    angles_last_ = angles_cur;
  }

 private:
  simple_flight::Axis3r angles_last_;  // Angles from last update
  uint64_t micros_last_;  // Timestamp of last update (microseconds)
  const simple_flight::IStateEstimator*
      pistateestimator_;  // The state estimator we're wrapping
  simple_flight::Axis3r velocity_angular_;  // Calculated angular velocities
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTESTIMATORFW_HPP_
