// Copyright (C) Microsoft Corporation. All rights reserved.

#include "state_manager.hpp"

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "constant.hpp"
#include "core_sim/actor/robot.hpp"
#include "core_sim/actuators/rotor.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/message/physics_state_message.hpp"
#include "core_sim/scene.hpp"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"
#include "nng/protocol/reqrep0/req.h"

namespace microsoft {
namespace projectairsim {

// ----------------------------------------------------------------------------
// Forward declarations

class StateManager::Impl {
 public:
  explicit Impl(const Logger& logger);

  void ConnectSims();

  void DisconnectSims();

  void Initialize(Scene* scene);

  bool IsClockSource() const;

  bool IsConnected() const;

  bool IsDistributed() const;

  bool IsLocalRobot(const std::string& robot_id);

  bool IsRunning() const;

  void Load(const json& config_json);

  bool ReceiveAck(nng_socket& socket);

  void ReceivePhysicsState(int robot_instance_idx);

  TimeNano ReceiveSimTime();

  void SendPhysicsState(PhysicsStateMessage physics_state_msg);

  void SendSimTime(TimeNano sim_time);

  void Start();

  void Stop();

  void SynchronizePhysicsState(TimeNano time_stamp);

 private:
  // Index of the instance that is the sim clock source so non-source instances
  // know which instance they need to receive the clock steps from
  int clock_source_idx_;

  // The full "distributed-sim" config JSON from the scene config
  json distributed_sim_settings_;

  // Flag set to true if this instance is the sim clock source for all of the
  // instances (there should only be one)
  bool is_clock_source_;

  // Flag set to true if the distributed instances have connected to each other
  bool is_connected_;

  // Flag set to true if this instance is part of a distributed scene
  bool is_distributed_;

  // Flag set to true if the scene tick is running
  bool is_running_;

  Logger logger_;

  // Map of robot IDs and their local instance indices
  std::unordered_map<std::string, int> robot_instance_map_;

  Scene* sim_scene_;

  // Socket matrix used for each instance to send to/receive from any other
  // instance. Currently, there is a separate REQ/RESP socket opened for each
  // message direction between each instance.
  // TODO Investigate using a common shared datastore/cache like Azure Cache for
  // Redis instead of peer-to-peer sockets.
  //   1st index = source instance
  //   2nd index = destination instance
  //   value = socket
  std::vector<std::vector<nng_socket>> socket_matrix_;

  // This scene's ID from the distributed sim setting's "sim-scenes" array
  // (including the appended "-{index}" which currently needs to be hard-coded
  // in the config)
  std::string this_scene_id_;

  // Index of this instance's scene config in the distributed sim setting's
  // "sim-scenes" array
  int this_scene_idx_;
};

// ----------------------------------------------------------------------------
// class StateManager

StateManager::StateManager(const Logger& logger)
    : pimpl_(std::make_shared<Impl>(logger)) {}

StateManager::~StateManager() {}

void StateManager::ConnectSims() { pimpl_->ConnectSims(); }

void StateManager::Initialize(Scene* scene) { pimpl_->Initialize(scene); }

bool StateManager::IsClockSource() const { return pimpl_->IsClockSource(); }

bool StateManager::IsConnected() const { return pimpl_->IsConnected(); }

bool StateManager::IsDistributed() const { return pimpl_->IsDistributed(); }

bool StateManager::IsLocalRobot(const std::string& robot_id) {
  return pimpl_->IsLocalRobot(robot_id);
}

bool StateManager::IsRunning() const { return pimpl_->IsRunning(); }

void StateManager::Load(const json& config_json) { pimpl_->Load(config_json); }

TimeNano StateManager::ReceiveSimTime() { return pimpl_->ReceiveSimTime(); }

void StateManager::SendSimTime(TimeNano sim_time) {
  pimpl_->SendSimTime(sim_time);
}

void StateManager::Start() { pimpl_->Start(); }

void StateManager::Stop() { pimpl_->Stop(); }

void StateManager::SynchronizePhysicsState(TimeNano time_stamp) {
  return pimpl_->SynchronizePhysicsState(time_stamp);
}

// ----------------------------------------------------------------------------
// class StateManager::Impl

StateManager::Impl::Impl(const Logger& logger) : logger_(logger) {
  Initialize(nullptr);
}

void StateManager::Impl::ConnectSims() {
  if (!is_distributed_ /* not a distributed scene */ ||
      !is_running_ /* scene tick is not running or in process of stopping*/ ||
      is_connected_ /* instances are already connected */) {
    return;
  }

  // ------------------------------------------------------------------------
  // Connect all of the sockets between sim instances.

  const json& sim_scenes = distributed_sim_settings_["sim-scenes"];
  const json& ports_simstate = distributed_sim_settings_["ports-simstate"];
  // First, open listeners for all the ports with this instance as destination.
  socket_matrix_.clear();
  socket_matrix_.resize(ports_simstate.size());

  for (int src_idx = 0; src_idx < ports_simstate.size(); ++src_idx) {
    socket_matrix_[src_idx].resize(ports_simstate.size());
    if (src_idx == this_scene_idx_) continue;
    int port = ports_simstate[src_idx][this_scene_idx_];
    // Open RESP socket
    std::string connection_string = "tcp://*:";
    // TODO Try to get IP filtering to work on the RESP listening side to
    // replace "*" with "sim_scenes[src_idx]["ip"]" in the connection string;
    connection_string += std::to_string(port);

    nng_socket resp_socket = NNG_SOCKET_INITIALIZER;
    int rv = nng_rep0_open(&resp_socket);

    logger_.LogVerbose(this_scene_id_,
                       "[StateManager] Sim %d listening for sim %d at %s...",
                       this_scene_idx_, src_idx, connection_string.c_str());

    rv = nng_listen(resp_socket, connection_string.c_str(),
                    NULL /*create a new listener*/, 0 /*no special flags*/);

    logger_.LogVerbose(this_scene_id_,
                       "[StateManager] nng_listen() return value: %d", rv);

    socket_matrix_[src_idx][this_scene_idx_] = resp_socket;
  }

  // Second, dial all the ports with this instance as a source.
  for (int dest_idx = 0; dest_idx < ports_simstate.size(); ++dest_idx) {
    if (dest_idx == this_scene_idx_) continue;
    int port = ports_simstate[this_scene_idx_][dest_idx];
    // Open REQ socket
    std::string connection_string = "tcp://";
    connection_string += sim_scenes[dest_idx]["ip"];
    connection_string += ":";
    connection_string += std::to_string(port);

    nng_socket req_socket = NNG_SOCKET_INITIALIZER;
    int rv = nng_req0_open(&req_socket);
    logger_.LogVerbose(this_scene_id_,
                       "[StateManager] Sim %d dialing sim %d at %s...",
                       this_scene_idx_, dest_idx, connection_string.c_str());
    do {
      // Bail out if sim reloads/shuts down while connecting is in progress
      if (!is_running_) return;

      rv = nng_dial(req_socket, connection_string.c_str(),
                    NULL /*create a new dialer*/, 0 /*no special flags*/);
      logger_.LogVerbose(
          this_scene_id_,
          "[StateManager] nng_dial() return value: %d. Waiting to try again...",
          rv);
      if (rv != 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    } while (rv != 0);

    socket_matrix_[this_scene_idx_][dest_idx] = req_socket;
  }

  is_connected_ = true;
}

void StateManager::Impl::DisconnectSims() {
  for (int src_idx = 0; src_idx < socket_matrix_.size(); ++src_idx) {
    for (int dest_idx = 0; dest_idx < socket_matrix_[src_idx].size();
         ++dest_idx) {
      if (src_idx == dest_idx) continue;
      nng_socket socket = socket_matrix_[src_idx][dest_idx];
      int rv = nng_close(socket);

      if (rv != 0) {
        logger_.LogVerbose(this_scene_id_,
                           "[StateManager] nng_close() return value: %d", rv);
      }
    }
  }

  is_connected_ = false;
}

void StateManager::Impl::Initialize(Scene* scene) {
  sim_scene_ = scene;

  // Reset to defaults
  this_scene_idx_ = -1;
  this_scene_id_ = "";
  distributed_sim_settings_ = "{}"_json;
  socket_matrix_.clear();
  is_distributed_ = false;
  is_running_ = false;
  is_connected_ = false;
  is_clock_source_ = true;
}

bool StateManager::Impl::IsClockSource() const { return is_clock_source_; }

bool StateManager::Impl::IsConnected() const { return is_connected_; }

bool StateManager::Impl::IsDistributed() const { return is_distributed_; }

bool StateManager::Impl::IsLocalRobot(const std::string& robot_id) {
  int robot_instance_idx = -1;
  auto instance_map_itr = robot_instance_map_.find(robot_id);
  if (instance_map_itr != robot_instance_map_.end()) {
    robot_instance_idx = instance_map_itr->second;
  }
  return (robot_instance_idx == this_scene_idx_);
}

bool StateManager::Impl::IsRunning() const { return is_running_; }

void StateManager::Impl::Load(const json& config_json) {
  if (sim_scene_ == nullptr) return;

  distributed_sim_settings_ =
      JsonUtils::GetJsonObject(config_json, Constant::Config::distributed_sim);

  // ------------------------------------------------------------------------
  // Find this instance's scene index and config

  if (JsonUtils::IsEmpty(distributed_sim_settings_)) return;
  const json& sim_scenes = distributed_sim_settings_["sim-scenes"];
  this_scene_id_ = sim_scene_->GetID();
  this_scene_idx_ = -1;
  for (int i = 0; i < sim_scenes.size(); ++i) {
    if (this_scene_id_ == sim_scenes[i]["id"].get<std::string>()) {
      this_scene_idx_ = i;
      break;
    }
  }

  if (this_scene_idx_ == -1) return;  // not found in distributed sim settings

  // ------------------------------------------------------------------------

  is_distributed_ = true;
  clock_source_idx_ = distributed_sim_settings_["simclock-source"];
  auto& actors = sim_scene_->GetActors();

  // ------------------------------------------------------------------------
  // Set up sim clock control

  if (this_scene_idx_ == clock_source_idx_) {
    is_clock_source_ = true;
    SimClock::Get()->SetExternallySteppedOnly(false);
  } else {
    is_clock_source_ = false;
    SimClock::Get()->SetExternallySteppedOnly(true);
  }

  // ------------------------------------------------------------------------
  // Build robot_instance_map_ from sim-scenes array

  robot_instance_map_.clear();
  for (auto& actor : actors) {
    if (actor.get().GetType() == ActorType::kRobot) {
      auto& robot = static_cast<Robot&>(actor.get());
      auto& robot_id = robot.GetID();
      int robot_instance_idx = -1;

      for (int instance_idx = 0; instance_idx < sim_scenes.size();
           ++instance_idx) {
        const json& local_robots = sim_scenes[instance_idx]["local-robots"];

        for (const auto& local_robot_id_json : local_robots) {
          std::string local_robot_id =
              local_robot_id_json
                  .get<std::string>();  // Convierte JSON a string
          if (robot_id == local_robot_id) {
            robot_instance_idx = instance_idx;
            break;
          }
        }

        if (robot_instance_idx != -1) break;
      }

      robot_instance_map_.emplace(robot_id, robot_instance_idx);
    }
  }

  // ------------------------------------------------------------------------
  // Overwrite non-local robots to non-physics type and disable sensors

  for (auto& actor : actors) {
    if (actor.get().GetType() == ActorType::kRobot) {
      auto& robot = static_cast<Robot&>(actor.get());
      if (!IsLocalRobot(robot.GetID())) {
        robot.SetPhysicsType(PhysicsType::kNonPhysics);

        auto& sensors = robot.GetSensors();
        for (auto& sensor : sensors) {
          sensor.get().SetEnabled(false);
        }
      }
    }
  }
}

bool StateManager::Impl::ReceiveAck(nng_socket& socket) {
  // Wait for a response confirmation
  bool* resp_data_buf = nullptr;
  size_t resp_data_len;

  int rv = NNG_EAGAIN;
  while (rv == NNG_EAGAIN && is_running_) {
    // TODO Add running flag condition to break in case of scene reload
    rv = nng_recv(socket, &resp_data_buf, &resp_data_len,
                  NNG_FLAG_ALLOC | NNG_FLAG_NONBLOCK);
  }

  // Process ack response
  bool is_ack_successful = false;
  if (rv != 0) {
    logger_.LogWarning(this_scene_id_,
                       "[StateManager::ReceiveAck] nng_recv() return value: %d",
                       rv);
  } else if (resp_data_len <= 0) {
    logger_.LogWarning(this_scene_id_,
                       "[StateManager::ReceiveAck] Empty response data.");
  } else {
    is_ack_successful = *resp_data_buf;
  }

  // Free the NNG-allocated reply buffer memory
  nng_free(resp_data_buf, resp_data_len);

  if (!is_ack_successful) {
    logger_.LogWarning(
        this_scene_id_,
        "[StateManager::ReceiveAck] Received unsuccessful response.");
  }

  return is_ack_successful;
}

void StateManager::Impl::ReceivePhysicsState(int src_idx) {
  nng_socket& socket = socket_matrix_[src_idx][this_scene_idx_];
  char* msg_data_buf = nullptr;
  size_t msg_data_len;

  int rv = NNG_EAGAIN;
  while (rv == NNG_EAGAIN && is_running_) {
    rv = nng_recv(
        socket, &msg_data_buf, &msg_data_len,
        NNG_FLAG_ALLOC /* have NNG allocate the necessary buffer memory*/ |
            NNG_FLAG_NONBLOCK);
  }

  if (rv != 0) {
    logger_.LogVerbose(
        this_scene_id_,
        "[StateManager::ReceivePhysicsState] nng_recv() return value: %d", rv);
    return;
  }

  // Copy the data from the buffer to a string to deserialize (must set the
  // length manually to avoid stopping at '\0' chars that it may contain)
  std::string msg_data_str;
  msg_data_str.resize(msg_data_len);
  msg_data_str.assign(msg_data_buf, msg_data_len);

  // Free the NNG-allocated reply buffer memory
  nng_free(msg_data_buf, msg_data_len);

  PhysicsStateMessage physics_state_msg;
  physics_state_msg.Deserialize(msg_data_str);

  TimeNano time_stamp = physics_state_msg.GetTimeStamp();
  const auto physics_state = physics_state_msg.GetPhysicsState();
  const auto actuated_rot_state = physics_state_msg.GetActuatedRotationsState();
  auto actors = sim_scene_->GetActors();
  for (auto& actor : actors) {
    if (actor.get().GetType() == ActorType::kRobot) {
      auto& robot = static_cast<Robot&>(actor.get());
      const auto& robot_id = robot.GetID();

      // Set robot's kinematics from message state data and time stamp
      const auto& phys_state_itr = physics_state.find(robot_id);
      if (phys_state_itr != physics_state.end()) {
        robot.UpdateKinematics(phys_state_itr->second, time_stamp);
      }

      // Set robot's actuated rotations from message state data and time stamp
      const auto& actuated_rot_state_itr = actuated_rot_state.find(robot_id);
      if (actuated_rot_state_itr != actuated_rot_state.end()) {
        robot.SetActuatedRotations(actuated_rot_state_itr->second, time_stamp);
      }
    }
  }

  // Send a acknowledgement response back
  bool ack = true;
  rv = nng_send(socket, &ack, sizeof(ack), 0 /*no special flags*/);

  if (rv != 0) {
    logger_.LogVerbose(
        this_scene_id_,
        "[StateManager::ReceivePhysicsState] nng_send() return value: %d", rv);
  }
}

TimeNano StateManager::Impl::ReceiveSimTime() {
  // Use NNG REQ-RESP to receive the new sim_time from the clock source.
  nng_socket& socket = socket_matrix_[clock_source_idx_][this_scene_idx_];
  TimeNano* req_data_buf = nullptr;
  size_t req_data_len;

  int rv = NNG_EAGAIN;
  while (rv == NNG_EAGAIN && is_running_) {
    rv = nng_recv(
        socket, &req_data_buf, &req_data_len,
        NNG_FLAG_ALLOC /* have NNG allocate the necessary buffer memory*/ |
            NNG_FLAG_NONBLOCK);
  }

  if (rv != 0) {
    logger_.LogVerbose(
        this_scene_id_,
        "[StateManager::ReceiveSimTime] nng_recv() return value: %d", rv);
    return -1;
  }

  TimeNano new_sim_time = *req_data_buf;

  // Free the NNG-allocated reply buffer memory
  nng_free(req_data_buf, req_data_len);

  // Send a acknowledgement response back
  bool ack = true;
  rv = nng_send(socket, &ack, sizeof(ack), 0 /*no special flags*/);

  if (rv != 0) {
    logger_.LogVerbose(
        this_scene_id_,
        "[StateManager::ReceiveSimTime] nng_send() return value: %d", rv);
  }

  return new_sim_time;
}

// SendPhysicsState() sends the physics state that was calculated by this sim
// instance for all of its local robots to every other instance so they will be
// able to replicate them in non-physics mode. For each destination instance,
// the physics state message is sent as a REQ message and then ReceiveAck() is
// called to wait until an acknowledgement RESP message comes back to guarantee
// no state messages are dropped.
// TODO Peer-to-peer physics state transfer scales exponentially with the number
// of robots that have their own instances, so in the future this method could
// change to sending the physics state to a shared cache/datastore instance or
// service that all other instances can pull from to get to linear scaling.
void StateManager::Impl::SendPhysicsState(
    PhysicsStateMessage physics_state_msg) {
  std::string request_str = physics_state_msg.Serialize();

  // Send physics state message to all other instances
  for (int dest_idx = 0; dest_idx < socket_matrix_[this_scene_idx_].size();
       ++dest_idx) {
    if (dest_idx == this_scene_idx_) continue;

    nng_socket& socket = socket_matrix_[this_scene_idx_][dest_idx];

    int rv = nng_send(socket, request_str.data(), request_str.length(), 0);

    if (rv != 0) {
      logger_.LogVerbose(
          this_scene_id_,
          "[StateManager::SendPhysicsState] nng_send() return value: %d", rv);
    }

    // Wait for a response confirmation
    bool ack_resp = ReceiveAck(socket);
    if (!ack_resp) {
      logger_.LogVerbose(this_scene_id_,
                         "[StateManager::SendPhysicsState] Ack response: %d",
                         ack_resp);
    }
  }
}

void StateManager::Impl::SendSimTime(TimeNano sim_time) {
  // Use NNG REQ-RESP to send the sim_time to the other instances.
  void* sim_time_ptr = &sim_time;
  for (int dest_idx = 0; dest_idx < socket_matrix_[this_scene_idx_].size();
       ++dest_idx) {
    if (dest_idx == this_scene_idx_) continue;
    nng_socket& socket = socket_matrix_[this_scene_idx_][dest_idx];
    int rv = nng_send(socket, sim_time_ptr, sizeof(sim_time),
                      0 /*no special flags*/);

    if (rv != 0) {
      logger_.LogVerbose(
          this_scene_id_,
          "[StateManager::SendSimTime] nng_send() return value: %d", rv);
    }

    if (rv != 0) continue;  // TODO How to handle failed sends?

    // Wait for a response confirmation
    // TODO Loop through all sending first, then loop through receives?
    bool ack_resp = ReceiveAck(socket);
    if (!ack_resp) {
      logger_.LogVerbose(this_scene_id_,
                         "[StateManager::SendSimTime] Ack response: %d",
                         ack_resp);
    }
  }
}

void StateManager::Impl::Start() { is_running_ = true; }

void StateManager::Impl::Stop() {
  is_running_ = false;
  DisconnectSims();
}

void StateManager::Impl::SynchronizePhysicsState(TimeNano time_stamp) {
  // k:robot ID, v:kinematics
  std::unordered_map<std::string, Kinematics> physics_state_to_send;

  // k:robot ID, v:{k:link ID, v:(rad/s x, rad/s y, rad/s z)}
  std::unordered_map<std::string, ActuatedRotations>
      actuated_rots_state_to_send;

  bool sent_physics_state = false;
  std::set<int> instances_to_recv_from;
  auto& actors = sim_scene_->GetActors();

  // Pack physics state to send from local robot data
  for (int i = 0; i < actors.size(); ++i) {
    auto& actor = actors[i];
    if (actor.get().GetType() == ActorType::kRobot) {
      auto& robot = static_cast<Robot&>(actor.get());
      const std::string& robot_id = robot.GetID();
      if (IsLocalRobot(robot_id)) {
        // Pack this robot's kinematics
        const Kinematics& kin = robot.GetKinematics();
        physics_state_to_send.insert({robot_id, kin});

        // Pack this robot's actuated rotations
        // k:link ID, v:(rad/s x, rad/s y, rad/s z)
        ActuatedRotations cur_robot_actuated_rots;

        const auto& actuators = robot.GetActuators();
        for (auto& actuator_ref : actuators) {
          const Actuator& actuator = actuator_ref.get();
          if (actuator.GetType() == ActuatorType::kRotor) {
            const auto& rotor_ref = static_cast<const Rotor&>(actuator);
            const auto& rotor_actuated_rots = rotor_ref.GetActuatedRotations();
            for (const auto& [output_link, rotor_ang_vel] :
                 rotor_actuated_rots) {
              cur_robot_actuated_rots.insert({output_link, rotor_ang_vel});
            }
          }
        }

        actuated_rots_state_to_send.insert({robot_id, cur_robot_actuated_rots});
      } else {
        int robot_instance_idx = robot_instance_map_.at(robot_id);
        instances_to_recv_from.insert(robot_instance_idx);
      }
    }
  }

  // Construct message from packed physics state data
  PhysicsStateMessage physics_state_msg(time_stamp, physics_state_to_send,
                                        actuated_rots_state_to_send);

  // Loop through the actors in vector order to either send (local robots) or
  // receive (non-local robots) their physics state so the sequence of blocking
  // send/receive loops will match between instances
  for (int i = 0; i < actors.size(); ++i) {
    auto& actor = actors[i];
    if (actor.get().GetType() == ActorType::kRobot) {
      auto& robot = static_cast<Robot&>(actor.get());
      const std::string& robot_id = robot.GetID();
      int robot_instance_idx = robot_instance_map_.at(robot_id);
      if (IsLocalRobot(robot_id)) {
        if (sent_physics_state) {
          // This robot's data should have already been sent to all other
          // instances as part of the physics state message from the first local
          // robot of this instance in the actor sequence.
          continue;
        }

        // Send physics state to all other instances
        SendPhysicsState(physics_state_msg);

        // Set sent_physics_state flag to true since all robot data from this
        // instance has already been sent and processed.
        sent_physics_state = true;
      } else {  // non-local robot
        if (instances_to_recv_from.find(robot_instance_idx) ==
            instances_to_recv_from.end()) {
          // This robot's data should have already been received as part of the
          // physics state message from the first robot of this remote instance
          // in the actor sequence.
          continue;
        }

        // Receive physics state from this non-local robot's instance
        ReceivePhysicsState(robot_instance_idx);

        // Remove this instance from the receive list since all robot data from
        // it has already been received and processed.
        instances_to_recv_from.erase(robot_instance_idx);
      }
    }
  }
}

}  // namespace projectairsim
}  // namespace microsoft
