// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "core_sim/simulator.hpp"

#include <exception>
#include <fstream>

#include "component.hpp"
#include "core_sim/error.hpp"
#include "core_sim/file_utils.hpp"
#include "core_sim/json_utils.hpp"
#include "core_sim/scene.hpp"
#include "json.hpp"

using namespace nlohmann;
namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

static auto default_logger_callback = [](const std::string&, LogLevel,
                                         const std::string&) {};

class Simulator::Loader {
 public:
  explicit Loader(Simulator::Impl& impl);

  void LoadSimulator(const json& config_json);

  void UnloadSimulator();

  void LoadSceneWithJSON(const std::string& json_data);

  void UnloadScene();

 private:
  void LoadID(const json& config_json);

  void LoadTopicSettings(const json& config_json);

  void LoadServiceSettings(const json& config_json);

  void LoadDefaultScene(const json& config_json);

  Simulator::Impl& impl_;
};

class Simulator::Impl : public ComponentWithTopicsAndServiceMethods {
 public:
  explicit Impl(LoggerCallback logger_callback,
                LogLevel level = LogLevel::kError);

  void LoadSimulator(const std::string& sim_json,
                     std::string client_auth_public_key);

  void LoadSceneWithJSON(const std::string& json_data);

  void UnloadScene();

  void UnloadSimulator();

  void StartSimulator();

  void StopSimulator();

  void StartScene();

  void StopScene();

  Logger& GetLogger();

  Scene& GetScene();

  const std::string& GetSceneConfigJSON() const;

  void RegisterServiceMethod(const ServiceMethod& method,
                             MethodHandler method_handler);

  void RegisterServiceMethods();

 private:
  friend class Simulator::Loader;

  bool SetInteractiveFeature(const std::string& feature_id, bool enable);

  Simulator::Loader loader_;
  Scene sim_scene_;
  std::string default_scene_config_;
  std::string scene_config_;

  static constexpr TimeNano kSceneTickPeriod = 3000000;  // 3 ms = 333 Hz
};

// -----------------------------------------------------------------------------
// class Simulator

Simulator::Simulator()
    : pimpl_(std::make_shared<Impl>(default_logger_callback)) {}

Simulator::Simulator(LoggerCallback logger_callback, LogLevel level)
    : pimpl_(std::make_shared<Impl>(logger_callback, level)) {}

void Simulator::LoadSimulator(const std::string& sim_json,
                              std::string client_auth_public_key) {
  pimpl_->LoadSimulator(sim_json, client_auth_public_key);
}

void Simulator::UnloadSimulator() { pimpl_->UnloadSimulator(); }

void Simulator::LoadSceneWithJSON(const std::string& json_data) {
  pimpl_->LoadSceneWithJSON(json_data);
}

void Simulator::UnloadScene() { pimpl_->UnloadScene(); }

bool Simulator::IsLoaded() { return pimpl_->IsLoaded(); }

void Simulator::StartSimulator() { return pimpl_->StartSimulator(); }

void Simulator::StopSimulator() { return pimpl_->StopSimulator(); }

void Simulator::StartScene() {
  // Register service methods each time a scene is loaded
  // (since service manager unregisters them each scene unload)
  pimpl_->RegisterServiceMethods();
  return pimpl_->StartScene();
}

void Simulator::StopScene() { return pimpl_->StopScene(); }

const std::string& Simulator::GetID() { return pimpl_->GetID(); }

Logger& Simulator::GetLogger() { return pimpl_->GetLogger(); }

Scene& Simulator::GetScene() { return pimpl_->GetScene(); }

const std::string& Simulator::GetSceneConfigJSON() const {
  return pimpl_->GetSceneConfigJSON();
}

void Simulator::RegisterServiceMethod(const ServiceMethod& method,
                                      MethodHandler method_handler) {
  pimpl_->RegisterServiceMethod(method, method_handler);
}

// -----------------------------------------------------------------------------
// class Simulator::Impl

Simulator::Impl::Impl(LoggerCallback logger_callback, LogLevel level)
    : ComponentWithTopicsAndServiceMethods(Constant::Component::simulator,
                                           Logger(logger_callback, level)),
      loader_(*this) {
  client_authorization_.SetLogger(GetLogger());
}

void Simulator::Impl::LoadSimulator(const std::string& sim_json,
                                    std::string client_auth_public_key) {
  nlohmann::json sim_config;

  std::stringstream(sim_json) >> sim_config;

  loader_.LoadSimulator(sim_config);

  logger_.LogVerbose(name_, "Setting client authorization public key to \"%s\"",
                     client_auth_public_key.c_str());
  client_authorization_.SetPublicKey(client_auth_public_key.data(),
                                     client_auth_public_key.size());
}

void Simulator::Impl::UnloadSimulator() { loader_.UnloadSimulator(); }

void Simulator::Impl::LoadSceneWithJSON(const std::string& json_data) {
  loader_.LoadSceneWithJSON(json_data);
}

void Simulator::Impl::UnloadScene() { loader_.UnloadScene(); }

void Simulator::Impl::StartSimulator() {
  topic_manager_.Start();
  service_manager_.Start();
}

void Simulator::Impl::StopSimulator() {
  service_manager_.Stop();
  topic_manager_.Stop();
}

void Simulator::Impl::StartScene() {
  sim_scene_.Start();
  // Update topic list after scene loads components that register topics
  topic_manager_.CreateTopicList();
}

void Simulator::Impl::StopScene() { sim_scene_.Stop(); }

Logger& Simulator::Impl::GetLogger() { return logger_; }

Scene& Simulator::Impl::GetScene() { return sim_scene_; }

const std::string& Simulator::Impl::GetSceneConfigJSON() const {
  return scene_config_;
}

//! Helper method to register external Service Methods at the simulator level
void Simulator::Impl::RegisterServiceMethod(const ServiceMethod& method,
                                            MethodHandler method_handler) {
  auto method_name = method.GetName();
  auto params_list = method.GetParamsList();
  // Re-use topic path to create a unique name for the
  // service method; Added to keep the naming consistent throughout even if
  // there will atmost be only one instance of simulator
  auto unique_method_name = topic_path_ + "/" + method_name;
  auto unique_method = ServiceMethod(unique_method_name, params_list);

  service_manager_.RegisterMethod(unique_method, method_handler);
}

bool Simulator::Impl::SetInteractiveFeature(const std::string& feature_id, bool enable) {
  if(feature_id == "viewport_camera"){
    sim_scene_.EnableViewportCamera(enable);
    return true;
  }
  return false;
}

void Simulator::Impl::RegisterServiceMethods() {
  /*
  auto unsubscribe_topic = ServiceMethod("Unsubscribe", {"topic_paths"});
  auto unsubscribe_topic_handler = unsubscribe_topic.CreateMethodHandler(
      &TopicManager::Unsubscribe, topic_manager_);
  RegisterServiceMethod(unsubscribe_topic, unsubscribe_topic_handler);
  */
 
  client_authorization_.RegisterServiceMethods(
      topic_path_,
      [this](const ServiceMethod& method, MethodHandler method_handler) {
        RegisterServiceMethod(method, method_handler);
      });
  
  // Register service method SetInteractiveFeature( feature_id(str), enable(bool))
  auto set_interactive_feature = ServiceMethod("SetInteractiveFeature", {"feature_id", "enable"});
  auto set_interactive_feature_handler = set_interactive_feature.CreateMethodHandler(
      &Simulator::Impl::SetInteractiveFeature, *this);
  RegisterServiceMethod(set_interactive_feature, set_interactive_feature_handler);

}

// -----------------------------------------------------------------------------
// class Simulator::Loader

Simulator::Loader::Loader(Simulator::Impl& impl) : impl_(impl) {}

void Simulator::Loader::LoadSimulator(const json& config_json) {
  impl_.logger_.LogTrace(impl_.name_, "Loading Simulator.");

  LoadID(config_json);
  LoadTopicSettings(config_json);
  LoadServiceSettings(config_json);
  LoadDefaultScene(config_json);

  impl_.logger_.LogTrace(impl_.name_, "[%s] Simulator successfully loaded.",
                         impl_.id_.c_str());
  impl_.is_loaded_ = true;
}

void Simulator::Loader::UnloadSimulator() {
  impl_.logger_.LogTrace(impl_.name_, "[%s] Unloading Simulator.",
                         impl_.id_.c_str());

  impl_.sim_scene_.Unload();
  impl_.is_loaded_ = false;

  impl_.logger_.LogTrace(impl_.name_, "[%s] Simulator successfully unloaded.",
                         impl_.id_.c_str());
}

void Simulator::Loader::LoadID(const json& config_json) {
  impl_.logger_.LogVerbose(impl_.name_, "Load 'id'.");

  impl_.id_ = JsonUtils::GetIdentifier(config_json, Constant::Config::id);
  impl_.SetTopicPath();

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'id' loaded.", impl_.id_.c_str());
}

void Simulator::Loader::LoadTopicSettings(const json& config_json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Load 'topics'.",
                           impl_.id_.c_str());

  auto topics_json =
      JsonUtils::GetJsonObject(config_json, Constant::Config::topics);

  impl_.topic_manager_.Load(topics_json);

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'topics' loaded.",
                           impl_.id_.c_str());
}

void Simulator::Loader::LoadServiceSettings(const json& config_json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Load 'services'.",
                           impl_.id_.c_str());

  auto services_json =
      JsonUtils::GetJsonObject(config_json, Constant::Config::services);
  impl_.service_manager_.Load(services_json);
  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'services' loaded.",
                           impl_.id_.c_str());
}

void Simulator::Loader::LoadDefaultScene(const json& config_json) {
  impl_.logger_.LogVerbose(impl_.name_, "[%s] Load 'default-scene'.",
                           impl_.id_.c_str());

  impl_.default_scene_config_ =
      JsonUtils::GetString(config_json, Constant::Config::default_scene);

  impl_.scene_config_ = impl_.default_scene_config_;

  if (impl_.default_scene_config_.length() == 0) {
    impl_.logger_.LogError(impl_.name_,
                           "[%s] 'default-scene' is missing or empty.",
                           impl_.id_.c_str());
    throw Error("Simulator 'default-scene' missing or empty.");
  }

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'default-scene' loaded.",
                           impl_.id_.c_str());
}

void Simulator::Loader::LoadSceneWithJSON(const std::string& json_data) {
  if (json_data.length() == 0) {
    impl_.scene_config_ = impl_.default_scene_config_;
    impl_.logger_.LogVerbose(impl_.name_, "Loading Default Scene");
  } else {
    impl_.scene_config_ = json_data;
    impl_.logger_.LogVerbose(impl_.name_, "Loading Scene with JSON Data.");
  }

  json config_data;

  std::stringstream(impl_.scene_config_) >> config_data;

  impl_.sim_scene_ =
      Scene(impl_.logger_, impl_.topic_manager_, impl_.topic_path_,
            impl_.service_manager_, impl_.state_manager_);

  impl_.state_manager_.Initialize(&impl_.sim_scene_);

  impl_.sim_scene_.LoadWithJSON(config_data);

  impl_.logger_.LogVerbose(impl_.name_, "[%s] 'scene' loaded.",
                           impl_.id_.c_str());
}

void Simulator::Loader::UnloadScene() { impl_.sim_scene_.Unload(); }

}  // namespace projectairsim
}  // namespace microsoft
