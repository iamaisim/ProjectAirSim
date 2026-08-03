// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IProjectAirSimVehicle.h"

#include "ProjectAirSimVehicleActorBase.generated.h"

/**
 * Optional base class for ProjectAirSim vehicle actors controlled by ProjectAirSim.
 *
 * Inherit your Blueprint from this class instead of Actor/Pawn to get free
 * default implementations of all kinematic getters and a default force-based
 * actuator response. Override SetActuatorSignal in your Blueprint for custom
 * behaviour.
 *
 * Default behaviour:
 *  - GetPosition()            -> GetActorLocation()
 *  - GetRotation()            -> GetActorQuat()
 *  - GetLinearVelocity()      -> root component physics linear velocity (cm/s)
 *  - GetAngularVelocity()     -> root component physics angular velocity (rad/s)
 *  - GetLinearAcceleration()  -> returns (0,0,0)  (override if needed)
 *  - GetAngularAcceleration() -> returns (0,0,0)  (override if needed)
 *  - GetActuatorSignal()      -> returns stored value (index-based)
 *  - SetActuatorSignal()      -> stores signal; Tick applies forces when
 *                                bApplyDefaultActuatorForces is true (default):
 *                                  Index 0 (throttle) -> forward acceleration
 *                                  Index 1 (brake)    -> reverse acceleration
 *                                  Index 2 (steering) -> yaw torque
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = "ProjectAirSim",
       meta = (DisplayName = "Project AirSim Vehicle Actor Base"))
class PROJECTAIRSIM_API AProjectAirSimVehicleActorBase
    : public AActor,
      public IProjectAirSimVehicle {
  GENERATED_BODY()

 public:
  AProjectAirSimVehicleActorBase();

  // ---- Configurable default force parameters ----

  /** When true (default), Tick applies forces to the root physics component
   *  based on the standard throttle/brake/steering actuator signals.
   *  Set to false in your Blueprint if you want to handle forces yourself. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAirSimVehicle|Defaults")
  bool bApplyDefaultActuatorForces = true;

  /** Acceleration (cm/s²) applied per unit of throttle minus brake. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAirSimVehicle|Defaults")
  float ThrottleAcceleration = 600.0f;

  /** Yaw angular acceleration (deg/s²) applied per unit of steering. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectAirSimVehicle|Defaults")
  float SteeringTorqueDegPerSec2 = 180.0f;

  // ---- IProjectAirSimVehicle ----

  /** Stores the signal for use in Tick. Override in Blueprint for custom logic. */
  virtual void SetActuatorSignal_Implementation(int32 Index, float Signal) override;

  virtual float GetActuatorSignal_Implementation(const FString& Name) override;

  virtual FVector GetPosition_Implementation() override;
  virtual FQuat   GetRotation_Implementation() override;
  virtual FVector GetLinearVelocity_Implementation() override;
  virtual FVector GetAngularVelocity_Implementation() override;
  virtual FVector GetLinearAcceleration_Implementation() override  { return FVector::ZeroVector; }
  virtual FVector GetAngularAcceleration_Implementation() override { return FVector::ZeroVector; }

 private:
  TArray<float> StoredActuatorSignals_;
};
