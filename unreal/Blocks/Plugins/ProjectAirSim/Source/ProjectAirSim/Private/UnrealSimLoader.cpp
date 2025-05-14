// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#include "UnrealSimLoader.h"

#include <exception>
#include <iostream>
#include <string>
#include <unordered_map>

#include "Engine/Engine.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjectAirSim.h"
#include "UnrealLogger.h"
#include "UnrealScene.h"
#include "simserver.hpp"

#ifdef ENABLE_CESIUM
#include "Cesium3DTileset.h"
#include "CesiumGeoreference.h"
#endif

namespace projectairsim = microsoft::projectairsim;

DEFINE_LOG_CATEGORY(SimPlugin);

AUnrealSimLoader::AUnrealSimLoader(const FString& SimLogDir)
    : SimServer(std::make_shared<projectairsim::SimServer>(
          UnrealLogger::LogSim, projectairsim::LogLevel::kVerbose)) {
  // Open log file output stream to start capturing clog
  FString SimLogPath =
      FPaths::Combine(SimLogDir, TEXT("projectairsim_server.log"));
  SimLogFile = std::ofstream(TCHAR_TO_UTF8(*SimLogPath));
  std::clog.rdbuf(SimLogFile.rdbuf());  // set clog output to log_file stream

  // Bind callbacks for Unreal scene load/start/stop/unloading
  SimServer->SetCallbackLoadExternalScene(
      [this]() { this->AUnrealSimLoader::LoadUnrealScene(); });

  SimServer->SetCallbackStartExternalScene(
      [this]() { this->AUnrealSimLoader::StartUnrealScene(); });

  SimServer->SetCallbackStopExternalScene(
      [this]() { this->AUnrealSimLoader::StopUnrealScene(); });

  SimServer->SetCallbackUnloadExternalScene(
      [this]() { this->AUnrealSimLoader::UnloadUnrealScene(); });
}

AUnrealSimLoader::~AUnrealSimLoader() {
  SimLogFile.close();
  std::clog.rdbuf(nullptr);  // detach clog output stream from log_file stream
}

void AUnrealSimLoader::LaunchSimulation(UWorld* World) {
  UnrealLogger::Log(
      projectairsim::LogLevel::kTrace,
      TEXT("[AUnrealSimLoader::LaunchSimulation()] Launching simulation "
           "in Unreal Engine version %d.%d."),
      ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);

  if (ENGINE_MAJOR_VERSION != SupportedUnrealVersionMajor ||
      ENGINE_MINOR_VERSION != SupportedUnrealVersionMinor) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kWarning,
        TEXT("Unreal Engine version is not the supported version of %d.%d."),
        SupportedUnrealVersionMajor, SupportedUnrealVersionMinor);
  }

  // Save Unreal world pointer
  UnrealWorld = World;

  try {
    // Set up Unreal Engine settings (for segmentation CustomDepth Stencil, etc)
    SetUnrealEngineSettings();

    std::string client_auth_public_key = LoadClientAuthorizationPublicKey();
    std::string TopicsPort = LoadTopicsPort();
    std::string ServicesPort = LoadServicesPort();

    // // Load simulator and default scene
    SimServer->LoadSimulator(std::stoi(TopicsPort), std::stoi(ServicesPort),
                             client_auth_public_key);

    SimServer->LoadScene();

    // Load UE UnrealRobots for each sim robot in scene with
    // SpawnActorDeferred() so that it will not trigger BeginPlay() until
    // UnrealScene->StartUnrealScene() is called
    LoadUnrealScene();

    // Start simulator and scene
    SimServer->StartSimulator();  // open client connection

    StartUnrealScene();
    SimServer->StartScene();
  } catch (const std::exception& Exception) {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("[AUnrealSimLoader::LaunchSimulation()] Exception "
                           "'%hs' encountered while launching the simulator."),
                      Exception.what());
  } catch (...) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[AUnrealSimLoader::LaunchSimulation()] Unknown exception "
             "encountered while launching the simulator."));
  }
}

void AUnrealSimLoader::TeardownSimulation() {
  UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                    TEXT("[AUnrealSimLoader] Tearing down simulation."));
  try {
    // Stop scene and simulator
    SimServer->StopScene();
    StopUnrealScene();

    SimServer->StopSimulator();  // close client connection

    // Unload scene and simulator
    UnloadUnrealScene();
    SimServer->UnloadScene();
    SimServer->UnloadSimulator();
  } catch (const std::exception& Exception) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[AUnrealSimLoader::TeardownSimulation()] Exception '%hs' "
             "encountered while tearing down the simulator"),
        Exception.what());
  } catch (...) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("[AUnrealSimLoader::TeardownSimulation()] Unknown "
             "exception encountered while tearing down the simulator"));
  }
}

// Load client authorization public key from command line or environment
// variable
std::string AUnrealSimLoader::LoadClientAuthorizationPublicKey() {
  FString public_key;

  if (FParse::Value(FCommandLine::Get(), TEXT("clientauthpubkey"),
                    public_key)) {
    public_key.RemoveFromStart(TEXT("="), ESearchCase::CaseSensitive);
  }
  if (public_key.IsEmpty()) {
    public_key = FPlatformMisc::GetEnvironmentVariable(
        TEXT("PROJECTAIRSIM_CLIENT_AUTH_PUBKEY"));
    public_key.RemoveFromStart(TEXT("\""), ESearchCase::CaseSensitive);
    public_key.RemoveFromEnd(TEXT("\""), ESearchCase::CaseSensitive);
    UnrealLogger::Log(
        projectairsim::LogLevel::kVerbose,
        TEXT("[AUnrealSimLoader::LoadClientAuthorizationPublicKey()] Loaded "
             "public key from environment variable: \"%hs\""),
        TCHAR_TO_UTF8(*public_key));
  }
  return std::string(TCHAR_TO_UTF8(*public_key));
}

// Load Topics Port from command line
std::string AUnrealSimLoader::LoadTopicsPort() {
  std::string TopicPortStr;
  FString TopicPort;
  if (FParse::Value(FCommandLine::Get(), UTF8_TO_TCHAR("topicsport"),
                    TopicPort)) {
    TopicPort = TopicPort.Replace(TEXT("="), TEXT(""));
    TopicPortStr = std::string(TCHAR_TO_UTF8(*TopicPort));
  } else {
    TopicPortStr = "8989";  // Default topic port
  }
  return TopicPortStr;
}

// Load Service Port from command line
std::string AUnrealSimLoader::LoadServicesPort() {
  std::string ServicePortStr;
  FString ServicePort;
  if (FParse::Value(FCommandLine::Get(), UTF8_TO_TCHAR("servicesport"),
                    ServicePort)) {
    ServicePort = ServicePort.Replace(TEXT("="), TEXT(""));
    ServicePortStr = std::string(TCHAR_TO_UTF8(*ServicePort));

  } else {
    ServicePortStr = "8990";  // default service port
  }

  return ServicePortStr;
}

FString AUnrealSimLoader::ResolveTilesDirectory(
    const std::string& SceneConfigTilesDir) {
  FString ResolvedTilesDir;

  if (FParse::Value(FCommandLine::Get(),
                    UTF8_TO_TCHAR(Constant::GltfDirCmdParamName),
                    ResolvedTilesDir)) {
    UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                      TEXT("[AUnrealSimLoader] Tile source provided by: server "
                           "gltfDir CLI arg."));
    ResolvedTilesDir = ResolvedTilesDir.Replace(TEXT("="), TEXT("")) + "/";

  } else if (!SceneConfigTilesDir.empty()) {
    UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                      TEXT("[AUnrealSimLoader] Tile source provided by: client "
                           "scene config."));
    ResolvedTilesDir = FString(SceneConfigTilesDir.c_str()) + "/";

  } else {
    UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                      TEXT("[AUnrealSimLoader] Tile source provided by: "
                           "default plugin path."));
    auto ContentDir = FPaths::ProjectContentDir();
    ResolvedTilesDir = ContentDir + Constant::GltfDefaultRelativePath + "/";
  }

  UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                    TEXT("[AUnrealSimLoader] Resolved tiles directory = '%s'"),
                    *ResolvedTilesDir);

  return ResolvedTilesDir;
}

#ifdef ENABLE_CESIUM
void AUnrealSimLoader::ConfigureCesiumTileset(
    ACesium3DTileset* CesiumTile, microsoft::projectairsim::GeoPoint& HomeGeoPoint,
    FString TilesetDir) {
  CesiumTile->Url = FString("file:///" + TilesetDir);

  // Disable culling and lower LODs since sensors might be independent from
  // frustum.
  CesiumTile->EnableFrustumCulling = false;
  CesiumTile->EnforceCulledScreenSpaceError = true;
  CesiumTile->CulledScreenSpaceError = CesiumTile->MaximumScreenSpaceError;

  auto CesiumRot = FRotator(
      0, 90, 0);  // to match cesium's ENU RHS coords with our NEU LHS coords.
  auto CesiumLoc =
      FVector(0.f, 0.f, 8844.f);  // TODO: can we get rid of this magic num and
                                  // smartly pick a height for the tileset?
  UGameplayStatics::FinishSpawningActor(CesiumTile,
                                        FTransform(CesiumRot, CesiumLoc));

  CesiumTile->Georeference->OriginLatitude = HomeGeoPoint.latitude;
  CesiumTile->Georeference->OriginLongitude = HomeGeoPoint.longitude;
  CesiumTile->Georeference->OriginHeight = HomeGeoPoint.altitude;
  CesiumTile->Georeference->KeepWorldOriginNearCamera =
      false;  // TODO: to maintain precision we should look into making this
              // work.
}
#endif

void AUnrealSimLoader::SetUnrealEngineSettings() {
  UnrealLogger::Log(projectairsim::LogLevel::kVerbose,
                    TEXT("[AUnrealSimLoader] Setting Unreal Engine settings."));
  // TODO should we only do below on SceneCapture2D components and cameras?
  // Disable motion blur in the viewport
  UnrealWorld->GetGameViewport()->GetEngineShowFlags()->SetMotionBlur(false);

  // Enable rendering with CustomDepth Stencil buffer for segmentation camera
  SetUnrealEngineVariableInt(TEXT("r.CustomDepth"), 3);

  // During startup we init segmentation stencil IDs to random hash and it takes
  // a long time for large environments so we can get error that GameThread has
  // timed out after 30 sec waiting on render thread, increase to 300 sec
  // TODO Engine default was already increased to 120 sec, is this still needed?
  SetUnrealEngineVariableInt(TEXT("g.TimeoutForBlockOnRenderFence"), 300000);

  // Set r.Vulkan.EnableDefrag=0 to as temporary workaround for Vulkan memory
  // crashes in UE 4.26
  // https://answers.unrealengine.com/questions/1018939/view.html
  SetUnrealEngineVariableInt(TEXT("r.Vulkan.EnableDefrag"), 0);

  // Set WebRTC DisableResolutionChange = false to allow resolution changing
  SetUnrealEngineVariableInt(
      TEXT("PixelStreaming.WebRTC.DisableResolutionChange"), 0);

  // Set PixelStreaming plugin settings for sim usage unless specified by user
  // command line arguments.
  // TODO Expose these settings to user config?
  const TCHAR* CmdLine = FCommandLine::Get();
  int32 CmdLineVal;
  if (!FParse::Value(CmdLine, TEXT("PixelStreamingWebRTCMaxBitrate="),
                     CmdLineVal)) {
    // 1080p@60 should need ~10 Mbps, 4k@60 ~40 Mbps
    SetUnrealEngineVariableInt(TEXT("PixelStreaming.WebRTC.MaxBitrate"),
                               40'000'000);
  }

  if (!FParse::Value(CmdLine, TEXT("PixelStreamingWebRTCMaxFps="),
                     CmdLineVal)) {
    SetUnrealEngineVariableInt(TEXT("PixelStreaming.WebRTC.MaxFps"), 60);
  }

  if (!FParse::Param(CmdLine,
                     TEXT("PixelStreamingWebRTCDisableTransmitAudio"))) {
    SetUnrealEngineVariableInt(
        TEXT("PixelStreaming.WebRTC.DisableTransmitAudio"), 1);
  }

  if (!FParse::Param(CmdLine,
                     TEXT("PixelStreamingWebRTCDisableReceiveAudio"))) {
    SetUnrealEngineVariableInt(
        TEXT("PixelStreaming.WebRTC.DisableReceiveAudio"), 1);
  }

  // if (FParse::Param(CmdLine, TEXT("PixelStreamingURL")) == false) {
  //   // If command line args did not include PixelStreamingURL, set it to a
  //   // default local host address/port.
  //   FCommandLine::Append(TEXT(" -PixelStreamingURL=ws://127.0.0.1:8888 "));
  // }
}

void AUnrealSimLoader::SetUnrealEngineVariableInt(const TCHAR* VarName,
                                                  int32 VarValue) {
  UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                    TEXT("[AUnrealSimLoader::SetUnrealEngineSettings()] "
                         "Setting console variable '%s' to %d"),
                    VarName, VarValue);
  // Set console variable value
  const auto ConsoleVar = IConsoleManager::Get().FindConsoleVariable(VarName);
  if (ConsoleVar != nullptr) {
    ConsoleVar->Set(VarValue);
  }

  // Confirm that variable value was set
  if (ConsoleVar == nullptr || ConsoleVar->GetInt() != VarValue) {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("[AUnrealSimLoader::SetUnrealEngineSettings()] "
                           "Console variable '%s' could not be set"),
                      VarName);
  }
}

void AUnrealSimLoader::LoadUnrealScene() {
  projectairsim::Scene& Scene = SimServer->GetScene();

  UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                    TEXT("[AUnrealSimLoader] Loading Unreal scene '%hs'."),
                    Scene.GetID().c_str());

  UnrealHelpers::RunCommandOnGameThread(
      [this, &Scene]() {
        {
          // Create an UnrealScene actor and have it load all of the Unreal
          // Actors
          UnrealScene = UnrealWorld->SpawnActorDeferred<AUnrealScene>(
              AUnrealScene::StaticClass(), FTransform(), nullptr, nullptr,
              ESpawnActorCollisionHandlingMethod::
                  AdjustIfPossibleButAlwaysSpawn);

          std::unordered_map<std::string,
                             microsoft::projectairsim::UnrealPhysicsBody*>
              UnrealPhysicsBodies;  // TODO Delete this dummy var and logic
          UnrealScene->LoadUnrealScene(UnrealWorld, Scene, UnrealPhysicsBodies);
        }

        {
          // Create a GIS tile rendering actor for GIS scene types
          auto TilesDir = ResolveTilesDirectory(Scene.GetTilesDirectory());
          auto HorizonTilesDir = Scene.GetHorizonTilesDirectory();

          switch (Scene.GetSceneType()) {
            case projectairsim::SceneType::kCesiumGIS: {

#ifdef ENABLE_CESIUM
              auto HomeGeoPoint = Scene.GetHomeGeoPoint().geo_point;
              auto CesiumTile =
                  UnrealWorld->SpawnActorDeferred<ACesium3DTileset>(
                      ACesium3DTileset::StaticClass(), FTransform(), nullptr,
                      nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
              ConfigureCesiumTileset(CesiumTile, HomeGeoPoint, TilesDir);
              break;
#else
              UnrealLogger::Log(
                  projectairsim::LogLevel::kTrace,
                  TEXT(
                      "[AUnrealSimLoader] Integration with CesiumForUnreal is "
                      "only enabled by default on Windows, defaulting to "
                      "CustomGIS. For experimental support in other platforms, "
                      "define ENABLE_CESIUM in ProjectAirSim.Build.cs before "
                      "building."));

          // will fall through to CustomGIS, since Cesium wasn't enabled.
#endif
            }

            case projectairsim::SceneType::kCustomGIS: {
              FActorSpawnParameters GISSpawnParams;
              GISSpawnParams.SpawnCollisionHandlingOverride =
                  ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

              GISRenderer = UnrealWorld->SpawnActor<AGISRenderer>(
                  AGISRenderer::StaticClass(), FTransform(), GISSpawnParams);

              GISRenderer->Init(SimServer, TilesDir,
                                Scene.GetHomeGeoPoint().geo_point,
                                Scene.GetTilesAltitudeOffset(),
                                HorizonTilesDir);
              break;
            }
            case projectairsim::SceneType::kBlackShark: {
              break;
            }

            default:
              UnrealLogger::Log(
                  projectairsim::LogLevel::kTrace,
                  TEXT("[AUnrealSimLoader] Not a GIS scene. SceneType = %d."),
                  static_cast<int>(Scene.GetSceneType()));
              break;
          }
        }

        {
          if (Scene.IsSimTopicCallbackEnabled()) {
            // Create a SimTopicData actor to expose published topics to Unreal
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride =
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            UnrealSimTopicData = UnrealWorld->SpawnActor<AUnrealSimTopicData>(
                AUnrealSimTopicData::StaticClass(), FTransform(), SpawnParams);

            UnrealSimTopicData->SetTopicPublishedCallback(Scene);
          }
        }
      },
      true);
}

void AUnrealSimLoader::StartUnrealScene() {
  UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                    TEXT("[AUnrealSimLoader] Starting Unreal scene."));
  // Handle pause before starting
  if (SimServer->GetScene().GetClockSettings().pause_on_start) {
    UGameplayStatics::SetGamePaused(UnrealWorld, true);
  }

  if (UnrealScene) {
    UnrealHelpers::RunCommandOnGameThread(
        [this]() { UnrealScene->StartUnrealScene(); }, true);
  }
}

void AUnrealSimLoader::StopUnrealScene() {
  if (UnrealScene) {
    UnrealHelpers::RunCommandOnGameThread(
        [this]() { UnrealScene->StopUnrealScene(); }, true);
  }
}

void AUnrealSimLoader::UnloadUnrealScene() {
  UnrealLogger::Log(projectairsim::LogLevel::kTrace,
                    TEXT("[AUnrealSimLoader] Unloading Unreal scene."));
  UnrealHelpers::RunCommandOnGameThread(
      [this]() {
        if (UnrealScene) {
          // Destroy UnrealRobot actors in UnrealScene
          UnrealScene->UnloadUnrealScene();

          // Destroy UnrealScene actor
          UnrealScene->Destroy();
          UnrealScene = nullptr;
        }

        // Destroy GIS rendering actors if they exist
        if (GISRenderer) {
          GISRenderer->UnloadAllProcMeshActors();
          GISRenderer->Destroy();
          GISRenderer = nullptr;
        }

        // Destroy UnrealSimTopicData actor if it exists
        if (UnrealSimTopicData) {
          UnrealSimTopicData->Destroy();
          UnrealSimTopicData = nullptr;
        }
      },
      true);

  // Force GC to clear out old actors so new actors can use their original names
  UnrealHelpers::ForceUnrealGarbageCollection();
}
