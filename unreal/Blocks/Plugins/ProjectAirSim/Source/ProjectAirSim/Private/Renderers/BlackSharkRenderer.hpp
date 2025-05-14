// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#pragma once
#include <core_sim/earth_utils.hpp>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnrealHelpers.h"

class BlackSharkRenderer {
 private:
  UPROPERTY() AActor* blackshark_actor_;
  microsoft::projectairsim::HomeGeoPoint home_geo_pt_;

  const FName LatitudeName = TEXT("WorldOriginLatitude");
  const FName LongitudeName = TEXT("WorldOriginLongitude");
  const FName AltitudeName = TEXT("WorldOriginAltitude");
  const FName ApplySettingsName = TEXT("ApplySettings");

 public:
  BlackSharkRenderer(microsoft::projectairsim::HomeGeoPoint home_geo_pt)
      : home_geo_pt_(home_geo_pt) {}

  void Initialize(UWorld* world, bool isGisScene);

  bool SetLocation(double latitude, double longitude, float altitude);
};
