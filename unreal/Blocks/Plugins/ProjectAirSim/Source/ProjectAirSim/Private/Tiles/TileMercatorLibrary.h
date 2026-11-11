#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TileMercatorLibrary.generated.h"

UCLASS()
class PROJECTAIRSIM_API UTileMercatorLibrary : public UBlueprintFunctionLibrary {
  GENERATED_BODY()

 public:
  // Computes the camera location (cm) and ortho width (cm) that frame the
  // OSM/XYZ tile (Zoom, X, Y), where Y counts down from the north edge.
  //
  // bNorthIsX = true uses the ProjectAirSim/NEU convention (X=North, Y=East).
  // bNorthIsX = false uses the Cesium-style convention (X=East, Y=South).
  //
  // bUseMercatorScale = false places tiles with the same local projection that
  // ProjectAirSim uses for GPS (EarthUtils::NedToGeodetic), so tiles line up
  // with the simulated lat/lon. bUseMercatorScale = true treats raw Web
  // Mercator meters as Unreal meters, which only fits levels built in Web
  // Mercator units.
  //
  // Returns false when the zoom level or tile index is out of range.
  UFUNCTION(BlueprintCallable, Category = "Tiles")
  static bool ComputeTileCamera(int32 Zoom,
                                int32 X,
                                int32 Y,
                                double OriginLat,
                                double OriginLon,
                                double AltitudeMeters,
                                bool bNorthIsX,
                                bool bUseMercatorScale,
                                FVector& OutLocationCm,
                                double& OutOrthoWidthCm);

  // World rotation for a straight-down camera whose image top is north.
  UFUNCTION(BlueprintPure, Category = "Tiles")
  static FRotator GetTileCameraRotation(bool bNorthIsX);

  UFUNCTION(BlueprintPure, Category = "Tiles")
  static bool IsValidTile(int32 Zoom, int32 X, int32 Y);
};
