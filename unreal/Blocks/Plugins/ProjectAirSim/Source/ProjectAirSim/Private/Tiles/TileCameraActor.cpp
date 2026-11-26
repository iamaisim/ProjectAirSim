#include "Tiles/TileCameraActor.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Math/Float16Color.h"
#include "Tiles/TileMercatorLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogTileCamera, Log, All);

namespace {
constexpr int32 kCalibrationSize = 128;
constexpr float kMiddleGrey = 0.18f;
}  // namespace

ATileCameraActor::ATileCameraActor() {
  PrimaryActorTick.bCanEverTick = true;

  USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
  SetRootComponent(Root);

  CaptureComponent =
      CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
  CaptureComponent->SetupAttachment(RootComponent);
  CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
  CaptureComponent->bCaptureEveryFrame = false;
  CaptureComponent->bCaptureOnMovement = false;
  CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
  CaptureComponent->bAlwaysPersistRenderingState = true;

  SetActorRotation(UTileMercatorLibrary::GetTileCameraRotation(bNorthIsX));
}

void ATileCameraActor::BeginPlay() {
  Super::BeginPlay();

  if (bUpdateOnBeginPlay) {
    UpdateFromTile();
  }
}

void ATileCameraActor::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (bUpdateEveryTick) {
    UpdateFromTile();
  }
}

void ATileCameraActor::UpdateFromTile() {
  int32 YForCalc = TileY;
  if (bTileYIsTMS && Zoom >= 0 && Zoom <= 30) {
    YForCalc = static_cast<int32>((int64{1} << Zoom) - 1 - TileY);
  }

  if (ApplyTile(Zoom, TileX, YForCalc) && CaptureComponent &&
      bCaptureAfterUpdate) {
    CaptureComponent->CaptureScene();
  }
}

bool ATileCameraActor::ApplyTile(int32 InZoom, int32 InX, int32 InY) {
  FVector LocationCm = FVector::ZeroVector;
  double OrthoWidthCm = 0.0;
  if (!UTileMercatorLibrary::ComputeTileCamera(
          InZoom, InX, InY, OriginLat, OriginLon, AltitudeMeters, bNorthIsX,
          bUseMercatorScale, LocationCm, OrthoWidthCm)) {
    return false;
  }

  if (bUseManualExposure && bAutoCalibrateExposure && !bExposureCalibrated) {
    CalibrateExposure();
  }

  // Force a north-up, straight-down view regardless of how the actor or its
  // capture component were rotated in the level.
  SetActorLocationAndRotation(
      LocationCm + LocalOffsetCm,
      UTileMercatorLibrary::GetTileCameraRotation(bNorthIsX));

  if (CaptureComponent) {
    CaptureComponent->SetRelativeLocationAndRotation(FVector::ZeroVector,
                                                     FRotator::ZeroRotator);
    ApplyCaptureSettings();
    if (bApplyOrthoWidthToCapture) {
      CaptureComponent->OrthoWidth = static_cast<float>(OrthoWidthCm);
    }
  }
  return true;
}

void ATileCameraActor::ApplyCaptureSettings() {
  if (!CaptureComponent) {
    return;
  }

  CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
  CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
  CaptureComponent->PostProcessBlendWeight = 1.0f;
  // Without a persistent view state the renderer skips eye adaptation and
  // locks exposure to 1.0, ignoring the manual exposure set below.
  CaptureComponent->bAlwaysPersistRenderingState = true;
  // Each tile is an unrelated view, so drop any temporal history.
  CaptureComponent->bCameraCutThisFrame = true;

  // Every tile is captured once from a new position, so temporal effects only
  // add jitter/blur and break pixel alignment between neighbouring tiles.
  CaptureComponent->ShowFlags.SetTemporalAA(false);
  CaptureComponent->ShowFlags.SetMotionBlur(false);

  FPostProcessSettings& PostProcess = CaptureComponent->PostProcessSettings;
  PostProcess.bOverride_VignetteIntensity = true;
  PostProcess.VignetteIntensity = 0.0f;

  PostProcess.bOverride_AutoExposureMethod = bUseManualExposure;
  PostProcess.bOverride_AutoExposureApplyPhysicalCameraExposure =
      bUseManualExposure;
  PostProcess.bOverride_AutoExposureBias = bUseManualExposure;
  if (bUseManualExposure) {
    PostProcess.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
    PostProcess.AutoExposureApplyPhysicalCameraExposure = 0;
    PostProcess.AutoExposureBias =
        ExposureBias + ((bAutoCalibrateExposure && bExposureCalibrated)
                            ? CalibratedExposureBias
                            : 0.0f);
  }
}

bool ATileCameraActor::CalibrateExposure() {
  if (!CaptureComponent || !GetWorld()) {
    return false;
  }

  UTextureRenderTarget2D* HdrTarget = NewObject<UTextureRenderTarget2D>(this);
  HdrTarget->InitCustomFormat(kCalibrationSize, kCalibrationSize, PF_FloatRGBA,
                              true);
  HdrTarget->UpdateResourceImmediate(true);

  UTextureRenderTarget2D* PrevTarget = CaptureComponent->TextureTarget;
  const TEnumAsByte<ESceneCaptureSource> PrevSource =
      CaptureComponent->CaptureSource;

  SetActorLocationAndRotation(
      FVector(0.0, 0.0, AltitudeMeters * 100.0) + LocalOffsetCm,
      UTileMercatorLibrary::GetTileCameraRotation(bNorthIsX));
  CaptureComponent->SetRelativeLocationAndRotation(FVector::ZeroVector,
                                                   FRotator::ZeroRotator);
  ApplyCaptureSettings();
  CaptureComponent->OrthoWidth =
      static_cast<float>(FMath::Max(ExposureCalibrationWidthMeters, 1.0) * 100.0);
  // Linear scene color before exposure/tonemapping. Dropping the view state
  // makes pre-exposure 1, so this reads the raw scene luminance.
  CaptureComponent->bAlwaysPersistRenderingState = false;
  CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
  CaptureComponent->TextureTarget = HdrTarget;
  CaptureComponent->CaptureScene();

  TArray<FFloat16Color> Pixels;
  FTextureRenderTargetResource* Resource =
      HdrTarget->GameThread_GetRenderTargetResource();
  const bool bRead = Resource && Resource->ReadFloat16Pixels(Pixels);

  CaptureComponent->bAlwaysPersistRenderingState = true;
  CaptureComponent->CaptureSource = PrevSource;
  CaptureComponent->TextureTarget = PrevTarget;

  TArray<float> LogLuminance;
  LogLuminance.Reserve(Pixels.Num());
  for (const FFloat16Color& Pixel : Pixels) {
    const float Luminance = 0.2126f * Pixel.R.GetFloat() +
                            0.7152f * Pixel.G.GetFloat() +
                            0.0722f * Pixel.B.GetFloat();
    if (FMath::IsFinite(Luminance) && Luminance > 1e-6f) {
      LogLuminance.Add(FMath::Log2(Luminance));
    }
  }

  if (!bRead || LogLuminance.Num() == 0) {
    UE_LOG(LogTileCamera, Warning,
           TEXT("Exposure calibration failed; using ExposureBias only."));
    return false;
  }

  // Average the 10th-90th percentile in log space, like the default
  // histogram auto exposure, and map it to middle grey.
  LogLuminance.Sort();
  const int32 Low = LogLuminance.Num() / 10;
  const int32 High = FMath::Max(Low + 1, LogLuminance.Num() * 9 / 10);
  double Sum = 0.0;
  for (int32 Index = Low; Index < High; ++Index) {
    Sum += LogLuminance[Index];
  }
  const float AverageLog2 = static_cast<float>(Sum / (High - Low));

  CalibratedExposureBias =
      FMath::Clamp(FMath::Log2(kMiddleGrey) - AverageLog2, -16.0f, 16.0f);
  bExposureCalibrated = true;
  UE_LOG(LogTileCamera, Log,
         TEXT("Exposure calibrated: average luminance %.4f -> bias %.2f EV"),
         FMath::Pow(2.0f, AverageLog2), CalibratedExposureBias);
  return true;
}
