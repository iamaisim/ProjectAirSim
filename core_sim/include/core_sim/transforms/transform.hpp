// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_TRANSFORMS_TRANSFORM_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_TRANSFORMS_TRANSFORM_HPP_

#include <string>

#include "core_sim/clock.hpp"
#include "core_sim/transforms/transform_utils.hpp"

namespace microsoft {
namespace projectairsim {

class Transform {
 public:
  //! Time stamp associated with the transform; default to current t in nanos at
  //! construction time
  TimeNano timestamp_ = SimClock::Get()->NowSimNanos();
  //! The frame ID associated with the transform
  std::string frame_id_ = "DEFAULT_ID";
  Vector3 translation_ = Vector3(0, 0, 0);
  Quaternion rotation_ = Quaternion::Identity();

  Transform() : frame_id_("DEFAULT_ID") {}
  Transform(const Vector3& translation, const Quaternion& rotation)
      : translation_(translation), rotation_(rotation) {}
  Transform(const TimeNano& timestamp, const std::string& frame_id,
            const Vector3& translation, const Quaternion& rotation)
      : timestamp_(timestamp),
        frame_id_(frame_id),
        translation_(translation),
        rotation_(rotation) {}
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_TRANSFORMS_TRANSFORM_HPP_
