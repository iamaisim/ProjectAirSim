// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_TOPIC_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_TOPIC_HPP_

#include <memory>
#include <string>

#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

enum class TopicType { kPublished = 0, kSubscribed = 1 };

class Topic {
 public:
  Topic();

  const std::string& GetName() const;

  const std::string& GetPath() const;

  TopicType GetType() const;

  int GetFrequency() const;

  MessageType GetMessageType() const;

  bool IsEmpty() const;

 private:
  friend class Robot;
  friend class Link;
  friend class Joint;
  friend class Camera;
  friend class Imu;
  friend class TopicManager;
  friend class SimpleFlightApi;
  friend class AirspeedSensor;
  friend class Barometer;
  friend class Magnetometer;
  friend class Lidar;
  friend class DistanceSensor;
  friend class Radar;
  friend class Gps;
  friend class EnvActor;
  friend class Battery;
  friend class ViewportCameraImpl;

  Topic(const std::string& name, const std::string& path, TopicType type,
        int frequency, MessageType message_type);

  void SetFrequency(int value);

  class Impl;
  std::shared_ptr<Impl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_TOPIC_HPP_
