// Copyright (C) Microsoft Corporation.  All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SENSORS_AIRSPEED_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SENSORS_AIRSPEED_HPP_

#include <string>

#include "core_sim/earth_utils.hpp"
#include "core_sim/message/airspeed_message.hpp"
#include "core_sim/sensors/sensor.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class SensorImpl;
class TopicManager;
class ServiceManager;
class StateManager;

//! AIRSPEED parameters

struct AirspeedSensorParams {
  // Ref: Mariner's Pressure Atlas, David Burch, 2014
  // https://www.starpath.com/ebooksamples/9780914025382_sample.pdf
  // sea level min,avh,max = 950,1013,1050 ie approx 3.65% variation
  // regular pressure changes in quiet conditions are taken as 1/20th of this
  // GM process may generate ~70% of sigma in tau interval
  // This means below config may produce ~10m variance per hour

  float pressure_factor_sigma = 0.0365f / 20;
  float pressure_factor_tau = 3600;

  // MEAS MS4525DO-DS5AI001D is a differential pressure sensor.  The datasheet
  // gives a [-1, 1] psi range at output counts [0x0666, 0x399a], giving a
  // resolution of 0.0001526 psi or 1.052 Pa
  // https://www.te.com/commerce/DocumentDelivery/DDEController?Action=srchrtrv&DocNm=MS4525DO&DocType=DS&DocLang=English
  float uncorrelated_noise_sigma = 1.052f;

  Vector3 forward_xyz = {1.0f, 0.0f, 0.0f};
};

//! AIRSPEED sensor type

class AirspeedSensor : public Sensor {
 public:
  AirspeedSensor();

  const AirspeedSensorParams& GetAirspeedSettings() const;

  void BeginUpdate();
  void Update(const TimeNano, const TimeNano) override;
  AirspeedMessage getOutput() const;
  void EndUpdate();

 private:
  friend class Robot;
  friend class SensorImpl;

  AirspeedSensor(const std::string& id, bool is_enabled,
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

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SENSORS_AIRSPEED_HPP_
