// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// The CameraInfoMessage implementation.

#include "core_sim/message/camera_info_message.hpp"

#include <array>
#include <memory>
#include <sstream>
#include <string>

#include "message_impl.hpp"
#include "msgpack.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// Forward declarations

class CameraInfoMessage::Impl : public MessageImpl {
 public:
  Impl();

  Impl(uint32_t width_val, uint32_t height_val,
       const std::string& distortion_model_val,
       const std::array<double, 5>& distortion_params_val,
       const std::array<double, 9>& intrinsic_camera_matrix_val,
       const std::array<double, 9>& rectification_matrix_val,
       const std::array<double, 12>& projection_matrix_val);

  ~Impl() override {}

  uint32_t GetWidth();

  uint32_t GetHeight();

  const std::string& GetDistortionModel();

  const std::array<double, 5>& GetDistortionParams();

  const std::array<double, 9>& GetIntrinsicCameraMatrix();

  const std::array<double, 9>& GetRectificationMatrix();

  const std::array<double, 12>& GetProjectionMatrix();

  std::string Serialize() override;

  void Deserialize(const std::string& buffer) override;

  MSGPACK_DEFINE_MAP(width, height, distortion_model, distortion_params,
                     intrinsic_camera_matrix, rectification_matrix,
                     projection_matrix);

 private:
  uint32_t width;
  uint32_t height;
  std::string distortion_model;
  std::array<double, 5> distortion_params;
  std::array<double, 9> intrinsic_camera_matrix;
  std::array<double, 9> rectification_matrix;
  std::array<double, 12> projection_matrix;
};

// -----------------------------------------------------------------------------
// class CameraInfoMessage

CameraInfoMessage::CameraInfoMessage()
    : Message(std::make_shared<CameraInfoMessage::Impl>()) {}

CameraInfoMessage::CameraInfoMessage(
    uint32_t width_val, uint32_t height_val,
    const std::string& distortion_model_val,
    const std::array<double, 5>& distortion_params_val,
    const std::array<double, 9>& intrinsic_camera_matrix_val,
    const std::array<double, 9>& rectification_matrix_val,
    const std::array<double, 12>& projection_matrix_val)
    : Message(std::make_shared<CameraInfoMessage::Impl>(
          width_val, height_val, distortion_model_val, distortion_params_val,
          intrinsic_camera_matrix_val, rectification_matrix_val,
          projection_matrix_val)) {}

CameraInfoMessage::~CameraInfoMessage() {}

uint32_t CameraInfoMessage::GetWidth() const {
  return static_cast<CameraInfoMessage::Impl*>(pimpl_.get())->GetWidth();
}

uint32_t CameraInfoMessage::GetHeight() const {
  return static_cast<CameraInfoMessage::Impl*>(pimpl_.get())->GetHeight();
}

const std::string& CameraInfoMessage::GetDistortionModel() const {
  return static_cast<CameraInfoMessage::Impl*>(pimpl_.get())
      ->GetDistortionModel();
}

const std::array<double, 5>& CameraInfoMessage::GetDistortionParams() const {
  return static_cast<CameraInfoMessage::Impl*>(pimpl_.get())
      ->GetDistortionParams();
}

const std::array<double, 9>& CameraInfoMessage::GetIntrinsicCameraMatrix()
    const {
  return static_cast<CameraInfoMessage::Impl*>(pimpl_.get())
      ->GetIntrinsicCameraMatrix();
}

const std::array<double, 9>& CameraInfoMessage::GetRectificationMatrix() const {
  return static_cast<CameraInfoMessage::Impl*>(pimpl_.get())
      ->GetRectificationMatrix();
}

const std::array<double, 12>& CameraInfoMessage::GetProjectionMatrix() const {
  return static_cast<CameraInfoMessage::Impl*>(pimpl_.get())
      ->GetProjectionMatrix();
}

std::string CameraInfoMessage::Serialize() const {
  return static_cast<CameraInfoMessage::Impl*>(pimpl_.get())->Serialize();
}

void CameraInfoMessage::Deserialize(const std::string& buffer) {
  static_cast<CameraInfoMessage::Impl*>(pimpl_.get())->Deserialize(buffer);
}

// -----------------------------------------------------------------------------
// class CameraInfoMessage::Impl

CameraInfoMessage::Impl::Impl() : MessageImpl(MessageType::kCameraInfo) {}

CameraInfoMessage::Impl::Impl(
    uint32_t width_val, uint32_t height_val,
    const std::string& distortion_model_val,
    const std::array<double, 5>& distortion_params_val,
    const std::array<double, 9>& intrinsic_camera_matrix_val,
    const std::array<double, 9>& rectification_matrix_val,
    const std::array<double, 12>& projection_matrix_val)
    : MessageImpl(MessageType::kCameraInfo),
      width(width_val),
      height(height_val),
      distortion_model(distortion_model_val),
      distortion_params(distortion_params_val),
      intrinsic_camera_matrix(intrinsic_camera_matrix_val),
      rectification_matrix(rectification_matrix_val),
      projection_matrix(projection_matrix_val) {}

uint32_t CameraInfoMessage::Impl::GetWidth() { return width; }

uint32_t CameraInfoMessage::Impl::GetHeight() { return height; }

const std::string& CameraInfoMessage::Impl::GetDistortionModel() {
  return distortion_model;
}

const std::array<double, 5>& CameraInfoMessage::Impl::GetDistortionParams() {
  return distortion_params;
}

const std::array<double, 9>&
CameraInfoMessage::Impl::GetIntrinsicCameraMatrix() {
  return intrinsic_camera_matrix;
}

const std::array<double, 9>& CameraInfoMessage::Impl::GetRectificationMatrix() {
  return rectification_matrix;
}

const std::array<double, 12>& CameraInfoMessage::Impl::GetProjectionMatrix() {
  return projection_matrix;
}

std::string CameraInfoMessage::Impl::Serialize() {
  std::stringstream stream;
  msgpack::packer<std::stringstream> packer(stream);
  this->msgpack_pack(packer);
  return stream.str();
}

void CameraInfoMessage::Impl::Deserialize(const std::string& buffer) {
  auto handle = msgpack::unpack(buffer.data(), buffer.size());
  auto object = handle.get();
  this->msgpack_unpack(object);
}

}  // namespace projectairsim
}  // namespace microsoft
