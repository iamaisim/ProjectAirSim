// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#ifndef CORE_SIM_SRC_SENSORS_SENSOR_IMPL_HPP_
#define CORE_SIM_SRC_SENSORS_SENSOR_IMPL_HPP_

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "component.hpp"
#include "constant.hpp"
#include "core_sim/error.hpp"
#include "core_sim/sensors/airspeed.hpp"
#include "core_sim/sensors/barometer.hpp"
#include "core_sim/sensors/battery.hpp"
#include "core_sim/sensors/camera.hpp"
#include "core_sim/sensors/distance_sensor.hpp"
#include "core_sim/sensors/gps.hpp"
#include "core_sim/sensors/imu.hpp"
#include "core_sim/sensors/lidar.hpp"
#include "core_sim/sensors/magnetometer.hpp"
#include "core_sim/sensors/radar.hpp"
#include "core_sim/sensors/sensor.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class SensorImpl : public ComponentWithTopicsAndServiceMethods {
 public:
  SensorImpl(SensorType type, const std::string& id, bool is_enabled,
             const std::string& parent_link, const std::string& component,
             const Logger& logger, const TopicManager& topic_manager,
             const std::string& parent_topic_path,
             const ServiceManager& service_manager,
             const StateManager& state_manager)
      : ComponentWithTopicsAndServiceMethods(component, logger, id,
                                             topic_manager, parent_topic_path,
                                             service_manager, state_manager),
        type_(type),
        is_enabled_(is_enabled),
        parent_link_(parent_link) {}

  const bool IsEnabled() const { return is_enabled_; }

  void SetEnabled(bool is_enabled) { is_enabled_ = is_enabled; }

  SensorType GetType() const { return type_; }

  const std::string& GetParentLink() const { return parent_link_; }

  // static helpers

  static void LoadSensors(const json& json,
                          std::vector<std::unique_ptr<Sensor>>& sensors,
                          const Logger& actor_logger,
                          const std::string& actor_name,
                          const std::string& actor_id,
                          const TopicManager& topic_manager,
                          const std::string& parent_topic_path,
                          const ServiceManager& service_manager,
                          const StateManager& state_manager) {
    actor_logger.LogVerbose(actor_name, "[%s] Loading 'sensors'.",
                            actor_id.c_str());

    auto sensors_json = JsonUtils::GetArray(json, Constant::Config::sensors);
    if (JsonUtils::IsEmptyArray(sensors_json)) {
      actor_logger.LogVerbose(actor_name, "[%s] 'sensors' missing or empty.",
                              actor_id.c_str());
    }

    try {
      std::transform(
          sensors_json.begin(), sensors_json.end(), std::back_inserter(sensors),
          [&actor_logger, &actor_name, &actor_id, &topic_manager,
           &parent_topic_path, &service_manager, &state_manager](auto& json) {
            return LoadSensor(json, actor_logger, actor_name, actor_id,
                              topic_manager, parent_topic_path, service_manager,
                              state_manager);
          });
    } catch (...) {
      sensors.clear();
      throw;
    }

    actor_logger.LogVerbose(actor_name, "[%s] 'sensors' loaded.",
                            actor_id.c_str());
  }

  static std::unique_ptr<Sensor> LoadSensor(
      const json& json, const Logger& actor_logger,
      const std::string& actor_name, const std::string& actor_id,
      const TopicManager& topic_manager, const std::string& parent_topic_path,
      const ServiceManager& service_manager,
      const StateManager& state_manager) {
    auto id = LoadSensorId(json, actor_logger, actor_name, actor_id);
    auto type = LoadSensorType(json, id, actor_logger, actor_name, actor_id);
    auto is_enabled =
        LoadSensorEnabled(json, id, actor_logger, actor_name, actor_id);
    auto parent_link =
        LoadSensorParentLink(json, id, actor_logger, actor_name, actor_id);

    actor_logger.LogVerbose(actor_name, "[%s][%s] Loading 'sensor'.",
                            actor_id.c_str(), id.c_str());

    if (type == Constant::Config::camera) {
      auto camera =
          new Camera(id, is_enabled, parent_link, actor_logger, topic_manager,
                     parent_topic_path, service_manager, state_manager);
      camera->Load(json);
      return std::unique_ptr<Sensor>(camera);
    } else if (type == Constant::Config::imu) {
      auto imu =
          new Imu(id, is_enabled, parent_link, actor_logger, topic_manager,
                  parent_topic_path, service_manager, state_manager);
      imu->Load(json);
      return std::unique_ptr<Sensor>(imu);
    } else if (type == Constant::Config::airspeed) {
      auto airspeed = new AirspeedSensor(
          id, is_enabled, parent_link, actor_logger, topic_manager,
          parent_topic_path, service_manager, state_manager);
      airspeed->Load(json);
      return std::unique_ptr<Sensor>(airspeed);
    } else if (type == Constant::Config::barometer) {
      auto barometer = new Barometer(id, is_enabled, parent_link, actor_logger,
                                     topic_manager, parent_topic_path,
                                     service_manager, state_manager);
      barometer->Load(json);
      return std::unique_ptr<Sensor>(barometer);
    } else if (type == Constant::Config::magnetometer) {
      auto magnetometer = new Magnetometer(
          id, is_enabled, parent_link, actor_logger, topic_manager,
          parent_topic_path, service_manager, state_manager);
      magnetometer->Load(json);
      return std::unique_ptr<Sensor>(magnetometer);
    } else if (type == Constant::Config::lidar) {
      auto lidar =
          new Lidar(id, is_enabled, parent_link, actor_logger, topic_manager,
                    parent_topic_path, service_manager, state_manager);
      lidar->Load(json);
      return std::unique_ptr<Sensor>(lidar);
    } else if (type == Constant::Config::distance_sensor) {
      auto distance_sensor = new DistanceSensor(
          id, is_enabled, parent_link, actor_logger, topic_manager,
          parent_topic_path, service_manager, state_manager);
      distance_sensor->Load(json);
      return std::unique_ptr<Sensor>(distance_sensor);
    } else if (type == Constant::Config::radar) {
      auto radar =
          new Radar(id, is_enabled, parent_link, actor_logger, topic_manager,
                    parent_topic_path, service_manager, state_manager);
      radar->Load(json);
      return std::unique_ptr<Sensor>(radar);
    } else if (type == Constant::Config::gps) {
      auto gps =
          new Gps(id, is_enabled, parent_link, actor_logger, topic_manager,
                  parent_topic_path, service_manager, state_manager);
      gps->Load(json);
      return std::unique_ptr<Sensor>(gps);
    } else if (type == Constant::Config::battery) {
      auto battery =
          new Battery(id, is_enabled, parent_link, actor_logger, topic_manager,
                      parent_topic_path, service_manager, state_manager);
      battery->Load(json);
      return std::unique_ptr<Sensor>(battery);
    } else {
      actor_logger.LogError(actor_name, "[%s] Invalid sensor type '%s'.",
                            id.c_str(), type.c_str());
      throw Error("Invalid sensor type.");
    }

    actor_logger.LogVerbose(actor_name, "[%s][%s] 'sensor' loaded.",
                            actor_id.c_str(), id.c_str());
  }

  static std::string LoadSensorId(const json& json, const Logger& actor_logger,
                                  const std::string& actor_name,
                                  const std::string& actor_id) {
    actor_logger.LogVerbose(actor_name, "[%s] Loading 'sensor.id'.",
                            actor_id.c_str());
    auto id = JsonUtils::GetIdentifier(json, Constant::Config::id);
    actor_logger.LogVerbose(actor_name, "[%s][%s] 'sensor.id' loaded.",
                            actor_id.c_str(), id.c_str());

    return id;
  }

  static std::string LoadSensorType(const json& json,
                                    const std::string& sensor_id,
                                    const Logger& actor_logger,
                                    const std::string& actor_name,
                                    const std::string& actor_id) {
    actor_logger.LogVerbose(actor_name, "[%s][%s] Loading 'sensor.type'.",
                            actor_id.c_str(), sensor_id.c_str());
    auto type = JsonUtils::GetIdentifier(json, Constant::Config::type);
    actor_logger.LogVerbose(actor_name, "[%s][%s] 'sensor.type' loaded.",
                            actor_id.c_str(), sensor_id.c_str());

    return type;
  }

  static bool LoadSensorEnabled(const json& json, const std::string& sensor_id,
                                const Logger& actor_logger,
                                const std::string& actor_name,
                                const std::string& actor_id) {
    actor_logger.LogVerbose(actor_name, "[%s][%s] Loading 'sensor.enabled'.",
                            actor_id.c_str(), sensor_id.c_str());
    auto is_enabled =
        JsonUtils::GetInteger(json, Constant::Config::enabled, 1 /*default*/);
    actor_logger.LogVerbose(actor_name, "[%s][%s] 'sensor.enabled' loaded.",
                            actor_id.c_str(), sensor_id.c_str());

    return is_enabled;
  }

  static std::string LoadSensorParentLink(const json& json,
                                          const std::string& sensor_id,
                                          const Logger& actor_logger,
                                          const std::string& actor_name,
                                          const std::string& actor_id) {
    actor_logger.LogVerbose(actor_name,
                            "[%s][%s] Loading 'sensor.parent_link'.",
                            actor_id.c_str(), sensor_id.c_str());

    std::string parent_link =
        JsonUtils::GetString(json, Constant::Config::parent_link, "");
    if (parent_link == "") {
      actor_logger.LogVerbose(actor_name,
                              "'parent_link' missing or empty. Using default.");
    }
    actor_logger.LogVerbose(actor_name, "[%s][%s] 'sensor.parent_link' loaded.",
                            actor_id.c_str(), sensor_id.c_str());

    return parent_link;
  }

 protected:
  SensorType type_;
  bool is_enabled_;
  std::string parent_link_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_SENSORS_SENSOR_IMPL_HPP_
