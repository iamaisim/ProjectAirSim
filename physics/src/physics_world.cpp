// Copyright (C) Microsoft Corporation. All rights reserved.

#include "physics_world.hpp"

#include "core_sim/actor/robot.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/scene.hpp"
#include "fast_physics.hpp"
#include "unreal_physics.hpp"

namespace microsoft {
namespace projectairsim {

void PhysicsWorld::AddRobot(const Robot& robot) {
  // TODO Handle case of blank robot object with nullptr to impl

  // Note: The model variants need to be constructed  in place to avoid move
  // semantics to allow for atomic/mutex data members).
  if (robot.GetPhysicsType() == PhysicsType::kFastPhysics) {
    // Construct physics body passing params from robot JSON physics params
    auto fast_physics_body = std::make_shared<FastPhysicsBody>(robot);

    // Add physics body pointer to physics world
    AddPhysicsBody(fast_physics_body);

    // Add FastPhysics model to the world's models if not already added
    if (physics_models_.count(PhysicsType::kFastPhysics) == 0) {
      physics_models_.emplace(PhysicsType::kFastPhysics,
                              std::in_place_type<FastPhysicsModel>);
    }
  } else if (robot.GetPhysicsType() == PhysicsType::kMatlabPhysics) {
    // Construct physics body passing params from robot JSON physics params
    auto matlab_physics_body = std::make_shared<MatlabPhysicsBody>(robot);

    // Add physics body pointer to physics world
    AddPhysicsBody(matlab_physics_body);

    // Add MatlabPhysics model to the world's models if not already added
    if (physics_models_.count(PhysicsType::kMatlabPhysics) == 0) {
      physics_models_.emplace(PhysicsType::kMatlabPhysics,
                              std::in_place_type<MatlabPhysicsModel>);
    }
  } else if (robot.GetPhysicsType() == PhysicsType::kUnrealPhysics) {
    // Make an UnrealPhysicsBody to aggregate actuator outputs and pass wrench
    // to UnrealRobot to be applied at Unreal's next physics step
    auto unreal_physics_body = std::make_shared<UnrealPhysicsBody>(robot);

    // Add physics body pointer to physics world
    AddPhysicsBody(unreal_physics_body);

    // Add UnrealPhysics model to the world's models if not already added.
    if (physics_models_.count(PhysicsType::kUnrealPhysics) == 0) {
      physics_models_.emplace(PhysicsType::kUnrealPhysics,
                              std::in_place_type<UnrealPhysicsModel>);
    }
  }
}

void PhysicsWorld::AddPhysicsBody(
    std::shared_ptr<BasePhysicsBody> physics_body) {
  physics_bodies_.push_back(physics_body);
  physics_bodies_ref_.push_back(physics_body.get());
}

const std::vector<BasePhysicsBody*>& PhysicsWorld::GetPhysicsBodies() const {
  return physics_bodies_ref_;
}

void PhysicsWorld::RemoveAllBodiesAndModels() {
  physics_bodies_.clear();
  physics_bodies_ref_.clear();
  physics_models_.clear();
}

void PhysicsWorld::SetSceneTickCallbacks(Scene& scene) {
  // Link callback functions for scene tick to call physics world functions
  scene.SetCallbackPhysicsStart([this]() { this->Start(); });

  scene.SetCallbackPhysicsSetWrenches(
      [this]() { this->SetWrenchesOnPhysicsBodies(); });

  scene.SetCallbackPhysicsStep(
      [this](TimeNano dt_nanos) { this->StepPhysicsWorld(dt_nanos); });

  scene.SetCallbackPhysicsStop([this]() { this->Stop(); });
}

void PhysicsWorld::Start() {
  for (auto body : physics_bodies_) {
    switch (body->GetPhysicsType()) {
      case PhysicsType::kFastPhysics: {
        break;
      }
      case PhysicsType::kMatlabPhysics: {
        // Matlab physics needs to start with sim scene to handle starting the
        // NNG communication loops.
        std::shared_ptr<MatlabPhysicsBody> matlab_body =
            std::dynamic_pointer_cast<MatlabPhysicsBody>(body);
        if (matlab_body) {
          auto& matlab_model = std::get<MatlabPhysicsModel>(
              physics_models_.at(PhysicsType::kMatlabPhysics));

          matlab_model.Start(matlab_body);
        }
        break;
      }
      case PhysicsType::kUnrealPhysics: {
        break;
      }
      default: {
        break;
      }
    }
  }
}

void PhysicsWorld::SetWrenchesOnPhysicsBodies() {
  for (auto body : physics_bodies_) {
    switch (body->GetPhysicsType()) {
      case PhysicsType::kFastPhysics: {
        auto& fp_model = std::get<FastPhysicsModel>(
            physics_models_.at(PhysicsType::kFastPhysics));
        fp_model.SetWrenchesOnPhysicsBody(body);
        break;
      }
      case PhysicsType::kMatlabPhysics: {
        auto& matlab_model = std::get<MatlabPhysicsModel>(
            physics_models_.at(PhysicsType::kMatlabPhysics));
        matlab_model.SetWrenchesOnPhysicsBody(body);
        break;
      }
      case PhysicsType::kUnrealPhysics: {
        auto& up_model = std::get<UnrealPhysicsModel>(
            physics_models_.at(PhysicsType::kUnrealPhysics));
        up_model.SetWrenchesOnPhysicsBody(body);
        break;
      }
      default: {
        break;
      }
    }
  }
}

void PhysicsWorld::StepPhysicsWorld(TimeNano dt_nanos) {
  for (auto body : physics_bodies_) {
    switch (body->GetPhysicsType()) {
      case PhysicsType::kFastPhysics: {
        auto& fp_model = std::get<FastPhysicsModel>(
            physics_models_.at(PhysicsType::kFastPhysics));
        fp_model.StepPhysicsBody(dt_nanos, body);
        break;
      }
      case PhysicsType::kMatlabPhysics: {
        auto& matlab_model = std::get<MatlabPhysicsModel>(
            physics_models_.at(PhysicsType::kMatlabPhysics));
        matlab_model.StepPhysicsBody(dt_nanos, body);
        break;
      }
      case PhysicsType::kUnrealPhysics: {
        // No-op, Unreal steps physics body directly
        break;
      }
      default: {
        break;
      }
    }
  }
}

void PhysicsWorld::Stop() {
  for (auto body : physics_bodies_) {
    switch (body->GetPhysicsType()) {
      case PhysicsType::kFastPhysics: {
        break;
      }
      case PhysicsType::kMatlabPhysics: {
        // Matlab physics needs to stop with sim scene to handle stopping the
        // NNG communication loops.
        std::shared_ptr<MatlabPhysicsBody> matlab_body =
            std::dynamic_pointer_cast<MatlabPhysicsBody>(body);
        if (matlab_body) {
          auto& matlab_model = std::get<MatlabPhysicsModel>(
              physics_models_.at(PhysicsType::kMatlabPhysics));

          matlab_model.Stop(matlab_body);
        }
        break;
      }
      case PhysicsType::kUnrealPhysics: {
        break;
      }
      default: {
        break;
      }
    }
  }
}

}  // namespace projectairsim
}  // namespace microsoft
