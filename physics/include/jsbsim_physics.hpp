// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef PHYSICS_INCLUDE_JSBSIM_PHYSICS_HPP_
#define PHYSICS_INCLUDE_JSBSIM_PHYSICS_HPP_

#include <vector>

#include "base_physics.hpp"
#include "core_sim/actor/robot.hpp"
#include "core_sim/environment.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/physics_common_types.hpp"



namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class JSBSimPhysicsBody

class JSBSimPhysicsBody : public BasePhysicsBody {
 public:
  JSBSimPhysicsBody() {}
  explicit JSBSimPhysicsBody(const Robot& robot);
  ~JSBSimPhysicsBody() override {}

  void InitializeJSBSimPhysicsBody(const Robot& robot);

  // Aggregate all externally applied wrenches on body CG into wrench_
  void CalculateExternalWrench() override;

  void ReadRobotData();
  void WriteRobotData(const Kinematics& kinematics,
                      TimeNano external_time_stamp = -1);

  bool IsStillGrounded();
  bool IsLandingCollision();
  bool NeedsCollisionResponse(const Kinematics& next_kin);

 protected:
  friend class JSBSimPhysicsModel;

  Robot sim_robot_;

  bool is_grounded_;
  Vector3 env_gravity_;
  float env_air_density_;
  Vector3 env_wind_velocity_;
  std::vector<DragFace> drag_faces_;
  std::vector<std::reference_wrapper<const Link>> lift_drag_links_;
  std::unordered_map<std::string, const float*> lift_drag_control_angles_;
  std::vector<const WrenchPoint*> external_wrench_points_;

  CollisionInfo collision_info_;  

  static constexpr float kGroundCollisionAxisTol = 0.01f;
  static constexpr float kCollisionOffset = 0.001f;  // land with 1 mm air gap

 private:
  void InitializeJSBSim();
  Kinematics GetKinematicsFromModel();
  Vector3 GetModelAngularAcceleration();
  Vector3 GetModelLinearAcceleration();
  Vector3 GetModelAngularVelocity();
  Vector3 GetModelLinearVelocity();
  Quaternion GetModelOrientation();
  Vector3 GetModelPosition();
  Vector3 GetModelCoordinates();
  void SetJSBSimAltitudeGrounded(float delta_altitude);

  Vector3d home_geopoint_ned_;
  Vector3 home_geopoint_;
  std::shared_ptr<JSBSim::FGFDMExec> model_;
};

// -----------------------------------------------------------------------------
// class JSBSimPhysicsModel

class JSBSimPhysicsModel {
 public:
  JSBSimPhysicsModel() {}
  ~JSBSimPhysicsModel() {}

  void SetWrenchesOnPhysicsBody(std::shared_ptr<BasePhysicsBody> body);

  void StepPhysicsBody(TimeNano dt_nanos,
                       std::shared_ptr<BasePhysicsBody> body);

  Kinematics CalcNextKinematicsNoCollision(
      TimeSec dt_sec, std::shared_ptr<JSBSimPhysicsBody>& fp_body);

  Kinematics CalcNextKinematicsWithCollision(
      TimeSec dt_sec, std::shared_ptr<JSBSimPhysicsBody>& fp_body);

 protected:
  static constexpr float kDragMinVelocity = 0.1f;
 private:
  TimeSec residual_time_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // PHYSICS_INCLUDE_JSBSIM_PHYSICS_HPP_