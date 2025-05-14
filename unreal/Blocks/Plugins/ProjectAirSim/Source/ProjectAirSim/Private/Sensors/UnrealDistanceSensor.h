// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// UnrealDistanceSensorSensor implementation that uses Ray Tracing in Unreal.

#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "UnrealSensor.h"
#include "core_sim/clock.hpp"
#include "core_sim/sensors/distance_sensor.hpp"
#include "core_sim/transforms/transform_utils.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "UnrealDistanceSensor.generated.h"  //?

UCLASS() class UUnrealDistanceSensor : public UUnrealSensor {
  GENERATED_BODY()

 public:
  explicit UUnrealDistanceSensor(const FObjectInitializer& ObjectInitializer);

  void Initialize(const microsoft::projectairsim::DistanceSensor& DistanceSensor);

  void TickComponent(float DeltaTime, ELevelTick TickType,
                     FActorComponentTickFunction* ThisTickFunction) override;

  void SetupDistanceSensorFromSettings(
      const microsoft::projectairsim::DistanceSensorSettings&
          DistanceSensorSettings);

 protected:
  void BeginPlay() override;
  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

 private:
  void InitializePose(const microsoft::projectairsim::Transform& Pose);

  FHitResult GetSensedPoint(const FVector& DistanceSensorBodyLoc,
                              const FRotator& DistanceSensorBodyRot);

  void Simulate();

 private:
  TimeNano LastSimTime = 0;        // Simulation time of last sensor update
  TimeNano LastSimTimeReport = 0;  // Simulation time of last sensor report
  microsoft::projectairsim::DistanceSensor
      DistanceSensor;  // Corresponding SDK sensor object
  AActor* OwnerActor;  // Owning engine actor
  float Distance;      // Latest sensor report
  microsoft::projectairsim::DistanceSensorSettings Settings;  // Sensor settings
  TimeNano SimTimeDeltaReportTarget =
      0;  // Desired sensor report interval (0 = as soon as possible)
};
