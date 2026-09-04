// Copyright (C) Microsoft Corporation.
// Copyright (C) 2025-2026 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#ifndef SAMPLES_PROJECTAIRSIM_RUNTIME_GROUND_COLLISION_HOST_HPP_
#define SAMPLES_PROJECTAIRSIM_RUNTIME_GROUND_COLLISION_HOST_HPP_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "core_sim/actor/robot.hpp"
#include "core_sim/scene.hpp"

namespace microsoft {
namespace projectairsim {

// Lightweight host-side collision integration for Project AirSim Runtime. It
// mirrors the Unreal host's kinematics callback -> collision sweep ->
// CollisionInfo flow, but only against a flat ground plane.
class GroundCollisionHost {
 public:
  struct Settings {
    // Ground plane altitude in local NED coordinates (positive Z is down).
    float ground_height_m = 0.0f;

    // Optional distance from the robot origin to an additional landing-gear
    // contact point along the robot's local +Z axis.
    float ground_clearance_m = 0.0f;
  };

  explicit GroundCollisionHost(const Settings& settings);
  ~GroundCollisionHost();

  void LoadScene(Scene& scene);
  void UnloadScene();

 private:
  struct ContactProbe {
    std::string name;
    Vector3 local_center = Vector3::Zero();
    Quaternion local_orientation = Quaternion::Identity();
    Vector3 half_extents = Vector3::Zero();
    float radius_m = 0.0f;
  };

  struct RobotCollisionModel {
    Robot* robot = nullptr;
    std::vector<ContactProbe> probes;
  };

  struct PendingKinematics {
    Kinematics kinematics;
    TimeNano time_stamp = 0;
  };

  std::unique_ptr<RobotCollisionModel> BuildRobotModel(
      Robot& robot, TransformTree& transform_tree) const;
  void QueueKinematics(RobotCollisionModel& model, const Kinematics& kinematics,
                       TimeNano time_stamp);
  void StartWorker();
  void StopWorker();
  void WorkerLoop();
  void UpdateCollision(RobotCollisionModel& model, const Kinematics& kinematics,
                       TimeNano time_stamp) const;

  Settings settings_;
  std::vector<std::unique_ptr<RobotCollisionModel>> robot_models_;
  std::mutex pending_mutex_;
  std::condition_variable pending_condition_;
  std::unordered_map<RobotCollisionModel*, PendingKinematics> pending_updates_;
  std::thread worker_;
  bool stop_worker_ = false;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // SAMPLES_PROJECTAIRSIM_RUNTIME_GROUND_COLLISION_HOST_HPP_
