// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "BlackSharkRenderer.hpp"

namespace projectairsim = microsoft::projectairsim;

void BlackSharkRenderer::Initialize(UWorld* world, bool isGisScene) {
  blackshark_actor_ = nullptr;
  blackshark_actor_ =
      UnrealHelpers::FindActor<AActor>(world, "BlackSharkGlobeActor");

  if (blackshark_actor_ == nullptr) {
    UnrealLogger::Log(
        projectairsim::LogLevel::kError,
        TEXT("No BlackSharkGlobeActor objects found. Blackshark settings would "
             "ignored/not applied"));
    return;
  } else {
    this->SetLocation(home_geo_pt_.geo_point.latitude,
                      home_geo_pt_.geo_point.longitude,
                      home_geo_pt_.geo_point.altitude);
  }
}

bool BlackSharkRenderer::SetLocation(double latitude, double longitude,
                                     float altitude) {
  bool status = false;

  status = UnrealHelpers::SetDoublePropertyValue(blackshark_actor_,
                                                 LatitudeName, latitude);
  status = status && UnrealHelpers::SetDoublePropertyValue(
                         blackshark_actor_, LongitudeName, longitude);
  status = status && UnrealHelpers::SetDoublePropertyValue(
                         blackshark_actor_, AltitudeName, altitude);
  status = status &&
           UnrealHelpers::CallFunction(blackshark_actor_, ApplySettingsName);
  return status;
}
