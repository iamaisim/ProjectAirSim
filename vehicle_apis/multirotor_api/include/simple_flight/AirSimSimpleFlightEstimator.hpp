// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTESTIMATOR_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTESTIMATOR_HPP_

#include "AirSimSimpleFlightCommon.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/physics_common_utils.hpp"
#include "core_sim/transforms/transform_utils.hpp"
#include "firmware/interfaces/CommonStructs.hpp"
#include "firmware/interfaces/IStateEstimator.hpp"

namespace microsoft {
namespace projectairsim {

class AirSimSimpleFlightEstimator : public simple_flight::IStateEstimator {
 public:
  virtual ~AirSimSimpleFlightEstimator() {}

  // for now we don't do any state estimation and use ground truth (i.e. assume
  // perfect sensors)
  void SetGroundTruthKinematics(const Kinematics* kinematics) {
    kinematics_ = kinematics;
  }

  simple_flight::Axis3r GetAngles() const override {
    simple_flight::Axis3r angles;
    Vector3 rpy = TransformUtils::ToRPY(kinematics_->pose.orientation);
    angles.Roll() = rpy[0];
    angles.Pitch() = rpy[1];
    angles.Yaw() = rpy[2];

    // Utils::log(Utils::stringf("Ang Est:\t(%f, %f, %f)", angles.pitch(),
    // angles.roll(), angles.yaw()));

    return angles;
  }

  simple_flight::Axis3r GetAngularVelocity() const override {
    const auto& angular = kinematics_->twist.angular;

    simple_flight::Axis3r conv;
    conv.X() = angular.x();
    conv.Y() = angular.y();
    conv.Z() = angular.z();

    return conv;
  }

  simple_flight::Axis3r GetPosition() const override {
    return AirSimSimpleFlightCommon::ToAxis3r(kinematics_->pose.position);
  }

  simple_flight::Axis3r TransformVectorToBodyFrame(
      const simple_flight::Axis3r& world_frame_val) const override {
    const Vector3& vec = AirSimSimpleFlightCommon::ToVector3(world_frame_val);
    const Vector3& trans = PhysicsUtils::TransformVectorToBodyFrame(
        vec, kinematics_->pose.orientation);
    return AirSimSimpleFlightCommon::ToAxis3r(trans);
  }

  simple_flight::Axis3r TransformVectorToBodyFromHorizontalPlaneFrame(
      const simple_flight::Axis3r& horizontal_plane_frame_val) const override {
    const Vector3& vec =
        AirSimSimpleFlightCommon::ToVector3(horizontal_plane_frame_val);
    auto orientation = kinematics_->pose.orientation;
    // quaternion to roll pitch yaw
    Vector3 rpy = TransformUtils::ToRPY(orientation);
    // set yaw to zero to correct for current roll and pitch only
    rpy[2] = 0;
    // convert back to quaternion
    orientation = TransformUtils::ToQuaternion(rpy);
    const Vector3& trans =
        PhysicsUtils::TransformVectorToBodyFrame(vec, orientation, false);
    return AirSimSimpleFlightCommon::ToAxis3r(trans);
  }
  // The next one can be a more intuitive and less efficient solution for the
  // previous method: simple_flight::Axis3r
  // TransformVectorToBodyFromHorizontalPlaneFrame(
  //     const simple_flight::Axis3r& horizontal_plane_frame_val) const override
  //     {
  //   const Vector3& vec =
  //   AirSimSimpleFlightCommon::ToVector3(horizontal_plane_frame_val);
  //   //transform to worl frame
  //   auto orientation = kinematics_->pose.orientation;
  //   //quaternion to roll pitch yaw
  //   Vector3 rpy = TransformUtils::ToRPY(orientation);
  //   //get yaw
  //   float yaw = rpy[2];
  //   //rotate vec by yaw
  //   auto vx = vec.x();
  //   auto vy = vec.y();
  //   float vx_adjusted = (vx * std::cosf(yaw)) - (vy * std::sinf(yaw));
  //   float vy_adjusted = (vx * std::sinf(yaw)) + (vy * std::cosf(yaw));
  //   Vector3 vec_adjusted(vx_adjusted, vy_adjusted, vec.z());
  //   return
  //   TransformVectorToBodyFrame(AirSimSimpleFlightCommon::ToAxis3r(vec_adjusted));
  // }

  simple_flight::Axis3r GetLinearVelocity() const override {
    return AirSimSimpleFlightCommon::ToAxis3r(kinematics_->twist.linear);
  }

  simple_flight::Axis4r GetOrientation() const override {
    return AirSimSimpleFlightCommon::ToAxis4r(kinematics_->pose.orientation);
  }

  simple_flight::KinematicsState GetKinematicsEstimated() const override {
    simple_flight::KinematicsState state;
    state.position = GetPosition();
    state.orientation = GetOrientation();
    state.linear_velocity = GetLinearVelocity();
    state.angular_velocity = GetAngularVelocity();
    state.linear_acceleration =
        AirSimSimpleFlightCommon::ToAxis3r(kinematics_->accels.linear);
    state.angular_acceleration =
        AirSimSimpleFlightCommon::ToAxis3r(kinematics_->accels.angular);

    return state;
  }

 private:
  const Kinematics* kinematics_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_AIRSIMSIMPLEFLIGHTESTIMATOR_HPP_
