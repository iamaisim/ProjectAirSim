// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "GISRenderer.h"

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Constant.h"
#include "Interfaces/IPluginManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/FileHelper.h"
#include "ProceduralMeshComponent.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"
#include "UnrealLogger.h"
#include "bing_maps_utils.hpp"
#include "core_sim/math_utils.hpp"
#include "tile_manager.hpp"

// Includes for internal prototype usage of DEM tiles
#include <filesystem>

#include "bing_maps_utils.hpp"
#include "tile_info.hpp"

AGISRenderer::AGISRenderer()
    : SimServer(nullptr),
      AltitudeOffset(0.f),
      MeshProvider(nullptr),
      TileCache(kTileCacheSize) {
  PrimaryActorTick.bCanEverTick = true;
}

AGISRenderer::~AGISRenderer() {
  while (!TileKeyFutures.empty()) {
    std::future<void>& Future = TileKeyFutures.front();
    if (Future.valid()) Future.get();
    TileKeyFutures.pop_front();
  }
}

// Called when the game starts or when spawned
void AGISRenderer::BeginPlay() { Super::BeginPlay(); }

void AGISRenderer::Init(
    std::shared_ptr<microsoft::projectairsim::SimServer> InSimServer,
    const FString& InputTilesDir,
    const microsoft::projectairsim::GeoPoint& GeoPoint,
    float AltitudeOffsetInMeters, std::string& HorizonTilesDir) {
  SimServer = InSimServer;
  TilesDir = InputTilesDir;
  GeoConverter = microsoft::projectairsim::GeodeticConverter(
      GeoPoint.latitude, GeoPoint.longitude, GeoPoint.altitude);

  AltitudeOffset = AltitudeOffsetInMeters * 100;  // UE cm

  std::unique_ptr<mrrs::FileReaderInterface> FileReader = nullptr;

  if (FPaths::DirectoryExists(TilesDir)) {  // use tile directory provided
    UnrealLogger::Log(microsoft::projectairsim::LogLevel::kTrace,
                      TEXT("[GISRenderer] Using tile source dir: '%s'"),
                      *TilesDir);
    FileReader = std::make_unique<mrrs::DefaultFileReader>(
        std::string(TCHAR_TO_UTF8(*TilesDir)));
  } else {  // use tiles packed into binary as a fallback
    UnrealLogger::Log(microsoft::projectairsim::LogLevel::kTrace,
                      TEXT("[GISRenderer] Tile source dir '%s' not found. "
                           "Ignoring tile source directory and using tiles "
                           "packaged into binary instead."),
                      *TilesDir);
    FileReader = std::make_unique<UnrealFileSystemReader>();
  }

  MeshProvider =
      std::make_unique<mrrs::GLTFDataProvider>(std::move(FileReader));

  if (!HorizonTilesDir.empty()) {
    // Load and render ALL horizon tiles at scene init. These will remain
    // static, to be unloaded only when the whole scene is unloaded.
    AddHorizonTiles(HorizonTilesDir);
  }
}

// We should be suing pimpl pattern for below fcns
UTexture2D* AGISRenderer::ConvertTextureToUnrealTexture(
    const mrrs::TexutureData& TextureData) {
  if (TextureData.width <= 0 || TextureData.height <= 0 ||
      TextureData.buffer.empty()) {
    return nullptr;
  }

  UTexture2D* Texture = UTexture2D::CreateTransient(
      TextureData.width, TextureData.height, EPixelFormat::PF_R8G8B8A8);
  FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];

  void* MipData = Mip.BulkData.Lock(LOCK_READ_WRITE);
  FMemory::Memcpy(MipData, TextureData.buffer.data(),
                  TextureData.buffer.size());
  Mip.BulkData.Unlock();

#undef UpdateResource
  Texture->UpdateResource();
#ifdef UNICODE
#define UpdateResource UpdateResourceW
#else
#define UpdateResource UpdateResourceA
#endif  // !UNICODE

  return Texture;
}

void AGISRenderer::ConvertMeshToUnrealData(const mrrs::MeshData& Mesh,
                                           TArray<FVector>* OutVertices,
                                           TArray<int32>* OutTriangles,
                                           TArray<FVector2D>* OutUV0) {
  // assert Mesh.vertices == Mesh.uvs;
  for (uint64_t Idx = 0; Idx < Mesh.vertices.size(); ++Idx) {
    // ECEF m -> NED m with rotation to flat (z down)
    const float EcefX = Mesh.vertices[Idx].x();
    const float EcefY = Mesh.vertices[Idx].y();
    const float EcefZ = Mesh.vertices[Idx].z();
    double North, East, Down;
    GeoConverter.ecef2Ned(EcefX, EcefY, EcefZ, &North, &East, &Down);

    // NED m -> UE NEU cm
    const float UeX = North * 100;
    const float UeY = East * 100;
    const float UeZ = -Down * 100;
    OutVertices->Add(FVector(UeX, UeY, UeZ));

    const float U = Mesh.uvs[Idx].x();
    const float V = Mesh.uvs[Idx].y();
    OutUV0->Add(FVector2D(U, V));
  }

  for (uint64_t Idx = 0; Idx < Mesh.triangle_indices.size(); ++Idx) {
    OutTriangles->Add(Mesh.triangle_indices[Idx]);
  }
}

AProcMeshActor* AGISRenderer::RenderTileMesh(
    mrrs::TileKey& CurTileKey, mrrs::MeshData const* CurTileMeshData,
    float SpawnOffsetZ) {
  FActorSpawnParameters ProcMeshActorSpawnInfo;
  ProcMeshActorSpawnInfo.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

  FVector SpawnLocation(0, 0, SpawnOffsetZ);
  FRotator SpawnRotation(0, 0, 0);

  auto ProcMesh = GetWorld()->SpawnActor<AProcMeshActor>(
      FVector::ZeroVector, FRotator::ZeroRotator, ProcMeshActorSpawnInfo);

  FTransform transform(SpawnRotation, SpawnLocation);
  ProcMesh->Init(1, transform, std::vector<float>({1, 1, 1}), false);

  ProcMesh->lod = CurTileKey.lod;
  ProcMesh->x = CurTileKey.x;
  ProcMesh->y = CurTileKey.y;
  ProcMesh->tilekey = FString(mrrs::BingMapsUtils::TileXYToQuadkey(
                                  CurTileKey.x, CurTileKey.y, CurTileKey.lod)
                                  .c_str());

  TArray<FVector> Vertices;
  TArray<int32> Triangles;
  TArray<FVector2D> UV0;
  TArray<FVector> Normals;
  TArray<FProcMeshTangent> Tangents;
  ConvertMeshToUnrealData(*CurTileMeshData, &Vertices, &Triangles, &UV0);

  // Set color values for GIS tiles which use base color textures and no
  // emissive color
  const FLinearColor BaseColor(1.0f, 1.0f, 1.0f, 1.0f);
  const FLinearColor EmissiveColor(0.0f, 0.0f, 0.0f, 0.0f);
  const float MetallicFactor = 0.0f;
  const float RoughnessFactor = 1.0f;
  const float SpecularFactor = 0.5f;

  UTexture2D* BaseColorTexture =
      ConvertTextureToUnrealTexture(CurTileMeshData->texture);

  ProcMesh->UpdateMesh(0, Vertices, Triangles, UV0, Normals, Tangents,
                       BaseColor, EmissiveColor, MetallicFactor,
                       RoughnessFactor, SpecularFactor, BaseColorTexture);
  return ProcMesh;
}

// Called every frame
void AGISRenderer::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (!SimServer || !MeshProvider) return;

  // ---------------------------------------------------------------------
  // Get tile keys to render based on latest robot locations

  auto TileKeys =
      HandleMissingTiles(SimServer->GetTileManager()->GetTileKeysToRender(),
                         missingTilesSearchDepth, false);

  std::unordered_set<mrrs::TileKey> TileKeysSet(TileKeys.begin(),
                                                TileKeys.end());

  // ---------------------------------------------------------------------
  // Load tile Mesh data from tile keys

  std::vector<std::pair<mrrs::TileKey, mrrs::MeshData const*>>
      TileMeshesToRender;

  // Clear any tile futures that have already completed
  auto TileKeysItr = TileKeyFutures.begin();
  while (TileKeysItr != TileKeyFutures.end()) {
    if (!TileKeysItr->valid() || IsFutureReady(*TileKeysItr)) {
      TileKeysItr = TileKeyFutures.erase(TileKeysItr);
    } else {
      TileKeysItr++;
    }
  }

  // If we have too many futures pending, wait until some are done
  while (TileKeyFutures.size() >= kPendingWorkloadsAllowed) {
    std::future<void>& Future = TileKeyFutures.front();
    if (Future.valid()) Future.get();
    TileKeyFutures.pop_front();
  }

  // Decide if each tile key needs to be cached/queued/rendered
  std::vector<mrrs::TileKey> KeysForWorkload;
  for (int Idx = 0; Idx < TileKeys.size(); Idx++) {
    const auto& CurTileKey = TileKeys[Idx];

    // If tile is already rendered, no need to do anything for it here
    if (RenderedTileKeys.count(CurTileKey) != 0) continue;

    std::lock_guard<std::mutex> CacheLockGuard(TileCacheMutex);
    std::lock_guard<std::mutex> QueueLockGuard(TileQueueMutex);

    // Ensure the first kMinTilesToLoadPerTick are loaded in the tile cache
    if (Idx < kMinTilesToLoadPerTick) {
      if (!TileCache.IsCached(CurTileKey)) {
        // Load tile Mesh data for this tile key
        auto MeshData = MeshProvider->GetTileMeshData(CurTileKey);

        if (MeshData.vertices.size() > 0) {
          TileCache.AddTile(CurTileKey, std::move(MeshData));
        }
        TileKeysQueued.erase(CurTileKey);
      }
    }

    if (TileCache.IsCached(CurTileKey) &&
        TileMeshesToRender.size() < kMaxTilesToRenderPerTick) {
      // If this key is available in tile cache, add it to TileMeshesToRender to
      // be rendered
      TileMeshesToRender.emplace_back(CurTileKey,
                                      TileCache.GetMesh(CurTileKey));
    } else if (TileKeysQueued.count(CurTileKey) == 0) {
      // Not present in cache yet (either file doesn't exists or we need to
      // load) and not already queued for async loading, so add it to the tile
      // key workload and queue
      KeysForWorkload.push_back(CurTileKey);
      TileKeysQueued.insert(CurTileKey);
    }
  }

  // Process the tile key workload to start loading them
  if (kAsyncWorkloads > 0 && KeysForWorkload.size() >= kAsyncWorkloads) {
    // Split tile keys across the async workloads
    int NumItemsInWorkload = KeysForWorkload.size() / kAsyncWorkloads;
    int ExtraWorkloadInEnd = KeysForWorkload.size() % kAsyncWorkloads;

    for (int Idx = 0; Idx < kAsyncWorkloads; Idx++) {
      int StartIndex = Idx * NumItemsInWorkload;
      int EndIndex = StartIndex + NumItemsInWorkload - 1;
      if (Idx == kAsyncWorkloads - 1) {
        EndIndex += ExtraWorkloadInEnd;
      }

      // Constructing vector with start/end itr does range [first, last) where
      // the last index is not included, so need to add one to the EndIndex for
      // it to be included
      std::vector<mrrs::TileKey> SubWorkloadData(
          &KeysForWorkload[StartIndex], &KeysForWorkload[EndIndex + 1]);

      std::future<void> WorkloadFuture = std::async(
          std::launch::async, [this, TileKeys{std::move(SubWorkloadData)}]() {
            for (const auto TileKey : TileKeys) {
              auto MeshData = this->MeshProvider->GetTileMeshData(TileKey);
              if (MeshData.vertices.size() > 0) {
                std::lock_guard<std::mutex> LockGuard(TileCacheMutex);
                TileCache.AddTile(TileKey, std::move(MeshData));
              }
              std::lock_guard<std::mutex> LockGuard(TileQueueMutex);
              this->TileKeysQueued.erase(TileKey);
            }
          });

      TileKeyFutures.push_back(std::move(WorkloadFuture));
    }
  } else if (KeysForWorkload.size() > 0) {
    // If the number of keys are less than the number of workloads to split
    // across, no point in splitting them.

    // TODO Should this small workload still be launched as a single async
    // workload future?
    for (auto Key : KeysForWorkload) {
      auto MeshData = MeshProvider->GetTileMeshData(Key);

      if (MeshData.vertices.size() > 0) {
        std::lock_guard<std::mutex> LockGuard(TileCacheMutex);
        TileCache.AddTile(Key, std::move(MeshData));
        TileMeshesToRender.emplace_back(Key, TileCache.GetMesh(Key));
      }

      std::lock_guard<std::mutex> LockGuard(TileQueueMutex);
      TileKeysQueued.erase(Key);
    }
  }

  // ---------------------------------------------------------------------
  // Spawn tile meshes from Mesh data to render

  for (size_t Idx = 0; Idx < TileMeshesToRender.size(); ++Idx) {
    auto CurTileKey = TileMeshesToRender[Idx].first;
    auto CurTileMeshData = TileMeshesToRender[Idx].second;

    if (!CurTileMeshData->vertices.empty()) {
      auto Itr = ProcMeshActors.find(CurTileKey);
      if (Itr == ProcMeshActors.end()) {
        AProcMeshActor* ProcMesh =
            RenderTileMesh(CurTileKey, CurTileMeshData, AltitudeOffset);
        ProcMeshActors[CurTileKey] = ProcMesh;
        RenderedTileKeys.insert(CurTileKey);
      }
    }
  }

  // ---------------------------------------------------------------------
  // Delete rendered tile meshes that are not in the set of tile keys

  // TODO Replace this temporary DestroyQ patch to avoid tile popping for demo.
  // Push batch of tiles to destroy onto DestroyQ
  std::vector<mrrs::TileKey> TilesToDestroyLater;
  for (auto Itr = ProcMeshActors.begin(); Itr != ProcMeshActors.end(); ++Itr) {
    if (TileKeysSet.count(Itr->first) == 0) {
      TilesToDestroyLater.push_back(Itr->first);
    }
  }
  DestroyQ.push(TilesToDestroyLater);

  // After DestroyQ has passed kDestroyQTicksToDelay, start destroying the batch
  // of tiles at the front of the queue
  if (DestroyQ.size() > AGISRenderer::kDestroyQTicksToDelay) {
    std::vector<mrrs::TileKey> TilesToDestroyNow = DestroyQ.front();
    DestroyQ.pop();
    for (const mrrs::TileKey& Tile : TilesToDestroyNow) {
      auto TileActorItr = ProcMeshActors.find(Tile);
      if (TileActorItr != ProcMeshActors.end()) {
        TileActorItr->second->Destroy();
        RenderedTileKeys.erase(Tile);
        ProcMeshActors.erase(Tile);
      }
    }
  }
}

void AGISRenderer::UnloadAllProcMeshActors() {
  for (auto& ProcMeshPair : ProcMeshActors) {
    ProcMeshPair.second->Destroy();
  }

  for (auto& ProcMeshPair : HorizonProcMeshActors) {
    ProcMeshPair.second->Destroy();
  }

  ProcMeshActors.clear();
  HorizonProcMeshActors.clear();
  RenderedTileKeys.clear();
}

std::string UnrealFileSystemReader::ReadFile(
    const std::string& FileName) const {
  auto FilePath = FPaths::ProjectContentDir() + Constant::GltfTilesToPakPath +
                  "/" + FString(FileName.c_str());

  TArray<uint8> OutBytes;
  if (FFileHelper::LoadFileToArray(OutBytes, *FilePath, FILEREAD_Silent)) {
    return std::string(OutBytes.GetData(), OutBytes.GetData() + OutBytes.Num());
  } else {
    return "";
  }
}

// Temporary hack to render globe tiles all at once when loading the scene.
// TODO:
// 1. Make loading async.
// 2. Set TileAvailable::OUTOFBOUNDS condition instead of fixed 200 sqkm bounds
// 3. Fix ned2Geodetic wrong alts when home geopoint is far.
// 4. Calculate distance to horizon and don't load / unload unneeded tiles.
// *Note: you might need to increase kMinTilesToLoadPerTick to 20+ to avoid
// clogging when moving at fast speeds
void AGISRenderer::AddHorizonTiles(std::string& HorizonDir) {
  auto HorizonFileReader =
      std::make_unique<mrrs::DefaultFileReader>(HorizonDir);

  auto HorizonMeshProvider =
      std::make_unique<mrrs::GLTFDataProvider>(std::move(HorizonFileReader));

  for (const auto& file : std::filesystem::directory_iterator(HorizonDir)) {
    auto filePath = file.path();
    auto quadKey = filePath.stem().string();
    int x, y, lod;
    mrrs::BingMapsUtils::TryCreateTilePosition(
        mrrs::BingMapsUtils::TileIdFromQuadkey(quadKey), &x, &y, &lod);
    auto TileKey = mrrs::TileKey(x, y, lod);

    auto MeshData = HorizonMeshProvider->GetTileMeshData(TileKey);
    AProcMeshActor* ProcMesh = RenderTileMesh(TileKey, &MeshData, 0.f);
    HorizonProcMeshActors[TileKey] = ProcMesh;
  }
}

// This algorithm assumes GetTileKeysToRender() works correctly and
// doesn't add the same tiles of different lods
std::vector<mrrs::TileKey> AGISRenderer::HandleMissingTiles(
    std::vector<mrrs::TileKey> tileKeysToRender, int lodSearchDepth,
    bool logVerbose) {
  // Convert to set for quick erasing
  std::unordered_set<mrrs::TileKey> tileKeysToRenderSet(
      tileKeysToRender.begin(), tileKeysToRender.end());

  // Used to store child tiles that will no longer be rendered
  std::unordered_set<mrrs::TileKey> tileKeysToIgnore;

  // Used to store final tiles to render
  std::unordered_set<mrrs::TileKey> resultTileKeys;

  for (const mrrs::TileKey& t : tileKeysToRenderSet) {
    if (tileKeysToIgnore.find(t) != tileKeysToIgnore.end()) continue;

    // Check if file exists in the tile directory.
    // If not, replace this tile and its siblings with a lower-lod tile
    if (!IsTileAvailable(t)) {
      // Search for parent tile within the specified search depth. If parent
      // exists, add it to tileKeysToRender, then remove all child tiles.
      for (int i = 1; i <= lodSearchDepth; i++) {
        int parentLod = t.lod - i;
        if (parentLod < SimServer->GetTileManager()->min_tile_lod_) break;

        // Get parent tile
        mrrs::TileKey tempParentTile = mrrs::TileKey(
            t.x / std::pow(2, i), t.y / std::pow(2, i), parentLod);

        if (IsTileAvailable(tempParentTile)) {
          resultTileKeys.insert(tempParentTile);

          // Logging
          if (logVerbose) {
            FString tQuadKey =
                mrrs::BingMapsUtils::TileXYToQuadkey(t.x, t.y, t.lod).c_str();
            FString tempParentQuadKey =
                mrrs::BingMapsUtils::TileXYToQuadkey(
                    tempParentTile.x, tempParentTile.y, tempParentTile.lod)
                    .c_str();
            UnrealLogger::Log(
                microsoft::projectairsim::LogLevel::kVerbose,
                TEXT("[GISRenderer] Tile '%s'(lod-'%d') not available. Using "
                     "'%s'(lod-'%d') instead."),
                *tQuadKey, t.lod, *tempParentQuadKey, tempParentTile.lod);
          }

          // Find and remove all child tiles within the used searched depth
          std::vector<mrrs::TileKey> allChildTilesAtDepth;
          GetTileChildrenAtDepth(tempParentTile, i, allChildTilesAtDepth);

          for (const mrrs::TileKey tileToRemove : allChildTilesAtDepth) {
            resultTileKeys.erase(tileToRemove);
            tileKeysToIgnore.insert(tileToRemove);
          }

          break;
        }
      }
    } else {
      resultTileKeys.insert(t);
    }
  }

  return std::vector<mrrs::TileKey>(resultTileKeys.begin(),
                                    resultTileKeys.end());
}

bool AGISRenderer::IsTileAvailable(mrrs::TileKey tileKey) {
  std::string quadKey =
      mrrs::BingMapsUtils::TileXYToQuadkey(tileKey.x, tileKey.y, tileKey.lod);
  return std::filesystem::exists(TCHAR_TO_UTF8(*TilesDir) + quadKey + ".glb");
}

bool AGISRenderer::GetTileChildrenAtDepth(mrrs::TileKey tileKey,
                                          int searchDepth,
                                          std::vector<mrrs::TileKey>& output) {
  if (searchDepth < 1) return false;

  std::vector<mrrs::TileKey> tempParents;
  tempParents.push_back(tileKey);

  for (int depth = 0; depth < searchDepth; depth++) {
    std::vector<mrrs::TileKey> tempNewParents;

    for (const mrrs::TileKey tempParent : tempParents) {
      mrrs::TileKey tempChildTiles[4];
      tempParent.GetChildTiles(&tempChildTiles[0]);

      for (int i = 0; i < 4; i++) {
        output.push_back(tempChildTiles[i]);
        tempNewParents.push_back(tempChildTiles[i]);
      }
    }
    tempParents = tempNewParents;
  }

  return true;
}