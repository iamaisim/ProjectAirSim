// Copyright (C) 2025 IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#ifndef MULTIROTOR_API_TEST_ROBOT_TEST_UTILS_HPP_
#define MULTIROTOR_API_TEST_ROBOT_TEST_UTILS_HPP_

#include "core_sim/actor/robot.hpp"
#include "core_sim/config_json.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class Scene {
 public:
  static Robot MakeRobot(const std::string& id) {
    // Keep test logging quiet.
    auto callback = [](const std::string& component, LogLevel level,
                       const std::string& message) {};
    return MakeRobot(id, Logger(callback));
  }

  static Robot MakeRobot(const std::string& id, Logger logger) {
    Transform origin = {{0, 0, 0}, {1, 0, 0, 0}};
    return Robot(id, origin, logger, TopicManager(logger), "",
                 ServiceManager(logger), StateManager(logger),
                 PROJECTAIRSIM_TEST_PLUGIN_DIR);
  }

  static void LoadRobot(Robot& robot, ConfigJson config_json) {
    robot.Load(config_json);
  }
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_TEST_ROBOT_TEST_UTILS_HPP_
