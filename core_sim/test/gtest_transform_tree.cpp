// Copyright (C) Microsoft Corporation. All rights reserved.

#include <cmath>

#include "core_sim/transforms/transform_tree.hpp"
#include "core_sim/transforms/transform_utils.hpp"
#include "gtest/gtest.h"

namespace projectairsim = microsoft::projectairsim;

TEST(TransformTree, Register) {
  // Test basic registration
  projectairsim::TransformTree::StaticRefFrame staticrefframeA("A");
  projectairsim::TransformTree transformtree;
  projectairsim::Pose poseA;
  projectairsim::Pose poseGlobal;

  transformtree.Register(&staticrefframeA,
                         projectairsim::TransformTree::kRefFrameGlobal);
}

TEST(TransformTree, Convert) {
  // Test basic conversion from a reference frame that's an immediate child of
  // the global reference frame to the global reference frame
  constexpr float ap_x = 1.0f;
  constexpr float ap_y = 2.0f;
  constexpr float ap_z = 3.0f;
  projectairsim::Quaternion quatA =
      projectairsim::TransformUtils::ToQuaternion(1.0f, 2.0f, 3.0f);
  projectairsim::TransformTree::StaticRefFrame staticrefframeA("A");
  projectairsim::TransformTree transformtree;
  projectairsim::Pose poseA;
  projectairsim::Pose poseGlobal;

  staticrefframeA.SetLocalPose(
      projectairsim::Pose(projectairsim::Vector3(ap_x, ap_y, ap_z), quatA));

  transformtree.Register(&staticrefframeA,
                         projectairsim::TransformTree::kRefFrameGlobal);
  transformtree.Convert(poseA, staticrefframeA,
                        projectairsim::TransformTree::kRefFrameGlobal,
                        &poseGlobal);

  // Verify the identity pose in A's reference frame is transformed into A's
  // pose in the global reference frame
  EXPECT_FLOAT_EQ(poseGlobal.position.x(), ap_x);
  EXPECT_FLOAT_EQ(poseGlobal.position.y(), ap_y);
  EXPECT_FLOAT_EQ(poseGlobal.position.z(), ap_z);

  EXPECT_FLOAT_EQ(poseGlobal.orientation.x(), quatA.x());
  EXPECT_FLOAT_EQ(poseGlobal.orientation.y(), quatA.y());
  EXPECT_FLOAT_EQ(poseGlobal.orientation.z(), quatA.z());
  EXPECT_FLOAT_EQ(poseGlobal.orientation.w(), quatA.w());
}

TEST(TransformTree, NonzeroRelativePositionAndTiltedFrame) {
  constexpr float ap_x = 0.0f;
  constexpr float ap_y = 0.0f;
  constexpr float ap_z = 0.0f;
  projectairsim::Quaternion quatA =
      projectairsim::TransformUtils::ToQuaternion(M_PI / 4, 0.0f, 0.0f);
  projectairsim::TransformTree::StaticRefFrame staticrefframeA("A");
  projectairsim::TransformTree transformtree;
  projectairsim::Pose poseA;
  poseA.position.z() = 1.0f;
  projectairsim::Pose poseGlobal;

  staticrefframeA.SetLocalPose(
      projectairsim::Pose(projectairsim::Vector3(ap_x, ap_y, ap_z), quatA));

  transformtree.Register(&staticrefframeA,
                         projectairsim::TransformTree::kRefFrameGlobal);
  transformtree.Convert(poseA, staticrefframeA,
                        projectairsim::TransformTree::kRefFrameGlobal,
                        &poseGlobal);

  // Verify the identity pose in A's reference frame is transformed into A's
  // pose in the global reference frame
  EXPECT_FLOAT_EQ(poseGlobal.position.x(), ap_x);
  EXPECT_NEAR(poseGlobal.position.y(), -(std::sin(M_PI / 4)), 1.0e-6);
  EXPECT_NEAR(poseGlobal.position.z(), (std::cos(M_PI / 4)), 1.0e-6);

  EXPECT_FLOAT_EQ(poseGlobal.orientation.x(), quatA.x());
  EXPECT_FLOAT_EQ(poseGlobal.orientation.y(), quatA.y());
  EXPECT_FLOAT_EQ(poseGlobal.orientation.z(), quatA.z());
  EXPECT_FLOAT_EQ(poseGlobal.orientation.w(), quatA.w());
}

TEST(TransformTree, Convert2) {
  // Test conversion from a reference frame that's not directly a child of the
  // global reference frame to the global reference frame
  constexpr float ap_x = 0.0f;
  constexpr float ap_y = 0.0f;
  constexpr float ap_z = 0.0f;
  constexpr float bp_x = 10.0f;
  constexpr float bp_y = 20.0f;
  constexpr float bp_z = 30.0f;
  projectairsim::Quaternion quatA =
      projectairsim::TransformUtils::ToQuaternion(M_PI, 0, 0);
  projectairsim::Quaternion quatB =
      projectairsim::TransformUtils::ToQuaternion(M_PI / 4, M_PI / 4, M_PI / 4);
  projectairsim::Quaternion quatGlobal = quatB * quatA;
  projectairsim::TransformTree::StaticRefFrame staticrefframeA("A");
  projectairsim::TransformTree::StaticRefFrame staticrefframeB("B");
  projectairsim::TransformTree transformtree;
  projectairsim::Pose poseB;
  projectairsim::Pose poseGlobal;

  staticrefframeA.SetLocalPose(
      projectairsim::Pose(projectairsim::Vector3(ap_x, ap_y, ap_z), quatA));
  staticrefframeB.SetLocalPose(
      projectairsim::Pose(projectairsim::Vector3(bp_x, bp_y, bp_z), quatB));

  transformtree.Register(&staticrefframeA,
                         projectairsim::TransformTree::kRefFrameGlobal);
  transformtree.Register(&staticrefframeB, staticrefframeA);
  transformtree.Convert(poseB, staticrefframeB,
                        projectairsim::TransformTree::kRefFrameGlobal,
                        &poseGlobal);

  // Verify the identity pose in B's reference frame is transformed into B's
  // pose in the global reference frame
  EXPECT_FLOAT_EQ(poseGlobal.position.x(), bp_x);
  EXPECT_FLOAT_EQ(poseGlobal.position.y(), -bp_y);
  EXPECT_FLOAT_EQ(poseGlobal.position.z(), -bp_z);

  EXPECT_FLOAT_EQ(poseGlobal.orientation.x(), quatGlobal.x());
  EXPECT_FLOAT_EQ(poseGlobal.orientation.y(), quatGlobal.y());
  EXPECT_FLOAT_EQ(poseGlobal.orientation.z(), quatGlobal.z());
  EXPECT_FLOAT_EQ(poseGlobal.orientation.w(), quatGlobal.w());
}

TEST(TransformTree, ConvertCross) {
  // Test conversion from a reference frame to another where both are children
  // of the global reference frame and neither are descendents of the other
  constexpr float ap_x = 0.0f;
  constexpr float ap_y = 1.0f;
  constexpr float ap_z = 0.0f;
  constexpr float bp_x = 0.0f;
  constexpr float bp_y = -1.0f;
  constexpr float bp_z = 0.0f;
  projectairsim::Quaternion quatA =
      projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f, M_PI);
  projectairsim::Quaternion quatB =
      projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f, M_PI);
  projectairsim::Quaternion quatR =
      projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f, 0.0f);
  projectairsim::Quaternion quatAInB = quatA * quatB.inverse();
  projectairsim::TransformTree::StaticRefFrame staticrefframeA("A");
  projectairsim::TransformTree::StaticRefFrame staticrefframeB("B");
  projectairsim::TransformTree transformtree;
  projectairsim::Pose poseA(projectairsim::Vector3(bp_x, bp_y, bp_z), quatR);
  projectairsim::Pose poseR;

  staticrefframeA.SetLocalPose(
      projectairsim::Pose(projectairsim::Vector3(ap_x, ap_y, ap_z), quatA));
  staticrefframeB.SetLocalPose(
      projectairsim::Pose(projectairsim::Vector3(bp_x, bp_y, bp_z), quatB));

  transformtree.Register(&staticrefframeA,
                         projectairsim::TransformTree::kRefFrameGlobal);
  transformtree.Register(&staticrefframeB,
                         projectairsim::TransformTree::kRefFrameGlobal);
  transformtree.Convert(poseA, staticrefframeA, staticrefframeB, &poseR);

  // Verify the identity pose in A's reference frame is transformed into the
  // expected pose in B's reference frame
  EXPECT_NEAR(poseR.position.x(), 0.0f, 1e-6);
  EXPECT_NEAR(poseR.position.y(), bp_y - ap_y + bp_y, 1e-6);
  EXPECT_NEAR(poseR.position.z(), 0.0f, 1e-6);

  EXPECT_FLOAT_EQ(poseR.orientation.x(), quatAInB.x());
  EXPECT_FLOAT_EQ(poseR.orientation.y(), quatAInB.y());
  EXPECT_FLOAT_EQ(poseR.orientation.z(), quatAInB.z());
  EXPECT_FLOAT_EQ(poseR.orientation.w(), quatAInB.w());
}

TEST(TransformTree, ConvertCross2) {
  // Test conversion from a reference frame to another where both are not direct
  // children of the global reference frame and neither are descendents of the
  // other
  projectairsim::Pose poseA;
  projectairsim::Pose poseB;
  projectairsim::Quaternion quatA =
      projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f, 0.0f);
  projectairsim::Quaternion quatB =
      projectairsim::TransformUtils::ToQuaternion(0.0f, M_PI, 0.0f);
  projectairsim::Quaternion quatA2 =
      projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f, 0.0f);
  projectairsim::Quaternion quatB2 =
      projectairsim::TransformUtils::ToQuaternion(0.0f, M_PI, 0.0f);
  projectairsim::Quaternion quatAToGlobal = quatA * quatA2;
  projectairsim::Quaternion quatGlobalToB = quatB2.inverse() * quatB.inverse();
  projectairsim::Quaternion quatAInB =
      poseA.orientation * quatAToGlobal * quatGlobalToB;
  projectairsim::TransformTree::StaticRefFrame staticrefframeA("A");
  projectairsim::TransformTree::StaticRefFrame staticrefframeA2("A2");
  projectairsim::TransformTree::StaticRefFrame staticrefframeB("B");
  projectairsim::TransformTree::StaticRefFrame staticrefframeB2("B2");
  projectairsim::TransformTree transformtree;
  projectairsim::Vector3 vec3A = projectairsim::Vector3(1.0f, 2.0f, 3.0f);
  projectairsim::Vector3 vec3A2 = projectairsim::Vector3(4.0f, 5.0f, 6.0f);
  projectairsim::Vector3 vec3B = projectairsim::Vector3(-10.0f, 20.0f, -30.0f);
  projectairsim::Vector3 vec3B2 = projectairsim::Vector3(40.0f, 50.0f, 60.0f);
  projectairsim::Vector3 vec3AToGlobal = vec3A + vec3A2;
  projectairsim::Vector3 vec3BToB2Global = quatB2 * vec3B;
  projectairsim::Vector3 vec3GlobalToB = -vec3B2 - vec3BToB2Global;
  projectairsim::Vector3 vec3AInB =
      poseA.position + vec3AToGlobal + vec3GlobalToB;

  staticrefframeA.SetLocalPose(projectairsim::Pose(vec3A, quatA));
  staticrefframeA2.SetLocalPose(projectairsim::Pose(vec3A2, quatA2));
  staticrefframeB.SetLocalPose(projectairsim::Pose(vec3B, quatB));
  staticrefframeB2.SetLocalPose(projectairsim::Pose(vec3B2, quatB2));

  // Create transform tree with Global-->A2-->A and Global-->B2-->B
  transformtree.Register(&staticrefframeA2,
                         projectairsim::TransformTree::kRefFrameGlobal);
  transformtree.Register(&staticrefframeB2,
                         projectairsim::TransformTree::kRefFrameGlobal);
  transformtree.Register(&staticrefframeA, staticrefframeA2);
  transformtree.Register(&staticrefframeB, staticrefframeB2);

  // Convert from leaf reference frame A to leaf reference frame B
  transformtree.Convert(poseA, staticrefframeA, staticrefframeB, &poseB);

  // Verify the pose in A's reference frame is transformed into the expected
  // pose in B's reference frame
  EXPECT_FLOAT_EQ(poseB.position.x(), vec3AInB.x());
  EXPECT_FLOAT_EQ(poseB.position.y(), vec3AInB.y());
  EXPECT_FLOAT_EQ(poseB.position.z(), vec3AInB.z());

  // Beware that error could be slightly greater than 1 ULP due to multiple
  // quaternion multiplies and us calculating the effective rotation in a
  // different order than TransformTree.  Single-precision floating point values
  // have around 7-8 decimal digits of precision
  EXPECT_NEAR(poseB.orientation.x(), quatAInB.x(), 1.0e-6);
  EXPECT_NEAR(poseB.orientation.y(), quatAInB.y(), 1.0e-6);
  EXPECT_NEAR(poseB.orientation.z(), quatAInB.z(), 1.0e-6);
  EXPECT_NEAR(poseB.orientation.w(), quatAInB.w(), 1.0e-6);
}

TEST(TransformTree, ConvertSemiCross) {
  // Test conversion from a reference frame to another where one is not a direct
  // children of the global reference frame and neither are descendents of the
  // other
  projectairsim::Pose poseAStart;
  projectairsim::Pose poseA;
  projectairsim::Pose poseB;
  projectairsim::Quaternion quatA =
      projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f, 0.0f);
  projectairsim::Quaternion quatB =
      projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f, 0.0f);
  projectairsim::Quaternion quatB2 =
      projectairsim::TransformUtils::ToQuaternion(0.0f, 0.0f, 0.0f);
  projectairsim::Quaternion quatGlobalToB = quatB2.inverse() * quatB.inverse();
  projectairsim::Quaternion quatAInB =
      poseA.orientation * quatA * quatGlobalToB;
  projectairsim::TransformTree::StaticRefFrame staticrefframeA("A");
  projectairsim::TransformTree::StaticRefFrame staticrefframeB("B");
  projectairsim::TransformTree::StaticRefFrame staticrefframeB2("B2");
  projectairsim::TransformTree transformtree;
  projectairsim::Vector3 vec3A = projectairsim::Vector3(1.0f, 2.0f, 3.0f);
  projectairsim::Vector3 vec3B = projectairsim::Vector3(10.0f, 20.0f, 30.0f);
  projectairsim::Vector3 vec3B2 = projectairsim::Vector3(40.0f, 50.0f, 60.0f);
  projectairsim::Vector3 vec3GlobalToB = -vec3B2 - vec3B;
  projectairsim::Vector3 vec3AInB = poseA.position + vec3A + vec3GlobalToB;

  staticrefframeA.SetLocalPose(projectairsim::Pose(vec3A, quatA));
  staticrefframeB.SetLocalPose(projectairsim::Pose(vec3B, quatB));
  staticrefframeB2.SetLocalPose(projectairsim::Pose(vec3B2, quatB2));

  // Create transform tree with Global-->A and Global-->B2-->B
  transformtree.Register(&staticrefframeB2,
                         projectairsim::TransformTree::kRefFrameGlobal);
  transformtree.Register(&staticrefframeA,
                         projectairsim::TransformTree::kRefFrameGlobal);
  transformtree.Register(&staticrefframeB, staticrefframeB2);

  // Convert from leaf reference frame A to leaf reference frame B
  poseA = poseAStart;
  transformtree.Convert(poseA, staticrefframeA, staticrefframeB, &poseB);

  // Verify the pose in A's reference frame is transformed into the expected
  // pose in B's reference frame
  EXPECT_FLOAT_EQ(poseB.position.x(), vec3AInB.x());
  EXPECT_FLOAT_EQ(poseB.position.y(), vec3AInB.y());
  EXPECT_FLOAT_EQ(poseB.position.z(), vec3AInB.z());

  // Beware that error could be slightly greater than 1 ULP due to multiple
  // quaternion multiplies and us calculating the effective rotation in a
  // different order than TransformTree.  Single-precision floating point values
  // have around 7-8 decimal digits of precision
  EXPECT_NEAR(poseB.orientation.x(), quatAInB.x(), 1.0e-6);
  EXPECT_NEAR(poseB.orientation.y(), quatAInB.y(), 1.0e-6);
  EXPECT_NEAR(poseB.orientation.z(), quatAInB.z(), 1.0e-6);
  EXPECT_NEAR(poseB.orientation.w(), quatAInB.w(), 1.0e-6);

  // Convert from leaf reference frame B back to leaf reference frame A
  transformtree.Convert(poseB, staticrefframeB, staticrefframeA, &poseA);

  // Verify the pose in B's reference frame is transformed into the expected
  // pose in A's reference frame
  EXPECT_FLOAT_EQ(poseA.position.x(), poseAStart.position.x());
  EXPECT_FLOAT_EQ(poseA.position.y(), poseAStart.position.y());
  EXPECT_FLOAT_EQ(poseA.position.z(), poseAStart.position.z());

  EXPECT_NEAR(poseA.orientation.x(), poseAStart.orientation.x(), 1.0e-6);
  EXPECT_NEAR(poseA.orientation.y(), poseAStart.orientation.y(), 1.0e-6);
  EXPECT_NEAR(poseA.orientation.z(), poseAStart.orientation.z(), 1.0e-6);
  EXPECT_NEAR(poseA.orientation.w(), poseAStart.orientation.w(), 1.0e-6);
}