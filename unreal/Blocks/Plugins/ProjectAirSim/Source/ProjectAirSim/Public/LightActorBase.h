// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LightActorBase.generated.h"

UCLASS(Abstract)
class PROJECTAIRSIM_API ALightActorBase : public AActor {
  GENERATED_BODY()

 public:
  bool SetIntensity(float NewIntensity);
  bool SetLightFColor(FColor NewLightColor);
  bool SetRadius(float NewRadius);

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TArray<ULightComponent*> lightComponents;
};
