// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "core_sim/sensors/battery.hpp"

#include <map>
#include <memory>
#include <string>

#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "json.hpp"
#include "sensor_impl.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class Battery::Loader {
 public:
  explicit Loader(Battery::Impl& impl);

  void Load(const json& json);

 private:
  void LoadBatterySettings(const json& json);

  Battery::Impl& impl;
};

enum BatteryStatus {
  BATTERY_CHARGE_STATE_OK,   // Normal state
  BATTERY_CHARGE_STATE_LOW,  // Need to determine what charge percent triggers
                             // this (perhaps customer customizable?)
  BATTERY_CHARGE_STATE_CRITICAL,  // Need to determine what charge percent
                                  // triggers this
  BATTERY_CHARGE_STATE_UNHEALTHY
};

std::string enum_to_string(BatteryStatus type) {
  switch (type) {
    case BATTERY_CHARGE_STATE_OK:
      return "BATTERY_CHARGE_STATE_OK";
    case BATTERY_CHARGE_STATE_LOW:
      return "BATTERY_CHARGE_STATE_LOW";
    case BATTERY_CHARGE_STATE_CRITICAL:
      return "BATTERY_CHARGE_STATE_CRITICAL";
    case BATTERY_CHARGE_STATE_UNHEALTHY:
      return "BATTERY_CHARGE_STATE_UNHEALTHY";
    default:
      return "INVALID_STATE";
  }
}

struct BatteryData {
  TimeNano time_stamp;
  float battery_remaining;
  float battery_pct_remaining;
  uint32_t estimated_time_remaining;
  BatteryStatus battery_charge_state;
};

class Battery::Impl : public SensorImpl {
 public:
  Impl(const std::string& id, bool is_enabled, const std::string& parent_link,
       const Logger& logger, const TopicManager& topic_manager,
       const std::string& parent_topic_path,
       const ServiceManager& service_manager,
       const StateManager& state_manager);

  void Load(ConfigJson config_json);

  void Initialize(const Kinematics&, const Environment&);

  void OnBeginUpdate() override;

  void CreateTopics();

  void Update(const TimeNano, const TimeNano);

  void Update(const TimeNano, const TimeNano, float power);

  void RegisterServiceMethod();

  const BatterySettings& GetBatterySettings() const;

  void OnEndUpdate() override;

  BatteryStateMessage GetState();

  bool SetBatteryRemaining(float desired_battery_level);

  bool SetBatteryDrainRate(float desired_drain_rate);

  float GetBatteryDrainRate();

  bool SetBatteryHealthStatus(bool battery_health_indicator);

 private:
  friend class Battery::Loader;

  Battery::Loader loader;

  Topic battery_topic;
  std::vector<Topic> topics;
  BatterySettings battery_settings_;
  BatteryData state_;
};

Battery::Battery() : Sensor(std::shared_ptr<SensorImpl>(nullptr)) {}

Battery::Battery(const std::string& id, bool is_enabled,
                 const std::string& parent_link, const Logger& logger,
                 const TopicManager& topic_manager,
                 const std::string& parent_topic_path,
                 const ServiceManager& service_manager,
                 const StateManager& state_manager)
    : Sensor(std::shared_ptr<SensorImpl>(new Battery::Impl(
          id, is_enabled, parent_link, logger, topic_manager, parent_topic_path,
          service_manager, state_manager))) {}

void Battery::Load(ConfigJson config_json) {
  return static_cast<Battery::Impl*>(pimpl.get())->Load(config_json);
}

void Battery::Initialize(const Kinematics& kinematics,
                         const Environment& environment) {
  static_cast<Battery::Impl*>(pimpl.get())->Initialize(kinematics, environment);
}

void Battery::BeginUpdate() {
  static_cast<Battery::Impl*>(pimpl.get())->BeginUpdate();
}

void Battery::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos) {
  static_cast<Battery::Impl*>(pimpl.get())->Update(sim_time, sim_dt_nanos);
}

void Battery::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos,
                     float power) {
  static_cast<Battery::Impl*>(pimpl.get())
      ->Update(sim_time, sim_dt_nanos, power);
}

void Battery::EndUpdate() {
  static_cast<Battery::Impl*>(pimpl.get())->EndUpdate();
}

BatteryStateMessage Battery::GetState() const {
  return static_cast<Battery::Impl*>(pimpl.get())->GetState();
}

bool Battery::SetBatteryRemaining(float desired_battery_remaining) {
  return static_cast<Battery::Impl*>(pimpl.get())
      ->SetBatteryRemaining(desired_battery_remaining);
}

bool Battery::SetBatteryDrainRate(float desired_drain_rate) {
  return static_cast<Battery::Impl*>(pimpl.get())
      ->SetBatteryDrainRate(desired_drain_rate);
}

float Battery::GetBatteryDrainRate() {
  return static_cast<Battery::Impl*>(pimpl.get())->GetBatteryDrainRate();
}

bool Battery::SetBatteryHealthStatus(bool battery_health_indicator) {
  return static_cast<Battery::Impl*>(pimpl.get())
      ->SetBatteryHealthStatus(battery_health_indicator);
}

Battery::Impl::Impl(const std::string& id, bool is_enabled,
                    const std::string& parent_link, const Logger& logger,
                    const TopicManager& topic_manager,
                    const std::string& parent_topic_path,
                    const ServiceManager& service_manager,
                    const StateManager& state_manager)
    : SensorImpl(SensorType::kBattery, id, is_enabled, parent_link,
                 Constant::Component::battery, logger, topic_manager,
                 parent_topic_path, service_manager, state_manager),
      loader(*this) {
  SetTopicPath();
  CreateTopics();
}

void Battery::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader.Load(json);
}

void Battery::Impl::Initialize(const Kinematics& kinematics,
                               const Environment& environment) {
  // TODO Decide on model and add appropriate parameters
  return;
}

void Battery::Impl::CreateTopics() {
  battery_topic = Topic("battery", topic_path_, TopicType::kPublished, 100,
                        MessageType::kBattery);
  topics.push_back(battery_topic);
}

const BatterySettings& Battery::Impl::GetBatterySettings() const {
  return battery_settings_;
}

void Battery::Impl::OnBeginUpdate() {
  topic_manager_.RegisterTopic(battery_topic);
  RegisterServiceMethod();
}

void Battery::Impl::Update(const TimeNano sim_time,
                           const TimeNano sim_dt_nanos) {
  std::lock_guard<std::mutex> lock(update_lock_);

  // TimeNano current_time = SimClock::Get()->NowNanos();
  auto current_time = sim_time;
  //! Calculate time since last update
  //! Using sim_dt_nanos supplied by the Scene as it is the fastest ticking
  //! components allowing downscaling for custom frequencies if needed
  TimeSec dt = sim_dt_nanos / 1.0e9;

  state_.battery_remaining =
      std::max(0.0f, state_.battery_remaining -
                         (dt * battery_settings_.battery_drain_rate));
  state_.battery_pct_remaining =
      100 * state_.battery_remaining / battery_settings_.battery_capacity;
  state_.time_stamp = current_time;
  state_.estimated_time_remaining =
      state_.battery_remaining / battery_settings_.battery_drain_rate;
  if (state_.battery_charge_state != BATTERY_CHARGE_STATE_UNHEALTHY) {
    if (state_.battery_pct_remaining < 20.0f) {
      state_.battery_charge_state = BatteryStatus::BATTERY_CHARGE_STATE_LOW;
    } else if (state_.battery_pct_remaining < 5.0f) {
      state_.battery_charge_state =
          BatteryStatus::BATTERY_CHARGE_STATE_CRITICAL;
    } else {
      state_.battery_charge_state = BatteryStatus::BATTERY_CHARGE_STATE_OK;
    }
  }
  //! Prepare battery_msg to publish
  BatteryStateMessage battery_msg(state_.time_stamp,
                                  state_.battery_pct_remaining,
                                  state_.estimated_time_remaining,
                                  enum_to_string(state_.battery_charge_state));

  //! Publish battery output msg (should we make this less frequent?)
  topic_manager_.PublishTopic(battery_topic, (Message&)battery_msg);
}

void Battery::Impl::Update(const TimeNano sim_time, const TimeNano sim_dt_nanos,
                           float power) {
  if (battery_settings_.mode == BatteryMode::kEnergyConsumption) {
    // Might need to add a constant to account for sensors/other equipment on
    // board
    battery_settings_.battery_drain_rate = power * battery_settings_.power_coefficient;
  }
  Update(sim_time, sim_dt_nanos);
}

BatteryStateMessage Battery::Impl::GetState() {
  return BatteryStateMessage(state_.time_stamp, state_.battery_pct_remaining,
                             state_.estimated_time_remaining,
                             enum_to_string(state_.battery_charge_state));
}

bool Battery::Impl::SetBatteryRemaining(float desired_battery_remaining) {
  if (desired_battery_remaining >= 0.0f &&
      desired_battery_remaining <= 100.0f) {
    std::lock_guard<std::mutex> lock(update_lock_);
    state_.battery_pct_remaining = desired_battery_remaining;
    state_.battery_remaining =
        desired_battery_remaining / 100 * battery_settings_.battery_capacity;
    return true;
  }
  return false;
}

bool Battery::Impl::SetBatteryDrainRate(float desired_drain_rate) {
  // Cannot manually set battery drain rate in EnergyConsumption model
  if (desired_drain_rate >= 0.0f &&
      battery_settings_.mode != BatteryMode::kEnergyConsumption) {
    std::lock_guard<std::mutex> lock(update_lock_);
    battery_settings_.battery_drain_rate = desired_drain_rate;
    return true;
  }
  return false;
}

float Battery::Impl::GetBatteryDrainRate() {
  return battery_settings_.battery_drain_rate;
}

bool Battery::Impl::SetBatteryHealthStatus(bool battery_health_indicator) {
  std::lock_guard<std::mutex> lock(update_lock_);
  if (!battery_health_indicator) {
    state_.battery_charge_state = BatteryStatus::BATTERY_CHARGE_STATE_UNHEALTHY;
  } else {
    state_.battery_charge_state = BatteryStatus::BATTERY_CHARGE_STATE_OK;
  }
  return true;
}

void Battery::Impl::RegisterServiceMethod() {
  //! Note: The service method endpoint is same as the topic name
  auto get_battery_data_name = topic_path_ + "/" + "GetBatteryStatus";
  auto get_battery_data = ServiceMethod(get_battery_data_name, {""});
  auto get_battery_data_handler =
      get_battery_data.CreateMethodHandler(&Battery::Impl::GetState, *this);
  service_manager_.RegisterMethod(get_battery_data, get_battery_data_handler);

  auto set_battery_remaining_name = topic_path_ + "/" + "SetBatteryRemaining";
  auto set_battery_remaining =
      ServiceMethod(set_battery_remaining_name, {"desired_battery_remaining"});
  auto set_battery_remaining_handler =
      set_battery_remaining.CreateMethodHandler(
          &Battery::Impl::SetBatteryRemaining, *this);
  service_manager_.RegisterMethod(set_battery_remaining,
                                  set_battery_remaining_handler);

  auto get_battery_drain_rate_name = topic_path_ + "/" + "GetBatteryDrainRate";
  auto get_battery_drain_rate =
      ServiceMethod(get_battery_drain_rate_name, {""});
  auto get_battery_drain_rate_handler =
      get_battery_drain_rate.CreateMethodHandler(
          &Battery::Impl::GetBatteryDrainRate, *this);
  service_manager_.RegisterMethod(get_battery_drain_rate,
                                  get_battery_drain_rate_handler);

  auto set_battery_drain_rate_name = topic_path_ + "/" + "SetBatteryDrainRate";
  auto set_battery_drain_rate =
      ServiceMethod(set_battery_drain_rate_name, {"desired_drain_rate"});
  auto set_battery_drain_rate_handler =
      set_battery_drain_rate.CreateMethodHandler(
          &Battery::Impl::SetBatteryDrainRate, *this);
  service_manager_.RegisterMethod(set_battery_drain_rate,
                                  set_battery_drain_rate_handler);

  auto set_battery_health_status_name =
      topic_path_ + "/" + "SetBatteryHealthStatus";
  auto set_battery_health_status = ServiceMethod(set_battery_health_status_name,
                                                 {"battery_health_indicator"});
  auto set_battery_health_status_handler =
      set_battery_health_status.CreateMethodHandler(
          &Battery::Impl::SetBatteryHealthStatus, *this);
  service_manager_.RegisterMethod(set_battery_health_status,
                                  set_battery_health_status_handler);
}

void Battery::Impl::OnEndUpdate() {
  topic_manager_.UnregisterTopic(battery_topic);
}

Battery::Loader::Loader(Battery::Impl& impl) : impl(impl) {}

void Battery::Loader::Load(const json& json) {
  impl.logger_.LogVerbose(impl.name_, "[%s] Loading 'battery_settings'.",
                          impl.id_.c_str());

  LoadBatterySettings(json);
  impl.is_loaded_ = true;
  impl.logger_.LogVerbose(impl.name_, "[%s] 'battery_settings' loaded.",
                          impl.id_.c_str());
}

void Battery::Loader::LoadBatterySettings(const json& json) {
  // Figure out battery default for battery drain rate? (right now its 0.02 ~
  // 1-2 hour capacity)
  auto mode = JsonUtils::GetString(json, Constant::Config::battery_mode);
  if (mode == Constant::Config::battery_simple_discharge_mode) {
    impl.battery_settings_.mode = BatteryMode::kSimpleDischarge;
  } else if (mode == Constant::Config::battery_rotor_power_discharge) {
    impl.battery_settings_.mode = BatteryMode::kEnergyConsumption;
  } else {
    impl.logger_.LogWarning(impl.name_, "Invalid battery mode specified");
  }
  impl.battery_settings_.battery_capacity =
      JsonUtils::GetNumber<float>(json, Constant::Config::total_battery_capacity, 100.0f);
  impl.state_.battery_remaining =
      JsonUtils::GetNumber<float>(json, Constant::Config::battery_capacity_at_start, 100.0f);
  impl.state_.battery_pct_remaining =
      impl.state_.battery_remaining / impl.battery_settings_.battery_capacity;
  if (impl.battery_settings_.mode == BatteryMode::kSimpleDischarge) {
    impl.battery_settings_.battery_drain_rate = JsonUtils::GetNumber<float>(
        json, Constant::Config::battery_drain_rate_at_start, 0.02f);
  }
  else if (impl.battery_settings_.mode == BatteryMode::kEnergyConsumption) {
    impl.battery_settings_.power_coefficient = JsonUtils::GetNumber<float>(
        json, Constant::Config::rotor_power_coefficient, 1.00f);
  }
  impl.logger_.LogVerbose(impl.name_, "Loaded Battery");
}

}  // namespace projectairsim
}  // namespace microsoft
