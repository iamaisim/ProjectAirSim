// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SENSORS_MAGNETOMETER_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SENSORS_MAGNETOMETER_HPP_

#include <string>

#include "core_sim/earth_utils.hpp"
#include "core_sim/message/magnetometer_message.hpp"
#include "core_sim/sensors/sensor.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class SensorImpl;
class TopicManager;
class StateManager;
class ServiceManager;

//! MAGNETOMETER parameters

struct MagnetometerParams {
  enum class ReferenceSource {
    kReferenceSourceConstant = 0,
    kReferenceSourceDipoleModel = 1
  };
  // 5 mgauss as per LSM303D spec-sheet
  // Table 3 (RMS is same as stddev)
  Vector3 noise_sigma = Vector3(0.005f, 0.005f, 0.005f);

  float scale_factor = 1.0f;
  // no offset as per specsheet (zero gauss level)
  Vector3 noise_bias = Vector3(0.0f, 0.0f, 0.0f);

  // use dipole model if there is enough compute power available
  bool dynamic_reference_source = true;
  ReferenceSource ref_source = ReferenceSource::kReferenceSourceDipoleModel;
  // bool dynamic_reference_source = false;
  // ReferenceSource ref_source = ReferenceSource::ReferenceSource_Constant;

  Transform origin_setting;
};

//! MAGNETOMETER sensor type

class Magnetometer : public Sensor {
 public:
  Magnetometer();

  const MagnetometerParams& GetMagnetometerSettings() const;

  void BeginUpdate();
  void Update(const TimeNano, const TimeNano) override;
  MagnetometerMessage getOutput() const;
  void EndUpdate();

 private:
  friend class Robot;
  friend class SensorImpl;

  Magnetometer(const std::string& id, bool is_enabled,
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

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SENSORS_MAGNETOMETER_HPP_
