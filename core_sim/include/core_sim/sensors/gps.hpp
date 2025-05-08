// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SENSORS_GPS_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SENSORS_GPS_HPP_

#include <string>

#include "core_sim/earth_utils.hpp"
#include "core_sim/message/gps_message.hpp"
#include "core_sim/sensors/sensor.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class SensorImpl;
class TopicManager;
class ServiceManager;
class StateManager;

//! GPS parameters
struct GpsParams {
  float eph_time_constant = 0.9f, epv_time_constant = 0.9f;
  float eph_initial = 100.0f,
        epv_initial = 100.0f;  // initially fully diluted precision
  float eph_final = 0.1f,
        epv_final = 0.1f;  // PX4 won't converge GPS sensor fusion until we get
                           // to 10% accuracty.
  float eph_min_3d = 3.0f, eph_min_2d = 4.0f;
};

//! GPS sensor

class Gps : public Sensor {
 public:
  Gps();

  const GpsParams& GetGpsSettings() const;

  void BeginUpdate();
  void Update(const TimeNano, const TimeNano) override;
  GpsMessage getOutput() const;
  void EndUpdate();

 private:
  friend class Robot;
  friend class SensorImpl;

  Gps(const std::string& id, bool is_enabled, const std::string& parent_link,
      const Logger& logger, const TopicManager& topic_manager,
      const std::string& parent_topic_path,
      const ServiceManager& service_manager, const StateManager& state_manager);

  void Load(ConfigJson config_json) override;

  void Initialize(const Kinematics&, const Environment&) override;

  class Impl;
  class Loader;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SENSORS_GPS_HPP_
