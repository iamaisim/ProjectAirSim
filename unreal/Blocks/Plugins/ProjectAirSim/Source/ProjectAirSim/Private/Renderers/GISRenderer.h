// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
#pragma once

#include <future>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProcMeshActor.h"
#include "core_sim/earth_utils.hpp"
#include "core_sim/geodetic_converter.hpp"
#include "gltf_data_provider.hpp"
#include "mesh.hpp"
#include "mesh_data_provider.hpp"
#include "simserver.hpp"
#include "tile_cache.hpp"
#include "tile_info.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "GISRenderer.generated.h"

namespace mrrs = microsoft::projectairsim::rendering::scene;

class UProceduralMeshComponent;

class UnrealFileSystemReader : public mrrs::FileReaderInterface {
 public:
  UnrealFileSystemReader() {}
  std::string ReadFile(const std::string& FileName) const override;
};

UCLASS()
class AGISRenderer : public AActor {
  GENERATED_BODY()

 public:
  // Sets default values for this actor's properties
  AGISRenderer();

  ~AGISRenderer();

 protected:
  // Called when the game starts or when spawned
  void BeginPlay() override;

 public:
  // Called every frame
  void Tick(float DeltaTime) override;

  void UnloadAllProcMeshActors();

  void Init(std::shared_ptr<microsoft::projectairsim::SimServer> InSimServer,
            const FString& InputTilesDir,
            const microsoft::projectairsim::GeoPoint& GeoPoint,
            float AltitudeOffsetInMeters, std::string& HorizonTilesDir);

 private:
  template <typename R>
  inline bool IsFutureReady(std::future<R> const& Future) {
    return (Future.wait_for(std::chrono::seconds(0)) ==
            std::future_status::ready);
  }

  UTexture2D* ConvertTextureToUnrealTexture(
      const mrrs::TexutureData& TextureData);

  void ConvertMeshToUnrealData(const mrrs::MeshData& Mesh,
                               TArray<FVector>* OutVertices,
                               TArray<int32>* OutTriangles,
                               TArray<FVector2D>* OutUV0);

  AProcMeshActor* RenderTileMesh(mrrs::TileKey& CurTileKey,
                                 mrrs::MeshData const* CurTileMeshData,
                                 float SpawnOffsetZ = 0.f);

  void AddHorizonTiles(std::string& HorizonDir);

  std::vector<mrrs::TileKey> HandleMissingTiles(
      std::vector<mrrs::TileKey> tileKeysToRender, int lodSearchDepth,
      bool logVerbose);

  bool IsTileAvailable(mrrs::TileKey tileKey);

  bool GetTileChildrenAtDepth(mrrs::TileKey tileKey, int searchDepth,
                              std::vector<mrrs::TileKey>& output);

  std::shared_ptr<microsoft::projectairsim::SimServer> SimServer;
  FString TilesDir;
  float AltitudeOffset;
  microsoft::projectairsim::GeodeticConverter GeoConverter;
  std::unordered_map<mrrs::TileKey, AProcMeshActor*> ProcMeshActors;
  std::unordered_map<mrrs::TileKey, AProcMeshActor*> HorizonProcMeshActors;
  std::unordered_set<mrrs::TileKey> RenderedTileKeys;
  std::unique_ptr<mrrs::MeshDataProvider> MeshProvider;

  // Holds all loaded tiles (whether or not they're rendered)
  mutable mrrs::TileCache TileCache;
  mutable std::mutex TileCacheMutex;

  // To manage future tiles, to be loaded async.
  mutable std::unordered_set<mrrs::TileKey> TileKeysQueued;
  mutable std::deque<std::future<void>> TileKeyFutures;
  mutable std::mutex TileQueueMutex;

  // TODO Replace this temporary DestroyQ patch to avoid tile popping for demo.
  std::queue<std::vector<mrrs::TileKey>> DestroyQ;
  static constexpr int kDestroyQTicksToDelay = 10;

  static constexpr int kAsyncWorkloads = 4;
  static constexpr int kPendingWorkloadsAllowed = 16;
  static constexpr int kTileCacheSize = 1000;
  static constexpr int kMinTilesToLoadPerTick = 10;
  static constexpr int kMaxTilesToRenderPerTick =
      std::numeric_limits<int>::max();

  int missingTilesSearchDepth = 1;
};
