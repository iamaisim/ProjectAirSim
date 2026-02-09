#include "Tiles/TileHttpServerActor.h"

#include "Async/Async.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformProcess.h"
#include "HttpPath.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Tiles/TileCameraActor.h"
#include "Tiles/TileMercatorLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogTileHttpServer, Log, All);

ATileHttpServerActor::ATileHttpServerActor() {
  PrimaryActorTick.bCanEverTick = false;

  Port = 8080;
  TileSize = 256;
  MinZoom = 0;
  MaxZoom = 20;
  bStartOnBeginPlay = true;
  bLogRequests = true;
}

void ATileHttpServerActor::BeginPlay() {
  Super::BeginPlay();

  NormalizeZoomSettings();
  if (bStartOnBeginPlay) {
    StartServer();
  }
}

void ATileHttpServerActor::EndPlay(const EEndPlayReason::Type EndPlayReason) {
  StopServer();
  Super::EndPlay(EndPlayReason);
}

void ATileHttpServerActor::StartServer() {
  if (HttpRouter.IsValid()) {
    return;
  }

  NormalizeZoomSettings();
  FHttpServerModule& HttpServerModule = FHttpServerModule::Get();
  HttpRouter = HttpServerModule.GetHttpRouter(Port);
  if (!HttpRouter.IsValid()) {
    UE_LOG(LogTileHttpServer, Warning,
           TEXT("Failed to get HTTP router on port %d"), Port);
    return;
  }

  const FString Route = FString::Printf(TEXT("%s/:z/:x/:y"), *RoutePrefix);
  TWeakObjectPtr<ATileHttpServerActor> WeakThis(this);
  const FHttpRequestHandler Handler = FHttpRequestHandler::CreateLambda(
      [WeakThis](const FHttpServerRequest& Request,
                 const FHttpResultCallback& OnComplete) {
        if (!WeakThis.IsValid()) {
          return false;
        }
        return WeakThis->HandleTileRequest(Request, OnComplete);
      });
  RouteHandle =
      HttpRouter->BindRoute(FHttpPath(Route), EHttpServerRequestVerbs::VERB_GET,
                            Handler);

  HttpServerModule.StartAllListeners();
  UE_LOG(LogTileHttpServer, Log,
         TEXT("Tile HTTP server listening on port %d, route %s"), Port, *Route);
}

void ATileHttpServerActor::StopServer() {
  if (!HttpRouter.IsValid()) {
    return;
  }

  if (RouteHandle.IsValid()) {
    HttpRouter->UnbindRoute(RouteHandle);
    RouteHandle.Reset();
  }

  FHttpServerModule::Get().StopAllListeners();
  HttpRouter.Reset();
  UE_LOG(LogTileHttpServer, Log, TEXT("Tile HTTP server stopped"));
}

bool ATileHttpServerActor::HandleTileRequest(const FHttpServerRequest& Request,
                                             const FHttpResultCallback& OnComplete) {
  if (bLogRequests) {
    UE_LOG(LogTileHttpServer, Log, TEXT("Request: %s"),
           *Request.RelativePath.GetPath());
  }

  const FString* ZStr = Request.PathParams.Find(TEXT("z"));
  const FString* XStr = Request.PathParams.Find(TEXT("x"));
  const FString* YStr = Request.PathParams.Find(TEXT("y"));
  if (!ZStr || !XStr || !YStr) {
    if (bLogRequests) {
      UE_LOG(LogTileHttpServer, Warning, TEXT("Missing path params for %s"),
             *Request.RelativePath.GetPath());
    }
    OnComplete(FHttpServerResponse::Error(EHttpServerResponseCodes::BadRequest,
                                          TEXT("BadRequest"),
                                          TEXT("Missing path params")));
    return true;
  }

  FString YValue = **YStr;
  if (YValue.EndsWith(TEXT(".png"), ESearchCase::IgnoreCase)) {
    YValue.LeftChopInline(4);
  }

  if (YValue.IsEmpty()) {
    if (bLogRequests) {
      UE_LOG(LogTileHttpServer, Warning, TEXT("Invalid Y value for %s"),
             *Request.RelativePath.GetPath());
    }
    OnComplete(FHttpServerResponse::Error(EHttpServerResponseCodes::BadRequest,
                                          TEXT("BadRequest"),
                                          TEXT("Invalid tile y")));
    return true;
  }

  const int32 Zoom = FCString::Atoi(**ZStr);
  const int32 X = FCString::Atoi(**XStr);
  const int32 Y = FCString::Atoi(*YValue);

  if (Zoom < MinZoom || Zoom > MaxZoom) {
    if (bLogRequests) {
      UE_LOG(LogTileHttpServer, Warning,
             TEXT("Zoom out of range: z=%d (min=%d max=%d)"), Zoom, MinZoom,
             MaxZoom);
    }
    OnComplete(FHttpServerResponse::Error(EHttpServerResponseCodes::NotFound,
                                          TEXT("NotFound"),
                                          TEXT("Zoom out of range")));
    return true;
  }

  const FString TilePath = BuildTilePath(Zoom, X, Y);
  TArray<uint8> Data;
  if (LoadFileToArray(TilePath, Data)) {
    if (bLogRequests) {
      UE_LOG(LogTileHttpServer, Log, TEXT("Cache hit: z=%d x=%d y=%d"), Zoom, X,
             Y);
    }
    OnComplete(FHttpServerResponse::Create(MoveTemp(Data), TEXT("image/png")));
    return true;
  }

  if (!bRenderMissingTiles || !TileCamera) {
    if (bLogRequests) {
      UE_LOG(LogTileHttpServer, Log,
             TEXT("Cache miss: z=%d x=%d y=%d (render disabled)"), Zoom, X, Y);
    }
    OnComplete(FHttpServerResponse::Error(EHttpServerResponseCodes::NotFound,
                                          TEXT("NotFound"),
                                          TEXT("Tile missing")));
    return true;
  }

  TArray<uint8> Png;
  bool bRendered = false;

  auto RenderOnGameThread = [this, Zoom, X, Y, &Png, &bRendered]() {
    FScopeLock Lock(&RenderMutex);
    bRendered = RenderTileToPng(Zoom, X, Y, Png);
  };

  if (IsInGameThread()) {
    RenderOnGameThread();
  } else {
    FEvent* DoneEvent = FPlatformProcess::GetSynchEventFromPool();
    AsyncTask(ENamedThreads::GameThread, [RenderOnGameThread, DoneEvent]() {
      RenderOnGameThread();
      DoneEvent->Trigger();
    });
    DoneEvent->Wait();
    FPlatformProcess::ReturnSynchEventToPool(DoneEvent);
  }

  if (!bRendered) {
    if (bLogRequests) {
      UE_LOG(LogTileHttpServer, Warning, TEXT("Render failed: z=%d x=%d y=%d"),
             Zoom, X, Y);
    }
    OnComplete(FHttpServerResponse::Error(EHttpServerResponseCodes::NotFound,
                                          TEXT("NotFound"),
                                          TEXT("Tile render failed")));
    return true;
  }

  SaveArrayToFile(TilePath, Png);
  if (bLogRequests) {
    UE_LOG(LogTileHttpServer, Log, TEXT("Rendered: z=%d x=%d y=%d (saved)"),
           Zoom, X, Y);
  }
  OnComplete(FHttpServerResponse::Create(MoveTemp(Png), TEXT("image/png")));
  return true;
}

void ATileHttpServerActor::NormalizeZoomSettings() {
  const int32 OldMinZoom = MinZoom;
  const int32 OldMaxZoom = MaxZoom;

  MinZoom = FMath::Clamp(MinZoom, 0, 30);
  MaxZoom = FMath::Clamp(MaxZoom, 0, 30);
  if (MaxZoom < MinZoom) {
    MaxZoom = MinZoom;
  }

  if (OldMinZoom != MinZoom || OldMaxZoom != MaxZoom) {
    UE_LOG(LogTileHttpServer, Warning,
           TEXT("Sanitized zoom range: %d..%d (was %d..%d)"), MinZoom, MaxZoom,
           OldMinZoom, OldMaxZoom);
  }
}

bool ATileHttpServerActor::RenderTileToPng(int32 Zoom, int32 X, int32 Y,
                                           TArray<uint8>& OutPng) {
  if (!TileCamera || !TileCamera->CaptureComponent) {
    return false;
  }

  const int32 YForCalc = bTileYIsTMS ? ((1 << Zoom) - 1 - Y) : Y;

  FVector LocationCm = FVector::ZeroVector;
  double OrthoWidthCm = 0.0;
  UTileMercatorLibrary::ComputeTileCamera(
      Zoom, X, YForCalc, TileCamera->OriginLat, TileCamera->OriginLon,
      TileCamera->AltitudeMeters, TileCamera->bNorthIsX,
      TileCamera->bUseMercatorScale, LocationCm, OrthoWidthCm);

  TileCamera->SetActorLocation(LocationCm + TileCamera->LocalOffsetCm);
  TileCamera->CaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
  TileCamera->CaptureComponent->OrthoWidth = OrthoWidthCm;
  TileCamera->CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

  if (!TileCamera->CaptureComponent->TextureTarget) {
    UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(TileCamera);
    RenderTarget->InitCustomFormat(TileSize, TileSize, PF_B8G8R8A8, false);
    RenderTarget->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
    RenderTarget->UpdateResourceImmediate(true);
    TileCamera->CaptureComponent->TextureTarget = RenderTarget;
  } else {
    TileCamera->CaptureComponent->TextureTarget->ClearColor =
        FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
  }

  TileCamera->CaptureComponent->CaptureScene();

  UTextureRenderTarget2D* RenderTarget = TileCamera->CaptureComponent->TextureTarget;
  FTextureRenderTargetResource* RenderResource =
      RenderTarget->GameThread_GetRenderTargetResource();
  if (!RenderResource) {
    return false;
  }

  TArray<FColor> Pixels;
  if (!RenderResource->ReadPixels(Pixels)) {
    return false;
  }

  for (FColor& Pixel : Pixels) {
    Pixel.A = 255;
  }

  const int32 Width = RenderTarget->SizeX;
  const int32 Height = RenderTarget->SizeY;

  IImageWrapperModule& ImageWrapperModule =
      FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
  TSharedPtr<IImageWrapper> ImageWrapper =
      ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
  if (!ImageWrapper.IsValid()) {
    return false;
  }

  if (!ImageWrapper->SetRaw(Pixels.GetData(), Pixels.Num() * sizeof(FColor), Width,
                            Height, ERGBFormat::BGRA, 8)) {
    return false;
  }

  const auto Compressed = ImageWrapper->GetCompressed(100);
  OutPng.Reset();
  OutPng.Append(Compressed.GetData(), static_cast<int32>(Compressed.Num()));

  return OutPng.Num() > 0;
}

bool ATileHttpServerActor::LoadFileToArray(const FString& FilePath,
                                           TArray<uint8>& OutData) const {
  return FFileHelper::LoadFileToArray(OutData, *FilePath);
}

bool ATileHttpServerActor::SaveArrayToFile(const FString& FilePath,
                                           const TArray<uint8>& Data) const {
  IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
  const FString Dir = FPaths::GetPath(FilePath);
  if (!PlatformFile.DirectoryExists(*Dir)) {
    PlatformFile.CreateDirectoryTree(*Dir);
  }

  return FFileHelper::SaveArrayToFile(Data, *FilePath);
}

FString ATileHttpServerActor::GetAbsoluteTileRoot() const {
  if (FPaths::IsRelative(TileRootDir)) {
    return FPaths::Combine(FPaths::ProjectSavedDir(), TileRootDir);
  }
  return TileRootDir;
}

FString ATileHttpServerActor::BuildTilePath(int32 Zoom, int32 X, int32 Y) const {
  const FString Root = GetAbsoluteTileRoot();
  return FPaths::Combine(Root, FString::FromInt(Zoom), FString::FromInt(X),
                         FString::Printf(TEXT("%d.png"), Y));
}
