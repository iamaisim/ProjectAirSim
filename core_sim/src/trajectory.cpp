// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include "core_sim/trajectory.hpp"

#include <memory>

#include "actor_impl.hpp"
#include "constant.hpp"
#include "core_sim/logger.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

class Trajectory::Loader {
 public:
  explicit Loader(Trajectory::Impl& impl);

  void Load(const json& json);

  void Load(const std::string& traj_name, const std::vector<float>& time,
            const std::vector<float>& x, const std::vector<float>& y,
            const std::vector<float>& z, const std::vector<float>& roll,
            const std::vector<float>& pitch, const std::vector<float>& yaw,
            const std::vector<float>& vel_x, const std::vector<float>& vel_y,
            const std::vector<float>& vel_z);

 private:
  void ClearParamArrays();
  void LoadName(const json& json);
  void LoadTimeSec(const json& json);
  void LoadPose(const json& json);
  void LoadAngularPose(const json& json);
  void LoadVelocity(const json& json);

  Trajectory::Impl& impl_;
};

class Trajectory::Impl : public Component {
 public:
  Impl(const Logger& logger);

  void Load(ConfigJson config_json);

  void Load(const std::string& traj_name, const std::vector<float>& time,
            const std::vector<float>& x, const std::vector<float>& y,
            const std::vector<float>& z, const std::vector<float>& roll,
            const std::vector<float>& pitch, const std::vector<float>& yaw,
            const std::vector<float>& vel_x, const std::vector<float>& vel_y,
            const std::vector<float>& vel_z);

  void KinematicsUpdate(TimeSec curr_time,
                        TrajectoryKinematics& traj_kinematics) const;

  const std::string& GetName() const;

 private:
  friend class Trajectory::Loader;

  Trajectory::Loader loader_;

  // Time Param
  std::vector<TimeSec> time_sec_;
  // Pose Params
  std::vector<float> pose_x_;
  std::vector<float> pose_y_;
  std::vector<float> pose_z_;
  // Angular Pose Params
  std::vector<float> pose_roll_;
  std::vector<float> pose_pitch_;
  std::vector<float> pose_yaw_;
  // Linerar Velocity Params
  std::vector<float> velocity_linear_x_;
  std::vector<float> velocity_linear_y_;
  std::vector<float> velocity_linear_z_;

  std::string traj_name_ = "";

  float Interpolate(const std::vector<float>& time_array,
                    const std::vector<float>& param_array, int upper_index,
                    int lower_index, float curr_time) const;
};

// class Trajectory
Trajectory::Trajectory() : pimpl_(std::shared_ptr<Trajectory::Impl>(nullptr)) {}

Trajectory::Trajectory(const Logger& logger)
    : pimpl_(std::shared_ptr<Trajectory::Impl>(new Trajectory::Impl(logger))) {}

void Trajectory::Load(ConfigJson config_json) {
  return static_cast<Trajectory::Impl*>(pimpl_.get())->Load(config_json);
}

void Trajectory::Load(
    const std::string& traj_name, const std::vector<float>& time,
    const std::vector<float>& x, const std::vector<float>& y,
    const std::vector<float>& z, const std::vector<float>& roll,
    const std::vector<float>& pitch, const std::vector<float>& yaw,
    const std::vector<float>& vel_x, const std::vector<float>& vel_y,
    const std::vector<float>& vel_z) {
  return static_cast<Trajectory::Impl*>(pimpl_.get())
      ->Load(traj_name, time, x, y, z, roll, pitch, yaw, vel_x, vel_y, vel_z);
}

void Trajectory::KinematicsUpdate(TimeSec curr_time,
                                  TrajectoryKinematics& traj_kinematics) const {
  static_cast<Trajectory::Impl*>(pimpl_.get())
      ->KinematicsUpdate(curr_time, traj_kinematics);
}

// not currently called/used
bool Trajectory::IsLoaded() const {
  return static_cast<Trajectory::Impl*>(pimpl_.get())->IsLoaded();
}

const std::string& Trajectory::GetName() const {
  return static_cast<Trajectory::Impl*>(pimpl_.get())->GetName();
}

// -----------------------------------------------------------------------------
// class Trajectory::Impl

Trajectory::Impl::Impl(const Logger& logger)
    : Component(Constant::Component::trajectory, logger), loader_(*this) {}

void Trajectory::Impl::Load(ConfigJson config_json) {
  json json = config_json;
  loader_.Load(json);
}

void Trajectory::Impl::Load(
    const std::string& traj_name, const std::vector<float>& time,
    const std::vector<float>& x, const std::vector<float>& y,
    const std::vector<float>& z, const std::vector<float>& roll,
    const std::vector<float>& pitch, const std::vector<float>& yaw,
    const std::vector<float>& vel_x, const std::vector<float>& vel_y,
    const std::vector<float>& vel_z) {
  loader_.Load(traj_name, time, x, y, z, roll, pitch, yaw, vel_x, vel_y, vel_z);
}

const std::string& Trajectory::Impl::GetName() const { return traj_name_; }

void Trajectory::Impl::KinematicsUpdate(TimeSec curr_time,
                                        TrajectoryKinematics& traj_kinematics) const {
  if (time_sec_.empty()) return;

  int lower;
  int upper;

  // incorporate time offset
  curr_time -= traj_kinematics.traj_params.time_offset;

  // If looping is enabled, resets the curr_time to restart the motion
  if (traj_kinematics.traj_params.to_loop) {
    if (traj_kinematics.traj_params.num_loops == 0)
      curr_time -= traj_kinematics.traj_params.num_loops * time_sec_.back();
    else {
      curr_time -= traj_kinematics.traj_params.num_loops *
                   (time_sec_.back() - time_sec_.front());
    }
  }

  // Env_actor will stay at origin until the first time entry.
  // After the last time entry, the env_actor will either stay at the same
  // position or loop back depending on the config params.
  // In between, kinematics is calculated by interpolating between the two
  // relevant values.
  if (curr_time >= time_sec_.front() && curr_time <= time_sec_.back()) {
    auto lower_index =
        std::lower_bound(time_sec_.begin(), time_sec_.end(), curr_time);
    lower = lower_index - time_sec_.begin() - 1;
    upper = lower + 1;

    float x = Interpolate(time_sec_, pose_x_, upper, lower, curr_time) +
              traj_kinematics.traj_params.x_offset;
    float y = Interpolate(time_sec_, pose_y_, upper, lower, curr_time) +
              traj_kinematics.traj_params.y_offset;
    float z = Interpolate(time_sec_, pose_z_, upper, lower, curr_time) +
              traj_kinematics.traj_params.z_offset;
    float roll = Interpolate(time_sec_, pose_roll_, upper, lower, curr_time) +
                 traj_kinematics.traj_params.roll_offset;
    float pitch = Interpolate(time_sec_, pose_pitch_, upper, lower, curr_time) +
                  traj_kinematics.traj_params.pitch_offset;
    float yaw = Interpolate(time_sec_, pose_yaw_, upper, lower, curr_time) +
                traj_kinematics.traj_params.yaw_offset;
    float x_vel =
        Interpolate(time_sec_, velocity_linear_x_, upper, lower, curr_time);
    float y_vel =
        Interpolate(time_sec_, velocity_linear_y_, upper, lower, curr_time);
    float z_vel =
        Interpolate(time_sec_, velocity_linear_z_, upper, lower, curr_time);

    // Updates the kinematics of the object
    traj_kinematics.kinematics.pose.position = Vector3(x, y, z);
    traj_kinematics.kinematics.pose.orientation =
        TransformUtils::ToQuaternion(roll, pitch, yaw);
    traj_kinematics.kinematics.twist.linear = Vector3(x_vel, y_vel, z_vel);
  }

  if (curr_time > time_sec_.back() && traj_kinematics.traj_params.to_loop) {
    // Increment loop counter
    traj_kinematics.traj_params.num_loops += 1;
  }
}

float Trajectory::Impl::Interpolate(const std::vector<float>& time_array,
                                    const std::vector<float>& param_array,
                                    int upper_index, int lower_index,
                                    float curr_time) const {
  auto param_lower = param_array[lower_index];
  auto param_upper = param_array[upper_index];
  auto time_lower = time_array[lower_index];
  auto time_upper = time_array[upper_index];

  if (time_lower == time_upper) {  // edge case where time is either lower than
                                   // t[0] or higher than t[-1]
    return param_lower;
  }

  // Simple linear interpolation between the two given values
  float param_val =
      param_lower + (curr_time - time_lower) * ((param_upper - param_lower) /
                                                (time_upper - time_lower));

  return param_val;
}

// -----------------------------------------------------------------------------
// class Trajectory::Loader

Trajectory::Loader::Loader(Trajectory::Impl& impl) : impl_(impl) {}

void Trajectory::Loader::Load(const json& json) {
  ClearParamArrays();

  LoadName(json);
  LoadTimeSec(json);
  LoadPose(json);
  LoadAngularPose(json);
  LoadVelocity(json);

  impl_.is_loaded_ = true;  // not currently used
}

void Trajectory::Loader::Load(
    const std::string& traj_name, const std::vector<float>& time,
    const std::vector<float>& x, const std::vector<float>& y,
    const std::vector<float>& z, const std::vector<float>& roll,
    const std::vector<float>& pitch, const std::vector<float>& yaw,
    const std::vector<float>& vel_x, const std::vector<float>& vel_y,
    const std::vector<float>& vel_z) {
  ClearParamArrays();

  impl_.traj_name_ = traj_name;
  impl_.time_sec_ = time;
  impl_.pose_x_ = x;
  impl_.pose_y_ = y;
  impl_.pose_z_ = z;
  impl_.pose_roll_ = roll;
  impl_.pose_pitch_ = pitch;
  impl_.pose_yaw_ = yaw;
  impl_.velocity_linear_x_ = vel_x;
  impl_.velocity_linear_y_ = vel_y;
  impl_.velocity_linear_z_ = vel_z;
  impl_.is_loaded_ = true;
}

void Trajectory::Loader::LoadName(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading trajectory name.");
  auto name = JsonUtils::GetIdentifier(json, Constant::Config::name);
  impl_.traj_name_ = name;
  impl_.logger_.LogVerbose(impl_.name_, "'%s' trajectory loaded.",
                           name.c_str());
}

void Trajectory::Loader::LoadTimeSec(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'time (sec)'.");

  auto time = JsonUtils::GetArray(json, Constant::Config::time_sec);

  std::copy(time.begin(), time.end(), std::back_inserter(impl_.time_sec_));

  impl_.logger_.LogVerbose(impl_.name_, "'time (sec)' loaded.");
}

void Trajectory::Loader::LoadPose(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'pose'.");

  auto x = JsonUtils::GetArray(json, Constant::Config::pose_x);
  auto y = JsonUtils::GetArray(json, Constant::Config::pose_y);
  auto z = JsonUtils::GetArray(json, Constant::Config::pose_z);

  // Convert to vector<int> type
  std::copy(x.begin(), x.end(), std::back_inserter(impl_.pose_x_));
  std::copy(y.begin(), y.end(), std::back_inserter(impl_.pose_y_));
  std::copy(z.begin(), z.end(), std::back_inserter(impl_.pose_z_));

  impl_.logger_.LogVerbose(impl_.name_, "'pose' loaded.");
}

void Trajectory::Loader::LoadAngularPose(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'angular pose'.");

  auto roll = JsonUtils::GetArray(json, Constant::Config::pose_roll);
  auto pitch = JsonUtils::GetArray(json, Constant::Config::pose_pitch);
  auto yaw = JsonUtils::GetArray(json, Constant::Config::pose_yaw);

  // Convert to vector<int> type
  std::copy(roll.begin(), roll.end(), std::back_inserter(impl_.pose_roll_));
  std::copy(pitch.begin(), pitch.end(), std::back_inserter(impl_.pose_pitch_));
  std::copy(yaw.begin(), yaw.end(), std::back_inserter(impl_.pose_yaw_));

  impl_.logger_.LogVerbose(impl_.name_, "'angular pose' loaded.");
}

void Trajectory::Loader::LoadVelocity(const json& json) {
  impl_.logger_.LogVerbose(impl_.name_, "Loading 'linear velocity'.");

  auto vx = JsonUtils::GetArray(json, Constant::Config::velocity_linear_x);
  auto vy = JsonUtils::GetArray(json, Constant::Config::velocity_linear_y);
  auto vz = JsonUtils::GetArray(json, Constant::Config::velocity_linear_z);

  // Convert to vector<int> type
  std::copy(vx.begin(), vx.end(), std::back_inserter(impl_.velocity_linear_x_));
  std::copy(vy.begin(), vy.end(), std::back_inserter(impl_.velocity_linear_y_));
  std::copy(vz.begin(), vz.end(), std::back_inserter(impl_.velocity_linear_z_));

  impl_.logger_.LogVerbose(impl_.name_, "'linear velocity' loaded.");
}

void Trajectory::Loader::ClearParamArrays() {
  impl_.traj_name_.clear();
  impl_.time_sec_.clear();
  impl_.pose_x_.clear();
  impl_.pose_y_.clear();
  impl_.pose_z_.clear();
  impl_.pose_roll_.clear();
  impl_.pose_pitch_.clear();
  impl_.pose_yaw_.clear();
  impl_.velocity_linear_x_.clear();
  impl_.velocity_linear_y_.clear();
  impl_.velocity_linear_z_.clear();
  impl_.is_loaded_ = false;
}

}  // namespace projectairsim
}  // namespace microsoft
