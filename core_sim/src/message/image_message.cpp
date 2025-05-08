// Copyright (C) Microsoft Corporation. All rights reserved.

#include "core_sim/message/image_message.hpp"

#include <memory>
#include <sstream>
#include <string>

#include "core_sim/json_utils.hpp"
#include "json.hpp"
#include "message/common_utils.hpp"
#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

using json = nlohmann::json;

// -----------------------------------------------------------------------------
// Forward declarations

class ImageMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(TimeNano time_stamp_val, uint32_t height_val, uint32_t width_val,
       const std::string& encoding_val, bool big_endian_val, uint32_t step_val,
       std::vector<uint8_t>&& data_val, std::vector<float>&& data_float, 
       float pos_x_val, float pos_y_val, float pos_z_val, float rot_w_val, 
       float rot_x_val, float rot_y_val, float rot_z_val,
       std::vector<Annotation> annotations = std::vector<Annotation>()
       );

  ~Impl() override {}

  TimeNano GetTimestamp() const;

  uint32_t GetHeight();

  uint32_t GetWidth();

  const std::string& GetEncoding();

  uint32_t IsBigEndian();

  uint32_t GetStep();

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  json GetData() const;

  std::vector<uint8_t>& GetPixelVector();

  std::vector<float> GetPositionData() const;

  const std::vector<Annotation> GetAnnotations() const;

  MSGPACK_DEFINE_MAP(time_stamp, height, width, encoding, big_endian, step,
                     data, pos_x, pos_y, pos_z, rot_w, rot_x, rot_y, rot_z,
                     annotations, data_float);

 private:
  TimeNano time_stamp;
  uint32_t height;
  uint32_t width;
  std::string encoding;
  bool big_endian;
  uint32_t step;
  std::vector<uint8_t> data;
  std::vector<float> data_float;
  std::vector<AnnotationMsgpack> annotations;

  float pos_x;
  float pos_y;
  float pos_z;
  float rot_w;
  float rot_x;
  float rot_y;
  float rot_z;
};

// -----------------------------------------------------------------------------
// class ImageMessage

ImageMessage::ImageMessage()
    : Message(std::make_shared<ImageMessage::Impl>()) {}

ImageMessage::ImageMessage(TimeNano time_stamp_val, uint32_t height_val,
                           uint32_t width_val, const std::string& encoding_val,
                           bool big_endian_val, uint32_t step_val,
                           std::vector<uint8_t>&& data_val,
                           std::vector<float>&& data_float_val, float pos_x_val,
                           float pos_y_val, float pos_z_val, float rot_w_val,
                           float rot_x_val, float rot_y_val, float rot_z_val,
                           std::vector<Annotation> annotations)
    : Message(std::make_shared<ImageMessage::Impl>(
          time_stamp_val, height_val, width_val, encoding_val, big_endian_val,
          step_val, std::move(data_val), std::move(data_float_val), pos_x_val, 
          pos_y_val, pos_z_val, rot_w_val, rot_x_val, rot_y_val, rot_z_val, 
          annotations)) {}

ImageMessage::~ImageMessage() {}

TimeNano ImageMessage::GetTimestamp() const {
  return static_cast<ImageMessage::Impl*>(pimpl_.get())->GetTimestamp();
}

uint32_t ImageMessage::GetHeight() const {
  return static_cast<ImageMessage::Impl*>(pimpl_.get())->GetHeight();
}

uint32_t ImageMessage::GetWidth() const {
  return static_cast<ImageMessage::Impl*>(pimpl_.get())->GetWidth();
}

const std::string& ImageMessage::GetEncoding() const {
  return static_cast<ImageMessage::Impl*>(pimpl_.get())->GetEncoding();
}

uint32_t ImageMessage::IsBigEndian() const {
  return static_cast<ImageMessage::Impl*>(pimpl_.get())->IsBigEndian();
}

uint32_t ImageMessage::GetStep() const {
  return static_cast<ImageMessage::Impl*>(pimpl_.get())->GetStep();
}

std::string ImageMessage::Serialize() const {
  return static_cast<ImageMessage::Impl*>(pimpl_.get())->Serialize();
}

void ImageMessage::Deserialize(const std::string& buffer) {
  static_cast<ImageMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

json ImageMessage::GetData() const {
  return static_cast<ImageMessage::Impl*>(pimpl_.get())->GetData();
}

std::vector<float> ImageMessage::GetPositionData() const {
  return static_cast<ImageMessage::Impl*>(pimpl_.get())->GetPositionData();
}

std::vector<uint8_t>& ImageMessage::GetPixelVector() {
  return static_cast<ImageMessage::Impl*>(pimpl_.get())->GetPixelVector();
}

const std::vector<Annotation> ImageMessage::GetAnnotations() const {
  return static_cast<ImageMessage::Impl*>(pimpl_.get())->GetAnnotations();
}
  // -----------------------------------------------------------------------------
// class ImageMessage::Impl

ImageMessage::Impl::Impl()
    : MessageImpl(MessageType::kImage),
      time_stamp(0),
      height(0),
      width(0),
      big_endian(false),
      step(0) {}

ImageMessage::Impl::Impl(TimeNano time_stamp_val, uint32_t height_val,
                         uint32_t width_val, const std::string& encoding_val,
                         bool big_endian_val, uint32_t step_val,
                         std::vector<uint8_t>&& data_val, std::vector<float>&& data_float_val,
                         float pos_x_val, float pos_y_val, float pos_z_val, 
                         float rot_w_val, float rot_x_val, float rot_y_val, float rot_z_val,
                         std::vector<Annotation> annotations_val
                         )
    : MessageImpl(MessageType::kImage),
      time_stamp(time_stamp_val),
      height(height_val),
      width(width_val),
      encoding(encoding_val),
      big_endian(big_endian_val),
      step(step_val),
      data(std::move(data_val)),
      data_float(std::move(data_float_val)),
      pos_x(pos_x_val),
      pos_y(pos_y_val),
      pos_z(pos_z_val),
      rot_w(rot_w_val),
      rot_x(rot_x_val),
      rot_y(rot_y_val),
      rot_z(rot_z_val) {
  for (auto& annotation : annotations_val) {
    annotations.emplace_back(annotation);
  }
}

TimeNano ImageMessage::Impl::GetTimestamp() const { return time_stamp; }

uint32_t ImageMessage::Impl::GetHeight() { return height; }

uint32_t ImageMessage::Impl::GetWidth() { return width; }

const std::string& ImageMessage::Impl::GetEncoding() { return encoding; }

uint32_t ImageMessage::Impl::IsBigEndian() { return big_endian; }

uint32_t ImageMessage::Impl::GetStep() { return step; }

std::string ImageMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void ImageMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

json ImageMessage::Impl::GetData() const {
  return json({{"time_stamp", time_stamp},
               {"height", height},
               {"width", width},
               {"encoding", encoding},
               {"big_endian", big_endian},
               {"step", step},
               {"data", data},
               {"data_float", data_float},
               {"pos_x", pos_x},
               {"pos_y", pos_y},
               {"pos_z", pos_z},
               {"rot_w", rot_w},
               {"rot_x", rot_x},
               {"rot_y", rot_y},
               {"rot_z", rot_z},
               {"annotations", GetAnnotations()}});
}

std::vector<uint8_t>& ImageMessage::Impl::GetPixelVector() {
  if (data.empty()) {
    std::vector<uint8_t> return_data;
    return_data.resize(data_float.size() * sizeof(float));
    std::memcpy(return_data.data(), data_float.data(), return_data.size());
    return return_data;
  }
  return data;
}

const std::vector<Annotation> ImageMessage::Impl::GetAnnotations() const {
  std::vector<Annotation> annotations_out;
  for (auto& annotation : annotations) {
    annotations_out.emplace_back(annotation.ToAnnotation());
  }
  return annotations_out;
}

std::vector<float> ImageMessage::Impl::GetPositionData() const {
  std::vector<float> position_vect{pos_x, pos_y, pos_z, rot_w,
                                   rot_x, rot_y, rot_z};
  return position_vect;
}

}  // namespace projectairsim
}  // namespace microsoft
