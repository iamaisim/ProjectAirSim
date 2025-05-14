// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_TRAJECTORY_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_TRAJECTORY_HPP_

#include <memory>

#include "core_sim/actor.hpp"
#include "core_sim/physics_common_types.hpp"

namespace microsoft {
namespace projectairsim {

class ConfigJson;
class Logger;
class TopicManager;

struct TrajectoryParams {
  float time_offset = 0;
  float x_offset = 0;
  float y_offset = 0;
  float z_offset = 0;
  float roll_offset = 0;
  float pitch_offset = 0;
  float yaw_offset = 0;

  bool to_loop = false;
  int num_loops = 0;
};

struct TrajectoryKinematics {
  TrajectoryParams traj_params;
  Kinematics kinematics;
};

class Trajectory {
 public:
  Trajectory();

  bool IsLoaded() const;

  const std::string& GetName() const;

 private:
  friend class EnvActor;
  friend class EnvActorGrounded;
  friend class EnvCar;
  friend class Scene;

  Trajectory(const Logger& logger);

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

  class Impl;
  class Loader;

  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_TRAJECTORY_HPP_