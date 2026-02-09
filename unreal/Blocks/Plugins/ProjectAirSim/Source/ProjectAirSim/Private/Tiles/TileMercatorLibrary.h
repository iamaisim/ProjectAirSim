#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TileMercatorLibrary.generated.h"

UCLASS()
class PROJECTAIRSIM_API UTileMercatorLibrary : public UBlueprintFunctionLibrary {
  GENERATED_BODY()

 public:
  // Computes camera location (cm) and ortho width (cm) for an OSM XYZ tile.
  UFUNCTION(BlueprintCallable, Category = "Tiles")
  static void ComputeTileCamera(int32 Zoom,
                                int32 X,
                                int32 Y,
                                double OriginLat,
                                double OriginLon,
                                double AltitudeMeters,
                                bool bNorthIsX,
                                bool bUseMercatorScale,
                                FVector& OutLocationCm,
                                double& OutOrthoWidthCm);
};
