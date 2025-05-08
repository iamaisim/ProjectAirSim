// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once

#include <Runtime/Engine/Classes/Components/DirectionalLightComponent.h>
#include <Runtime/Engine/Classes/Components/SkyAtmosphereComponent.h>

#include "Components/InputComponent.h"
#include "CoreMinimal.h"
#include "Engine/DirectionalLight.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerInput.h"
#include "Misc/OutputDeviceNull.h"
#include "UObject/ConstructorHelpers.h"
#include "UnrealHelpers.h"
#include "core_sim/earth_celestial.hpp"
#include "core_sim/earth_utils.hpp"

constexpr float default_sun_intensity = 2.75f;

struct TimeOfDaySetting {
  bool enabled = false;
  std::string start_datetime =
      "2020-01-01 12:00:00";  // format: %Y-%m-%d %H:%M:%S
  bool is_start_datetime_dst = true;
  float celestial_clock_speed = 1;
  float update_interval_secs = 1;
  bool move_sun = false;
};

struct SunSkyDefaults {
  float latitude;
  float longitude;
  float timezone;
  float solartime;
  float intensity;
  int day;
  int month;
};

class TimeOfDay {
 private:
  UPROPERTY() AActor* sun_sky_;
  UPROPERTY() UDirectionalLightComponent* sun_;
  UPROPERTY() USkyAtmosphereComponent* skyatmosphere_;

  const FName LatitudeName = TEXT("Latitude");
  const FName LongitudeName = TEXT("Longitude");
  const FName TimeZone = TEXT("TimeZone");
  const FName Day = TEXT("Day");
  const FName Month = TEXT("Month");
  const FName SolarTime = TEXT("SolarTime");

  SunSkyDefaults sunskydefaults;
  TimeNano tod_sim_clock_start_;  // sim start in local time
  TimeNano tod_last_update_;
  std::tm tod_current_time_;  // tod, configurable

  float tod_celestial_clock_speed_;
  float tod_update_interval_secs_;

  std::string toString(time_t tt, const char* format = "%Y-%m-%d %H-%M-%S");
  std::time_t to_time_t(const std::string& str, bool is_dst = false,
                        const std::string& format = "%Y-%m-%d %H:%M:%S");

  std::tm to_tm(const std::string& str, bool is_dst = false,
                const std::string& format = "%Y-%m-%d %H:%M:%S");

  microsoft::projectairsim::HomeGeoPoint home_geo_point;

 public:
  TimeOfDay(microsoft::projectairsim::HomeGeoPoint home_geo_pt)
      : home_geo_point(home_geo_pt) {}

  bool SetDoublePropertyValue(FName propertyName, float value);

  float GetDoublePropertyValue(FName propertyName, float default_val = 0.0);

  bool SetIntPropertyValue(FName propertyName, int value);

  int GetIntPropertyValue(FName propertyName, int default_val = 0.0);

  bool set(bool is_enabled, const std::string& start_datetime,
           bool is_start_datetime_dst, float celestial_clock_speed,
           float update_interval_secs, bool move_sun);

  void initialize(UWorld* world, bool isGisScene);

  void ResetSunSkyToDefault();

  void StoreSunSkyDefaults();

  bool SetLocation(float latitude, float longitude);

  bool SetDateAndTime(tm time);

  float GetSolarTime(tm time);

  void advance();

  bool updateSunRotation();

  bool setSunPositionFromDateTime(
      const std::string& datetime,
      const std::string& format = "%Y-%m-%d %H:%M:%S", bool is_dst = false);

  bool setSunLightIntensity(float intensity);

  bool setCloudShadowStrength(float strength);

  float getCloudShadowStrength();

  float getSunLightIntensity();

  std::string getTimeofDay();

  bool tod_move_sun_;
  bool tod_enabled_;
};