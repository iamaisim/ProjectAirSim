// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#define S_FUNCTION_LEVEL 2
#define S_FUNCTION_NAME matlab_vtol_control_model_sfcn

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "message/vtol_control_model_message.hpp"
#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"
#include "simstruc.h"

using namespace microsoft::projectairsim;

namespace {
constexpr int kParameterCount =
    4;  // Host, port, sample period, N-by-3 effector cell array.
constexpr int kOutputWidths[] = {19, 1, 3, 10, 3, 1, 6, 12};

#ifdef MATLAB_MEX_FILE
void FinalizeTransport() { nng_fini(); }
#endif

std::string GetString(const mxArray* value) {
  if (!value || !mxIsChar(value))
    throw std::invalid_argument("Expected a character vector");
  char* text = mxArrayToString(value);
  if (!text) throw std::runtime_error("Cannot read character vector");
  std::string result(text);
  mxFree(text);
  return result;
}

std::vector<ExternalEffector> GetEffectors(const mxArray* value) {
  if (!mxIsCell(value) || mxGetN(value) != 3 || mxGetM(value) == 0) {
    throw std::invalid_argument(
        "Effectors must be an N-by-3 cell array of id, kind, units");
  }
  const size_t count = mxGetM(value);
  std::vector<ExternalEffector> result(count);
  for (size_t i = 0; i < count; ++i) {
    result[i].id = GetString(mxGetCell(value, i));
    result[i].kind = GetString(mxGetCell(value, i + count));
    result[i].units = GetString(mxGetCell(value, i + 2 * count));
    if (result[i].id.empty() || result[i].kind.empty() ||
        result[i].units.empty()) {
      throw std::invalid_argument("Effector metadata must not be empty");
    }
  }
  return result;
}

struct Transport {
  nng_socket socket = NNG_SOCKET_INITIALIZER;
  bool open = false;
  std::vector<ExternalEffector> effectors;
  ~Transport() {
    if (open) nng_close(socket);
  }
};

void ReportError(SimStruct* s, const std::exception& error) {
  mexPrintf("External controller: %s\n", error.what());
  ssSetErrorStatus(s,
                   "External controller transport or protocol error; see "
                   "MATLAB Command Window");
}

void CopyOutputs(SimStruct* s, const VTOLControlModelInputMessage& input) {
  const auto& k = input.state.kinematics;
  const auto& b = input.state.barometer;
  const auto& i = input.state.imu;
  const auto& m = input.state.magnetometer;
  const auto& g = input.state.gps;
  const auto& c = input.setpoint;
  const double kinematics[] = {
      k.pose_position_x,    k.pose_position_y,    k.pose_position_z,
      k.pose_orientation_x, k.pose_orientation_y, k.pose_orientation_z,
      k.pose_orientation_w, k.twist_linear_x,     k.twist_linear_y,
      k.twist_linear_z,     k.twist_angular_x,    k.twist_angular_y,
      k.twist_angular_z,    k.accels_linear_x,    k.accels_linear_y,
      k.accels_linear_z,    k.accels_angular_x,   k.accels_angular_y,
      k.accels_angular_z};
  const double airspeed[] = {input.state.airspeed};
  const double barometer[] = {b.altitude, b.pressure, b.qnh};
  const double imu[] = {i.orientation_x,         i.orientation_y,
                        i.orientation_z,         i.orientation_w,
                        i.angular_velocity_x,    i.angular_velocity_y,
                        i.angular_velocity_z,    i.linear_acceleration_x,
                        i.linear_acceleration_y, i.linear_acceleration_z};
  const double magnetometer[] = {m.x, m.y, m.z};
  const double distance[] = {input.state.distance};
  const double gps[] = {g.longitude,  g.latitude,   g.altitude,
                        g.velocity_x, g.velocity_y, g.velocity_z};
  const double command[] = {double(c.mode),
                            double(c.frame),
                            c.x,
                            c.y,
                            c.z,
                            double(c.yaw_is_rate),
                            c.yaw,
                            double(c.vtol_mode),
                            double(c.api_enabled),
                            double(c.armed),
                            double(c.request),
                            double(input.state.time_stamp) * 1e-9};
  const double* values[] = {kinematics,   airspeed, barometer, imu,
                            magnetometer, distance, gps,       command};
  for (int port = 0; port < 8; ++port) {
    auto* out = ssGetOutputPortRealSignal(s, port);
    std::copy(values[port], values[port] + kOutputWidths[port], out);
  }
}
}  // namespace

static void mdlInitializeSizes(SimStruct* s) {
  ssSetNumSFcnParams(s, kParameterCount);
  if (ssGetNumSFcnParams(s) != ssGetSFcnParamsCount(s)) return;
  try {
    const auto effectors = GetEffectors(ssGetSFcnParam(s, 3));
    for (int p = 0; p < kParameterCount; ++p) ssSetSFcnParamTunable(s, p, 0);
    if (!ssSetNumInputPorts(s, 2) || !ssSetNumOutputPorts(s, 8)) return;
    ssSetInputPortWidth(s, 0, static_cast<int>(effectors.size()));
    ssSetInputPortWidth(s, 1, 2);  // Ready and landed status, each 0 or 1.
    for (int p = 0; p < 2; ++p) {
      ssSetInputPortDataType(s, p, SS_DOUBLE);
      ssSetInputPortRequiredContiguous(s, p, 1);
      ssSetInputPortDirectFeedThrough(s, p, 0);
    }
    for (int p = 0; p < 8; ++p) {
      ssSetOutputPortWidth(s, p, kOutputWidths[p]);
      ssSetOutputPortDataType(s, p, SS_DOUBLE);
    }
    ssSetNumPWork(s, 1);
    ssSetNumSampleTimes(s, 1);
  } catch (const std::exception& error) {
    ReportError(s, error);
  }
}

static void mdlInitializeSampleTimes(SimStruct* s) {
  const auto* value = ssGetSFcnParam(s, 2);
  if (!mxIsDouble(value) || mxIsComplex(value) ||
      mxGetNumberOfElements(value) != 1 || !std::isfinite(mxGetScalar(value)) ||
      mxGetScalar(value) <= 0 || ssIsVariableStepSolver(s)) {
    ssSetErrorStatus(s,
                     "External controller requires a positive sample period "
                     "and fixed-step solver");
    return;
  }
  ssSetSampleTime(s, 0, mxGetScalar(value));
  ssSetOffsetTime(s, 0, 0);
}

#define MDL_START
static void mdlStart(SimStruct* s) {
  try {
    std::unique_ptr<Transport> transport(new Transport());
    transport->effectors = GetEffectors(ssGetSFcnParam(s, 3));
    int rv = nng_rep0_open(&transport->socket);
    if (rv != 0) throw std::runtime_error(nng_strerror(rv));
    transport->open = true;
    nng_setopt_ms(transport->socket, NNG_OPT_RECVTIMEO, 5000);
    nng_setopt_ms(transport->socket, NNG_OPT_SENDTIMEO, 1000);
    const auto url = "tcp://" + GetString(ssGetSFcnParam(s, 0)) + ":" +
                     GetString(ssGetSFcnParam(s, 1));
    rv = nng_listen(transport->socket, url.c_str(), nullptr, 0);
    if (rv != 0) throw std::runtime_error(nng_strerror(rv));
    VTOLControlModelInputMessage initial;
    CopyOutputs(s, initial);
    ssSetPWorkValue(s, 0, transport.release());
  } catch (const std::exception& error) {
    ReportError(s, error);
  }
}

static void mdlOutputs(SimStruct*, int_T) {}

#define MDL_UPDATE
static void mdlUpdate(SimStruct* s, int_T) {
  auto* transport = static_cast<Transport*>(ssGetPWorkValue(s, 0));
  if (!transport) return;
  try {
    char* data = nullptr;
    size_t size = 0;
    int rv = nng_recv(transport->socket, &data, &size, NNG_FLAG_ALLOC);
    if (rv != 0) throw std::runtime_error(nng_strerror(rv));
    auto free_message = [size](char* ptr) { nng_free(ptr, size); };
    std::unique_ptr<char, decltype(free_message)> message(data, free_message);
    std::string bytes(message.get(), size);
    VTOLControlModelInputMessage input;
    input.Deserialize(bytes);
    if (input.effectors.size() != transport->effectors.size()) {
      throw std::runtime_error("Effector count differs from block metadata");
    }
    for (size_t i = 0; i < input.effectors.size(); ++i) {
      const auto& actual = input.effectors[i];
      const auto& expected = transport->effectors[i];
      if (actual.id != expected.id || actual.kind != expected.kind ||
          actual.units != expected.units) {
        throw std::runtime_error(
            "Effector order, kind, or units differ from block metadata");
      }
    }
    const auto* status = ssGetInputPortRealSignal(s, 1);
    if ((status[0] != 0 && status[0] != 1) ||
        (status[1] != 0 && status[1] != 1)) {
      throw std::runtime_error("Ready and landed status must be 0 or 1");
    }
    VTOLControlModelOutputMessage output;
    output.time_stamp = input.state.time_stamp;
    output.ready = status[0] != 0;
    output.landed = status[1] != 0;
    const auto* controls = ssGetInputPortRealSignal(s, 0);
    output.control_values.assign(controls,
                                 controls + transport->effectors.size());
    for (float value : output.control_values) {
      if (!std::isfinite(value))
        throw std::runtime_error("Control values must be finite");
    }
    // Reply with the previous step's control output, then expose the new
    // request.
    bytes = output.Serialize();
    rv = nng_send(transport->socket, &bytes[0], bytes.size(), 0);
    if (rv != 0) throw std::runtime_error(nng_strerror(rv));
    CopyOutputs(s, input);
  } catch (const std::exception& error) {
    ReportError(s, error);
  }
}

static void mdlTerminate(SimStruct* s) {
  delete static_cast<Transport*>(ssGetPWorkValue(s, 0));
  ssSetPWorkValue(s, 0, nullptr);
}

#ifdef MATLAB_MEX_FILE
#define PROCESS_MEX_SFUNCTION_EVERY_CALL
static bool ProcessMexSfunctionEveryCall(int_T, mxArray**, int_T,
                                        const mxArray**) {
  static bool module_locked = false;
  if (!module_locked) {
    // Register in the MEX entry context, before direct Simulink callbacks.
    mexAtExit(FinalizeTransport);
    mexLock();
    module_locked = true;
  }
  return false;
}
#include "simulink.c"
#else
#include "cg_sfun.h"
#endif
