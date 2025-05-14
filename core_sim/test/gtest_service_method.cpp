// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include <iostream>
#include <string>

#include "core_sim/actor/robot.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_method.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "topic_manager.hpp"

namespace projectairsim = microsoft::projectairsim;
using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {

class Scene {
 public:
  static ServiceMethod GetMethod() {
    ServiceMethod new_method = ServiceMethod();
    return new_method;
  }
};

}  // namespace projectairsim
}  // namespace microsoft

class TestComponentClass {
 public:
  int APIMethodA() { return age; }

  int APIMethodB(int new_age) {
    age = new_age;
    return age;
  }

  int APIMethodC(int years_from_now) { return age + years_from_now; }

  std::string APIMethodD(int new_age, std::string new_name) {
    age = new_age;
    name = new_name;
    return "Success";
  }

  int age = 10;
  std::string name = "default";
};

TEST(ServiceMethod, TestCreateInstanceMethodHandlerForGetters) {
  TestComponentClass test_api_class_instance;
  auto new_method = projectairsim::Scene::GetMethod();
  projectairsim::MethodHandler test_handler = new_method.CreateMethodHandler(
      &TestComponentClass::APIMethodA, test_api_class_instance);
  auto result = test_handler("[]"_json);
  EXPECT_EQ(result, 10);
}
TEST(ServiceMethod, TestCreateInstanceMethodHandlerForSetters) {
  TestComponentClass test_api_class_instance;
  auto new_method = projectairsim::Scene::GetMethod();
  projectairsim::MethodHandler test_handler = new_method.CreateMethodHandler(
      &TestComponentClass::APIMethodB, test_api_class_instance);
  auto result = test_handler(R"([5])"_json);
  EXPECT_EQ(result, 5);
}

TEST(ServiceMethod, TestCreateInstanceMethodHandlerForMutators) {
  TestComponentClass test_api_class_instance;
  auto new_method = projectairsim::Scene::GetMethod();
  projectairsim::MethodHandler test_handler = new_method.CreateMethodHandler(
      &TestComponentClass::APIMethodC, test_api_class_instance);
  auto result = test_handler(R"([5])"_json);
  EXPECT_EQ(result, 15);
}

TEST(ServiceMethod, TestCreateInstanceMethodHandlerMixedTypes) {
  TestComponentClass test_api_class_instance;
  auto new_method = projectairsim::Scene::GetMethod();
  projectairsim::MethodHandler test_handler = new_method.CreateMethodHandler(
      &TestComponentClass::APIMethodD, test_api_class_instance);
  auto result = test_handler(R"([5, "New Name"])"_json);
  EXPECT_EQ(result, "Success");
}

TEST(ServiceMethod, TestCreateMethodHandlerFromClassScope) {
  class TestClass {
   public:
    TestClass() {}
    int SelfMethodA() { return 11; }
    json GetResult() { return test_handler("[]"_json); }
    projectairsim::ServiceMethod new_method = projectairsim::Scene::GetMethod();
    projectairsim::MethodHandler test_handler =
        new_method.CreateMethodHandler(&TestClass::SelfMethodA, *this);
  };
  TestClass obj;
  auto result = obj.GetResult();
  EXPECT_EQ(result, 11);
}
