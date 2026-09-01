// Copyright (C) Microsoft Corporation.  
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.
// The Unreal robot implementation.

#include "UnrealRobot.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/Pawn.h"
#include "ProjectAirSimVehicleActorBase.h"
#include "IProjectAirSimVehicle.h"
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

namespace {

constexpr float kGroundClampTraceDistanceCm = 1000000.0f;
constexpr float kGroundClampOffsetCm = 1.0f;

bool TryGetGroundZAtXY(UWorld* World, AActor* IgnoredActor,
                       const FVector& Location, float& OutGroundZ) {
  if (World == nullptr) return false;

  const FVector StartTrace(Location.X, Location.Y,
                           Location.Z + kGroundClampTraceDistanceCm);
  const FVector EndTrace(Location.X, Location.Y,
                         Location.Z - kGroundClampTraceDistanceCm);

  FCollisionQueryParams TraceParams;
  TraceParams.bTraceComplex = true;
  TraceParams.bReturnPhysicalMaterial = false;
  if (IgnoredActor != nullptr) {
    TraceParams.AddIgnoredActor(IgnoredActor);
  }

  FHitResult HitResult(ForceInit);
  bool bWasHit = World->LineTraceSingleByChannel(
      HitResult, StartTrace, EndTrace, ECC_Visibility, TraceParams,
      FCollisionResponseParams::DefaultResponseParam);
  if (!bWasHit) {
    bWasHit = World->LineTraceSingleByChannel(
        HitResult, StartTrace, EndTrace, ECC_WorldStatic, TraceParams,
        FCollisionResponseParams::DefaultResponseParam);
  }

  if (!bWasHit || !HitResult.bBlockingHit) return false;

  OutGroundZ = HitResult.ImpactPoint.Z;
  return std::isfinite(OutGroundZ);
}

float GetMeshBoundsMinWorldZ(UStaticMeshComponent* Component,
                             const FVector& Location,
                             const FRotator& Rotation) {
  if (Component == nullptr || Component->GetStaticMesh() == nullptr) {
    return Location.Z;
  }

  FVector MinBounds;
  FVector MaxBounds;
  Component->GetLocalBounds(MinBounds, MaxBounds);

  const FTransform TargetTransform(Rotation.Quaternion(), Location,
                                   Component->GetComponentScale());
  const std::array<FVector, 8> Corners = {
      FVector(MinBounds.X, MinBounds.Y, MinBounds.Z),
      FVector(MinBounds.X, MinBounds.Y, MaxBounds.Z),
      FVector(MinBounds.X, MaxBounds.Y, MinBounds.Z),
      FVector(MinBounds.X, MaxBounds.Y, MaxBounds.Z),
      FVector(MaxBounds.X, MinBounds.Y, MinBounds.Z),
      FVector(MaxBounds.X, MinBounds.Y, MaxBounds.Z),
      FVector(MaxBounds.X, MaxBounds.Y, MinBounds.Z),
      FVector(MaxBounds.X, MaxBounds.Y, MaxBounds.Z),
  };

  float MinWorldZ = std::numeric_limits<float>::max();
  for (const FVector& Corner : Corners) {
    const float CornerWorldZ =
        static_cast<float>(TargetTransform.TransformPosition(Corner).Z);
    MinWorldZ = std::min(MinWorldZ, CornerWorldZ);
  }

  return MinWorldZ;
}

FVector ClampRootMeshToGround(UUnrealRobotLink* RootLink,
                              const FVector& Location,
                              const FRotator& Rotation) {
  float GroundZ = 0.0f;
  if (RootLink == nullptr ||
      !TryGetGroundZAtXY(RootLink->GetWorld(), RootLink->GetOwner(), Location,
                         GroundZ)) {
    return Location;
  }

  const float MinWorldZ = GetMeshBoundsMinWorldZ(RootLink, Location, Rotation);
  const float MinAllowedZ = GroundZ + kGroundClampOffsetCm;
  if (MinWorldZ >= MinAllowedZ) {
    return Location;
  }

  FVector ClampedLocation = Location;
  ClampedLocation.Z += MinAllowedZ - MinWorldZ;
  return ClampedLocation;
}

}  // namespace

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

  bool bIsProjectAirSimVehicle =
      (InSimRobot.GetPhysicsType() ==
       projectairsim::PhysicsType::kUnrealPhysics) &&
      !InSimRobot.GetUnrealVehicleClass().empty();

  bool bWithUnrealPhysics =
      (InSimRobot.GetPhysicsType() ==
       projectairsim::PhysicsType::kUnrealPhysics) &&
      !bIsProjectAirSimVehicle;

  if (bWithUnrealPhysics || bIsProjectAirSimVehicle) {
    // For Unreal-calculated or ProjectAirSim vehicle physics, do the updates after
    // Unreal has completed the physics tick calculations.
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

  if (bIsProjectAirSimVehicle) {
    // ProjectAirSim vehicle physics: initialize the root component and
    // find/spawn the vehicle BEFORE sensors so sensors attach to the correct
    // root.
    InitializeProjectAirSimVehicle();
  }

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

void AUnrealRobot::InitializeProjectAirSimVehicle() {
  // Create a minimal invisible root component for sensor attachment
  auto* SceneRoot =
      NewObject<USceneComponent>(this, TEXT("ProjectAirSimVehicleRoot"));
  SceneRoot->SetMobility(EComponentMobility::Movable);
  SceneRoot->RegisterComponent();
  RootComponent = SceneRoot;
  RobotRootLink = nullptr;  // No physics link for ProjectAirSim vehicle

  UWorld* World = GetWorld();
  if (World == nullptr) return;

  // Compute the spawn transform from the robot's initial kinematics (NED_m).
  // Convert NED_m → NEU_cm for Unreal world coordinates.
  const auto& InitKin = SimRobot.GetKinematics();
  const FVector SpawnLoc =
      UnrealHelpers::ToFVector(projectairsim::TransformUtils::NedToNeuLinear(
          projectairsim::TransformUtils::ToCentimeters(
              InitKin.pose.position)));
  const FRotator SpawnRot =
      UnrealHelpers::ToFRotator(InitKin.pose.orientation);
  FTransform SpawnTransform(SpawnRot, SpawnLoc);

  FString RobotName = FString(SimRobot.GetID().c_str());
  std::string ProjectAirSimVehicleClassPath = SimRobot.GetUnrealVehicleClass();

  if (!ProjectAirSimVehicleClassPath.empty()) {
    // Spawn or find by class path from config (e.g. Blueprint class path)
    FString ClassPath = FString(ProjectAirSimVehicleClassPath.c_str());
    UClass* ActorClass = LoadClass<AActor>(nullptr, *ClassPath);
    if (ActorClass != nullptr) {
      // Each configured robot owns a distinct vehicle actor. Reusing the first
      // actor of this class makes two robots with the same Blueprint control
      // the same car and ignores the second robot's spawn transform.
      FActorSpawnParameters SpawnParams;
      SpawnParams.SpawnCollisionHandlingOverride =
          ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
      ProjectAirSimVehicleActor =
          World->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);
      if (ProjectAirSimVehicleActor != nullptr) {
        UnrealLogger::Log(
            projectairsim::LogLevel::kTrace,
            TEXT("[%s] Spawned ProjectAirSim vehicle of class %s at (%.1f, %.1f, %.1f)"),
            *RobotName, *ClassPath, SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z);
      } else {
        UnrealLogger::Log(
            projectairsim::LogLevel::kError,
            TEXT("[%s] Failed to spawn ProjectAirSim vehicle of class %s"),
            *RobotName, *ClassPath);
      }
      // Check if the actor implements the extended interface
      if (ProjectAirSimVehicleActor != nullptr) {
        bProjectAirSimVehicleHasInterface = ProjectAirSimVehicleActor->GetClass()->
            ImplementsInterface(UProjectAirSimVehicle::StaticClass());
        if (!bProjectAirSimVehicleHasInterface) {
          UnrealLogger::Log(
              projectairsim::LogLevel::kWarning,
              TEXT("[%s] ProjectAirSim vehicle %s does not implement "
                   "IProjectAirSimVehicle. Kinematics will use standard UE "
                   "API (GetVelocity). Actuator forwarding disabled."),
              *RobotName, *ProjectAirSimVehicleActor->GetName());
        } else if (!bProjectAirSimVehicleParameterServiceRegistered) {
          auto set_parameter =
              projectairsim::ServiceMethod("SetParameter", {"index", "value"});
          auto set_parameter_handler = set_parameter.CreateMethodHandler(
              &AUnrealRobot::SetParameter, *this);
          SimRobot.RegisterServiceMethod(set_parameter, set_parameter_handler);
          bProjectAirSimVehicleParameterServiceRegistered = true;
        }
      }
    } else {
      UnrealLogger::Log(
          projectairsim::LogLevel::kError,
          TEXT("[%s] Could not load ProjectAirSim vehicle class: %s"),
          *RobotName, *ClassPath);
    }
  } else {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[%s] Missing required 'unreal-vehicle-class' in robot config. "
             "Automatic actor discovery is disabled."),
        *RobotName);
  }

  if (ProjectAirSimVehicleActor == nullptr) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kWarning,
        TEXT("[%s] No ProjectAirSim vehicle found or spawned. "
             "ProjectAirSim vehicle physics will not function."),
        *RobotName);
  } else if (APawn* VehiclePawn = Cast<APawn>(ProjectAirSimVehicleActor);
             VehiclePawn != nullptr && VehiclePawn->GetController() == nullptr) {
    // Chaos vehicle input can be cleared when a spawned Pawn has no controller.
    // Match the behavior expected by placed vehicle Blueprints and ensure the
    // movement component can consume SetThrottle/Brake/SteeringInput values.
    VehiclePawn->SpawnDefaultController();
    UnrealLogger::Log(
        VehiclePawn->GetController() != nullptr
            ? projectairsim::LogLevel::kTrace
            : projectairsim::LogLevel::kWarning,
        TEXT("[%s] Unreal vehicle pawn default controller: %s"), *RobotName,
        VehiclePawn->GetController() != nullptr ? TEXT("spawned")
                                                : TEXT("unavailable"));
  }

  if (ProjectAirSimVehicleActor != nullptr) {
    ProjectAirSimVehicleMovement =
        ProjectAirSimVehicleActor
            ->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
    UnrealLogger::Log(
        ProjectAirSimVehicleMovement != nullptr
            ? projectairsim::LogLevel::kTrace
            : projectairsim::LogLevel::kWarning,
        TEXT("[%s] Chaos wheeled vehicle movement component: %s"), *RobotName,
        ProjectAirSimVehicleMovement != nullptr ? TEXT("found")
                                                : TEXT("not found"));
  }

  const auto ResetProjectAirSimVehiclePose =
      [this, &RobotName, &SpawnLoc, &SpawnRot](UPrimitiveComponent* PhysicsComponent) {
        PrevExtPosition = SpawnLoc;
        PrevExtQuat = SpawnRot.Quaternion();
        PrevEstLinearVelocity = FVector::ZeroVector;
        PrevEstAngularVelocity = FVector::ZeroVector;
        bHasPrevExtState = false;

        if (ProjectAirSimVehicleActor == nullptr) return;

        // Always teleport using the config's initial position (source of truth).
        ProjectAirSimVehicleActor->SetActorLocationAndRotation(
            SpawnLoc, SpawnRot, false, nullptr, ETeleportType::TeleportPhysics);

        if (PhysicsComponent != nullptr) {
          PhysicsComponent->SetWorldLocationAndRotation(
              SpawnLoc, SpawnRot, false, nullptr, ETeleportType::TeleportPhysics);

          if (PhysicsComponent->IsSimulatingPhysics()) {
            PhysicsComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
            PhysicsComponent->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
            PhysicsComponent->SetAllPhysicsPosition(SpawnLoc);
            PhysicsComponent->SetAllPhysicsRotation(SpawnRot.Quaternion());
          }
        }

        // Notify the actor so Blueprint subclasses can reset their own state
        // (AI variables, throttle values, animation state, etc.).
        if (bProjectAirSimVehicleHasInterface) {
          IProjectAirSimVehicle::Execute_ResetToSpawnPose(ProjectAirSimVehicleActor);
        }

        UnrealLogger::Log(
            projectairsim::LogLevel::kTrace,
            TEXT("[%s] Reset ProjectAirSim vehicle '%s' to initial pose (%.1f, %.1f, %.1f)"),
            *RobotName, *ProjectAirSimVehicleActor->GetName(), SpawnLoc.X, SpawnLoc.Y,
            SpawnLoc.Z);
      };

  // Position this actor at the ProjectAirSim vehicle's location so sensors
  // start at the right place.  We sync every tick in TickProjectAirSimVehicle()
  // because UE attachment does not propagate to/from physics-simulated actors.
  if (ProjectAirSimVehicleActor != nullptr) {
    // Find the first UPrimitiveComponent on the ProjectAirSim vehicle.
    // At init time physics may not be active yet, so we accept ANY
    // UPrimitiveComponent (preferring one with a body instance).
    // We will re-check during tick if needed.
    TInlineComponentArray<UPrimitiveComponent*> PrimComps;
    ProjectAirSimVehicleActor->GetComponents<UPrimitiveComponent>(PrimComps);

    UnrealLogger::Log(
        projectairsim::LogLevel::kWarning,
        TEXT("[%s] ProjectAirSim vehicle has %d primitive components"),
        *RobotName, PrimComps.Num());

    for (UPrimitiveComponent* PC : PrimComps) {
      if (PC != nullptr) {
        UnrealLogger::Log(
            projectairsim::LogLevel::kWarning,
            TEXT("[%s]   Component: %s  Class: %s  SimPhysics: %s"),
            *RobotName, *PC->GetName(),
            *PC->GetClass()->GetName(),
            PC->IsSimulatingPhysics() ? TEXT("YES") : TEXT("NO"));
        // Take the first one we find (prefer one already simulating)
        if (ProjectAirSimVehicleComponent == nullptr || PC->IsSimulatingPhysics()) {
          ProjectAirSimVehicleComponent = PC;
          if (PC->IsSimulatingPhysics()) break;
        }
      }
    }

    if (ProjectAirSimVehicleComponent != nullptr) {
      UnrealLogger::Log(
          projectairsim::LogLevel::kWarning,
          TEXT("[%s] Using physics component: %s"),
          *RobotName, *ProjectAirSimVehicleComponent->GetName());
    } else {
      UnrealLogger::Log(
          projectairsim::LogLevel::kError,
          TEXT("[%s] No UPrimitiveComponent found on ProjectAirSim vehicle!"),
          *RobotName);
    }

            ResetProjectAirSimVehiclePose(ProjectAirSimVehicleComponent);

    // Initial position sync
    FVector ExtLoc;
    FRotator ExtRot;
    if (ProjectAirSimVehicleComponent != nullptr) {
      ExtLoc = ProjectAirSimVehicleComponent->GetComponentLocation();
      ExtRot = ProjectAirSimVehicleComponent->GetComponentRotation();
    } else {
      ExtLoc = ProjectAirSimVehicleActor->GetActorLocation();
      ExtRot = ProjectAirSimVehicleActor->GetActorRotation();
    }
    this->SetActorLocationAndRotation(ExtLoc, ExtRot, false, nullptr,
                                      ETeleportType::TeleportPhysics);
  }
}

bool AUnrealRobot::SetParameter(int32 Index, float Value) {
  if (Index < 0) return false;

  FScopeLock ScopeLock(&UpdateMutex);
  ProjectAirSimVehicleParameters.Add(Index, Value);
  return true;
}

void AUnrealRobot::TickProjectAirSimVehicle(float DeltaTime) {
  if (ProjectAirSimVehicleActor == nullptr) return;

  // Forward the indexed value through the single Unreal vehicle parameter
  // contract. The target Blueprint defines the meaning of each index.
  const auto DispatchParameterSignal = [this](int32 Index, float Value) {
    IProjectAirSimVehicle::Execute_SetParameterSignal(
        ProjectAirSimVehicleActor, Index, Value);
  };

  // Apply the standard vehicle behavior for the separate SimpleDrive workflow.
  const auto ApplyStandardVehicleForces =
      [this](float Throttle, float Brake, float Steering) {
        // Feed Chaos first so its wheel steering, drivetrain, and animation
        // receive the same standard controls as the Blueprint event.
        if (ProjectAirSimVehicleMovement != nullptr) {
          ProjectAirSimVehicleMovement->SetThrottleInput(Throttle);
          ProjectAirSimVehicleMovement->SetBrakeInput(Brake);
          ProjectAirSimVehicleMovement->SetSteeringInput(Steering);
        }

        if (ProjectAirSimVehicleComponent == nullptr ||
            !ProjectAirSimVehicleComponent->IsSimulatingPhysics()) {
          return;
        }

        if (!FMath::IsNearlyZero(Throttle)) {
          ProjectAirSimVehicleComponent->AddForce(
              ProjectAirSimVehicleComponent->GetForwardVector() * Throttle *
                  600.f,
              NAME_None, /*bAcceleration=*/true);
        }
        if (!FMath::IsNearlyZero(Brake)) {
          const FVector Velocity =
              ProjectAirSimVehicleComponent->GetPhysicsLinearVelocity();
          const FVector PlanarVelocity(Velocity.X, Velocity.Y, 0.f);
          if (!PlanarVelocity.IsNearlyZero()) {
            ProjectAirSimVehicleComponent->AddForce(
                -PlanarVelocity.GetSafeNormal() * Brake * 900.f, NAME_None,
                /*bAcceleration=*/true);
          }
        }
        if (!FMath::IsNearlyZero(Steering)) {
          ProjectAirSimVehicleComponent->AddTorqueInRadians(
              FVector(0.f, 0.f,
                      Steering * FMath::DegreesToRadians(180.f)),
              NAME_None, /*bAcceleration=*/true);
        }
      };

  // Lazy re-check: if we found a component at init but it wasn't simulating
  // physics yet, check again now that the game is running.
  if (ProjectAirSimVehicleComponent != nullptr &&
      !ProjectAirSimVehicleComponent->IsSimulatingPhysics()) {
    // Search again for a simulating component
    TInlineComponentArray<UPrimitiveComponent*> PrimComps;
    ProjectAirSimVehicleActor->GetComponents<UPrimitiveComponent>(PrimComps);
    for (UPrimitiveComponent* PC : PrimComps) {
      if (PC != nullptr && PC->IsSimulatingPhysics()) {
        ProjectAirSimVehicleComponent = PC;
        break;
      }
    }
  }

  // Forward indexed parameter values to the ProjectAirSim vehicle.
  if (bProjectAirSimVehicleHasInterface) {
    TMap<int32, float> Parameters;
    {
      FScopeLock ScopeLock(&UpdateMutex);
      Parameters = ProjectAirSimVehicleParameters;
    }
    for (const auto& Parameter : Parameters) {
      DispatchParameterSignal(Parameter.Key, Parameter.Value);
    }

    // Apply the standard indexed vehicle controls: throttle=0, brake=1,
    // steering=2. The vehicle actor receives every indexed value above; these
    // standard channels must also reach Chaos (and the force-based fallback)
    // when the actor does not consume SetActuatorSignal itself.
    ApplyStandardVehicleForces(Parameters.FindRef(0), Parameters.FindRef(1),
                               Parameters.FindRef(2));
  }

  // A configured controller is a separate workflow, such as SimpleDrive.
  auto* Controller = SimRobot.GetController();
  if (Controller != nullptr &&
      SimRobot.GetControllerType() == "simple-drive-api") {
    std::vector<float> Signals;
    Signals = Controller->GetControlSignals("");

    float Throttle = Signals.size() > 0 ? Signals[0] : 0.f;
    float Steering = Signals.size() > 1 ? Signals[1] : 0.f;
    float Brake = Signals.size() > 2 ? Signals[2] : 0.f;

    // The SimpleDrive controller uses the conventional vehicle signal order.
    if (bProjectAirSimVehicleHasInterface) {
      DispatchParameterSignal(0, Throttle);
      DispatchParameterSignal(1, Brake);
      DispatchParameterSignal(2, Steering);
    }

    ApplyStandardVehicleForces(Throttle, Brake, Steering);
  }

  // Read kinematics from the ProjectAirSim vehicle.
  // Use the physics component's transform (not GetActorLocation) because in
  // many Blueprints the root is a static DefaultSceneRoot while the mesh
  // that simulates physics is a child component that moves independently.
  bHasUnrealPoseUpdated = true;

  TimeNano DeltaTimeThisTick = UnrealHelpers::DeltaTimeToNanos(DeltaTime);
  TimeNano LastSimtime = projectairsim::SimClock::Get()->NowSimNanos();
  UnrealPoseUpdatedTimeStamp = LastSimtime + DeltaTimeThisTick;

  projectairsim::Kinematics NewKin;

  FVector ActorPos;
  FQuat   ActorQuat;
  if (ProjectAirSimVehicleComponent != nullptr &&
      ProjectAirSimVehicleComponent->IsSimulatingPhysics()) {
    // Prefer the physics component directly — the most reliable source when
    // the actor root is a non-physics DefaultSceneRoot (common in Blueprints).
    // This also handles actors that inherit AProjectAirSimVehicleActorBase correctly
    // since their root IS the physics mesh.
    ActorPos  = ProjectAirSimVehicleComponent->GetComponentLocation();
    ActorQuat = ProjectAirSimVehicleComponent->GetComponentQuat();
  } else if (bProjectAirSimVehicleHasInterface) {
    ActorPos  = IProjectAirSimVehicle::Execute_GetPosition(ProjectAirSimVehicleActor);
    ActorQuat = IProjectAirSimVehicle::Execute_GetRotation(ProjectAirSimVehicleActor);
  } else if (ProjectAirSimVehicleComponent != nullptr) {
    ActorPos  = ProjectAirSimVehicleComponent->GetComponentLocation();
    ActorQuat = ProjectAirSimVehicleComponent->GetComponentQuat();
  } else {
    ActorPos  = ProjectAirSimVehicleActor->GetActorLocation();
    ActorQuat = ProjectAirSimVehicleActor->GetActorQuat();
  }

  // NEU_cm -> NEU_m -> NED_m
  NewKin.pose.position = projectairsim::TransformUtils::NeuToNedLinear(
      projectairsim::TransformUtils::ToMeters(
          projectairsim::Vector3(ActorPos.X, ActorPos.Y, ActorPos.Z)));

  NewKin.pose.orientation = projectairsim::Quaternion(
      ActorQuat.W, ActorQuat.X, ActorQuat.Y, ActorQuat.Z);

  FVector VelLin, VelAng, AccLin, AccAng;
  AccLin = FVector::ZeroVector;
  AccAng = FVector::ZeroVector;

  // Strategy: read velocity from the best available source.
  //  1) If the interface provides non-zero velocity, use it (explicit override).
  //  2) Else if physics is simulating, read directly from the engine.
  //  3) Else estimate from finite differences.
  bool bGotVelocity = false;

  if (bProjectAirSimVehicleHasInterface) {
    VelLin = IProjectAirSimVehicle::Execute_GetLinearVelocity(ProjectAirSimVehicleActor);
    VelAng = IProjectAirSimVehicle::Execute_GetAngularVelocity(ProjectAirSimVehicleActor);
    if (!VelLin.IsNearlyZero() || !VelAng.IsNearlyZero()) {
      // Blueprint provided an explicit velocity override
      AccLin = IProjectAirSimVehicle::Execute_GetLinearAcceleration(ProjectAirSimVehicleActor);
      AccAng = IProjectAirSimVehicle::Execute_GetAngularAcceleration(ProjectAirSimVehicleActor);
      bGotVelocity = true;
    }
  }

  if (!bGotVelocity && ProjectAirSimVehicleComponent != nullptr &&
      ProjectAirSimVehicleComponent->IsSimulatingPhysics()) {
    // Read directly from the physics engine (Chaos / PhysX).
    VelLin = ProjectAirSimVehicleComponent->GetPhysicsLinearVelocity();  // cm/s
    VelAng = ProjectAirSimVehicleComponent->GetPhysicsAngularVelocityInRadians();
    bGotVelocity = true;
  }

  if (!bGotVelocity) {
    // Fallback: finite differences for kinematic / Blueprint-driven actors.
    if (bHasPrevExtState && DeltaTime > 0.0f) {
      VelLin = (ActorPos - PrevExtPosition) / DeltaTime;  // cm/s

    // Angular velocity from quaternion finite difference
    FQuat DeltaQuat = ActorQuat * PrevExtQuat.Inverse();
    DeltaQuat.Normalize();
    FVector Axis;
    float AngleRad;
    DeltaQuat.ToAxisAndAngle(Axis, AngleRad);
    VelAng = Axis * (AngleRad / DeltaTime);

      AccLin = (VelLin - PrevEstLinearVelocity) / DeltaTime;
      AccAng = (VelAng - PrevEstAngularVelocity) / DeltaTime;
    } else {
      VelLin = FVector::ZeroVector;
      VelAng = FVector::ZeroVector;
    }
  }

  // Store current state for next-tick finite differences
  PrevExtPosition = ActorPos;
  PrevExtQuat     = ActorQuat;
  PrevEstLinearVelocity = VelLin;
  PrevEstAngularVelocity = VelAng;
  bHasPrevExtState = true;

  // Debug: log raw values to diagnose which branch was taken
  static int VelDebugCounter = 0;
  if (++VelDebugCounter % 60 == 0) {
    bool bSimPhys = ProjectAirSimVehicleComponent != nullptr &&
                    ProjectAirSimVehicleComponent->IsSimulatingPhysics();
    UnrealLogger::Log(
        projectairsim::LogLevel::kWarning,
        TEXT("[ExtActorVel] HasInterface=%d  PhysComp=%s  SimPhys=%d  "
             "Pos=(%.1f,%.1f,%.1f)  VelLin=(%.1f,%.1f,%.1f)  dt=%.4f"),
        bProjectAirSimVehicleHasInterface ? 1 : 0,
        ProjectAirSimVehicleComponent ? *ProjectAirSimVehicleComponent->GetName() : TEXT("NULL"),
        bSimPhys ? 1 : 0,
        ActorPos.X, ActorPos.Y, ActorPos.Z,
        VelLin.X, VelLin.Y, VelLin.Z,
        DeltaTime);
  }

  // NEU_cm/s -> NEU_m/s -> NED_m/s
  NewKin.twist.linear = projectairsim::TransformUtils::NeuToNedLinear(
      projectairsim::TransformUtils::ToMeters(
          projectairsim::Vector3(VelLin.X, VelLin.Y, VelLin.Z)));

  // NEU -> NED
  NewKin.twist.angular = projectairsim::TransformUtils::NeuToNedAngular(
      projectairsim::Vector3(VelAng.X, VelAng.Y, VelAng.Z));

  // If interface provided accelerations, use them; otherwise estimate from deltas
  if (bProjectAirSimVehicleHasInterface && !AccLin.IsNearlyZero()) {
    NewKin.accels.linear = projectairsim::TransformUtils::NeuToNedLinear(
        projectairsim::TransformUtils::ToMeters(
            projectairsim::Vector3(AccLin.X, AccLin.Y, AccLin.Z)));
    NewKin.accels.angular = projectairsim::TransformUtils::NeuToNedAngular(
        projectairsim::Vector3(AccAng.X, AccAng.Y, AccAng.Z));
  } else if (DeltaTime > 0.0f) {
    auto DeltaVelLin = NewKin.twist.linear - RobotKinematics.twist.linear;
    auto DeltaVelAng = NewKin.twist.angular - RobotKinematics.twist.angular;
    NewKin.accels.linear = DeltaVelLin / DeltaTime;
    NewKin.accels.angular = DeltaVelAng / DeltaTime;
  }

  // Update kinematics — both on the local AUnrealRobot copy AND on the
  // sim Robot so the controller's GetKinematics reads fresh data.
  SetRobotKinematics(NewKin, UnrealPoseUpdatedTimeStamp);
  SimRobot.UpdateKinematics(NewKin, UnrealPoseUpdatedTimeStamp);

  // Move this entire actor (and all attached sensor components) to follow
  // the ProjectAirSim vehicle.  Standard UE attachment does not work when the
  // target actor is physics-simulated, so we teleport every tick.
  this->SetActorLocationAndRotation(ActorPos, ActorQuat, false, nullptr,
                                    ETeleportType::TeleportPhysics);
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

          // External-actor robots may have no links/root yet at this point.
          // Ensure sensor creation always has a valid UObject outer.
          if (Parent == nullptr) {
            USceneComponent* AutoRoot =
                NewObject<USceneComponent>(this, TEXT("ProjectAirSimVehicleSensorRoot"));
            if (AutoRoot != nullptr) {
              AutoRoot->RegisterComponent();
              SetRootComponent(AutoRoot);
              Parent = AutoRoot;
              UnrealLogger::Log(
                  projectairsim::LogLevel::kWarning,
                  TEXT("[%s] Root component was null during sensor init; "
                       "created ProjectAirSimVehicleSensorRoot."),
                  *GetName());
            }
          }

          if (Parent == nullptr) {
            UnrealLogger::Log(
                projectairsim::LogLevel::kError,
                TEXT("[%s] Failed to create sensor '%hs': null parent component."),
                *GetName(), CurSensor.get().GetId().c_str());
            return;
          }

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
  FVector TgtLocNEU =
      UnrealHelpers::ToFVector(projectairsim::TransformUtils::NedToNeuLinear(
          projectairsim::TransformUtils::ToCentimeters(TgtPose.position)));
  const FRotator TgtRot = UnrealHelpers::ToFRotator(TgtPose.orientation);

  if (SimRobot.GetPhysicsType() == projectairsim::PhysicsType::kJSBSimPhysics) {
    TgtLocNEU = ClampRootMeshToGround(RobotRootLink, TgtLocNEU, TgtRot);
  }

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

void AUnrealRobot::UpdateCachedTerrainElevation() {
  if (SimRobot.GetPhysicsType() !=
      projectairsim::PhysicsType::kJSBSimPhysics) {
    return;
  }

  if (SimRobot.GetJSBSimGroundSettings().mode ==
      projectairsim::JSBSimGroundMode::kConstant) {
    return;
  }

  const auto terrain_cb = SimRobot.GetTerrainElevationCallback();
  if (terrain_cb == nullptr) {
    return;
  }

  const auto& position = SimRobot.GetKinematics().pose.position;
  const auto terrain_asl_m = terrain_cb(static_cast<double>(position.x()),
                                      static_cast<double>(position.y()));
  if (std::isfinite(terrain_asl_m)) {
    SimRobot.SetCachedTerrainElevationASL(terrain_asl_m);
  }
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

  UpdateCachedTerrainElevation();
}

void AUnrealRobot::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
  // In case of a tick coming through while it should be paused, just return
  if (UGameplayStatics::IsGamePaused(this)) return;

  UpdateCachedTerrainElevation();

  // Main conditions by physics type
  if (SimRobot.GetPhysicsType() == projectairsim::PhysicsType::kUnrealPhysics &&
      !SimRobot.GetUnrealVehicleClass().empty()) {
    //-------------------------------------------------------------------------
    // ProjectAirSimVehicle

    // Read kinematics from the ProjectAirSim vehicle and forward actuator signals
    TickProjectAirSimVehicle(DeltaTime);
  } else if (SimRobot.GetPhysicsType() ==
             projectairsim::PhysicsType::kUnrealPhysics &&
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

  // Non-physics drones are moved directly with SetPose and therefore have no
  // actuator output to animate their propeller links. Spin the standard
  // quadrotor Prop_* meshes visually while preserving their fixed offsets.
  if (SimRobot.GetPhysicsType() == projectairsim::PhysicsType::kNonPhysics) {
    constexpr float PropellerDegreesPerSecond = 2160.0f;
    for (const auto& [LinkId, Link] : RobotLinks) {
      if (Link == nullptr || LinkId.rfind("Prop_", 0) != 0) continue;
      const bool bClockwise = LinkId == "Prop_FR" || LinkId == "Prop_RL";
      const float Direction = bClockwise ? -1.0f : 1.0f;
      Link->AddLocalRotation(
          FRotator(0.0f, Direction * PropellerDegreesPerSecond * DeltaTime,
                   0.0f));
    }
  }

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
  // Destroy the ProjectAirSim vehicle actor so it doesn't persist across scene
  // reloads. This covers both actors that were found in the world and those
  // that were spawned by InitializeProjectAirSimVehicle().
  if (ProjectAirSimVehicleActor != nullptr && IsValid(ProjectAirSimVehicleActor)) {
    ProjectAirSimVehicleActor->Destroy();
    ProjectAirSimVehicleActor = nullptr;
  }
  ProjectAirSimVehicleComponent = nullptr;

  Super::EndPlay(EndPlayReason);
}
