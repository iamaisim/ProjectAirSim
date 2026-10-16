#include "Tiles/TileMercatorLibrary.h"

namespace {
constexpr double kEarthRadiusMeters = 6378137.0;
constexpr double kPi = 3.14159265358979323846;
constexpr double kDegToRad = kPi / 180.0;
constexpr double kRadToDeg = 180.0 / kPi;
constexpr double kMetersToCm = 100.0;

double ClampLatitude(double LatDeg) {
  constexpr double MaxLat = 85.05112878;
  return FMath::Clamp(LatDeg, -MaxLat, MaxLat);
}

void LatLonToMercator(double LatDeg, double LonDeg, double& OutX,
                      double& OutY) {
  const double Lat = ClampLatitude(LatDeg) * kDegToRad;
  const double Lon = LonDeg * kDegToRad;
  OutX = kEarthRadiusMeters * Lon;
  OutY = kEarthRadiusMeters * FMath::Loge(FMath::Tan(kPi / 4.0 + Lat / 2.0));
}

void MercatorToLatLon(double X, double Y, double& OutLatDeg,
                      double& OutLonDeg) {
  const double Lon = X / kEarthRadiusMeters;
  const double Lat = 2.0 * FMath::Atan(FMath::Exp(Y / kEarthRadiusMeters)) -
                     kPi / 2.0;
  OutLatDeg = Lat * kRadToDeg;
  OutLonDeg = Lon * kRadToDeg;
}

void LatLonToLocalEnuMeters(double LatDeg, double LonDeg, double OriginLat,
                            double OriginLon, double& OutEast,
                            double& OutNorth) {
  const double LatRad = LatDeg * kDegToRad;
  const double OriginLatRad = OriginLat * kDegToRad;
  const double OriginLonRad = OriginLon * kDegToRad;
  const double LonRad = LonDeg * kDegToRad;

  const double dLat = LatRad - OriginLatRad;
  const double dLon = LonRad - OriginLonRad;

  OutNorth = dLat * kEarthRadiusMeters;
  OutEast = dLon * kEarthRadiusMeters * FMath::Cos(OriginLatRad);
}

double MetersPerPixel(double LatDeg, int32 Zoom) {
  const double LatRad = LatDeg * kDegToRad;
  const double N = FMath::Pow(2.0, static_cast<double>(Zoom));
  return (2.0 * kPi * kEarthRadiusMeters * FMath::Cos(LatRad)) / (256.0 * N);
}
}  // namespace

void UTileMercatorLibrary::ComputeTileCamera(int32 Zoom,
                                             int32 X,
                                             int32 Y,
                                             double OriginLat,
                                             double OriginLon,
                                             double AltitudeMeters,
                                             bool bNorthIsX,
                                             bool bUseMercatorScale,
                                             FVector& OutLocationCm,
                                             double& OutOrthoWidthCm) {
  if (Zoom < 0) {
    OutLocationCm = FVector::ZeroVector;
    OutOrthoWidthCm = 0.0;
    return;
  }

  const double N = FMath::Pow(2.0, static_cast<double>(Zoom));
  const double WorldMeters = 2.0 * kPi * kEarthRadiusMeters;

  double OriginX = 0.0;
  double OriginY = 0.0;
  LatLonToMercator(OriginLat, OriginLon, OriginX, OriginY);

  const double TileMetersMercator = WorldMeters / N;
  const double MinX = -WorldMeters / 2.0 + static_cast<double>(X) * TileMetersMercator;
  const double MaxY = WorldMeters / 2.0 - static_cast<double>(Y) * TileMetersMercator;
  const double CenterX = MinX + TileMetersMercator / 2.0;
  const double CenterY = MaxY - TileMetersMercator / 2.0;

  double LocalX = 0.0;
  double LocalY = 0.0;
  double OrthoWidthMeters = 0.0;

  if (bUseMercatorScale) {
    LocalX = (CenterX - OriginX) * kMetersToCm;
    LocalY = (CenterY - OriginY) * kMetersToCm;
    OrthoWidthMeters = TileMetersMercator;
  } else {
    double TileLat = 0.0;
    double TileLon = 0.0;
    MercatorToLatLon(CenterX, CenterY, TileLat, TileLon);

    double EastMeters = 0.0;
    double NorthMeters = 0.0;
    LatLonToLocalEnuMeters(TileLat, TileLon, OriginLat, OriginLon, EastMeters,
                           NorthMeters);

    LocalX = EastMeters * kMetersToCm;
    LocalY = NorthMeters * kMetersToCm;
    OrthoWidthMeters = MetersPerPixel(TileLat, Zoom) * 256.0;
  }

  const double LocalZ = AltitudeMeters * kMetersToCm;
  if (bNorthIsX) {
    OutLocationCm = FVector(LocalY, LocalX, LocalZ);
  } else {
    OutLocationCm = FVector(LocalX, LocalY, LocalZ);
  }

  OutOrthoWidthCm = OrthoWidthMeters * kMetersToCm;
}
