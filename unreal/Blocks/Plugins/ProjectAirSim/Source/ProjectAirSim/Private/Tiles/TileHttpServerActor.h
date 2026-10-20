#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HAL/CriticalSection.h"
#include "HttpRequestHandler.h"
#include "HttpRouteHandle.h"
#include "HttpServerRequest.h"
#include "HttpServerResponse.h"
#include "TileHttpServerActor.generated.h"

class ATileCameraActor;
class IHttpRouter;

UCLASS()
class PROJECTAIRSIM_API ATileHttpServerActor : public AActor {
  GENERATED_BODY()

 public:
  ATileHttpServerActor();

 protected:
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

 public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTTP")
  int32 Port = 8080;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTTP")
  FString RoutePrefix = TEXT("/tiles");

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTTP")
  bool bStartOnBeginPlay = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HTTP")
  bool bLogRequests = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  FString TileRootDir = TEXT("Tiles");

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles", meta = (ClampMin = "0", ClampMax = "30"))
  int32 MinZoom = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles", meta = (ClampMin = "0", ClampMax = "30"))
  int32 MaxZoom = 20;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  int32 TileSize = 256;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  bool bTileYIsTMS = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
  bool bRenderMissingTiles = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
  ATileCameraActor* TileCamera = nullptr;

  UFUNCTION(BlueprintCallable, Category = "HTTP")
  void StartServer();

  UFUNCTION(BlueprintCallable, Category = "HTTP")
  void StopServer();

 private:
  void NormalizeZoomSettings();
  bool HandleTileRequest(const FHttpServerRequest& Request,
                         const FHttpResultCallback& OnComplete);
  bool RenderTileToPng(int32 Zoom, int32 X, int32 Y, TArray<uint8>& OutPng);
  bool LoadFileToArray(const FString& FilePath, TArray<uint8>& OutData) const;
  bool SaveArrayToFile(const FString& FilePath, const TArray<uint8>& Data) const;
  FString GetAbsoluteTileRoot() const;
  FString BuildTilePath(int32 Zoom, int32 X, int32 Y) const;

  TSharedPtr<IHttpRouter> HttpRouter;
  FHttpRouteHandle RouteHandle;
  FCriticalSection RenderMutex;
};
