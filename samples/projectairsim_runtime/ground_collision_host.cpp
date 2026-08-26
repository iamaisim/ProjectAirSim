// Copyright (C) Microsoft Corporation.
// Copyright (C) 2025-2026 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include "ground_collision_host.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <utility>

#include "core_sim/actuators/wheel.hpp"

namespace microsoft {
namespace projectairsim {

GroundCollisionHost::GroundCollisionHost(const Settings& settings)
    : settings_(settings) {}

GroundCollisionHost::~GroundCollisionHost() { UnloadScene(); }

void GroundCollisionHost::LoadScene(Scene& scene) {
  UnloadScene();

  auto actors = scene.GetActors();
  robot_models_.reserve(actors.size());

  for (auto& actor_ref : actors) {
    Actor& actor = actor_ref.get();
    if (actor.GetType() != ActorType::kRobot) continue;

    auto& robot = static_cast<Robot&>(actor);
    if (robot.GetPhysicsType() != PhysicsType::kFastPhysics) {
      if (robot.GetPhysicsType() == PhysicsType::kUnrealPhysics) {
        std::cout << "[Project AirSim Runtime] Robot '" << robot.GetID()
                  << "' requests unreal-physics, which needs Unreal Engine; "
                     "use fast-physics for lightweight collision response."
                  << std::endl;
      }
      continue;
    }

    robot_models_.push_back(BuildRobotModel(robot, *scene.GetTransformTree()));
  }

  StartWorker();

  // Like Unreal, the kinematics callback only copies the new state. Collision
  // evaluation happens outside Robot's update mutex in the host worker.
  for (auto& model : robot_models_) {
    auto* model_ptr = model.get();
    model->robot->SetCallbackKinematicsUpdated(
        [this, model_ptr](const Kinematics& kinematics, TimeNano time_stamp) {
          QueueKinematics(*model_ptr, kinematics, time_stamp);
        });

    std::cout << "[Project AirSim Runtime] Ground collision enabled for robot '"
              << model->robot->GetID() << "' with " << model->probes.size()
              << " contact probe(s)." << std::endl;
  }
}

void GroundCollisionHost::UnloadScene() {
  for (auto& model : robot_models_) {
    if (model->robot != nullptr) {
      model->robot->SetCallbackKinematicsUpdated(Robot::KinematicsCallback());
    }
  }
  StopWorker();
  robot_models_.clear();
}

std::unique_ptr<GroundCollisionHost::RobotCollisionModel>
GroundCollisionHost::BuildRobotModel(Robot& robot,
                                     TransformTree& transform_tree) const {
  auto model = std::make_unique<RobotCollisionModel>();
  model->robot = &robot;

  const auto& links = robot.GetLinks();
  if (!links.empty() && links.front().GetCollision().IsCollisionEnabled()) {
    const Link& root_link = links.front();
    ContactProbe body_probe;
    body_probe.name = root_link.GetID();
    body_probe.local_center = root_link.GetInertial().GetOrigin().translation_;
    body_probe.local_orientation =
        root_link.GetInertial().GetOrigin().rotation_;
    body_probe.half_extents = root_link.GetInertial().GetBodyBox() * 0.5f;
    model->probes.push_back(body_probe);
  }

  // Unreal explicitly checks secondary wheel links against the ground. Build
  // equivalent spherical contact probes from wheel centers and radii.
  for (const Wheel* wheel : robot.GetWheels()) {
    if (wheel == nullptr) continue;

    Link* wheel_link = robot.GetLink(wheel->GetChildLink());
    if (wheel_link == nullptr ||
        !wheel_link->IsGroundCollisionDetectionEnabled()) {
      continue;
    }

    Pose wheel_pose_robot;
    const bool converted = transform_tree.Convert(
        Pose(), wheel_link->GetVisualRefFrame(), robot, &wheel_pose_robot);
    if (!converted) {
      std::cout << "[Project AirSim Runtime] Unable to resolve wheel link '"
                << wheel_link->GetID() << "' for robot '" << robot.GetID()
                << "'." << std::endl;
      continue;
    }

    ContactProbe wheel_probe;
    wheel_probe.name = wheel_link->GetID();
    wheel_probe.local_center = wheel_pose_robot.position;
    wheel_probe.radius_m = wheel->GetRadius();
    model->probes.push_back(wheel_probe);
  }

  if (settings_.ground_clearance_m > 0.0f) {
    ContactProbe clearance_probe;
    clearance_probe.name = "configured-landing-gear";
    clearance_probe.local_center =
        Vector3(0.0f, 0.0f, settings_.ground_clearance_m);
    model->probes.push_back(clearance_probe);
  }

  return model;
}

void GroundCollisionHost::QueueKinematics(RobotCollisionModel& model,
                                          const Kinematics& kinematics,
                                          TimeNano time_stamp) {
  {
    std::lock_guard<std::mutex> lock(pending_mutex_);
    if (stop_worker_) return;
    pending_updates_[&model] = PendingKinematics{kinematics, time_stamp};
  }
  pending_condition_.notify_one();
}

void GroundCollisionHost::StartWorker() {
  if (worker_.joinable()) return;
  {
    std::lock_guard<std::mutex> lock(pending_mutex_);
    stop_worker_ = false;
    pending_updates_.clear();
  }
  worker_ = std::thread(&GroundCollisionHost::WorkerLoop, this);
}

void GroundCollisionHost::StopWorker() {
  {
    std::lock_guard<std::mutex> lock(pending_mutex_);
    stop_worker_ = true;
    pending_updates_.clear();
  }
  pending_condition_.notify_all();
  if (worker_.joinable()) worker_.join();
}

void GroundCollisionHost::WorkerLoop() {
  while (true) {
    std::unordered_map<RobotCollisionModel*, PendingKinematics> updates;
    {
      std::unique_lock<std::mutex> lock(pending_mutex_);
      pending_condition_.wait(
          lock, [this] { return stop_worker_ || !pending_updates_.empty(); });
      if (stop_worker_) return;
      updates.swap(pending_updates_);
    }

    for (const auto& [model, update] : updates) {
      UpdateCollision(*model, update.kinematics, update.time_stamp);
    }
  }
}

void GroundCollisionHost::UpdateCollision(RobotCollisionModel& model,
                                          const Kinematics& kinematics,
                                          TimeNano time_stamp) const {
  Robot& robot = *model.robot;

  // Unreal clears the previous collision before each swept pose update.
  robot.SetHasCollided(false);
  if (model.probes.empty()) return;

  float deepest_contact_z = -std::numeric_limits<float>::infinity();
  Vector3 deepest_contact = Vector3::Zero();

  for (const auto& probe : model.probes) {
    const Vector3 world_center =
        kinematics.pose.position +
        kinematics.pose.orientation * probe.local_center;
    const Quaternion world_orientation =
        kinematics.pose.orientation * probe.local_orientation;
    const Matrix3x3 rotation = world_orientation.toRotationMatrix();

    // Projection of an oriented box onto the global NED Z axis. Wheels are
    // represented as spheres, so their support distance is their radius.
    const float box_support_z =
        std::abs(rotation(2, 0)) * probe.half_extents.x() +
        std::abs(rotation(2, 1)) * probe.half_extents.y() +
        std::abs(rotation(2, 2)) * probe.half_extents.z();
    const float support_z = std::max(box_support_z, probe.radius_m);

    Vector3 contact = world_center;
    contact.z() += support_z;
    if (contact.z() > deepest_contact_z) {
      deepest_contact_z = contact.z();
      deepest_contact = contact;
    }
  }

  if (deepest_contact_z < settings_.ground_height_m) return;

  CollisionInfo collision;
  collision.has_collided = true;
  collision.normal = Vector3(0.0f, 0.0f, -1.0f);
  collision.impact_point = deepest_contact;
  collision.impact_point.z() = settings_.ground_height_m;
  collision.position = kinematics.pose.position;
  collision.penetration_depth = deepest_contact_z - settings_.ground_height_m;
  collision.time_stamp = time_stamp;
  collision.object_name = "IAMAI_Ground";
  collision.segmentation_id = -1;
  robot.UpdateCollisionInfo(collision);
}

}  // namespace projectairsim
}  // namespace microsoft
