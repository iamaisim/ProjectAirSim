// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM_SRC_ALGORITHMS_HPP_
#define CORE_SIM_SRC_ALGORITHMS_HPP_

#include <unordered_map>
#include <vector>

namespace microsoft {
namespace projectairsim {

template <typename Value>
auto ToRefs(const std::vector<Value>& container) {
  std::vector<std::reference_wrapper<typename Value::element_type>> refs;

  refs.reserve(container.size());
  std::transform(container.begin(), container.end(), std::back_inserter(refs),
                 [](auto& x) { return std::ref(*x); });

  return refs;
}

template <typename Key, typename Value>
auto ToRefs(const std::unordered_map<Key, Value>& container) {
  std::unordered_map<std::string,
                     std::reference_wrapper<typename Value::element_type>>
      refs;

  for (auto& [actuator_id, actuator_uptr] : container) {
    refs.emplace(actuator_id, std::ref(*actuator_uptr));
  }

  return refs;
}

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_SRC_ALGORITHMS_HPP_
