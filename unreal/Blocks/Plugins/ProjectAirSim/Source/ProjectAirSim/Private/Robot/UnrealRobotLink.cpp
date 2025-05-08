// Copyright (C) Microsoft Corporation.  All rights reserved.
// The Unreal robot link implementation.

#include "UnrealRobotLink.h"

#include <limits>

#include "Engine/StaticMesh.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "ProjectAirSim.h"
#include "UnrealLogger.h"
#include "core_sim/link/geometry/unreal_mesh.hpp"
#include "core_sim/math_utils.hpp"

namespace projectairsim = microsoft::projectairsim;

UUnrealRobotLink::UUnrealRobotLink(const FObjectInitializer& ObjectInitializer)
    : UStaticMeshComponent(ObjectInitializer) {
  bAutoActivate = true;
  bAllowReregistration = true;  // needed to override UE's calculated inertia

  // Tick in PrePhysics to wake on every tick in case of using Unreal physics
  PrimaryComponentTick.TickGroup = TG_PrePhysics;
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;

  custom_material =
      CreateDefaultSubobject<UPhysicalMaterial>(TEXT("CustomPhysMaterial"));
}

void UUnrealRobotLink::Initialize(const projectairsim::Link& Link,
                                  bool with_unreal_physics) {
  // TODO Handle mesh types besides UnrealMesh
  auto mesh = static_cast<const projectairsim::UnrealMesh*>(
      Link.GetVisual().GetGeometry());

  // Initialize the mesh
  InitializeMesh(mesh);

  // Set initial relative transform according to pose and scaling
  {
    auto fvector_scale_local_cur = GetRelativeScale3D();
    auto fvector_scale_world_new =
        (mesh == nullptr) ? microsoft::projectairsim::Vector3(1.0f, 1.0f, 1.0f)
                          : mesh->GetScale();  // Get desired world scaling
    auto fvector_scale_world_cur = GetComponentScale();  // Get current world
                                                         // scaling
    float scale_x_local =
        microsoft::projectairsim::MathUtils::IsApproximatelyZero(
            fvector_scale_world_cur.X)
            ? std::numeric_limits<float>::max()
            : fvector_scale_world_new.x() / fvector_scale_world_cur.X *
                  fvector_scale_local_cur.X;
    float scale_y_local =
        microsoft::projectairsim::MathUtils::IsApproximatelyZero(
            fvector_scale_world_cur.Y)
            ? std::numeric_limits<float>::max()
            : fvector_scale_world_new.y() / fvector_scale_world_cur.Y *
                  fvector_scale_local_cur.Y;
    float scale_z_local =
        microsoft::projectairsim::MathUtils::IsApproximatelyZero(
            fvector_scale_world_cur.Z)
            ? std::numeric_limits<float>::max()
            : fvector_scale_world_new.z() / fvector_scale_world_cur.Z *
                  fvector_scale_local_cur.Z;

    InitializePose(Link.GetVisual().GetOrigin(),
                   FVector(scale_x_local, scale_y_local, scale_z_local));
  }

  if (with_unreal_physics) {
    InitializePhysics(Link.GetInertial(), Link.GetCollision());
  }

  link = Link;

  RegisterComponent();
}

void UUnrealRobotLink::InitializeMesh(const projectairsim::UnrealMesh* Mesh) {
  if (Mesh == nullptr) {
    return;
  }

  auto meshPath = FString(Mesh->GetName().c_str());
  auto static_mesh = LoadObject<UStaticMesh>(nullptr, *meshPath);
  if (static_mesh == nullptr) {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("Failed to load mesh '%s'"), *meshPath);
    return;
  }

  SetStaticMesh(static_mesh);
}

void UUnrealRobotLink::InitializePose(const projectairsim::Transform& Pose,
                                      const FVector& Scale_Initial) {
  auto Translation_Initial =
      UnrealHelpers::ToFVectorTranslation(Pose.translation_);
  auto Rotation_Initial = UnrealHelpers::ToFQuat(
      Pose.rotation_
          .conjugate());  // Need to conjugate because pose is in NED (RH
                          // coordinates) while UE coordinates are LH

  Transform_Initial_Translation = FTransform(Translation_Initial);
  Transform_Initial_No_Translation =
      FTransform(Rotation_Initial, FVector::ZeroVector, Scale_Initial);

  Transform_Initial =
      FTransform(Rotation_Initial, Translation_Initial, Scale_Initial);
  SetRelativeTransform(Transform_Initial);
}

void UUnrealRobotLink::InitializePhysics(
    const projectairsim::Inertial& LinkInertial,
    const projectairsim::Collision& LinkCollision) {
  SetSimulatePhysics(true);
  SetEnableGravity(true);

  // Apply friction/restitution from collision config settings before other
  // physics settings like mass, etc so that other config values are not
  // overwritten by the material properties
  custom_material->Friction = LinkCollision.GetFriction();
  custom_material->Restitution = LinkCollision.GetRestitution();
  this->SetPhysMaterialOverride(custom_material);

  auto mass = LinkInertial.GetMass();
  SetMassOverrideInKg(NAME_None, mass);

  // UE calculates inertia automatically from mass/density and collision
  // geometry. Override it by getting UE's base inertia tensor, and setting the
  // scale calculated by cancelling out the base inertia and applying the config
  // inertia values.
  RegisterComponent();  // Use set mass for UE to calc its base inertia
  FVector base_inertia = this->BodyInstance.GetBodyInertiaTensor();
  const projectairsim::Matrix3x3& inertia_matrix = LinkInertial.GetInertia();
  this->BodyInstance.InertiaTensorScale = {
      inertia_matrix(0, 0) * 10000 / base_inertia.X,
      inertia_matrix(1, 1) * 10000 / base_inertia.Y,
      inertia_matrix(2, 2) * 10000 / base_inertia.Z};  // kg*m^2 -> kg*cm^2

  this->BodyInstance.StabilizationThresholdMultiplier =
      LinkInertial.GetStablizationThresholdMultiplier();

  // Use aerodynamic drag coefficient as linear/angular damping directly since
  // UE only uses a pseudo velocity damping term
  const float drag_coef = LinkInertial.GetDragCoefficient();
  this->BodyInstance.LinearDamping = drag_coef;
  this->BodyInstance.AngularDamping = drag_coef;
  this->BodyInstance.UpdateDampingProperties();

  this->bIgnoreRadialForce = true;
  this->bIgnoreRadialImpulse = true;
  this->bApplyImpulseOnDamage = false;
  this->BodyInstance.PositionSolverIterationCount =
      LinkInertial.GetPositionSolverIterationCount();
  this->BodyInstance.VelocitySolverIterationCount =
      LinkInertial.GetVelocitySolverIterationCount();
  this->SetCollisionProfileName(TEXT("SimPhysicsActor"));
}

void UUnrealRobotLink::BeginPlay() {
  Super::BeginPlay();

  // If collisions are requested and engine-based collision is requested, enable
  // collision detection on the component.
  //
  // For root components, OnComponentHit.AddDynamic() requests the engine to
  // invoke the callback on collisions.  However Unreal Engine doesn't perform
  // collision detection on non-root components.  To do collision detection
  // on secondary components like wheels, a flag in the associated link object
  // is set so that link.IsGroundCollisionDetectionEnabled() will be true.  In
  // that case we don't bother calling OnComponentHit.AddDynamic() here (since
  // it won't do anything) and we do explicit ground collision detection via
  // CheckGroundCollision().  Note that enabling ground collison detection on
  // the root component works just like for secondary components, but of course
  // that would lose out on the engine doing the collision detection and would
  // only report collisions with the ground.
  if (Collision_Hit_Callback && !link.IsGroundCollisionDetectionEnabled()) {
    OnComponentHit.AddDynamic(this, &UUnrealRobotLink::OnCollisionHit);
  }
}

void UUnrealRobotLink::SetActuatedFTransform(
    const ActuatedFTransform& actuatedftransform) {
  switch (actuatedftransform.applyorder) {
    case ActuatedFTransform::ApplyOrder::Pre:
      // Apply actuated transform then apply mesh's initial placement
      // transform
      SetRelativeTransform(actuatedftransform.ftransform * Transform_Initial);
      break;

    case ActuatedFTransform::ApplyOrder::PreTranslation:
      // Apply mesh's initial placement transform without translation, apply
      // actuated transform, then apply initial placement translate
      SetRelativeTransform(Transform_Initial_No_Translation *
                           actuatedftransform.ftransform *
                           Transform_Initial_Translation);
      break;

    case ActuatedFTransform::ApplyOrder::Post:
      // Apply mesh's initial placement transform then apply actuated
      // transform
      SetRelativeTransform(Transform_Initial * actuatedftransform.ftransform);
      break;
  }  // switch (actuatedftransform.applyorder)
}

void UUnrealRobotLink::SetCollisionHitCallback(
    FnCollisionHitCallback CollisionHitCallback) {
  Collision_Hit_Callback = CollisionHitCallback;
}

void UUnrealRobotLink::CheckGroundCollision(void) {
  static const float kHitTestDistanceCentimeters =
      projectairsim::TransformUtils::ToCentimeters(10000.0);
  static constexpr float nan = std::numeric_limits<float>::quiet_NaN();

  FVector StartTrace = GetComponentLocation();
  FVector EndTrace(StartTrace.X, StartTrace.Y, -kHitTestDistanceCentimeters);
  FHitResult HitResult(ForceInit);
  FCollisionQueryParams TraceParams;
  projectairsim::Vector3 Vec3Hit;
  Vec3Hit = projectairsim::Vector3(nan, nan, nan);
  auto World = GetWorld();
  bool WasHit = false;

  TraceParams.bTraceComplex = true;
  TraceParams.bReturnPhysicalMaterial = false;
  TraceParams.AddIgnoredComponent(this);

  // Look for ground straight down
  WasHit = World->LineTraceSingleByChannel(
      HitResult, StartTrace, EndTrace, ECC_Visibility, TraceParams,
      FCollisionResponseParams::DefaultResponseParam);
  if (!WasHit) {
    // Nothing below?  Look for the ground straight up
    EndTrace.Z = -EndTrace.Z;
    WasHit = World->LineTraceSingleByChannel(
        HitResult, StartTrace, EndTrace, ECC_Visibility, TraceParams,
        FCollisionResponseParams::DefaultResponseParam);
  }

  // Check for bottom of wheel being at or below ground leven
  if (WasHit) {
    FVector MinVectorBounds, MaxVectorBounds;
    auto ScaleVector = GetComponentScale();
    GetLocalBounds(MinVectorBounds, MaxVectorBounds);

    if ((StartTrace.Z + MinVectorBounds.Z * ScaleVector.Z) <=
        HitResult.ImpactPoint.Z) {
      // Wheel is on or below ground, trigger collision
      auto Collision_Hit_Callback_T = Collision_Hit_Callback;

      if (Collision_Hit_Callback_T != nullptr) {
        Collision_Hit_Callback_T(this, HitResult.GetActor(),
                               HitResult.GetComponent(), FVector(), HitResult);
      }
    }
  }
}

void UUnrealRobotLink::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (GetBodyInstance()->bSimulatePhysics) {
    GetBodyInstance()->WakeInstance();
  }

  // Do ground collision check if requested
  if ((Collision_Hit_Callback != nullptr) &&
      link.IsGroundCollisionDetectionEnabled()) {
    CheckGroundCollision();
  }
}

bool UUnrealRobotLink::IsGroundCollisionDetectionEnabled(void) const {
  return link.IsGroundCollisionDetectionEnabled();
}

bool UUnrealRobotLink::IsLinkCollisionEnabled() const {
  return link.GetCollision().IsCollisionEnabled();
}

void UUnrealRobotLink::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);
}

void UUnrealRobotLink::OnCollisionHit(UPrimitiveComponent* HitComponent,
                                      AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp,
                                      FVector NormalImpulse,
                                      const FHitResult& Hit) {
#ifdef DEBUG_VERBOSE
  UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                    TEXT("UnrealRobotLink: Collision detected between '%s' and "
                         "'%s' at z= '%f'"),
                    *(HitComponent->GetName()), *(OtherActor->GetName()),
                    Hit.Location.Z);
#endif//DEBUG_VERBOSE

  if (Collision_Hit_Callback)
    Collision_Hit_Callback(HitComponent, OtherActor, OtherComp, NormalImpulse,
                           Hit);
}

UUnrealRobotLink::ActuatedFTransform::ActuatedFTransform(
    const microsoft::projectairsim::ActuatedTransform& actuatedtransform) {
  static const auto kScalingConvertCoordinateSystem =
      Eigen::Scaling(1.0f, 1.0f, -1.0f);

  this->applyorder = actuatedtransform.applyorder;
  this->ftransform = UnrealHelpers::ToFTransform(
      kScalingConvertCoordinateSystem * actuatedtransform.affine3 *
      kScalingConvertCoordinateSystem);  // ActuatedTransforms are RH
                                         // coordinates (NED) but UE uses LH
                                         // coordinates
}