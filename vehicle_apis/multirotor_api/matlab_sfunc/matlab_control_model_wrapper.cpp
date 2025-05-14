// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "matlab_control_model_wrapper.hpp"

#include <iostream>

#include "message/common_message_utils.hpp"
#include "message/control_model_input_message.hpp"
#include "message/control_model_output_message.hpp"
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

void* SetupRuntimeResourcesWrapper(const std::string& connection_str) {
  // Initialize connection resource
  return reinterpret_cast<void*>(new NngMgr(connection_str));
}

void StartWrapper(void* nng_mgr, double* out_port_0_kinematics,
                  double* out_port_1_airspeed, double* out_port_2_barometer,
                  double* out_port_3_imu, double* out_port_4_magnetometer,
                  double* out_port_5_distance, double* out_port_6_gps) {
  // Set initialization for output port values to base default values to prevent
  // the Simulink model from using invalid data on the first time step. We
  // connect to the sim and get the first real data from it the first block
  // update step.

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

  auto nng_mgr_ptr = reinterpret_cast<NngMgr*>(nng_mgr);

  // ------------------------------------------------------------

  if (!nng_mgr_ptr->connected_) {
    // Open NNG connection
    reinterpret_cast<NngMgr*>(nng_mgr)->StartListening();
    nng_mgr_ptr->connected_ = true;
  }

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
  // Clean up any in-process requests when the model is terminated (called at
  // the end of every Fast Restart).
}

void CleanUpRuntimeResoucesWrapper(void* nng_mgr) {
  // Clean up at the end of the simulation (not called at the end of every
  // Fast Restart).

  delete reinterpret_cast<NngMgr*>(nng_mgr);
}
