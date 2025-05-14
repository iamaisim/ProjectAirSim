// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SENSORS_BAROMETER_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SENSORS_BAROMETER_HPP_

#include <string>

#include "core_sim/earth_utils.hpp"
#include "core_sim/message/barometer_message.hpp"
#include "core_sim/sensors/sensor.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class SensorImpl;
class TopicManager;
class ServiceManager;
class StateManager;

//! BAROMETER parameters

struct BarometerParams {
  // Atmospheric pressure at zero-altitude/sea-level in
  // HectoPascals(hPa)/Millibars(mbar). Depends on geo-location so, better be
  // user-specified
  float qnh = EarthUtils::kSeaLevelPressure / 100.0f;

  // Ref: Mariner's Pressure Atlas, David Burch, 2014
  // https://www.starpath.com/ebooksamples/9780914025382_sample.pdf
  // sea level min,avh,max = 950,1013,1050 ie approx 3.65% variation
  // regular pressure changes in quiet conditions are taken as 1/20th of this
  // GM process may generate ~70% of sigma in tau interval
  // This means below config may produce ~10m variance per hour

  float pressure_factor_sigma = 0.0365f / 20;
  float pressure_factor_tau = 3600;

  // Experiments for MEAS MS56112 sensor shows 0.021mbar, datasheet has
  // resoultion of 0.027mbar @ 1024
  // http://www.te.com/commerce/DocumentDelivery/DDEController?Action=srchrtrv&DocNm=MS5611-01BA03&DocType=Data+Sheet&DocLang=English
  float uncorrelated_noise_sigma = 0.027f * 100;
  // jMavSim uses below
  // real_T unnorrelated_noise_sigma = 0.1f;

  // see PX4 param reference for EKF:
  // https://dev.px4.io/en/advanced/parameter_reference.html
  float update_latency = 0.0f;  // sec
  float update_frequency = 50;  // Hz
  float startup_delay = 0;      // sec
};

//! BAROMETER sensor type

class Barometer : public Sensor {
 public:
  Barometer();

  const BarometerParams& GetBarometerSettings() const;

  void BeginUpdate();
  void Update(const TimeNano, const TimeNano) override;
  BarometerMessage getOutput() const;
  void EndUpdate();

 private:
  friend class Robot;
  friend class SensorImpl;

  Barometer(const std::string& id, bool is_enabled,
            const std::string& parent_link, const Logger& logger,
            const TopicManager& topic_manager,
            const std::string& parent_topic_path,
            const ServiceManager& service_manager,
            const StateManager& state_manager);

  void Load(ConfigJson config_json) override;

  void Initialize(const Kinematics&, const Environment&) override;

  class Impl;
  class Loader;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SENSORS_BAROMETER_HPP_
