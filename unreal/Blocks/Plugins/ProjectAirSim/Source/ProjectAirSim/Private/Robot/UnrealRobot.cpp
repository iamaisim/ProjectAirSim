// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// The Unreal robot implementation.

#include "UnrealRobot.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/GameUserSettings.h"
#include "Misc/ScopeLock.h"
#include "ProjectAirSim.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Sensors/UnrealSensorFactory.h"
#include "UnrealHelpers.h"
#include "UnrealLogger.h"
#include "UnrealScene.h"
#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/transforms/transform_utils.hpp"

namespace projectairsim = microsoft::projectairsim;

AUnrealRobot::AUnrealRobot(const FObjectInitializer& ObjectInitialize)
    : AActor(ObjectInitialize) {
  PrimaryActorTick.bCanEverTick = true;
  // Tick group is set in Initialize() based on the robot's physics type
}

void AUnrealRobot::Initialize(const projectairsim::Robot& InSimRobot,
                              projectairsim::UnrealPhysicsBody* InPhysBody,
                              AUnrealScene* InUnrealScene) {
  // Store ptrs to other corresponding components for this robot
  UnrealScene = InUnrealScene;
  SimPhysicsBody = InPhysBody;
  this->SimRobot = InSimRobot;  // This makes a copy from the robot ref. It does
                                // get another shared_ptr for the same address
                                // of the robot's impl/ActorImpl, but any data
                                // at the robot level would be decoupled so only
                                // store robot data at the impl level.

  // Detect which links are roots based on their joint attachments
  auto RootLinks = GetRootLinks(InSimRobot.GetLinks(), InSimRobot.GetJoints());

  bool bWithUnrealPhysics = (InSimRobot.GetPhysicsType() ==
                             projectairsim::PhysicsType::kUnrealPhysics);

  if (bWithUnrealPhysics) {
    // For Unreal-calculated physics, do the updates after Unreal has completed
    // the physics tick calculations.
    PrimaryActorTick.TickGroup = TG_PostPhysics;
  } else {
    // For -calculated physics, do the updates during Unreal's world
    // physics tick to stay in sequence with the other components.
    PrimaryActorTick.TickGroup = TG_DuringPhysics;
  }

  // Initialize the configured robot component structure
  InitializeId(InSimRobot.GetID());
  InitializeLinks(InSimRobot.GetLinks(), RootLinks, bWithUnrealPhysics);
  InitializeJoints(InSimRobot.GetJoints());
  InitializeSensors(InSimRobot.GetSensors());

  StreamingCameraActiveIdx = 0;

  // Set the initial pose from the robot's kinematics
  SetRobotKinematics(InSimRobot.GetKinematics(), 0);  // timestamp=0
  MoveRobotToUnrealPose(false);  // move without collision sweep
}

void AUnrealRobot::InitializeId(const std::string& InId) {
  UnrealHelpers::SetActorName(this, InId);
}

void AUnrealRobot::InitializeLinks(
    const std::vector<projectairsim::Link>& InLinks,
    const std::set<std::string>& InRootLinks, bool bWithUnrealPhysics) {
  std::for_each(
      InLinks.begin(), InLinks.end(),
      [this, &InRootLinks,
       bWithUnrealPhysics](const projectairsim::Link& CurLink) {
        bool bIsRootLink = false;
        if (InRootLinks.find(CurLink.GetID()) != InRootLinks.end()) {
          bIsRootLink = true;
        }
        RobotLinks.insert(CreateLink(CurLink, bIsRootLink, bWithUnrealPhysics));
      });
}

std::pair<std::string, UUnrealRobotLink*> AUnrealRobot::CreateLink(
    const projectairsim::Link& InLink, bool bIsRootLink,
    bool bWithUnrealPhysics) {
  auto Id = InLink.GetID();

  auto NewLink = NewObject<UUnrealRobotLink>(this, Id.c_str());
  NewLink->Initialize(InLink, bWithUnrealPhysics);

  if (bIsRootLink) {
    RobotRootLink = NewLink;
    RootComponent = NewLink;  // Also set as the USceneComponent's RootComponent
  }

  return {Id, NewLink};
}

void AUnrealRobot::OnCollisionHit(UPrimitiveComponent* HitComponent,
                                  AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp,
                                  FVector NormalImpulse,
                                  const FHitResult& Hit) {
  if ((OtherActor != nullptr) && (OtherActor != this) &&
      (OtherComp != nullptr)) {
    projectairsim::CollisionInfo NewCollisionInfo;
    NewCollisionInfo.has_collided = true;

    // NEU -> NED_m
    NewCollisionInfo.normal =
        projectairsim::TransformUtils::NeuToNedLinear(projectairsim::Vector3(
            Hit.ImpactNormal.X, Hit.ImpactNormal.Y, Hit.ImpactNormal.Z));

    // NEU_cm -> NEU_m -> NED_m
    NewCollisionInfo.impact_point =
        projectairsim::TransformUtils::NeuToNedLinear(
            projectairsim::TransformUtils::ToMeters(projectairsim::Vector3(
                Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z)));

    // NEU_cm -> NEU_m -> NED_m
    NewCollisionInfo.position = projectairsim::TransformUtils::NeuToNedLinear(
        projectairsim::TransformUtils::ToMeters(projectairsim::Vector3(
            Hit.Location.X, Hit.Location.Y, Hit.Location.Z)));

    // cm -> m
    NewCollisionInfo.penetration_depth =
        projectairsim::TransformUtils::ToMeters(Hit.PenetrationDepth);

    NewCollisionInfo.time_stamp = projectairsim::SimClock::Get()->NowSimNanos();
    NewCollisionInfo.object_name =
        std::string(TCHAR_TO_UTF8(*(OtherActor->GetName())));

    UPrimitiveComponent* OtherRootComp =
        Cast<class UPrimitiveComponent>(OtherActor->GetRootComponent());
    NewCollisionInfo.segmentation_id =
        OtherRootComp ? OtherRootComp->CustomDepthStencilValue : -1;

    SimRobot.UpdateCollisionInfo(NewCollisionInfo);

    if (HitComponent == RobotRootLink) {
      UnrealLogger::Log(
          projectairsim::LogLevel::kTrace,
          TEXT("Collision detected between '%s' and '%s' at z= '%f'"),
          *(HitComponent->GetName()), *(OtherActor->GetName()), Hit.Location.Z);
    }
  }
}

const microsoft::projectairsim::Kinematics& AUnrealRobot::GetKinematics()
    const {
  return RobotKinematics;
}

// Cycles through each streaming capture of each streaming camera. Returns true
// if it has cycled back to the first streaming camera, otherwise returns false.
bool AUnrealRobot::SetNextStreamingCapture() {
  if (StreamingCameras.Num() == 0) {
    // Return true to trigger switching to next robot since there are no valid
    // cameras to increment through
    return true;
  }

  UUnrealCamera* ActiveStreamingCam =
      StreamingCameras[StreamingCameraActiveIdx];

  // If currently active streaming cam is invalid, look for next valid one
  while (ActiveStreamingCam == nullptr &&
         StreamingCameraActiveIdx < StreamingCameras.Num() - 1) {
    StreamingCameraActiveIdx++;
    ActiveStreamingCam = StreamingCameras[StreamingCameraActiveIdx];
  }

  if (ActiveStreamingCam == nullptr) {
    // There were no more valid streaming cams, so cycle back to the beginning
    StreamingCameraActiveIdx = 0;
    return true;
  }

  // Cycle to the next streaming capture of the current active streaming camera
  auto ActiveStreamingCaptureIdx =
      ActiveStreamingCam->GetNextStreamingCaptureIdx();

  // If it has cycled back to the first streaming capture of this streaming
  // camera, cycle to the next streaming camera. If this cycles back to the
  // first streaming camera, return true.
  if (ActiveStreamingCaptureIdx < 1) {
    StreamingCameraActiveIdx++;
    if (StreamingCameraActiveIdx >= StreamingCameras.Num()) {
      StreamingCameraActiveIdx = 0;
      return true;
    }
  }

  // Has not cycled back to the first streaming camera yet, so return false.
  return false;
}

// Get the robot's currently active streaming camera
UUnrealCamera* AUnrealRobot::GetActiveStreamingCamera() {
  if (StreamingCameras.Num() == 0) return nullptr;

  UUnrealCamera* Camera = StreamingCameras[StreamingCameraActiveIdx];
  return Camera;
}

// Get the robot's currently active streaming camera capture
USceneCaptureComponent2D* AUnrealRobot::GetActiveStreamingCapture() {
  if (StreamingCameras.Num() == 0) return nullptr;

  UUnrealCamera* Camera = StreamingCameras[StreamingCameraActiveIdx];
  if (Camera == nullptr) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kWarning,
        TEXT("[%s] Invalid pointer to the currently active streaming camera "
             "when getting the active streaming capture."),
        *GetName());
    return nullptr;
  }

  USceneCaptureComponent2D* Capture = Camera->GetActiveStreamingCapture();

  return Capture;
}

// Set viewport resolution to match the active streaming camera's render target
// resolution so that the pixel stream will also update its resolution to match.
void AUnrealRobot::SetViewportResolution() {
  if (StreamingCameras.Num() == 0) return;

  UUnrealCamera* Camera = StreamingCameras[StreamingCameraActiveIdx];
  if (Camera == nullptr) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kWarning,
        TEXT("[%s] Invalid pointer to the currently active streaming camera "
             "when setting viewport resolution."),
        *GetName());
    return;
  }

  USceneCaptureComponent2D* Capture = Camera->GetActiveStreamingCapture();
  if (Capture == nullptr) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kWarning,
        TEXT("[%s] Invalid pointer to the currently active streaming capture "
             "when setting viewport resolution."),
        *GetName());
    return;
  }

  UTextureRenderTarget2D* RenderTarget = Capture->TextureTarget;
  if (RenderTarget == nullptr) {
    UnrealLogger::Log(projectairsim::LogLevel::kWarning,
                      TEXT("[%s] Invalid pointer to the currently active "
                           "streaming capture's render target when setting "
                           "viewport resolution."),
                      *GetName());
    return;
  }

  FIntPoint Resolution;
  Resolution.X = RenderTarget->SizeX;
  Resolution.Y = RenderTarget->SizeY;

  UGameUserSettings* Settings = GEngine->GetGameUserSettings();
  Settings->SetScreenResolution(Resolution);
  Settings->ApplyResolutionSettings(/*bCheckForCommandLineOverrides*/ false);
}

void AUnrealRobot::InitializeJoints(
    const std::vector<projectairsim::Joint>& InJoints) {
  std::for_each(InJoints.begin(), InJoints.end(),
                [this](const projectairsim::Joint& CurJoint) {
                  RobotJoints.insert(CreateJoint(CurJoint));
                });
}

std::pair<std::string, UUnrealRobotJoint*> AUnrealRobot::CreateJoint(
    const projectairsim::Joint& InJoint) {
  std::string Id = InJoint.GetID();
  UUnrealRobotLink* Parent = nullptr;
  UUnrealRobotLink* Child = nullptr;

  auto ParentRobotLinkItr = RobotLinks.find(InJoint.GetParentLink().c_str());
  if (ParentRobotLinkItr != RobotLinks.end()) {
    Parent = ParentRobotLinkItr->second;
  } else {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("Joint '%hs' has an invalid parent link."),
                      Id.c_str());
    return {Id, nullptr};
  }

  auto ChildRobotLinkItr = RobotLinks.find(InJoint.GetChildLink().c_str());
  if (ChildRobotLinkItr != RobotLinks.end()) {
    Child = ChildRobotLinkItr->second;
  } else {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("Joint '%hs' has an invalid child link."),
                      Id.c_str());
    return {Id, nullptr};
  }

  if (Parent->GetStaticMesh() == nullptr || Child->GetStaticMesh() == nullptr ||
      Parent->GetStaticMesh() == Child->GetStaticMesh()) {
    // This is an invalid Unreal Physics joint (like if any UnrealLinks are
    // created without a visible Static Mesh assigned), so just directly attach
    // the UnrealLinks and return a null UUnrealRobotJoint pointer
    UnrealLogger::Log(projectairsim::LogLevel::kWarning,
                      TEXT("Joint '%hs' has an invalid physical constraint, "
                           "making direct fixed attachment instead."),
                      Id.c_str());
    Child->AttachToComponent(Parent,
                             FAttachmentTransformRules::KeepRelativeTransform);
    return {Id, nullptr};
  }

  auto NewJoint = NewObject<UUnrealRobotJoint>(this, Id.c_str());
  NewJoint->Initialize(InJoint);

  NewJoint->ConstraintActor1 = this;
  NewJoint->ConstraintActor2 = this;

  NewJoint->AttachToComponent(Parent,
                              FAttachmentTransformRules::KeepRelativeTransform);
  Child->AttachToComponent(NewJoint,
                           FAttachmentTransformRules::KeepRelativeTransform);
  NewJoint->SetConstrainedComponents(Parent, NAME_None, Child, NAME_None);

  return {Id, NewJoint};
}

std::set<std::string> AUnrealRobot::GetRootLinks(
    const std::vector<projectairsim::Link>& InLinks,
    const std::vector<projectairsim::Joint>& InJoints) {
  std::set<std::string> Roots;

  std::transform(
      InLinks.begin(), InLinks.end(), std::inserter(Roots, Roots.begin()),
      [Roots](const projectairsim::Link& CurLink) { return CurLink.GetID(); });

  std::for_each(InJoints.begin(), InJoints.end(),
                [&Roots](const projectairsim::Joint& CurJoint) {
                  Roots.erase(CurJoint.GetChildLink());
                });

  return Roots;
}

void AUnrealRobot::InitializeSensors(
    const std::vector<std::reference_wrapper<projectairsim::Sensor>>&
        InSensors) {
  std::for_each(
      InSensors.begin(), InSensors.end(),
      [this](const std::reference_wrapper<projectairsim::Sensor> CurSensor) {
        if (CurSensor.get().IsEnabled()) {
          USceneComponent* Parent = nullptr;
          const std::string& ParentLink = CurSensor.get().GetParentLink();
          if (ParentLink != "") {
            auto ParentRobotLinkItr = RobotLinks.find(ParentLink.c_str());
            if (ParentRobotLinkItr != RobotLinks.end()) {
              Parent = ParentRobotLinkItr->second;
            } else {
              UnrealLogger::Log(
                  projectairsim::LogLevel::kWarning,
                  TEXT("Sensor '%hs' has an invalid parent link. Setting its "
                       "parent to the robot's root component instead."),
                  CurSensor.get().GetId().c_str());
            }
          }
          if (Parent == nullptr) Parent = GetRootComponent();

          std::pair<std::string, UUnrealSensor*> Pair =
              UnrealSensorFactory::CreateSensor(CurSensor.get(), Parent,
                                                UnrealScene);
          RobotSensors.insert(Pair);

          // Add UnrealCameras that have streaming captures to the UnrealRobot's
          // StreamingCameras array.
          if (CurSensor.get().GetType() ==
              microsoft::projectairsim::SensorType::kCamera) {
            UUnrealCamera* UnrealCamera = Cast<UUnrealCamera>(Pair.second);
            if (UnrealCamera != nullptr &&
                UnrealCamera->GetActiveStreamingCapture() != nullptr) {
              StreamingCameras.Add(UnrealCamera);
            }
          }
        }
      });
}

void AUnrealRobot::MoveRobotToUnrealPose(bool bUseCollisionSweep) {
  if (RobotRootLink == nullptr || bHasKinematicsUpdated == false) return;

  // Clear robot's has_collided flag before trying to move again
  SimRobot.SetHasCollided(false);

  // Copy target pose data in case it gets updated again while processing
  projectairsim::Pose TgtPose = RobotKinematics.pose;
  UnrealPoseUpdatedTimeStamp = KinematicsUpdatedTimeStamp;
  bHasKinematicsUpdated = false;  // done processing kinematics, clear flag

  // Use local copy of target pose to do actual robot pose update
  const FVector TgtLocNEU =
      UnrealHelpers::ToFVector(projectairsim::TransformUtils::NedToNeuLinear(
          projectairsim::TransformUtils::ToCentimeters(TgtPose.position)));
  const FRotator TgtRot = UnrealHelpers::ToFRotator(TgtPose.orientation);

  // Move UE position
  RobotRootLink->SetWorldLocationAndRotation(TgtLocNEU, TgtRot,
                                             bUseCollisionSweep, nullptr,
                                             ETeleportType::TeleportPhysics);
  // If 'bUseCollisionSweep' is true, collision hits during
  // SetWorldLocationAndRotation will be handled by the
  // callback AUnrealRobot::OnCollisionHit() with the FHitResult info

  bHasUnrealPoseUpdated = true;

  // UnrealLogger::Log(projectairsim::LogLevel::kTrace,
  //                   TEXT("SetWorldLocationAndRotation xyz=%f, %f, %f"),
  //                   TgtLocNEU.X, TgtLocNEU.Y, TgtLocNEU.Z);
}

void AUnrealRobot::ApplyActuatedTransforms() {
  FScopeLock ScopeLock(&UpdateMutex);

  // TODO Add handling of rotating lift-drag control surfaces.

  // Spin actuator output link meshes (e.g. propellers) by manually adding
  // local rotation
  for (const auto& [ActuatorOutputLink, ActuatedTransform] :
       RobotActuatedTransforms) {
    std::string ActuatorOutputLinkStr(TCHAR_TO_UTF8(*ActuatorOutputLink));
    auto OutputRobotLinkItr = RobotLinks.find(ActuatorOutputLinkStr);
    if (OutputRobotLinkItr != RobotLinks.end()) {
      UUnrealRobotLink* OutputRobotLink = OutputRobotLinkItr->second;
      OutputRobotLink->SetActuatedFTransform(ActuatedTransform);
    } else {
      UnrealLogger::Log(projectairsim::LogLevel::kError,
                        TEXT("Actuator output link '%hs' is invalid."),
                        ActuatorOutputLinkStr.c_str());
    }
  }

  // Clear the angles to start accumulating them again
  RobotActuatedTransforms.Empty();
}

void AUnrealRobot::SetRobotKinematics(const projectairsim::Kinematics& InKin,
                                      TimeNano InTimestamp) {
  FScopeLock ScopeLock(&UpdateMutex);
  RobotKinematics = InKin;
  KinematicsUpdatedTimeStamp = InTimestamp;
  bHasKinematicsUpdated = true;

  // UnrealLogger::Log(projectairsim::LogLevel::kTrace,
  //                   TEXT("UpdateRobotTargetKinematics xyz=%f, %f, %f"),
  //                   RobotTargetKinematics.pose.position.x(),
  //                   RobotTargetKinematics.pose.position.y(),
  //                   RobotTargetKinematics.pose.position.z());
}

void AUnrealRobot::SetActuatedTransforms(
    const projectairsim::ActuatedTransforms& InActuatedTransforms,
    TimeNano DeltaSimtime) {
  FScopeLock ScopeLock(&UpdateMutex);

  // Accumulate incremental rotation angles for these actuated links
  for (const auto& [ActuatedLinkIDStr, ASVActuatedTransform] :
       InActuatedTransforms) {
    FString ActuatedLinkID(ActuatedLinkIDStr.c_str());
    ActuatedFTransform* ActuatedTransformRef =
        RobotActuatedTransforms.Find(ActuatedLinkID);

    if (ActuatedTransformRef == nullptr) {
      RobotActuatedTransforms.Add(ActuatedLinkID, ASVActuatedTransform);
    } else {
      *ActuatedTransformRef = ASVActuatedTransform;
    }
  }
}

void AUnrealRobot::SetExternalWrench(projectairsim::Wrench InWrench) {
  // Apply rigid body wrench from physics body to root link's origin
  // NED_m -> NED_cm -> NEU_cm
  projectairsim::Vector3 ForceNEU =
      projectairsim::TransformUtils::NedToNeuLinear(
          projectairsim::TransformUtils::ToCentimeters(InWrench.force));

  // N*m = (kg*m/s^2)*m = kg*m^2/s^2, NED_m^2 -> NED_cm^2 -> NEU_cm^2
  projectairsim::Vector3 TorqueNEU =
      projectairsim::TransformUtils::NedToNeuAngular(
          projectairsim::TransformUtils::ToCentimeters(
              projectairsim::TransformUtils::ToCentimeters(InWrench.torque)));

  RobotRootLink->GetBodyInstance()->AddForceAtPosition(
      {ForceNEU.x(), ForceNEU.y(), ForceNEU.z()}, FVector::ZeroVector, true,
      true);

  RobotRootLink->GetBodyInstance()->AddTorqueInRadians(
      {TorqueNEU.x(), TorqueNEU.y(), TorqueNEU.z()});

  // TODO Propeller meshes will be rotated manually by
  // ApplyActuatedTransforms(), but this might not work for robot's with Unreal
  // Physics active. Leaving it for now, as Unreal Physics is no longer
  // supported and may be deprecated.
}

void AUnrealRobot::BeginPlay() {
  // Register callback for detected collisions using root mesh's OnComponentHit
  if (RobotRootLink != nullptr) {
    RobotRootLink->SetCollisionHitCallback(
        [this](UPrimitiveComponent* HitComponent, AActor* OtherActor,
               UPrimitiveComponent* OtherComp, FVector NormalImpulse,
               const FHitResult& Hit) {
          OnCollisionHit(HitComponent, OtherActor, OtherComp, NormalImpulse,
                         Hit);
        });
  }

  // Register callbacks for secondary links that want explicit ground collision
  // checks
  for (auto& pair : RobotLinks) {
    if (pair.second->IsGroundCollisionDetectionEnabled()) {
      pair.second->SetCollisionHitCallback(
          [this](UPrimitiveComponent* HitComponent, AActor* OtherActor,
                 UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                 const FHitResult& Hit) {
            OnCollisionHit(HitComponent, OtherActor, OtherComp, NormalImpulse,
                           Hit);
          });
    }
  }

  Super::BeginPlay();

  // Register callback for UnrealPhysicsBody to set wrench on UnrealRobot
  if (SimPhysicsBody != nullptr) {
    SimPhysicsBody->SetCallbackSetExternalWrench(
        [this](const projectairsim::Wrench& InWrench) {
          this->SetExternalWrench(InWrench);
        });
  }

  // Register callback for sim robot to set pose on UnrealRobot
  SimRobot.SetCallbackKinematicsUpdated(
      [this](const projectairsim::Kinematics& Kin, TimeNano Timestamp) {
        this->SetRobotKinematics(Kin, Timestamp);
      });

  // Register callback for sim robot to set actuated rotations on UnrealRobot
  SimRobot.SetCallbackActuatorOutputUpdated(
      [this](const projectairsim::ActuatedTransforms& InActuatedTransforms,
             TimeNano DeltaSimtime) {
        this->SetActuatedTransforms(InActuatedTransforms, DeltaSimtime);
      });
}

void AUnrealRobot::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
  // In case of a tick coming through while it should be paused, just return
  if (UGameplayStatics::IsGamePaused(this)) return;

  // Main conditions by physics type
  if (SimRobot.GetPhysicsType() == projectairsim::PhysicsType::kUnrealPhysics &&
      SimPhysicsBody != nullptr) {
    //-------------------------------------------------------------------------
    // UnrealPhysics

    // Unreal has moved the robot to it's new pose for this tick, so write the
    // new kinematics data to the SimPhysicsBody
    bHasUnrealPoseUpdated = true;

    // Since Unreal is advancing the sim time, SimClock is still one step behind
    // (UnrealScene sets it after the UnrealRobot ticks in TG_PostUpdateWork),
    // so add the current Unreal dt to set the timestamp correctly.
    TimeNano DeltaTimeThisTick = UnrealHelpers::DeltaTimeToNanos(DeltaTime);
    TimeNano LastSimtime = projectairsim::SimClock::Get()->NowSimNanos();
    UnrealPoseUpdatedTimeStamp = LastSimtime + DeltaTimeThisTick;

    // Output UnrealRobot's new kinematics from this tick to UnrealPhysicsBody
    projectairsim::Kinematics NewKin;
    auto RootBody = RobotRootLink->GetBodyInstance();
    auto RootTransform = RootBody->GetUnrealWorldTransform();
    auto RootPos = RootTransform.GetLocation();
    auto RootRot = RootTransform.Rotator();
    auto RootVelLin = RootBody->GetUnrealWorldVelocity();
    auto RootVelAng = RootBody->GetUnrealWorldAngularVelocityInRadians();

    // NEU_cm -> NEU_m -> NED_m
    NewKin.pose.position = projectairsim::TransformUtils::NeuToNedLinear(
        projectairsim::TransformUtils::ToMeters(
            projectairsim::Vector3(RootPos.X, RootPos.Y, RootPos.Z)));

    NewKin.pose.orientation = projectairsim::TransformUtils::ToQuaternion(
        projectairsim::TransformUtils::ToRadians(RootRot.Roll),
        projectairsim::TransformUtils::ToRadians(RootRot.Pitch),
        projectairsim::TransformUtils::ToRadians(RootRot.Yaw));

    // NEU_cm -> NEU_m -> NED_m
    NewKin.twist.linear = projectairsim::TransformUtils::NeuToNedLinear(
        projectairsim::TransformUtils::ToMeters(
            projectairsim::Vector3(RootVelLin.X, RootVelLin.Y, RootVelLin.Z)));

    // NEU -> NED
    NewKin.twist.angular = projectairsim::TransformUtils::NeuToNedAngular(
        projectairsim::Vector3(RootVelAng.X, RootVelAng.Y, RootVelAng.Z));

    // Estimate accels from prev vel and Unreal's DeltaTime tick period (sim
    // clock time is not updated from DeltaTime until after the physics tick has
    // completed so need to use DeltaTime here)
    auto DeltaVelLin = NewKin.twist.linear - RobotKinematics.twist.linear;
    auto DeltaVelAng = NewKin.twist.angular - RobotKinematics.twist.angular;
    NewKin.accels.linear = DeltaVelLin / DeltaTime;
    NewKin.accels.angular = DeltaVelAng / DeltaTime;

    // Write kinematics data to sim with an external timestamp
    SimPhysicsBody->WriteRobotData(NewKin, UnrealPoseUpdatedTimeStamp);
    SetRobotKinematics(NewKin, UnrealPoseUpdatedTimeStamp);
  } else if (SimRobot.GetPhysicsType() ==
             projectairsim::PhysicsType::kNonPhysics) {
    //-------------------------------------------------------------------------
    // NonPhysics (computer vision mode)

    // Attempt to move the robot in case a new target pose was requested
    const bool bUseCollisionSweep =
        RobotRootLink ? RobotRootLink->IsLinkCollisionEnabled() : false;

    MoveRobotToUnrealPose(bUseCollisionSweep);

    // Force setting the pose updated flag/timestamp to keep the sensors
    // publishing their data on every tick even if no new target pose was
    // requested
    bHasUnrealPoseUpdated = true;
    UnrealPoseUpdatedTimeStamp = projectairsim::SimClock::Get()->NowSimNanos();
  } else {
    //-------------------------------------------------------------------------
    // Other than UnrealPhysics/NonPhysics (ex. FastPhysics)

    // Move the UnrealRobot to its target pose set by sim
    const bool bUseCollisionSweep =
        RobotRootLink ? RobotRootLink->IsLinkCollisionEnabled() : false;

    MoveRobotToUnrealPose(bUseCollisionSweep);
  }  // end conditions by physics type

  ApplyActuatedTransforms();

  // For all physics types, set a flag and pose timestamp on the sensors to
  // synchronize their updates with the robot's pose
  if (bHasUnrealPoseUpdated) {
    for (auto [Id, Sensor] : RobotSensors) {
      if (Sensor) {
        Sensor->SetHasNewState(true);
        Sensor->SetSimTimeAtPoseUpdate(UnrealPoseUpdatedTimeStamp);
      }
    }
  }

  bHasUnrealPoseUpdated = false;  // done processing pose update, clear flag
}

// Sets the viewport view for this actor which is used when this is the active
// view target by setting the OutResult view info values.
void AUnrealRobot::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) {
  USceneCaptureComponent2D* Capture = GetActiveStreamingCapture();

  if (bFindCameraComponentWhenViewTarget == false || Capture == nullptr) {
    // Bail out with setting the view to the base actor pose.
    GetActorEyesViewPoint(OutResult.Location, OutResult.Rotation);
    return;
  }

  // Set OutResult to the camera view of the capture component, including
  // and post-process materials like for depth/segmentation cameras.
  Capture->GetCameraView(DeltaTime, OutResult);

  if (Capture->PostProcessBlendWeight > 0.0f) {
    OutResult.PostProcessSettings = Capture->PostProcessSettings;
  }
}

void AUnrealRobot::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);
}