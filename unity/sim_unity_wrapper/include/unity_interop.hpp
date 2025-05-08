// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef UNITY_SIM_UNITY_WRAPPER_INCLUDE_UNITY_INTEROP_STRUCTS_HPP_
#define UNITY_SIM_UNITY_WRAPPER_INCLUDE_UNITY_INTEROP_STRUCTS_HPP_

#include "core_sim/math_utils.hpp"
#include "core_sim/message/image_message.hpp"
#include "core_sim/message/lidar_message.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/sensors/camera.hpp"
#include "core_sim/sensors/lidar.hpp"

namespace projectairsim = microsoft::projectairsim;

typedef projectairsim::ActuatedTransform::ApplyOrder ApplyOrder;

namespace UnityInterop {

struct InteropVector3 {
  float x;
  float y;
  float z;

  InteropVector3() : x(0), y(0), z(0) {}
  InteropVector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
  InteropVector3(projectairsim::Vector3 vec) {
    x = vec.x();
    y = vec.y();
    z = vec.z();
  }

  InteropVector3 operator+(const InteropVector3& V) const {
    return InteropVector3(x + V.x, y + V.y, z + V.z);
  }

  InteropVector3 operator-(const InteropVector3& V) const {
    return InteropVector3(x - V.x, y - V.y, z - V.z);
  }
};

struct InteropQuaternion {
  float x;
  float y;
  float z;
  float w;

  InteropQuaternion() : x(0), y(0), z(0), w(1) {}
  InteropQuaternion(float _x, float _y, float _z, float _w)
      : x(_x), y(_y), z(_z), w(_w) {}
  InteropQuaternion(projectairsim::Quaternion quat) {
    x = quat.x();
    y = quat.y();
    z = quat.z();
    w = quat.w();
  }
};

struct InteropPose {
  InteropVector3 position;
  InteropQuaternion orientation;

  InteropPose() {
    position = InteropVector3();
    orientation = InteropQuaternion();
  }

  InteropPose(const projectairsim::Pose& pose) {
    position = pose.position;
    orientation = pose.orientation;
  }

  InteropPose(const projectairsim::Transform& pose) {
    position = pose.translation_;
    orientation = pose.rotation_;
  }

  InteropPose(const InteropVector3& in_position,
              const InteropQuaternion& in_orientation) {
    position = in_position;
    orientation = in_orientation;
  }
};

struct InteropTwist {
  InteropVector3 linear;
  InteropVector3 angular;

  InteropTwist() {
    linear = InteropVector3();
    angular = InteropVector3();
  }

  InteropTwist(const projectairsim::Twist& twist) {
    linear = twist.linear;
    angular = twist.angular;
  }
};

struct InteropAccelerations {
  InteropVector3 linear;
  InteropVector3 angular;

  InteropAccelerations() {
    linear = InteropVector3();
    angular = InteropVector3();
  }

  InteropAccelerations(const projectairsim::Accelerations& accels) {
    linear = accels.linear;
    angular = accels.angular;
  }
};

struct InteropKinematics {
  InteropPose pose;
  InteropTwist twist;
  InteropAccelerations accels;

  InteropKinematics() {
    pose = InteropPose();
    twist = InteropTwist();
    accels = InteropAccelerations();
  }

  InteropKinematics(const projectairsim::Kinematics& kin) {
    pose = kin.pose;
    twist = kin.twist;
    accels = kin.accels;
  }
};

struct InteropActuatedTransform {
  InteropPose actuated_transform;
  ApplyOrder apply_order = ApplyOrder::PreTranslation;
  const char* actuated_link_id = nullptr;
};

struct InteropCollisionInfo {
  bool has_collided = false;
  InteropVector3 normal;
  InteropVector3 impact_point;
  InteropVector3 position;
  float penetration_depth = 0.0f;
  int64_t time_stamp = 0;
  char* object_name;
  int segmentation_id = -1;
};

static projectairsim::Vector3 ToSimVector3(const InteropVector3& vec) {
  projectairsim::Vector3 out_vec(vec.x, vec.y, vec.z);
  return out_vec;
}

static projectairsim::Quaternion ToSimQuaternion(
    const InteropQuaternion& quat) {
  projectairsim::Quaternion out_quat(quat.w, quat.x, quat.y, quat.z);
  return out_quat;
}

static projectairsim::Pose ToSimPose(const InteropPose& pose) {
  projectairsim::Pose out_pose(ToSimVector3(pose.position),
                               ToSimQuaternion(pose.orientation));
  return out_pose;
}

static projectairsim::CollisionInfo ToSimCollisionInfo(
    const InteropCollisionInfo& collision_info) {
  projectairsim::CollisionInfo out_collision_info;

  out_collision_info.has_collided = collision_info.has_collided;
  out_collision_info.impact_point = ToSimVector3(collision_info.impact_point);
  out_collision_info.normal = ToSimVector3(collision_info.normal);
  out_collision_info.segmentation_id = collision_info.segmentation_id;
  out_collision_info.object_name = collision_info.object_name;
  out_collision_info.penetration_depth = collision_info.penetration_depth;
  out_collision_info.position = ToSimVector3(collision_info.position);
  out_collision_info.time_stamp = collision_info.time_stamp;

  return out_collision_info;
}

struct InteropImageMessage {
  int64_t time_stamp = 0;
  uint32_t height = 0;
  uint32_t width = 0;
  char* encoding;
  bool big_endian = false;
  uint32_t step;
  unsigned char* image_data_uint;
  InteropVector3 pos;
  InteropQuaternion rot;
};

static projectairsim::ImageMessage ToSimImageMessage(
    const InteropImageMessage& interop_image_message,
    projectairsim::ImageType imageType) {
  bool isDepthImage =
      imageType == projectairsim::ImageType::kDepthPerspective ||
      imageType == projectairsim::ImageType::kDepthPlanar;
  uint32_t data_len;
  if (isDepthImage) {
    data_len = interop_image_message.height * interop_image_message.width * 2;
  } else {
    data_len = interop_image_message.height * interop_image_message.width * 3;
  }

  std::vector<uint8_t> out_image_data(data_len);
  for (int i = 0; i < data_len; i++) {
    out_image_data[i] =
        static_cast<uint8_t>(interop_image_message.image_data_uint[i]);
  }

  return projectairsim::ImageMessage(
      interop_image_message.time_stamp, interop_image_message.height,
      interop_image_message.width, interop_image_message.encoding,
      interop_image_message.big_endian, interop_image_message.step,
      std::move(out_image_data), std::vector<float>(), interop_image_message.pos.x,
      interop_image_message.pos.y, interop_image_message.pos.z,
      interop_image_message.rot.w, interop_image_message.rot.x,
      interop_image_message.rot.y, interop_image_message.rot.z);
  // TODO: add annotation
}

struct InteropLidarMessage {
  int64_t time_stamp = 0;
  int num_points = 0;
  float* point_cloud;
  int* segmentation_cloud;
  float* intensity_cloud;
  int* laser_index_cloud;
  InteropPose pose;
};

static projectairsim::LidarMessage ToSimLidarMessage(
    const InteropLidarMessage& interop_lidar_message) {
  int num_points = interop_lidar_message.num_points;

  std::vector<float> point_cloud(num_points * 3);
  for (int i = 0; i < num_points * 3; i++) {
    point_cloud[i] = interop_lidar_message.point_cloud[i];
  }

  std::vector<int> segmentation_cloud(num_points);
  for (int i = 0; i < num_points; i++) {
    segmentation_cloud[i] = interop_lidar_message.segmentation_cloud[i];
  }

  std::vector<float> intensity_cloud(num_points);
  for (int i = 0; i < num_points; i++) {
    intensity_cloud[i] = interop_lidar_message.intensity_cloud[i];
  }

  std::vector<int> laser_index_cloud(num_points);
  for (int i = 0; i < num_points; i++) {
    laser_index_cloud[i] = interop_lidar_message.laser_index_cloud[i];
  }

  // TODO unity support for az/el/range output format
  return projectairsim::LidarMessage(
      interop_lidar_message.time_stamp, std::move(point_cloud), std::vector<float>(),
      std::move(segmentation_cloud), std::move(intensity_cloud),
      std::move(laser_index_cloud), ToSimPose(interop_lidar_message.pose));
}

}  // namespace UnityInterop

#endif  // UNITY_SIM_UNITY_WRAPPER_INCLUDE_UNITY_INTEROP_STRUCTS_HPP_
