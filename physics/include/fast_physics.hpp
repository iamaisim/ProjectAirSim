// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef PHYSICS_INCLUDE_FAST_PHYSICS_HPP_
#define PHYSICS_INCLUDE_FAST_PHYSICS_HPP_

#include <vector>

#include "base_physics.hpp"
#include "core_sim/actor/robot.hpp"
#include "core_sim/environment.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/transforms/transform_tree.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class FastPhysicsBody

class FastPhysicsBody : public BasePhysicsBody {
 public:
  FastPhysicsBody() {}
  explicit FastPhysicsBody(const Robot& robot);
  ~FastPhysicsBody() override {}

  void InitializeFastPhysicsBody();

  // Aggregate all externally applied wrenches on body CG into wrench_
  void CalculateExternalWrench() override;

  Matrix3x3 CalculateInertia(const std::vector<Link>& links);

  std::vector<DragFace> InitializeDragFaces(const std::vector<Link>& links);

  std::vector<std::reference_wrapper<const Link>> InitializeLiftDragLinks(
      const std::vector<Link>& links);

  void ReadRobotData();
  void WriteRobotData(const Kinematics& kinematics,
                      TimeNano external_time_stamp = -1);

  bool IsStillGrounded();
  bool IsLandingCollision();
  bool NeedsCollisionResponse(const Kinematics& next_kin);

  // These conversion operators allow this object to be passed directly to
  // TransformTree methods
  operator TransformTree::RefFrame&(void);
  operator const TransformTree::RefFrame&(void) const;

 protected:
  // Entry for each external wrench
  struct ExternalWrenchEntry {
    const TransformTree::RefFrame*
        refframe;  // External wrench point's reference frame
    const WrenchPoint*
        wrench_point;  // External wrench point, relative to refframe

    ExternalWrenchEntry(const WrenchPoint* wrench_point_in,
                        const TransformTree::RefFrame* refframe_in)
        : refframe(refframe_in), wrench_point(wrench_point_in) {}
  };  // struct ExternalWrenchEntry

 protected:
  float CalculateWheelCollisionZ(void);
  void UpdateCollisionState(void);

 protected:
  friend class FastPhysicsModel;

  Robot sim_robot_;

  bool is_grounded_;
  Vector3 env_gravity_;
  float env_air_density_;
  Vector3 env_wind_velocity_;
  Vector3 ext_force_ = Vector3::Zero();
  std::vector<DragFace> drag_faces_;
  std::vector<std::reference_wrapper<const Link>> lift_drag_links_;
  std::unordered_map<std::string, const float*> lift_drag_control_angles_;
  std::vector<ExternalWrenchEntry> external_wrench_entries_;

  CollisionInfo collision_info_;

  static constexpr float kGroundCollisionAxisTol = 0.01f;
  static constexpr float kCollisionOffset = 0.001f;  // land with 1 mm air gap
  float rover_length_;                               //  for rovers
};

// -----------------------------------------------------------------------------
// class FastPhysicsModel

class FastPhysicsModel {
 public:
  FastPhysicsModel() {}
  ~FastPhysicsModel() {}

  void SetWrenchesOnPhysicsBody(std::shared_ptr<BasePhysicsBody> body);

  void StepPhysicsBody(TimeNano dt_nanos,
                       std::shared_ptr<BasePhysicsBody> body);

  Kinematics CalcNextKinematicsGrounded(const Vector3& position,
                                        const Quaternion& orientation);

  Kinematics CalcNextKinematicsNoCollision(
      TimeSec dt_sec, std::shared_ptr<FastPhysicsBody>& fp_body);

  Kinematics CalcNextKinematicsWithCollision(
      TimeSec dt_sec, std::shared_ptr<FastPhysicsBody>& fp_body);

  Kinematics CalcNextKinematicsWithWheels(
      TimeSec dt_sec, std::shared_ptr<FastPhysicsBody>& fp_body,
      const Vector3& landed_pos);

  Wrench CalcDragFaceWrench(const Vector3& ave_vel_lin,
                            const Vector3& ave_vel_ang,
                            const std::vector<DragFace>& drag_faces,
                            const Quaternion& orientation, float air_density,
                            const Vector3& wind_velocity);

  Wrench CalcLiftDragWrench(
      const std::vector<std::reference_wrapper<const Link>>& lift_drag_links,
      const std::unordered_map<std::string, const float*>&
          lift_drag_control_angles,
      const Quaternion& body_orientation, const Vector3& body_velocity,
      float air_density, const Vector3& wind_velocity);

 protected:
  static constexpr float kDragMinVelocity = 0.1f;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // PHYSICS_INCLUDE_FAST_PHYSICS_HPP_
