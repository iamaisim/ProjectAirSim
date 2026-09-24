#pragma once

#include <ProjectAirsimClient/ProjectAirsimClient.h>

#include <string>
#include <functional>
#include <map>
#include <mutex>

namespace projectairsim_ros2_cpp {

inline const std::string& EffectiveVehicleName(const std::string& requested,
                                               const std::string& configured) {
  return requested.empty() ? configured : requested;
}

class RobotTypeCache {
 public:
  using Query = std::function<bool(nlohmann::json*, std::string*)>;

  bool Get(const std::string& robot_path, const Query& query,
           std::string* type, std::string* error) {
    // Serialize misses too: concurrent callers make only one successful lookup.
    std::lock_guard<std::mutex> lock(mutex_);
    const auto found = types_.find(robot_path);
    if (found != types_.end()) {
      *type = found->second;
      error->clear();
      return true;
    }
    nlohmann::json result;
    if (!query(&result, error)) {
      *error = "GetRobotType failed for " + robot_path + ": " + *error;
      return false;
    }
    if (!result.is_string()) {
      *error = "Expected string result from GetRobotType for " + robot_path;
      return false;
    }
    const auto value = result.get<std::string>();
    if (value != "wheeled-vehicle" && value != "unreal-vehicle" &&
        value != "drone" && value != "jsbsim" && value != "other") {
      *error = "Unknown GetRobotType result for " + robot_path + ": " + value;
      return false;
    }
    types_.emplace(robot_path, value);
    *type = value;
    error->clear();
    return true;
  }

  void Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    types_.clear();
  }

 private:
  std::mutex mutex_;
  std::map<std::string, std::string> types_;
};

// Resolve from the server-reported type. Unreal indices remain opaque.
inline bool ResolveVehicleParameter(const std::string& robot_type, int index,
                                    float value, std::string* method,
                                    nlohmann::json* params,
                                    std::string* error) {
  using json = nlohmann::json;
  *error = "set_parameter is unsupported for robot type: " + robot_type;
  if (robot_type != "wheeled-vehicle" && robot_type != "unreal-vehicle")
    return false;
  if (robot_type == "wheeled-vehicle") {
    static const char* methods[] = {"SetThrottle", "SetSteering", "SetBrakes"};
    if (index < 0 || index > 2) {
      *error = "Invalid WheeledVehicle index: expected 0, 1 or 2";
      return false;
    }
    *method = methods[index];
    *params = json{{"value", value}};
  } else {
    *method = "SetParameter";
    *params = json{{"index", index}, {"value", value}};
  }
  error->clear();
  return true;
}

using RobotRequest = std::function<bool(const std::string&, const nlohmann::json&,
                                       nlohmann::json*, std::string*)>;

inline bool SetVehicleParameter(RobotTypeCache& cache,
                                const std::string& robot_path, int index,
                                float value, const RobotRequest& request,
                                std::string* error) {
  using json = nlohmann::json;
  std::string type, method;
  json params, result;
  if (!cache.Get(robot_path,
                 [&](json* output, std::string* status) {
                   return request(robot_path + "/GetRobotType", json::object(),
                                  output, status);
                 }, &type, error))
    return false;
  if (!ResolveVehicleParameter(type, index, value, &method, &params, error))
    return false;
  if (!request(robot_path + "/" + method, params, &result, error))
    return false;
  if (!result.is_boolean()) {
    *error = "Expected boolean result from " + method;
    return false;
  }
  if (!result.get<bool>()) *error = method + " returned false";
  return result.get<bool>();
}
}  // namespace projectairsim_ros2_cpp
