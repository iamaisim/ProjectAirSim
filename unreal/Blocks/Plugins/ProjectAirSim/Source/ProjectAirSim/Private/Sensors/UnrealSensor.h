// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// The Unreal sensor base.
// It is currently empty and could potentially be replaced by USceneComponent.

#pragma once

#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "core_sim/clock.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "UnrealSensor.generated.h"

UCLASS()
class UUnrealSensor : public USceneComponent {
  GENERATED_BODY()

 public:
  explicit UUnrealSensor(const FObjectInitializer& ObjectInitializer)
      : USceneComponent(ObjectInitializer) {
    // GetWorld() should only be called on the game thread, so call it in the
    // sensor's constructor to save it
    UnrealWorld = this->GetWorld();
  }

  void SetHasNewState(bool bNewState) { bHasNewState = bNewState; }

  void SetSimTimeAtPoseUpdate(TimeNano SimTimeNano) {
    PoseUpdatedTimeStamp = SimTimeNano;
  }

 protected:
  bool bHasNewState = false;
  TimeNano PoseUpdatedTimeStamp = 0;
  UWorld* UnrealWorld = nullptr;

 private:
};
