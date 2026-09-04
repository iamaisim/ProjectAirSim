// Copyright (C) Microsoft Corporation.
// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "UnrealScene.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <limits>

#include "Chaos/SimCallbackObject.h"
#include "CineCameraActor.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/EngineVersionComparison.h"
#include "PBDRigidsSolver.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "ProjectAirSim.h"
#include "Robot/UnrealEnvActor.h"
#include "Robot/UnrealRobot.h"
#include "Sensors/UnrealCamera.h"
#include "UObject/ConstructorHelpers.h"
#include "UnrealHelpers.h"
#include "UnrealLogger.h"
#include "World/WeatherLib.h"
#include "World/WorldSimApi.h"
#include "core_sim/actor/robot.hpp"
#include "core_sim/clock.hpp"

namespace projectairsim = microsoft::projectairsim;

#if UE_VERSION_OLDER_THAN(5, 7, 0)
constexpr Chaos::ESimCallbackOptions kProjectAirSimSceneCallbackOptions =
    Chaos::ESimCallbackOptions::Presimulate;
#else
constexpr Chaos::ESimCallbackOptions kProjectAirSimSceneCallbackOptions =
    Chaos::ESimCallbackOptions::Presimulate |
    Chaos::ESimCallbackOptions::PostSolve;
#endif

struct FProjectAirSimSceneSubstepInput : public Chaos::FSimCallbackInput {
  void Reset() {}
};

struct FProjectAirSimSceneSubstepOutput : public Chaos::FSimCallbackOutput {
  TArray<float> DeltaSeconds;

  void Reset() { DeltaSeconds.Reset(); }
};

class FProjectAirSimSceneSubstepCallback
    : public Chaos::TSimCallbackObject<FProjectAirSimSceneSubstepInput,
                                       FProjectAirSimSceneSubstepOutput,
                                       kProjectAirSimSceneCallbackOptions> {
 protected:
  void OnPreSimulate_Internal() override {
#if UE_VERSION_OLDER_THAN(5, 7, 0)
    CaptureDelta_Internal();
#endif
  }

#if !UE_VERSION_OLDER_THAN(5, 7, 0)
  void OnPostSolve_Internal() override { CaptureDelta_Internal(); }
#endif

 private:
  void CaptureDelta_Internal() {
    const float DeltaSeconds = GetDeltaTime_Internal();
    if (DeltaSeconds > 0.0f) {
      GetProducerOutputData_Internal().DeltaSeconds.Add(DeltaSeconds);
    }
  }
};

class FProjectAirSimSceneSubstepSampler {
 public:
  ~FProjectAirSimSceneSubstepSampler() { Stop(); }

  bool Start(UWorld* World) {
    if (Callback != nullptr) return true;
    if (World == nullptr) return false;

    FPhysScene_Chaos* PhysicsScene = World->GetPhysicsScene();
    if (PhysicsScene == nullptr) return false;

    Solver = PhysicsScene->GetSolver();
    if (Solver == nullptr) return false;

    Callback = Solver->CreateAndRegisterSimCallbackObject_External<
        FProjectAirSimSceneSubstepCallback>();
    if (Callback == nullptr) {
      Solver = nullptr;
      return false;
    }
    return true;
  }

  void Stop() {
    if (Callback != nullptr && Solver != nullptr) {
      Solver->UnregisterAndFreeSimCallbackObject_External(Callback);
    }
    Callback = nullptr;
    Solver = nullptr;
    PendingDeltas.clear();
  }

  bool PopNextDelta(TimeNano& OutDeltaNanos) {
    DrainOutputs();
    if (PendingDeltas.empty()) return false;

    OutDeltaNanos = PendingDeltas.front();
    PendingDeltas.pop_front();
    return true;
  }

 private:
  void DrainOutputs() {
    if (Callback == nullptr) return;

    while (auto Output = Callback->PopFutureOutputData_External()) {
      for (const float DeltaSeconds : Output->DeltaSeconds) {
        if (DeltaSeconds <= 0.0f) continue;
        PendingDeltas.push_back(std::max<TimeNano>(
            1, static_cast<TimeNano>(std::llround(DeltaSeconds * 1.0e9))));
      }
    }

  }

  Chaos::FPhysicsSolver* Solver = nullptr;
  FProjectAirSimSceneSubstepCallback* Callback = nullptr;
  std::deque<TimeNano> PendingDeltas;
};

AUnrealScene::AUnrealScene(const FObjectInitializer& ObjectInitialize)
    : AActor(ObjectInitialize) {
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.bTickEvenWhenPaused = true;  // see comment on Tick()
  // UnrealScene ticks after physics to handle pause/unpause for next cycle
  PrimaryActorTick.TickGroup = TG_PostUpdateWork;

  // Add InputComponent for keyboard input bindings
  InputComponent = CreateDefaultSubobject<UInputComponent>("UnrealScene");
}

AUnrealScene::~AUnrealScene() { delete scene_substep_sampler_; }

void AUnrealScene::LoadUnrealScene(
    UWorld* World, projectairsim::Scene& Scene,
    const std::unordered_map<std::string, projectairsim::UnrealPhysicsBody*>&
        UnrealPhysicsBodies) {
  UnrealHelpers::SetActorName(this, "UnrealScene");
  const auto Actors = Scene.GetActors();
  using_unreal_vehicle_physics = std::any_of(
      Actors.begin(), Actors.end(),
      [](const std::reference_wrapper<projectairsim::Actor>& Actor) {
        return Actor.get().GetType() == projectairsim::ActorType::kRobot &&
               !static_cast<const projectairsim::Robot&>(Actor.get())
                    .GetUnrealVehicleClass()
                    .empty();
      });
  using_unreal_physics =
      !UnrealPhysicsBodies.empty() || using_unreal_vehicle_physics;

  const auto& ClockSettings = Scene.GetClockSettings();
  using_unreal_physics_substep_clock =
      ClockSettings.type == projectairsim::ClockType::kEngineDriven &&
      ClockSettings.engine_substepping;
  UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
  PhysicsSettings->bSubstepping = using_unreal_physics_substep_clock;
  if (using_unreal_physics_substep_clock) {
    PhysicsSettings->MaxSubstepDeltaTime =
        static_cast<float>(ClockSettings.step / 1.0e9);
    // UPhysicsSettings clamps MaxSubsteps to 16 in Unreal Engine 5.7.
    PhysicsSettings->MaxSubsteps = 16;
  } else if (ClockSettings.engine_substepping) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kWarning,
        TEXT("[UnrealScene] clock.engine-substepping is only supported by "
             "the engine-driven clock and will be ignored."));
  }

  // Load UnrealRobot actors for each robot in sim scene
  std::for_each(Actors.begin(), Actors.end(),
                [this, World, UnrealPhysicsBodies](
                    const std::reference_wrapper<projectairsim::Actor> actor) {
                  LoadUnrealActor(World, actor.get(), UnrealPhysicsBodies);
                });

  // Load UnrealEnvActor actors for each env actor in sim scene
  auto env_actors = Scene.GetEnvActors();
  std::for_each(
      env_actors.begin(), env_actors.end(),
      [this,
       World](const std::reference_wrapper<projectairsim::Actor> env_actor) {
        LoadUnrealEnvActor(World, env_actor.get());
      });

  unreal_world = World;
  sim_scene = &Scene;

  RegisterServiceMethods();
}

void AUnrealScene::LoadUnrealActor(
    UWorld* World, const projectairsim::Actor& Actor,
    const std::unordered_map<std::string, projectairsim::UnrealPhysicsBody*>&
        UnrealPhysicsBodies) {
  UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                    TEXT("[UnrealScene] Loading Unreal actor %S."),
                    Actor.GetID().c_str());

  if (Actor.GetType() == projectairsim::ActorType::kRobot) {
    const auto& sim_robot = static_cast<const projectairsim::Robot&>(Actor);

    home_geo_point = sim_robot.GetEnvironment().home_geo_point;

    // Load sim robot into UE rendering world
    auto xyz_ned = sim_robot.GetOrigin().translation_;
    FVector translation(xyz_ned.x() * 100, xyz_ned.y() * 100,
                        -xyz_ned.z() * 100);  // NED m -> NEU cm

    auto rpy = sim_robot.GetOrigin().rotation_;
    FQuat rotation(rpy.x(), rpy.y(), rpy.z(), rpy.w());

    FTransform transform(rotation, translation);

    auto unreal_robot = World->SpawnActorDeferred<AUnrealRobot>(
        AUnrealRobot::StaticClass(), transform, nullptr, nullptr,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

    if (unreal_robot != nullptr) {
      // Check for UnrealPhysicsBody ptr in map
      auto it_pbody = UnrealPhysicsBodies.find(sim_robot.GetID());
      projectairsim::UnrealPhysicsBody* sim_physbody =
          it_pbody != UnrealPhysicsBodies.end() ? it_pbody->second : nullptr;

      // Load UnrealRobot data from sim_robot
      unreal_robot->Initialize(sim_robot, sim_physbody, this,
                               using_unreal_physics_substep_clock);

      // Save ptr to unreal_robot
      unreal_actors.Add(unreal_robot);
    }
  }
}

void AUnrealScene::LoadUnrealEnvActor(UWorld* World,
                                      const projectairsim::Actor& Actor) {
  UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                    TEXT("[UnrealScene] Loading Unreal environment actor %S."),
                    Actor.GetID().c_str());

  if (Actor.GetType() == projectairsim::ActorType::kEnvActor) {
    const auto& sim_env_actor =
        static_cast<const projectairsim::EnvActor&>(Actor);

    // Load sim robot into UE rendering world
    auto xyz_ned = sim_env_actor.GetOrigin().translation_;
    FVector translation(xyz_ned.x() * 100, xyz_ned.y() * 100,
                        -xyz_ned.z() * 100);  // NED m -> NEU cm

    auto rpy = sim_env_actor.GetOrigin().rotation_;
    FQuat rotation(rpy.x(), rpy.y(), rpy.z(), rpy.w());

    FTransform transform(rotation, translation);

    auto unreal_env_actor = World->SpawnActorDeferred<AUnrealEnvActor>(
        AUnrealEnvActor::StaticClass(), transform, nullptr, nullptr,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

    unreal_env_actor->Initialize(sim_env_actor);

    // Save ptr to unreal_robot
    unreal_env_actors.Add(unreal_env_actor);
  }
}

void AUnrealScene::UnloadUnrealScene() {
  delete scene_substep_sampler_;
  scene_substep_sampler_ = nullptr;

  for (auto unreal_actor : unreal_actors) {
    unreal_actor->Destroy();
  }

  for (auto unreal_env_actor : unreal_env_actors) {
    unreal_env_actor->Destroy();
  }

  bool temp = world_api->destroyAllSpawnedObjects();

  unreal_actors.Empty();
  unreal_env_actors.Empty();
  idx_actor_to_view = 0;
  time_of_day->ResetSunSkyToDefault();
  UWeatherLib::unloadWeather(unreal_world);
  UpdateWindVelocity(projectairsim::Vector3::Zero());
  world_api->debugFlushPersistentMarkers();

  found_actor = false;
  tracing_enabled = false;

  // Reset persistent engine settings back to defaults
  GEngine->bUseFixedFrameRate = false;  // set to default from GameEngine.cpp
  GEngine->FixedFrameRate = 30.0f;      // set to default from GameEngine.cpp
  FApp::SetFixedDeltaTime(1 / 30.0);    // set to default from App.cpp
}

void AUnrealScene::StartUnrealScene() {
  // Calling FinishSpawning() with bIsDefaultTransform = true will use the
  // transform that was originally set at SpawnActorDeferred() so we can pass
  // dummy FTransforms here.
  this->FinishSpawning(FTransform(), true);
  for (const auto unreal_actor : unreal_actors) {
    unreal_actor->FinishSpawning(FTransform(), true);
  }

  for (const auto unreal_env_actor : unreal_env_actors) {
    unreal_env_actor->FinishSpawning(FTransform(), true);
  }
  // The Unreal actor BeginPlay() methods are triggered by the
  // FinishSpawning() calls, where we added our sim begin steps. Should
  // consider if the sim begin steps should be split out into an independent
  // BeginUpdate() method to be called here.
}

void AUnrealScene::StopUnrealScene() {
  // TODO Do we need to manually stop anything on the UnrealRobot components?
}

void AUnrealScene::SwitchStreamingView() {
  if (unreal_viewport_camera_) return;

  if (unreal_actors.Num() == 0) {
    found_actor = false;
    return;
  }

  AUnrealRobot* UnrealRobotToView = unreal_actors[idx_actor_to_view];
  if (UnrealRobotToView == nullptr) {
    found_actor = false;
    return;
  }

  bool bReachedLastCapture = UnrealRobotToView->SetNextStreamingCapture();

  if (bReachedLastCapture) {
    // Switch game view target to the next actor
    idx_actor_to_view = (idx_actor_to_view + 1) % unreal_actors.Num();
    UnrealRobotToView = unreal_actors[idx_actor_to_view];
    if (UnrealRobotToView == nullptr) return;
  }
  found_actor = true;
  UnrealRobotToView->SetViewportResolution();
  unreal_world->GetFirstPlayerController()->SetViewTarget(UnrealRobotToView);

  UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                    TEXT("[UnrealScene] Switched view to actor: %s"),
                    *(unreal_actors[idx_actor_to_view]->GetName()));
}

void AUnrealScene::ToggleTrace() {
  tracing_enabled = !tracing_enabled;

  if (!tracing_enabled)
    world_api->debugFlushPersistentMarkers();
  else if (found_actor) {
    last_position = unreal_actors[idx_actor_to_view]->GetActorLocation();
  }
}

void AUnrealScene::SetTraceLine(const std::vector<float>& color_rgba,
                                float thickness) {
  trace_color =
      FLinearColor{color_rgba[0], color_rgba[1], color_rgba[2], color_rgba[3]}
          .ToFColor(true);
  trace_thickness = thickness;
}

void AUnrealScene::UpdateWindVelocity(const projectairsim::Vector3& wind_vel) {
  FScopeLock ScopeLock(&UpdateMutex);

  UWeatherLib::setWeatherWindDirection(
      unreal_world,
      UnrealHelpers::ToFVector(
          projectairsim::TransformUtils::NedToNeuLinear(wind_vel)));
}

void AUnrealScene::EnableUnrealViewportCamera(bool enable) {
  if (enable && !unreal_viewport_camera_) {
    // Get location and rotation from unreal_actors[idx_actor_to_view]
    auto viewed_actor = unreal_actors[idx_actor_to_view];
    auto current_camera = viewed_actor->GetActiveStreamingCamera();
    // declare unreal location and rotation
    FVector location;
    FRotator rotation;
    if (current_camera) {
      location = current_camera->GetComponentTransform().GetLocation();
      rotation =
          current_camera->GetComponentTransform().GetRotation().Rotator();
    } else {  // if no camera, use actor location and rotation
      location = viewed_actor->GetActorLocation();
      rotation = viewed_actor->GetActorRotation();
    }
    // Spawn viewport camera on view target location and rotation
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    unreal_viewport_camera_ = unreal_world->SpawnActor<AUnrealViewportCamera>(
        AUnrealViewportCamera::StaticClass(), location, rotation, SpawnParams);
    unreal_world->GetFirstPlayerController()->SetViewTarget(
        unreal_viewport_camera_);
    unreal_viewport_camera_->Initialize(
        *(sim_scene->GetViewportCamera().get()));
  } else if (!enable && unreal_viewport_camera_) {
    unreal_viewport_camera_->Destroy();
    unreal_viewport_camera_ = nullptr;
    SwitchStreamingView();
  }
}

void AUnrealScene::BeginPlay() {
  Super::BeginPlay();
  unreal_time = 0;

  // Use static_cast instead of dynamic_cast since RTTI is disabled (/GR-)
  // in Unreal Engine builds. Determine the type from clock settings instead.
  if (sim_scene && (sim_scene->GetClockSettings().type ==
                        projectairsim::ClockType::kEngineDriven ||
                    sim_scene->GetClockSettings().type ==
                        projectairsim::ClockType::kExternalClock)) {
    unreal_driven_clock_ = static_cast<projectairsim::EngineDrivenClock*>(
        projectairsim::SimClock::Get());
  } else {
    unreal_driven_clock_ = nullptr;
  }
  using_unreal_driven_clock = unreal_driven_clock_ != nullptr;
  using_unreal_physics_substep_clock =
      sim_scene &&
      sim_scene->GetClockSettings().type ==
          projectairsim::ClockType::kEngineDriven &&
      sim_scene->GetClockSettings().engine_substepping;

  if (using_unreal_physics_substep_clock && !using_unreal_vehicle_physics) {
    scene_substep_sampler_ = new FProjectAirSimSceneSubstepSampler();
    if (!scene_substep_sampler_->Start(unreal_world)) {
      delete scene_substep_sampler_;
      scene_substep_sampler_ = nullptr;
      using_unreal_physics_substep_clock = false;
      UnrealLogger::Log(
          projectairsim::LogLevel::kWarning,
          TEXT("[UnrealScene] Could not register the scene physics substep "
               "sampler; falling back to configured clock steps."));
    }
  }
  UnrealLogger::Log(
      projectairsim::LogLevel::kTrace,
      TEXT("[UnrealScene] engine-driven=%s unreal-physics=%s substep-clock=%s"),
      using_unreal_driven_clock ? TEXT("true") : TEXT("false"),
      using_unreal_physics ? TEXT("true") : TEXT("false"),
      using_unreal_physics_substep_clock ? TEXT("true") : TEXT("false"));

  // Set up Unreal loop timing
  if (sim_scene && (sim_scene->GetClockSettings().type ==
                        projectairsim::ClockType::kSteppable ||
                    sim_scene->GetClockSettings().type ==
                        projectairsim::ClockType::kEngineDriven ||
                    sim_scene->GetClockSettings().type ==
                        projectairsim::ClockType::kExternalClock)) {
    // For steppable and fixed host-driven clocks, synchronize Unreal's
    // DeltaTime to the configured simulation step. A substep-driven clock
    // instead receives each actual Chaos step duration.
    double sim_step_sec = sim_scene->GetClockSettings().step / 1.0e9;
    if (!using_unreal_physics_substep_clock) {
      FApp::SetFixedDeltaTime(sim_step_sec);  // persists in UE project
    }

    if (using_unreal_physics) {
      // If scene has any robots with Unreal Physics, steppable sim clock must
      // follow Unreal's DeltaTime by external step calls
      if (!using_unreal_driven_clock) {
        projectairsim::SimClock::Get()->SetExternallySteppedOnly(true);
      }

      // TODO Uncommenting the below will set Unreal to do sequential ticks as
      // fast as possible without sleeps to synchronize to real time execution
      // periods. Not sure how this could be used properly yet.
      // FApp::SetUseFixedTimeStep(true);
      // FApp::SetBenchmarking(true);

      if (using_unreal_physics_substep_clock) {
        // Chaos divides each variable render frame into configured physics
        // substeps, so do not force the render loop to the core_sim step.
        GEngine->bUseFixedFrameRate = false;
      } else {
        // Since using UnrealPhysics means the physics step is linked to the
        // engine step, cap max FPS to the configured simulation period.
        double sim_fps = 1.0 / sim_step_sec;
        GEngine->FixedFrameRate = sim_fps;   // persists in UE project
        GEngine->bUseFixedFrameRate = true;  // persists in UE project
      }
    }
  } else {
    // For non-steppable clock, set persistent engine settings to defaults
    FApp::SetFixedDeltaTime(1 / 30.0);    // set to default from App.cpp
    GEngine->FixedFrameRate = 30.0f;      // set to default from GameEngine.cpp
    GEngine->bUseFixedFrameRate = false;  // set to default from GameEngine.cpp
  }

  if (using_unreal_physics_substep_clock) {
    // This setting persists across scene loads. A substep-driven scene must
    // not inherit the fixed frame rate selected by a previous scene unless
    // engine-fixed-fps explicitly enables it below.
    GEngine->bUseFixedFrameRate = false;
  }

  if (sim_scene) {
    const auto& ClockSettings = sim_scene->GetClockSettings();
    if (ClockSettings.engine_fixed_fps_enabled) {
      const double FixedFrameRate = ClockSettings.engine_fixed_fps;
      FApp::SetFixedDeltaTime(1.0 / FixedFrameRate);
      GEngine->FixedFrameRate = FixedFrameRate;
      GEngine->bUseFixedFrameRate = true;
      UnrealLogger::Log(
          projectairsim::LogLevel::kTrace,
          TEXT("[UnrealScene] Fixed Unreal frame rate set to %.3f FPS."),
          FixedFrameRate);
    }
  }

  // Set up keyboard input bindings (use Tab key to switch chase cam target)
  APlayerController* P1Controller = unreal_world->GetFirstPlayerController();
  EnableInput(P1Controller);

  if (sim_scene && !sim_scene->GetVRMode()) {
    FInputActionKeyMapping SwitchViewAction("SwitchStreamingView", EKeys::Tab);
    P1Controller->PlayerInput->AddActionMapping(SwitchViewAction);
    InputComponent->BindAction("SwitchStreamingView", IE_Pressed, this,
                               &AUnrealScene::SwitchStreamingView);
  }

  // Set up keyboard input bindings (use T key to enable/disable path tracing)
  FInputActionKeyMapping ToggleTraceAction("ToggleTrace", EKeys::T);
  P1Controller->PlayerInput->AddActionMapping(ToggleTraceAction);
  InputComponent->BindAction("ToggleTrace", IE_Pressed, this,
                             &AUnrealScene::ToggleTrace);

  if (sim_scene && !sim_scene->GetVRMode()) {
    // Set game view target to one of the robots
    idx_actor_to_view = 0;
    AUnrealRobot* UnrealRobotToView = nullptr;
    if (unreal_actors.Num() > 0) {
      // Find the first robot with a valid streaming camera. If no robots have
      // any valid streaming cameras, leave the view target index as the first
      // robot since the view will fall back to the origin of the actor to still
      // be able to display something.
      for (int i = 0; i < unreal_actors.Num(); ++i) {
        UnrealRobotToView = unreal_actors[i];
        if (UnrealRobotToView != nullptr &&
            UnrealRobotToView->GetActiveStreamingCapture() != nullptr) {
          idx_actor_to_view = i;
          break;
        }
      }

      UnrealRobotToView = unreal_actors[idx_actor_to_view];
      if (UnrealRobotToView != nullptr) {
        found_actor = true;
        UnrealRobotToView->SetViewportResolution();
        P1Controller->SetViewTarget(UnrealRobotToView);
      }
    }
  }

  // Initialize WorldSimAPI, Weather, and TimeOfDay
  time_of_day.reset(new TimeOfDay(home_geo_point));
  world_api.reset(new WorldSimApi(unreal_world, time_of_day, sim_scene));

  // Register terrain elevation callback for JSBSim robots. Callback returns
  // terrain altitude ASL in meters from Unreal raycast.
  const auto& actors = sim_scene->GetActors();
  for (const auto& actor_ref : actors) {
    auto& actor = actor_ref.get();
    if (actor.GetType() == projectairsim::ActorType::kRobot) {
      auto& sim_robot = static_cast<projectairsim::Robot&>(actor);
      if (sim_robot.GetPhysicsType() ==
          projectairsim::PhysicsType::kJSBSimPhysics) {
        sim_robot.SetCallbackTerrainElevationUpdated(
            [this](double x_ned_m, double y_ned_m) {
              if (world_api == nullptr) {
                return std::numeric_limits<double>::quiet_NaN();
              }

              const auto terrain_rel_m = world_api->GetZAtPoint(
                  static_cast<float>(x_ned_m), static_cast<float>(y_ned_m));

              if (!std::isfinite(terrain_rel_m)) {
                return std::numeric_limits<double>::quiet_NaN();
              }

              const double terrain_asl_m =
                  static_cast<double>(home_geo_point.geo_point.altitude) -
                  static_cast<double>(terrain_rel_m);

              return terrain_asl_m;
            });
      }
    }
  }

  UWeatherLib::initWeather(unreal_world, unreal_actors);

  UpdateWindVelocity(sim_scene->GetWindVelocity());

  bool isGisScene =
      (sim_scene->GetSceneType() == projectairsim::SceneType::kCustomGIS ||
       sim_scene->GetSceneType() == projectairsim::SceneType::kCesiumGIS ||
       sim_scene->GetSceneType() == projectairsim::SceneType::kBlackShark);

  if (sim_scene->GetSceneType() == projectairsim::SceneType::kBlackShark) {
    black_shark_renderer.reset(
        new BlackSharkRenderer(sim_scene->GetHomeGeoPoint()));
    black_shark_renderer->Initialize(unreal_world, isGisScene);
  }

  time_of_day->initialize(unreal_world, isGisScene);
  time_of_day->set(tod_setting.enabled, tod_setting.start_datetime,
                   tod_setting.is_start_datetime_dst,
                   tod_setting.celestial_clock_speed,
                   tod_setting.update_interval_secs, tod_setting.move_sun);

  // Initialize wind vel callback
  sim_scene->SetCallbackWindVelocityUpdated(
      [this](const projectairsim::Vector3& wind_vel) {
        this->UpdateWindVelocity(wind_vel);
      });

  // SetCallbackEnableUnrealViewportCamera
  sim_scene->SetCallbackEnableUnrealViewportCamera([this](bool enable) {
    UnrealHelpers::RunCommandOnGameThread(
        [this, enable]() { EnableUnrealViewportCamera(enable); }, true);
  });
}

// UnrealScene's ticks continue during Unreal pause, bTickEvenWhenPaused =
// true, in order to be able to pause/resume the Unreal side in sync with the
// sim clock. Any time-related sim logic should refer to SimClock and not
// DeltaTime.
void AUnrealScene::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  TimeNano cur_sim_time;
  bool is_unreal_paused = UGameplayStatics::IsGamePaused(unreal_world);
  bool is_simclock_paused = projectairsim::SimClock::Get()->IsPaused();

  if (using_unreal_driven_clock && unreal_driven_clock_ != nullptr) {
    // A previously loaded non-Unreal-physics scene can leave the Unreal world
    // paused while waiting for sim time. Resume it before waiting for Chaos
    // samples, otherwise no physics callback can produce the first substep.
    if (is_unreal_paused && !is_simclock_paused) {
      UGameplayStatics::SetGamePaused(unreal_world, false);
    }

    if (!is_unreal_paused) {
      if (using_unreal_physics_substep_clock) {
        while (true) {
          TimeNano SubstepDeltaNanos = 0;
          bool HasSubstep = false;
          if (using_unreal_vehicle_physics) {
            for (AUnrealRobot* UnrealRobot : unreal_actors) {
              if (UnrealRobot != nullptr &&
                  UnrealRobot->PeekProjectAirSimVehicleSubstepDelta(
                      SubstepDeltaNanos)) {
                HasSubstep = true;
                break;
              }
            }
          } else if (scene_substep_sampler_ != nullptr) {
            HasSubstep =
                scene_substep_sampler_->PopNextDelta(SubstepDeltaNanos);
          }
          if (!HasSubstep || SubstepDeltaNanos <= 0) break;

          const TimeNano NextSimTime =
              projectairsim::SimClock::Get()->NowSimNanos() + SubstepDeltaNanos;
          for (AUnrealRobot* UnrealRobot : unreal_actors) {
            if (UnrealRobot != nullptr) {
              UnrealRobot->ApplyProjectAirSimVehicleSubstepKinematics(
                  NextSimTime);
            }
          }

          // Run the full core_sim scene exactly once for this Chaos substep,
          // using its actual duration instead of iterating the configured
          // fixed step over the render-frame DeltaTime.
          if (!sim_scene->ExternalTick(SubstepDeltaNanos)) {
            UnrealLogger::Log(
                projectairsim::LogLevel::kWarning,
                TEXT("[UnrealScene] Substep-driven scene tick failed."));
            break;
          }
        }
      } else {
        unreal_driven_clock_->AccumulateStep(
            UnrealHelpers::DeltaTimeToNanos(DeltaTime));

        while (unreal_driven_clock_->HasPendingStep()) {
          // Preserve the fixed-step engine-driven behavior when physics
          // substepping is not driving core_sim.
          const TimeNano NextSimTime =
              projectairsim::SimClock::Get()->NowSimNanos() +
              sim_scene->GetClockSettings().step;
          for (AUnrealRobot* UnrealRobot : unreal_actors) {
            if (UnrealRobot != nullptr) {
              UnrealRobot->ApplyProjectAirSimVehicleSubstepKinematics(
                  NextSimTime);
            }
          }

          if (!sim_scene->ExternalTick()) {
            UnrealLogger::Log(
                projectairsim::LogLevel::kWarning,
                TEXT("[UnrealScene] Engine-driven scene tick failed."));
            break;
          }
        }
      }
    }
    cur_sim_time = projectairsim::SimClock::Get()->NowSimNanos();
  } else if (using_unreal_physics) {
    //-------------------------------------------------------------------------
    // UnrealPhysics

    // Unreal DeltaTime ticks lead SimClock so we externally set SimClock's
    // next step amounts and need to track pause states to stay in sync.
    cur_sim_time = unreal_time;

    if (is_unreal_paused && !is_simclock_paused) {
      // Condition #1:
      // Unreal was paused during this tick so physics did not advance. Since
      // simclock is ready to advance (not paused), just unpause Unreal so the
      // next tick will start a physics update (should reach Condition #2 on
      // the next loop).
      UGameplayStatics::SetGamePaused(unreal_world, false);
    } else if (!is_unreal_paused && !is_simclock_paused) {
      // Condition #2:
      // Neither is paused, so advance sim time for this tick
      cur_sim_time += UnrealHelpers::DeltaTimeToNanos(DeltaTime);
      projectairsim::SimClock::Get()->SetNextSimTime(cur_sim_time);

      // Wait for sim to catch up processing new sim time
      while (projectairsim::SimClock::Get()->NowSimNanos() < cur_sim_time) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
      // If cur_sim_time is now at the time for simclock to pause, pause
      // Unreal now so physics will not advance again at the next tick.
      TimeNano sim_time_to_pause =
          projectairsim::SimClock::Get()->GetSimTimeToPauseNanos();
      if (cur_sim_time >= sim_time_to_pause) {
        UGameplayStatics::SetGamePaused(unreal_world, true);
      }
    } else if (is_unreal_paused && is_simclock_paused) {
      // Condition #3:
      // Everything is paused, so just stay that way (no-op)
    } else {  // (!is_unreal_paused && is_simclock_paused)
      // Condition #4:
      // Unreal advanced physics while simclock was paused, this should not
      // have happened. Warn user and pause Unreal for next tick.
      UnrealLogger::Log(
          projectairsim::LogLevel::kWarning,
          TEXT("[UnrealScene] WARNING: Unreal physics was not paused for this "
               "tick while simclock was. Simulation may be out of sync."));
      UGameplayStatics::SetGamePaused(unreal_world, true);
    }
  } else {
    //-------------------------------------------------------------------------
    // Non-UnrealPhysics (ex. FastPhysics)

    // Unreal time follows after SimClock so just check if sim time has
    // advanced to decide if Unreal needs to pause/resume to stay in sync.
    cur_sim_time = projectairsim::SimClock::Get()->NowSimNanos();
    if (is_unreal_paused && unreal_time < cur_sim_time) {
      UGameplayStatics::SetGamePaused(unreal_world, false);
    } else if (!is_unreal_paused && unreal_time >= cur_sim_time) {
      UGameplayStatics::SetGamePaused(unreal_world, true);
    }
  }

  // trace path if tracing_enabled
  if (found_actor) {
    FVector position = unreal_actors[idx_actor_to_view]->GetActorLocation();
    float distance_threshold = 0.25f;
    if (tracing_enabled &&
        (position - last_position).SizeSquared() > distance_threshold) {
      DrawDebugLine(unreal_world, last_position, position, trace_color, true,
                    -1.0F, 0, trace_thickness);
      last_position = position;
    } else if (!tracing_enabled) {
      last_position = position;
    }
  }

  // Save sim time for next loop
  unreal_time = cur_sim_time;

  if (time_of_day->tod_move_sun_) time_of_day->advance();

  // Update unreal viewport camera
  if (unreal_viewport_camera_) unreal_viewport_camera_->Tick(DeltaTime);
}

AActor* AUnrealScene::FindActor(const std::string& object_name) const {
  if (world_api) return world_api->FindActor(object_name);
  return nullptr;
}

bool AUnrealScene::GetWorldAlignedBoundingBox3D(
    const std::string& ObjectName, FOrientedBox& OrientedBox) const {
  AActor* actor = world_api->FindActor(ObjectName);
  if (!actor) {
    return false;
  }
  auto AABBox = actor->GetComponentsBoundingBox();

  OrientedBox.Center = AABBox.GetCenter();
  OrientedBox.ExtentX = AABBox.GetExtent().X;
  OrientedBox.ExtentY = AABBox.GetExtent().Y;
  OrientedBox.ExtentZ = AABBox.GetExtent().Z;
  OrientedBox.AxisX = FVector(1, 0, 0);
  OrientedBox.AxisY = FVector(0, 1, 0);
  OrientedBox.AxisZ = FVector(0, 0, 1);

  return true;
}

bool AUnrealScene::GetActorAlignedBoundingBox3D(const std::string& ObjectName,
                                                FOrientedBox& OutOrientedBox,
                                                FRotator& OutRotation) const {
  AActor* actor = world_api->FindActor(ObjectName);
  if (!actor) {
    return false;
  }

  FBox AAScaledLocalBB;
  if (actor_bbox_cache.find(ObjectName) != actor_bbox_cache.end()) {
    AAScaledLocalBB = actor_bbox_cache[ObjectName];
  } else {
    AAScaledLocalBB = actor->CalculateComponentsBoundingBoxInLocalSpace(
        /*bNonColliding=*/false,
        /*bInlcudeFromChildActors=*/false);
    AAScaledLocalBB =
        AAScaledLocalBB.TransformBy(FScaleMatrix::Make(actor->GetActorScale()));
    actor_bbox_cache[ObjectName] = AAScaledLocalBB;
  }

  // Need to rotate then translate to get the correct center.
  // The center of the rotated+translated AABB will be the same center as the
  // oriented BB, but will have the wrong extents (since the AABB size will
  // increase)
  OutOrientedBox.Center =
      AAScaledLocalBB
          .TransformBy(FRotationMatrix::Make(actor->GetActorRotation()))
          .TransformBy(FTranslationMatrix::Make(actor->GetActorLocation()))
          .GetCenter();

  // Now only transform local BB by translation to get the right extents.
  // Don't transform by rotation, otherwise you will get a loose AABB.
  // Don't transform by scale, since that was already precomputed in our
  // cache.
  auto AAScaledWorldBB = AAScaledLocalBB.TransformBy(
      FTranslationMatrix::Make(actor->GetActorLocation()));

  OutOrientedBox.ExtentX = AAScaledWorldBB.GetExtent().X;
  OutOrientedBox.ExtentY = AAScaledWorldBB.GetExtent().Y;
  OutOrientedBox.ExtentZ = AAScaledWorldBB.GetExtent().Z;
  OutOrientedBox.AxisX =
      actor->GetActorRotation().RotateVector(FVector(1, 0, 0));
  OutOrientedBox.AxisY =
      actor->GetActorRotation().RotateVector(FVector(0, 1, 0));
  OutOrientedBox.AxisZ =
      actor->GetActorRotation().RotateVector(FVector(0, 0, 1));
  OutRotation = actor->GetActorRotation();

  return true;
}

bool AUnrealScene::GetUnrealBoundingBox3D(
    const std::string& ObjectName, projectairsim::BoxAlignment BoxAlignment,
    FOrientedBox& OutOrientedBox, FRotator& OutRotation) const {
  if (BoxAlignment == projectairsim::BoxAlignment::kAxis) {
    // we only need to cache object-aligned boxes. UE manages AABBs on its own
    OutRotation = FRotator::ZeroRotator;
    return GetWorldAlignedBoundingBox3D(ObjectName, OutOrientedBox);
  }

  return GetActorAlignedBoundingBox3D(ObjectName, OutOrientedBox, OutRotation);
}

bool AUnrealScene::GetSimBoundingBox3D(
    const std::string& object_name, projectairsim::BoxAlignment box_alignment,
    FOrientedBox& OutUnrealBox, projectairsim::BBox3D& OutSimBox) const {
  FRotator BoxRotation;

  if (!(this->GetUnrealBoundingBox3D(object_name, box_alignment, OutUnrealBox,
                                     BoxRotation))) {
    // TODO: check for min bounding box and skip here too
    return false;  // skip, no such actor
  }

  auto AirSimCenter3D = UnrealTransform::UnrealToNedLinear(OutUnrealBox.Center);
  auto AirSimSize3D =
      UnrealTransform::UnrealToNedLinear(FVector(OutUnrealBox.ExtentX * 2.0,
                                                 OutUnrealBox.ExtentY * 2.0,
                                                 OutUnrealBox.ExtentZ * 2.0))
          .cwiseAbs();
  auto AirSimQuat = UnrealHelpers::ToQuaternion(BoxRotation);
  OutSimBox = projectairsim::BBox3D(AirSimCenter3D, AirSimSize3D, AirSimQuat);

  // DrawDebugBox(UnrealWorld, OutBox.Center,
  //             FVector(OutBox.ExtentX, OutBox.ExtentY, OutBox.ExtentZ),
  //             BoxRotation.Quaternion(),
  //             FColor::Blue);

  return true;
}

nlohmann::json AUnrealScene::Get3DBoundingBoxServiceMethod(
    const std::string& object_name, int box_alignment) {
  FOrientedBox OrientedBox;
  projectairsim::BBox3D OutBBox;
  auto BoxAlignment =
      projectairsim::MathUtils::ToEnum<projectairsim::BoxAlignment>(
          box_alignment);
  if (GetSimBoundingBox3D(object_name, BoxAlignment, OrientedBox, OutBBox)) {
    return OutBBox;
  }
  return nlohmann::json({});
}

void AUnrealScene::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  delete scene_substep_sampler_;
  scene_substep_sampler_ = nullptr;
  Super::EndPlay(EndPlayReason);
}

void AUnrealScene::RegisterServiceMethods() {
  if (sim_scene && !sim_scene->GetVRMode()) {
    //! Register "SwitchStreamingView" as a service method through sim scene
    auto switch_streaming_view =
        projectairsim::ServiceMethod("SwitchStreamingView", {""});
    auto switch_streaming_view_handler =
        switch_streaming_view.CreateMethodHandler(
            &AUnrealScene::SwitchStreamingViewServiceMethod, *this);
    sim_scene->RegisterServiceMethod(switch_streaming_view,
                                     switch_streaming_view_handler);
  }

  //! Register "SetTraceLine" as a service method through sim scene
  auto set_trace_line =
      projectairsim::ServiceMethod("SetTraceLine", {"color_rgba", "thickness"});
  auto set_trace_line_handler = set_trace_line.CreateMethodHandler(
      &AUnrealScene::SetTraceLineServiceMethod, *this);
  sim_scene->RegisterServiceMethod(set_trace_line, set_trace_line_handler);

  //! Register "ToggleTrace" as a service method through sim scene
  auto toggle_trace = projectairsim::ServiceMethod("ToggleTrace", {""});
  auto toggle_trace_handler = toggle_trace.CreateMethodHandler(
      &AUnrealScene::ToggleTraceServiceMethod, *this);
  sim_scene->RegisterServiceMethod(toggle_trace, toggle_trace_handler);

  auto get_bbox = projectairsim::ServiceMethod(
      "Get3DBoundingBox", {"object_name", "box_alignment"});
  auto get_bbox_handler = get_bbox.CreateMethodHandler(
      &AUnrealScene::Get3DBoundingBoxServiceMethod, *this);
  sim_scene->RegisterServiceMethod(get_bbox, get_bbox_handler);
}

bool AUnrealScene::SwitchStreamingViewServiceMethod() {
  UnrealHelpers::RunCommandOnGameThread([this]() { SwitchStreamingView(); },
                                        true);
  return true;
}

bool AUnrealScene::SetTraceLineServiceMethod(
    const std::vector<float>& color_rgba, float thickness) {
  UnrealHelpers::RunCommandOnGameThread(
      [this, &color_rgba, thickness]() { SetTraceLine(color_rgba, thickness); },
      true);
  return true;
}

bool AUnrealScene::ToggleTraceServiceMethod() {
  UnrealHelpers::RunCommandOnGameThread([this]() { ToggleTrace(); }, true);
  return true;
}
