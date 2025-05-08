// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_VIEWPORT_CAMERA_IMPL_HPP_
#define CORE_SIM_VIEWPORT_CAMERA_IMPL_HPP_

#include <string>

#include "component.hpp"
#include "core_sim/actor.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/message/float_message.hpp"
#include "core_sim/message/int_list_message.hpp"
#include "core_sim/message/pose_message.hpp"
#include "core_sim/service_manager.hpp"
#include "core_sim/transforms/transform.hpp"
#include "string_utils.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class ViewportCameraImpl : public ComponentWithTopicsAndServiceMethods {
 public:
  ViewportCameraImpl(const std::string& id, const std::string& component,
                     const Logger& logger, const TopicManager& topic_manager,
                     const std::string& parent_topic_path,
                     const ServiceManager& service_manager,
                     const StateManager& state_manager)
      : ComponentWithTopicsAndServiceMethods(component, logger, id,
                                             topic_manager, parent_topic_path,
                                             service_manager, state_manager) {
    SetTopicPath();
    CreateTopics();
  }

  // destructor
  ~ViewportCameraImpl() {
    // Unregister all topics
    // TODO: Not working, why?
    /* for (auto& topic : topics_) {
       topic_manager_.UnregisterTopic(topic);
     }
     */
  }

  void RegisterAndSubscribeTopic(
      Topic& topic,
      std::function<void(const Topic&, const Message&)> callback) {
    topic_manager_.RegisterTopic(topic);

    topic_manager_.SubscribeTopic(topic, callback);

    topics_.push_back(topic);
  }

  void CreateTopics(void) {
    // Add camera_pose topic for setting the camera pose
    auto pose_topic_ = Topic("desired_pose", topic_path_,
                             TopicType::kSubscribed, 0, MessageType::kPose);
    auto callback = [this](const Topic& topic, const Message& message) {
      OnTopicCameraPose(topic, message);
    };
    RegisterAndSubscribeTopic(pose_topic_, callback);

    // Add camera_aspect_ratio topic for setting the camera width and height
    auto aspect_ratio_topic_ =
        Topic("aspect_ratio", topic_path_, TopicType::kSubscribed, 0,
              MessageType::kFloat);
    auto callback_wh = [this](const Topic& topic, const Message& message) {
      OnTopicCameraAspectRatio(topic, message);
    };
    RegisterAndSubscribeTopic(aspect_ratio_topic_, callback_wh);

    //
    auto zoom_topic_ = Topic("zoom", topic_path_, TopicType::kSubscribed, 0,
                             MessageType::kFloat);
    auto callback_zoom = [this](const Topic& topic, const Message& message) {
      OnTopicCameraZoom(topic, message);
    };
    RegisterAndSubscribeTopic(zoom_topic_, callback_zoom);
  }

  void OnTopicCameraPose(const Topic& topic, const Message& message) {
    auto pose_msg = static_cast<const PoseMessage&>(message);
    Pose pose;
    pose.position = pose_msg.GetPosition();
    pose.orientation = pose_msg.GetOrientation();
    callback_pose_updated_(pose);
  }

  void OnTopicCameraAspectRatio(const Topic& topic, const Message& message) {
    auto aspect_ratio_msg = static_cast<const FloatMessage&>(message);
    auto aspect_ratio = aspect_ratio_msg.GetValue();
    callback_aspect_ratio_updated_(aspect_ratio);
  }

  void OnTopicCameraZoom(const Topic& topic, const Message& message) {
    auto zoom_msg = static_cast<const FloatMessage&>(message);
    float zoom = zoom_msg.GetValue();
    callback_zoom_updated_(zoom);
  }

  void SetCallbackPoseUpdated(
      const std::function<void(const Pose&)>& callback) {
    callback_pose_updated_ = callback;
  }

  void SetCallbackAspectRatioUpdated(
      const std::function<void(float)>& callback) {
    callback_aspect_ratio_updated_ = callback;
  }

  void SetCallbackZoomUpdated(const std::function<void(float)>& callback) {
    callback_zoom_updated_ = callback;
  }

 protected:
  Pose pose_;
  std::function<void(const Pose&)> callback_pose_updated_;
  std::function<void(float)> callback_aspect_ratio_updated_;
  std::function<void(float)> callback_zoom_updated_;

 private:
  // Pose Topic
  Topic pose_topic_;
  // Width Height Topic
  Topic aspect_ratio_topic_;
  // Zoom Topic
  Topic zoom_topic_;
  std::vector<std::reference_wrapper<Topic>> topics_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_VIEWPORT_CAMERA_IMPL_HPP_
