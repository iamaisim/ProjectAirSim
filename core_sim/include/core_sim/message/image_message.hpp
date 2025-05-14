// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_IMAGE_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_IMAGE_MESSAGE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/message/message.hpp"
#include "json.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

struct BBox2D {
  Vector2 center;
  Vector2 size;

  BBox2D() {}
  BBox2D(const Vector2& in_center, const Vector2& in_size)
      : center(in_center), size(in_size) {}
  BBox2D(const Box2f& eigenBox)
      : center(eigenBox.center()), size(eigenBox.sizes()) {}
};

struct BBox3D {
  Vector3 center;
  Vector3 size;
  Quaternion quaternion;

  BBox3D() {}
  BBox3D(const Vector3& in_center, const Vector3& in_size,
         const Quaternion& in_quat)
      : center(in_center), size(in_size), quaternion(in_quat) {}
};

struct Annotation {
  std::string object_id;
  // TODO: add class label. Add to json_utils too.
  BBox2D bbox2D;
  BBox3D bbox3D;
  std::vector<Vector2> bbox3d_in_image_space;
  std::vector<Vector3> bbox3d_in_projection_space;

  Annotation(const std::string& in_id) : object_id(in_id) {}
  Annotation(const std::string& in_id, const BBox2D& in_bbox2d,
             const BBox3D& in_bbox3d)
      : object_id(in_id),
        bbox2D(in_bbox2d),
        bbox3D(in_bbox3d),
        bbox3d_in_image_space(0),
        bbox3d_in_projection_space(0) {}
  Annotation(const std::string& in_id, const BBox2D& in_bbox2d,
             const BBox3D& in_bbox3d,
             const std::vector<Vector2>& in_bbox3d_in_image_space)
      : object_id(in_id),
        bbox2D(in_bbox2d),
        bbox3D(in_bbox3d),
        bbox3d_in_image_space(in_bbox3d_in_image_space),
        bbox3d_in_projection_space(0) {}
  Annotation(const std::string& in_id, const BBox2D& in_bbox2d,
             const BBox3D& in_bbox3d,
             const std::vector<Vector2>& in_bbox3d_in_image_space,
             const std::vector<Vector3>& in_bbox3d_in_projection_space)
      : object_id(in_id),
        bbox2D(in_bbox2d),
        bbox3D(in_bbox3d),
        bbox3d_in_image_space(in_bbox3d_in_image_space),
        bbox3d_in_projection_space(in_bbox3d_in_projection_space) {}
};

class ImageMessage : public Message {
 public:
  ImageMessage(TimeNano time_stamp_val, uint32_t height_val, uint32_t width_val,
               const std::string& encoding_val, bool big_endian_val,
               uint32_t step_val, std::vector<uint8_t>&& data_val,
               std::vector<float>&& data_float_val, float pos_x_val, 
               float pos_y_val, float pos_z_val, float rot_w_val, 
               float rot_x_val, float rot_y_val, float rot_z_val, 
               std::vector<Annotation> annotations = std::vector<Annotation>()
               );

  ImageMessage();

  ~ImageMessage() override;

  TimeNano GetTimestamp() const;

  uint32_t GetHeight() const;

  uint32_t GetWidth() const;

  const std::string& GetEncoding() const;

  uint32_t IsBigEndian() const;

  uint32_t GetStep() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

  json GetData() const;

  std::vector<uint8_t>& GetPixelVector();

  const std::vector<Annotation> GetAnnotations() const;

  std::vector<float> GetPositionData() const;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_IMAGE_MESSAGE_HPP_
