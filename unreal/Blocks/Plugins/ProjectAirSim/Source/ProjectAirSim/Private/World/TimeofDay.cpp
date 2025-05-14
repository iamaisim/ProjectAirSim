// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "TimeofDay.hpp"

namespace projectairsim = microsoft::projectairsim;

void TimeOfDay::initialize(UWorld* world, bool isGisScene) {
  sun_sky_ = nullptr;
  sun_ = nullptr;
  sun_sky_ = UnrealHelpers::FindActor<AActor>(world, "SunSky");
  if (!sun_sky_) {
    UnrealLogger::Log(projectairsim::LogLevel::kError,
                      TEXT("No SunSky objects found. TimeOfDay settings would "
                           "ignored/not applied"));
    return;
  }

  auto components = sun_sky_->GetComponents();
  auto sun_obj = components.Array()[0];
  auto skyAtmosphereObj = components.Array()[2];
  for (auto comp : components) {
    if (comp->GetName() == "DirectionalLight") {
      sun_obj = comp;
    } else if (comp->GetName() == "SkyAtmosphere") {
      skyAtmosphereObj = comp;
    }
  }

  sun_ = Cast<UDirectionalLightComponent>(sun_obj);
  skyatmosphere_ = Cast<USkyAtmosphereComponent>(skyAtmosphereObj);

  // We identify our scene by looking for default of 0.0, 0.0
  // Otherwise, we might be in customer scene and we don't want to modify
  // their defaults.
  this->StoreSunSkyDefaults();
  if (isGisScene) {
    this->setSunLightIntensity(default_sun_intensity);
    this->SetLocation(home_geo_point.geo_point.latitude,
                      home_geo_point.geo_point.longitude);
  }
}

// Perhaps, a lot of these methods should be part of a separate SunSky class but
// interacting with blueprint objects is weird and it might get more confusing
// if we move away from Sunsky in the future.
void TimeOfDay::ResetSunSkyToDefault() {
  if (sun_sky_) {
    this->SetDoublePropertyValue(LatitudeName, sunskydefaults.latitude);
    this->SetDoublePropertyValue(LongitudeName, sunskydefaults.longitude);
    this->SetDoublePropertyValue(TimeZone, sunskydefaults.timezone);

    this->SetDoublePropertyValue(SolarTime, sunskydefaults.solartime);
    this->SetIntPropertyValue(Day, sunskydefaults.day);
    this->SetIntPropertyValue(Month, sunskydefaults.month);

    this->setSunLightIntensity(sunskydefaults.intensity);
  }
}

void TimeOfDay::StoreSunSkyDefaults() {
  sunskydefaults.latitude = this->GetDoublePropertyValue(LatitudeName);
  sunskydefaults.longitude = this->GetDoublePropertyValue(LongitudeName);
  sunskydefaults.timezone = this->GetDoublePropertyValue(TimeZone);

  sunskydefaults.solartime = this->GetDoublePropertyValue(SolarTime, 12.0);
  sunskydefaults.day = this->GetIntPropertyValue(Day, 1);
  sunskydefaults.month = this->GetIntPropertyValue(Month, 1);

  sunskydefaults.intensity = this->getSunLightIntensity();
}

bool TimeOfDay::SetLocation(float latitude, float longitude) {
  bool status = this->SetDoublePropertyValue(LatitudeName, latitude);
  auto approx_timezone = longitude / 15.0;
  status = status & this->SetDoublePropertyValue(LongitudeName, longitude);
  status = status & this->SetDoublePropertyValue(TimeZone, approx_timezone);
  return status;
}

bool TimeOfDay::SetDateAndTime(tm time) {
  bool status = false;
  UnrealHelpers::RunCommandOnGameThread(
      [this, &status, &time]() {
        status = this->SetIntPropertyValue(Month, time.tm_mon + 1);
        status = status & this->SetIntPropertyValue(Day, time.tm_mday);
        auto solar_time = this->GetSolarTime(time);
        status = status & this->SetDoublePropertyValue(SolarTime, solar_time);
        sun_->MarkRenderStateDirty();
      },
      true /*wait*/);
  return status;
}

float TimeOfDay::GetSolarTime(tm time) {
  float solar_time = static_cast<float>(time.tm_hour) +
                     static_cast<float>(time.tm_min) / 60.0 +
                     static_cast<float>(time.tm_sec) / 3600.0;
  return solar_time;
}

bool TimeOfDay::SetDoublePropertyValue(FName propertyName, float value) {
  FDoubleProperty* DoubleProp =
      FindFProperty<FDoubleProperty>(sun_sky_->GetClass(), propertyName);
  if (DoubleProp != NULL) {
    DoubleProp->SetPropertyValue_InContainer(sun_sky_, value);
  } else {
    return false;
  }
  return true;
}

float TimeOfDay::GetDoublePropertyValue(FName propertyName, float default_val) {
  FDoubleProperty* DoubleProp =
      FindFProperty<FDoubleProperty>(sun_sky_->GetClass(), propertyName);
  if (DoubleProp != NULL) {
    auto value = DoubleProp->GetPropertyValue_InContainer(sun_sky_);
    return value;
  }
  return default_val;
}

bool TimeOfDay::SetIntPropertyValue(FName propertyName, int value) {
  FIntProperty* IntProp =
      FindFProperty<FIntProperty>(sun_sky_->GetClass(), propertyName);
  if (IntProp != NULL) {
    IntProp->SetPropertyValue_InContainer(sun_sky_, value);
  } else {
    return false;
  }
  return true;
}

int TimeOfDay::GetIntPropertyValue(FName propertyName, int default_val) {
  FIntProperty* IntProp =
      FindFProperty<FIntProperty>(sun_sky_->GetClass(), propertyName);
  if (IntProp != NULL) {
    auto value = IntProp->GetPropertyValue_InContainer(sun_sky_);
    return value;
  }
  return default_val;
}

bool TimeOfDay::set(bool is_enabled, const std::string& start_datetime,
                    bool is_start_datetime_dst, float celestial_clock_speed,
                    float update_interval_secs, bool move_sun) {
  if (is_enabled) {
    tod_enabled_ = true;
    if (!sun_) {
      UnrealHelpers::OnScreenLogMessage(
          TEXT("SunSky was not found. "),
          TEXT("TimeOfDay settings would be ignored."), LogDebugLevel::Failure);
    } else {
      // this is a bit odd but given how advanceTimeOfDay() works currently,
      // tod_sim_clock_start_ needs to be reset here.
      tod_sim_clock_start_ = projectairsim::SimClock::Get()->NowSimNanos();

      tod_last_update_ = 0;
      if (start_datetime != "") {
        auto time = this->to_tm(start_datetime, is_start_datetime_dst);
        this->SetDateAndTime(time);
        tod_current_time_ = time;
      } else {
        tod_current_time_ = std::tm();
      }

      updateSunRotation();
    }
  } else if (tod_enabled_) {
    // Going from enabled to disabled
    if (sun_) {
      updateSunRotation();
    }
  }

  // do these in the end to ensure that advanceTimeOfDay() doesn't see
  // any inconsistent state.
  tod_enabled_ = is_enabled;
  tod_celestial_clock_speed_ = celestial_clock_speed;
  tod_update_interval_secs_ = update_interval_secs;
  tod_move_sun_ = move_sun;

  return tod_enabled_ == is_enabled;
}

void TimeOfDay::advance() {
  if (tod_enabled_ && sun_sky_ && sun_ && tod_move_sun_) {
    auto now_nsecs = projectairsim::SimClock::Get()->NowSimNanos();
    auto secs = (now_nsecs - tod_last_update_) / 1E9;
    if (secs > tod_update_interval_secs_) {
      tod_last_update_ = projectairsim::SimClock::Get()->NowSimNanos();

      auto interval = ((tod_last_update_ - tod_sim_clock_start_) / 1E9) *
                      tod_celestial_clock_speed_;
      tod_current_time_.tm_sec += (int)interval;
      mktime(&tod_current_time_);
      this->SetDateAndTime(tod_current_time_);
      updateSunRotation();
    }
  }
}

bool TimeOfDay::updateSunRotation() {
  bool status = false;
  if (sun_ && sun_sky_) {
    UnrealHelpers::RunCommandOnGameThread(
        [this, &status]() {
          status = true;

          FOutputDeviceNull ar;
          sun_sky_->CallFunctionByNameWithArguments(TEXT("UpdateSun"), ar, NULL,
                                                    true);
        },
        true /*wait*/);
  }
  return status;
}

bool TimeOfDay::setSunPositionFromDateTime(const std::string& datetime,
                                           const std::string& format,
                                           bool is_dst) {
  if (datetime != "") {
    auto time = this->to_tm(datetime, is_dst, format);
    this->SetDateAndTime(time);
    return updateSunRotation();
  }
  return false;
}

bool TimeOfDay::setSunLightIntensity(float intensity) {
  if (sun_) {
    UnrealHelpers::RunCommandOnGameThread(
        [this, &intensity]() { sun_->SetIntensity(intensity); }, true /*wait*/);
    return true;
  }
  return false;
}

bool TimeOfDay::setCloudShadowStrength(float strength) {
  if (sun_) {
    UnrealHelpers::RunCommandOnGameThread(
        [this, &strength]() {
          sun_->bCastCloudShadows = true;
          sun_->CloudShadowStrength = strength;
          sun_->MarkRenderStateDirty();
        },
        true /*wait*/);
    return true;
  }
  return false;
}

float TimeOfDay::getCloudShadowStrength() {
  if (sun_) {
    return sun_->CloudShadowStrength;
  }
  return 0.0f;
}

float TimeOfDay::getSunLightIntensity() {
  if (sun_) {
    return sun_->Intensity;
  }
  // What is the appropriate return value here?
  // Although we should really never hit this.
  return 0.0f;
}

std::string TimeOfDay::toString(time_t tt, const char* format) {
  char str[1024];

  struct tm timeinfo;
#ifdef _WIN32
  localtime_s(&timeinfo, &tt);
#else
  localtime_r(&tt, &timeinfo);
#endif

  if (std::strftime(str, sizeof(str), format, &timeinfo))
    return std::string(str);
  else
    return std::string();
}

std::time_t TimeOfDay::to_time_t(const std::string& str, bool is_dst,
                                 const std::string& format) {
  std::tm t;
  t.tm_isdst = is_dst ? 1 : 0;
  std::istringstream ss(str);
  ss >> std::get_time(&t, format.c_str());
  return mktime(&t);
}

std::tm TimeOfDay::to_tm(const std::string& str, bool is_dst,
                         const std::string& format) {
  std::tm t;
  t.tm_isdst = is_dst ? 1 : 0;
  std::istringstream ss(str);
  ss >> std::get_time(&t, format.c_str());
  return t;
}

std::string TimeOfDay::getTimeofDay() {
  char buffer[80];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S",
                &tod_current_time_);
  return std::string(buffer);
}