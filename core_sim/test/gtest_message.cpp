// Copyright (C) Microsoft Corporation. All rights reserved.

#include <iostream>
#include <string>

#include "core_sim/message/request_message.hpp"
#include "core_sim/message/response_failure_message.hpp"
#include "core_sim/message/response_success_message.hpp"
#include "gtest/gtest.h"
#include "json.hpp"

namespace projectairsim = microsoft::projectairsim;
using json = nlohmann::json;

TEST(Message, RequestMessageContent) {
  int id = 1;
  std::string method = "TestRequestMethod";
  json params = R"({"param1": "val1"})"_json;
  float version = 10.0;
  auto request_message =
      projectairsim::RequestMessage(id, method, params, version);

  EXPECT_EQ(request_message.GetID(), 1);
  EXPECT_TRUE(request_message.GetMethod() == method);
  //! Using PREDn instead of EQ or TRUE to assist with debuggin on failure;
  //! With predicate assertions, the func args gets printed for free if failed
  // Enable after Json-Msgpack upgrade
  // EXPECT_PRED2([](auto returned_val,
  //                auto actual_val) { return returned_val == actual_val; },
  //             request_message.GetParams(), params);
  // EXPECT_EQ(request_message.GetParams(), params);

  EXPECT_FLOAT_EQ(request_message.GetVersion(), version);
}

TEST(Message, RequestMessagePackingAndUnpacking) {
  int id = 1;
  std::string method = "TestRequestMethod";
  json params = R"({"param1": "val1"})"_json;
  float version = 10.0;
  auto original_message =
      projectairsim::RequestMessage(id, method, params, version);
  auto original_message_packed = original_message.Serialize();

  auto unpacked_result = projectairsim::RequestMessage();
  unpacked_result.Deserialize(original_message_packed);

  EXPECT_EQ(original_message.GetID(), unpacked_result.GetID());

  EXPECT_TRUE(original_message.GetMethod() == unpacked_result.GetMethod());
  EXPECT_PRED2([](auto returned_val,
                  auto actual_val) { return returned_val == actual_val; },
               original_message.GetParams(), unpacked_result.GetParams());
  // EXPECT_EQ(original_message.GetParams(), result.GetParams());

  EXPECT_FLOAT_EQ(original_message.GetVersion(), unpacked_result.GetVersion());
}

TEST(Message, ResponseSuccessMessageContent) {
  int id = 1;
  json result = R"({"res1": "val1"})"_json;
  float version = 10.0;
  auto request_message =
      projectairsim::ResponseSuccessMessage(id, result, version);

  EXPECT_EQ(request_message.GetID(), 1);
  //! Using PREDn instead of EQ or TRUE to assist with debuggin on failure;
  //! With predicate assertions, the func args gets printed for free if failed
  // Enable after Json-Msgpack upgrade
  // EXPECT_PRED2([](auto returned_val,
  //                auto actual_val) { return returned_val == actual_val; },
  //             request_message.GetResult(), result);
  // EXPECT_EQ(request_message.GetResult(), result);

  EXPECT_FLOAT_EQ(request_message.GetVersion(), version);
}

TEST(Message, ResponseSuccessMessagePackingAndUnpacking) {
  int id = 1;
  json result = R"({"res1": "val1"})"_json;
  float version = 10.0;
  auto original_message =
      projectairsim::ResponseSuccessMessage(id, result, version);
  auto original_message_packed = original_message.Serialize();

  auto unpacked_result = projectairsim::ResponseSuccessMessage();
  unpacked_result.Deserialize(original_message_packed);

  EXPECT_EQ(original_message.GetID(), unpacked_result.GetID());

  EXPECT_PRED2([](auto returned_val,
                  auto actual_val) { return returned_val == actual_val; },
               original_message.GetResult(), unpacked_result.GetResult());
  // EXPECT_EQ(original_message.GetResult(), result.GetResult());

  EXPECT_FLOAT_EQ(original_message.GetVersion(), unpacked_result.GetVersion());
}

TEST(Message, ResponseFailureMessageContent) {
  int id = 1;
  json error = R"({"code": "err_code", "message": "err_msg"})"_json;
  float version = 10.0;
  auto request_message =
      projectairsim::ResponseFailureMessage(id, error, version);

  EXPECT_EQ(request_message.GetID(), 1);
  //! Using PREDn instead of EQ or TRUE to assist with debuggin on failure;
  //! With predicate assertions, the func args gets printed for free if failed
  // Enable after Json-Msgpack upgrade
  // EXPECT_PRED2([](auto returned_val,
  //                auto actual_val) { return returned_val == actual_val; },
  //             request_message.GetError(), error);
  // EXPECT_EQ(request_message.GetError(), error);

  EXPECT_FLOAT_EQ(request_message.GetVersion(), version);
}

TEST(Message, ResponseFailureMessagePackingAndUnpacking) {
  int id = 1;
  json error = R"({"code": "err_code", "message": "err_msg"})"_json;
  float version = 10.0;
  auto original_message =
      projectairsim::ResponseFailureMessage(id, error, version);
  auto original_message_packed = original_message.Serialize();

  auto unpacked_result = projectairsim::ResponseFailureMessage();
  unpacked_result.Deserialize(original_message_packed);

  EXPECT_EQ(original_message.GetID(), unpacked_result.GetID());

  EXPECT_PRED2([](auto returned_val,
                  auto actual_val) { return returned_val == actual_val; },
               original_message.GetError(), unpacked_result.GetError());
  // EXPECT_EQ(original_message.GetError(), result.GetError());

  EXPECT_FLOAT_EQ(original_message.GetVersion(), unpacked_result.GetVersion());
}