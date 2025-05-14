// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "core_sim/transforms/transform_utils.hpp"
#include "gtest/gtest.h"

namespace projectairsim = microsoft::projectairsim;

TEST(TransformUtils, ToQuaternion) {
  // Test basic example case
  constexpr float t0_roll = M_PI / 6.0f;
  constexpr float t0_pitch = M_PI / 3.0f;
  constexpr float t0_yaw = M_PI / 2.0f;
  constexpr float a0_w = 0.68301266f;
  constexpr float a0_x = -0.18301269f;
  constexpr float a0_y = 0.5f;
  constexpr float a0_z = 0.5f;

  auto q = projectairsim::TransformUtils::ToQuaternion(t0_roll, t0_pitch, t0_yaw);

  EXPECT_FLOAT_EQ(q.w(), a0_w);
  EXPECT_FLOAT_EQ(q.x(), a0_x);
  EXPECT_FLOAT_EQ(q.y(), a0_y);
  EXPECT_FLOAT_EQ(q.z(), a0_z);

  // Test near pitch singularity clip at 89.9 deg
  constexpr float t1_roll = projectairsim::TransformUtils::kEulerSingularityMinor;
  constexpr float t1_pitch = projectairsim::TransformUtils::kEulerSingularityMinor;
  constexpr float t1_yaw = projectairsim::TransformUtils::kEulerSingularityMinor;
  constexpr float a1_w = 0.7071075f;
  constexpr float a1_x = 0.000617057f;
  constexpr float a1_y = 0.7071054f;
  constexpr float a1_z = 0.000617057f;

  q = projectairsim::TransformUtils::ToQuaternion(t1_roll, t1_pitch, t1_yaw);

  EXPECT_FLOAT_EQ(q.w(), a1_w);
  EXPECT_FLOAT_EQ(q.x(), a1_x);
  EXPECT_FLOAT_EQ(q.y(), a1_y);
  EXPECT_FLOAT_EQ(q.z(), a1_z);

  // Test over pitch singularity clip at 89.9 deg
  constexpr float t2_roll = t1_roll;
  constexpr float t2_pitch =
      89.99 * M_PI / 180.0f;
  constexpr float t2_yaw = t1_yaw;
  constexpr float a2_w = a1_w;
  constexpr float a2_x = a1_x;
  constexpr float a2_y = a1_y;
  constexpr float a2_z = a1_z;

  q = projectairsim::TransformUtils::ToQuaternion(t2_roll, t2_pitch, t2_yaw);

  EXPECT_FLOAT_EQ(q.w(), a2_w);
  EXPECT_FLOAT_EQ(q.x(), a2_x);
  EXPECT_FLOAT_EQ(q.y(), a2_y);
  EXPECT_FLOAT_EQ(q.z(), a2_z);

  // Test near pitch singularity clip at 90.1 deg
  constexpr float t1b_roll = projectairsim::TransformUtils::kEulerSingularityMajor;
  constexpr float t1b_pitch = projectairsim::TransformUtils::kEulerSingularityMajor;
  constexpr float t1b_yaw = projectairsim::TransformUtils::kEulerSingularityMajor;
  constexpr float a1b_w = 0.7071075f;
  constexpr float a1b_x = -0.00061708689f;
  constexpr float a1b_y = 0.7071054f;
  constexpr float a1b_z = -0.00061708689f;

  q = projectairsim::TransformUtils::ToQuaternion(t1b_roll, t1b_pitch, t1b_yaw);

  EXPECT_FLOAT_EQ(q.w(), a1b_w);
  EXPECT_FLOAT_EQ(q.x(), a1b_x);
  EXPECT_FLOAT_EQ(q.y(), a1b_y);
  EXPECT_FLOAT_EQ(q.z(), a1b_z);

  // Test over pitch singularity clip at 90.1 deg
  constexpr float t2b_roll = t1b_roll;
  constexpr float t2b_pitch =
      90.01 * M_PI / 180.0f;
  constexpr float t2b_yaw = t1b_yaw;
  constexpr float a2b_w = a1b_w;
  constexpr float a2b_x = a1b_x;
  constexpr float a2b_y = a1b_y;
  constexpr float a2b_z = a1b_z;

  q = projectairsim::TransformUtils::ToQuaternion(t2b_roll, t2b_pitch, t2b_yaw);

  EXPECT_FLOAT_EQ(q.w(), a2b_w);
  EXPECT_FLOAT_EQ(q.x(), a2b_x);
  EXPECT_FLOAT_EQ(q.y(), a2b_y);
  EXPECT_FLOAT_EQ(q.z(), a2b_z);

  // Test near pitch singularity negative clip at -89.9 deg
  constexpr float t3_roll = -projectairsim::TransformUtils::kEulerSingularityMinor;
  constexpr float t3_pitch = -projectairsim::TransformUtils::kEulerSingularityMinor;
  constexpr float t3_yaw = -projectairsim::TransformUtils::kEulerSingularityMinor;
  constexpr float a3_w = 0.0018511415f;
  constexpr float a3_x = -0.7071054f;
  constexpr float a3_y = -0.00061705709f;
  constexpr float a3_z = -0.7071054f;

  q = projectairsim::TransformUtils::ToQuaternion(t3_roll, t3_pitch, t3_yaw);

  EXPECT_FLOAT_EQ(q.w(), a3_w);
  EXPECT_FLOAT_EQ(q.x(), a3_x);
  EXPECT_FLOAT_EQ(q.y(), a3_y);
  EXPECT_FLOAT_EQ(q.z(), a3_z);

  // Test over pitch singularity negative clip at -89.9 deg
  constexpr float t4_roll = t3_roll;
  constexpr float t4_pitch = -t2_pitch;
  constexpr float t4_yaw = t3_yaw;
  constexpr float a4_w = a3_w;
  constexpr float a4_x = a3_x;
  constexpr float a4_y = a3_y;
  constexpr float a4_z = a3_z;

  q = projectairsim::TransformUtils::ToQuaternion(t4_roll, t4_pitch, t4_yaw);

  EXPECT_FLOAT_EQ(q.w(), a4_w);
  EXPECT_FLOAT_EQ(q.x(), a4_x);
  EXPECT_FLOAT_EQ(q.y(), a4_y);
  EXPECT_FLOAT_EQ(q.z(), a4_z);

  // Test near pitch singularity negative clip at -90.1 deg
  constexpr float t3b_roll = -projectairsim::TransformUtils::kEulerSingularityMajor;
  constexpr float t3b_pitch = -projectairsim::TransformUtils::kEulerSingularityMajor;
  constexpr float t3b_yaw = -projectairsim::TransformUtils::kEulerSingularityMajor;
  constexpr float a3b_w = -0.0018512607f;
  constexpr float a3b_x = -0.7071054f;
  constexpr float a3b_y = 0.00061711669f;
  constexpr float a3b_z = -0.7071054f;

  q = projectairsim::TransformUtils::ToQuaternion(t3b_roll, t3b_pitch, t3b_yaw);

  EXPECT_FLOAT_EQ(q.w(), a3b_w);
  EXPECT_FLOAT_EQ(q.x(), a3b_x);
  EXPECT_FLOAT_EQ(q.y(), a3b_y);
  EXPECT_FLOAT_EQ(q.z(), a3b_z);

  // Test over pitch singularity negative clip at -90.1 deg
  constexpr float t4b_roll = t3b_roll;
  constexpr float t4b_pitch = -t2b_pitch;
  constexpr float t4b_yaw = t3b_yaw;
  constexpr float a4b_w = a3b_w;
  constexpr float a4b_x = a3b_x;
  constexpr float a4b_y = a3b_y;
  constexpr float a4b_z = a3b_z;

  q = projectairsim::TransformUtils::ToQuaternion(t4b_roll, t4b_pitch, t4b_yaw);

  EXPECT_FLOAT_EQ(q.w(), a4b_w);
  EXPECT_FLOAT_EQ(q.x(), a4b_x);
  EXPECT_FLOAT_EQ(q.y(), a4b_y);
  EXPECT_FLOAT_EQ(q.z(), a4b_z);

  // Test roll allowed without any clipping
  constexpr float t5_roll = t2_pitch;
  constexpr float t5_pitch = t1_pitch;
  constexpr float t5_yaw = t1_yaw;
  constexpr float a5_w = 0.70710683;
  constexpr float a5_x = 0.0011723936;
  constexpr float a5_y = 0.70710564f;
  constexpr float a5_z = 6.172061e-05f;

  q = projectairsim::TransformUtils::ToQuaternion(t5_roll, t5_pitch, t5_yaw);

  EXPECT_FLOAT_EQ(q.w(), a5_w);
  EXPECT_FLOAT_EQ(q.x(), a5_x);
  EXPECT_FLOAT_EQ(q.y(), a5_y);
  EXPECT_FLOAT_EQ(q.z(), a5_z);

  // a5 would be equal to a1 if roll clipped
  EXPECT_NE(a5_w, a1_w);
  EXPECT_NE(a5_x, a1_x);
  EXPECT_NE(a5_y, a1_y);
  EXPECT_NE(a5_z, a1_z);

  // Test yaw allowed without any clipping
  constexpr float t6_roll = t1_roll;
  constexpr float t6_pitch = t1_pitch;
  constexpr float t6_yaw = t2_pitch;
  constexpr float a6_w = 0.70710683f;
  constexpr float a6_x = 6.172061e-05f;
  constexpr float a6_y = 0.70710564f;
  constexpr float a6_z = 0.0011723936f;

  q = projectairsim::TransformUtils::ToQuaternion(t6_roll, t6_pitch, t6_yaw);

  EXPECT_FLOAT_EQ(q.w(), a6_w);
  EXPECT_FLOAT_EQ(q.x(), a6_x);
  EXPECT_FLOAT_EQ(q.y(), a6_y);
  EXPECT_FLOAT_EQ(q.z(), a6_z);

  // a6 would be equal to a1 if yaw clipped
  EXPECT_NE(a6_w, a1_w);
  EXPECT_NE(a6_x, a1_x);
  EXPECT_NE(a6_y, a1_y);
  EXPECT_NE(a6_z, a1_z);

  // Test combining clipped pitch and yaw allowed without any clipping
  constexpr float t7_roll = t1_roll;
  constexpr float t7_pitch = t2_pitch;
  constexpr float t7_yaw = t2_pitch;
  constexpr float a7_w = a6_w;
  constexpr float a7_x = a6_x;
  constexpr float a7_y = a6_y;
  constexpr float a7_z = a6_z;

  q = projectairsim::TransformUtils::ToQuaternion(t7_roll, t7_pitch, t7_yaw);

  EXPECT_FLOAT_EQ(q.w(), a7_w);
  EXPECT_FLOAT_EQ(q.x(), a7_x);
  EXPECT_FLOAT_EQ(q.y(), a7_y);
  EXPECT_FLOAT_EQ(q.z(), a7_z);
}

TEST(TransformUtils, ToRpy) {
  microsoft::projectairsim::Quaternion q(0.68301266f, -0.18301269f, 0.5f,
                                       0.5f);  // w, x, y, z

  auto v = projectairsim::TransformUtils::ToRPY(q);

  EXPECT_FLOAT_EQ(v.x(), M_PI / 6.0f);
  EXPECT_FLOAT_EQ(v.y(), M_PI / 3.0f);
  EXPECT_FLOAT_EQ(v.z(), M_PI / 2.0f);
}

TEST(TransformUtils, Vector3ToCentimeters) {
  auto x_meters = projectairsim::Vector3(1, 2, 3);
  auto v = projectairsim::TransformUtils::ToCentimeters<projectairsim::Vector3>(
      x_meters);

  EXPECT_FLOAT_EQ(v.x(), 100);
  EXPECT_FLOAT_EQ(v.y(), 200);
  EXPECT_FLOAT_EQ(v.z(), 300);
}

TEST(TransformUtils, FloatToCentimeters) {
  float x_meters = 1.23;
  auto v = projectairsim::TransformUtils::ToCentimeters<float>(x_meters);

  EXPECT_FLOAT_EQ(v, 123);
}

TEST(TransformUtils, Vector3ToMeters) {
  auto x_centimeters = projectairsim::Vector3(100, 200, 300);
  auto v = projectairsim::TransformUtils::ToMeters<projectairsim::Vector3>(
      x_centimeters);

  EXPECT_FLOAT_EQ(v.x(), 1.0);
  EXPECT_FLOAT_EQ(v.y(), 2.0);
  EXPECT_FLOAT_EQ(v.z(), 3.0);
}

TEST(TransformUtils, FloatToMeters) {
  float x_centimeters = 123;
  auto v = projectairsim::TransformUtils::ToMeters<float>(x_centimeters);

  EXPECT_FLOAT_EQ(v, 1.23);
}