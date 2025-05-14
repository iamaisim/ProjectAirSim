// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "matlab_physics.hpp"

#include <iostream>

#include "core_sim/actuators/lift_drag_control_surface.hpp"
#include "core_sim/actuators/rotor.hpp"
#include "json.hpp"
#include "message/physics_model_input_message.hpp"
#include "message/physics_model_output_message.hpp"
#include "nng/protocol/reqrep0/req.h"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class MatlabPhysicsBody

MatlabPhysicsBody::MatlabPhysicsBody(const Robot& robot) : sim_robot_(robot) {
  SetName(robot.GetID());
  SetPhysicsType(PhysicsType::kMatlabPhysics);
  InitializeMatlabPhysicsBody();

  const json& connection_json =
      json::parse(robot.GetPhysicsConnectionSettings());

  connection_string_ = "tcp://";
  connection_string_ += connection_json.value("ip", "127.0.0.1");
  connection_string_ += ":";
  connection_string_ += std::to_string(connection_json.value("port", 8900));
}

void MatlabPhysicsBody::InitializeMatlabPhysicsBody() {
  // TODO Copy robot's mass, inertia, drag, etc and send to Matlab to intialize
  // its physical parameters
}

void MatlabPhysicsBody::CalculateExternalWrench() {
  // No need to do anything here, all wrenches are sent to Matlab for processing
}

void MatlabPhysicsBody::ReadRobotData() {
  // Copy robot's latest data to Matlab physics body to be ready to pack and
  // send to Matlab

  // TODO Add a method on sim robot to GetActuatorWrenches() and
  // GetActuatorLiftDragControlAngles() directly
  external_wrench_entries_.clear();
  lift_drag_control_angles_.clear();
  auto& actuators = sim_robot_.GetActuators();
  for (auto& actuator_ref : actuators) {
    Actuator& actuator = actuator_ref.get();

    if (actuator.GetType() == ActuatorType::kRotor) {
      auto& rotor_ref = static_cast<Rotor&>(actuator);
      external_wrench_entries_.emplace_back(
          &rotor_ref.GetWrenchPoint(),
          &static_cast<TransformTree::RefFrame&>(rotor_ref));
    } else if (actuator.GetType() == ActuatorType::kLiftDragControlSurface) {
      // Populate lift_drag_control_angles_ map as:
      //   key = control surface actuator's parent link (wing it affects)
      //   value = control surface actuator's control angle
      const auto& control_surface_ref =
          static_cast<const LiftDragControlSurface&>(actuator);
      lift_drag_control_angles_.emplace_back(
          control_surface_ref.GetControlAngle());
    }
  }

  environment_info_ = sim_robot_.GetEnvironment().env_info;
  collision_info_ = sim_robot_.GetCollisionInfo();
}

void MatlabPhysicsBody::WriteRobotData(const Kinematics& kinematics) {
  sim_robot_.UpdateKinematics(kinematics);
}

MatlabPhysicsBody::operator TransformTree::RefFrame&(void) {
  return (const_cast<TransformTree::RefFrame&>(
      operator const TransformTree::RefFrame&()));
}

MatlabPhysicsBody::operator const TransformTree::RefFrame&(void) const {
  // If the physics body isn't loaded (units tests), return the global frame
  return (sim_robot_.IsLoaded()
              ? static_cast<const TransformTree::RefFrame&>(sim_robot_)
              : TransformTree::kRefFrameGlobal);
}

// -----------------------------------------------------------------------------
// class MatlabPhysicsModel

MatlabPhysicsModel::MatlabPhysicsModel() {}

MatlabPhysicsModel::~MatlabPhysicsModel() {}

void MatlabPhysicsModel::Start(std::shared_ptr<BasePhysicsBody> body) {
  // Dynamic cast to a MatlabPhysicsBody
  std::shared_ptr<MatlabPhysicsBody> matlab_body =
      std::dynamic_pointer_cast<MatlabPhysicsBody>(body);
  if (matlab_body != nullptr) {
    running_ = true;
  }
}

void MatlabPhysicsModel::SetWrenchesOnPhysicsBody(
    std::shared_ptr<BasePhysicsBody> body) {
  // Dynamic cast to a MatlabPhysicsBody
  std::shared_ptr<MatlabPhysicsBody> matlab_body =
      std::dynamic_pointer_cast<MatlabPhysicsBody>(body);
  if (matlab_body != nullptr) {
    matlab_body->CalculateExternalWrench();
  }
}

void MatlabPhysicsModel::ConnectToMatlab(
    std::shared_ptr<MatlabPhysicsBody> matlab_body) {
  // Create a Request socket to dial Matlab server
  int rv = nng_req0_open(&matlab_body->nng_socket_);

  // Connect to server
  // TODO Give up after a timeout?
  std::cout << "Trying to connect to Matlab physics model..." << std::endl;
  do {
    rv = nng_dial(matlab_body->nng_socket_,
                  matlab_body->connection_string_.c_str(), NULL, 0);
    std::cout << "NNG dial return value: " << rv << std::endl;
    if (rv != 0) std::this_thread::sleep_for(std::chrono::milliseconds(100));
  } while (rv != 0);

  matlab_body->connected_ = true;
}

void MatlabPhysicsModel::StepPhysicsBody(
    TimeNano dt_nanos, std::shared_ptr<BasePhysicsBody> body) {
  // Dynamic cast to a MatlabPhysicsBody
  std::shared_ptr<MatlabPhysicsBody> matlab_body =
      std::dynamic_pointer_cast<MatlabPhysicsBody>(body);

  if (matlab_body == nullptr) return;

  // ------------------------------------------------------------

  if (!matlab_body->connected_ && running_.load() == true) {
    ConnectToMatlab(matlab_body);
  }

  // ------------------------------------------------------------

  auto dt_sec = SimClock::Get()->NanosToSec(dt_nanos);

  // Update physics body's data from robot's latest data
  matlab_body->ReadRobotData();

  // ------------------------------------------------------------

  // Send a request to Matlab with wrenches and collision info flag
  auto cur_simtime = static_cast<uint64_t>(SimClock::Get()->NowSimNanos());

  std::vector<WrenchPointFlatMsgpack> wrench_pts;
  auto& refframe_body =
      static_cast<const TransformTree::RefFrame&>(*matlab_body.get());
  auto transform_tree = refframe_body.GetTransformTree();
  for (const auto& wrench_entry : matlab_body->external_wrench_entries_) {
    // External wrench point values are relative to their local frame--
    // make the values relative to the body
    auto& wrench_pt = *wrench_entry.wrench_point;
    auto pose = Pose(wrench_pt.position, Quaternion::Identity());

    transform_tree->Convert(pose, *wrench_entry.refframe, refframe_body, &pose);

    auto force = pose.orientation._transformVector(wrench_pt.wrench.force);
    auto& position = pose.position;
    auto torque = pose.orientation._transformVector(wrench_pt.wrench.torque);

    wrench_pts.emplace_back(force.x(), force.y(), force.z(), torque.x(),
                            torque.y(), torque.z(), position.x(), position.y(),
                            position.z());
  }

  EnvironmentInfoFlatMsgpack env_info(
      matlab_body->environment_info_.geo_point.latitude,
      matlab_body->environment_info_.geo_point.longitude,
      matlab_body->environment_info_.geo_point.altitude,
      matlab_body->environment_info_.gravity.x(),
      matlab_body->environment_info_.gravity.y(),
      matlab_body->environment_info_.gravity.z(),
      matlab_body->environment_info_.temperature,
      matlab_body->environment_info_.air_pressure,
      matlab_body->environment_info_.air_density);

  CollisionInfoFlatMsgpack collision_info(
      matlab_body->collision_info_.has_collided,
      matlab_body->collision_info_.normal.x(),
      matlab_body->collision_info_.normal.y(),
      matlab_body->collision_info_.normal.z(),
      matlab_body->collision_info_.impact_point.x(),
      matlab_body->collision_info_.impact_point.y(),
      matlab_body->collision_info_.impact_point.z(),
      matlab_body->collision_info_.position.x(),
      matlab_body->collision_info_.position.y(),
      matlab_body->collision_info_.position.z(),
      matlab_body->collision_info_.penetration_depth,
      matlab_body->collision_info_.time_stamp,
      matlab_body->collision_info_.object_name,
      matlab_body->collision_info_.segmentation_id);

  PhysicsModelInputMessage request_msg(cur_simtime, wrench_pts,
                                       matlab_body->lift_drag_control_angles_,
                                       env_info, collision_info);

  std::string request_str = request_msg.Serialize();

  int rv = nng_send(matlab_body->nng_socket_, request_str.data(),
                    request_str.length(), 0);

  if (rv != 0) {
    std::cout << "nng_send() return value: " << rv
              << " length = " << request_str.length() << std::endl;
  }

  if (rv != 0 || running_.load() == false) {
    matlab_body->connected_ = false;
    return;
  }

  // ------------------------------------------------------------

  // Wait for a response from Matlab with new kinamatics
  char* msg_data_buf = nullptr;
  size_t msg_data_len;

  rv = NNG_EAGAIN;
  while (rv == NNG_EAGAIN && running_.load() == true) {
    rv = nng_recv(matlab_body->nng_socket_, &msg_data_buf, &msg_data_len,
                  NNG_FLAG_ALLOC | NNG_FLAG_NONBLOCK);
  }

  if (rv != 0) {
    std::cout << "nng_recv() return value: " << rv
              << " length = " << msg_data_len << std::endl;
  }

  if (rv != 0 || running_.load() == false) {
    matlab_body->connected_ = false;
    return;
  }

  // Copy the data from the buffer to a string to deserialize (must set the
  // length manually to avoid stopping at '\0' chars that it may contain)
  std::string response_str;
  response_str.resize(msg_data_len);
  response_str.assign(msg_data_buf, msg_data_len);

  // Free the NNG-allocated reply buffer memory
  nng_free(msg_data_buf, msg_data_len);

  PhysicsModelOutputMessage response_msg;
  response_msg.Deserialize(response_str);

  Kinematics next_kin;
  next_kin.pose.position.x() = response_msg.kinematics.pose_position_x;
  next_kin.pose.position.y() = response_msg.kinematics.pose_position_y;
  next_kin.pose.position.z() = response_msg.kinematics.pose_position_z;
  next_kin.pose.orientation.x() = response_msg.kinematics.pose_orientation_x;
  next_kin.pose.orientation.y() = response_msg.kinematics.pose_orientation_y;
  next_kin.pose.orientation.z() = response_msg.kinematics.pose_orientation_z;
  next_kin.pose.orientation.w() = response_msg.kinematics.pose_orientation_w;
  next_kin.twist.linear.x() = response_msg.kinematics.twist_linear_x;
  next_kin.twist.linear.y() = response_msg.kinematics.twist_linear_y;
  next_kin.twist.linear.z() = response_msg.kinematics.twist_linear_z;
  next_kin.twist.angular.x() = response_msg.kinematics.twist_angular_x;
  next_kin.twist.angular.y() = response_msg.kinematics.twist_angular_y;
  next_kin.twist.angular.z() = response_msg.kinematics.twist_angular_z;
  next_kin.accels.linear.x() = response_msg.kinematics.accels_linear_x;
  next_kin.accels.linear.y() = response_msg.kinematics.accels_linear_y;
  next_kin.accels.linear.z() = response_msg.kinematics.accels_linear_z;
  next_kin.accels.angular.x() = response_msg.kinematics.accels_angular_x;
  next_kin.accels.angular.y() = response_msg.kinematics.accels_angular_y;
  next_kin.accels.angular.z() = response_msg.kinematics.accels_angular_z;
  matlab_body->WriteRobotData(next_kin);
}

void MatlabPhysicsModel::Stop(std::shared_ptr<BasePhysicsBody> body) {
  running_ = false;

  // Dynamic cast to a MatlabPhysicsBody
  std::shared_ptr<MatlabPhysicsBody> matlab_body =
      std::dynamic_pointer_cast<MatlabPhysicsBody>(body);
  if (matlab_body == nullptr) return;

  // Close the socket
  int rv = nng_close(matlab_body->nng_socket_);

  if (rv != 0) {
    std::cout << "nng_close() return value: " << rv << std::endl;
  }

  matlab_body->connected_ = false;
}

}  // namespace projectairsim
}  // namespace microsoft
