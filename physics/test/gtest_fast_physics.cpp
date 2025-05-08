// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/actor.hpp"
#include "core_sim/actor/robot.hpp"
#include "core_sim/simulator.hpp"
#include "core_sim/transforms/transform_utils.hpp"
#include "fast_physics.hpp"
#include "gtest/gtest.h"
#include "test_data/physics_test_config.hpp"  // defines physics_test_config

namespace microsoft {
namespace projectairsim {

class TestFastPhysicsBody : public FastPhysicsBody {
 public:
  using FastPhysicsBody::ExternalWrenchEntry;

 public:
  TestFastPhysicsBody() {}
  explicit TestFastPhysicsBody(const Robot& robot) : FastPhysicsBody(robot) {}

  void SetExternalWrench(Wrench wrench) { external_wrench_ = wrench; }
  void SetEnvGravity(Vector3 gravity) { env_gravity_ = gravity; }
  void SetIsGrounded(bool is_grounded) { is_grounded_ = is_grounded; }
  void SetKinematics(Kinematics kinematics) { kinematics_ = kinematics; }
  void SetCollisionInfo(CollisionInfo col_info) { collision_info_ = col_info; }
  void SetExternalWrenchEntries(
      std::vector<ExternalWrenchEntry> external_wrench_entries) {
    external_wrench_entries_ = external_wrench_entries;
  }
  void SetExternalForce(const std::vector<float>& ext_force) {
    ext_force_ = Vector3(ext_force[0], ext_force[1], ext_force[2]);
  }
  Wrench GetExternalWrench() { return external_wrench_; }
  Vector3 GetEnvGravity() { return env_gravity_; }
  float GetEnvAirDensity() { return env_air_density_; }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;

// -----------------------------------------------------------------------------
// class FastPhysicsBody

TEST(FastPhysicsBody, Constructor) {
  auto body = std::make_unique<projectairsim::FastPhysicsBody>();
  EXPECT_NE(body, nullptr);
}

TEST(FastPhysicsBody, BasePhysicsSetGetName) {
  projectairsim::FastPhysicsBody body;
  EXPECT_EQ(body.GetName(), "");

  body.SetName("Test_Name");
  EXPECT_EQ(body.GetName(), "Test_Name");
}

TEST(FastPhysicsBody, BasePhysicsSetGetMass) {
  projectairsim::FastPhysicsBody body;
  body.SetMass(4.0);
  EXPECT_FLOAT_EQ(body.GetMass(), 4.0f);
  EXPECT_FLOAT_EQ(body.GetMassInv(), 0.25f);
}

TEST(FastPhysicsBody, BasePhysicsSetGetInertia) {
  projectairsim::FastPhysicsBody body;
  projectairsim::Matrix3x3 inertia;
  inertia << 1., 0., 0., 0., 2., 0., 0., 0., 3.;
  body.SetInertia(inertia);
  EXPECT_EQ(body.GetInertia(), inertia);
  EXPECT_EQ(body.GetInertiaInv(), inertia.inverse());
}

TEST(FastPhysicsBody, GetPhysicsType) {
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();
  auto actors = scene.GetActors();
  EXPECT_EQ(actors.size(), 1);

  for (auto actor_ref : actors) {
    if (actor_ref.get().GetType() == projectairsim::ActorType::kRobot) {
      auto& sim_robot = dynamic_cast<projectairsim::Robot&>(actor_ref.get());
      projectairsim::FastPhysicsBody body(sim_robot);
      EXPECT_EQ(body.GetPhysicsType(),
                projectairsim::PhysicsType::kFastPhysics);
    }
  }
}

TEST(FastPhysicsBody, CalculateExternalWrench) {
  projectairsim::TransformTree::RefFrame refframe("TestBody");
  projectairsim::TransformTree transform_tree;
  projectairsim::TestFastPhysicsBody body;
  projectairsim::Kinematics kin;

  // Wrenches are in the "TestBody" reference frame which is a child of the
  // global frame
  transform_tree.Register(refframe,
                          projectairsim::TransformTree::kRefFrameGlobal);

  kin.pose.orientation =
      projectairsim::TransformUtils::ToQuaternion(0., 0., 0.);
  body.SetKinematics(kin);

  std::vector<projectairsim::TestFastPhysicsBody::ExternalWrenchEntry>
      wrench_entries;
  projectairsim::WrenchPoint wrench_pt1;
  projectairsim::WrenchPoint wrench_pt2;
  projectairsim::Vector3 force_ans;
  projectairsim::Vector3 torque_ans;
  projectairsim::Wrench ext_wrench;

  // Cancelled force on Y axis, reactive torque on Z axis
  wrench_entries.clear();
  wrench_pt1.position = {1., 0., 0.};
  wrench_pt1.wrench = projectairsim::Wrench({0., 2., 0.}, {0., 0., 0.});
  wrench_entries.emplace_back(&wrench_pt1, &refframe);

  wrench_pt2.position = {2., 0., 0.};
  wrench_pt2.wrench = projectairsim::Wrench({0., -2., 0.}, {0., 0., 0.});
  wrench_entries.emplace_back(&wrench_pt2, &refframe);

  body.SetExternalWrenchEntries(wrench_entries);
  body.CalculateExternalWrench();

  ext_wrench = body.GetExternalWrench();
  force_ans = {0., 0., 0.};
  EXPECT_EQ(ext_wrench.force, force_ans);

  torque_ans = {0., 0., -2.};
  EXPECT_EQ(ext_wrench.torque, torque_ans);

  // Additive force on Y axis, cancelled torque on Z axis
  wrench_entries.clear();
  wrench_pt1.position = {1., 0., 0.};
  wrench_pt1.wrench = projectairsim::Wrench({0., 2., 0.}, {0., 0., 0.});
  wrench_entries.emplace_back(&wrench_pt1, &refframe);

  wrench_pt2.position = {2., 0., 0.};
  wrench_pt2.wrench = projectairsim::Wrench({0., -1., 0.}, {0., 0., 0.});
  wrench_entries.emplace_back(&wrench_pt2, &refframe);

  body.SetExternalWrenchEntries(wrench_entries);
  body.CalculateExternalWrench();

  ext_wrench = body.GetExternalWrench();
  force_ans = {0., 1., 0.};
  EXPECT_EQ(ext_wrench.force, force_ans);

  torque_ans = {0., 0., 0.};
  EXPECT_EQ(ext_wrench.torque, torque_ans);

  // Additive force on Y axis, reaction torque on Z axis
  wrench_entries.clear();
  wrench_pt1.position = {1., 0., 0.};
  wrench_pt1.wrench = projectairsim::Wrench({0., 2., 0.}, {0., 0., 0.});
  wrench_entries.emplace_back(&wrench_pt1, &refframe);

  wrench_pt2.position = {2., 0., 0.};
  wrench_pt2.wrench = projectairsim::Wrench({0., 1., 0.}, {0., 0., 0.});
  wrench_entries.emplace_back(&wrench_pt2, &refframe);

  body.SetExternalWrenchEntries(wrench_entries);
  body.CalculateExternalWrench();

  ext_wrench = body.GetExternalWrench();
  force_ans = {0., 3., 0.};
  EXPECT_EQ(ext_wrench.force, force_ans);

  torque_ans = {0., 0., 4.};
  EXPECT_EQ(ext_wrench.torque, torque_ans);

  // Additive force on Y axis, additive torque on Z axis
  wrench_entries.clear();
  wrench_pt1.position = {1., 0., 0.};
  wrench_pt1.wrench = projectairsim::Wrench({0., 2., 0.}, {0., 0., 1.});
  wrench_entries.emplace_back(&wrench_pt1, &refframe);

  wrench_pt2.position = {2., 0., 0.};
  wrench_pt2.wrench = projectairsim::Wrench({0., 1., 0.}, {0., 0., 2.});
  wrench_entries.emplace_back(&wrench_pt2, &refframe);

  body.SetExternalWrenchEntries(wrench_entries);
  body.CalculateExternalWrench();

  ext_wrench = body.GetExternalWrench();
  force_ans = {0., 3., 0.};
  EXPECT_EQ(ext_wrench.force, force_ans);

  torque_ans = {0., 0., 7.};
  EXPECT_EQ(ext_wrench.torque, torque_ans);

  // Additive force on Y axis, additive torque on Z axis, external force on Z
  // axis
  std::vector<float> ext_force{0., 0., 10.};
  body.SetExternalForce(ext_force);
  body.CalculateExternalWrench();

  ext_wrench = body.GetExternalWrench();
  force_ans = {0., 3., 10.};
  EXPECT_EQ(ext_wrench.force, force_ans);

  torque_ans = {0., 0., 7.};
  EXPECT_EQ(ext_wrench.torque, torque_ans);
}

TEST(FastPhysicsBody, CalculateInertia) {
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();
  auto actors = scene.GetActors();
  EXPECT_EQ(actors.size(), 1);

  for (auto actor_ref : actors) {
    if (actor_ref.get().GetType() == projectairsim::ActorType::kRobot) {
      auto& sim_robot = dynamic_cast<projectairsim::Robot&>(actor_ref.get());
      projectairsim::FastPhysicsBody body(sim_robot);
      auto inertia = body.CalculateInertia(sim_robot.GetLinks());

      EXPECT_FLOAT_EQ(inertia(0, 0), 0.0152456462f);
      EXPECT_FLOAT_EQ(inertia(1, 1), 0.0169373136f);
      EXPECT_FLOAT_EQ(inertia(2, 2), 0.0318722911f);
      EXPECT_FLOAT_EQ(inertia(0, 1), 0.0);
      EXPECT_FLOAT_EQ(inertia(0, 2), 0.0);
      EXPECT_FLOAT_EQ(inertia(1, 0), 0.0);
      EXPECT_FLOAT_EQ(inertia(1, 2), 0.0);
      EXPECT_FLOAT_EQ(inertia(2, 0), 0.0);
      EXPECT_FLOAT_EQ(inertia(2, 1), 0.0);
    }
  }
}

TEST(FastPhysicsBody, InitializeDragFaces) {
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();
  auto actors = scene.GetActors();
  EXPECT_EQ(actors.size(), 1);

  for (auto actor_ref : actors) {
    if (actor_ref.get().GetType() == projectairsim::ActorType::kRobot) {
      auto& sim_robot = dynamic_cast<projectairsim::Robot&>(actor_ref.get());
      projectairsim::FastPhysicsBody body;
      auto drag_faces = body.InitializeDragFaces(sim_robot.GetLinks());

      // Initialized 6 drag faces (2x per axis)
      EXPECT_EQ(drag_faces.size(), 6);

      // Check values of top/bottom faces
      projectairsim::Vector3 pos_ans = {0.0f, 0.0f, -0.0199999996f};
      EXPECT_EQ(drag_faces[0].position, pos_ans);
      EXPECT_EQ(drag_faces[1].position, -pos_ans);

      projectairsim::Vector3 normal_ans = {0.0f, 0.0f, -1.0f};
      EXPECT_EQ(drag_faces[0].normal, normal_ans);
      EXPECT_EQ(drag_faces[1].normal, -normal_ans);

      EXPECT_FLOAT_EQ(drag_faces[0].area, 0.18397322f);
      EXPECT_FLOAT_EQ(drag_faces[1].area, 0.18397322f);

      EXPECT_FLOAT_EQ(drag_faces[0].drag_factor, 0.162499994f);
      EXPECT_FLOAT_EQ(drag_faces[1].drag_factor, 0.162499994f);

      // Check values of left/right faces
      pos_ans = {0.0f, -0.0549999997f, 0.0f};
      EXPECT_EQ(drag_faces[2].position, pos_ans);
      EXPECT_EQ(drag_faces[3].position, -pos_ans);

      normal_ans = {0.0f, -1.0f, 0.0f};
      EXPECT_EQ(drag_faces[2].normal, normal_ans);
      EXPECT_EQ(drag_faces[3].normal, -normal_ans);

      EXPECT_FLOAT_EQ(drag_faces[2].area, 0.0163439997f);
      EXPECT_FLOAT_EQ(drag_faces[3].area, 0.0163439997f);

      EXPECT_FLOAT_EQ(drag_faces[2].drag_factor, 0.162499994f);
      EXPECT_FLOAT_EQ(drag_faces[3].drag_factor, 0.162499994f);

      // Check values of back/front faces
      pos_ans = {-0.0900000035f, 0.0f, 0.0f};
      EXPECT_EQ(drag_faces[4].position, pos_ans);
      EXPECT_EQ(drag_faces[5].position, -pos_ans);

      normal_ans = {-1.0f, 0.0f, 0.0f};
      EXPECT_EQ(drag_faces[4].normal, normal_ans);
      EXPECT_EQ(drag_faces[5].normal, -normal_ans);

      EXPECT_FLOAT_EQ(drag_faces[4].area, 0.0135439998f);
      EXPECT_FLOAT_EQ(drag_faces[5].area, 0.0135439998f);

      EXPECT_FLOAT_EQ(drag_faces[4].drag_factor, 0.162499994f);
      EXPECT_FLOAT_EQ(drag_faces[5].drag_factor, 0.162499994f);
    }
  }
}

TEST(FastPhysicsBody, ReadWriteRobotData) {
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();
  auto actors = scene.GetActors();
  EXPECT_EQ(actors.size(), 1);

  for (auto actor_ref : actors) {
    if (actor_ref.get().GetType() == projectairsim::ActorType::kRobot) {
      auto& sim_robot = dynamic_cast<projectairsim::Robot&>(actor_ref.get());
      auto fp_body =
          std::make_shared<projectairsim::TestFastPhysicsBody>(sim_robot);

      EXPECT_FLOAT_EQ(fp_body->GetMass(), 1.0);

      projectairsim::Kinematics kin;
      fp_body->ReadRobotData();
      kin = fp_body->GetKinematics();

      EXPECT_FLOAT_EQ(kin.pose.position.x(), 0);
      EXPECT_FLOAT_EQ(kin.pose.position.y(), 0);
      EXPECT_FLOAT_EQ(kin.pose.position.z(), -4);

      EXPECT_FLOAT_EQ(fp_body->GetEnvGravity().z(),
                      projectairsim::EarthUtils::kGravity);

      kin.pose.position = {1., 2., 3.};
      fp_body->WriteRobotData(kin);

      auto kin_robot = sim_robot.GetKinematics();

      EXPECT_EQ(kin_robot.pose.position, kin.pose.position);
    }
  }
}

TEST(FastPhysicsBody, IsStillGrounded) {
  projectairsim::TestFastPhysicsBody body;
  body.SetMass(2.0);
  body.SetExternalWrench(projectairsim::Wrench({0., 0., 0.}, {0., 0., 0.}));
  body.SetEnvGravity({0, 0, projectairsim::EarthUtils::kGravity});
  body.SetIsGrounded(false);

  // Not grounded stays not grounded
  EXPECT_FALSE(body.IsStillGrounded());

  // Grounded with zero force is still grounded
  body.SetIsGrounded(true);
  EXPECT_TRUE(body.IsStillGrounded());

  // Force a bit less than weight is still grounded
  body.SetExternalWrench(projectairsim::Wrench({0., 0., -18.0}, {0., 0., 0.}));
  EXPECT_TRUE(body.IsStillGrounded());

  // Force a bit more than weight is not still grounded
  body.SetExternalWrench(projectairsim::Wrench({0., 0., -20.0}, {0., 0., 0.}));
  EXPECT_FALSE(body.IsStillGrounded());
}

TEST(FastPhysicsBody, IsLandingCollision) {
  projectairsim::TestFastPhysicsBody body;
  projectairsim::CollisionInfo col_info(false, {0., 0., 0.}, {0., 0., 0.},
                                        {0., 0., 0.}, 0., 0, "obj", 0);
  projectairsim::Kinematics kin;

  body.SetMass(2.0);
  body.SetExternalWrench(projectairsim::Wrench({0., 0., 0.}, {0., 0., 0.}));
  body.SetEnvGravity({0, 0, projectairsim::EarthUtils::kGravity});
  body.SetIsGrounded(false);
  body.SetCollisionInfo(col_info);
  body.SetKinematics(kin);

  // No collision case
  EXPECT_FALSE(body.IsLandingCollision());

  // Collision moving down in correct orientation
  col_info.has_collided = true;
  col_info.normal = {0., 0., -1.};
  body.SetCollisionInfo(col_info);
  kin.pose.orientation =
      projectairsim::TransformUtils::ToQuaternion(0., 0., 0.);
  kin.twist.linear = {0., 0., 0.1};
  body.SetKinematics(kin);
  EXPECT_TRUE(body.IsLandingCollision());
  EXPECT_TRUE(body.IsStillGrounded());

  // Collision moving down and sideways in correct orientation
  kin.twist.linear = {0., 1., 0.1};
  body.SetKinematics(kin);
  body.SetIsGrounded(false);
  EXPECT_FALSE(body.IsLandingCollision());
  EXPECT_FALSE(body.IsStillGrounded());

  // Collision moving down in off-axis orientation
  kin.twist.linear = {0., 0., 0.1};
  kin.pose.orientation =
      projectairsim::TransformUtils::ToQuaternion(M_PI / 4, 0., 0.);
  body.SetKinematics(kin);
  body.SetIsGrounded(false);
  EXPECT_FALSE(body.IsLandingCollision());
  EXPECT_FALSE(body.IsStillGrounded());

  // Collision moving down with off-axis collision normal
  kin.twist.linear = {0., 0., 0.1};
  kin.pose.orientation =
      projectairsim::TransformUtils::ToQuaternion(0., 0., 0.);
  body.SetKinematics(kin);
  col_info.normal = {0., 0.7071, -0.7071};
  body.SetCollisionInfo(col_info);
  body.SetIsGrounded(false);
  EXPECT_FALSE(body.IsLandingCollision());
  EXPECT_FALSE(body.IsStillGrounded());
}

TEST(FastPhysicsBody, NeedsCollisionResponse) {
  projectairsim::TestFastPhysicsBody body;
  projectairsim::CollisionInfo col_info(false, {0., 0., 0.}, {0., 0., 0.},
                                        {0., 0., 0.}, 0., 0, "obj", 0);
  projectairsim::Kinematics kin;
  kin.twist.linear = {0., 0., -1.0};

  // No collision case
  EXPECT_FALSE(body.NeedsCollisionResponse(kin));

  // Not moving into collision case
  col_info.has_collided = true;
  col_info.normal = {0., 0., -1.};
  body.SetCollisionInfo(col_info);
  EXPECT_FALSE(body.NeedsCollisionResponse(kin));

  // Moving into collision case
  kin.twist.linear = {0., 0., 1.0};
  EXPECT_TRUE(body.NeedsCollisionResponse(kin));
}

// -----------------------------------------------------------------------------
// class FastPhysicsModel

TEST(FastPhysicsModel, Constructor) {
  auto model = std::make_unique<projectairsim::FastPhysicsModel>();
  EXPECT_NE(model, nullptr);
}

TEST(FastPhysicsModel, CalcDragWrench) {
  projectairsim::FastPhysicsModel model;
  std::vector<projectairsim::DragFace> drag_faces;

  projectairsim::Vector3 face1_pos = {0., 0., 0.};
  projectairsim::Vector3 face1_normal = {0., 0., 1.};
  drag_faces.emplace_back(face1_pos, face1_normal, 2.0f, 0.5f);

  projectairsim::Vector3 face2_pos = {0., 0., 0.};
  projectairsim::Vector3 face2_normal = {0., 0., -1.};
  drag_faces.emplace_back(face2_pos, face2_normal, 3.0f, 0.5f);

  float air_density = 1.0;
  projectairsim::Vector3 wind_velocity = projectairsim::Vector3::Zero();

  projectairsim::Vector3 ans_force, ans_torque, ave_vel_lin, ave_vel_ang;
  projectairsim::Quaternion orientation;
  projectairsim::Wrench wrench;

  // Zero velocity case
  ave_vel_lin = {0., 0., 0.};
  ave_vel_ang = {0., 0., 0.};
  orientation = projectairsim::TransformUtils::ToQuaternion(0., 0., 0.);

  wrench = model.CalcDragFaceWrench(ave_vel_lin, ave_vel_ang, drag_faces,
                                    orientation, air_density, wind_velocity);

  ans_force = {0., 0., 0.};
  ans_torque = {0., 0., 0.};
  EXPECT_EQ(wrench.force, ans_force);
  EXPECT_EQ(wrench.torque, ans_torque);

  // Velocity aligned with drag face
  ave_vel_lin = {0., 0., 2.};

  wrench = model.CalcDragFaceWrench(ave_vel_lin, ave_vel_ang, drag_faces,
                                    orientation, air_density, wind_velocity);

  ans_force = {0., 0., -4.};
  ans_torque = {0., 0., 0.};
  EXPECT_EQ(wrench.force, ans_force);
  EXPECT_EQ(wrench.torque, ans_torque);

  ave_vel_lin = {0., 0., -2.};

  wrench = model.CalcDragFaceWrench(ave_vel_lin, ave_vel_ang, drag_faces,
                                    orientation, air_density, wind_velocity);

  ans_force = {0., 0., 6.};
  ans_torque = {0., 0., 0.};
  EXPECT_EQ(wrench.force, ans_force);
  EXPECT_EQ(wrench.torque, ans_torque);

  // Velocity perpendicular to drag face
  ave_vel_lin = {1., 1., 0.};

  wrench = model.CalcDragFaceWrench(ave_vel_lin, ave_vel_ang, drag_faces,
                                    orientation, air_density, wind_velocity);

  ans_force = {0., 0., 0.};
  ans_torque = {0., 0., 0.};
  EXPECT_EQ(wrench.force, ans_force);
  EXPECT_EQ(wrench.torque, ans_torque);

  // Orientation perpendicular to drag face
  ave_vel_lin = {0., 0., 1.};
  orientation = projectairsim::TransformUtils::ToQuaternion(0., M_PI / 2, 0.);

  wrench = model.CalcDragFaceWrench(ave_vel_lin, ave_vel_ang, drag_faces,
                                    orientation, air_density, wind_velocity);

  ans_force = {0., 0., 0.};
  ans_torque = {0., 0., 0.};
  EXPECT_EQ(wrench.force, ans_force);
  EXPECT_EQ(wrench.torque, ans_torque);

  // Make sure wind_velocity affects drag face
  orientation = projectairsim::TransformUtils::ToQuaternion(0., 0., 0.);
  wind_velocity = projectairsim::Vector3(5.0, 0.0, 0.0);

  auto wrench_nonzero_wind =
      model.CalcDragFaceWrench(ave_vel_lin, ave_vel_ang, drag_faces,
                               orientation, air_density, wind_velocity);

  wind_velocity = projectairsim::Vector3::Zero();
  auto wrench_zero_wind =
      model.CalcDragFaceWrench(ave_vel_lin, ave_vel_ang, drag_faces,
                               orientation, air_density, wind_velocity);

  EXPECT_NE(wrench_zero_wind.force, wrench_nonzero_wind.force);

  // Torque from drag face position offset
  projectairsim::Vector3 face3_pos = {1., 0., 0.};
  projectairsim::Vector3 face3_normal = {0., 0., 1.};
  drag_faces.emplace_back(face3_pos, face3_normal, 2.0f, 0.5f);

  ave_vel_lin = {0., 0., 2.};
  orientation = projectairsim::TransformUtils::ToQuaternion(0., 0., 0.);

  wrench = model.CalcDragFaceWrench(ave_vel_lin, ave_vel_ang, drag_faces,
                                    orientation, air_density, wind_velocity);

  ans_force = {0., 0., -8.};
  // Drag is only applied in the direction opposing the linear motion of the CG
  // so it does not apply rotational torque. For aerodynamic drag to cause
  // rotational torque, more complex modeling of the center of pressure would be
  // needed to apply the force away from the CG.
  ans_torque = {0., 0., 0.};
  EXPECT_EQ(wrench.force, ans_force);
  EXPECT_EQ(wrench.torque, ans_torque);
}

TEST(FastPhysicsModel, CalcNextKinematicsGrounded) {
  projectairsim::FastPhysicsModel model;
  projectairsim::Vector3 position;
  projectairsim::Quaternion orientation;
  projectairsim::Kinematics kin;

  // Pass-through position, zero velocities/accels
  position = {1.0, 2.0, 3.0};
  orientation =
      projectairsim::TransformUtils::ToQuaternion(M_PI / 4, M_PI / 4, M_PI / 4);
  kin = model.CalcNextKinematicsGrounded(position, orientation);

  EXPECT_FLOAT_EQ(kin.pose.position.x(), 1.0);
  EXPECT_FLOAT_EQ(kin.pose.position.y(), 2.0);
  EXPECT_FLOAT_EQ(kin.pose.position.z(), 3.0);

  auto ans_orientation =
      projectairsim::TransformUtils::ToQuaternion(0., 0., M_PI / 4);
  EXPECT_FLOAT_EQ(kin.pose.orientation.w(), ans_orientation.w());
  EXPECT_FLOAT_EQ(kin.pose.orientation.x(), ans_orientation.x());
  EXPECT_FLOAT_EQ(kin.pose.orientation.y(), ans_orientation.y());
  EXPECT_FLOAT_EQ(kin.pose.orientation.z(), ans_orientation.z());

  EXPECT_EQ(kin.twist.linear, projectairsim::Vector3::Zero());
  EXPECT_EQ(kin.twist.angular, projectairsim::Vector3::Zero());
  EXPECT_EQ(kin.accels.linear, projectairsim::Vector3::Zero());
  EXPECT_EQ(kin.accels.angular, projectairsim::Vector3::Zero());
}

TEST(FastPhysicsModel, CalcNextKinematicsNoCollision) {
  projectairsim::FastPhysicsModel model;
  auto body = std::make_shared<projectairsim::TestFastPhysicsBody>();
  auto fp_body = std::static_pointer_cast<projectairsim::FastPhysicsBody>(body);
  float test_mass = 2.0;
  body->SetMass(test_mass);
  body->SetInertia(projectairsim::Matrix3x3::Identity());
  body->SetExternalWrench(projectairsim::Wrench({0., 0., 0.}, {0., 0., 0.}));
  body->SetEnvGravity({0, 0, projectairsim::EarthUtils::kGravity});
  body->SetIsGrounded(false);

  projectairsim::Kinematics init_kin, kin;

  // Fall by gravity from zero accel
  init_kin.pose.position = {0., 0., -10.0};
  init_kin.pose.orientation =
      projectairsim::TransformUtils::ToQuaternion(0., 0., 0.);
  init_kin.twist.linear = {0., 0., 0.};
  init_kin.twist.angular = {0., 0., 0.};
  init_kin.accels.linear = {0., 0., 0.};
  init_kin.accels.angular = {0., 0., 0.};
  body->SetKinematics(init_kin);

  kin = model.CalcNextKinematicsNoCollision(1.0f, fp_body);

  // Note: Current algorithm calculates pose from prev velocities/accels
  EXPECT_FLOAT_EQ(kin.pose.position.z(), -10.0);
  EXPECT_FLOAT_EQ(kin.twist.linear.z(), 4.9033251);
  EXPECT_FLOAT_EQ(kin.accels.linear.z(), projectairsim::EarthUtils::kGravity);

  // Step again to see pose update from prev velocities/accels
  body->SetKinematics(kin);
  kin = model.CalcNextKinematicsNoCollision(1.0f, fp_body);

  EXPECT_FLOAT_EQ(kin.pose.position.z(), -0.19334984);
  EXPECT_FLOAT_EQ(kin.twist.linear.z(), 14.709975);
  EXPECT_FLOAT_EQ(kin.accels.linear.z(), projectairsim::EarthUtils::kGravity);

  // Hover by balancing wrench with gravity
  body->SetKinematics(init_kin);
  auto balance_force = test_mass * projectairsim::EarthUtils::kGravity;
  body->SetExternalWrench(
      projectairsim::Wrench({0., 0., -balance_force}, {0., 0., 0.}));

  kin = model.CalcNextKinematicsNoCollision(1.0f, fp_body);

  EXPECT_FLOAT_EQ(kin.pose.position.z(), -10.0);
  EXPECT_FLOAT_EQ(kin.twist.linear.z(), 0.0);
  EXPECT_FLOAT_EQ(kin.accels.linear.z(), 0.0);

  // Hover and move sideways
  body->SetKinematics(init_kin);
  body->SetExternalWrench(
      projectairsim::Wrench({10., 0., -balance_force}, {0., 0., 0.}));

  kin = model.CalcNextKinematicsNoCollision(1.0f, fp_body);

  // Note: Current algorithm calculates pose from prev velocities/accels
  EXPECT_FLOAT_EQ(kin.pose.position.x(), 0.0);
  EXPECT_FLOAT_EQ(kin.pose.position.y(), 0.0);
  EXPECT_FLOAT_EQ(kin.pose.position.z(), -10.0);
  EXPECT_FLOAT_EQ(kin.twist.linear.x(), 2.5);
  EXPECT_FLOAT_EQ(kin.twist.linear.y(), 0.0);
  EXPECT_FLOAT_EQ(kin.twist.linear.z(), 0.0);
  EXPECT_FLOAT_EQ(kin.accels.linear.z(), 0.0);

  // Step again to see pose update from prev velocities/accels
  body->SetKinematics(kin);
  kin = model.CalcNextKinematicsNoCollision(1.0f, fp_body);

  EXPECT_FLOAT_EQ(kin.pose.position.x(), 5.0);
  EXPECT_FLOAT_EQ(kin.pose.position.y(), 0.0);
  EXPECT_FLOAT_EQ(kin.pose.position.z(), -10.0);
  EXPECT_FLOAT_EQ(kin.twist.linear.x(), 7.5);
  EXPECT_FLOAT_EQ(kin.twist.linear.y(), 0.0);
  EXPECT_FLOAT_EQ(kin.twist.linear.z(), 0.0);
  EXPECT_FLOAT_EQ(kin.accels.linear.z(), 0.0);

  // TODO Add more detailed tests for drag, angular components, etc.
}

TEST(FastPhysicsModel, CalcNextKinematicsWithCollision) {
  projectairsim::FastPhysicsModel model;
  auto body = std::make_shared<projectairsim::TestFastPhysicsBody>();
  auto fp_body = std::static_pointer_cast<projectairsim::FastPhysicsBody>(body);
  float test_mass = 2.0;
  body->SetMass(test_mass);
  body->SetInertia(projectairsim::Matrix3x3::Identity());
  body->SetRestitution(0.5);
  body->SetFriction(0.5);

  projectairsim::Kinematics init_kin, kin;
  projectairsim::CollisionInfo col_info;

  // Vertical collision aligned, zero velocity -> set pos to col_info.pos
  init_kin.pose.position = {0., 0., -9.0};
  init_kin.pose.orientation =
      projectairsim::TransformUtils::ToQuaternion(0., 0., 0.);
  init_kin.twist.linear = {0., 0., 0.};
  init_kin.twist.angular = {0., 0., 0.};
  init_kin.accels.linear = {0., 0., 0.};
  init_kin.accels.angular = {0., 0., 0.};
  body->SetKinematics(init_kin);

  col_info.normal = {0., 0., -1.};
  col_info.position = {0., 0., -10.0};
  col_info.impact_point = {0., 0., -9.0};
  col_info.penetration_depth = 0.;
  body->SetCollisionInfo(col_info);

  kin = model.CalcNextKinematicsWithCollision(1.0f, fp_body);

  EXPECT_EQ(kin.accels.linear, projectairsim::Vector3::Zero());
  EXPECT_EQ(kin.accels.angular, projectairsim::Vector3::Zero());

  EXPECT_FLOAT_EQ(kin.twist.linear.x(), 0.);
  EXPECT_FLOAT_EQ(kin.twist.linear.y(), 0.);
  EXPECT_FLOAT_EQ(kin.twist.linear.z(), 0.);

  EXPECT_EQ(kin.twist.angular, projectairsim::Vector3::Zero());

  EXPECT_FLOAT_EQ(kin.pose.position.x(), 0.);
  EXPECT_FLOAT_EQ(kin.pose.position.y(), 0.);
  EXPECT_FLOAT_EQ(kin.pose.position.z(), -10.);

  // Vertical collision aligned, with velocity -> reflect restitution to vel
  init_kin.pose.position = {0., 0., -9.0};
  init_kin.pose.orientation =
      projectairsim::TransformUtils::ToQuaternion(0., 0., 0.);
  init_kin.twist.linear = {0., 0., 2.};
  init_kin.twist.angular = {0., 0., 0.};
  init_kin.accels.linear = {0., 0., 0.};
  init_kin.accels.angular = {0., 0., 0.};
  body->SetKinematics(init_kin);

  col_info.normal = {0., 0., -1.};
  col_info.position = {0., 0., -10.0};
  col_info.impact_point = {0., 0., -9.0};
  col_info.penetration_depth = 0.;
  body->SetCollisionInfo(col_info);

  kin = model.CalcNextKinematicsWithCollision(1.0f, fp_body);

  EXPECT_EQ(kin.accels.linear, projectairsim::Vector3::Zero());
  EXPECT_EQ(kin.accels.angular, projectairsim::Vector3::Zero());

  EXPECT_FLOAT_EQ(kin.twist.linear.x(), 0.);
  EXPECT_FLOAT_EQ(kin.twist.linear.y(), 0.);
  EXPECT_LT(kin.twist.linear.z(), 2.);  // should be reversing speed

  EXPECT_EQ(kin.twist.angular, projectairsim::Vector3::Zero());

  EXPECT_FLOAT_EQ(kin.pose.position.x(), 0.);
  EXPECT_FLOAT_EQ(kin.pose.position.y(), 0.);
  EXPECT_LT(kin.pose.position.z(), -10.);  // should bounce up

  // Tangential collision, with velocity -> reflect friction to vel
  init_kin.pose.position = {0., 0., -9.0};
  init_kin.pose.orientation =
      projectairsim::TransformUtils::ToQuaternion(0., 0., 0.);
  init_kin.twist.linear = {2., 0., 0.};
  init_kin.twist.angular = {0., 0., 0.};
  init_kin.accels.linear = {0., 0., 0.};
  init_kin.accels.angular = {0., 0., 0.};
  body->SetKinematics(init_kin);

  col_info.normal = {0., 0., -1.};
  col_info.position = {0., 0., -10.0};
  col_info.impact_point = {0., 0., -9.0};
  col_info.penetration_depth = 0.;
  body->SetCollisionInfo(col_info);

  kin = model.CalcNextKinematicsWithCollision(1.0f, fp_body);

  EXPECT_EQ(kin.accels.linear, projectairsim::Vector3::Zero());
  EXPECT_EQ(kin.accels.angular, projectairsim::Vector3::Zero());

  EXPECT_LT(kin.twist.linear.x(), 2.);  // friction causes slowdown
  EXPECT_FLOAT_EQ(kin.twist.linear.y(), 0.);
  EXPECT_FLOAT_EQ(kin.twist.linear.z(), 0.);

  EXPECT_FLOAT_EQ(kin.twist.angular.x(), 0.);
  EXPECT_LT(kin.twist.angular.y(), 0.);  // friction causes pitch
  EXPECT_FLOAT_EQ(kin.twist.angular.z(), 0.);

  EXPECT_GT(kin.pose.position.x(), 0.);  // horizontal motion continues
  EXPECT_FLOAT_EQ(kin.pose.position.y(), 0.);
  EXPECT_FLOAT_EQ(kin.pose.position.z(), -10.);  // collision sets z pos
}

TEST(FastPhysicsModel, StepPhysicsBody) {
  projectairsim::Simulator simulator;
  simulator.LoadSceneWithJSON(physics_test_config);
  auto& scene = simulator.GetScene();
  auto actors = scene.GetActors();
  EXPECT_EQ(actors.size(), 1);

  for (auto actor_ref : actors) {
    if (actor_ref.get().GetType() == projectairsim::ActorType::kRobot) {
      auto& sim_robot = dynamic_cast<projectairsim::Robot&>(actor_ref.get());
      auto fp_body =
          std::make_shared<projectairsim::FastPhysicsBody>(sim_robot);
      projectairsim::FastPhysicsModel model;
      projectairsim::Kinematics kin_body, kin_robot;
      float vel_linear_z_robot;

      model.StepPhysicsBody(1.0, fp_body);
      kin_body = fp_body->GetKinematics();
      kin_robot = sim_robot.GetKinematics();
      vel_linear_z_robot = kin_robot.twist.linear.z();

      // On first step, fp_body kinematics gets set to robot's initial kin
      EXPECT_FLOAT_EQ(kin_body.pose.position.x(), 0);
      EXPECT_FLOAT_EQ(kin_body.pose.position.y(), 0);
      EXPECT_FLOAT_EQ(kin_body.pose.position.z(), -4);
      EXPECT_FLOAT_EQ(kin_body.twist.linear.z(), 0.0);

      // On first step, robot kinematics gets a new linear z vel from gravity
      EXPECT_FLOAT_EQ(kin_robot.pose.position.x(), 0);
      EXPECT_FLOAT_EQ(kin_robot.pose.position.y(), 0);
      EXPECT_FLOAT_EQ(kin_robot.pose.position.z(), -4);
      EXPECT_NE(vel_linear_z_robot, 0.0);

      model.StepPhysicsBody(1.0, fp_body);
      kin_body = fp_body->GetKinematics();
      kin_robot = sim_robot.GetKinematics();

      // On second step, fp_body kinematics gets set to robot's prev linear z
      // vel, and robot gets another new linear z vel from gravity
      EXPECT_FLOAT_EQ(kin_body.twist.linear.z(), vel_linear_z_robot);
      EXPECT_NE(kin_robot.twist.linear.z(), vel_linear_z_robot);
    }
  }
}

// TODO TEST(FastPhysicsModel, SetWrenchesOnPhysicsBody) {}