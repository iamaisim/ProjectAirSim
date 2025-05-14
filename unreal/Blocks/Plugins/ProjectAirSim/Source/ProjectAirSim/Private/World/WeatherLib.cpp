// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "WeatherLib.h"

#include "Materials/MaterialParameterCollection.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Blueprint/WidgetBlueprintLibrary.h"

AExponentialHeightFog* UWeatherLib::weather_fog_ = nullptr;

UMaterialParameterCollectionInstance*
UWeatherLib::getWeatherMaterialCollectionInstance(UWorld* World) {
  if (World) {
    UMaterialParameterCollection* WeatherParameterCollection =
        Cast<UMaterialParameterCollection>(
            StaticLoadObject(UMaterialParameterCollection::StaticClass(), NULL,
                             getWeatherParamsObjectPath()));

    if (WeatherParameterCollection) {
      UMaterialParameterCollectionInstance* Instance =
          World->GetParameterCollectionInstance(WeatherParameterCollection);
      if (Instance) {
        return Instance;
      } else {
        UE_LOG(LogTemp, Warning,
               TEXT("Warning, WeatherAPI could NOT get "
                    "WeatherParameterCollectionInstance1!"));
      }

    } else {
      UE_LOG(LogTemp, Warning,
             TEXT("Warning, WeatherAPI could NOT get "
                  "WeatherParameterCollection1!"));
    }
  } else {
    UE_LOG(LogTemp, Warning, TEXT("Warning, WeatherAPI could NOT get World!"));
  }

  return NULL;
}

void UWeatherLib::initWeather(UWorld* World,
                              TArray<AUnrealRobot*> ActorsToAttachTo) {
  weather_fog_ = nullptr;

  if (World) {
    // Reset weather parameter instance with a new set of default values
    resetWeatherParams(World);
    UClass* WeatherActorClass = getWeatherActorPath().TryLoadClass<AActor>();
    if (WeatherActorClass) {
      for (int32 i = 0; i < ActorsToAttachTo.Num(); i++) {
        // Get root component of the parent robot
        USceneComponent* parent = ActorsToAttachTo[i]->GetRootComponent();

        // Position weather actor at a hardcoded 500 cm above robot
        // (Unreal convention - cm and +ve Z)
        const FVector Location = FVector(0, 0, 500);
        const FRotator Rotation = parent->GetComponentRotation();
        FActorSpawnParameters WeatherActorSpawnInfo;
        WeatherActorSpawnInfo.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* SpawnedWeatherActor = World->SpawnActor(
            WeatherActorClass, &Location, &Rotation, WeatherActorSpawnInfo);

        // Attach weather actor to robot with a relative transform
        SpawnedWeatherActor->AttachToComponent(
            parent, FAttachmentTransformRules::KeepRelativeTransform);
      }
    } else {
      UE_LOG(LogTemp, Warning,
             TEXT("Warning, WeatherAPI got invalid weather actor class!"));
    }
    // still need the menu class for f10
    UClass* MenuActorClass = getWeatherMenuObjectPath().TryLoadClass<AActor>();
    if (MenuActorClass) {
      // UClass* Class, FTransform const* Transform, const
      // FActorSpawnParameters& SpawnParameters = FActorSpawnParameters()
      const FVector Location = FVector(0, 0, 0);
      const FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
      FActorSpawnParameters SpawnInfo;
      SpawnInfo.SpawnCollisionHandlingOverride =
          ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
      World->SpawnActor(MenuActorClass, &Location, &Rotation, SpawnInfo);
    } else {
      UE_LOG(LogTemp, Warning,
             TEXT("Warning, WeatherAPI got invalid menu actor class!"));
    }
  }

  // showWeatherMenu(WorldContextObject);
}

void UWeatherLib::resetWeatherParams(UWorld* World) {
  if (World) {
    // Save existing wind direction when resetting weather params
    FVector CurWindDir = getWeatherWindDirection(World);

    // Reset weather parameter instance with a new set of default values
    UMaterialParameterCollection* WeatherParameterCollection =
        Cast<UMaterialParameterCollection>(
            StaticLoadObject(UMaterialParameterCollection::StaticClass(), NULL,
                             getWeatherParamsObjectPath()));
    World->AddParameterCollectionInstance(WeatherParameterCollection, true);

    // Re-apply existing wind direction
    setWeatherWindDirection(World, CurWindDir);
  }
}

void UWeatherLib::unloadWeather(UWorld* World) {
  if (World) {
    TArray<AActor*> ActorsToDestroy;
    // Destroy all weather actors
    UClass* WeatherActorClass = getWeatherActorPath().TryLoadClass<AActor>();
    if (WeatherActorClass) {
      UGameplayStatics::GetAllActorsOfClass(World, WeatherActorClass,
                                            ActorsToDestroy);
      for (auto& CurActor : ActorsToDestroy) {
        CurActor->Destroy();
      }
    }
    ActorsToDestroy.Empty();

    // Destroy all menu actors
    UClass* MenuActorClass = getWeatherMenuObjectPath().TryLoadClass<AActor>();
    if (MenuActorClass) {
      UGameplayStatics::GetAllActorsOfClass(World, MenuActorClass,
                                            ActorsToDestroy);
      for (auto& CurActor : ActorsToDestroy) {
        CurActor->Destroy();
      }
    }
  }

  weather_fog_ = nullptr;
}

bool UWeatherLib::setWeatherParamScalar(UWorld* World,
                                        EWeatherParamScalar Param,
                                        float Amount) {
  UMaterialParameterCollectionInstance* WeatherMaterialCollectionInstance =
      UWeatherLib::getWeatherMaterialCollectionInstance(World);

  bool status = false;
  if (WeatherMaterialCollectionInstance) {
    FName ParamName = GetWeatherParamScalarName(Param);
    if (ParamName == TEXT("")) {
      UE_LOG(LogTemp, Warning,
             TEXT("Warning, WeatherAPI got invalid paramname!"));
      return status;
    }

    // Make sure weather is enabled
    if (Param == EWeatherParamScalar::WEATHER_PARAM_SCALAR_WEATHERENABLED) {
      WeatherMaterialCollectionInstance->SetScalarParameterValue(ParamName,
                                                                 Amount);
      status = true;
    } else {
      if (getIsWeatherEnabled(World)) {
        WeatherMaterialCollectionInstance->SetScalarParameterValue(ParamName,
                                                                   Amount);
        status = true;
      }

      // If weather is not enabled, dont allow any weather values to be set
      // because it would not work anyway.
      else {
        WeatherMaterialCollectionInstance->SetScalarParameterValue(ParamName,
                                                                   0.0f);
      }

      if (weather_fog_) {
        if (ParamName.IsEqual("Dust") | ParamName.IsEqual("Fog")) {
          auto fog = getWeatherParamScalar(
              World, EWeatherParamScalar::WEATHER_PARAM_SCALAR_FOG);
          auto dust = getWeatherParamScalar(
              World, EWeatherParamScalar::WEATHER_PARAM_SCALAR_DUST);

          auto weather_root_comp = weather_fog_->GetRootComponent();

          if( weather_root_comp ) {
            if (fog + dust > 0.0f) {
              weather_root_comp->SetVisibility(true);
            } else {
              weather_root_comp->SetVisibility(false);
            }
          }
        }
      }
    }
  } else {
    UE_LOG(
        LogTemp, Warning,
        TEXT("Warning, WeatherAPI could NOT get MaterialCollectionInstance!"));
    return status;
  }
  return status;
}

float UWeatherLib::getWeatherParamScalar(UWorld* World,
                                         EWeatherParamScalar Param) {
  UMaterialParameterCollectionInstance* WeatherMaterialCollectionInstance =
      UWeatherLib::getWeatherMaterialCollectionInstance(World);
  if (WeatherMaterialCollectionInstance) {
    FName ParamName = GetWeatherParamScalarName(Param);
    if (ParamName == TEXT("")) {
      UE_LOG(LogTemp, Warning,
             TEXT("Warning, WeatherAPI got invalid paramname!"));
    }
    float Amount;
    WeatherMaterialCollectionInstance->GetScalarParameterValue(
        ParamName, Amount);  // SetScalarParameterValue(ParamName, Amount);

    return Amount;
  } else {
    UE_LOG(
        LogTemp, Warning,
        TEXT("Warning, WeatherAPI could NOT get MaterialCollectionInstance!"));
  }
  return 0.0f;
}

FVector UWeatherLib::getWeatherWindDirection(UWorld* World) {
  UMaterialParameterCollectionInstance* WeatherMaterialCollectionInstance =
      UWeatherLib::getWeatherMaterialCollectionInstance(World);
  if (WeatherMaterialCollectionInstance) {
    FName ParamName = GetWeatherParamVectorName(
        EWeatherParamVector::WEATHER_PARAM_VECTOR_WIND);
    if (ParamName == TEXT("")) {
      UE_LOG(LogTemp, Warning,
             TEXT("Warning, WeatherAPI got invalid paramname!"));
    }
    FLinearColor Direction;
    WeatherMaterialCollectionInstance->GetVectorParameterValue(
        ParamName, Direction);  // SetScalarParameterValue(ParamName, Amount);

    return FVector(Direction);
  } else {
    UE_LOG(
        LogTemp, Warning,
        TEXT("Warning, WeatherAPI could NOT get MaterialCollectionInstance!"));
  }
  return FVector(0, 0, 0);
}

void UWeatherLib::setWeatherWindDirection(UWorld* World, FVector NewWind) {
  UMaterialParameterCollectionInstance* WeatherMaterialCollectionInstance =
      UWeatherLib::getWeatherMaterialCollectionInstance(World);
  if (WeatherMaterialCollectionInstance) {
    FName ParamName = GetWeatherParamVectorName(
        EWeatherParamVector::WEATHER_PARAM_VECTOR_WIND);
    if (ParamName == TEXT("")) {
      UE_LOG(LogTemp, Warning,
             TEXT("Warning, WeatherAPI got invalid paramname!"));
    }
    WeatherMaterialCollectionInstance->SetVectorParameterValue(ParamName,
                                                               NewWind);
  } else {
    UE_LOG(
        LogTemp, Warning,
        TEXT("Warning, WeatherAPI could NOT get MaterialCollectionInstance!"));
  }
}

bool UWeatherLib::getIsWeatherEnabled(UWorld* World) {
  if (getWeatherParamScalar(
          World, EWeatherParamScalar::WEATHER_PARAM_SCALAR_WEATHERENABLED) ==
      1.0f) {
    return true;
  }
  return false;
}

bool UWeatherLib::setWeatherEnabled(UWorld* World, bool bEnabled) {
  return setWeatherParamScalar(
      World, EWeatherParamScalar::WEATHER_PARAM_SCALAR_WEATHERENABLED,
      (float)bEnabled);
}

void UWeatherLib::showWeatherMenu(UWorld* World) {
  // UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,
  // EGetWorldErrorMode::LogAndReturnNull);

  if (UClass* MenuWidgetClass =
          getWeatherMenuWidgetClass().TryLoadClass<UUserWidget>()) {
    UUserWidget* MenuWidget = CreateWidget<UUserWidget>(World, MenuWidgetClass);

    if (MenuWidget) {
      MenuWidget->AddToViewport();
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC) {
      PC->bShowMouseCursor = true;
      PC->DisableInput(PC);
    }
  } else {
    UE_LOG(LogTemp, Warning,
           TEXT("Warning, WeatherAPI could not load weather widget!"));
  }
}

void UWeatherLib::hideWeatherMenu(UWorld* World) {
  UClass* MenuWidgetClass =
      getWeatherMenuWidgetClass().TryLoadClass<UUserWidget>();

  if (World && MenuWidgetClass) {
    // get all menu actors, if any
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets,
                                                  UUserWidget::StaticClass());

    // UE_LOG(LogTemp, Warning, TEXT("%s Warning, WeatherAPI"),
    //        *MenuWidgetClass->GetClass()->GetFName().ToString());

    if (FoundWidgets.Num() > 0) {
      for (int32 i = 0; i < FoundWidgets.Num(); i++) {
        // hacky test to make sure we are getting the right class. for some
        // reason cast above doesn't work, so we use this instead to test for
        // class
        if (FoundWidgets[i] &&
            FoundWidgets[i]->GetClass()->GetFName().ToString() ==
                getWeatherMenuClassName()) {
          FoundWidgets[i]->RemoveFromParent();
        }
      }
      APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
      if (PC) {
        PC->bShowMouseCursor = false;
        PC->EnableInput(PC);
      }
    }
  }
}

bool UWeatherLib::isMenuVisible(UWorld* World) {
  UClass* MenuWidgetClass =
      getWeatherMenuWidgetClass().TryLoadClass<UUserWidget>();

  if (World && MenuWidgetClass) {
    // get all menu actors, if any
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets,
                                                  UUserWidget::StaticClass());

    // UE_LOG(LogTemp, Warning, TEXT("%s Warning, WeatherAPI"),
    //        *MenuWidgetClass->GetClass()->GetFName().ToString());

    if (FoundWidgets.Num() > 0) {
      for (int32 i = 0; i < FoundWidgets.Num(); i++) {
        // hacky test to make sure we are getting the right class. for some
        // reason cast above doesn't work, so we use this instead to test for
        // class
        if (FoundWidgets[i] &&
            FoundWidgets[i]->GetClass()->GetFName().ToString() ==
                getWeatherMenuClassName()) {
          return true;
        }
      }
    }
  }
  // get all menu actors, if any, then hide the menu
  return false;
}

void UWeatherLib::toggleWeatherMenu(UWorld* World) {
  if (isMenuVisible(World)) {
    hideWeatherMenu(World);
  } else {
    showWeatherMenu(World);
  }
}

UWorld* UWeatherLib::widgetGetWorld(UUserWidget* Widget) {
  if (Widget) {
    return Widget->GetWorld();
  }
  return NULL;
}

UWorld* UWeatherLib::actorGetWorld(AActor* Actor) {
  if (Actor) {
    return Actor->GetWorld();
  }
  return NULL;
}

void UWeatherLib::setWeatherFog(AExponentialHeightFog* fog) {
  weather_fog_ = fog;
}
