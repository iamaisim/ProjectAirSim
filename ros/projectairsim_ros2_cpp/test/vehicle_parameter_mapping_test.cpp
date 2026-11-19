#include "projectairsim_ros2_cpp/vehicle_parameter_mapping.hpp"

#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <vector>

using json = nlohmann::json;
using projectairsim_ros2_cpp::ResolveVehicleParameter;
using projectairsim_ros2_cpp::RobotTypeCache;
using projectairsim_ros2_cpp::SetVehicleParameter;

namespace {
struct MockServer {
  json type = "wheeled-vehicle";
  json applied = true;
  bool lookup_ok = true;
  bool control_ok = true;
  int queries = 0;
  std::vector<std::pair<std::string, json>> controls;
  std::string lookup_method;

  bool Request(const std::string& method, const json& params,
               json* result, std::string* error) {
    if (method.size() >= 13 && method.substr(method.size() - 13) == "/GetRobotType") {
      ++queries;
      lookup_method = method;
      EXPECT_EQ(params, json::object());
      *result = type;
      if (!lookup_ok) *error = "ServiceMethod not supported";
      return lookup_ok;
    }
    controls.emplace_back(method, params);
    *result = applied;
    if (!control_ok) *error = "Transport failure";
    return control_ok;
  }
};

bool Send(RobotTypeCache& cache, MockServer& server, const std::string& path,
          int index, std::string* error) {
  return SetVehicleParameter(cache, path, index, .5f,
      [&](const std::string& method, const json& params, json* result,
          std::string* status) {
        return server.Request(method, params, result, status);
      }, error);
}

TEST(VehicleParameterMapping, ServerTypeRoutesArbitraryAndMisleadingNames) {
  RobotTypeCache cache;
  MockServer server;
  std::string error;
  const char* methods[] = {"SetThrottle", "SetSteering", "SetBrakes"};
  for (const auto& name : {"CarA", "UnrealVehicle_WheeledVehicle"}) {
    const auto path = std::string("/Sim/Scene/robots/") + name;
    for (int i = 0; i < 3; ++i) {
      ASSERT_TRUE(Send(cache, server, path, i, &error));
      EXPECT_EQ(server.controls.back().first, path + "/" + methods[i]);
      EXPECT_EQ(server.controls.back().second, json({{"value", .5f}}));
    }
    for (int i : {-1, 3, 100}) {
      const auto count = server.controls.size();
      EXPECT_FALSE(Send(cache, server, path, i, &error));
      EXPECT_EQ(server.controls.size(), count);
      EXPECT_FALSE(error.empty());
    }
  }
  EXPECT_EQ(server.queries, 2);
}

TEST(VehicleParameterMapping, UnrealKeepsOpaqueBlueprintIndices) {
  RobotTypeCache cache;
  MockServer server;
  server.type = "unreal-vehicle";
  std::string error;
  for (int index : {-1, 0, 1, 2, 42}) {
    ASSERT_TRUE(Send(cache, server, "/Sim/Scene/robots/CarB", index, &error));
    EXPECT_EQ(server.controls.back().first, "/Sim/Scene/robots/CarB/SetParameter");
    EXPECT_EQ(server.controls.back().second, json({{"index", index}, {"value", .5f}}));
  }
  EXPECT_EQ(server.queries, 1);
}

TEST(VehicleParameterMapping, EmptyRequestUsesConfiguredNameForLookupAndControl) {
  RobotTypeCache cache;
  MockServer server;
  std::string error;
  const std::string name = projectairsim_ros2_cpp::EffectiveVehicleName("", "CarA");
  ASSERT_TRUE(Send(cache, server, "/Sim/Scene/robots/" + name, 0, &error));
  EXPECT_EQ(server.lookup_method, "/Sim/Scene/robots/CarA/GetRobotType");
  EXPECT_EQ(server.controls.back().first, "/Sim/Scene/robots/CarA/SetThrottle");
  EXPECT_EQ(projectairsim_ros2_cpp::EffectiveVehicleName("CarB", "CarA"), "CarB");
}

TEST(VehicleParameterMapping, UnsupportedTypesAreCachedButNeverControlled) {
  for (const auto& type : {"drone", "jsbsim", "other"}) {
    RobotTypeCache cache;
    MockServer server;
    server.type = type;
    std::string error;
    EXPECT_FALSE(Send(cache, server, "/Sim/Scene/robots/Car", 0, &error));
    EXPECT_FALSE(Send(cache, server, "/Sim/Scene/robots/Car", 0, &error));
    EXPECT_EQ(server.queries, 1);
    EXPECT_TRUE(server.controls.empty());
    EXPECT_NE(error.find("unsupported"), std::string::npos);
  }
}

TEST(VehicleParameterMapping, FailedMalformedAndUnknownLookupsAreNotCached) {
  for (const auto& type : {json(nullptr), json(7), json(true), json::object(),
                           json::array(), json(""), json("WheeledVehicle"), json("future")}) {
    RobotTypeCache cache;
    MockServer server;
    server.type = type;
    std::string error;
    EXPECT_FALSE(Send(cache, server, "/Sim/Scene/robots/WheeledVehicle", 0, &error));
    EXPECT_FALSE(Send(cache, server, "/Sim/Scene/robots/WheeledVehicle", 0, &error));
    EXPECT_EQ(server.queries, 2);
    EXPECT_TRUE(server.controls.empty());
    EXPECT_FALSE(error.empty());
    server.type = "wheeled-vehicle";
    EXPECT_TRUE(Send(cache, server, "/Sim/Scene/robots/WheeledVehicle", 0, &error));
  }
  RobotTypeCache cache;
  MockServer server;
  server.lookup_ok = false;
  std::string error;
  EXPECT_FALSE(Send(cache, server, "/Sim/Scene/robots/Car", 0, &error));
  EXPECT_FALSE(Send(cache, server, "/Sim/Scene/robots/Car", 0, &error));
  EXPECT_EQ(server.queries, 2);
  EXPECT_TRUE(server.controls.empty());
  EXPECT_NE(error.find("GetRobotType failed"), std::string::npos);
  server.lookup_ok = true;
  EXPECT_TRUE(Send(cache, server, "/Sim/Scene/robots/Car", 0, &error));
}

TEST(VehicleParameterMapping, CacheKeysIncludeSceneAndRobotAndClearAdoptsChanges) {
  RobotTypeCache cache;
  MockServer server;
  std::string error;
  ASSERT_TRUE(Send(cache, server, "/Sim/SceneA/robots/CarA", 0, &error));
  ASSERT_TRUE(Send(cache, server, "/Sim/SceneA/robots/CarB", 0, &error));
  ASSERT_TRUE(Send(cache, server, "/Sim/SceneB/robots/CarA", 0, &error));
  EXPECT_EQ(server.queries, 3);
  server.type = "unreal-vehicle";
  ASSERT_TRUE(Send(cache, server, "/Sim/SceneA/robots/CarA", 0, &error));
  EXPECT_EQ(server.controls.back().first, "/Sim/SceneA/robots/CarA/SetThrottle");
  cache.Clear();  // Successful scene load or reconnect, including same scene ID.
  ASSERT_TRUE(Send(cache, server, "/Sim/SceneA/robots/CarA", 42, &error));
  EXPECT_EQ(server.queries, 4);
  EXPECT_EQ(server.controls.back().first, "/Sim/SceneA/robots/CarA/SetParameter");
}

TEST(VehicleParameterMapping, ControlFailuresAndNonBooleanResultsPropagate) {
  RobotTypeCache cache;
  MockServer server;
  std::string error;
  server.applied = false;
  EXPECT_FALSE(Send(cache, server, "/Sim/Scene/robots/Car", 0, &error));
  EXPECT_NE(error.find("returned false"), std::string::npos);
  server.applied = 1;
  EXPECT_FALSE(Send(cache, server, "/Sim/Scene/robots/Car", 0, &error));
  EXPECT_NE(error.find("Expected boolean"), std::string::npos);
  server.control_ok = false;
  EXPECT_FALSE(Send(cache, server, "/Sim/Scene/robots/Car", 0, &error));
  EXPECT_EQ(error, "Transport failure");
  EXPECT_EQ(server.queries, 1);
}

TEST(VehicleParameterMapping, ConcurrentCacheMissesQueryOnce) {
  RobotTypeCache cache;
  std::atomic<int> queries{0};
  std::atomic<int> failures{0};
  std::vector<std::thread> threads;
  for (int i = 0; i < 8; ++i) {
    threads.emplace_back([&]() {
      std::string type, error;
      if (!cache.Get("/Sim/Scene/robots/Car",
          [&](json* result, std::string*) {
            ++queries;
            *result = "wheeled-vehicle";
            return true;
          }, &type, &error) || type != "wheeled-vehicle") ++failures;
    });
  }
  for (auto& thread : threads) thread.join();
  EXPECT_EQ(queries.load(), 1);
  EXPECT_EQ(failures.load(), 0);
}
}  // namespace
