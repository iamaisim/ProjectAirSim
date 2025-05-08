// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/viewport_camera.hpp"

#include "viewport_camera_impl.hpp"

using namespace microsoft::projectairsim;

ViewportCamera::ViewportCamera(
    const std::string& id, const std::string& component, const Logger& logger,
    const TopicManager& topic_manager, const std::string& parent_topic_path,
    const ServiceManager& service_manager, const StateManager& state_manager) {
  pimpl_ = std::make_shared<ViewportCameraImpl>(
      id, component, logger, topic_manager, parent_topic_path, service_manager,
      state_manager);
}

void ViewportCamera::CreateTopics() { pimpl_->CreateTopics(); }

void ViewportCamera::SetCallbackPoseUpdated(
    const std::function<void(const Pose&)>& callback) {
  pimpl_->SetCallbackPoseUpdated(callback);
}

void ViewportCamera::SetCallbackAspectRatioUpdated(
    const std::function<void(float)>& callback) {
  pimpl_->SetCallbackAspectRatioUpdated(callback);
}

void ViewportCamera::SetCallbackZoomUpdated(
    const std::function<void(float)>& callback) {
  pimpl_->SetCallbackZoomUpdated(callback);
}
