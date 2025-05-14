// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UnrealSimLoader.h"

//
#include "ProjectAirSimGameMode.generated.h"

UCLASS()
class PROJECTAIRSIM_API AProjectAirSimGameMode : public AGameModeBase {
  GENERATED_BODY()

 public:
  explicit AProjectAirSimGameMode(const FObjectInitializer& ObjectInitializer);

  void StartPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

 private:
  AUnrealSimLoader UnrealSimLoader;
};
