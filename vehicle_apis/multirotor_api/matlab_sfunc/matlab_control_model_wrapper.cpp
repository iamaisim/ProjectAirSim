// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include "matlab_control_model_wrapper.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

#include "message/common_message_utils.hpp"
#include "message/control_model_input_message.hpp"
#include "message/control_model_output_message.hpp"
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
    const microsoft::projectairsim::ControlModelInputMessage& request_msg,
    double* out_port_0_kinematics, double* out_port_1_airspeed,
    double* out_port_2_barometer, double* out_port_3_imu,
    double* out_port_4_magnetometer, double* out_port_5_distance,
    double* out_port_6_gps) {
  *out_port_0_kinematics++ = request_msg.kinematics.pose_position_x;
  *out_port_0_kinematics++ = request_msg.kinematics.pose_position_y;
  *out_port_0_kinematics++ = request_msg.kinematics.pose_position_z;
  *out_port_0_kinematics++ = request_msg.kinematics.pose_orientation_x;
  *out_port_0_kinematics++ = request_msg.kinematics.pose_orientation_y;
  *out_port_0_kinematics++ = request_msg.kinematics.pose_orientation_z;
  *out_port_0_kinematics++ = request_msg.kinematics.pose_orientation_w;
  *out_port_0_kinematics++ = request_msg.kinematics.twist_linear_x;
  *out_port_0_kinematics++ = request_msg.kinematics.twist_linear_y;
  *out_port_0_kinematics++ = request_msg.kinematics.twist_linear_z;
  *out_port_0_kinematics++ = request_msg.kinematics.twist_angular_x;
  *out_port_0_kinematics++ = request_msg.kinematics.twist_angular_y;
  *out_port_0_kinematics++ = request_msg.kinematics.twist_angular_z;
  *out_port_0_kinematics++ = request_msg.kinematics.accels_linear_x;
  *out_port_0_kinematics++ = request_msg.kinematics.accels_linear_y;
  *out_port_0_kinematics++ = request_msg.kinematics.accels_linear_z;
  *out_port_0_kinematics++ = request_msg.kinematics.accels_angular_x;
  *out_port_0_kinematics++ = request_msg.kinematics.accels_angular_y;
  *out_port_0_kinematics++ = request_msg.kinematics.accels_angular_z;

  *out_port_1_airspeed++ = request_msg.airspeed;

  *out_port_2_barometer++ = request_msg.barometer.altitude;
  *out_port_2_barometer++ = request_msg.barometer.pressure;
  *out_port_2_barometer++ = request_msg.barometer.qnh;

  *out_port_3_imu++ = request_msg.imu.orientation_x;
  *out_port_3_imu++ = request_msg.imu.orientation_y;
  *out_port_3_imu++ = request_msg.imu.orientation_z;
  *out_port_3_imu++ = request_msg.imu.orientation_w;
  *out_port_3_imu++ = request_msg.imu.angular_velocity_x;
  *out_port_3_imu++ = request_msg.imu.angular_velocity_y;
  *out_port_3_imu++ = request_msg.imu.angular_velocity_z;
  *out_port_3_imu++ = request_msg.imu.linear_acceleration_x;
  *out_port_3_imu++ = request_msg.imu.linear_acceleration_y;
  *out_port_3_imu++ = request_msg.imu.linear_acceleration_z;

  *out_port_4_magnetometer++ = request_msg.magnetometer.x;
  *out_port_4_magnetometer++ = request_msg.magnetometer.y;
  *out_port_4_magnetometer++ = request_msg.magnetometer.z;

  *out_port_5_distance++ = request_msg.distance;

  *out_port_6_gps++ = request_msg.gps.latitude;
  *out_port_6_gps++ = request_msg.gps.longitude;
  *out_port_6_gps++ = request_msg.gps.altitude;
  *out_port_6_gps++ = request_msg.gps.velocity_x;
  *out_port_6_gps++ = request_msg.gps.velocity_y;
  *out_port_6_gps++ = request_msg.gps.velocity_z;
}

microsoft::projectairsim::ControlModelOutputMessage CopyDataFromInputPorts(
    const double* in_port_0_control) {
  uint64_t dummy_timestamp = 0;  // TODO synchronize time stamps

  std::vector<float> control_values;
  for (auto i = 0; i < NUM_SUPPORTED_CONTROL_VALUES; i++) {
    control_values.push_back(*in_port_0_control++);
  }

  microsoft::projectairsim::ControlModelOutputMessage response_msg(
      dummy_timestamp, control_values);

  return response_msg;
}

// -----------------------------------------------------------------------
// Wrapper functions to be called in S-function callbacks

void* SetupRuntimeResourcesWrapper(const std::string& connection_str,
                                   double timeout_sec) {
  // Initialize connection resource
  return reinterpret_cast<void*>(new NngMgr(connection_str, timeout_sec));
}

void StartWrapper(void* nng_mgr, double* out_port_0_kinematics,
                  double* out_port_1_airspeed, double* out_port_2_barometer,
                  double* out_port_3_imu, double* out_port_4_magnetometer,
                  double* out_port_5_distance, double* out_port_6_gps) {
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
  microsoft::projectairsim::ControlModelInputMessage default_msg;

  CopyDataToOutputPorts(default_msg, out_port_0_kinematics, out_port_1_airspeed,
                        out_port_2_barometer, out_port_3_imu,
                        out_port_4_magnetometer, out_port_5_distance,
                        out_port_6_gps);
}

void UpdateWrapper(void* nng_mgr, const double* in_port_0_control,
                   double* out_port_0_kinematics, double* out_port_1_airspeed,
                   double* out_port_2_barometer, double* out_port_3_imu,
                   double* out_port_4_magnetometer, double* out_port_5_distance,
                   double* out_port_6_gps) {
  // This gets called once per major time step to do the block's update.

  if (nng_mgr == nullptr) {
    throw std::runtime_error("Matlab model runtime resources are unavailable.");
  }
  auto nng_mgr_ptr = reinterpret_cast<NngMgr*>(nng_mgr);

  // ------------------------------------------------------------

  // Poll for next request from sim server with new outputs data for the
  // S-function block (sensor data)

  std::string request_str = nng_mgr_ptr->RetrieveRequest();

  microsoft::projectairsim::ControlModelInputMessage request_msg;
  request_msg.Deserialize(request_str);

  // Set the port output values from the deserialized data
  CopyDataToOutputPorts(request_msg, out_port_0_kinematics, out_port_1_airspeed,
                        out_port_2_barometer, out_port_3_imu,
                        out_port_4_magnetometer, out_port_5_distance,
                        out_port_6_gps);

  // ------------------------------------------------------------

  // Send the S-function block's inputs (control signals) as a response to sim
  microsoft::projectairsim::ControlModelOutputMessage response_msg =
      CopyDataFromInputPorts(in_port_0_control);

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
