// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include "matlab_physics_model_wrapper.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

#include "message/common_message_utils.hpp"
#include "message/physics_model_input_message.hpp"
#include "message/physics_model_output_message.hpp"
#include "msgpack.hpp"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"

// -----------------------------------------------------------------------
// NngMgr class for managing socket connection with the sim

namespace {

class NngMgr {
 public:
  NngMgr(const std::string& addr, double timeout_sec);

  ~NngMgr();

  void StartListening();

  void StopListening();

  void SendResponse(std::string response_str);

  std::string RetrieveRequest();

 private:
  bool connected_ = false;
  nng_socket socket_ = NNG_SOCKET_INITIALIZER;
  std::string addr_;
  nng_duration timeout_ms_;
};

nng_duration TransportTimeoutMs(double timeout_sec) {
  if (!(timeout_sec > 0.0) || std::isinf(timeout_sec)) {
    return NNG_DURATION_INFINITE;
  }
  const double timeout_ms = std::ceil(timeout_sec * 1000.0);
  const double max_ms = std::numeric_limits<nng_duration>::max();
  return static_cast<nng_duration>(std::min(timeout_ms, max_ms));
}

NngMgr::NngMgr(const std::string& addr, double timeout_sec)
    : addr_(addr), timeout_ms_(TransportTimeoutMs(timeout_sec)) {}

NngMgr::~NngMgr() { StopListening(); }

void NngMgr::StopListening() {
  if (!connected_) return;
  nng_close(socket_);
  socket_ = NNG_SOCKET_INITIALIZER;
  connected_ = false;
}

void NngMgr::StartListening() {
  if (connected_) return;

  std::cout << "Opening connection with sim server..." << std::endl;

  int rv = nng_rep0_open(&socket_);
  if (rv != 0) {
    throw std::runtime_error("nng_rep0_open(): " +
                             std::string(nng_strerror(rv)));
  }

  rv = nng_socket_set_ms(socket_, NNG_OPT_RECVTIMEO, timeout_ms_);
  if (rv == 0) {
    rv = nng_socket_set_ms(socket_, NNG_OPT_SENDTIMEO, timeout_ms_);
  }
  if (rv == 0) {
    rv = nng_listen(socket_, addr_.c_str(), nullptr, 0);
  }
  if (rv != 0) {
    nng_close(socket_);
    socket_ = NNG_SOCKET_INITIALIZER;
    throw std::runtime_error("NNG listener " + addr_ + ": " + nng_strerror(rv));
  }
  connected_ = true;
}

void NngMgr::SendResponse(std::string response_str) {
  int rv = nng_send(socket_, const_cast<char*>(response_str.data()),
                    response_str.length(), 0);
  if (rv != 0) {
    throw std::runtime_error("nng_send() on " + addr_ + ": " +
                             nng_strerror(rv));
  }
}

std::string NngMgr::RetrieveRequest() {
  char* msg_data_buf = nullptr;
  size_t msg_data_len = 0;
  int rv = nng_recv(socket_, &msg_data_buf, &msg_data_len, NNG_FLAG_ALLOC);
  if (rv != 0) {
    throw std::runtime_error("nng_recv() on " + addr_ + ": " +
                             nng_strerror(rv));
  }

  auto free_message = [msg_data_len](char* ptr) {
    nng_free(ptr, msg_data_len);
  };
  std::unique_ptr<char, decltype(free_message)> message(msg_data_buf,
                                                        free_message);
  return std::string(message.get(), msg_data_len);
}

}  // namespace

void CopyDataToOutputPorts(
    const microsoft::projectairsim::PhysicsModelInputMessage& request_msg,
    double* out_port_0_wrench_points, double* out_port_1_wing_control_angles,
    double* out_port_2_env_info, double* out_port_3_collision_info) {
  for (int i = 0; i < NUM_SUPPORTED_ROTORS; ++i) {
    if (i < request_msg.external_wrench_points.size()) {
      const auto& wrench_pt = request_msg.external_wrench_points[i];
      *out_port_0_wrench_points++ = wrench_pt.force_x;
      *out_port_0_wrench_points++ = wrench_pt.force_y;
      *out_port_0_wrench_points++ = wrench_pt.force_z;
      *out_port_0_wrench_points++ = wrench_pt.torque_x;
      *out_port_0_wrench_points++ = wrench_pt.torque_y;
      *out_port_0_wrench_points++ = wrench_pt.torque_z;
      *out_port_0_wrench_points++ = wrench_pt.position_x;
      *out_port_0_wrench_points++ = wrench_pt.position_y;
      *out_port_0_wrench_points++ = wrench_pt.position_z;
    } else {
      for (int j = 0; j < 9; ++j) {  // 9 values per wrench
        *out_port_0_wrench_points++ = 0.0;
      }
    }
  }

  for (int i = 0; i < NUM_SUPPORTED_CONTROL_SURFACES; ++i) {
    if (i < request_msg.lift_drag_control_angles.size()) {
      *out_port_1_wing_control_angles++ =
          request_msg.lift_drag_control_angles[i];
    } else {
      *out_port_1_wing_control_angles++ = 0.0;
    }
  }

  *out_port_2_env_info++ = request_msg.environment_info.geopoint_latitude;
  *out_port_2_env_info++ = request_msg.environment_info.geopoint_longitude;
  *out_port_2_env_info++ = request_msg.environment_info.geopoint_altitude;
  *out_port_2_env_info++ = request_msg.environment_info.gravity_x;
  *out_port_2_env_info++ = request_msg.environment_info.gravity_y;
  *out_port_2_env_info++ = request_msg.environment_info.gravity_z;
  *out_port_2_env_info++ = request_msg.environment_info.temperature;
  *out_port_2_env_info++ = request_msg.environment_info.air_pressure;
  *out_port_2_env_info++ = request_msg.environment_info.air_density;

  *out_port_3_collision_info++ = request_msg.collision_info.has_collided;
  *out_port_3_collision_info++ = request_msg.collision_info.normal_x;
  *out_port_3_collision_info++ = request_msg.collision_info.normal_y;
  *out_port_3_collision_info++ = request_msg.collision_info.normal_z;
  *out_port_3_collision_info++ = request_msg.collision_info.impact_point_x;
  *out_port_3_collision_info++ = request_msg.collision_info.impact_point_y;
  *out_port_3_collision_info++ = request_msg.collision_info.impact_point_z;
  *out_port_3_collision_info++ = request_msg.collision_info.position_x;
  *out_port_3_collision_info++ = request_msg.collision_info.position_y;
  *out_port_3_collision_info++ = request_msg.collision_info.position_z;
  *out_port_3_collision_info++ = request_msg.collision_info.penetration_depth;
}

microsoft::projectairsim::PhysicsModelOutputMessage CopyDataFromInputPorts(
    const double* in_port_0_kinematics) {
  uint64_t dummy_timestamp = 0;  // TODO synchronize time stamps

  const auto pose_position_x = static_cast<float>(*in_port_0_kinematics++);
  const auto pose_position_y = static_cast<float>(*in_port_0_kinematics++);
  const auto pose_position_z = static_cast<float>(*in_port_0_kinematics++);
  const auto pose_orientation_x = static_cast<float>(*in_port_0_kinematics++);
  const auto pose_orientation_y = static_cast<float>(*in_port_0_kinematics++);
  const auto pose_orientation_z = static_cast<float>(*in_port_0_kinematics++);
  const auto pose_orientation_w = static_cast<float>(*in_port_0_kinematics++);
  const auto twist_linear_x = static_cast<float>(*in_port_0_kinematics++);
  const auto twist_linear_y = static_cast<float>(*in_port_0_kinematics++);
  const auto twist_linear_z = static_cast<float>(*in_port_0_kinematics++);
  const auto twist_angular_x = static_cast<float>(*in_port_0_kinematics++);
  const auto twist_angular_y = static_cast<float>(*in_port_0_kinematics++);
  const auto twist_angular_z = static_cast<float>(*in_port_0_kinematics++);
  const auto accels_linear_x = static_cast<float>(*in_port_0_kinematics++);
  const auto accels_linear_y = static_cast<float>(*in_port_0_kinematics++);
  const auto accels_linear_z = static_cast<float>(*in_port_0_kinematics++);
  const auto accels_angular_x = static_cast<float>(*in_port_0_kinematics++);
  const auto accels_angular_y = static_cast<float>(*in_port_0_kinematics++);
  const auto accels_angular_z = static_cast<float>(*in_port_0_kinematics);

  microsoft::projectairsim::PhysicsModelOutputMessage response_msg(
      dummy_timestamp, pose_position_x, pose_position_y, pose_position_z,
      pose_orientation_x, pose_orientation_y, pose_orientation_z,
      pose_orientation_w, twist_linear_x, twist_linear_y, twist_linear_z,
      twist_angular_x, twist_angular_y, twist_angular_z, accels_linear_x,
      accels_linear_y, accels_linear_z, accels_angular_x, accels_angular_y,
      accels_angular_z);

  return response_msg;
}

// -----------------------------------------------------------------------
// Wrapper functions to be called in S-function callbacks

void* SetupRuntimeResourcesWrapper(const std::string& connection_str,
                                   double timeout_sec) {
  // Initialize connection resource
  return reinterpret_cast<void*>(new NngMgr(connection_str, timeout_sec));
}

void StartWrapper(void* nng_mgr, double* out_port_0_wrench_points,
                  double* out_port_1_wing_control_angles,
                  double* out_port_2_env_info,
                  double* out_port_3_collision_info) {
  if (nng_mgr == nullptr) {
    throw std::runtime_error("Matlab model runtime resources are unavailable.");
  }
  reinterpret_cast<NngMgr*>(nng_mgr)->StartListening();

  // Set initialization for output port values to base default values to prevent
  // the Simulink model from using invalid data on the first time step. The
  // first update receives the initial data from the sim.

  // TODO Get real initial data values from the sim at initial connection
  // between Simulink and the sim after the interface is restructured to have a
  // single S-function block for passing data for all components (physics,
  // controllers, etc).
  microsoft::projectairsim::PhysicsModelInputMessage default_msg;

  CopyDataToOutputPorts(default_msg, out_port_0_wrench_points,
                        out_port_1_wing_control_angles, out_port_2_env_info,
                        out_port_3_collision_info);
}

void UpdateWrapper(void* nng_mgr, const double* in_port_0_kinematics,
                   double* out_port_0_wrench_points,
                   double* out_port_1_wing_control_angles,
                   double* out_port_2_env_info,
                   double* out_port_3_collision_info) {
  // This gets called once per major time step to do the block's update.

  if (nng_mgr == nullptr) {
    throw std::runtime_error("Matlab model runtime resources are unavailable.");
  }
  auto nng_mgr_ptr = reinterpret_cast<NngMgr*>(nng_mgr);

  // ------------------------------------------------------------

  // Poll for next request from sim server with new outputs data for the
  // S-function block (wrenches, environment info, collision info)

  std::string request_str = nng_mgr_ptr->RetrieveRequest();

  microsoft::projectairsim::PhysicsModelInputMessage request_msg;
  request_msg.Deserialize(request_str);

  // Set the port output values from the deserialized data
  CopyDataToOutputPorts(request_msg, out_port_0_wrench_points,
                        out_port_1_wing_control_angles, out_port_2_env_info,
                        out_port_3_collision_info);

  // ------------------------------------------------------------

  // Send the S-function block's inputs (kinematics) as a response to sim
  microsoft::projectairsim::PhysicsModelOutputMessage response_msg =
      CopyDataFromInputPorts(in_port_0_kinematics);

  std::string response_str = response_msg.Serialize();

  nng_mgr_ptr->SendResponse(std::move(response_str));
}

void OutputsWrapper(void* nng_mgr) {
  // The model may call `OutputsWrapper()` multiple times per major time step
  // for the solver to converge to it's target accuracy, so we shouldn't do
  // any transport handling here. Instead, do it in `UpdateWrapper()` which is
  // only called once per major time step.
}

void TerminateWrapper(void* nng_mgr) {
  // Close this block's socket at the end of every simulation and Fast Restart.
  if (nng_mgr != nullptr) {
    reinterpret_cast<NngMgr*>(nng_mgr)->StopListening();
  }
}

void CleanUpRuntimeResoucesWrapper(void* nng_mgr) {
  // Clean up at the end of the simulation (not called at the end of every
  // Fast Restart).

  delete reinterpret_cast<NngMgr*>(nng_mgr);
}

void FinalizeRuntimeResourcesWrapper() noexcept { nng_fini(); }
