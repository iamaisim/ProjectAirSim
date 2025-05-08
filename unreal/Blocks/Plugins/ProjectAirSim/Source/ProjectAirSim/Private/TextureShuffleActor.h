#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/Material.h"
#include "TextureShuffleActor.generated.h"

class UAsyncTaskDownloadImage;

UCLASS()
class PROJECTAIRSIM_API ATextureShuffleActor : public AStaticMeshActor {
  GENERATED_BODY()

 protected:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextureShuffle)
  UMaterialInterface* DynamicMaterial = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TextureShuffle)
  TArray<UTexture2D*> SwappableTextures;

 public:
  UFUNCTION(BlueprintNativeEvent)
  void SwapTexture(int tex_id = 0, int component_id = 0, int material_id = 0);

  UFUNCTION(BlueprintCallable)
  void SetTextureFromUrl(const FString& url, int material_id = 0);

 private:
  UAsyncTaskDownloadImage* DownloadImageTask = nullptr;

  bool MaterialCacheInitialized = false;
  int NumComponents = -1;
  int DownloadingMaterialID = -1;

  bool InitMaterialCache(int component_id);
  void InitCachedMaterial(int component_id, int material_id);

  UFUNCTION()
  void OnTextureDownloaded(UTexture2DDynamic* downloaded);

  UPROPERTY()
  TArray<UMaterialInstanceDynamic*> DynamicMaterialInstances;
};