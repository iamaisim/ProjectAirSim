// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_SRC_MESSAGE_COMMON_UTILS_HPP_
#define CORE_SIM_SRC_MESSAGE_COMMON_UTILS_HPP_

#include <vector>

#include "core_sim/actuators/rotor.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/sensors/camera.hpp"
#include "core_sim/sensors/radar.hpp"
#include "json.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

struct Vector3Msgpack {
  float x = 0, y = 0, z = 0;
  MSGPACK_DEFINE_MAP(x, y, z);

  Vector3Msgpack() {}

  explicit Vector3Msgpack(const Vector3& v) {
    x = v.x();
    y = v.y();
    z = v.z();
  }
  Vector3 ToVector3() const { return Vector3(x, y, z); }
};

struct Vector2Msgpack {
  float x = 0, y = 0;
  MSGPACK_DEFINE_MAP(x, y);

  Vector2Msgpack() {}

  explicit Vector2Msgpack(const Vector2& v) {
    x = v.x();
    y = v.y();
  }
  Vector2 ToVector2() const { return Vector2(x, y); }
};

struct QuaternionMsgpack {
  float w = 1, x = 0, y = 0, z = 0;
  MSGPACK_DEFINE_MAP(w, x, y, z);

  QuaternionMsgpack() {}

  explicit QuaternionMsgpack(const Quaternion& q) {
    w = q.w();
    x = q.x();
    y = q.y();
    z = q.z();
  }
  Quaternion ToQuaternion() const { return Quaternion(w, x, y, z); }
};

struct PoseMsgpack {
  Vector3Msgpack position;
  QuaternionMsgpack orientation;
  MSGPACK_DEFINE_MAP(position, orientation);

  PoseMsgpack() {}

  explicit PoseMsgpack(const Pose& pose) {
    position = Vector3Msgpack(pose.position);
    orientation = QuaternionMsgpack(pose.orientation);
  }

  Pose ToPose() const {
    return Pose(position.ToVector3(), orientation.ToQuaternion());
  }
};

struct TwistMsgpack {
  Vector3Msgpack linear;
  Vector3Msgpack angular;
  MSGPACK_DEFINE_MAP(linear, angular);

  TwistMsgpack() {}

  explicit TwistMsgpack(const Twist& twist) {
    linear = Vector3Msgpack(twist.linear);
    angular = Vector3Msgpack(twist.angular);
  }

  Twist ToTwist() const {
    return Twist(linear.ToVector3(), angular.ToVector3());
  }
};

struct AccelerationsMsgpack {
  Vector3Msgpack linear;
  Vector3Msgpack angular;
  MSGPACK_DEFINE_MAP(linear, angular);

  AccelerationsMsgpack() {}

  explicit AccelerationsMsgpack(const Accelerations& accels) {
    linear = Vector3Msgpack(accels.linear);
    angular = Vector3Msgpack(accels.angular);
  }

  Accelerations ToAccelerations() const {
    return Accelerations(linear.ToVector3(), angular.ToVector3());
  }
};

struct KinematicsMsgpack {
  PoseMsgpack pose;
  TwistMsgpack twist;
  AccelerationsMsgpack accels;
  MSGPACK_DEFINE_MAP(pose, twist, accels);

  KinematicsMsgpack() {}

  explicit KinematicsMsgpack(const Kinematics& kin) {
    pose = PoseMsgpack(kin.pose);
    twist = TwistMsgpack(kin.twist);
    accels = AccelerationsMsgpack(kin.accels);
  }

  Kinematics ToKinematics() const {
    return Kinematics(pose.ToPose(), twist.ToTwist(), accels.ToAccelerations());
  }
};

struct ActuatedRotationsMsgpack {
  // k:link ID, v:(rad/s x, rad/s y, rad/s z)
  std::unordered_map<std::string, Vector3Msgpack> actuated_rotations;

  MSGPACK_DEFINE_MAP(actuated_rotations);

  ActuatedRotationsMsgpack() {}

  explicit ActuatedRotationsMsgpack(const ActuatedRotations& actuated_rots) {
    for (auto& actuated_rot : actuated_rots) {
      // Convert Vector3 to Vector3Msgpack
      actuated_rotations.emplace(actuated_rot.first,
                                 Vector3Msgpack(actuated_rot.second));
    }
  }

  ActuatedRotations ToActuatedRotations() const {
    // Convert Vector3Msgpack to Vector3
    ActuatedRotations actuated_rots_out;
    for (auto& actuated_rot : actuated_rotations) {
      actuated_rots_out.emplace(actuated_rot.first,
                                actuated_rot.second.ToVector3());
    }
    return actuated_rots_out;
  }
};

struct JsonMsgpack {
  std::vector<uint8_t> data;
  MSGPACK_DEFINE_MAP(data);

  JsonMsgpack() {}

  explicit JsonMsgpack(const json& data_json) {
    data = json::to_msgpack(data_json);
  }
  json ToJson() const { return json::from_msgpack(data); }
};

struct RadarDetectionMsgpack {
  float range = 0.0f;
  float azimuth = 0.0f;
  float elevation = 0.0f;
  float velocity = 0.0f;
  float rcs_sqm = 0.0f;
  MSGPACK_DEFINE_MAP(range, azimuth, elevation, velocity, rcs_sqm);

  RadarDetectionMsgpack() {}
  explicit RadarDetectionMsgpack(const RadarDetection& d)
      : range(d.range),
        azimuth(d.azimuth),
        elevation(d.elevation),
        velocity(d.velocity),
        rcs_sqm(d.rcs_sqm) {}

  RadarDetection ToRadarDetection() const {
    return RadarDetection(range, azimuth, elevation, velocity, rcs_sqm);
  }
};

struct RadarTrackMsgpack {
  int id = -1;
  float azimuth_est = 0.0f;
  float elevation_est = 0.0f;
  float range_est = 0.0f;
  Vector3Msgpack position_est;
  Vector3Msgpack velocity_est;
  Vector3Msgpack accel_est;
  float rcs_sqm = 0.0f;
  MSGPACK_DEFINE_MAP(id, azimuth_est, elevation_est, range_est, position_est,
                     velocity_est, accel_est, rcs_sqm);

  RadarTrackMsgpack() {}
  explicit RadarTrackMsgpack(const RadarTrack& t)
      : id(t.id),
        azimuth_est(t.azimuth_est),
        elevation_est(t.elevation_est),
        range_est(t.range_est),
        position_est(t.position_est),
        velocity_est(t.velocity_est),
        accel_est(t.accel_est),
        rcs_sqm(t.rcs_sqm) {}

  RadarTrack ToRadarTrack() const {
    return RadarTrack(id, azimuth_est, elevation_est, range_est,
                      position_est.ToVector3(), velocity_est.ToVector3(),
                      accel_est.ToVector3(), rcs_sqm);
  }
};

struct BBox2DMsgpack {
  Vector2Msgpack center;
  Vector2Msgpack size;
  MSGPACK_DEFINE_MAP(center, size);

  BBox2DMsgpack() {}

  explicit BBox2DMsgpack(const BBox2D& bbox2d) {
    center = Vector2Msgpack(bbox2d.center);
    size = Vector2Msgpack(bbox2d.size);
  }

  BBox2D ToBBox2D() const {
    return BBox2D(center.ToVector2(), size.ToVector2());
  }
};

struct BBox3DMsgpack {
  Vector3Msgpack center;
  Vector3Msgpack size;
  QuaternionMsgpack quaternion;
  MSGPACK_DEFINE_MAP(center, size, quaternion);

  BBox3DMsgpack() {}

  explicit BBox3DMsgpack(const BBox3D& bbox3d) {
    center = Vector3Msgpack(bbox3d.center);
    size = Vector3Msgpack(bbox3d.size);
    quaternion = QuaternionMsgpack(bbox3d.quaternion);
  }

  BBox3D ToBBox3D() const {
    return BBox3D(center.ToVector3(), size.ToVector3(),
                  quaternion.ToQuaternion());
  }
};

struct AnnotationMsgpack {
  std::string object_id;
  // TODO: add class label
  BBox2DMsgpack bbox2d;
  BBox3DMsgpack bbox3d;
  std::vector<Vector2Msgpack> bbox3d_in_image_space;
  std::vector<Vector3Msgpack> bbox3d_in_projection_space;
  MSGPACK_DEFINE_MAP(object_id, bbox2d, bbox3d, bbox3d_in_image_space,
                     bbox3d_in_projection_space);

  AnnotationMsgpack() {}
  explicit AnnotationMsgpack(const Annotation& a)
      : object_id(a.object_id),
        bbox3d_in_image_space(8),
        bbox3d_in_projection_space(8) {
    bbox2d = BBox2DMsgpack(a.bbox2D);
    bbox3d = BBox3DMsgpack(a.bbox3D);
    std::transform(
        a.bbox3d_in_image_space.begin(), a.bbox3d_in_image_space.end(),
        bbox3d_in_image_space.begin(),
        [](Vector2 v) -> Vector2Msgpack { return Vector2Msgpack(v); });
    std::transform(
        a.bbox3d_in_projection_space.begin(),
        a.bbox3d_in_projection_space.end(), bbox3d_in_projection_space.begin(),
        [](Vector3 v) -> Vector3Msgpack { return Vector3Msgpack(v); });
  }

  Annotation ToAnnotation() const {
    auto bbox_image_space = std::vector<Vector2>(8);
    auto bbox_projected_space = std::vector<Vector3>(8);
    std::transform(bbox3d_in_image_space.begin(), bbox3d_in_image_space.end(),
                   bbox_image_space.begin(),
                   [](Vector2Msgpack v) -> Vector2 { return v.ToVector2(); });
    std::transform(bbox3d_in_projection_space.begin(),
                   bbox3d_in_projection_space.end(),
                   bbox_projected_space.begin(),
                   [](Vector3Msgpack v) -> Vector3 { return v.ToVector3(); });
    return Annotation(object_id, bbox2d.ToBBox2D(), bbox3d.ToBBox3D(),
                      bbox_image_space, bbox_projected_space);
  }
};

struct RotorInfoMsgPack {
  std::string rotor_id;
  float speed = 0;
  float angle = 0;
  float torque = 0;
  float thrust = 0;
  MSGPACK_DEFINE_MAP(rotor_id, speed, angle, torque, thrust);

  RotorInfoMsgPack() {}

  explicit RotorInfoMsgPack(const RotorInfo& r)
      : rotor_id(r.rotor_id),
        speed(r.speed),
        angle(r.angle),
        torque(r.torque),
        thrust(r.thrust) {}

  RotorInfo ToRotorInfo() const {
    return RotorInfo(rotor_id, speed, angle, torque, thrust);
  }
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_MESSAGE_COMMON_UTILS_HPP_
