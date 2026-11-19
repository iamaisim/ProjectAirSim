// Copyright (C) Microsoft Corporation.
// Copyright (C) 2026 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

#include "core_sim/message/image_message.hpp"

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "message/common_utils.hpp"
#include "msgpack.hpp"

namespace projectairsim = microsoft::projectairsim;

namespace {

struct LegacyImageMessageWire {
  TimeNano time_stamp;
  uint32_t height;
  uint32_t width;
  std::string encoding;
  bool big_endian;
  uint32_t step;
  std::vector<uint8_t> data;
  float pos_x;
  float pos_y;
  float pos_z;
  float rot_w;
  float rot_x;
  float rot_y;
  float rot_z;
  std::vector<projectairsim::AnnotationMsgpack> annotations;

  MSGPACK_DEFINE_MAP(time_stamp, height, width, encoding, big_endian, step, data,
                     pos_x, pos_y, pos_z, rot_w, rot_x, rot_y, rot_z,
                     annotations);
};

std::string SerializeLegacyWire(
    TimeNano time_stamp, uint32_t height, uint32_t width,
    const std::string& encoding, bool big_endian, uint32_t step,
    const std::vector<uint8_t>& data, float pos_x, float pos_y, float pos_z,
    float rot_w, float rot_x, float rot_y, float rot_z,
    const std::vector<projectairsim::Annotation>& annotations) {
  LegacyImageMessageWire wire{time_stamp,
                              height,
                              width,
                              encoding,
                              big_endian,
                              step,
                              data,
                              pos_x,
                              pos_y,
                              pos_z,
                              rot_w,
                              rot_x,
                              rot_y,
                              rot_z,
                              {}};
  for (const auto& annotation : annotations) {
    wire.annotations.emplace_back(annotation);
  }

  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  wire.msgpack_pack(packer);
  return stream.str();
}

projectairsim::Annotation MakeAnnotation() {
  std::vector<projectairsim::Vector2> image_space;
  std::vector<projectairsim::Vector3> projection_space;
  image_space.reserve(8);
  projection_space.reserve(8);
  for (int i = 0; i < 8; ++i) {
    image_space.emplace_back(static_cast<float>(i), static_cast<float>(i + 1));
    projection_space.emplace_back(static_cast<float>(i),
                                  static_cast<float>(i + 1),
                                  static_cast<float>(i + 2));
  }

  return projectairsim::Annotation(
      "vehicle",
      projectairsim::BBox2D(projectairsim::Vector2(1.0f, 2.0f),
                            projectairsim::Vector2(3.0f, 4.0f)),
      projectairsim::BBox3D(projectairsim::Vector3(5.0f, 6.0f, 7.0f),
                            projectairsim::Vector3(8.0f, 9.0f, 10.0f),
                            projectairsim::Quaternion(1.0f, 0.0f, 0.0f,
                                                      0.0f)),
      image_space, projection_space);
}

void VerifyImageSerializationMatchesLegacy(
    TimeNano time_stamp, uint32_t height, uint32_t width,
    const std::string& encoding, bool big_endian, uint32_t step,
    std::vector<uint8_t> data,
    const std::vector<projectairsim::Annotation>& annotations = {}) {
  const std::vector<uint8_t> expected_data = data;
  const float pos_x = 1.0f;
  const float pos_y = 2.0f;
  const float pos_z = 3.0f;
  const float rot_w = 1.0f;
  const float rot_x = 0.1f;
  const float rot_y = 0.2f;
  const float rot_z = 0.3f;

  const std::string expected_wire = SerializeLegacyWire(
      time_stamp, height, width, encoding, big_endian, step, expected_data,
      pos_x, pos_y, pos_z, rot_w, rot_x, rot_y, rot_z, annotations);

  projectairsim::ImageMessage message(
      time_stamp, height, width, encoding, big_endian, step, std::move(data),
      pos_x, pos_y, pos_z, rot_w, rot_x, rot_y, rot_z, annotations);
  const std::string serialized = message.Serialize();

  EXPECT_EQ(serialized, expected_wire);

  projectairsim::ImageMessage decoded;
  decoded.Deserialize(serialized);
  EXPECT_EQ(decoded.GetTimestamp(), time_stamp);
  EXPECT_EQ(decoded.GetHeight(), height);
  EXPECT_EQ(decoded.GetWidth(), width);
  EXPECT_EQ(decoded.GetEncoding(), encoding);
  EXPECT_EQ(decoded.IsBigEndian(), big_endian);
  EXPECT_EQ(decoded.GetStep(), step);
  EXPECT_EQ(decoded.GetPixelVector(), expected_data);
  ASSERT_EQ(decoded.GetAnnotations().size(), annotations.size());
  for (size_t i = 0; i < annotations.size(); ++i) {
    EXPECT_EQ(decoded.GetAnnotations()[i].object_id, annotations[i].object_id);
  }
}

}  // namespace

TEST(ImageMessage, SerializesBgrImageWithLegacyWireSchema) {
  VerifyImageSerializationMatchesLegacy(12345, 2, 3, "BGR", false, 3,
                                        {0, 1, 2, 3, 4, 5, 6, 7, 8});
}

TEST(ImageMessage, SerializesDepth16UC1ImageWithLegacyWireSchema) {
  VerifyImageSerializationMatchesLegacy(67890, 2, 2, "16UC1", true, 2,
                                        {0x01, 0x02, 0x10, 0x20, 0x30, 0x40,
                                         0x50, 0x60});
}

TEST(ImageMessage, SerializesEmptyImageWithLegacyWireSchema) {
  VerifyImageSerializationMatchesLegacy(0, 0, 0, "BGR", false, 0, {});
}

TEST(ImageMessage, SerializesAnnotationsWithLegacyWireSchema) {
  VerifyImageSerializationMatchesLegacy(24680, 1, 1, "BGR", false, 3,
                                        {9, 8, 7}, {MakeAnnotation()});
}

TEST(ImageMessage, Serializes4KPayloadWithLegacyWireSchema) {
  std::vector<uint8_t> data(3840 * 2160 * 3);
  for (size_t i = 0; i < data.size(); ++i) {
    data[i] = static_cast<uint8_t>(i % 251);
  }

  VerifyImageSerializationMatchesLegacy(13579, 2160, 3840, "BGR", false, 3,
                                        std::move(data));
}
