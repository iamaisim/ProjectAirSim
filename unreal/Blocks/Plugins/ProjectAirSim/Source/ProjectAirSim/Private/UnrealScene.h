// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once

#include <memory>

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Renderers/BlackSharkRenderer.hpp"
#include "Robot/UnrealEnvActor.h"
#include "Robot/UnrealEnvCar.h"
#include "Robot/UnrealEnvHuman.h"
#include "Robot/UnrealEnvParticleEffect.h"
#include "Robot/UnrealRobot.h"
#include "Sensors/UnrealViewportCamera.h"
#include "World/TimeofDay.hpp"
#include "World/WorldSimApi.h"
#include "core_sim/clock.hpp"
#include "core_sim/earth_utils.hpp"
#include "core_sim/scene.hpp"
#include "json.hpp"
#include "unreal_physics.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "UnrealScene.generated.h"

UCLASS()
class AUnrealScene : public AActor {
  GENERATED_BODY()

 public:
  explicit AUnrealScene(const FObjectInitializer& ObjectInitialize);

  void LoadUnrealScene(
      UWorld* World, microsoft::projectairsim::Scene& Scene,
      const std::unordered_map<std::string,
                               microsoft::projectairsim::UnrealPhysicsBody*>&
          UnrealPhysicsBodies);

  void UnloadUnrealScene();

  void SwitchStreamingView();

  void ToggleTrace();

  void SetTraceLine(const std::vector<float>& color_rgba, float thickness);

  void StartUnrealScene();

  void StopUnrealScene();

  void Tick(float DeltaTime) override;

  bool GetUnrealBoundingBox3D(
      const std::string& ObjectName,
      microsoft::projectairsim::BoxAlignment BoxAlignment,
      FOrientedBox& OutOrientedBox, FRotator& OutRotation) const;

  bool GetSimBoundingBox3D(const std::string& object_name,
                           microsoft::projectairsim::BoxAlignment box_alignment,
                           FOrientedBox& OutUnrealBox,
                           microsoft::projectairsim::BBox3D& OutSimBox) const;

  AActor* FindActor(const std::string& object_name) const;

  TMap<FString, AActor*> scene_object_map;
  TMap<FString, FAssetData> asset_map_;

 protected:
  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

 private:
  void LoadUnrealActor(
      UWorld* World, const microsoft::projectairsim::Actor& Actor,
      const std::unordered_map<std::string,
                               microsoft::projectairsim::UnrealPhysicsBody*>&
          UnrealPhysicsBodies);

  void LoadUnrealEnvActor(UWorld* World,
                          const microsoft::projectairsim::Actor& Actor);

  void LoadUnrealEnvCar(UWorld* World,
                            const microsoft::projectairsim::Actor& Actor);

  void LoadUnrealEnvHuman(UWorld* World,
                            const microsoft::projectairsim::Actor& Actor);

  void LoadUnrealEnvParticleEffect(UWorld* World,
                            const microsoft::projectairsim::Actor& Actor);            

  bool GetWorldAlignedBoundingBox3D(const std::string& ObjectName,
                                    FOrientedBox& OrientedBox) const;
  bool GetActorAlignedBoundingBox3D(const std::string& ObjectName,
                                    FOrientedBox& OrientedBox,
                                    FRotator& OutRotation) const;

  void RegisterServiceMethods();

  bool SwitchStreamingViewServiceMethod();

  bool SetTraceLineServiceMethod(const std::vector<float>& color_rgba,
                                 float thickness);

  bool ToggleTraceServiceMethod();

  void UpdateWindVelocity(const microsoft::projectairsim::Vector3& wind_vel);

  void EnableUnrealViewportCamera(bool enable);

  nlohmann::json Get3DBoundingBoxServiceMethod(const std::string& object_name,
                                               int box_alignment);

  UWorld* unreal_world;
  microsoft::projectairsim::Scene* sim_scene;
  microsoft::projectairsim::HomeGeoPoint home_geo_point;
  AUnrealViewportCamera* unreal_viewport_camera_;
  TArray<AUnrealRobot*> unreal_actors;
  TArray<AUnrealEnvActor*> unreal_env_actors;
  TArray<AUnrealEnvActorCar*> unreal_env_cars;
  TArray<AUnrealEnvHuman*> unreal_env_humans;
  TArray<AUnrealEnvParticleEffect*> unreal_env_particles;
  size_t idx_actor_to_view = 0;
  bool found_actor = false;

  FCriticalSection UpdateMutex;

  TimeNano unreal_time;
  bool using_unreal_physics;

  std::unique_ptr<WorldSimApi> world_api;
  std::shared_ptr<TimeOfDay> time_of_day;
  std::vector<std::string> objects_;

  UClass* sky_sphere_class_;
  TimeOfDaySetting tod_setting;
  std::shared_ptr<BlackSharkRenderer>  black_shark_renderer;

  // actor name -> scaled bbox in actor space
  mutable std::unordered_map<std::string, FBox> actor_bbox_cache;

  // trace path variables
  bool tracing_enabled = false;
  FColor trace_color = FColor::Purple;
  float trace_thickness = 3.0f;

  FVector last_position;
};
