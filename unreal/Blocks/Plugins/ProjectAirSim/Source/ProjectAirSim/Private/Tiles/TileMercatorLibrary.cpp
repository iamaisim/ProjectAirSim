#include "Tiles/TileMercatorLibrary.h"

namespace {
// Same sphere radius as core_sim EarthUtils::kEarthRadius, which ProjectAirSim
// uses to convert NED positions to GPS coordinates.
constexpr double kEarthRadiusMeters = 6378137.0;
constexpr double kPi = 3.14159265358979323846;
constexpr double kDegToRad = kPi / 180.0;
constexpr double kMetersToCm = 100.0;
constexpr int32 kMaxZoom = 30;

double ClampLatitude(double LatDeg) {
  constexpr double MaxLat = 85.05112878;
  return FMath::Clamp(LatDeg, -MaxLat, MaxLat);
}

double TileXToLonDeg(double X, double N) { return X / N * 360.0 - 180.0; }

double TileYToLatDeg(double Y, double N) {
  const double MercN = kPi * (1.0 - 2.0 * Y / N);
  return FMath::Atan(0.5 * (FMath::Exp(MercN) - FMath::Exp(-MercN))) / kDegToRad;
}

void LatLonToMercator(double LatDeg, double LonDeg, double& OutX,
                      double& OutY) {
  const double Lat = ClampLatitude(LatDeg) * kDegToRad;
  const double Lon = LonDeg * kDegToRad;
  OutX = kEarthRadiusMeters * Lon;
  OutY = kEarthRadiusMeters * FMath::Loge(FMath::Tan(kPi / 4.0 + Lat / 2.0));
}

// Inverse of EarthUtils::NedToGeodetic (spherical azimuthal equidistant
// projection around the origin), so a tile corner lands exactly where the
// simulator reports that lat/lon.
void LatLonToLocalNorthEast(double LatDeg, double LonDeg, double OriginLatDeg,
                            double OriginLonDeg, double& OutNorth,
                            double& OutEast) {
  const double Lat = LatDeg * kDegToRad;
  const double Lat0 = OriginLatDeg * kDegToRad;
  const double DLon = (LonDeg - OriginLonDeg) * kDegToRad;

  const double SinHalfDLat = FMath::Sin(0.5 * (Lat - Lat0));
  const double SinHalfDLon = FMath::Sin(0.5 * DLon);
  const double Hav = FMath::Clamp(
      SinHalfDLat * SinHalfDLat +
          FMath::Cos(Lat0) * FMath::Cos(Lat) * SinHalfDLon * SinHalfDLon,
      0.0, 1.0);
  const double C = 2.0 * FMath::Asin(FMath::Sqrt(Hav));
  const double K = C < 1e-12 ? 1.0 : C / FMath::Sin(C);

  OutNorth = kEarthRadiusMeters * K *
             (FMath::Cos(Lat0) * FMath::Sin(Lat) -
              FMath::Sin(Lat0) * FMath::Cos(Lat) * FMath::Cos(DLon));
  OutEast = kEarthRadiusMeters * K * FMath::Cos(Lat) * FMath::Sin(DLon);
}
}  // namespace

bool UTileMercatorLibrary::IsValidTile(int32 Zoom, int32 X, int32 Y) {
  if (Zoom < 0 || Zoom > kMaxZoom) {
    return false;
  }
  const int64 N = int64{1} << Zoom;
  return X >= 0 && Y >= 0 && X < N && Y < N;
}

FRotator UTileMercatorLibrary::GetTileCameraRotation(bool bNorthIsX) {
  // Pitch -90 looks straight down with the image top along the camera's
  // pre-pitch forward axis, so yaw selects which world axis is "north".
  return bNorthIsX ? FRotator(-90.0, 0.0, 0.0) : FRotator(-90.0, -90.0, 0.0);
}

bool UTileMercatorLibrary::ComputeTileCamera(int32 Zoom,
                                             int32 X,
                                             int32 Y,
                                             double OriginLat,
                                             double OriginLon,
                                             double AltitudeMeters,
                                             bool bNorthIsX,
                                             bool bUseMercatorScale,
                                             FVector& OutLocationCm,
                                             double& OutOrthoWidthCm) {
  OutLocationCm = FVector::ZeroVector;
  OutOrthoWidthCm = 0.0;
  if (!IsValidTile(Zoom, X, Y)) {
    return false;
  }

  const double N = static_cast<double>(int64{1} << Zoom);
  const double West = TileXToLonDeg(X, N);
  const double East = TileXToLonDeg(X + 1.0, N);
  const double North = TileYToLatDeg(Y, N);
  const double South = TileYToLatDeg(Y + 1.0, N);

  double CenterNorth = 0.0;
  double CenterEast = 0.0;
  double WidthMeters = 0.0;

  if (bUseMercatorScale) {
    double OriginX = 0.0;
    double OriginY = 0.0;
    LatLonToMercator(OriginLat, OriginLon, OriginX, OriginY);

    const double TileMeters = 2.0 * kPi * kEarthRadiusMeters / N;
    const double MinX = -kPi * kEarthRadiusMeters + X * TileMeters;
    const double MaxY = kPi * kEarthRadiusMeters - Y * TileMeters;
    CenterEast = MinX + 0.5 * TileMeters - OriginX;
    CenterNorth = MaxY - 0.5 * TileMeters - OriginY;
    WidthMeters = TileMeters;
  } else {
    double NwN, NwE, NeN, NeE, SwN, SwE, SeN, SeE;
    LatLonToLocalNorthEast(North, West, OriginLat, OriginLon, NwN, NwE);
    LatLonToLocalNorthEast(North, East, OriginLat, OriginLon, NeN, NeE);
    LatLonToLocalNorthEast(South, West, OriginLat, OriginLon, SwN, SwE);
    LatLonToLocalNorthEast(South, East, OriginLat, OriginLon, SeN, SeE);

    CenterNorth = 0.25 * (NwN + NeN + SwN + SeN);
    CenterEast = 0.25 * (NwE + NeE + SwE + SeE);
    WidthMeters = 0.5 * ((NeE - NwE) + (SeE - SwE));
  }

  const double LocalZ = AltitudeMeters * kMetersToCm;
  if (bNorthIsX) {
    OutLocationCm = FVector(CenterNorth * kMetersToCm,
                            CenterEast * kMetersToCm, LocalZ);
  } else {
    OutLocationCm = FVector(CenterEast * kMetersToCm,
                            -CenterNorth * kMetersToCm, LocalZ);
  }

  OutOrthoWidthCm = WidthMeters * kMetersToCm;
  return true;
}
