// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "Types.h"

#include <chrono>

#include "pch.h"

namespace microsoft {
namespace projectairsim {
namespace client {

ASC_DECL size_t BoolArray::Cf(void) const {
  return ((piref_counted_ == nullptr)
              ? 0
              : static_cast<const IBuffer*>(piref_counted_)->Cf());
}

ASC_DECL void BoolArray::Clear(void) { RefCountedContainer::Clear(); }

ASC_DECL void BoolArray::SetBuffer(IBuffer* pibuffer) {
  RefCountedContainer::SetPIRefCounted(pibuffer);
}

ASC_DECL bool BoolArray::operator[](size_t ibool) const {
  return ((piref_counted_ == nullptr)
              ? false
              : static_cast<const IBuffer*>(piref_counted_)->operator[](ibool));
}

ASC_DECL ColorRGBA::ColorRGBA(void) : red(0), green(0), blue(0), alpha(1.0f) {}

ASC_DECL ColorRGBA::ColorRGBA(float red_in, float green_in, float blue_in,
                              float alpha_in)
    : red(red_in), green(green_in), blue(blue_in), alpha(alpha_in) {}

ASC_DECL GeoPosition::GeoPosition(void)
    : altitude(0), latitude(0), longitude(0) {}

ASC_DECL Pose::Pose(void) noexcept : position(), orientation() {}

ASC_DECL Pose::Pose(const Vector3 translation_in,
                    const Quaternion rotation_in) noexcept
    : position(translation_in), orientation(rotation_in) {}

ASC_DECL Quaternion::Quaternion(void) noexcept : w(1), x(0), y(0), z(0) {}

ASC_DECL Quaternion::Quaternion(float w_in, float x_in, float y_in,
                                float z_in) noexcept
    : w(w_in), x(x_in), y(y_in), z(z_in) {}

ASC_DECL Transform::Transform(void) noexcept
    : timestamp(
          std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::high_resolution_clock::now().time_since_epoch())
              .count()),
      frame_id("DEFAULT_FRAME"),
      translation(),
      rotation() {}

ASC_DECL Transform::Transform(const Vector3 translation_in,
                              const Quaternion rotation_in) noexcept
    : timestamp(
          (std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
               .count())),
      frame_id("DEFAULT_FRAME"),
      translation(translation_in),
      rotation(rotation_in) {}

ASC_DECL Vector3::Vector3(void) noexcept : x(0), y(0), z(0) {}

ASC_DECL Vector3::Vector3(float x_in, float y_in, float z_in) noexcept
    : x(x_in), y(y_in), z(z_in) {}

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
