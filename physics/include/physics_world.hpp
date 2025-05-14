// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef PHYSICS_INCLUDE_PHYSICS_WORLD_HPP_
#define PHYSICS_INCLUDE_PHYSICS_WORLD_HPP_

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include "core_sim/actor/robot.hpp"
#include "core_sim/scene.hpp"
#include "fast_physics.hpp"
#include "matlab_physics.hpp"
#include "unreal_physics.hpp"

namespace microsoft {
namespace projectairsim {

// Use physics model classes as std::variant so that they can have unique
// methods and not force a common base class interface. If a common interface
// is decided that could work for all physics types, this could be changed to
// use standard base-class inheritance instead of variants.
using ModelVariant =
    std::variant<FastPhysicsModel, UnrealPhysicsModel, MatlabPhysicsModel>;

class PhysicsWorld {
 public:
  PhysicsWorld() {}
  ~PhysicsWorld() {}

  void AddRobot(const Robot& robot);

  void AddPhysicsBody(std::shared_ptr<BasePhysicsBody> physics_body);

  const std::vector<BasePhysicsBody*>& GetPhysicsBodies() const;

  void RemoveAllBodiesAndModels();

  void SetSceneTickCallbacks(Scene& scene);

  void Start();

  void SetWrenchesOnPhysicsBodies();

  void StepPhysicsWorld(TimeNano dt_nanos);

  void Stop();

 protected:
  std::vector<std::shared_ptr<BasePhysicsBody>> physics_bodies_;
  std::vector<BasePhysicsBody*> physics_bodies_ref_;
  std::unordered_map<PhysicsType, ModelVariant> physics_models_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // PHYSICS_INCLUDE_PHYSICS_WORLD_HPP_
