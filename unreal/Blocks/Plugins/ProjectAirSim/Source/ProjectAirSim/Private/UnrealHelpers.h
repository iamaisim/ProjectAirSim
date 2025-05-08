// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <regex>
#include <string>

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/InputComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerInput.h"
#include "IImageWrapperModule.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "LandscapeProxy.h"
#include "NiagaraComponent.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "UnrealLogger.h"
#include "core_sim/transforms/transform_utils.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "UnrealHelpers.generated.h"

UENUM(BlueprintType)
enum class LogDebugLevel : uint8 {
  Informational UMETA(DisplayName = "Informational"),
  Success UMETA(DisplayName = "Success"),
  Failure UMETA(DisplayName = "Failure"),
  Unimportant UMETA(DisplayName = "Unimportant")
};

UCLASS()
class UnrealHelpers : public UBlueprintFunctionLibrary {
  GENERATED_BODY()

 public:
  template <typename T>
  static T* GetActorComponent(AActor* actor, FString name);

  template <typename T>
  static T* FindActor(
      const UObject* context, FString name,
      const TMap<FString, AActor*>* SceneObjectMapPtr = nullptr) {
    // First, check if name is a key in the scene object map
    if (SceneObjectMapPtr) {
      auto ActorLookupResult = SceneObjectMapPtr->Find(name);
      if (ActorLookupResult) {
        return static_cast<T*>(*ActorLookupResult);
      }
    }

    // Second, loop through all actors of the specified class and find all
    // matches that contain the name or have it as a tag
    TArray<AActor*> AllActorsMatchingClass;
    FindAllActors<T>(context, AllActorsMatchingClass);

    TArray<AActor*> AllActorsMatchingName;
    for (AActor* CurActor : AllActorsMatchingClass) {
      FString CurActorName = CurActor->GetName();
      if (CurActorName.Contains(name) || CurActor->ActorHasTag(FName(*name))) {
        AllActorsMatchingName.Add(CurActor);
      }
    }

    if (AllActorsMatchingName.Num() == 0) {
      return nullptr;
    } else {
      if (AllActorsMatchingName.Num() > 1) {
        UnrealLogger::Log(microsoft::projectairsim::LogLevel::kWarning,
                          TEXT("[FindActor] Multiple actors found matching "
                               "name: %s. Returning the first one."),
                          *name);
        for (int Idx = 0; Idx < AllActorsMatchingName.Num(); ++Idx) {
          UnrealLogger::Log(microsoft::projectairsim::LogLevel::kWarning,
                            TEXT("[FindActor]  -> found actor #%d: %s"),
                            Idx + 1, *(AllActorsMatchingName[Idx]->GetName()));
        }
      }
      return static_cast<T*>(AllActorsMatchingName[0]);
    }
  }

  template <typename T>
  static void FindAllActors(const UObject* context,
                            TArray<AActor*>& foundActors) {
    UGameplayStatics::GetAllActorsOfClass(context, T::StaticClass(),
                                          foundActors);
  }

  static std::vector<std::string> ListMatchingActors(
      UWorld* unreal_world, const std::string& name_regex);

  static std::vector<std::string> ListMatchingAssets(
      UWorld* unreal_world, const std::string& name_regex,
      TMap<FString, FAssetData>& asset_map,
      TMap<FString, FAssetData>& blueprint_map,
      TMap<FString, FAssetData>& skeletal_mesh_map);

  static UObject* GetMeshFromRegistry(const std::string& load_object);

  static bool IsInGameThread() { return ::IsInGameThread(); }

  static void RunCommandOnGameThread(TFunction<void()> InFunction,
                                     bool wait = false,
                                     const TStatId InStatId = TStatId());

  static void OnScreenLogMessageString(const std::string& prefix,
                                       const std::string& suffix,
                                       LogDebugLevel level,
                                       float persist_sec = 60);
  UFUNCTION(BlueprintCallable, Category = "Utils")
  static void OnScreenLogMessage(const FString& prefix, const FString& suffix,
                                 LogDebugLevel level, float persist_sec = 60);

  template <typename E>
  static constexpr typename std::underlying_type<E>::type toNumeric(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
  }
  template <typename E>
  static constexpr E toEnum(typename std::underlying_type<E>::type u) {
    return static_cast<E>(u);
  }

  static bool GenerateActorMap(UWorld* unreal_world,
                               TMap<FString, AActor*>& scene_object_map);

  static bool GenerateAssetRegistryMap(TMap<FString, FAssetData>& asset_map);

  static bool GenerateBlueprintRegistryMap(
      TMap<FString, FAssetData>& blueprint_map);

  static bool GenerateSkeletalMeshRegistryMap(
      TMap<FString, FAssetData>& skeletal_mesh_map);

  static bool setSimulatePhysics(AActor* actor, bool simulate_physics);

  static FVector ToFVector(const microsoft::projectairsim::Vector3& vec);

  static FVector ToFVectorTranslation(
      const microsoft::projectairsim::Vector3& vec);

  static microsoft::projectairsim::Vector3 ToVector3(const FVector& vec);

  static FQuat ToFQuat(const microsoft::projectairsim::Quaternion& quat);

  static microsoft::projectairsim::Quaternion ToQuaternion(const FQuat& quat);

  static microsoft::projectairsim::Quaternion ToQuaternion(const FRotator& rot);

  static FRotator ToFRotator(const microsoft::projectairsim::Vector3& rpy);

  static FRotator ToFRotator(const microsoft::projectairsim::Quaternion& quat);

  static FTransform ToFTransform(
      const microsoft::projectairsim::Affine3& affine3);

  static void SetActorName(AActor* actor, const std::string& new_name);

  template <typename T>
  static void InitSegmentationID(T* mesh, bool ignore_existing,
                                 bool use_owner_name,
                                 TMap<FString, int>& seg_name_to_id) {
    FString seg_name = GetSegmentationName(mesh, use_owner_name).ToLower();

    if (seg_name.IsEmpty() || seg_name.StartsWith("default_")) {
      return;
    }

    // Calculate a segmentation ID from a hash for each mesh's base name chars
    int seg_id = 5;
    for (int idx = 0; idx < seg_name.Len(); ++idx) {
      auto char_num = UKismetStringLibrary::GetCharacterAsNumber(seg_name, idx);
      if (char_num < 97) continue;  // ignore numerics and other punctuations
      seg_id += char_num;
    }
    seg_id %= 256;

    // Set object's segmentation ID if existing value isn't already enabled, or
    // the ignore_existing option is used to set everything
    if (!seg_name_to_id.Contains(seg_name) || ignore_existing) {
      UnrealLogger::Log(
          microsoft::projectairsim::LogLevel::kTrace,
          TEXT("[InitSegmentationID] seg_id: %3d, seg_name: '%s'"), seg_id,
          *seg_name);
      SetSegmentationID(mesh, seg_id);
      seg_name_to_id.Add(seg_name, seg_id);
    }
  }

  static FString GetSegmentationName(UProceduralMeshComponent* mesh,
                                     bool use_owner_name);

  static FString GetSegmentationName(UStaticMeshComponent* mesh,
                                     bool use_owner_name);

  static FString GetSegmentationName(USkinnedMeshComponent* mesh,
                                     bool use_owner_name);

  static FString GetSegmentationName(USkeletalMeshComponent* mesh,
                                     bool use_owner_name);

  static FString GetSegmentationName(UParticleSystemComponent* mesh,
                                     bool use_owner_name);

  static FString GetSegmentationName(UNiagaraComponent* mesh,
                                     bool use_owner_name);

  static FString GetSegmentationName(ALandscapeProxy* mesh,
                                     bool use_owner_name);

  static void SetSegmentationID(UPrimitiveComponent* mesh, int segmentation_id);

  static void SetSegmentationID(ALandscapeProxy* mesh, int segmentation_id);

  template <typename T>
  static void SetSegmentationIDIfMatch(T* mesh, int segmentation_id,
                                       const std::string& mesh_name,
                                       bool is_name_regex,
                                       const std::regex& name_regex,
                                       int& changes, bool use_owner_name,
                                       TMap<FString, int>& seg_name_to_id) {
    auto seg_name_fstr = GetSegmentationName(mesh, use_owner_name);
    std::string comp_mesh_name(TCHAR_TO_UTF8(*seg_name_fstr));
    if (comp_mesh_name == "") return;

    bool is_match =
        ((!is_name_regex && comp_mesh_name == mesh_name) ||
         (is_name_regex && std::regex_match(comp_mesh_name, name_regex)));

    if (is_match) {
      ++changes;
      SetSegmentationID(mesh, segmentation_id);
      seg_name_to_id.Add(seg_name_fstr, segmentation_id);
    }
  }

  static bool SetSegmentationIDByName(const std::string& mesh_name,
                                      int segmentation_id, bool is_name_regex,
                                      bool use_owner_name,
                                      TMap<FString, int>& seg_name_to_id);

  static TimeNano DeltaTimeToNanos(float DeltaTime);

  static void ForceUnrealGarbageCollection();

  // Function for interacting with Unreal Objects

  static bool SetDoublePropertyValue(AActor* actor, FName propertyName,
                                     double value);

  static bool CallFunction(AActor* actor, FName functionName);

  static double GetDoublePropertyValue(AActor* actor, FName propertyName,
                                      double default_val);

  static bool SetIntPropertyValue(AActor* actor, FName propertyName, int value);

  static int GetIntPropertyValue(AActor* actor, FName propertyName,
                                 int default_val);
};
