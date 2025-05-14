// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "Camera/CameraComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Actor.h"
#include "Runtime/Core/Public/Templates/SharedPointer.h"
#include "Sensors/UnrealSensor.h"
#include "UnrealRobotJoint.h"
#include "UnrealRobotLink.h"
#include "core_sim/actor/robot.hpp"
#include "core_sim/clock.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/sensors/camera.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "unreal_physics.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "UnrealRobot.generated.h"

class AUnrealScene; 
class UUnrealCamera;

UCLASS()
class AUnrealRobot : public AActor {
  GENERATED_BODY()

 public:
  explicit AUnrealRobot(const FObjectInitializer& ObjectInitialize);

  void Initialize(const microsoft::projectairsim::Robot& InSimRobot,
                  microsoft::projectairsim::UnrealPhysicsBody* InPhysBody,
                  AUnrealScene* UnrealScene);    
  void Tick(float DeltaTime) override;

  void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

  UFUNCTION()
  void OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                      UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                      const FHitResult& Hit);

  const microsoft::projectairsim::Kinematics& GetKinematics() const;

  bool SetNextStreamingCapture();

  UUnrealCamera* GetActiveStreamingCamera();

  USceneCaptureComponent2D* GetActiveStreamingCapture();

  void SetViewportResolution();

 protected:
  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

 private:
  typedef UUnrealRobotLink::ActuatedFTransform ActuatedFTransform;

 private:
  void InitializeId(const std::string& InId);

  void InitializeLinks(
      const std::vector<microsoft::projectairsim::Link>& InLinks,
      const std::set<std::string>& InRootLinks, bool bWithUnrealPhysics);

  std::pair<std::string, UUnrealRobotLink*> CreateLink(
      const microsoft::projectairsim::Link& InLink, bool IsRootLink,
      bool bWithUnrealPhysics);

  void InitializeJoints(
      const std::vector<microsoft::projectairsim::Joint>& InJoints);

  std::pair<std::string, UUnrealRobotJoint*> CreateJoint(
      const microsoft::projectairsim::Joint& InJoint);

  void InitializeSensors(
      const std::vector<
          std::reference_wrapper<microsoft::projectairsim::Sensor>>& InSensors);

  void SetRobotKinematics(const microsoft::projectairsim::Kinematics& InKin,
                          TimeNano InTimestamp);

  void SetActuatedTransforms(
      const microsoft::projectairsim::ActuatedTransforms& InActuatedTransforms,
      TimeNano DeltaSimtime);

  void MoveRobotToUnrealPose(bool bUseCollisionSweep);

  void ApplyActuatedTransforms();

  void SetExternalWrench(microsoft::projectairsim::Wrench InWrench);

  std::set<std::string> GetRootLinks(
      const std::vector<microsoft::projectairsim::Link>& InLinks,
      const std::vector<microsoft::projectairsim::Joint>& InJoints);

  UUnrealRobotLink* RobotRootLink;

  microsoft::projectairsim::Robot SimRobot;
  microsoft::projectairsim::UnrealPhysicsBody* SimPhysicsBody;
  std::map<std::string, UUnrealRobotLink*> RobotLinks;
  std::map<std::string, UUnrealRobotJoint*> RobotJoints;
  std::map<std::string, UUnrealSensor*> RobotSensors;

  TArray<UUnrealCamera*> StreamingCameras;
  int32 StreamingCameraActiveIdx = 0;

  AUnrealScene* UnrealScene;

  FCriticalSection UpdateMutex;

  bool bHasKinematicsUpdated = false;
  microsoft::projectairsim::Kinematics RobotKinematics;
  TimeNano KinematicsUpdatedTimeStamp = 0;

  bool bHasUnrealPoseUpdated = false;
  TimeNano UnrealPoseUpdatedTimeStamp = 0;

  TMap<FString, ActuatedFTransform> RobotActuatedTransforms;

  // New Box Component for Physics
    UPROPERTY()
    UBoxComponent* PhysicsBoxComponent;
};
