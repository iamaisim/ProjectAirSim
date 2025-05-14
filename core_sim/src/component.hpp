// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef CORE_SIM_SRC_COMPONENT_HPP_
#define CORE_SIM_SRC_COMPONENT_HPP_

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include "constant.hpp"
#include "core_sim/client_authorization.hpp"
#include "core_sim/json_utils.hpp"
#include "core_sim/logger.hpp"
#include "core_sim/service_manager.hpp"
#include "state_manager.hpp"
#include "topic_manager.hpp"

namespace microsoft {
namespace projectairsim {

class Component {
 public:
  bool IsLoaded() const { return is_loaded_; }

 protected:
  Component(const std::string& name, const Logger& logger)
      : name_(name), logger_(logger), is_loaded_(false) {}

  Logger logger_;
  std::string name_;
  bool is_loaded_;
};

class ComponentWithID : public Component {
 public:
  const std::string& GetID() const { return id_; }

 protected:
  ComponentWithID(const std::string& name, const Logger& logger)
      : Component(name, logger) {}

  ComponentWithID(const std::string& name, const Logger& logger,
                  const std::string& id)
      : Component(name, logger), id_(id) {}

  std::string id_;
};

class ComponentWithTopics : public ComponentWithID {
 public:
  void BeginUpdate() {
    std::lock_guard<std::mutex> lock(update_lock_);
    OnBeginUpdate();
    update_enabled_ = true;
  }

  void EndUpdate() {
    std::lock_guard<std::mutex> lock(update_lock_);
    update_enabled_ = false;
    OnEndUpdate();
  }

  bool IsUpdating() const { return update_enabled_; }

 protected:
  ComponentWithTopics(const std::string& name, const Logger& logger,
                      const TopicManager& topic_manager,
                      const std::string& parent_topic_path)
      : ComponentWithID(name, logger),
        client_authorization_(),
        topic_manager_(topic_manager),
        update_enabled_(false),
        parent_topic_path_(parent_topic_path) {}

  ComponentWithTopics(const std::string& name, const Logger& logger,
                      const std::string& id, const TopicManager& topic_manager,
                      const std::string& parent_topic_path)
      : ComponentWithID(name, logger, id),
        client_authorization_(),
        topic_manager_(topic_manager),
        update_enabled_(false),
        parent_topic_path_(parent_topic_path) {}

  ComponentWithTopics(const std::string& name, const Logger& logger)
      : ComponentWithID(name, logger),
        client_authorization_(),
        topic_manager_(logger, client_authorization_),
        update_enabled_(false) {}

  virtual void OnBeginUpdate() {}

  virtual void OnEndUpdate() {}

  void SetTopicPath() { topic_path_ = parent_topic_path_ + "/" + id_; }

  ClientAuthorization client_authorization_;
  std::string parent_topic_path_;
  std::string topic_path_;
  TopicManager topic_manager_;
  std::atomic<bool> update_enabled_;
  std::mutex update_lock_;
};

class ComponentWithServiceMethods : public ComponentWithID {
 public:
  void BeginUpdate() {
    std::lock_guard<std::mutex> lock(update_lock_);
    OnBeginUpdate();
    update_enabled_ = true;
  }

  void EndUpdate() {
    std::lock_guard<std::mutex> lock(update_lock_);
    update_enabled_ = false;
    OnEndUpdate();
  }

  bool IsUpdating() const { return update_enabled_; }

 protected:
  ComponentWithServiceMethods(const std::string& name, const Logger& logger,
                              const ServiceManager& service_manager)
      : ComponentWithID(name, logger),
        service_manager_(service_manager),
        update_enabled_(false) {}

  ComponentWithServiceMethods(const std::string& name, const Logger& logger,
                              const std::string& id,
                              const ServiceManager& service_manager)
      : ComponentWithID(name, logger, id),
        service_manager_(service_manager),
        update_enabled_(false) {}

  ComponentWithServiceMethods(const std::string& name, const Logger& logger)
      : ComponentWithID(name, logger),
        client_authorization_(),
        service_manager_(logger, client_authorization_),
        update_enabled_(false) {}

  virtual void OnBeginUpdate() {}

  virtual void OnEndUpdate() {}

  ClientAuthorization client_authorization_;
  ServiceManager service_manager_;
  std::atomic<bool> update_enabled_;
  std::mutex update_lock_;
};

class ComponentWithTopicsAndServiceMethods : public ComponentWithID {
 public:
  void BeginUpdate() {
    std::lock_guard<std::mutex> lock(update_lock_);
    OnBeginUpdate();
    update_enabled_ = true;
  }

  void EndUpdate() {
    std::lock_guard<std::mutex> lock(update_lock_);
    update_enabled_ = false;
    OnEndUpdate();
  }

  bool IsUpdating() const { return update_enabled_; }

 protected:
  ComponentWithTopicsAndServiceMethods(const std::string& name,
                                       const Logger& logger,
                                       const TopicManager& topic_manager,
                                       const std::string& parent_topic_path,
                                       const ServiceManager& service_manager,
                                       const StateManager& state_manager)
      : ComponentWithID(name, logger),
        topic_manager_(topic_manager),
        service_manager_(service_manager),
        state_manager_(state_manager),
        update_enabled_(false),
        parent_topic_path_(parent_topic_path) {}

  ComponentWithTopicsAndServiceMethods(
      const std::string& name, const Logger& logger, const std::string& id,
      const TopicManager& topic_manager, const std::string& parent_topic_path,
      const ServiceManager& service_manager, const StateManager& state_manager)
      : ComponentWithID(name, logger, id),
        topic_manager_(topic_manager),
        service_manager_(service_manager),
        state_manager_(state_manager),
        update_enabled_(false),
        parent_topic_path_(parent_topic_path) {}

  ComponentWithTopicsAndServiceMethods(const std::string& name,
                                       const Logger& logger)
      : ComponentWithID(name, logger),
        client_authorization_(),
        topic_manager_(logger, client_authorization_),
        service_manager_(logger, client_authorization_),
        state_manager_(logger),
        update_enabled_(false) {}

  virtual void OnBeginUpdate() {}

  virtual void OnEndUpdate() {}

  void SetTopicPath() { topic_path_ = parent_topic_path_ + "/" + id_; }

  ClientAuthorization client_authorization_;
  std::string parent_topic_path_;
  std::string topic_path_;
  TopicManager topic_manager_;
  ServiceManager service_manager_;
  StateManager state_manager_;
  std::atomic<bool> update_enabled_;
  std::mutex update_lock_;
};
}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_COMPONENT_HPP_
