// Copyright (C) Microsoft Corporation.  
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "ProjectAirSimGameMode.h"
#include "GameFramework/PlayerController.h"
#include <exception>

#include "Runtime/Core/Public/Misc/Paths.h"

AProjectAirSimGameMode::AProjectAirSimGameMode(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer),
      UnrealSimLoader(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir())) {
  DefaultPawnClass = nullptr;
  FApp::bUseFixedSeed = true;  // for determinism, persists in UE project

    SettingsMenuClass = FSoftClassPath(TEXT("/ProjectAirSim/UI/ConfigEditor/WBP_SettingsMenu.WBP_SettingsMenu_C"));
}

void AProjectAirSimGameMode::StartPlay() {
    Super::StartPlay();

    UnrealSimLoader.LaunchSimulation(this->GetWorld());
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    
    if (PC && PC->PlayerCameraManager)
    {
        UClass* LoadedWidgetClass = SettingsMenuClass.LoadSynchronous();
        if (LoadedWidgetClass)
        {
            SettingsMenuInstance = CreateWidget<USettingsMenu>(PC, LoadedWidgetClass);
            if (SettingsMenuInstance) 
            {
                SettingsMenuInstance->AddToViewport();
                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(SettingsMenuInstance->TakeWidget());
                PC->SetInputMode(InputMode);
                PC->bShowMouseCursor = true;
            }
        }
    }    
}

void AProjectAirSimGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  Super::EndPlay(EndPlayReason);

  UnrealSimLoader.TeardownSimulation();
  FApp::bUseFixedSeed = false;  // reset back to default from App.cpp
}
