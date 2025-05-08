// Copyright (C) Microsoft Corporation. All rights reserved.

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
                        TrajectoryKinematics& traj_kinematics,
                        const TimeSec time_at_load,
                        const Kinematics& kinematics_at_load) const;

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

  float InterpolateAngle(const std::vector<float>& time_array,
                         const std::vector<float>& angle_array, int upper_index,
                         int lower_index, float curr_time) const;
  
  float InterpolateAngle(const float time_lower, const float time_upper, const float param_lower,
                          const float param_upper, const float curr_time) const;

  float Interpolate(const std::vector<float>& time_array,
                    const std::vector<float>& param_array, int upper_index,
                    int lower_index, float curr_time) const;
  
  float Interpolate(const float time_lower, const float time_upper, const float param_lower,
                    const float param_upper, const float curr_time) const;
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
                                  TrajectoryKinematics& traj_kinematics,
                                  const TimeSec time_at_load,
                                  const Kinematics& kinematics_at_load) const {
  static_cast<Trajectory::Impl*>(pimpl_.get())
      ->KinematicsUpdate(curr_time, traj_kinematics, time_at_load,
                         kinematics_at_load);
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

void Trajectory::Impl::KinematicsUpdate(
    TimeSec curr_time, TrajectoryKinematics& traj_kinematics, const TimeSec time_at_load,
    const Kinematics& kinematics_at_load) const {
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

  // Env_actor will move towards the first trajectory point before the first time entry.
  // After the last time entry, the env_actor will either stay at the same
  // position or loop back depending on the config params.
  // In between, kinematics is calculated by interpolating between the two
  // relevant values.
  if (curr_time < time_sec_.front() && pose_x_.size() > 0) {
    // Interpolate between last pose and new traj first pose
    float x = Interpolate(time_at_load, time_sec_.front(), kinematics_at_load.pose.position.x(),
                          pose_x_.front(), curr_time);
    float y = Interpolate(time_at_load, time_sec_.front(), kinematics_at_load.pose.position.y(),
                          pose_y_.front(), curr_time);
    float z = Interpolate(time_at_load, time_sec_.front(), kinematics_at_load.pose.position.z(),
                          pose_z_.front(), curr_time);

    auto rpy = TransformUtils::ToRPY(kinematics_at_load.pose.orientation);
    float roll = InterpolateAngle(time_at_load, time_sec_.front(),
                                  rpy.x(), pose_roll_.front(),
                                  curr_time);
    float pitch = InterpolateAngle(time_at_load, time_sec_.front(),
                                   rpy.y(), pose_pitch_.front(),
                                   curr_time);
    float yaw = InterpolateAngle(time_at_load, time_sec_.front(),
                                 rpy.z(), pose_yaw_.front(),
                                 curr_time);

    float x_vel = Interpolate(time_at_load, time_sec_.front(),
                              kinematics_at_load.twist.linear.x(), velocity_linear_x_.front(),
                              curr_time);
    float y_vel = Interpolate(time_at_load, time_sec_.front(),
                              kinematics_at_load.twist.linear.y(), velocity_linear_y_.front(),
                              curr_time);
    float z_vel = Interpolate(time_at_load, time_sec_.front(),
                              kinematics_at_load.twist.linear.z(), velocity_linear_z_.front(),
                              curr_time);

    traj_kinematics.kinematics.pose.position = Vector3(x, y, z);
    traj_kinematics.kinematics.pose.orientation =
        TransformUtils::ToQuaternion(roll, pitch, yaw);
    traj_kinematics.kinematics.twist.linear = Vector3(x_vel, y_vel, z_vel);
  } else if (curr_time >= time_sec_.front() && curr_time <= time_sec_.back()) {
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
    float roll = InterpolateAngle(time_sec_, pose_roll_, upper, lower, curr_time) +
                 traj_kinematics.traj_params.roll_offset;
    float pitch =
        InterpolateAngle(time_sec_, pose_pitch_, upper, lower, curr_time) +
        traj_kinematics.traj_params.pitch_offset;
    float yaw = InterpolateAngle(time_sec_, pose_yaw_, upper, lower, curr_time) +
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

float Trajectory::Impl::InterpolateAngle(const std::vector<float>& time_array,
                                   const std::vector<float>& angle_array,
                                   int upper_index, int lower_index,
                                   float curr_time) const {
  return InterpolateAngle(time_array[lower_index], time_array[upper_index],
                          angle_array[lower_index], angle_array[upper_index],
                          curr_time);
}

float Trajectory::Impl::InterpolateAngle(float time_lower, float time_upper,
                                   float param_lower, float param_upper,
                                   float curr_time) const {
  auto param_lower_mod = fmod(param_lower, 2 * M_PI);
  auto param_upper_mod = fmod(param_upper, 2 * M_PI);

  if (abs(param_lower_mod - param_upper_mod) > 1.5 * M_PI) {
    if (param_lower_mod > param_upper_mod) {
      param_upper_mod += 2 * M_PI;
    } else {
      param_upper_mod -= 2 * M_PI;
    }
  }

  if (time_lower == time_upper) {  // edge case where time is either lower than
    // t[0] or higher than t[-1]
    return param_lower_mod;
  }

  // Simple linear interpolation between the two given values
  float param_val =
      param_lower_mod + (curr_time - time_lower) * ((param_upper_mod - param_lower_mod) /
                                                (time_upper - time_lower));

  return fmod(param_val, 2 * M_PI);
}

float Trajectory::Impl::Interpolate(const std::vector<float>& time_array,
                                    const std::vector<float>& param_array,
                                    int upper_index, int lower_index,
                                    float curr_time) const {
  auto param_lower = param_array[lower_index];
  auto param_upper = param_array[upper_index];
  auto time_lower = time_array[lower_index];
  auto time_upper = time_array[upper_index];
  
  return Interpolate(time_lower, time_upper, param_lower, param_upper, curr_time);
}

float Trajectory::Impl::Interpolate(const float time_lower, const float time_upper,
                                    const float param_lower, const float param_upper,
                                    const float curr_time) const {
  // edge case where currernt time is either lower than lower time or higher than upper time
  if (curr_time <= time_lower) {
    return param_lower;
  } 
  else if (curr_time >= time_upper) {
    return param_upper;
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
