// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "ASCDecl.h"
#include "AsyncResult.h"
#include "IRefCounted.h"

namespace microsoft {
namespace projectairsim {
namespace client {

// Clock types
using TimeNano = std::int64_t;

// Array of boolean values that can cross the DLL boundary
class BoolArray : public RefCountedContainer {
 public:
  struct IBuffer : public IRefCounted {
   public:
    // Return the number of bools
    virtual std::size_t Cf(void) const = 0;

    // Return the boolean value at the specified index
    virtual bool operator[](size_t ibool) const = 0;
  };  // struct IBuffer

 public:
  // Return the number of bools
  ASC_DECL size_t Cf(void) const;

  // Delete the attached buffer
  ASC_DECL void Clear(void);

  // Set the boolean array buffer
  ASC_DECL void SetBuffer(IBuffer* pibuffer);

  // Return the boolean value at the specified index
  ASC_DECL bool operator[](size_t ibool) const;
};  // class BoolArray

// RGBA Color Value
struct ColorRGBA {
  float red;    // Red color value (0.0 - 1.0)
  float green;  // Green color value (0.0 - 1.0)
  float blue;   // Blue color value (0.0 - 1.0)
  float alpha;  // Alpha channel value (0.0 - 1.0)

  ASC_DECL ColorRGBA(void);
  ASC_DECL ColorRGBA(float red_in, float green_in, float blue_in,
                     float alpha_in);
};  // struct ColorRGBA

typedef std::vector<ColorRGBA> VecColorRGBA;

// Geographic position
struct GeoPosition {
  float altitude;   // Altitude (meters)
  float latitude;   // Latitude (degrees North)
  float longitude;  // Longitude (degrees East)

  ASC_DECL GeoPosition(void);
};  // struct GeoPosition

// Four-element quaternion
struct Quaternion {
  float w;  // Scalar value
  float x;  // i-coordinate
  float y;  // j-coordinate
  float z;  // k-coordinate

  ASC_DECL Quaternion(void) noexcept;
  ASC_DECL Quaternion(float w_in, float x_in, float y_in, float z_in) noexcept;
};  // struct Quaternion

// Three-element vector
struct Vector3 {
  float x;  // X-coordinate
  float y;  // Y-coordinate
  float z;  // Z-coordinate

  ASC_DECL Vector3(void) noexcept;
  ASC_DECL Vector3(float x_in, float y_in, float z_in) noexcept;
};  // struct Vector3

typedef std::vector<Vector3> VecVector3;

// Pose (position and orientation)
struct Pose {
  Vector3 position;        // Position
  Quaternion orientation;  // Orientation

  ASC_DECL Pose(void) noexcept;
  ASC_DECL Pose(const Vector3 position_in,
                const Quaternion orientation_in) noexcept;
};  // struct Pose

typedef std::vector<Pose> VecPose;

// Transform (frame ID, translation, and rotation)
struct Transform {
  TimeNano timestamp;    // Timestamp of when this transform was constructed
  std::string frame_id;  // Frame ID
  Vector3 translation;   // Position
  Quaternion rotation;   // Orientation

  ASC_DECL Transform(void) noexcept;
  ASC_DECL Transform(const Vector3 translation_in,
                     const Quaternion rotation_in) noexcept;
};  // struct Transform

typedef std::vector<Transform> VecTransform;

}  // namespace client
}  // namespace projectairsim
}  // namespace microsoft
