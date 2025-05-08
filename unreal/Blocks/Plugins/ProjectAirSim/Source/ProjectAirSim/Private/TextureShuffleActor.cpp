#include "TextureShuffleActor.h"

#include "Blueprint/AsyncTaskDownloadImage.h"

void ATextureShuffleActor::SwapTexture_Implementation(int tex_id,
                                                      int component_id,
                                                      int material_id) {
  if (SwappableTextures.Num() < 1) return;

  if (!InitMaterialCache(component_id)) return;

  tex_id %= SwappableTextures.Num();
  component_id %= NumComponents;
  material_id %= DynamicMaterialInstances.Num();

  InitCachedMaterial(component_id, material_id);

  DynamicMaterialInstances[material_id]->SetTextureParameterValue(
      "TextureParameter", SwappableTextures[tex_id]);
}

void ATextureShuffleActor::SetTextureFromUrl(const FString& url,
                                             int material_id) {
  if (!InitMaterialCache(0)) return;

  DownloadImageTask =
      NewObject<UAsyncTaskDownloadImage>();  // UE Garbage collector will clean
                                             // up the previous one.

  InitCachedMaterial(0, material_id);
  DownloadingMaterialID = material_id;

  DownloadImageTask->OnSuccess.RemoveAll(this);
  DownloadImageTask->OnSuccess.AddDynamic(
      this, &ATextureShuffleActor::OnTextureDownloaded);
  DownloadImageTask->Start(url);
}

void ATextureShuffleActor::OnTextureDownloaded(UTexture2DDynamic* downloaded) {
  if (DownloadingMaterialID < 0 ||
      DownloadingMaterialID > DynamicMaterialInstances.Num() - 1)
    return;

  DynamicMaterialInstances[DownloadingMaterialID]->SetTextureParameterValue(
      "TextureParameter", (UTexture*)downloaded);
}

bool ATextureShuffleActor::InitMaterialCache(int component_id) {
  if (!MaterialCacheInitialized) {
    TArray<UStaticMeshComponent*> components;
    GetComponents<UStaticMeshComponent>(components);
    NumComponents = components.Num();
    DynamicMaterialInstances.Init(nullptr,
                                  components[component_id]->GetNumMaterials());
    MaterialCacheInitialized = true;
  }

  return NumComponents > 0 && DynamicMaterialInstances.Num() > 0;
}

void ATextureShuffleActor::InitCachedMaterial(int component_id,
                                              int material_id) {
  if (DynamicMaterialInstances[material_id] == nullptr) {
    DynamicMaterialInstances[material_id] =
        UMaterialInstanceDynamic::Create(DynamicMaterial, this);
    TArray<UStaticMeshComponent*> components;
    GetComponents<UStaticMeshComponent>(components);
    components[component_id]->SetMaterial(
        material_id, DynamicMaterialInstances[material_id]);
  }
}