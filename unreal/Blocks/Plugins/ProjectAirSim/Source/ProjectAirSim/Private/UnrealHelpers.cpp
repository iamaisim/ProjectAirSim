// Copyright (C) Microsoft Corporation.  All rights reserved.

#include "UnrealHelpers.h"

#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "LandscapeComponent.h"
#include "ProceduralMeshComponent.h"

namespace projectairsim = microsoft::projectairsim;

template <typename T>
T* UnrealHelpers::GetActorComponent(AActor* actor, FString name) {
  TArray<T*> components;
  actor->GetComponents(components);
  T* found = nullptr;
  for (T* component : components) {
    if (component->GetName().Compare(name) == 0) {
      found = component;
      break;
    }
  }
  return found;
}

bool UnrealHelpers::setSimulatePhysics(AActor* actor, bool simulate_physics) {
  bool success = false;
  if (actor) {
    TInlineComponentArray<UPrimitiveComponent*> components;
    actor->GetComponents(components);

    for (UPrimitiveComponent* component : components) {
      component->SetSimulatePhysics(simulate_physics);
    }
    success = true;
  }
  return success;
}

bool UnrealHelpers::GenerateAssetRegistryMap(
    TMap<FString, FAssetData>& asset_map) {
  bool success = false;
  UnrealHelpers::RunCommandOnGameThread(
      [&success, &asset_map]() {
        FARFilter Filter;
        Filter.ClassPaths.Add(UStaticMesh::StaticClass()->GetClassPathName());
        Filter.bRecursivePaths = true;

        TArray<FAssetData> AssetData;

        // Find mesh in /Game asset registry. When more plugins are
        // added this function might have to change
        FAssetRegistryModule& AssetRegistryModule =
            FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
                "AssetRegistry");
        AssetRegistryModule.Get().GetAssets(Filter, AssetData);

        for (const auto& asset : AssetData) {
          FString asset_name = asset.AssetName.ToString();
          asset_map.Add(asset_name, asset);
        }

        UnrealLogger::Log(projectairsim::LogLevel::kVerbose,
                          TEXT("Asset database ready."));
        success = true;
      },
      true);

  if (!success) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[UnrealHelpers] Unable to generate asset database!"));
  }
  return success;
}

bool UnrealHelpers::GenerateBlueprintRegistryMap(
    TMap<FString, FAssetData>& blueprint_map) {
  bool success = false;
  UnrealHelpers::RunCommandOnGameThread(
      [&success, &blueprint_map]() {
        FARFilter Filter;
        Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
        Filter.bRecursivePaths = true;

        TArray<FAssetData> AssetData;

        // Find mesh in /Game asset registry. When more plugins are
        // added this function might have to change
        FAssetRegistryModule& AssetRegistryModule =
            FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
                "AssetRegistry");
        AssetRegistryModule.Get().GetAssets(Filter, AssetData);

        for (const auto& asset : AssetData) {
          FString asset_name = asset.AssetName.ToString();
          blueprint_map.Add(asset_name, asset);
        }

        UnrealLogger::Log(projectairsim::LogLevel::kVerbose,
                          TEXT("Blueprint database ready."));
        success = true;
      },
      true);

  if (!success) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[UnrealHelpers] Unable to generate blueprint database!"));
  }
  return success;
}

bool UnrealHelpers::GenerateSkeletalMeshRegistryMap(
    TMap<FString, FAssetData>& skeletal_mesh_map) {
  bool success = false;
  UnrealHelpers::RunCommandOnGameThread(
      [&success, &skeletal_mesh_map]() {
        FARFilter Filter;
        Filter.ClassPaths.Add(USkeletalMesh::StaticClass()->GetClassPathName());
        Filter.bRecursivePaths = true;

        TArray<FAssetData> AssetData;

        // Find mesh in /Game asset registry. When more plugins are
        // added this function might have to change
        FAssetRegistryModule& AssetRegistryModule =
            FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
                "AssetRegistry");
        AssetRegistryModule.Get().GetAssets(Filter, AssetData);

        for (const auto& asset : AssetData) {
          FString asset_name = asset.AssetName.ToString();
          skeletal_mesh_map.Add(asset_name, asset);
        }

        UnrealLogger::Log(projectairsim::LogLevel::kVerbose,
                          TEXT("Skeletal mesh database ready."));
        success = true;
      },
      true);

  if (!success) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[UnrealHelpers] Unable to generate skeletal mesh database!"));
  }
  return success;
}

bool UnrealHelpers::GenerateActorMap(UWorld* unreal_world,
                                     TMap<FString, AActor*>& scene_object_map) {
  bool success = false;
  if (unreal_world) {
    for (TActorIterator<AActor> actorIterator(unreal_world); actorIterator;
         ++actorIterator) {
      AActor* actor = *actorIterator;
      FString name = *actor->GetName();

      scene_object_map.Add(name, actor);
    }
    success = true;
  } else {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[UnrealHelpers] Unable to generate actor database!"));
  }
  return success;
}

std::vector<std::string> UnrealHelpers::ListMatchingActors(
    UWorld* unreal_world, const std::string& name_regex) {
  std::vector<std::string> results;
  std::regex compiledRegex(name_regex, std::regex::optimize);

  // Override FActorRange/FActorIterator's default InFlags parameter to disable
  // EActorIteratorFlags::OnlyActiveLevels which can return 0 actors if the
  // world happens to be ticking the StaticLevels level collection as the active
  // level instead of the DynamicSourceLevels where all of the actors are
  for (AActor* actor :
       FActorRange(unreal_world, EActorIteratorFlags::SkipPendingKill)) {
    auto name = std::string(TCHAR_TO_UTF8(*actor->GetName()));
    bool match = std::regex_match(name, compiledRegex);
    if (match) results.push_back(name);
  }

  return results;
}

std::vector<std::string> UnrealHelpers::ListMatchingAssets(
    UWorld* unreal_world, const std::string& name_regex,
    TMap<FString, FAssetData>& asset_map,
    TMap<FString, FAssetData>& blueprint_map,
    TMap<FString, FAssetData>& skeletal_mesh_map) {
  std::vector<std::string> Results;
  std::regex CompiledRegex(name_regex, std::regex::optimize);

  UObject* LoadObject = NULL;
  for (auto& Asset : asset_map) {
    auto Name = std::string(TCHAR_TO_UTF8(*Asset.Value.AssetName.ToString()));
    bool bMatch = std::regex_match(Name, CompiledRegex);
    if (bMatch) {
      Results.push_back(Name);
    }
  }
  for (auto& Asset : blueprint_map) {
    auto Name = std::string(TCHAR_TO_UTF8(*Asset.Value.AssetName.ToString()));
    bool bMatch = std::regex_match(Name, CompiledRegex);
    if (bMatch) {
      Results.push_back(Name);
    }
  }
  for (auto& Asset : skeletal_mesh_map) {
    auto Name = std::string(TCHAR_TO_UTF8(*Asset.Value.AssetName.ToString()));
    bool bMatch = std::regex_match(Name, CompiledRegex);
    if (bMatch) {
      Results.push_back(Name);
    }
  }
  return Results;
}

void UnrealHelpers::RunCommandOnGameThread(TFunction<void()> InFunction,
                                           bool wait, const TStatId InStatId) {
  if (IsInGameThread())
    InFunction();
  else {
    FGraphEventRef task = FFunctionGraphTask::CreateAndDispatchWhenReady(
        MoveTemp(InFunction), InStatId, nullptr, ENamedThreads::GameThread);
    if (wait) FTaskGraphInterface::Get().WaitUntilTaskCompletes(task);
  }
}

void UnrealHelpers::OnScreenLogMessageString(const std::string& prefix,
                                             const std::string& suffix,
                                             LogDebugLevel level,
                                             float persist_sec) {
  OnScreenLogMessage(FString(prefix.c_str()), FString(suffix.c_str()), level,
                     persist_sec);
}

void UnrealHelpers::OnScreenLogMessage(const FString& prefix,
                                       const FString& suffix,
                                       LogDebugLevel level, float persist_sec) {
  // if (log_messages_hidden_) return; // TO DO

  static TMap<FString, int> loggingKeys;
  static int counter = 1;

  int key = loggingKeys.FindOrAdd(prefix);
  if (key == 0) {
    key = counter++;
    loggingKeys[prefix] = key;
  }

  FColor color;
  switch (level) {
    case LogDebugLevel::Informational:
      color = FColor(147, 231, 237);
      UE_LOG(LogTemp, Log, TEXT("%s%s"), *prefix, *suffix);
      break;
    case LogDebugLevel::Success:
      color = FColor(156, 237, 147);
      UE_LOG(LogTemp, Log, TEXT("%s%s"), *prefix, *suffix);
      break;
    case LogDebugLevel::Failure:
      color = FColor(237, 147, 168);
      UE_LOG(LogTemp, Error, TEXT("%s%s"), *prefix, *suffix);
      break;
    case LogDebugLevel::Unimportant:
      color = FColor(237, 228, 147);
      UE_LOG(LogTemp, Verbose, TEXT("%s%s"), *prefix, *suffix);
      break;
    default:
      color = FColor::Black;
      break;
  }
  if (GEngine) {
    GEngine->AddOnScreenDebugMessage(key, persist_sec, color, prefix + suffix);
  }
}

UObject* UnrealHelpers::GetMeshFromRegistry(const std::string& load_object) {
  FARFilter Filter;
  Filter.ClassPaths.Add(UStaticMesh::StaticClass()->GetClassPathName());
  Filter.bRecursivePaths = true;

  TArray<FAssetData> AssetData;
  FAssetRegistryModule& AssetRegistryModule =
      FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
  AssetRegistryModule.Get().GetAssets(Filter, AssetData);

  UObject* LoadObject = NULL;
  for (auto asset : AssetData) {
    UE_LOG(LogTemp, Log, TEXT("Asset path: %s"), *asset.PackagePath.ToString());
    if (asset.AssetName == FName(load_object.c_str())) {
      LoadObject = asset.GetAsset();
      break;
    }
  }
  return LoadObject;
}

FVector UnrealHelpers::ToFVector(const projectairsim::Vector3& vec) {
  return FVector(vec.x(), vec.y(), vec.z());
}

FVector UnrealHelpers::ToFVectorTranslation(
    const projectairsim::Vector3& vec_ned) {
  return FVector(vec_ned.x() * 100, vec_ned.y() * 100,
                 -vec_ned.z() * 100);  // NED m -> NEU cm
}

projectairsim::Vector3 UnrealHelpers::ToVector3(const FVector& vec) {
  return projectairsim::Vector3(vec.X, vec.Y, vec.Z);
}

FQuat UnrealHelpers::ToFQuat(const projectairsim::Quaternion& quat) {
  return FQuat(quat.x(), quat.y(), quat.z(), quat.w());
}

FTransform UnrealHelpers::ToFTransform(
    const microsoft::projectairsim::Affine3& affine3) {
  const auto& mat = affine3.matrix();
  FVector vec_x(mat(0, 0), mat(1, 0), mat(2, 0));
  FVector vec_y(mat(0, 1), mat(1, 1), mat(2, 1));
  FVector vec_z(mat(0, 2), mat(1, 2), mat(2, 2));
  FVector vec_translation(mat(0, 3), mat(1, 3), mat(2, 3));

  return (FTransform(vec_x, vec_y, vec_z, vec_translation));
}

projectairsim::Quaternion UnrealHelpers::ToQuaternion(const FQuat& quat) {
  return projectairsim::Quaternion(quat.W, quat.X, quat.Y, quat.Z);
}

projectairsim::Quaternion UnrealHelpers::ToQuaternion(const FRotator& rot) {
  float roll_rad = projectairsim::TransformUtils::ToRadians(rot.Roll);
  float pitch_rad = projectairsim::TransformUtils::ToRadians(rot.Pitch);
  float yaw_rad = projectairsim::TransformUtils::ToRadians(rot.Yaw);
  return projectairsim::TransformUtils::ToQuaternion(roll_rad, pitch_rad,
                                                     yaw_rad);
}

FRotator UnrealHelpers::ToFRotator(const projectairsim::Vector3& rpy) {
  float roll_deg = projectairsim::TransformUtils::ToDegrees<float>(rpy.x());
  float pitch_deg = projectairsim::TransformUtils::ToDegrees<float>(rpy.y());
  float yaw_deg = projectairsim::TransformUtils::ToDegrees<float>(rpy.z());
  return FRotator(pitch_deg, yaw_deg, roll_deg);
}

FRotator UnrealHelpers::ToFRotator(const projectairsim::Quaternion& quat) {
  return ToFRotator(projectairsim::TransformUtils::ToRPY(quat));
}

void UnrealHelpers::SetActorName(AActor* actor, const std::string& new_name) {
  if (actor == nullptr) return;

  FString NewName(UTF8_TO_TCHAR(new_name.c_str()));

  // Test if actor can be renamed to the new name (not used by existing actor)
  bool bCanRename = actor->Rename(*NewName, nullptr, REN_Test);

  // If new name is not unique, make a unique version to do the renaming
  if (!bCanRename) {
    NewName =
        MakeUniqueObjectName(actor, actor->GetClass(), *NewName).ToString();
  }

  // Rename actor
  actor->Rename(*NewName);

#if WITH_EDITORONLY_DATA
  // Set the label in the Editor tree
  actor->SetActorLabel(*NewName);
#endif

  UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                    TEXT("[UnrealHelpers] SetActorName() to '%s'."), *NewName);
}

FString UnrealHelpers::GetSegmentationName(UProceduralMeshComponent* mesh,
                                           bool /*use_owner_name*/) {
  if (mesh == nullptr) return "";
  if (mesh->GetOwner() == nullptr) return "";

  // UProceduralMeshComponent names are not set directly since their data is
  // managed at runtime by their owner AProcMeshActor, so always use the
  // owner's name for segmentation name matching.
  return mesh->GetOwner()->GetName();
}

FString UnrealHelpers::GetSegmentationName(UStaticMeshComponent* mesh,
                                           bool use_owner_name) {
  if (mesh == nullptr) return "";

  if (use_owner_name && mesh->GetOwner()) {
    return mesh->GetOwner()->GetName();
  } else {
    return mesh->GetStaticMesh() ? mesh->GetStaticMesh()->GetName() : "";
  }
}

FString UnrealHelpers::GetSegmentationName(USkinnedMeshComponent* mesh,
                                           bool use_owner_name) {
  if (mesh == nullptr) return "";

  if (use_owner_name && mesh->GetOwner()) {
    return mesh->GetOwner()->GetName();
  } else {
    return mesh->GetSkinnedAsset() ? mesh->GetSkinnedAsset()->GetName() : "";
  }
}

FString UnrealHelpers::GetSegmentationName(USkeletalMeshComponent* mesh,
                                           bool use_owner_name) {
  if (mesh == nullptr) return "";

  if (use_owner_name && mesh->GetOwner()) {
    return mesh->GetOwner()->GetName();
  } else {
    return mesh->GetSkinnedAsset() ? mesh->GetSkinnedAsset()->GetName() : "";
  }
}

FString UnrealHelpers::GetSegmentationName(UParticleSystemComponent* mesh,
                                           bool use_owner_name) {
  if (mesh == nullptr) return "";

  if (use_owner_name && mesh->GetOwner()) {
    return mesh->GetOwner()->GetName();
  } else {
    return mesh->Template ? mesh->Template->GetName() : "";
  }
}

FString UnrealHelpers::GetSegmentationName(UNiagaraComponent* mesh,
                                           bool use_owner_name) {
  if (mesh == nullptr) return "";

  if (use_owner_name && mesh->GetOwner()) {
    return mesh->GetOwner()->GetName();
  } else {
    return mesh->GetAsset() ? mesh->GetAsset()->GetName() : "";
  }
}

FString UnrealHelpers::GetSegmentationName(ALandscapeProxy* mesh,
                                           bool use_owner_name) {
  return mesh ? mesh->GetName() : "";
}

void UnrealHelpers::SetSegmentationID(UPrimitiveComponent* mesh,
                                      int segmentation_id) {
  if (segmentation_id < 0) {
    mesh->SetRenderCustomDepth(false);
  } else {
    mesh->SetCustomDepthStencilValue(segmentation_id);
    mesh->SetRenderCustomDepth(true);
  }
}

void UnrealHelpers::SetSegmentationID(ALandscapeProxy* mesh,
                                      int segmentation_id) {
  if (segmentation_id < 0) {
    mesh->bRenderCustomDepth = false;
  } else {
    mesh->CustomDepthStencilValue = segmentation_id;
    mesh->bRenderCustomDepth = true;
  }

  // Explicitly set the custom depth state on the components so the
  // render state is marked dirty and the update actually takes effect
  // immediately.
  for (ULandscapeComponent* comp : mesh->LandscapeComponents) {
    SetSegmentationID(Cast<UPrimitiveComponent>(comp), segmentation_id);
  }
}

bool UnrealHelpers::SetSegmentationIDByName(
    const std::string& mesh_name, int segmentation_id, bool is_name_regex,
    bool use_owner_name, TMap<FString, int>& seg_name_to_id) {
  std::regex name_regex;

  if (is_name_regex) {
    // Set up case-insensitive regex match
    name_regex.assign(mesh_name, std::regex_constants::icase);
  }

  int changes = 0;
  for (TObjectIterator<UProceduralMeshComponent> comp; comp; ++comp) {
    SetSegmentationIDIfMatch(*comp, segmentation_id, mesh_name, is_name_regex,
                             name_regex, changes, use_owner_name,
                             seg_name_to_id);
  }
  for (TObjectIterator<UStaticMeshComponent> comp; comp; ++comp) {
    SetSegmentationIDIfMatch(*comp, segmentation_id, mesh_name, is_name_regex,
                             name_regex, changes, use_owner_name,
                             seg_name_to_id);
  }
  for (TObjectIterator<USkinnedMeshComponent> comp; comp; ++comp) {
    SetSegmentationIDIfMatch(*comp, segmentation_id, mesh_name, is_name_regex,
                             name_regex, changes, use_owner_name,
                             seg_name_to_id);
  }
  for (TObjectIterator<USkeletalMeshComponent> comp; comp; ++comp) {
    SetSegmentationIDIfMatch(*comp, segmentation_id, mesh_name, is_name_regex,
                             name_regex, changes, use_owner_name,
                             seg_name_to_id);
  }
  for (TObjectIterator<UParticleSystemComponent> comp; comp; ++comp) {
    SetSegmentationIDIfMatch(*comp, segmentation_id, mesh_name, is_name_regex,
                             name_regex, changes, use_owner_name,
                             seg_name_to_id);
  }
  for (TObjectIterator<UNiagaraComponent> comp; comp; ++comp) {
    SetSegmentationIDIfMatch(*comp, segmentation_id, mesh_name, is_name_regex,
                             name_regex, changes, use_owner_name,
                             seg_name_to_id);
  }
  for (TObjectIterator<ALandscapeProxy> comp; comp; ++comp) {
    SetSegmentationIDIfMatch(*comp, segmentation_id, mesh_name, is_name_regex,
                             name_regex, changes, use_owner_name,
                             seg_name_to_id);
  }

  return changes > 0;
}

TimeNano UnrealHelpers::DeltaTimeToNanos(float DeltaTime) {
  double dt_round_to_microsec =
      static_cast<int64_t>(DeltaTime * 1e6 + 0.5) / 1.0e6;
  TimeNano dt_nanos = static_cast<TimeNano>(dt_round_to_microsec * 1.0e9);
  return dt_nanos;
}

void UnrealHelpers::ForceUnrealGarbageCollection() {
  UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                    TEXT("[UnrealHelpers] Forcing Unreal garbage collection "
                         "before loading new scene."));
  // Store last GC frame counter
  auto prev_gc_frame = GLastGCFrame;

  // Set request for Unreal garbage collection to clear destroyed actors
  RunCommandOnGameThread([]() { GEngine->ForceGarbageCollection(true); }, true);

  if (IsInGameThread()) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kWarning,
        TEXT("[UnrealHelpers] ForceUnrealGarbageCollection() was called on the "
             "game thread, so unable to wait until it completes before "
             "continuing. GC will be done at the next chance Unreal allows."));
  } else {
    // Wait until GC has actually occurred by watching update to GLastGCFrame
    while (GLastGCFrame == prev_gc_frame) {
      std::this_thread::yield();
    }

    UnrealLogger::Log(
        projectairsim::LogLevel::kTrace,
        TEXT("[UnrealHelpers] Forced Unreal garbage collection is complete."));
  }
}


bool UnrealHelpers::SetDoublePropertyValue(AActor* actor, FName propertyName,
                                           double value) {
  FDoubleProperty* DoubleProp =
      FindFProperty<FDoubleProperty>(actor->GetClass(), propertyName);
  if (DoubleProp != NULL) {
    DoubleProp->SetPropertyValue_InContainer(actor, value);
  } else {
    return false;
  }
  return true;
}

bool UnrealHelpers::CallFunction(AActor* actor, FName functionName) {
  auto function = actor->GetClass()->FindFunctionByName(functionName);
  if (function != NULL) {
    actor->ProcessEvent(function, nullptr);
  } else {
    return false;
  }
  return true;
}

double UnrealHelpers::GetDoublePropertyValue(AActor* actor, FName propertyName,
                                            double default_val) {
  FDoubleProperty* DoubleProp =
      FindFProperty<FDoubleProperty>(actor->GetClass(), propertyName);
  if (DoubleProp != NULL) {
    auto value = DoubleProp->GetPropertyValue_InContainer(actor);
    return value;
  }
  return default_val;
}

bool UnrealHelpers::SetIntPropertyValue(AActor* actor, FName propertyName,
                                        int value) {
  FIntProperty* IntProp =
      FindFProperty<FIntProperty>(actor->GetClass(), propertyName);
  if (IntProp != NULL) {
    IntProp->SetPropertyValue_InContainer(actor, value);
  } else {
    return false;
  }
  return true;
}

int UnrealHelpers::GetIntPropertyValue(AActor* actor, FName propertyName,
                                       int default_val) {
  FIntProperty* IntProp =
      FindFProperty<FIntProperty>(actor->GetClass(), propertyName);
  if (IntProp != NULL) {
    auto value = IntProp->GetPropertyValue_InContainer(actor);
    return value;
  }
  return default_val;
}
