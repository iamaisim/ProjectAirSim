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

  /** Latitude of the Unreal world origin. Must match the scene's home-geo-point. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  double OriginLat = 47.641468;

  /** Longitude of the Unreal world origin. Must match the scene's home-geo-point. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  double OriginLon = -122.140165;

  /** Camera height above the Unreal world origin. Must be above all geometry. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  double AltitudeMeters = 122.0;

  /** true: X=North, Y=East (ProjectAirSim). false: X=East, Y=South (Cesium). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  bool bNorthIsX = true;

  /** Use raw Web Mercator meters instead of ProjectAirSim's GPS projection. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
  bool bUseMercatorScale = false;

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

  /**
   * Use one fixed exposure for every tile. Auto exposure does not work in
   * single-shot captures (no eye-adaptation history) and would differ per tile.
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles|Exposure")
  bool bUseManualExposure = true;

  /**
   * Measure the scene once around the world origin and pick the exposure that
   * maps its average brightness to middle grey, like auto exposure does.
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles|Exposure",
            meta = (EditCondition = "bUseManualExposure"))
  bool bAutoCalibrateExposure = true;

  /** Width of the area around the world origin used for calibration. */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles|Exposure",
            meta = (EditCondition = "bUseManualExposure && bAutoCalibrateExposure",
                    ClampMin = "1.0"))
  double ExposureCalibrationWidthMeters = 400.0;

  /** Extra exposure compensation in EV (+1 doubles brightness). */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles|Exposure",
            meta = (EditCondition = "bUseManualExposure"))
  float ExposureBias = 0.0f;

  UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
            Category = "Tiles|Exposure")
  float CalibratedExposureBias = 0.0f;

  UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient,
            Category = "Tiles|Exposure")
  bool bExposureCalibrated = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tiles")
  USceneCaptureComponent2D* CaptureComponent = nullptr;

  UFUNCTION(BlueprintCallable, Category = "Tiles")
  void UpdateFromTile();

  /**
   * Positions and configures the capture for XYZ tile (InZoom, InX, InY),
   * where InY counts from the north edge. Returns false for invalid tiles.
   */
  UFUNCTION(BlueprintCallable, Category = "Tiles")
  bool ApplyTile(int32 InZoom, int32 InX, int32 InY);

  /** Applies capture settings that make tiles consistent and seamless. */
  UFUNCTION(BlueprintCallable, Category = "Tiles")
  void ApplyCaptureSettings();

  /**
   * (Re)measures scene brightness for bAutoCalibrateExposure. Runs
   * automatically before the first tile; call again after lighting changes.
   */
  UFUNCTION(BlueprintCallable, Category = "Tiles|Exposure")
  bool CalibrateExposure();
};
