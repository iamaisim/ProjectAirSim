#include "Tiles/TileCameraActor.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"
#include "Engine/SceneCapture2D.h"
#include "Tiles/TileMercatorLibrary.h"

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

  SetActorRotation(FRotator(-90.0f, 0.0f, 0.0f));
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
  if (bTileYIsTMS && Zoom >= 0) {
    const int32 MaxIndex = (1 << Zoom) - 1;
    YForCalc = MaxIndex - TileY;
  }

  FVector LocationCm = FVector::ZeroVector;
  double OrthoWidthCm = 0.0;
  UTileMercatorLibrary::ComputeTileCamera(
      Zoom, TileX, YForCalc, OriginLat, OriginLon, AltitudeMeters, bNorthIsX,
      bUseMercatorScale, LocationCm, OrthoWidthCm);

  SetActorLocation(LocationCm + LocalOffsetCm);

  if (CaptureComponent && bApplyOrthoWidthToCapture) {
    CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
    CaptureComponent->OrthoWidth = OrthoWidthCm;
  }

  if (CaptureComponent && bCaptureAfterUpdate) {
    CaptureComponent->CaptureScene();
  }
}
