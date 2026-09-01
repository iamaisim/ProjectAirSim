// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "ProjectAirSimVehicleActorBase.h"

#include "Components/PrimitiveComponent.h"

AProjectAirSimVehicleActorBase::AProjectAirSimVehicleActorBase() {
  PrimaryActorTick.bCanEverTick = true;
}

void AProjectAirSimVehicleActorBase::SetParameterSignal_Implementation(
    int32 Index, float Value) {
  if (Index < 0) return;
  if (Index >= StoredParameterSignals_.Num()) {
    StoredParameterSignals_.SetNum(Index + 1, false);
  }
  StoredParameterSignals_[Index] = Value;
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
