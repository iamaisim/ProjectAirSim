// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#include "simserver.hpp"

#include <simple_drive/simple_drive_api.hpp>
#include <simple_flight_api.hpp>
#include <string>

#include "arducopter_api.hpp"
#include "core_sim/simulator.hpp"
#include "manual_controller_api.hpp"
#include "matlab_controller_api.hpp"
#include "mavlink_api.hpp"
#include "physics_world.hpp"
#include "tile_manager.hpp"

namespace microsoft {
namespace projectairsim {

SimServer::SimServer(Simulator::LoggerCallback logger_callback, LogLevel level)
    : simulator_(std::make_shared<Simulator>(logger_callback, level)),
      physics_world_(std::make_shared<PhysicsWorld>()),
      tile_manager_(nullptr),
      load_external_scene_callback_(nullptr),
      start_external_scene_callback_(nullptr),
      stop_external_scene_callback_(nullptr),
      unload_external_scene_callback_(nullptr) {}

SimServer::~SimServer() {}

void SimServer::LoadSimulator(int topics_port, int services_port,
                              const std::string& client_auth_public_key) {
  // Load simulator and default scene
  std::string default_sim_json = R"(
      {
        "id": "Sim",
        "default-scene": "{\"id\": \"DefaultScene\"}",
        "topics": {"ip": "*", "port":)" +
                                 std::to_string(topics_port) + R"(},
        "services": {"ip": "*", "port":)" +
                                 std::to_string(services_port) + R"(}
      }
    )";

  simulator_->LoadSimulator(default_sim_json, client_auth_public_key);

  RegisterServiceMethods();
}

void SimServer::StartSimulator() {
  simulator_->GetLogger().LogVerbose("SimServer::StartSimulator",
                                     "Starting simulator.");
  simulator_->StartSimulator();
}

void SimServer::StopSimulator() {
  simulator_->GetLogger().LogVerbose("SimServer::StopSimulator",
                                     "Stopping simulator.");
  simulator_->StopSimulator();
}

void SimServer::UnloadSimulator() {
  simulator_->GetLogger().LogVerbose("SimServer::UnloadSimulator",
                                     "Unloading simulator.");
  simulator_->UnloadSimulator();
}

void SimServer::LoadScene(const std::string& scene_json) {
  simulator_->GetLogger().LogVerbose("SimServer::LoadScene", "Loading scene.");

  // Step-1: Load core simulation scene
  simulator_->LoadSceneWithJSON(scene_json);

  auto& scene = simulator_->GetScene();

  // Step-2: Load host specific runtime feature components
  // like physics, controller, rendering, etc.
  {
    // Load physics world bodies and set scene tick callbacks
    LoadPhysicsWorld(scene);

    // Load API and Controllers
    LoadControllers(scene);

    // Load tile manager for GIS scenes
    LoadTileManager(scene);
  }
}

void SimServer::StartScene() {
  simulator_->GetLogger().LogVerbose("SimServer::StartScene",
                                     "Starting scene.");
  simulator_->StartScene();
}

void SimServer::StopScene() {
  simulator_->GetLogger().LogVerbose("SimServer::StopScene", "Stopping scene.");
  simulator_->StopScene();
}

void SimServer::UnloadScene() {
  simulator_->GetLogger().LogVerbose("SimServer::UnloadScene",
                                     "Unloading scene.");
  UnloadControllers();
  UnloadPhysicsWorld();
  UnloadTileManager();
  simulator_->UnloadScene();
}

void SimServer::SetCallbackLoadExternalScene(
    const std::function<void()>& callback) {
  load_external_scene_callback_ = callback;
}

void SimServer::SetCallbackStartExternalScene(
    const std::function<void()>& callback) {
  start_external_scene_callback_ = callback;
}

void SimServer::SetCallbackStopExternalScene(
    const std::function<void()>& callback) {
  stop_external_scene_callback_ = callback;
}

void SimServer::SetCallbackUnloadExternalScene(
    const std::function<void()>& callback) {
  unload_external_scene_callback_ = callback;
}

std::shared_ptr<Simulator> SimServer::GetSimulator() { return simulator_; }

const std::string& SimServer::GetSceneConfigJSON() const {
  return simulator_->GetSceneConfigJSON();
}

std::shared_ptr<PhysicsWorld> SimServer::GetPhysicsWorld() {
  return physics_world_;
}

std::shared_ptr<rendering::scene::TileManager> SimServer::GetTileManager() {
  return tile_manager_;
}

Scene& SimServer::GetScene() { return simulator_->GetScene(); }

void SimServer::RegisterServiceMethods() {
  simulator_->GetLogger().LogVerbose("SimServer::RegisterServiceMethods",
                                     "Registering service methods.");

  // Register "LoadScene" as a service method through simulator
  auto sim_reload_scene = ServiceMethod("LoadScene", {"scene_config"});
  auto sim_reload_scene_handler = sim_reload_scene.CreateMethodHandler(
      &SimServer::SimReloadSceneWithJSON, *this);
  simulator_->RegisterServiceMethod(sim_reload_scene, sim_reload_scene_handler);
}

std::string SimServer::SimReloadSceneWithJSON(std::string json_data) {
  simulator_->GetLogger().LogVerbose("SimServer::SimReloadSceneWithJSON",
                                     "Reloading scene.");

  try {
    // --------------------------------------------------------------------
    // Stop current sim scene
    StopScene();

    // Stop current external scene
    std::function<void()> stop_external_scene_func = nullptr;
    stop_external_scene_func = stop_external_scene_callback_;
    if (stop_external_scene_func != nullptr) {
      stop_external_scene_func();
    }

    // --------------------------------------------------------------------
    // Unload current external scene
    std::function<void()> unload_external_scene_func = nullptr;
    unload_external_scene_func = unload_external_scene_callback_;
    if (unload_external_scene_func != nullptr) {
      unload_external_scene_func();
    }

    // Unload current sim scene
    UnloadScene();

    // --------------------------------------------------------------------
    // Load new sim scene
    LoadScene(json_data);

    // Re-register service methods with new scene because all service methods
    // were unregistered during scene stopping
    RegisterServiceMethods();

    // Load new external scene
    std::function<void()> load_external_scene_func = nullptr;
    load_external_scene_func = load_external_scene_callback_;
    if (load_external_scene_func != nullptr) {
      load_external_scene_func();
    }

    // --------------------------------------------------------------------
    // Start new external scene
    std::function<void()> start_external_scene_func = nullptr;
    start_external_scene_func = start_external_scene_callback_;
    if (start_external_scene_func != nullptr) {
      start_external_scene_func();
    }

    // Start new sim scene
    StartScene();
  } catch (const Error& error) {
    simulator_->GetLogger().LogError(
        "SimServer::SimReloadSceneWithJSON",
        "Exception '%s' encountered while reloading the scene.", error.what());

    // Load default scene and re-register service methods to be ready for
    // client to reload a new scene
    LoadScene();
    RegisterServiceMethods();

    throw;
  } catch (const std::exception& exception) {
    simulator_->GetLogger().LogError(
        "SimServer::SimReloadSceneWithJSON",
        "Exception '%s' encountered while reloading the scene.",
        exception.what());

    // Load default scene and re-register service methods to be ready for
    // client to reload a new scene
    LoadScene();
    RegisterServiceMethods();

    throw;
  } catch (...) {
    simulator_->GetLogger().LogError(
        "SimServer::SimReloadSceneWithJSON",
        "Unknown exception encountered while reloading the scene.");

    // Load default scene and re-register service methods to be ready for
    // client to reload a new scene
    LoadScene();
    RegisterServiceMethods();

    throw;
  }

  // Return the new sim scene ID for the client to update topic/service paths
  return simulator_->GetScene().GetID();
}

void SimServer::LoadPhysicsWorld(Scene& scene) {
  simulator_->GetLogger().LogVerbose("SimServer::LoadPhysicsWorld",
                                     "Loading physics bodies for scene '%s'.",
                                     scene.GetID().c_str());

  // Add all robots into physics world
  auto actors = scene.GetActors();
  for (const auto& actor_ref : actors) {
    if (actor_ref.get().GetType() == ActorType::kRobot) {
      const auto& sim_robot = static_cast<const Robot&>(actor_ref.get());
      // Load sim robot into physics world
      physics_world_->AddRobot(sim_robot);
    }
  }

  // Connect callback between sim scene and physics world
  physics_world_->SetSceneTickCallbacks(scene);
}

void SimServer::UnloadPhysicsWorld() {
  simulator_->GetLogger().LogVerbose("SimServer::UnloadPhysicsWorld",
                                     "Unloading physics bodies.");
  physics_world_->RemoveAllBodiesAndModels();  // clear physics world
}

void SimServer::LoadControllers(Scene& scene) {
  simulator_->GetLogger().LogVerbose("SimServer::LoadControllers",
                                     "Loading controllers for scene '%s'.",
                                     scene.GetID().c_str());

  auto actors = scene.GetActors();
  auto* ptransformtree = scene.GetTransformTree();
  for (const auto& actor_ref : actors) {
    if (actor_ref.get().GetType() == ActorType::kRobot) {
      auto& sim_robot = static_cast<Robot&>(actor_ref.get());

      const std::string controller_type = sim_robot.GetControllerType();

      // TODO: Need to figure out how to make this more loosely coupled
      // w.r.t. identifying and loading the class supporting the feature.
      if (controller_type == "simple-flight-api") {
        auto simple_flight_api = new SimpleFlightApi(sim_robot, ptransformtree);
        sim_robot.SetController(
            std::unique_ptr<IController>(simple_flight_api));
      } else if (controller_type == "px4-api") {
        auto px4_api = new MavLinkApi(sim_robot, ptransformtree);
        sim_robot.SetController(std::unique_ptr<IController>(px4_api));
      } else if (controller_type == "ardupilot-api") {
        auto ap_api = new ArduCopterApi(sim_robot, ptransformtree);
        sim_robot.SetController(std::unique_ptr<IController>(ap_api));
      } else if (controller_type == "manual-controller-api") {
        auto manual_controller_api = new ManualControllerApi(sim_robot);
        sim_robot.SetController(
            std::unique_ptr<IController>(manual_controller_api));
      } else if (controller_type == "matlab-controller-api") {
        auto matlab_controller_api = new MatlabControllerApi(sim_robot);
        sim_robot.SetController(
            std::unique_ptr<IController>(matlab_controller_api));
      } else if (controller_type == "simple-drive-api") {
        auto simple_drive_api = new simple_drive::SimpleDriveApi(sim_robot, ptransformtree);
        sim_robot.SetController(std::unique_ptr<IController>(simple_drive_api));
      } else {
        simulator_->GetLogger().LogWarning("SimServer::LoadControllers",
                                           "Unsupported controller type '%s'",
                                           controller_type.c_str());
      }
    }
  }
}

void SimServer::UnloadControllers() {
  simulator_->GetLogger().LogVerbose("SimServer::UnloadControllers",
                                     "Unloading controllers.");
  // Controllers should get destructed by robot holding its unique ptr
}

void SimServer::LoadTileManager(const Scene& scene) {
  if (scene.GetSceneType() != SceneType::kCustomGIS) return;

  simulator_->GetLogger().LogVerbose("SimServer::LoadTileManager",
                                     "Loading tile manager for GIS scene '%s'.",
                                     scene.GetID().c_str());

  const auto& center_geo_point = scene.GetHomeGeoPoint().geo_point;
  const auto& tiles_lod_max = scene.GetTilesLodMax();
  const auto& tiles_lod_min = scene.GetTilesLodMin();

  // TODO Pass tile quality as param from scene config?
  tile_manager_ = std::make_shared<rendering::scene::TileManager>(
      center_geo_point, tiles_lod_min, tiles_lod_max);

  // Add all robots into tile manager
  auto actors = scene.GetActors();
  for (const auto& actor_ref : actors) {
    if (actor_ref.get().GetType() == ActorType::kRobot) {
      const auto& sim_robot = static_cast<const Robot&>(actor_ref.get());
      tile_manager_->AddRobot(sim_robot);
    }
  }
}

void SimServer::UnloadTileManager() {
  simulator_->GetLogger().LogVerbose("SimServer::UnloadTileManager",
                                     "Unloading tile manager.");

  if (tile_manager_) {
    tile_manager_->RemoveAllRobots();
  }

  tile_manager_.reset();
}

}  // namespace projectairsim
}  // namespace microsoft
