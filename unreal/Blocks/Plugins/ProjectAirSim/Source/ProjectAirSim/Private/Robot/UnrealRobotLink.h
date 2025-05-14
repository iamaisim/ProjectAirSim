// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.
// The Unreal robot link declaration.

#pragma once

#include <functional>

#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "core_sim/link.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "UnrealRobotLink.generated.h"

namespace microsoft {
namespace projectairsim {
class UnrealMesh;
}  // namespace projectairsim
}  // namespace microsoft

UCLASS()
class UUnrealRobotLink : public UStaticMeshComponent {
  GENERATED_BODY()

 public:
  // Transform from an actuator
  struct ActuatedFTransform {
    typedef microsoft::projectairsim::ActuatedTransform::ApplyOrder ApplyOrder;

    ApplyOrder applyorder;  // Where to apply the transform
    FTransform ftransform;  // Transform to appy

    ActuatedFTransform(const ActuatedFTransform& actuatedftransform)
        : applyorder(actuatedftransform.applyorder),
          ftransform(actuatedftransform.ftransform) {}
    ActuatedFTransform(
        const microsoft::projectairsim::ActuatedTransform& actuatedtransform);
  };  // struct ActuatedTransform

  // Collision hit callback function prototype
  typedef std::function<void(UPrimitiveComponent* HitComponent,
                             AActor* OtherActor, UPrimitiveComponent* OtherComp,
                             FVector NormalImpulse, const FHitResult& Hit)>
      FnCollisionHitCallback;

 public:
  explicit UUnrealRobotLink(const FObjectInitializer& ObjectInitializer);

  void Initialize(const microsoft::projectairsim::Link& Link,
                  bool with_unreal_physics = false);

  void TickComponent(float DeltaTime, ELevelTick TickType,
                     FActorComponentTickFunction* ThisTickFunction) override;

  bool IsGroundCollisionDetectionEnabled(void) const;
  bool IsLinkCollisionEnabled() const;

  void SetActuatedFTransform(const ActuatedFTransform& actuatedftransform);
  void SetCollisionHitCallback(FnCollisionHitCallback CollisionHitCallback);

  UFUNCTION()
  void OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                      UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                      const FHitResult& Hit);

 protected:
  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

 private:
  void InitializeMesh(const microsoft::projectairsim::UnrealMesh* Mesh);

  void InitializePose(const microsoft::projectairsim::Transform& Pose,
                      const FVector& Scale_Initial);

  void InitializePhysics(
      const microsoft::projectairsim::Inertial& LinkInertial,
      const microsoft::projectairsim::Collision& LinkCollision);

  void CheckGroundCollision(void);

  UPhysicalMaterial* custom_material;

  microsoft::projectairsim::Link link;

  FTransform Transform_Initial_Translation;  // Translation in Transform_Initial
  FTransform
      Transform_Initial_No_Translation;  // Initial transform specified by
                                         // configuration with no translation
  FTransform Transform_Initial;  // Initial transform specified by configuration
  FnCollisionHitCallback
      Collision_Hit_Callback;  // Callback to invoke on collision
};