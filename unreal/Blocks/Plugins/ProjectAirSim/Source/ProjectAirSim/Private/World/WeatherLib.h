// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/ExponentialHeightFog.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Robot/UnrealRobot.h"
#include "WeatherLib.generated.h"

// NOTE: when adding new enums, you must add it to GetWeatherParamScalarName()'s
// switch statement with a name for the corresponding material collection param
// name can't use enum's display names because on package those get deleted.
UENUM(BlueprintType)
enum class EWeatherParamScalar : uint8 {
  WEATHER_PARAM_SCALAR_WEATHERENABLED = 0 UMETA(DisplayName = "WeatherEnabled"),
  WEATHER_PARAM_SCALAR_RAIN = 1 UMETA(DisplayName = "Rain"),
  WEATHER_PARAM_SCALAR_ROADWETNESS = 2 UMETA(DisplayName = "RoadWetness"),
  WEATHER_PARAM_SCALAR_SNOW = 3 UMETA(DisplayName = "Snow"),
  WEATHER_PARAM_SCALAR_ROADSNOW = 4 UMETA(DisplayName = "RoadSnow"),
  WEATHER_PARAM_SCALAR_MAPLELEAF = 5 UMETA(DisplayName = "MapleLeaf"),
  WEATHER_PARAM_SCALAR_ROADLEAF = 6 UMETA(DisplayName = "RoadLeaf"),
  WEATHER_PARAM_SCALAR_DUST = 7 UMETA(DisplayName = "Dust"),
  WEATHER_PARAM_SCALAR_FOG = 8 UMETA(DisplayName = "Fog")
};

UENUM(BlueprintType)
enum class EWeatherParamVector : uint8 {
  WEATHER_PARAM_VECTOR_WIND = 0 UMETA(DisplayName = "Wind"),
  WEATHER_PARAM_VECTOR_MAX = 1 UMETA(DisplayName = "MAX")
};
/**
 *
 */
UCLASS(BlueprintType)
class UWeatherLib : public UBlueprintFunctionLibrary {
  GENERATED_BODY()

  // not sure why, but content folder should be omitted in the path
  // location of the weather UMaterialParameterCollection, params for rain snow
  // wind etc
  static const TCHAR* getWeatherParamsObjectPath() {
    return TEXT("/ProjectAirSim/Weather/WeatherFX/WeatherGlobalParams");
  }
  static const FSoftClassPath getWeatherMenuObjectPath() {
    return FSoftClassPath(
        TEXT("AActor'/ProjectAirSim/Weather/UI/MenuActor.MenuActor_C'"));
  }

  static const FSoftClassPath getWeatherActorPath() {
    return FSoftClassPath(TEXT(
        "AActor'/ProjectAirSim/Weather/WeatherFX/WeatherActor.WeatherActor_C'"));
  }

  static const FSoftClassPath getWeatherMenuWidgetClass() {
    return FSoftClassPath(
        TEXT("UUserWidget'/ProjectAirSim/Weather/UI/OptionsMenu.OptionsMenu_C'"));
  }
  /*static const FSoftClassPath getMenuClass()
  {
          return TEXT("/ProjectAirSim/Weather/UI/OptionsMenu.OptionsMenu_C");
  }*/
  // menu class name for finding and closing it
  static const FString getWeatherMenuClassName() {
    return TEXT("OptionsMenu_C");
  }

  // corresponding param name to set in Weather Params material collection
  static const FName GetWeatherParamScalarName(
      EWeatherParamScalar WeatherParam) {
    switch (WeatherParam) {
      case EWeatherParamScalar::WEATHER_PARAM_SCALAR_RAIN: {
        return TEXT("Rain");
        break;
      }
      case EWeatherParamScalar::WEATHER_PARAM_SCALAR_ROADWETNESS: {
        return TEXT("RoadWetness");
        break;
      }
      case EWeatherParamScalar::WEATHER_PARAM_SCALAR_SNOW: {
        return TEXT("Snow");
        break;
      }
      case EWeatherParamScalar::WEATHER_PARAM_SCALAR_ROADSNOW: {
        return TEXT("RoadSnow");
        break;
      }
      case EWeatherParamScalar::WEATHER_PARAM_SCALAR_MAPLELEAF: {
        return TEXT("MapleLeaf");
        break;
      }
      case EWeatherParamScalar::WEATHER_PARAM_SCALAR_ROADLEAF: {
        return TEXT("RoadLeaf");
        break;
      }
      case EWeatherParamScalar::WEATHER_PARAM_SCALAR_DUST: {
        return TEXT("Dust");
        break;
      }
      case EWeatherParamScalar::WEATHER_PARAM_SCALAR_FOG: {
        return TEXT("Fog");
        break;
      }
      case EWeatherParamScalar::WEATHER_PARAM_SCALAR_WEATHERENABLED: {
        return TEXT("WeatherEnabled");
        break;
      }
    }
    return TEXT("");
  }
  // corresponding param name to set in Weather Params material collection
  static const FName GetWeatherParamVectorName(
      EWeatherParamVector WeatherParam) {
    switch (WeatherParam) {
      case EWeatherParamVector::WEATHER_PARAM_VECTOR_WIND: {
        return TEXT("Wind");
        break;
      }
    }
    return TEXT("");
  }

  static UMaterialParameterCollectionInstance*
  getWeatherMaterialCollectionInstance(UWorld* World);

 public:
  // ActorsToAttachTo is an array of actors that we will attach weather
  // particles to in most cases, this will be the playable pawns so they will
  // always have weather fx
  UFUNCTION(BlueprintCallable, Category = Weather)
  static void initWeather(UWorld* World,
                          TArray<AUnrealRobot*> ActorsToAttachTo);

  UFUNCTION(BlueprintCallable, Category = Weather)
  static void unloadWeather(UWorld* World);

  /* only sets or gets one param. need any actor in the world for
   * WorldContextObject, so we can get world*/
  UFUNCTION(BlueprintCallable, Category = Weather)
  static bool setWeatherParamScalar(UWorld* World, EWeatherParamScalar Param,
                                    float Amount);

  UFUNCTION(BlueprintCallable, Category = Weather)
  static float getWeatherParamScalar(UWorld* World, EWeatherParamScalar Param);

  // only vector for now
  UFUNCTION(BlueprintCallable, Category = Weather)
  static FVector getWeatherWindDirection(UWorld* World);

  // in a range of (-1, -1, -1) to (1, 1, 1)
  UFUNCTION(BlueprintCallable, Category = Weather)
  static void setWeatherWindDirection(UWorld* World, FVector NewWind);

  UFUNCTION(BlueprintCallable, Category = Weather)
  static bool getIsWeatherEnabled(UWorld* World);

  UFUNCTION(BlueprintCallable, Category = Weather)
  static bool setWeatherEnabled(UWorld* World, bool bEnabled);

  UFUNCTION(BlueprintCallable, Category = Weather)
  static void resetWeatherParams(UWorld* World);

  UFUNCTION(BlueprintCallable, Category = Weather)
  static void showWeatherMenu(UWorld* World);

  UFUNCTION(BlueprintCallable, Category = Weather)
  static void hideWeatherMenu(UWorld* World);

  UFUNCTION(BlueprintCallable, Category = Weather)
  static bool isMenuVisible(UWorld* World);

  UFUNCTION(BlueprintCallable, Category = Weather)
  static void toggleWeatherMenu(UWorld* World);

  // blueprint callable function for widget to get world
  // since GetWorld() isn't exposed to bp
  // widget isnt a subclass of actor, so it needs its own implementation
  UFUNCTION(BlueprintCallable, Category = World)
  static UWorld* widgetGetWorld(UUserWidget* Widget);

  // blueprint callable function for actor to get world
  // since GetWorld() isn't exposed to bp
  UFUNCTION(BlueprintCallable, Category = World)
  static UWorld* actorGetWorld(AActor* Actor);

  UFUNCTION(BlueprintCallable, Category = Weather)
  static void setWeatherFog(AExponentialHeightFog* fog);

 private:
  static AExponentialHeightFog* weather_fog_;
};
