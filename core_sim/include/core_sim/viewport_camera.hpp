// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_VIEWPORT_CAMERA_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_VIEWPORT_CAMERA_HPP_

// The viewport camera, when enabled, is used to render the scene from the
// perspective of the camera. The camera is not attached to any vehicle and
// is not affected by the physics engine. The camera is controlled by topics
// that can be used to set the position, orientation, zoom, aspect ratio
// of the camera, as well as enabling/disabling it as a streaming camera.
#include <memory>
#include <string>

#include "core_sim/actor.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
#include "core_sim/transforms/transform.hpp"

namespace microsoft {
namespace projectairsim {

class ViewportCameraImpl;
class TopicManager;
class StateManager;

class ViewportCamera {
 public:
  ViewportCamera(const std::string& id,
            const std::string& component, const Logger& logger,
            const TopicManager& topic_manager,
            const std::string& parent_topic_path,
            const ServiceManager& service_manager,
            const StateManager& state_manager);
  void CreateTopics();

  void SetCallbackPoseUpdated(const std::function<
    void(const Pose&)>& callback); 
  
  void SetCallbackAspectRatioUpdated(const std::function<
    void(float)>& callback);

  void SetCallbackZoomUpdated(const std::function<
    void(float)>& callback);

 protected:
  std::shared_ptr<ViewportCameraImpl> pimpl_;

};

}  // namespace projectairsim
}  // namespace microsoft

#endif
