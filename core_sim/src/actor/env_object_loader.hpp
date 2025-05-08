// env_object_loader.hpp

#ifndef CORE_SIM_ACTOR_ENV_OBJECT_LOADER_HPP_
#define CORE_SIM_ACTOR_ENV_OBJECT_LOADER_HPP_

#include "core_sim/actor/env_object.hpp"

#include "json.hpp"

using json = nlohmann::json;

namespace microsoft {
namespace projectairsim {

class EnvObject; // Forward declaration

class EnvObject::Loader {
 public:
  explicit Loader(EnvObject::Impl& impl);

  void Load(const json& json);

 private:
  void LoadVisual(const json& json);

  EnvObject::Impl& impl_;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_ACTOR_ENV_OBJECT_LOADER_HPP_
