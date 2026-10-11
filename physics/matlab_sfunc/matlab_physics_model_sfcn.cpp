// Copyright (C) Microsoft Corporation.
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#define S_FUNCTION_LEVEL 2
#define S_FUNCTION_NAME matlab_physics_model_sfcn

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "matlab_physics_model_wrapper.hpp"
#include "simstruc.h"

// Define block's parameters
#define HOST_NAME_P 0
#define PORT_NUM_P 1
#define TIME_STEP_P 2
#define NUM_PRMS 3

#define NUM_INPUTS 1

// Kinematics: 3 position + 4 orientation + 3 twist_linear + 3 twist angular +
//             3 accels_linear + 3 accels_angular = 19 values
#define IN_0_WIDTH 19

#define NUM_OUTPUTS 4

// Rotor wrenches: 9 values per rotor wrench (3 for XYZ force, 3 for XYZ torque,
// 3 for XYZ position)
#define OUT_0_WIDTH (NUM_SUPPORTED_ROTORS * 9)

// Wing control surface angles: 1 control value per surfaces
#define OUT_1_WIDTH NUM_SUPPORTED_CONTROL_SURFACES

// Environment info: 3 geopoint + 3 gravity + 1 temp + 1 air pressure +
//                   1 air density = 9 values
#define OUT_2_WIDTH 9

// Collision info: 1 has_collided + 3 normal + 3 impact point + 3 position +
//                 1 penetration_depth = 11 values
#define OUT_3_WIDTH 11

// Alias for getting the NNG pointer saved in PWork index 0
#define GET_NNG_PTR(S) ssGetPWorkValue(S, 0)
#define GET_ERROR_PTR(S) static_cast<std::string*>(ssGetPWorkValue(S, 1))

// Helper type to wrap C-strings allocated by mxArrayToString() as unique ptr
// with the mxFree() custom deleter
auto MxDeleter = [](char* ptr) { mxFree(ptr); };
using MxCharUniquePtr = std::unique_ptr<char, decltype(MxDeleter)>;

static void SetRuntimeError(SimStruct* S, const char* message) noexcept {
  try {
    auto error = GET_ERROR_PTR(S);
    if (error != nullptr) {
      *error = message;
      ssSetErrorStatus(S, error->c_str());
      return;
    }
  } catch (...) {
  }
  ssSetErrorStatus(S, "Matlab model runtime resource failure.");
}

// --------------------------------------------------------------------------

#define MDL_INITIAL_SIZES
// Method to configure the S-Function block.
// https://www.mathworks.com/help/simulink/sfg/mdlinitializesizes.html
static void mdlInitializeSizes(SimStruct* S) {
  std::cout << "Initializing Matlab physics S-function block..." << std::endl;
  ssSetNumSFcnParams(S, NUM_PRMS);

  ssSetSFcnParamTunable(S, HOST_NAME_P, false);
  ssSetSFcnParamTunable(S, PORT_NUM_P, false);
  ssSetSFcnParamTunable(S, TIME_STEP_P, false);

  // Setup input ports
  if (!ssSetNumInputPorts(S, NUM_INPUTS)) return;

  // Input port 0 settings
  ssSetInputPortWidth(S, 0, IN_0_WIDTH);
  ssSetInputPortDataType(S, 0, SS_DOUBLE);
  ssSetInputPortComplexSignal(S, 0, COMPLEX_NO);
  // Set input port data to be contiguous memory for array pointer access
  ssSetInputPortRequiredContiguous(S, 0, 1);

  // Setup output ports
  if (!ssSetNumOutputPorts(S, NUM_OUTPUTS)) return;

  // Output port 0 settings
  ssSetOutputPortWidth(S, 0, OUT_0_WIDTH);
  ssSetOutputPortDataType(S, 0, SS_DOUBLE);
  ssSetOutputPortComplexSignal(S, 0, COMPLEX_NO);

  // Output port 1 settings
  ssSetOutputPortWidth(S, 1, OUT_1_WIDTH);
  ssSetOutputPortDataType(S, 1, SS_DOUBLE);
  ssSetOutputPortComplexSignal(S, 1, COMPLEX_NO);

  // Output port 2 settings
  ssSetOutputPortWidth(S, 2, OUT_2_WIDTH);
  ssSetOutputPortDataType(S, 2, SS_DOUBLE);
  ssSetOutputPortComplexSignal(S, 2, COMPLEX_NO);

  // Output port 3 settings
  ssSetOutputPortWidth(S, 3, OUT_3_WIDTH);
  ssSetOutputPortDataType(S, 3, SS_DOUBLE);
  ssSetOutputPortComplexSignal(S, 3, COMPLEX_NO);

  // Save the socket manager and this block's persistent error message.
  ssSetNumPWork(S, 2);

  // Setup sample times
  ssSetNumSampleTimes(S, 1);

  // Setup general model options
  ssSetOptions(S, SS_OPTION_EXCEPTION_FREE_CODE);
}

// --------------------------------------------------------------------------

#define MDL_INITIALIZE_SAMPLE_TIMES
// Method to initialize the block's sample time.
// https://www.mathworks.com/help/simulink/sfg/mdlinitializesampletimes.html
static void mdlInitializeSampleTimes(SimStruct* S) {
  if (ssIsVariableStepSolver(S)) {
    ssSetErrorStatus(
        S,
        "Error: Variable-step solver detected. Please switch to fixed-step.");
    return;
  }

  std::cout << "Initializing Matlab physics sample times..." << std::endl;
  double time_step = *mxGetDoubles(ssGetSFcnParam(S, TIME_STEP_P));

  if (time_step <= 0) {
    std::cout << "Detected non-positive value for 'AirSim time step'. "
                 "Inheriting time step from model..."
              << std::endl;
    time_step = INHERITED_SAMPLE_TIME;
  }

  ssSetSampleTime(S, 0, time_step);
  ssSetOffsetTime(S, 0, 0.0);
  ssSetModelReferenceSampleTimeDefaultInheritance(S);
}

// --------------------------------------------------------------------------

#define MDL_SETUP_RUNTIME_RESOURCES
// Method called at the beginning of one or multiple simulations.
// https://www.mathworks.com/help/simulink/sfg/mdlsetupruntimeresources.html
void mdlSetupRuntimeResources(SimStruct* S) {
  std::cout << "Setting up Matlab physics runtime resources..." << std::endl;

  try {
    std::unique_ptr<std::string> error(new std::string);
    ssSetPWorkValue(S, 1, error.release());
    MxCharUniquePtr host_name(mxArrayToString(ssGetSFcnParam(S, HOST_NAME_P)),
                             MxDeleter);
    MxCharUniquePtr port_num(mxArrayToString(ssGetSFcnParam(S, PORT_NUM_P)),
                            MxDeleter);

    if (host_name == nullptr || port_num == nullptr) {
      throw std::runtime_error("Failed to read the Matlab connection parameters.");
    }

    std::string connection_str = "tcp://";
    connection_str += host_name.get();
    connection_str += ":";
    connection_str += port_num.get();

    // Allocate and store pointer to NNG using wrapper in PWork index 0
    void* nng_mgr = SetupRuntimeResourcesWrapper(connection_str);
    ssSetPWorkValue(S, 0, nng_mgr);
  } catch (const std::exception& e) {
    SetRuntimeError(S, e.what());
  } catch (...) {
    SetRuntimeError(S, "Unexpected Matlab model setup failure.");
  }
}

// --------------------------------------------------------------------------

#define MDL_START
// Method called at the beginning of every simulation, including Fast Restarts.
// https://www.mathworks.com/help/simulink/sfg/mdlstart.html
static void mdlStart(SimStruct* S) {
  std::cout << "Starting Matlab physics..." << std::endl;

  // Get pointers to initialize the output port data
  auto out_port_0_wrench_points =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 0));
  auto out_port_1_wing_control_angles =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 1));
  auto out_port_2_env_info =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 2));
  auto out_port_3_collision_info =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 3));

  try {
    StartWrapper(GET_NNG_PTR(S), out_port_0_wrench_points,
                 out_port_1_wing_control_angles, out_port_2_env_info,
                 out_port_3_collision_info);
  } catch (const std::exception& e) {
    SetRuntimeError(S, e.what());
  } catch (...) {
    SetRuntimeError(S, "Unexpected Matlab model runtime failure.");
  }
}

// --------------------------------------------------------------------------

#define MDL_UPDATE
// Method called at each step to update the block's internal state.
// https://www.mathworks.com/help/simulink/sfg/mdlupdate.html
static void mdlUpdate(SimStruct* S, int_T tid) {
  // std::cout << "Update Matlab physics model..." << std::endl;

  // Get input data pointers from contiguous input port 0
  const double* in_port_0_kinematics = ssGetInputPortRealSignal(S, 0);

  // Get pointers to write the output port data
  auto out_port_0_wrench_points =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 0));
  auto out_port_1_wing_control_angles =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 1));
  auto out_port_2_env_info =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 2));
  auto out_port_3_collision_info =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 3));

  try {
    UpdateWrapper(GET_NNG_PTR(S), in_port_0_kinematics, out_port_0_wrench_points,
                  out_port_1_wing_control_angles, out_port_2_env_info,
                  out_port_3_collision_info);
  } catch (const std::exception& e) {
    SetRuntimeError(S, e.what());
  } catch (...) {
    SetRuntimeError(S, "Unexpected Matlab model runtime failure.");
  }
}

// --------------------------------------------------------------------------

#define MDL_OUTPUTS
// Method called at each step to calculate the block's output signals
// https://www.mathworks.com/help/simulink/sfg/mdloutputs.html
static void mdlOutputs(SimStruct* S, int_T tid) {
  // std::cout << "Calculate Matlab physics outputs..." << std::endl;

  try {
    OutputsWrapper(GET_NNG_PTR(S));
  } catch (const std::exception& e) {
    SetRuntimeError(S, e.what());
  } catch (...) {
    SetRuntimeError(S, "Unexpected Matlab model output failure.");
  }
}

// --------------------------------------------------------------------------

// Called at the end of every simulation, including every Fast Restart.
// https://www.mathworks.com/help/simulink/sfg/mdlterminate.html
static void mdlTerminate(SimStruct* S) {
  std::cout << "Terminating Matlab physics..." << std::endl;
  TerminateWrapper(GET_NNG_PTR(S));
}

// --------------------------------------------------------------------------

#define MDL_CLEANUP_RUNTIME_RESOURCES
// Called at the end of one or multiple simulations. This method is NOT called
// at the end of each subsequent Fast Restart.
// https://www.mathworks.com/help/simulink/sfg/mdlcleanupruntimeresources.html
static void mdlCleanupRuntimeResources(SimStruct* S) {
  std::cout << "Cleaning up Matlab physics resources..." << std::endl;
  void* nng_mgr = GET_NNG_PTR(S);
  ssSetPWorkValue(S, 0, nullptr);
  CleanUpRuntimeResoucesWrapper(nng_mgr);
  ssSetErrorStatus(S, nullptr);
  delete GET_ERROR_PTR(S);
  ssSetPWorkValue(S, 1, nullptr);
}

// --------------------------------------------------------------------------

#ifdef MATLAB_MEX_FILE /* Is this file being compiled as a MEX-file? */
#define PROCESS_MEX_SFUNCTION_EVERY_CALL
static bool ProcessMexSfunctionEveryCall(int_T, mxArray**, int_T,
                                        const mxArray**) {
  static bool module_locked = false;
  if (!module_locked) {
    // MEX APIs require the entry-point context, before Simulink callbacks.
    mexAtExit(FinalizeRuntimeResourcesWrapper);
    mexLock();
    module_locked = true;
  }
  return false;
}
#include "simulink.c"  /* MEX-file interface mechanism */
#else
#include "cg_sfun.h" /* Code generation registration function */
#endif
