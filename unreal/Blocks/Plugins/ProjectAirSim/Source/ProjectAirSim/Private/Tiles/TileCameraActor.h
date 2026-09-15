#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileCameraActor.generated.h"

class USceneCaptureComponent2D;

UCLASS()
class PROJECTAIRSIM_API ATileCameraActor : public AActor {
  GENERATED_BODY()

 public:
  ATileCameraActor();

 protected:
  virtual void BeginPlay() override;

 public:
  virtual void Tick(float DeltaTime) override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  int32 Zoom = 20;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  int32 TileX = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  int32 TileY = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  double OriginLat = 47.641468;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  double OriginLon = -122.140165;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  double AltitudeMeters = 122.0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  bool bNorthIsX = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  bool bUseMercatorScale = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  FVector LocalOffsetCm = FVector::ZeroVector;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  bool bTileYIsTMS = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  bool bUpdateOnBeginPlay = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  bool bUpdateEveryTick = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  bool bApplyOrthoWidthToCapture = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  bool bCaptureAfterUpdate = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tiles")
  USceneCaptureComponent2D* CaptureComponent = nullptr;

  UFUNCTION(BlueprintCallable, Category = "Tiles")
  void UpdateFromTile();
};
