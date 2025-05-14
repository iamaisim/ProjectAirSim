// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef SIMSERVER_INCLUDE_SIMSERVER_HPP_
#define SIMSERVER_INCLUDE_SIMSERVER_HPP_

#include <functional>
#include <memory>
#include <string>

#include "core_sim/simulator.hpp"

namespace microsoft {
namespace projectairsim {

class PhysicsWorld;
class Scene;

namespace rendering {
namespace scene {

class TileManager;

}
}  // namespace rendering

class SimServer {
 public:
  SimServer(Simulator::LoggerCallback logger_callback, LogLevel level);

  ~SimServer();

  void LoadSimulator(int topics_port = 8989, int services_port = 8990,
                     const std::string& client_auth_public_key = std::string());

  void StartSimulator();

  void StopSimulator();

  void UnloadSimulator();

  void LoadScene(const std::string& scene_json = "");

  void StartScene();

  void StopScene();

  void UnloadScene();

  void SetCallbackLoadExternalScene(const std::function<void()>& callback);

  void SetCallbackStartExternalScene(const std::function<void()>& callback);

  void SetCallbackStopExternalScene(const std::function<void()>& callback);

  void SetCallbackUnloadExternalScene(const std::function<void()>& callback);

  std::shared_ptr<Simulator> GetSimulator();

  std::shared_ptr<PhysicsWorld> GetPhysicsWorld();

  std::shared_ptr<rendering::scene::TileManager> GetTileManager();

  Scene& GetScene();

  const std::string& GetSceneConfigJSON() const;

  void RegisterServiceMethods();

 private:
  std::string SimReloadSceneWithJSON(std::string json_data);

  void LoadPhysicsWorld(Scene& scene);

  void UnloadPhysicsWorld();

  void LoadControllers(Scene& scene);

  void UnloadControllers();

  void LoadTileManager(const Scene& scene);

  void UnloadTileManager();

  std::shared_ptr<Simulator> simulator_;
  std::shared_ptr<PhysicsWorld> physics_world_;
  std::shared_ptr<rendering::scene::TileManager> tile_manager_;

  std::function<void()> load_external_scene_callback_;
  std::function<void()> start_external_scene_callback_;
  std::function<void()> stop_external_scene_callback_;
  std::function<void()> unload_external_scene_callback_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // SIMSERVER_INCLUDE_SIMSERVER_HPP_
