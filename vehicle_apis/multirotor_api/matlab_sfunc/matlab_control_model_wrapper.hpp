// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MATLAB_CONTROL_MODEL_WRAPPER_HPP
#define MATLAB_CONTROL_MODEL_WRAPPER_HPP

#include <string>

#define NUM_SUPPORTED_CONTROL_VALUES 16

void* SetupRuntimeResourcesWrapper(const std::string& connection_str);

void StartWrapper(void* nng_mgr, double* out_port_0_kinematics,
                  double* out_port_1_airspeed, double* out_port_2_barometer,
                  double* out_port_3_imu, double* out_port_4_magnetometer,
                  double* out_port_5_distance, double* out_port_6_gps);

void UpdateWrapper(void* nng_mgr, const double* in_port_0_control,
                   double* out_port_0_kinematics, double* out_port_1_airspeed,
                   double* out_port_2_barometer, double* out_port_3_imu,
                   double* out_port_4_magnetometer, double* out_port_5_distance,
                   double* out_port_6_gps);

void OutputsWrapper(void* nng_mgr);

void TerminateWrapper(void* nng_mgr);

void CleanUpRuntimeResoucesWrapper(void* nng_mgr);

#endif  // MATLAB_CONTROL_MODEL_WRAPPER_HPP
