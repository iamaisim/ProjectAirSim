// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "IProjectAirSimVehicle.generated.h"

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UProjectAirSimVehicle : public UInterface {
  GENERATED_BODY()
};

/**
 * Interface that any AActor (Blueprint or C++) can implement to receive
 * parameter signals from Project AirSim.
 *
 * To use in Blueprint: Class Settings -> Interfaces -> Add
 * "ProjectAirSimVehicle" Then implement SetParameterSignal from the My
 * Blueprint panel.
 */
class PROJECTAIRSIM_API IProjectAirSimVehicle {
  GENERATED_BODY()

 public:
  /** Called when a parameter signal is sent from the Python client.
   *  Override in Blueprint to interpret the index for the target vehicle.
   *  @param Index Parameter index defined by the vehicle implementation
   *  @param Value Parameter value */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable,
            Category = "ProjectAirSimVehicle")
  void SetParameterSignal(int32 Index, float Value);
};
