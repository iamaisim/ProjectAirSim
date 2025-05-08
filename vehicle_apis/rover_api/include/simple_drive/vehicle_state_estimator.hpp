// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef ROVER_API_SIMPLE_DRIVE_VEHICLE_ESTIMATOR_HPP_
#define ROVER_API_SIMPLE_DRIVE_VEHICLE_ESTIMATOR_HPP_

#include "common/IStateEstimator.hpp"
#include "common/common.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/physics_common_utils.hpp"
#include "core_sim/transforms/transform_utils.hpp"
#include "utils.hpp"

namespace microsoft {
namespace projectairsim {
namespace simple_drive {

class VehicleStateEstimator : public vehicle_apis::IStateEstimator {
 public:
  virtual ~VehicleStateEstimator() {}

  // for now we don't do any state estimation and use ground truth (i.e. assume
  // perfect sensors)
  void SetGroundTruthKinematics(const Kinematics* kinematics) {
    kinematics_ = kinematics;
  }

  vehicle_apis::Axis3r GetAngles() const override {
    vehicle_apis::Axis3r angles;
    Vector3 rpy = TransformUtils::ToRPY(kinematics_->pose.orientation);
    angles.Roll() = rpy[0];
    angles.Pitch() = rpy[1];
    angles.Yaw() = rpy[2];

    // Utils::log(Utils::stringf("Ang Est:\t(%f, %f, %f)", angles.pitch(),
    // angles.roll(), angles.yaw()));

    return angles;
  }

  vehicle_apis::Axis3r GetAngularVelocity() const override {
    const auto& angular = kinematics_->twist.angular;

    vehicle_apis::Axis3r conv;
    conv.X() = angular.x();
    conv.Y() = angular.y();
    conv.Z() = angular.z();

    return conv;
  }

  vehicle_apis::Axis3r GetPosition() const override {
    return Utils::ToAxis3r(kinematics_->pose.position);
  }

  vehicle_apis::Axis3r TransformVectorToBodyFrame(
      const vehicle_apis::Axis3r& world_frame_val) const override {
    const Vector3& vec = Utils::ToVector3(world_frame_val);
    const Vector3& trans = PhysicsUtils::TransformVectorToBodyFrame(
        vec, kinematics_->pose.orientation);
    return Utils::ToAxis3r(trans);
  }

  vehicle_apis::Axis3r TransformVectorToBodyFromHorizontalPlaneFrame(
      const vehicle_apis::Axis3r& horizontal_plane_frame_val) const override {
    const Vector3& vec =
        Utils::ToVector3(horizontal_plane_frame_val);
    auto orientation = kinematics_->pose.orientation;
    // quaternion to roll pitch yaw
    Vector3 rpy = TransformUtils::ToRPY(orientation);
    // set yaw to zero to correct for current roll and pitch only
    rpy[2] = 0;
    // convert back to quaternion
    orientation = TransformUtils::ToQuaternion(rpy);
    const Vector3& trans =
        PhysicsUtils::TransformVectorToBodyFrame(vec, orientation, false);
    return Utils::ToAxis3r(trans);
  }
  // The next one can be a more intuitive and less efficient solution for the
  // previous method: vehicle_apis::Axis3r
  // TransformVectorToBodyFromHorizontalPlaneFrame(
  //     const vehicle_apis::Axis3r& horizontal_plane_frame_val) const override
  //     {
  //   const Vector3& vec =
  //   Utils::ToVector3(horizontal_plane_frame_val);
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
  //   TransformVectorToBodyFrame(Utils::ToAxis3r(vec_adjusted));
  // }

  vehicle_apis::Axis3r GetLinearVelocity() const override {
    return Utils::ToAxis3r(kinematics_->twist.linear);
  }

  vehicle_apis::Axis4r GetOrientation() const override {
    return Utils::ToAxis4r(kinematics_->pose.orientation);
  }

  vehicle_apis::KinematicsState GetKinematicsEstimated() const override {
    vehicle_apis::KinematicsState state;
    state.position = GetPosition();
    state.orientation = GetOrientation();
    state.linear_velocity = GetLinearVelocity();
    state.angular_velocity = GetAngularVelocity();
    state.linear_acceleration =
        Utils::ToAxis3r(kinematics_->accels.linear);
    state.angular_acceleration =
        Utils::ToAxis3r(kinematics_->accels.angular);

    return state;
  }

 private:
  const Kinematics* kinematics_;
};  // class VehicleEstimator

}  // namespace simple_drive
}  // namespace projectairsim
}  // namespace microsoft

#endif  // ROVER_API_SIMPLE_DRIVE_VEHICLE_ESTIMATOR_HPP_
