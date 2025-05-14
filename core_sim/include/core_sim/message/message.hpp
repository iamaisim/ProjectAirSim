// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_MESSAGE_HPP_

#include <memory>
#include <string>

namespace microsoft {
namespace projectairsim {

class MessageImpl;

enum class MessageType {
  kTopicList = 0,
  kInt8 = 1,
  kJointState = 2,
  kImage = 3,
  kCameraInfo = 4,
  kPosestamped = 5,
  kFlightControlSetpoint = 6,
  kImu = 7,
  kTransform = 8,
  kRequest = 9,
  kResponseSuccess = 10,
  kResponseFailure = 11,
  kBarometer = 12,
  kCollisionInfo = 13,
  kMagnetometer = 14,
  kLidar = 15,
  kGps = 16,
  kKinematics = 17,
  kRadarDetection = 18,
  kRadarTrack = 19,
  kAirspeed = 20,
  kImageAnnotation = 21,
  kFlightControlRCInput = 22,
  kBattery = 23,
  kPhysicsState = 24,
  kDistanceSensor = 25,
  kRotorInfo = 26,
  kReadyState = 27,
  kPose = 28,
  kIntList = 29,
  kFloat = 30,
  kInt32
};

class Message {
 public:
  virtual ~Message();

  MessageType GetType() const;

  virtual std::string Serialize() const;

  virtual void Deserialize(const std::string& buffer);

 protected:
  explicit Message(const std::shared_ptr<MessageImpl>& pimpl);

  std::shared_ptr<MessageImpl> pimpl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_MESSAGE_HPP_
