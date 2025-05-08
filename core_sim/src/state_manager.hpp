// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_SRC_STATE_MANAGER_HPP_
#define CORE_SIM_SRC_STATE_MANAGER_HPP_

#include <memory>

#include "core_sim/clock.hpp"
#include "core_sim/config_json.hpp"

namespace microsoft {
namespace projectairsim {

class Logger;
class Scene;

// StateManager is intended to manage the synchronization of state for
// distributed simulation instances. It does not own the state of the robots or
// robot components, but can read/write them and serialize the state into
// messages to pass between instances. It also encapsulates the transport layer
// used for this purpose, currently re-using NNG REQ-RESP to pass the physics
// state peer-to-peer to allow partitioning by robot over multiple instances.
//
// Currently, all state for each robot is calculated on a single instance (and
// replicated as a non-physics robot on the other instances), but in the future
// the robot components (like individual sensors) could also be distributed over
// a sub-layer of instances for that robot. All
// ComponentWithTopicsAndServiceMethods objects have access to the StateManager
// reference, so the robots could manage their state distribution directly
// through this same class and transport layer.
//
// In the future, there may also be ways to leverage StateManager to
// save/restore the general scene state for checkpointing or data logging, but
// it is not intended to capture all internal state of all components.
class StateManager {
 public:
  explicit StateManager(const Logger& logger);

  ~StateManager();

  void ConnectSims();

  void Initialize(Scene* scene);

  bool IsClockSource() const;

  bool IsConnected() const;

  bool IsDistributed() const;

  bool IsLocalRobot(const std::string& robot_id);

  bool IsRunning() const;

  void Load(const json& config_json);

  TimeNano ReceiveSimTime();

  void SendSimTime(TimeNano sim_time);

  void Start();

  void Stop();

  void SynchronizePhysicsState(TimeNano time_stamp);

 private:
  class Impl;
  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_STATE_MANAGER_HPP_
