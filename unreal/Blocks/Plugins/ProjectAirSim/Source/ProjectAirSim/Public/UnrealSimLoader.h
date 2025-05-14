// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#pragma once

#include <fstream>
#include <memory>
#include <string>

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Renderers/GISRenderer.h"
#include "UnrealScene.h"
#include "UnrealSimTopicData.h"

namespace microsoft {
namespace projectairsim {

class SimServer;
class Scene;
class Actor;

}  // namespace projectairsim
}  // namespace microsoft

#ifdef ENABLE_CESIUM
class ACesium3DTileset;
#endif

class PROJECTAIRSIM_API AUnrealSimLoader {
 public:
  AUnrealSimLoader(const FString& SimLogDir = "");

  ~AUnrealSimLoader();

  void LaunchSimulation(UWorld* World);

  void TeardownSimulation();

 private:
  // --------------------------------------------------------------------
  // Configurations from Unreal

  std::string LoadClientAuthorizationPublicKey();

  std::string LoadTopicsPort();

  std::string LoadServicesPort();

  FString ResolveTilesDirectory(const std::string& SceneConfigTilesDir);

#ifdef ENABLE_CESIUM
  void ConfigureCesiumTileset(ACesium3DTileset* CesiumTile,
                              microsoft::projectairsim::GeoPoint& HomeGeoPoint,
                              FString TilesetDir);
#endif

  void SetUnrealEngineSettings();

  void SetUnrealEngineVariableInt(const TCHAR* VarName, int32 VarValue);

  // --------------------------------------------------------------------
  // Manage Unreal scene

  void LoadUnrealScene();

  void StartUnrealScene();

  void StopUnrealScene();

  void UnloadUnrealScene();

  // --------------------------------------------------------------------
  // Member variables

  UWorld* UnrealWorld;

  std::shared_ptr<microsoft::projectairsim::SimServer> SimServer;

  AUnrealScene* UnrealScene;
  AGISRenderer* GISRenderer;
  AUnrealSimTopicData* UnrealSimTopicData;

  std::ofstream SimLogFile;

  static constexpr int32 SupportedUnrealVersionMajor = 5;
  static constexpr int32 SupportedUnrealVersionMinor = 1;
};
