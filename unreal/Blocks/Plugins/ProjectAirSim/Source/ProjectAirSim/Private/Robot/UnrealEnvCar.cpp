// Copyright (C) Microsoft Corporation. All rights reserved.
// The Unreal EnvCar implementation.

#include "UnrealEnvCar.h"

#include "core_sim/actor/env_car.hpp"
#include "core_sim/link/geometry/unreal_mesh.hpp"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/SkeletalMesh.h"

namespace projectairsim = microsoft::projectairsim;

AUnrealEnvActorCar::AUnrealEnvActorCar(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {}

std::pair<std::string, UUnrealRobotLink*> AUnrealEnvActorCar::CreateLink(
    const projectairsim::Link& InLink, bool bIsRootLink) {
  auto Id = InLink.GetID();

  auto NewLink = NewObject<UUnrealRobotLink>(this, Id.c_str());
  NewLink->Initialize(InLink,
                      false);  // with_unreal_physics always false for EnvActors

  if (bIsRootLink) {
    EnvActorRootLink = NewLink;
    RootComponent = NewLink;  // Also set as the USceneComponent's RootComponent
  }

  auto skeletal_mesh = static_cast<const projectairsim::SkeletalMesh*>(InLink.GetVisual().GetGeometry());

  auto meshPath = FString(skeletal_mesh->GetName().c_str());
  auto skeletalMesh = LoadObject<USkeletalMesh>(nullptr, *meshPath);

  if(skeletalMesh == nullptr){
    if (Id == "Frame") {
        auto mesh = static_cast<const projectairsim::UnrealMesh*>(InLink.GetVisual().GetGeometry());
        if(mesh != nullptr) {
            scale = mesh->GetScale();
            // Change the car materials
            auto material_mesh = InLink.GetVisual().GetMaterial();
            auto colored_material = FString(material_mesh.GetColoredTexture().c_str());
            auto MaterialAsset =
                LoadObject<UMaterialInterface>(nullptr, *colored_material);
            for (int32 i = 0; i < NewLink->GetNumMaterials(); ++i) {
                FName SlotName = NewLink->GetMaterialSlotNames()[i];
                if (SlotName.ToString().Contains("Carpaint") && MaterialAsset != nullptr) {
                    NewLink->SetMaterial(i, MaterialAsset);
                    break;
                }
            }
        }
    }
    if (Id.find("Wheel") != std::string::npos) {
        wheel_radius =
            NewLink->GetStaticMesh()->GetBounds().BoxExtent.Z * scale.z();
        auto simEnvCar = static_cast<microsoft::projectairsim::EnvCar&>(SimEnvActor);
        simEnvCar.SetWheelRadius(projectairsim::TransformUtils::ToMeters(wheel_radius));  // Set the wheel radius
    }
  }
  else {
        InitializeSkeletalMesh(skeletalMesh);
        has_skeletal_mesh = true;
        auto simEnvCar = static_cast<microsoft::projectairsim::EnvCar&>(SimEnvActor);
        simEnvCar.HasSkeletalMesh(has_skeletal_mesh);  // Set that the car has a skeletal mesh
        // Scale for EnvCar is set in the skeletal mesh
        {
            auto fvector_scale_local_cur = PoseableMeshComponent->GetRelativeScale3D();
            auto fvector_scale_world_new =
                (skeletal_mesh == nullptr)
                    ? projectairsim::Vector3(1.0f, 1.0f, 1.0f)
                    : skeletal_mesh->GetScale();  // Get desired world scaling
            auto fvector_scale_world_cur =
                PoseableMeshComponent->GetComponentScale();  // Get current world scaling
            float scale_x_local =
                projectairsim::MathUtils::IsApproximatelyZero(fvector_scale_world_cur.X)
                    ? std::numeric_limits<float>::max()
                    : fvector_scale_world_new.x() / fvector_scale_world_cur.X *
                        fvector_scale_local_cur.X;
            float scale_y_local =
                projectairsim::MathUtils::IsApproximatelyZero(fvector_scale_world_cur.Y)
                    ? std::numeric_limits<float>::max()
                    : fvector_scale_world_new.y() / fvector_scale_world_cur.Y *
                        fvector_scale_local_cur.Y;
            float scale_z_local =
                projectairsim::MathUtils::IsApproximatelyZero(fvector_scale_world_cur.Z)
                    ? std::numeric_limits<float>::max()
                    : fvector_scale_world_new.z() / fvector_scale_world_cur.Z *
                        fvector_scale_local_cur.Z;
            // Set the new local scaling
            PoseableMeshComponent->SetRelativeScale3D(FVector(scale_x_local, scale_y_local, scale_z_local));
            float MeshBoundsX = PoseableMeshComponent->GetSkinnedAsset()->GetBounds().BoxExtent.X;
            float MeshBoundsY = PoseableMeshComponent->GetSkinnedAsset()->GetBounds().BoxExtent.Y;
            float MeshBoundsZ = PoseableMeshComponent->GetSkinnedAsset()->GetBounds().BoxExtent.Z;
            if (MeshBoundsX > MeshBoundsY) {
                CapsuleComponent->SetCapsuleSize(MeshBoundsX, MeshBoundsZ);
            } else {
                CapsuleComponent->SetCapsuleSize(MeshBoundsY, MeshBoundsZ);
            }
            capsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
        }
        // Set Carpaint material
        auto material_mesh = InLink.GetVisual().GetMaterial();
        auto colored_material = FString(material_mesh.GetColoredTexture().c_str());
        auto MaterialAsset =
            LoadObject<UMaterialInterface>(nullptr, *colored_material);
        for (int32 i = 0; i < PoseableMeshComponent->GetNumMaterials(); ++i) {
            FName SlotName = PoseableMeshComponent->GetMaterialSlotNames()[i];
            if (SlotName.ToString().Contains("Carpaint") && MaterialAsset != nullptr) {
                PoseableMeshComponent->SetMaterial(i, MaterialAsset);
                break;
            }
        }
    }
    return {Id, NewLink};
}

void AUnrealEnvActorCar::InitializeSkeletalMesh(USkeletalMesh* skeletalMesh) {
    if (CapsuleComponent == nullptr) {
        CapsuleComponent = NewObject<UCapsuleComponent>(this, TEXT("CapsuleComponent"));
        CapsuleComponent->SetupAttachment(RootComponent);  // Attach to the RootComponent
        CapsuleComponent->RegisterComponent();  // Register the component
    }

    PoseableMeshComponent = NewObject<UPoseableMeshComponent>(this, TEXT("PoseableMeshComponent"));
    PoseableMeshComponent->SetupAttachment(RootComponent);  // Attach to the RootComponent
    PoseableMeshComponent->RegisterComponent(); 
    PoseableMeshComponent->SetSkinnedAssetAndUpdate(skeletalMesh);
    PoseableMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PoseableMeshComponent->SetCollisionProfileName(TEXT("BlockAll"));

    //Initialize wheel bones
    int32 NumBones = PoseableMeshComponent->GetNumBones();
    FVector FrontLeftWheelLocation, BackLeftWheelLocation, WheelScale;

    for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++) {
        FName BoneName = PoseableMeshComponent->GetBoneName(BoneIndex);
        FString BoneNameString = BoneName.ToString();

        if (BoneNameString.Contains(TEXT("FL"))) {
            // add BoneIndex to FrontWheels vector
            FrontWheels.push_back(BoneIndex);
            // this is to get the distance between wheels
            FrontLeftWheelLocation = PoseableMeshComponent->GetBoneLocationByName(BoneName, EBoneSpaces::ComponentSpace);
            WheelScale = PoseableMeshComponent->GetBoneScaleByName(BoneName, EBoneSpaces::ComponentSpace);
        } else if (BoneNameString.Contains(TEXT("FR"))) {
            FrontWheels.push_back(BoneIndex);
        } else if (BoneNameString.Contains(TEXT("BL"))) {
            BackWheels.push_back(BoneIndex);
            BackLeftWheelLocation = PoseableMeshComponent->GetBoneLocationByName(BoneName, EBoneSpaces::ComponentSpace);
        } else if (BoneNameString.Contains(TEXT("BR"))) {
            BackWheels.push_back(BoneIndex);
        }
    }

    wheel_radius = WheelScale.Z * FrontLeftWheelLocation.Z;
    auto simEnvCar = static_cast<microsoft::projectairsim::EnvCar&>(SimEnvActor);
    simEnvCar.SetWheelRadius(projectairsim::TransformUtils::ToMeters(wheel_radius));

    wheels_distance = FVector::Distance(FrontLeftWheelLocation, BackLeftWheelLocation);
    simEnvCar.SetWheelsDistance(projectairsim::TransformUtils::ToMeters(wheels_distance));
}

void AUnrealEnvActorCar::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);
  // In case of a tick coming through while it should be paused, just return
  if (UGameplayStatics::IsGamePaused(this->GetWorld())) return;

  if(PoseableMeshComponent == nullptr) {
    DetectZPositionwithStaticMesh(wheel_radius);
  }
  else{
    DetectZPositionwithSkeletalMesh(wheel_radius);
    UpdateWheelRotation();
  }
}

void AUnrealEnvActorCar::DetectZPositionwithStaticMesh(float wheel_rad){
    FVector start = GetActorLocation() + FVector(0, 0, -wheel_radius);
    FVector endUnder = start + FVector(0, 0, -wheel_rad);  // The end of the vector under the car
    FVector endOver = start + FVector(0, 0, wheel_rad);  // The end of the vector over the car

  FHitResult HitResultUnder;
  FHitResult HitResultOver;
  FCollisionQueryParams TraceParams;
  TraceParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResultUnder, start, endUnder, ECC_Visibility, TraceParams);
    bool bHitOver = GetWorld()->LineTraceSingleByChannel(HitResultOver, endOver, start, ECC_Visibility, TraceParams);

    auto SimEnvActorGrounded = static_cast<microsoft::projectairsim::EnvActorGrounded&>(SimEnvActor);

    if (bHitOver) {
        // Ceiling detected
        FVector CeilingLocation = HitResultOver.Location;
        SimEnvActorGrounded.SetGroundLevel(
            projectairsim::TransformUtils::NeuToNedLinear(
                projectairsim::TransformUtils::ToMeters(
                    UnrealHelpers::ToVector3(CeilingLocation + FVector(0, 0, wheel_rad)))).z());
    } else if (bHit) {
        // Floor detected
        FVector FloorLocation = HitResultUnder.Location;
        SimEnvActorGrounded.SetGroundLevel(
            projectairsim::TransformUtils::NeuToNedLinear(
                projectairsim::TransformUtils::ToMeters(
                    UnrealHelpers::ToVector3(FloorLocation + FVector(0, 0, wheel_rad)))).z());
    } else {
        // Floor not detected
        auto ground_level = projectairsim::TransformUtils::NeuToNedLinear(
                projectairsim::TransformUtils::ToMeters(
                    UnrealHelpers::ToVector3(endUnder + FVector(0, 0, wheel_rad)))).z();
        SimEnvActorGrounded.SetGroundLevel(ground_level);
    }
}
  
void AUnrealEnvActorCar::DetectZPositionwithSkeletalMesh(float wheel_rad){
    FVector start = GetActorLocation();
    FVector endUnder = start + FVector(0, 0, -wheel_rad);  // a trace down to the ground level
    FVector endOver = start + FVector(0, 0, wheel_rad);  // a trace up to the ceiling

    FHitResult HitResultUnder;
    FHitResult HitResultOver;
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(this);

    // Perform a LineTrace downward to detect the floor
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResultUnder, start, endUnder, ECC_Visibility, TraceParams);
    bool bHitOver = GetWorld()->LineTraceSingleByChannel(HitResultOver, endOver, start, ECC_Visibility, TraceParams);

    auto SimEnvActorGrounded = static_cast<microsoft::projectairsim::EnvActorGrounded&>(SimEnvActor);

  if (bHitOver) {
    // Ceiling detected
    FVector CeilingLocation = HitResultOver.Location;
    SimEnvActorGrounded.SetGroundLevel(
        projectairsim::TransformUtils::NeuToNedLinear(
            projectairsim::TransformUtils::ToMeters(
                UnrealHelpers::ToVector3(CeilingLocation)))
            .z());
  } else if (bHit) {
    // Floor detected
    FVector FloorLocation = HitResultUnder.Location;
    SimEnvActorGrounded.SetGroundLevel(
        projectairsim::TransformUtils::NeuToNedLinear(
            projectairsim::TransformUtils::ToMeters(
                UnrealHelpers::ToVector3(FloorLocation)))
            .z());
  } else {
    // Floor not detected
    auto ground_level = projectairsim::TransformUtils::NeuToNedLinear(
            projectairsim::TransformUtils::ToMeters(
                UnrealHelpers::ToVector3(endUnder)))
            .z();
    SimEnvActorGrounded.SetGroundLevel(ground_level);
  }
}

void AUnrealEnvActorCar::UpdateWheelRotation() {
    if (PoseableMeshComponent) {
        auto simEnvCar = static_cast<microsoft::projectairsim::EnvCar&>(SimEnvActor);
        TimeNano NowNanos = projectairsim::SimClock::Get()->NowSimNanos();
        float dt = (NowNanos - PrevSimTimeNanos) / 1.0e9f;
        PrevSimTimeNanos = NowNanos;

        float WheelRotationRateDeg = -1 * simEnvCar.GetWheelRotationRateDeg();
        CurrentAngle += WheelRotationRateDeg * dt;
        float SteeringAngle = simEnvCar.GetSteeringAngleDeg();

        // Iterate over front wheels
        for (auto& fwheel : FrontWheels) {
                PoseableMeshComponent->SetBoneRotationByName(
                PoseableMeshComponent->GetBoneName(fwheel),
                FRotator(CurrentAngle, SteeringAngle, 0),
                EBoneSpaces::ComponentSpace
            );
        }

        // interate over back wheels
                for (auto& bwheel : BackWheels) {
                PoseableMeshComponent->SetBoneRotationByName(
                PoseableMeshComponent->GetBoneName(bwheel),
                FRotator(CurrentAngle, 0, 0),
                EBoneSpaces::ComponentSpace
            );
        }
    }
}