// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#include "message_utils.hpp"

#include "core_sim/error.hpp"
#include "core_sim/message/flight_control_rc_input_message.hpp"
#include "core_sim/message/flight_control_setpoint_message.hpp"
#include "core_sim/message/int8_message.hpp"
#include "core_sim/message/int32_message.hpp"
#include "core_sim/message/joint_state_message.hpp"
#include "core_sim/message/pose_stamped_message.hpp"
#include "core_sim/message/pose_message.hpp"
#include "core_sim/message/int_list_message.hpp"
#include "core_sim/message/float_message.hpp"

namespace microsoft {
namespace projectairsim {

Message MessageUtils::ToMessage(const Topic& topic, const std::string& buffer) {
  switch (topic.GetMessageType()) {
    case MessageType::kInt8: {
      Int8Message message;
      message.Deserialize(buffer);
      return message;
    }
    case MessageType::kInt32: {
      Int32Message message;
      message.Deserialize(buffer);
      return message;
    }
    case MessageType::kJointState: {
      JointStateMessage message;
      message.Deserialize(buffer);
      return message;
    }
    case MessageType::kPosestamped: {
      PoseStampedMessage message;
      message.Deserialize(buffer);
      return message;
    }
    case MessageType::kPose: {
      PoseMessage message;
      message.Deserialize(buffer);
      return message;
    }
    case MessageType::kIntList: {
      IntListMessage message;
      message.Deserialize(buffer);
      return message;
    }
    case MessageType::kFloat: {
      FloatMessage message;
      message.Deserialize(buffer);
      return message;
    }
    case MessageType::kFlightControlRCInput: {
      FlightControlRCInputMessage message;
      message.Deserialize(buffer);
      return message;
    }
    case MessageType::kFlightControlSetpoint: {
      FlightControlSetpointMessage message;
      message.Deserialize(buffer);
      return message;
    }
    default: {
      break;
    }
  }

  throw Error("Invalid message type.");
}

}  // namespace projectairsim
}  // namespace microsoft
