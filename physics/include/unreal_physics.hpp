// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef PHYSICS_INCLUDE_UNREAL_PHYSICS_HPP_
#define PHYSICS_INCLUDE_UNREAL_PHYSICS_HPP_

#include <vector>

#include "base_physics.hpp"
#include "core_sim/actor/robot.hpp"
#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class UnrealPhysicsBody

class UnrealPhysicsBody : public BasePhysicsBody {
 public:
  UnrealPhysicsBody() {}
  explicit UnrealPhysicsBody(const Robot& robot);
  ~UnrealPhysicsBody() override {}

  void InitializeUnrealPhysicsBody();

  // Aggregate all externally applied wrenches on body CG into wrench_
  void CalculateExternalWrench() override;

  void WriteRobotData(const Kinematics& kinematics,
                      TimeNano external_time_stamp = -1);

  void SetCallbackSetExternalWrench(
      const std::function<void(const Wrench&)>& callback);

 protected:
  Robot sim_robot_;
  std::vector<const WrenchPoint*> external_wrench_points_;
  std::function<void(const Wrench&)> set_external_wrench_callback_;
};

// -----------------------------------------------------------------------------
// class UnrealPhysicsModel

class UnrealPhysicsModel {
 public:
  UnrealPhysicsModel() {}
  ~UnrealPhysicsModel() {}

  void SetWrenchesOnPhysicsBody(std::shared_ptr<BasePhysicsBody> body);
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // PHYSICS_INCLUDE_UNREAL_PHYSICS_HPP_
