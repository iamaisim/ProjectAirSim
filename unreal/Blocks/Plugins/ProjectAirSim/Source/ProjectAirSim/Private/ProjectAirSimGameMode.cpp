// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "ProjectAirSimGameMode.h"

#include <exception>

#include "Runtime/Core/Public/Misc/Paths.h"

AProjectAirSimGameMode::AProjectAirSimGameMode(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer),
      UnrealSimLoader(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir())) {
  DefaultPawnClass = nullptr;
  FApp::bUseFixedSeed = true;  // for determinism, persists in UE project
}

void AProjectAirSimGameMode::StartPlay() {
  Super::StartPlay();

  UnrealSimLoader.LaunchSimulation(this->GetWorld());
}

void AProjectAirSimGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);

  UnrealSimLoader.TeardownSimulation();
  FApp::bUseFixedSeed = false;  // reset back to default from App.cpp
}
