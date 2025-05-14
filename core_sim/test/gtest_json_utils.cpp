// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/error.hpp"
#include "core_sim/json_utils.hpp"
#include "core_sim/math_utils.hpp"
#include "gtest/gtest.h"

namespace projectairsim = microsoft::projectairsim;
using json = nlohmann::json;

TEST(JsonUtils, GetIdentifier) {
  EXPECT_EQ(
      projectairsim::JsonUtils::GetIdentifier("{ \"id\": \"abc\" }"_json, "id"),
      "abc");
  EXPECT_EQ(
      projectairsim::JsonUtils::GetIdentifier("{ \"id\": \"Abc\" }"_json, "id"),
      "Abc");
  EXPECT_EQ(projectairsim::JsonUtils::GetIdentifier(
                "{ \"id\": \"a123bc-_\" }"_json, "id"),
            "a123bc-_");
  EXPECT_EQ(projectairsim::JsonUtils::GetIdentifier(
                "{ \"id\": \"A123bc-_\" }"_json, "id"),
            "A123bc-_");
  EXPECT_EQ(projectairsim::JsonUtils::GetIdentifier(
                "{ \"id\": \" A123bc-_\" }"_json, "id"),
            "A123bc-_");
  EXPECT_EQ(projectairsim::JsonUtils::GetIdentifier(
                "{ \"id\": \"A123bc-_\" }"_json, "id"),
            "A123bc-_");
  EXPECT_EQ(projectairsim::JsonUtils::GetIdentifier(
                "{ \"id\": \" A123bc-_\" }"_json, "id"),
            "A123bc-_");
  EXPECT_EQ(projectairsim::JsonUtils::GetIdentifier(
                "{ \"id\": \" A123bc-_\" }"_json, "id"),
            "A123bc-_");
  EXPECT_EQ(projectairsim::JsonUtils::GetIdentifier("{ }"_json, "id", "abc"),
            "abc");

  EXPECT_THROW(
      projectairsim::JsonUtils::GetIdentifier("{ \"id\": \"1a\" }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetIdentifier("{ \"id\": \"-a\" }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetIdentifier("{ \"id\": \"_a\" }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetIdentifier("{ \"id\": \"a b\" }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetIdentifier("{ \"id\": 1 }"_json, "id"),
      json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetIdentifier("{ \"id\": true }"_json, "id"),
      json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetIdentifier("{ \"id\": [] }"_json, "id"),
      json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetIdentifier("{ \"id\": { } }"_json, "id"),
      json::type_error);
}

TEST(JsonUtils, GetString) {
  EXPECT_EQ(
      projectairsim::JsonUtils::GetString("{ \"id\": \"abc\" }"_json, "id"),
      "abc");
  EXPECT_EQ(
      projectairsim::JsonUtils::GetString("{ \"id\": \"123\" }"_json, "id"),
      "123");
  EXPECT_EQ(projectairsim::JsonUtils::GetString(
                "{ \"id\": \" A123bc-_\" }"_json, "id"),
            "A123bc-_");
  EXPECT_EQ(projectairsim::JsonUtils::GetString("{ \"id\": \"A123bc-_\" }"_json,
                                                "id"),
            "A123bc-_");
  EXPECT_EQ(projectairsim::JsonUtils::GetString(
                "{ \"id\": \" A123bc-_\" }"_json, "id"),
            "A123bc-_");
  EXPECT_EQ(projectairsim::JsonUtils::GetString(
                "{ \"id\": \" A123bc-_\" }"_json, "id"),
            "A123bc-_");
  EXPECT_EQ(projectairsim::JsonUtils::GetString("{ }"_json, "id"), "");
  EXPECT_EQ(projectairsim::JsonUtils::GetString("{ }"_json, "id", "abc"),
            "abc");

  EXPECT_THROW(projectairsim::JsonUtils::GetString("{ \"id\": 1 }"_json, "id"),
               json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetString("{ \"id\": true }"_json, "id"),
      json::type_error);
  EXPECT_THROW(projectairsim::JsonUtils::GetString("{ \"id\": [] }"_json, "id"),
               json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetString("{ \"id\": { } }"_json, "id"),
      json::type_error);
}

TEST(JsonUtils, GetInteger) {
  EXPECT_EQ(projectairsim::JsonUtils::GetInteger("{ \"id\": 10 }"_json, "id"),
            10);
  EXPECT_EQ(projectairsim::JsonUtils::GetInteger("{ \"id\": 10.1 }"_json, "id"),
            10);
  EXPECT_EQ(projectairsim::JsonUtils::GetInteger("{ \"id\": 10.5 }"_json, "id"),
            10);
  EXPECT_EQ(projectairsim::JsonUtils::GetInteger("{ \"id\": true }"_json, "id"),
            1);
  EXPECT_EQ(
      projectairsim::JsonUtils::GetInteger("{ \"id\": false }"_json, "id"), 0);

  EXPECT_THROW(
      projectairsim::JsonUtils::GetInteger("{ \"id\": \"a\" }"_json, "id"),
      json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetInteger("{ \"id\": \"1\" }"_json, "id"),
      json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetInteger("{ \"id\": [] }"_json, "id"),
      json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetInteger("{ \"id\": { } }"_json, "id"),
      json::type_error);
}

TEST(JsonUtils, GetNumber) {
  // Test GetNumber<float>
  EXPECT_EQ(
      projectairsim::JsonUtils::GetNumber<float>("{ \"id\": 10 }"_json, "id"),
      10.0f);
  EXPECT_EQ(
      projectairsim::JsonUtils::GetNumber<float>("{ \"id\": 10.1 }"_json, "id"),
      10.1f);
  EXPECT_EQ(
      projectairsim::JsonUtils::GetNumber<float>("{ \"id\": 10.5 }"_json, "id"),
      10.5f);
  EXPECT_EQ(
      projectairsim::JsonUtils::GetNumber<float>("{ \"id\": 0.5 }"_json, "id"),
      0.5f);
  EXPECT_EQ(
      projectairsim::JsonUtils::GetNumber<float>("{ \"id\": true }"_json, "id"),
      1.0f);
  EXPECT_EQ(projectairsim::JsonUtils::GetNumber<float>("{ \"id\": false }"_json,
                                                       "id"),
            0.0f);

  // Test GetNumber<double>
  EXPECT_EQ(
      projectairsim::JsonUtils::GetNumber<double>("{ \"id\": 10 }"_json, "id"),
      10.0);
  EXPECT_EQ(projectairsim::JsonUtils::GetNumber<double>("{ \"id\": 10.1 }"_json,
                                                        "id"),
            10.1);
  EXPECT_EQ(projectairsim::JsonUtils::GetNumber<double>("{ \"id\": 10.5 }"_json,
                                                        "id"),
            10.5);
  EXPECT_EQ(
      projectairsim::JsonUtils::GetNumber<double>("{ \"id\": 0.5 }"_json, "id"),
      0.5);

  // Test type errors
  EXPECT_THROW(
      projectairsim::JsonUtils::GetInteger("{ \"id\": \"a\" }"_json, "id"),
      json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetInteger("{ \"id\": \"1.12\" }"_json, "id"),
      json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetNumber<float>("{ \"id\": [] }"_json, "id"),
      json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetNumber<float>("{ \"id\": { } }"_json, "id"),
      json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetNumber<double>("{ \"id\": [] }"_json, "id"),
      json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetNumber<double>("{ \"id\": { } }"_json, "id"),
      json::type_error);
}

TEST(JsonUtils, GetObject) {
  EXPECT_EQ(projectairsim::JsonUtils::GetJsonObject(
                "{ \"id\": { \"id\": \"abc\" } }"_json, "id"),
            "{\"id\": \"abc\"}"_json);
  EXPECT_EQ(
      projectairsim::JsonUtils::GetJsonObject("{ \"id\": { } }"_json, "id"),
      "{ }"_json);
  EXPECT_EQ(projectairsim::JsonUtils::GetJsonObject("{ }"_json, "id",
                                                    "{ \"id\": 1}"_json),
            "{ \"id\": 1 }"_json);
  EXPECT_EQ(projectairsim::JsonUtils::GetJsonObject("{ }"_json, "id"),
            "{ }"_json);

  EXPECT_THROW(
      projectairsim::JsonUtils::GetJsonObject("{ \"id\": \"abc\" }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetJsonObject("{ \"id\": 1 }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetJsonObject("{ \"id\": true }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetJsonObject("{ \"id\": [] }"_json, "id"),
      projectairsim::Error);
}

TEST(JsonUtils, GetArray) {
  EXPECT_EQ(projectairsim::JsonUtils::GetArray(
                "{ \"id\": [ { \"x\": 1 } ] }"_json, "id"),
            "[ { \"x\": 1 } ]"_json);
  EXPECT_EQ(projectairsim::JsonUtils::GetArray("{ \"id\": [ ] }"_json, "id"),
            "[ ]"_json);
  EXPECT_EQ(projectairsim::JsonUtils::GetArray("{ }"_json, "id",
                                               "[ { \"x\": 1 } ]"_json),
            "[ { \"x\": 1 } ]"_json);
  EXPECT_EQ(projectairsim::JsonUtils::GetArray("{ }"_json, "id"), "[ ]"_json);

  EXPECT_THROW(
      projectairsim::JsonUtils::GetArray("{ \"id\": \"abc\" }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(projectairsim::JsonUtils::GetArray("{ \"id\": 1 }"_json, "id"),
               projectairsim::Error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetArray("{ \"id\": true }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(projectairsim::JsonUtils::GetArray("{ \"id\": {} }"_json, "id"),
               projectairsim::Error);
}

TEST(JsonUtils, GetVector3) {
  EXPECT_EQ(projectairsim::JsonUtils::GetVector3("{ \"xyz\": \"1 2 3\" }"_json,
                                                 "xyz"),
            projectairsim::Vector3(1, 2, 3));
  EXPECT_EQ(projectairsim::JsonUtils::GetVector3(
                "{ \"xyz\": \"1  2  3\" }"_json, "xyz"),
            projectairsim::Vector3(1, 2, 3));
  EXPECT_EQ(projectairsim::JsonUtils::GetVector3(
                "{ \"xyz\": \"1.9 2.5 3.0\" }"_json, "xyz"),
            projectairsim::Vector3(1.9f, 2.5f, 3.0f));
  EXPECT_EQ(projectairsim::JsonUtils::GetVector3(
                "{ \"xyz\": \" 10.1 10 10\" }"_json, "xyz"),
            projectairsim::Vector3(10.1f, 10, 10));
  EXPECT_EQ(projectairsim::JsonUtils::GetVector3(
                "{ \"xyz\": \"10.1 10 10 \" }"_json, "xyz"),
            projectairsim::Vector3(10.1f, 10, 10));
  EXPECT_EQ(projectairsim::JsonUtils::GetVector3(
                "{ \"xyz\": \" 10.1 10 10 \" }"_json, "xyz"),
            projectairsim::Vector3(10.1f, 10, 10));
  EXPECT_EQ(projectairsim::JsonUtils::GetVector3(
                "{ \"xyz\": \"10 10 10\" }"_json, "id"),
            projectairsim::Vector3(0, 0, 0));
  EXPECT_EQ(projectairsim::JsonUtils::GetVector3(
                "{ \"id\": \"10, 10, 10\" }"_json, "id"),
            projectairsim::Vector3(10, 10, 10));
  EXPECT_EQ(
      projectairsim::JsonUtils::GetVector3("{}"_json, "id",
                                           projectairsim::Vector3(5, 5, 5)),
      projectairsim::Vector3(5, 5, 5));  // test to check if defualt value
                                         // is returned when key is not found
  EXPECT_EQ(
      projectairsim::JsonUtils::GetVector3("{ \"xyz\": \"10 10 10\" }"_json,
                                           "id",
                                           projectairsim::Vector3(15, 15, 15)),
      projectairsim::Vector3(15, 15, 15));  // test to check if defualt value
                                            // is returned when key is not found
  EXPECT_EQ(projectairsim::JsonUtils::GetVector3(
                "{ \"xyz\": \"10 10 10\" }"_json, "xyz",
                projectairsim::Vector3(15, 15, 15)),
            projectairsim::Vector3(
                10, 10, 10));  // test to check if provided value is returned
                               // even if default value is given

  EXPECT_THROW(
      projectairsim::JsonUtils::GetVector3("{ \"id\": \"abc\" }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetVector3("{ \"id\": \"\" }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(projectairsim::JsonUtils::GetVector3("{ \"id\": 1 }"_json, "id"),
               json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetVector3("{ \"id\": true }"_json, "id"),
      json::type_error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetVector3("{ \"id\": [] }"_json, "id"),
      json::type_error);
}

TEST(JsonUtils, GetTransform) {
  auto transform = projectairsim::JsonUtils::GetTransform(
      "{ \"origin\": { \"xyz\": \"1 2 3\", \"rpy\": \"30 30 30\" } }"_json,
      "origin");
  auto quaternion = projectairsim::TransformUtils::ToQuaternion(30, 30, 30);
  EXPECT_EQ(transform.translation_, projectairsim::Vector3(1, 2, 3));
  EXPECT_EQ(transform.rotation_.w(), quaternion.w());
  EXPECT_EQ(transform.rotation_.x(), quaternion.x());
  EXPECT_EQ(transform.rotation_.y(), quaternion.y());
  EXPECT_EQ(transform.rotation_.x(), quaternion.z());

  // invalid translation tag, expect default translation of 0
  transform = projectairsim::JsonUtils::GetTransform(
      "{ \"origin\": { \"xz\": \"1 2 3\", \"rpy\": \"30 30 30\" } }"_json,
      "origin");
  EXPECT_EQ(transform.translation_, projectairsim::Vector3(0, 0, 0));
  EXPECT_EQ(transform.rotation_.w(), quaternion.w());
  EXPECT_EQ(transform.rotation_.x(), quaternion.x());
  EXPECT_EQ(transform.rotation_.y(), quaternion.y());
  EXPECT_EQ(transform.rotation_.x(), quaternion.z());

  // invalid rotation tag, expect default rotation of 0
  transform = projectairsim::JsonUtils::GetTransform(
      "{ \"origin\": { \"xyz\": \"1 2 3\", \"py\": \"30 30 30\" } }"_json,
      "origin");
  EXPECT_EQ(transform.translation_, projectairsim::Vector3(1, 2, 3));
  EXPECT_EQ(transform.rotation_.w(), 1);
  EXPECT_EQ(transform.rotation_.x(), 0);
  EXPECT_EQ(transform.rotation_.y(), 0);
  EXPECT_EQ(transform.rotation_.z(), 0);

  // test degrees
  transform = projectairsim::JsonUtils::GetTransform(
      "{ \"origin\": { \"xyz\": \"1 2 3\", \"rpy-deg\": \"420 420 420\" } }"_json,
      "origin");
  auto ang_rad = projectairsim::MathUtils::deg2Rad(420);
  quaternion =
      projectairsim::TransformUtils::ToQuaternion(ang_rad, ang_rad, ang_rad);
  EXPECT_EQ(transform.translation_, projectairsim::Vector3(1, 2, 3));
  EXPECT_FLOAT_EQ(transform.rotation_.w(), quaternion.w());
  EXPECT_NEAR(transform.rotation_.x(), quaternion.x(), 1e-6);
  EXPECT_FLOAT_EQ(transform.rotation_.y(), quaternion.y());
  EXPECT_NEAR(transform.rotation_.x(), quaternion.z(), 1e-6);

  // invalid rotation tag, expect default rotation of 0
  transform = projectairsim::JsonUtils::GetTransform(
      "{ \"origin\": { \"xyz\": \"1 2 3\", \"py-deg\": \"420 420 420\" } }"_json,
      "origin");
  EXPECT_EQ(transform.translation_, projectairsim::Vector3(1, 2, 3));
  EXPECT_EQ(transform.rotation_.w(), 1);
  EXPECT_EQ(transform.rotation_.x(), 0);
  EXPECT_EQ(transform.rotation_.y(), 0);
  EXPECT_EQ(transform.rotation_.z(), 0);

  EXPECT_THROW(
      projectairsim::JsonUtils::GetTransform("{ \"id\": \"abc\" }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetTransform("{ \"id\": 1 }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetTransform("{ \"id\": true }"_json, "id"),
      projectairsim::Error);
  EXPECT_THROW(
      projectairsim::JsonUtils::GetTransform("{ \"id\": [] }"_json, "id"),
      projectairsim::Error);
}

TEST(JsonUtils, IsEmpty) {
  EXPECT_FALSE(
      projectairsim::JsonUtils::IsEmpty("{ \"id\": [ { \"x\": 1 } ] }"_json));
  EXPECT_TRUE(projectairsim::JsonUtils::IsEmpty("{ }"_json));
}

TEST(JsonUtils, IsEmptyArray) {
  EXPECT_FALSE(projectairsim::JsonUtils::IsEmptyArray("[ { \"x\": 1 } ]"_json));
  EXPECT_TRUE(projectairsim::JsonUtils::IsEmptyArray("[ ]"_json));
}
