// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#define S_FUNCTION_LEVEL 2
#define S_FUNCTION_NAME matlab_control_model_sfcn

#include <iostream>
#include <memory>
#include <string>

#include "matlab_control_model_wrapper.hpp"
#include "simstruc.h"

// Define block's parameters
#define HOST_NAME_P 0
#define PORT_NUM_P 1
#define TIME_STEP_P 2
#define NUM_PRMS 3

#define NUM_INPUTS 1

// Actuator control values: supporting a max of 16
#define IN_0_WIDTH 16

#define NUM_OUTPUTS 7

// Ground Truth Kinematics: 3 position + 4 orientation + 3 twist_linear +
//            3 twist angular + 3 accels_linear + 3 accels_angular = 19 values
#define OUT_0_WIDTH 19

// Airspeed Sensor
#define OUT_1_WIDTH 1

// Barometer Sensor
#define OUT_2_WIDTH 3

// IMU Sensor
#define OUT_3_WIDTH 10

// Magnetometer Sensor
#define OUT_4_WIDTH 3

// Distance Sensor
#define OUT_5_WIDTH 1

// GPS Sensor
#define OUT_6_WIDTH 6

// Alias for getting the NNG pointer saved in PWork index 0
#define GET_NNG_PTR(S) ssGetPWorkValue(S, 0)

// Helper type to wrap C-strings allocated by mxArrayToString() as unique ptr
// with the mxFree() custom deleter
auto MxDeleter = [](char* ptr) { mxFree(ptr); };
using MxCharUniquePtr = std::unique_ptr<char, decltype(MxDeleter)>;

// --------------------------------------------------------------------------

#define MDL_INITIAL_SIZES
// Method to configure the S-Function block.
// https://www.mathworks.com/help/simulink/sfg/mdlinitializesizes.html
static void mdlInitializeSizes(SimStruct* S) {
  std::cout << "Initializing Matlab control S-function block..." << std::endl;
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

  // Output port 4 settings
  ssSetOutputPortWidth(S, 4, OUT_4_WIDTH);
  ssSetOutputPortDataType(S, 4, SS_DOUBLE);
  ssSetOutputPortComplexSignal(S, 4, COMPLEX_NO);

  // Output port 5 settings
  ssSetOutputPortWidth(S, 5, OUT_5_WIDTH);
  ssSetOutputPortDataType(S, 5, SS_DOUBLE);
  ssSetOutputPortComplexSignal(S, 5, COMPLEX_NO);

  // Output port 6 settings
  ssSetOutputPortWidth(S, 6, OUT_6_WIDTH);
  ssSetOutputPortDataType(S, 6, SS_DOUBLE);
  ssSetOutputPortComplexSignal(S, 6, COMPLEX_NO);

  // Set up PWork vector to save pointers
  ssSetNumPWork(S, 1);

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

  std::cout << "Initializing Matlab control sample times..." << std::endl;
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
  std::cout << "Setting up Matlab control runtime resources..." << std::endl;

  MxCharUniquePtr host_name(mxArrayToString(ssGetSFcnParam(S, HOST_NAME_P)),
                            MxDeleter);
  MxCharUniquePtr port_num(mxArrayToString(ssGetSFcnParam(S, PORT_NUM_P)),
                           MxDeleter);

  std::string connection_str = "tcp://";
  connection_str += host_name.get();
  connection_str += ":";
  connection_str += port_num.get();

  // Allocate and store pointer to NNG using wrapper in PWork index 0
  void* nng_mgr = SetupRuntimeResourcesWrapper(connection_str);
  ssSetPWorkValue(S, 0, nng_mgr);
}

// --------------------------------------------------------------------------

#define MDL_START
// Method called at the beginning of every simulation, including Fast Restarts.
// https://www.mathworks.com/help/simulink/sfg/mdlstart.html
static void mdlStart(SimStruct* S) {
  std::cout << "Starting Matlab control..." << std::endl;

  // Get pointers to initialize the output port data
  auto out_port_0_kinematics =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 0));
  auto out_port_1_airspeed =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 1));
  auto out_port_2_barometer =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 2));
  auto out_port_3_imu = static_cast<double*>(ssGetOutputPortRealSignal(S, 3));
  auto out_port_4_magnetometer =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 4));
  auto out_port_5_distance =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 5));
  auto out_port_6_gps = static_cast<double*>(ssGetOutputPortRealSignal(S, 6));

  StartWrapper(GET_NNG_PTR(S), out_port_0_kinematics, out_port_1_airspeed,
               out_port_2_barometer, out_port_3_imu, out_port_4_magnetometer,
               out_port_5_distance, out_port_6_gps);
}

// --------------------------------------------------------------------------

#define MDL_UPDATE
// Method called at each step to update the block's internal state.
// https://www.mathworks.com/help/simulink/sfg/mdlupdate.html
static void mdlUpdate(SimStruct* S, int_T tid) {
  // std::cout << "Update Matlab control model..." << std::endl;

  // Get input data pointers from contiguous input port 0
  const double* in_port_0_control = ssGetInputPortRealSignal(S, 0);

  // Get pointers to write the output port data
  auto out_port_0_kinematics =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 0));
  auto out_port_1_airspeed =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 1));
  auto out_port_2_barometer =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 2));
  auto out_port_3_imu = static_cast<double*>(ssGetOutputPortRealSignal(S, 3));
  auto out_port_4_magnetometer =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 4));
  auto out_port_5_distance =
      static_cast<double*>(ssGetOutputPortRealSignal(S, 5));
  auto out_port_6_gps = static_cast<double*>(ssGetOutputPortRealSignal(S, 6));

  UpdateWrapper(GET_NNG_PTR(S), in_port_0_control, out_port_0_kinematics,
                out_port_1_airspeed, out_port_2_barometer, out_port_3_imu,
                out_port_4_magnetometer, out_port_5_distance, out_port_6_gps);
}

// --------------------------------------------------------------------------

#define MDL_OUTPUTS
// Method called at each step to calculate the block's output signals
// https://www.mathworks.com/help/simulink/sfg/mdloutputs.html
static void mdlOutputs(SimStruct* S, int_T tid) {
  // std::cout << "Calculate Matlab control outputs..." << std::endl;

  try {
    OutputsWrapper(GET_NNG_PTR(S));
  } catch (std::exception& e) {
    static std::string errstr(e.what());
    ssSetErrorStatus(S, errstr.c_str());
    return;
  }
}

// --------------------------------------------------------------------------

// Called at the end of every simulation, including every Fast Restart.
// https://www.mathworks.com/help/simulink/sfg/mdlterminate.html
static void mdlTerminate(SimStruct* S) {
  std::cout << "Terminating Matlab control..." << std::endl;
  TerminateWrapper(GET_NNG_PTR(S));
}

// --------------------------------------------------------------------------

#define MDL_CLEANUP_RUNTIME_RESOURCES
// Called at the end of one or multiple simulations. This method is NOT called
// at the end of each subsequent Fast Restart.
// https://www.mathworks.com/help/simulink/sfg/mdlcleanupruntimeresources.html
static void mdlCleanupRuntimeResources(SimStruct* S) {
  std::cout << "Cleaning up Matlab control resources..." << std::endl;
  CleanUpRuntimeResoucesWrapper(GET_NNG_PTR(S));
}

// --------------------------------------------------------------------------

#ifdef MATLAB_MEX_FILE /* Is this file being compiled as a MEX-file? */
#include "simulink.c"  /* MEX-file interface mechanism */
#else
#include "cg_sfun.h" /* Code generation registration function */
#endif
