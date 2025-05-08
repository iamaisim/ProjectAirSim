// Copyright (C) Microsoft Corporation.  All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_CAMERA_INFO_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_CAMERA_INFO_MESSAGE_HPP_

#include <array>
#include <memory>
#include <string>

#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

class CameraInfoMessage : public Message {
 public:
  CameraInfoMessage(uint32_t width_val, uint32_t height_val,
                    const std::string& distortion_model_val,
                    const std::array<double, 5>& distortion_params_val,
                    const std::array<double, 9>& intrinsic_camera_matrix_val,
                    const std::array<double, 9>& rectification_matrix_val,
                    const std::array<double, 12>& projection_matrix_val);

  CameraInfoMessage();

  ~CameraInfoMessage() override;

  uint32_t GetWidth() const;

  uint32_t GetHeight() const;

  const std::string& GetDistortionModel() const;

  const std::array<double, 5>& GetDistortionParams() const;

  const std::array<double, 9>& GetIntrinsicCameraMatrix() const;

  const std::array<double, 9>& GetRectificationMatrix() const;

  const std::array<double, 12>& GetProjectionMatrix() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_CAMERA_INFO_MESSAGE_HPP_
