#ifndef CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_WIDTH_HEIGHT_MESSAGE_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_WIDTH_HEIGHT_MESSAGE_HPP_

#include <memory>
#include <string>

#include "core_sim/clock.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/message/message.hpp"

namespace microsoft {
namespace projectairsim {

class WidthHeightMessage : public Message {
 public:
  WidthHeightMessage(const int width,
              const int height);

  WidthHeightMessage();

  ~WidthHeightMessage() override;

  int GetWidth() const;
  int GetHeight() const;

  std::string Serialize() const override;

  void Deserialize(const std::string& buffer) override;

 private:
  class Impl;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_MESSAGE_WIDTH_HEIGHT_MESSAGE_HPP_
