// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MATLAB_PHYSICS_MODEL_WRAPPER_HPP
#define MATLAB_PHYSICS_MODEL_WRAPPER_HPP

#include <string>

#define NUM_SUPPORTED_ROTORS 8
#define NUM_SUPPORTED_CONTROL_SURFACES 8

void* SetupRuntimeResourcesWrapper(const std::string& connection_str);

void StartWrapper(void* nng_mgr, double* out_port_0_wrench_points,
                  double* out_port_1_wing_control_angles,
                  double* out_port_2_env_info,
                  double* out_port_3_collision_info);

void UpdateWrapper(void* nng_mgr, const double* in_port_0_kinematics,
                   double* out_port_0_wrench_points,
                   double* out_port_1_wing_control_angles,
                   double* out_port_2_env_info,
                   double* out_port_3_collision_info);

void OutputsWrapper(void* nng_mgr);

void TerminateWrapper(void* nng_mgr);

void CleanUpRuntimeResoucesWrapper(void* nng_mgr);

#endif  // MATLAB_PHYSICS_MODEL_WRAPPER_HPP
