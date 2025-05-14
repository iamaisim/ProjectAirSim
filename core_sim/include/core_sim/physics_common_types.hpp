// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_PHYSICS_COMMON_TYPES_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_PHYSICS_COMMON_TYPES_HPP_

#include <string>
#include <unordered_map>

#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"

namespace microsoft {
namespace projectairsim {

enum class PhysicsType {
  kNonPhysics = 0,
  kFastPhysics = 1,
  kUnrealPhysics = 2,
  kMatlabPhysics = 3
};

struct Pose {
  Vector3 position = Vector3::Zero();
  Quaternion orientation = Quaternion::Identity();

  Pose() {}

  Pose(const Vector3& position_val, const Quaternion& orientation_val)
      : position(position_val), orientation(orientation_val) {}

  static const Pose Zero() {
    static const Pose zero_pose(Vector3::Zero(),
                                Quaternion(Quaternion::Identity()));
    return zero_pose;
  }
};

struct Twist {
  Vector3 linear = Vector3::Zero();
  Vector3 angular = Vector3::Zero();

  Twist() {}

  Twist(const Vector3& linear_val, const Vector3& angular_val)
      : linear(linear_val), angular(angular_val) {}

  static const Twist Zero() {
    static const Twist zero_twist(Vector3::Zero(), Vector3::Zero());
    return zero_twist;
  }
};

struct Accelerations {
  Vector3 linear = Vector3::Zero();
  Vector3 angular = Vector3::Zero();

  Accelerations() {}

  Accelerations(const Vector3& linear_val, const Vector3& angular_val)
      : linear(linear_val), angular(angular_val) {}

  static const Accelerations Zero() {
    static const Accelerations zero_val(Vector3::Zero(), Vector3::Zero());
    return zero_val;
  }
};

struct Kinematics {
  Pose pose = Pose::Zero();
  Twist twist = Twist::Zero();
  Accelerations accels = Accelerations::Zero();

  Kinematics() {}

  Kinematics(const Pose& pose_val, const Twist& twist_val,
             const Accelerations& accels_val)
      : pose(pose_val), twist(twist_val), accels(accels_val) {}

  static const Kinematics Zero() {
    static const Kinematics zero_kin(Pose::Zero(), Twist::Zero(),
                                     Accelerations::Zero());
    return zero_kin;
  }
};

struct Wrench {
  Vector3 force = Vector3::Zero();
  Vector3 torque = Vector3::Zero();

  Wrench() {}

  Wrench(const Vector3& force_val, const Vector3& torque_val)
      : force(force_val), torque(torque_val) {}

  // support basic arithmatic
  Wrench operator+(const Wrench& other) const {
    Wrench result;
    result.force = this->force + other.force;
    result.torque = this->torque + other.torque;
    return result;
  }

  Wrench operator+=(const Wrench& other) {
    force += other.force;
    torque += other.torque;
    return *this;
  }

  Wrench operator-(const Wrench& other) const {
    Wrench result;
    result.force = this->force - other.force;
    result.torque = this->torque - other.torque;
    return result;
  }

  Wrench operator-=(const Wrench& other) {
    force -= other.force;
    torque -= other.torque;
    return *this;
  }

  static const Wrench Zero() {
    static const Wrench zero_wrench(Vector3::Zero(), Vector3::Zero());
    return zero_wrench;
  }
};

struct WrenchPoint {  // Aggregated in AirSim's getBodyWrench()
  Vector3 position = Vector3::Zero();
  Wrench wrench = Wrench::Zero();

  WrenchPoint() {}
  WrenchPoint(const Vector3& position_val, const Wrench& wrench_val)
      : position(position_val), wrench(wrench_val) {}
};

struct DragFace {
  Vector3 position = Vector3::Zero();
  Vector3 normal = Vector3::Zero();
  float area = 0.0f;
  float drag_factor = 0.0f;

  DragFace() {}
  DragFace(const Vector3& position_val, const Vector3& normal_val,
           float area_val, float drag_factor_val)
      : position(position_val),
        normal(normal_val),
        area(area_val),
        drag_factor(drag_factor_val) {}
};

struct LiftDrag {
  // This enabled flag is automatically set when a link is loaded that has
  // lift-drag parameters so that physics world will track updating the
  // lift-drag wrench on that link.
  bool enabled = false;

  // The amount of Angle of Attack (AOA) alpha to shift the coefficient curves
  // to the right such that the zero coefficient point aligns with zero AOA.
  float alpha_0 = 0.0f;

  // The AOA at which the coefficient curve slope changes from the base one
  // (rising slope) to the stall one (falling slope). Note: This value also
  // includes the alpha_0 offset shift.
  float alpha_stall = 0.0f;

  // Coefficient of lift for base and stall condition.
  float c_lift_alpha = 0.0f;
  float c_lift_alpha_stall = 0.0f;

  // Coefficient of drag for base and stall condition.
  float c_drag_alpha = 0.0f;
  float c_drag_alpha_stall = 0.0f;

  // Coefficient of moment for base and stall condition.
  float c_moment_alpha = 0.0f;
  float c_moment_alpha_stall = 0.0f;

  // Surface area that's generating the lift-drag, m^2.
  float area = 0.0f;

  // Amount of lift/drag/moment coefficient to add per radian of control
  // surface actuation angle. For a pair of elevons moving together, pitching
  // moment will be generated. For differential elevon movement, pitching
  // moments are cancelled out and rolling lifts will be generated.
  float control_surface_cl_per_rad = 0.0f;
  float control_surface_cd_per_rad = 0.0f;
  float control_surface_cm_per_rad = 0.0f;

  // Center of pressure is the translation offset from the link's origin where
  // the lift-drag forces will be applied.
  Vector3 center_pressure_xyz = Vector3::Zero();  // local NED frame

  // Unit direction vectors, where forward is the direction for determining the
  // velocity component to generate lift-drag, and upward is the direction of
  // the generated lift. Drag is in the direction opposite to forward. Moment is
  // in the direction of the right-hand cross product of forward and upward.
  Vector3 forward_xyz = {1.0f, 0.0f, 0.0f};  // local NED frame unit vector
  Vector3 upward_xyz = {0.0f, 0.0f, -1.0f};  // local NED frame unit vector
};

struct CollisionInfo {  // Currently based on UE-provided collision info
  // TODO Generalize the core collision info for any external source
  bool has_collided = false;
  Vector3 normal = Vector3::Zero();
  Vector3 impact_point = Vector3::Zero();
  Vector3 position = Vector3::Zero();
  float penetration_depth = 0.0f;
  TimeNano time_stamp = 0;
  std::string object_name;
  int segmentation_id = -1;

  CollisionInfo() {}

  CollisionInfo(bool has_collided_val, const Vector3& normal_val,
                const Vector3& impact_point_val, const Vector3& position_val,
                float penetration_depth_val, TimeNano time_stamp_val,
                const std::string& object_name_val, int segmentation_id_val)
      : has_collided(has_collided_val),
        normal(normal_val),
        impact_point(impact_point_val),
        position(position_val),
        penetration_depth(penetration_depth_val),
        time_stamp(time_stamp_val),
        object_name(object_name_val),
        segmentation_id(segmentation_id_val) {}
};

// Map of actuated rotations with k:link ID, v:(rad/s x, rad/s y, rad/s z)
typedef std::unordered_map<std::string, Vector3> ActuatedRotations;

// Transform generated by actuator and how it is to be applied
struct ActuatedTransform {
  enum class ApplyOrder {
    Pre,   // Apply before initial transform
    PreTranslation,  // Apply after initial transform but before initial
                     // translation (useful for rotating)
    Post,            // Apply after initial transform
  } applyorder;
  Affine3 affine3;  // Transform to appy

  ActuatedTransform(void) : applyorder(ApplyOrder::PreTranslation), affine3() {}
  ActuatedTransform(const Affine3& affine3_in)
      : applyorder(ApplyOrder::PreTranslation), affine3(affine3_in) {}
  ActuatedTransform(const Affine3& affine3_in, ApplyOrder applyorder_in)
      : applyorder(applyorder_in), affine3(affine3_in) {}
  ActuatedTransform(const ActuatedTransform& actuatedtransform)
      : applyorder(actuatedtransform.applyorder),
        affine3(actuatedtransform.affine3) {}
};  // struct ActuatedTransform

// Map of actuated transforms with k:link ID, v:actuated transform
typedef std::unordered_map<std::string, ActuatedTransform> ActuatedTransforms;

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_PHYSICS_COMMON_TYPES_HPP_
