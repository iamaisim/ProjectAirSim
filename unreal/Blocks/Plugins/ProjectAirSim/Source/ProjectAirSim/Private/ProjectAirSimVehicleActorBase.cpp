// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "ProjectAirSimVehicleActorBase.h"

#include "Components/PrimitiveComponent.h"

AProjectAirSimVehicleActorBase::AProjectAirSimVehicleActorBase() {
  PrimaryActorTick.bCanEverTick = true;
}

void AProjectAirSimVehicleActorBase::SetActuatorSignal_Implementation(int32 Index, float Signal) {
  if (Index < 0) return;
  if (Index >= StoredActuatorSignals_.Num()) {
    StoredActuatorSignals_.SetNum(Index + 1, false);
  }
  StoredActuatorSignals_[Index] = Signal;
}

float AProjectAirSimVehicleActorBase::GetActuatorSignal_Implementation(const FString& Name) {
  // Simple index-based lookup by converting name to index if possible.
  // Blueprints that need name-based access should override this.
  return 0.f;
}

FVector AProjectAirSimVehicleActorBase::GetPosition_Implementation() {
  return GetActorLocation();
}

FQuat AProjectAirSimVehicleActorBase::GetRotation_Implementation() {
  return GetActorQuat();
}

FVector AProjectAirSimVehicleActorBase::GetLinearVelocity_Implementation() {
  if (UPrimitiveComponent* Root =
          Cast<UPrimitiveComponent>(GetRootComponent())) {
    if (Root->IsSimulatingPhysics()) {
      return Root->GetPhysicsLinearVelocity();
    }
  }
  return GetVelocity();
}

FVector AProjectAirSimVehicleActorBase::GetAngularVelocity_Implementation() {
  if (UPrimitiveComponent* Root =
          Cast<UPrimitiveComponent>(GetRootComponent())) {
    if (Root->IsSimulatingPhysics()) {
      return Root->GetPhysicsAngularVelocityInRadians();
    }
  }
  return FVector::ZeroVector;
}
