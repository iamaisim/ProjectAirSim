// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/config_json.hpp"
#include "core_sim/error.hpp"
#include "core_sim/link.hpp"
#include "core_sim/logger.hpp"
#include "gtest/gtest.h"
#include "json.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class Robot {
 public:
  static Link MakeLink() {
    auto callback = [](const std::string& component, LogLevel level,
                       const std::string& message) {};
    Logger logger(callback);
    TopicManager topic_mgr(logger);
    return Link(logger, topic_mgr, "");
  }

  static void LoadLink(Link& link, ConfigJson config_json) {
    link.Load(config_json);
  }
};

}  // namespace projectairsim
}  // namespace microsoft

namespace projectairsim = microsoft::projectairsim;
using json = nlohmann::json;

TEST(Link, Constructor) {
  EXPECT_FALSE(projectairsim::Robot::MakeLink().IsLoaded());
}

TEST(Link, Load) {
  json json =
      "{ \
            \"name\": \"a\", \
            \"visual\" : { \
                \"geometry\" : { \
                    \"type\": \"unreal_mesh\", \
                    \"name\" : \"Link_a\" \
                } \
            } \
        }"_json;

  auto link = projectairsim::Robot::MakeLink();
  projectairsim::Robot::LoadLink(link, json);
}

TEST(Link, IsLoaded) {
  json json =
      "{ \
            \"name\": \"a\", \
            \"visual\" : { \
                \"geometry\" : { \
                    \"type\": \"unreal_mesh\", \
                    \"name\" : \"Link_a\" \
                } \
            } \
        }"_json;

  auto link = projectairsim::Robot::MakeLink();
  projectairsim::Robot::LoadLink(link, json);
  EXPECT_TRUE(link.IsLoaded());
}

TEST(Link, GetID) {
  json json =
      "{ \
            \"name\": \"a\", \
            \"visual\" : { \
                \"geometry\" : { \
                    \"type\": \"unreal_mesh\", \
                    \"name\" : \"Link_a\" \
                } \
            } \
        }"_json;

  auto link = projectairsim::Robot::MakeLink();
  projectairsim::Robot::LoadLink(link, json);
  EXPECT_EQ(link.GetID(), "a");
}
