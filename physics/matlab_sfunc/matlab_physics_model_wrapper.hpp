// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#ifndef MATLAB_PHYSICS_MODEL_WRAPPER_HPP
#define MATLAB_PHYSICS_MODEL_WRAPPER_HPP

#include <string>

#define NUM_SUPPORTED_ROTORS 8
#define NUM_SUPPORTED_CONTROL_SURFACES 8

// Bound each transport operation in wall-clock seconds, including the legacy
// three-parameter S-function form. Indefinite waits prevent Simulink Stop.
constexpr double kDefaultTransportTimeoutSec = 5.0;

// timeout_sec must be positive and finite.
void* SetupRuntimeResourcesWrapper(const std::string& connection_str,
                                   double timeout_sec);

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

void FinalizeRuntimeResourcesWrapper() noexcept;

#endif  // MATLAB_PHYSICS_MODEL_WRAPPER_HPP
