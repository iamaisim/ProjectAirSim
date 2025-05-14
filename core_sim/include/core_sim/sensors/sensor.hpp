// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SENSORS_SENSOR_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SENSORS_SENSOR_HPP_

#include <memory>
#include <string>
#include <vector>

#include "core_sim/environment.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/transforms/transform.hpp"

namespace microsoft {
namespace projectairsim {

class SensorImpl;
class ConfigJson;
class logger;

enum class SensorType {
  kCamera = 0,
  kBarometer = 1,
  kImu = 2,
  kGps = 3,
  kMagnetometer = 4,
  kDistanceSensor = 5,
  kLidar = 6,
  kRadar = 7,
  kAirspeed = 8,
  kBattery = 9,
};

// abstract base class
class Sensor {
 public:
  virtual ~Sensor() {}

  bool IsLoaded() const;

  void BeginUpdate();

  void EndUpdate();

  const std::string& GetId() const;
  SensorType GetType() const;
  bool IsEnabled() const;
  void SetEnabled(bool is_enabled);
  bool IsUpdating() const;
  const std::string& GetParentLink() const;

  virtual void Initialize(const Kinematics&, const Environment&) = 0;

  virtual void Update(const TimeNano, const TimeNano) = 0;

 protected:
  explicit Sensor(const std::shared_ptr<SensorImpl>& pimpl);

  virtual void Load(ConfigJson config_json) = 0;

  std::shared_ptr<SensorImpl> pimpl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SENSORS_SENSOR_HPP_
