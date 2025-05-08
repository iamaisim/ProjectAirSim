#ifndef CORE_SIM_INCLUDE_CORE_SIM_SENSORS_BATTERY_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SENSORS_BATTERY_HPP_

#include "core_sim/message/battery_message.hpp"
#include "core_sim/sensors/sensor.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class SensorImpl;
class TopicManager;
class ServiceManager;
class StateManager;

enum class BatteryMode : int {
  kSimpleDischarge = 0,
  kEnergyConsumption = 1,
};

struct BatterySettings {
  BatteryMode mode = BatteryMode::kEnergyConsumption;
  float battery_capacity = 150000.0f;
  float battery_drain_rate = 0.0f;  // Rate in % per second in float
  float power_coefficient = 1.0f;
};

class Battery : public Sensor {
 public:
  Battery();

  void BeginUpdate();

  void Update(const TimeNano, const TimeNano) override;

  void Update(const TimeNano, const TimeNano, float power);

  void EndUpdate();

  BatteryStateMessage GetState() const;

  bool SetBatteryRemaining(float desired_battery_level);

  bool SetBatteryHealthStatus(bool battery_health_indicator);

  bool SetBatteryDrainRate(float desired_drain_rate);

  float GetBatteryDrainRate();

 private:
  friend class Robot;
  friend class SensorImpl;

  Battery(const std::string& id, bool is_enabled,
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

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SENSORS_BATTERY_HPP_
