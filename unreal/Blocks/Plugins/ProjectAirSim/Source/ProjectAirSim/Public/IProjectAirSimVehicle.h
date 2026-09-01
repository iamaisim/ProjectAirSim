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
 * parameter signals from ProjectAirSim.
 *
 * Kinematics (position, velocity, acceleration) are read automatically
 * from the actor's transform and physics — no need to implement anything
 * for those.
 *
 * To use in Blueprint: Class Settings -> Interfaces -> Add "ProjectAirSimVehicle"
 * Then implement SetParameterSignal from the My Blueprint panel.
 */
class PROJECTAIRSIM_API IProjectAirSimVehicle {
  GENERATED_BODY()

 public:
  /** Called when a parameter signal is sent from the Python client.
   *  Override in Blueprint to interpret the index for the target vehicle.
   *  @param Index Parameter index defined by the vehicle implementation
   *  @param Value Parameter value */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProjectAirSimVehicle")
  void SetParameterSignal(int32 Index, float Value);

  /** Get the actor's linear velocity in cm/s (Unreal coords).
   *  Default: returns GetVelocity() of the owning actor. */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProjectAirSimVehicle")
  FVector GetLinearVelocity();

  /** Get the actor's angular velocity in rad/s (Unreal coords).
   *  Default: returns physics angular velocity of root component. */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProjectAirSimVehicle")
  FVector GetAngularVelocity();

  /** Get the actor's linear acceleration in cm/s^2 (Unreal coords).
   *  Default: returns (0,0,0). */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProjectAirSimVehicle")
  FVector GetLinearAcceleration();

  /** Get the actor's angular acceleration in rad/s^2 (Unreal coords).
   *  Default: returns (0,0,0). */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProjectAirSimVehicle")
  FVector GetAngularAcceleration();

  /** Get the actor's world position in cm (Unreal coords).
   *  Default: returns GetActorLocation() of the owning actor. */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProjectAirSimVehicle")
  FVector GetPosition();

  /** Get the actor's world rotation as a quaternion (Unreal coords).
   *  Default: returns GetActorQuat() of the owning actor. */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProjectAirSimVehicle")
  FQuat GetRotation();

  /** Reset the actor to its spawn-time pose (position, rotation, zero velocity).
   *  Called by ProjectAirSim whenever the simulation is reset/reloaded.
   *  Default implementation does nothing — AProjectAirSimVehicleActorBase provides
   *  a full default. Override in Blueprint for custom reset logic (e.g. AI state). */
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProjectAirSimVehicle")
  void ResetToSpawnPose();
};
