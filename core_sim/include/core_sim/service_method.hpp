// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SERVICE_METHOD_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SERVICE_METHOD_HPP_

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "core_sim/error.hpp"
#include "core_sim/json_utils.hpp"
#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

using ParamsList = std::vector<std::string>;
using MethodParameters = std::vector<json>;
using MethodHandler = std::function<json(const MethodParameters&)>;

class ServiceMethod {
 public:
  ServiceMethod() {}

  const std::string& GetName() const { return name_; }
  const ParamsList& GetParamsList() const { return params_list_; }

  template <typename T, typename ReturnType, typename... ParamTypes>
  MethodHandler CreateMethodHandler(ReturnType (T::*method)(ParamTypes...),
                                    T& instance);

  template <typename ReturnType, typename... ParamTypes>
  MethodHandler CreateMethodHandler(
      std::function<ReturnType(ParamTypes...)> method);

  ServiceMethod(const std::string& name, const ParamsList& params_list)
      : name_(name), params_list_(params_list) {}

 private:
  friend class ServiceManager;
  friend class Scene;

  template <typename T>
  void ValidateMethodParamTypes(
      size_t index, const json& param, json::value_t expected_type,
      typename std::enable_if<!std::is_arithmetic<T>::value>::type* = 0);

  template <typename T>
  void ValidateMethodParamTypes(
      size_t index, const json& param, json::value_t expected_type,
      typename std::enable_if<std::is_arithmetic<T>::value>::type* = 0);

  template <typename ReturnType, typename... ParamTypes, std::size_t... index>
  MethodHandler GetHandler(std::function<ReturnType(ParamTypes...)> method,
                           std::index_sequence<index...>);

  std::string name_;
  ParamsList params_list_;
};

template <typename T>
inline void ValidateMethodParamTypes(
    size_t index, const json& param, json::value_t expected_type,
    typename std::enable_if<!std::is_arithmetic<T>::value>::type* = 0) {
  if (param.type() != expected_type) {
    throw Error("Invalid parameter type for parameter at index" +
                std::to_string(index) + "; Must be " +
                JsonUtils::GetTypeNameAsStr(expected_type) + " got " +
                JsonUtils::GetTypeNameAsStr(param.type()));
  }
}

template <typename T>
inline void ValidateMethodParamTypes(
    size_t index, const json& param, json::value_t expected_type,
    typename std::enable_if<std::is_arithmetic<T>::value>::type* = 0) {
  if (expected_type == json::value_t::number_unsigned &&
      param.type() == json::value_t::number_integer) {
    if (param.get<long long int>() < 0) {
      throw Error("Invalid parameter type for parameter at index" +
                  std::to_string(index) + "; Must be " +
                  JsonUtils::GetTypeNameAsStr(expected_type) + " got " +
                  JsonUtils::GetTypeNameAsStr(param.type()));
    }
  } else if (param.type() == json::value_t::number_unsigned &&
             expected_type == json::value_t::number_integer) {
    if (param.get<long long unsigned>() > std::numeric_limits<T>::max()) {
      throw Error("Invalid parameter type for parameter at index" +
                  std::to_string(index) +
                  "; Exceeds range of values of expected type " +
                  JsonUtils::GetTypeNameAsStr(expected_type));
    }
  } else if (param.type() != expected_type) {
    throw Error("Invalid parameter type for parameter at index" +
                std::to_string(index) + "; Must be " +
                JsonUtils::GetTypeNameAsStr(expected_type) + " got " +
                JsonUtils::GetTypeNameAsStr(param.type()));
  }
}

template <typename ReturnType, typename... ParamTypes, std::size_t... index>
inline MethodHandler ServiceMethod::GetHandler(
    std::function<ReturnType(ParamTypes...)> method,
    std::index_sequence<index...>) {
  MethodHandler handler = [method](const MethodParameters& params) -> json {
    size_t deduced_size = params.size();
    size_t provided_size = sizeof...(ParamTypes);

    if (deduced_size != provided_size) {
      throw Error(
          "Invalid number of parameters received while creating MethodHandler; "
          "Expected " +
          std::to_string(deduced_size) + " argument(s), got " +
          std::to_string(provided_size));
    }

    // ValidateMethodParamTypes<typename std::decay<ParamTypes>::type>(
    //    index, params[index],
    //    JsonUtils::GetType(type<typename std::decay<ParamTypes>::type>()));
    return json(
        method(params[index].get<typename std::decay<ParamTypes>::type>()...));
  };
  return handler;
}

template <typename ReturnType, typename... ParamTypes>
inline MethodHandler ServiceMethod::CreateMethodHandler(
    std::function<ReturnType(ParamTypes...)> method) {
  return GetHandler(method, std::index_sequence_for<ParamTypes...>{});
}

template <typename T, typename ReturnType, typename... ParamTypes>
inline MethodHandler ServiceMethod::CreateMethodHandler(
    ReturnType (T::*method)(ParamTypes...), T& instance) {
  std::function<ReturnType(ParamTypes...)> function =
      [&instance, method](ParamTypes&&... params) -> ReturnType {
    return (instance.*method)(std::forward<ParamTypes>(params)...);
  };
  return CreateMethodHandler(function);
}

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SERVICE_METHOD_HPP_
