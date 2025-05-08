// Copyright (C) Microsoft Corporation. All rights reserved.

#include "matlab_physics_model_wrapper.hpp"

#include <iostream>

#include "message/common_message_utils.hpp"
#include "message/physics_model_input_message.hpp"
#include "message/physics_model_output_message.hpp"
#include "msgpack.hpp"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"

// -----------------------------------------------------------------------
// NngMgr class for managing socket connection with the sim

class NngMgr {
 public:
  NngMgr(const std::string& addr);

  ~NngMgr();

  void StartListening();

  void SendResponse(std::string response_str);

  std::string RetrieveRequest();

  bool connected_ = false;

 private:
  nng_socket socket_ = NNG_SOCKET_INITIALIZER;
  std::string addr_;
};

NngMgr::NngMgr(const std::string& addr) : addr_(addr) {}

NngMgr::~NngMgr() {
  std::cout << "Closing the connection..." << std::endl;

  int rv = nng_close(socket_);

  if (rv != 0) {
    std::cout << "nng_close() return value: " << rv << std::endl;
  }

  nng_fini();  // terminate NNG library completely to release all resources
               // (needed to fully unlock the built library file for
               // re-building without having to close Matlab)
}

void NngMgr::StartListening() {
  std::cout << "Opening connection with sim server..." << std::endl;

  // Create a Response socket to listen for Matlab client requests
  int rv = nng_rep0_open(&socket_);

  if (rv != 0) {
    std::cout << "nng_rep0_open() return value: " << rv << std::endl;
  }

  // Set socket options
  // rv = nng_setopt_ms(nng_socket_, NNG_OPT_RECVTIMEO, 5000);

  // Start socket listening
  rv = nng_listen(socket_, addr_.c_str(), NULL /*create a new listener*/,
                  0 /*no special flags*/);

  if (rv != 0) {
    std::cout << "nng_listen() return value: " << rv << std::endl;
  }
}

void NngMgr::SendResponse(std::string response_str) {
  // std::cout << "Sending response..." << std::endl;

  // Send response data back
  char* response_data_ptr = const_cast<char*>(response_str.data());
  int rv = nng_send(socket_, response_data_ptr, response_str.length(), 0);

  if (rv != 0) {
    std::cout << "nng_send() return value: " << rv << std::endl;
  }
}

std::string NngMgr::RetrieveRequest() {
  // Block until receiving a new request or timeout
  // std::cout << "Waiting to receive new request..." << std::endl;

  char* msg_data_buf = nullptr;
  size_t msg_data_len;

  int rv = nng_recv(
      socket_, &msg_data_buf, &msg_data_len,
      NNG_FLAG_ALLOC /* have NNG allocate the necessary buffer memory*/);

  if (rv != 0) {
    std::cout << "nng_recv() return value: " << rv
              << " length = " << msg_data_len << std::endl;
  }

  // Copy the data from the buffer to a string to deserialize (must set the
  // length manually to avoid stopping at '\0' chars that it may contain)
  std::string request_str;
  request_str.resize(msg_data_len);
  request_str.assign(msg_data_buf, msg_data_len);

  // Free the NNG-allocated reply buffer memory
  nng_free(msg_data_buf, msg_data_len);

  return request_str;
}

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

void* SetupRuntimeResourcesWrapper(const std::string& connection_str) {
  // Initialize connection resource
  return reinterpret_cast<void*>(new NngMgr(connection_str));
}

void StartWrapper(void* nng_mgr, double* out_port_0_wrench_points,
                  double* out_port_1_wing_control_angles,
                  double* out_port_2_env_info,
                  double* out_port_3_collision_info) {
  // Set initialization for output port values to base default values to prevent
  // the Simulink model from using invalid data on the first time step. We
  // connect to the sim and get the first real data from it the first block
  // update step.

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

  auto nng_mgr_ptr = reinterpret_cast<NngMgr*>(nng_mgr);

  // ------------------------------------------------------------

  if (!nng_mgr_ptr->connected_) {
    // Open NNG connection
    reinterpret_cast<NngMgr*>(nng_mgr)->StartListening();
    nng_mgr_ptr->connected_ = true;
  }

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
  // Clean up any in-process requests when the model is terminated (called at
  // the end of every Fast Restart).
}

void CleanUpRuntimeResoucesWrapper(void* nng_mgr) {
  // Clean up at the end of the simulation (not called at the end of every
  // Fast Restart).

  delete reinterpret_cast<NngMgr*>(nng_mgr);
}
