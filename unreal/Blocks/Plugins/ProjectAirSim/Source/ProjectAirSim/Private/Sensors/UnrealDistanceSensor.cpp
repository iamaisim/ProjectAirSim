// Copyright (C) Microsoft Corporation.  All rights reserved.
// Unreal DistanceSensor Implementation

#include "UnrealDistanceSensor.h"

#include <cmath>

#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Engine/World.h"
#include "ProjectAirSim.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "UnrealLogger.h"

namespace projectairsim = microsoft::projectairsim;

UUnrealDistanceSensor::UUnrealDistanceSensor(
    const FObjectInitializer& ObjectInitializer)
    : UUnrealSensor(ObjectInitializer) {
  bAutoActivate = true;

  // Tick in PostUpdateWork to update at the same time as UnrealCamera, instead
  // of waiting for the PrePhysics tick on the next loop
  PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UUnrealDistanceSensor::Initialize(
    const projectairsim::DistanceSensor& SimDistanceSensor) {
  DistanceSensor = SimDistanceSensor;
  OwnerActor = GetOwner();

  SetupDistanceSensorFromSettings(
      SimDistanceSensor.GetDistanceSensorSettings());

  RegisterComponent();
}

void UUnrealDistanceSensor::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  const TimeNano CurSimTime = projectairsim::SimClock::Get()->NowSimNanos();
  const TimeNano SimTimeDeltaLastReport = CurSimTime - LastSimTimeReport;

  // Generate report message according to reporting frequency setting
  if ((LastSimTimeReport == 0) ||
      (SimTimeDeltaLastReport >= SimTimeDeltaReportTarget)) {
    // Retrieve new sensor returns
    Simulate();

    const auto DistanceSensorTransformStamped =
        UnrealTransform::GetPoseNed(this);
    projectairsim::Pose DistanceSensorPose(
        DistanceSensorTransformStamped.translation_,
        DistanceSensorTransformStamped.rotation_);

    projectairsim::DistanceSensorMessage DistanceSensorMsg(CurSimTime, Distance,
                                                           DistanceSensorPose);
    DistanceSensor.PublishDistanceSensorMsg(DistanceSensorMsg);

    // Save report time, adjusting for the difference between actual and target
    // interval to achieve the target reporting interval on average
    {
      auto DSimTime = (SimTimeDeltaLastReport - SimTimeDeltaReportTarget);

      if (DSimTime > 0) {
        if (DSimTime > (2 * SimTimeDeltaReportTarget))
          DSimTime =
              2 * SimTimeDeltaReportTarget;  // Limit the time adjustment so it
                                             // doesn't grow out of control

        LastSimTimeReport = CurSimTime - DSimTime;
      }
    }
  }

  LastSimTime = CurSimTime;
}

void UUnrealDistanceSensor::SetupDistanceSensorFromSettings(
    const projectairsim::DistanceSensorSettings& DistanceSensorSettings) {
  Settings = DistanceSensorSettings;

  InitializePose(Settings.origin);

  // Calculate reporting interval from frequency
  SimTimeDeltaReportTarget = (Settings.report_frequency < 0.01f)
                                 ? 0
                                 : projectairsim::SimClock::Get()->SecToNanos(
                                       1.0f / Settings.report_frequency);
}

void UUnrealDistanceSensor::BeginPlay() { Super::BeginPlay(); }

void UUnrealDistanceSensor::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);
  DistanceSensor.EndUpdate();
}

void UUnrealDistanceSensor::InitializePose(
    const projectairsim::Transform& PoseNed) {
  SetRelativeTransform(UnrealTransform::FromGlobalNed(PoseNed));

  // Check that the initial pose was set correctly
  projectairsim::Transform InitializedPose = UnrealTransform::GetPoseNed(this);
  projectairsim::Vector3 InitializedRPY =
      projectairsim::TransformUtils::ToDegrees(
          projectairsim::TransformUtils::ToRPY(InitializedPose.rotation_));
  UnrealLogger::Log(
      projectairsim::LogLevel::kTrace,
      TEXT("[UnrealDistanceSensor] DistanceSensor '%S': InitializePose(). "
           "RelativeLocation (%f,%f,%f) RelativeRotationRPY (%f,%f,%f)"),
      DistanceSensor.GetId().c_str(), InitializedPose.translation_.x(),
      InitializedPose.translation_.y(), InitializedPose.translation_.z(),
      InitializedRPY.x(), InitializedRPY.y(), InitializedRPY.z());
}

// Simulate shooting a laser via Unreal ray-tracing.
FHitResult UUnrealDistanceSensor::GetSensedPoint(
    const FVector& DistanceSensorBodyLoc,
    const FRotator& DistanceSensorBodyRot) {
  const FVector RayDirectionVector =
      UKismetMathLibrary::GetForwardVector(DistanceSensorBodyRot);

  // Calculate "EndTrace": point corresponding to end of ray trace.
  const FVector EndTrace =
      DistanceSensorBodyLoc +
      (projectairsim::TransformUtils::ToCentimeters(Settings.max_distance) *
       RayDirectionVector);

  // Shoot ray via LineTraceSingleByChannel, result is saved in HitInfo
  FCollisionQueryParams TraceParams;
  TraceParams.AddIgnoredActor(OwnerActor);  // don't hit yourself
  TraceParams.bTraceComplex = true;
  TraceParams.bReturnPhysicalMaterial = false;

  FHitResult HitInfo(ForceInit);

  UnrealWorld->LineTraceSingleByChannel(
      HitInfo, DistanceSensorBodyLoc, EndTrace, ECC_Visibility, TraceParams,
      FCollisionResponseParams::DefaultResponseParam);

  return HitInfo;
}

void UUnrealDistanceSensor::Simulate() {
  const FVector DistanceSensorBodyLoc = GetComponentLocation();
  const FRotator DistanceSensorBodyRot = GetComponentRotation();

  const FHitResult HitInfo =
      GetSensedPoint(DistanceSensorBodyLoc, DistanceSensorBodyRot);

  Distance = HitInfo.Distance;

  // Draw debug point, if enabled

  if (Settings.draw_debug_points && UnrealWorld) {
    DrawDebugPoint(UnrealWorld, HitInfo.ImpactPoint,
                   10,                 // size
                   FColor(0, 0, 255),  // RGB
                   false,              // persistent (never goes away)
                   0.1                 // time that point persists on object
    );
  }
}