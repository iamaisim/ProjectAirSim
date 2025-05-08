// Copyright (c) Microsoft Corporation. All rights reserved.

#include "MavLinkMessages.hpp"

#include <sstream>
using namespace mavlinkcom;

int MavLinkHeartbeat::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->custom_mode),
                0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 4);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->autopilot), 5);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->base_mode), 6);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->system_status),
               7);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mavlink_version),
               8);
  return 9;
}

int MavLinkHeartbeat::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->custom_mode), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->autopilot), 5);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->base_mode), 6);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->system_status), 7);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mavlink_version), 8);
  return 9;
}

std::string MavLinkHeartbeat::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HEARTBEAT\", \"id\": 0, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"custom_mode\":" << this->custom_mode;
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"autopilot\":" << static_cast<unsigned int>(this->autopilot);
  ss << ", \"base_mode\":" << static_cast<unsigned int>(this->base_mode);
  ss << ", \"system_status\":"
     << static_cast<unsigned int>(this->system_status);
  ss << ", \"mavlink_version\":"
     << static_cast<unsigned int>(this->mavlink_version);
  ss << "} },";
  return ss.str();
}

int MavLinkSysStatus::pack(char* buffer) const {
  pack_uint32_t(
      buffer,
      reinterpret_cast<const uint32_t*>(&this->onboard_control_sensors_present),
      0);
  pack_uint32_t(
      buffer,
      reinterpret_cast<const uint32_t*>(&this->onboard_control_sensors_enabled),
      4);
  pack_uint32_t(
      buffer,
      reinterpret_cast<const uint32_t*>(&this->onboard_control_sensors_health),
      8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->load), 12);
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->voltage_battery), 14);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->current_battery),
               16);
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->drop_rate_comm), 18);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->errors_comm),
                20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->errors_count1),
                22);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->errors_count2),
                24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->errors_count3),
                26);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->errors_count4),
                28);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->battery_remaining),
              30);
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(
                    &this->onboard_control_sensors_present_extended),
                31);
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(
                    &this->onboard_control_sensors_enabled_extended),
                35);
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(
                    &this->onboard_control_sensors_health_extended),
                39);
  return 43;
}

int MavLinkSysStatus::unpack(const char* buffer) {
  unpack_uint32_t(
      buffer,
      reinterpret_cast<uint32_t*>(&this->onboard_control_sensors_present), 0);
  unpack_uint32_t(
      buffer,
      reinterpret_cast<uint32_t*>(&this->onboard_control_sensors_enabled), 4);
  unpack_uint32_t(
      buffer,
      reinterpret_cast<uint32_t*>(&this->onboard_control_sensors_health), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->load), 12);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->voltage_battery),
                  14);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->current_battery),
                 16);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->drop_rate_comm),
                  18);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->errors_comm), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->errors_count1),
                  22);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->errors_count2),
                  24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->errors_count3),
                  26);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->errors_count4),
                  28);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->battery_remaining),
                30);
  unpack_uint32_t(buffer,
                  reinterpret_cast<uint32_t*>(
                      &this->onboard_control_sensors_present_extended),
                  31);
  unpack_uint32_t(buffer,
                  reinterpret_cast<uint32_t*>(
                      &this->onboard_control_sensors_enabled_extended),
                  35);
  unpack_uint32_t(buffer,
                  reinterpret_cast<uint32_t*>(
                      &this->onboard_control_sensors_health_extended),
                  39);
  return 43;
}

std::string MavLinkSysStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SYS_STATUS\", \"id\": 1, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"onboard_control_sensors_present\":"
     << this->onboard_control_sensors_present;
  ss << ", \"onboard_control_sensors_enabled\":"
     << this->onboard_control_sensors_enabled;
  ss << ", \"onboard_control_sensors_health\":"
     << this->onboard_control_sensors_health;
  ss << ", \"load\":" << this->load;
  ss << ", \"voltage_battery\":" << this->voltage_battery;
  ss << ", \"current_battery\":" << this->current_battery;
  ss << ", \"drop_rate_comm\":" << this->drop_rate_comm;
  ss << ", \"errors_comm\":" << this->errors_comm;
  ss << ", \"errors_count1\":" << this->errors_count1;
  ss << ", \"errors_count2\":" << this->errors_count2;
  ss << ", \"errors_count3\":" << this->errors_count3;
  ss << ", \"errors_count4\":" << this->errors_count4;
  ss << ", \"battery_remaining\":" << static_cast<int>(this->battery_remaining);
  ss << ", \"onboard_control_sensors_present_extended\":"
     << this->onboard_control_sensors_present_extended;
  ss << ", \"onboard_control_sensors_enabled_extended\":"
     << this->onboard_control_sensors_enabled_extended;
  ss << ", \"onboard_control_sensors_health_extended\":"
     << this->onboard_control_sensors_health_extended;
  ss << "} },";
  return ss.str();
}

int MavLinkSystemTime::pack(char* buffer) const {
  pack_uint64_t(buffer,
                reinterpret_cast<const uint64_t*>(&this->time_unix_usec), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                8);
  return 12;
}

int MavLinkSystemTime::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_unix_usec),
                  0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 8);
  return 12;
}

std::string MavLinkSystemTime::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SYSTEM_TIME\", \"id\": 2, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_unix_usec\":" << this->time_unix_usec;
  ss << ", \"time_boot_ms\":" << this->time_boot_ms;
  ss << "} },";
  return ss.str();
}

int MavLinkPing::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->seq), 8);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               12);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 13);
  return 14;
}

int MavLinkPing::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->seq), 8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 12);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 13);
  return 14;
}

std::string MavLinkPing::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PING\", \"id\": 4, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"seq\":" << this->seq;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkChangeOperatorControl::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->control_request),
               1);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->version), 2);
  pack_char_array(25, buffer, reinterpret_cast<const char*>(&this->passkey[0]),
                  3);
  return 28;
}

int MavLinkChangeOperatorControl::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->control_request), 1);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->version), 2);
  unpack_char_array(25, buffer, reinterpret_cast<char*>(&this->passkey[0]), 3);
  return 28;
}

std::string MavLinkChangeOperatorControl::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CHANGE_OPERATOR_CONTROL\", \"id\": 5, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"control_request\":"
     << static_cast<unsigned int>(this->control_request);
  ss << ", \"version\":" << static_cast<unsigned int>(this->version);
  ss << ", \"passkey\":"
     << "\""
     << char_array_tostring(25, reinterpret_cast<char*>(&this->passkey[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkChangeOperatorControlAck::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->gcs_system_id),
               0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->control_request),
               1);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->ack), 2);
  return 3;
}

int MavLinkChangeOperatorControlAck::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->gcs_system_id), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->control_request), 1);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->ack), 2);
  return 3;
}

std::string MavLinkChangeOperatorControlAck::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CHANGE_OPERATOR_CONTROL_ACK\", \"id\": 6, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"gcs_system_id\":" << static_cast<unsigned int>(this->gcs_system_id);
  ss << ", \"control_request\":"
     << static_cast<unsigned int>(this->control_request);
  ss << ", \"ack\":" << static_cast<unsigned int>(this->ack);
  ss << "} },";
  return ss.str();
}

int MavLinkAuthKey::pack(char* buffer) const {
  pack_char_array(32, buffer, reinterpret_cast<const char*>(&this->key[0]), 0);
  return 32;
}

int MavLinkAuthKey::unpack(const char* buffer) {
  unpack_char_array(32, buffer, reinterpret_cast<char*>(&this->key[0]), 0);
  return 32;
}

std::string MavLinkAuthKey::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"AUTH_KEY\", \"id\": 7, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"key\":"
     << "\"" << char_array_tostring(32, reinterpret_cast<char*>(&this->key[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkLinkNodeStatus::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->timestamp), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->tx_rate), 8);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->rx_rate), 12);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->messages_sent),
                16);
  pack_uint32_t(
      buffer, reinterpret_cast<const uint32_t*>(&this->messages_received), 20);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->messages_lost),
                24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->rx_parse_err),
                28);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->tx_overflows),
                30);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->rx_overflows),
                32);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->tx_buf), 34);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->rx_buf), 35);
  return 36;
}

int MavLinkLinkNodeStatus::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->timestamp), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->tx_rate), 8);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->rx_rate), 12);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->messages_sent),
                  16);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->messages_received),
                  20);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->messages_lost),
                  24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->rx_parse_err), 28);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->tx_overflows), 30);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->rx_overflows), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->tx_buf), 34);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rx_buf), 35);
  return 36;
}

std::string MavLinkLinkNodeStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LINK_NODE_STATUS\", \"id\": 8, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"timestamp\":" << this->timestamp;
  ss << ", \"tx_rate\":" << this->tx_rate;
  ss << ", \"rx_rate\":" << this->rx_rate;
  ss << ", \"messages_sent\":" << this->messages_sent;
  ss << ", \"messages_received\":" << this->messages_received;
  ss << ", \"messages_lost\":" << this->messages_lost;
  ss << ", \"rx_parse_err\":" << this->rx_parse_err;
  ss << ", \"tx_overflows\":" << this->tx_overflows;
  ss << ", \"rx_overflows\":" << this->rx_overflows;
  ss << ", \"tx_buf\":" << static_cast<unsigned int>(this->tx_buf);
  ss << ", \"rx_buf\":" << static_cast<unsigned int>(this->rx_buf);
  ss << "} },";
  return ss.str();
}

int MavLinkSetMode::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->custom_mode),
                0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               4);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->base_mode), 5);
  return 6;
}

int MavLinkSetMode::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->custom_mode), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->base_mode), 5);
  return 6;
}

std::string MavLinkSetMode::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SET_MODE\", \"id\": 11, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"custom_mode\":" << this->custom_mode;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"base_mode\":" << static_cast<unsigned int>(this->base_mode);
  ss << "} },";
  return ss.str();
}

int MavLinkParamRequestRead::pack(char* buffer) const {
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->param_index), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 3);
  pack_char_array(16, buffer, reinterpret_cast<const char*>(&this->param_id[0]),
                  4);
  return 20;
}

int MavLinkParamRequestRead::unpack(const char* buffer) {
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->param_index), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 3);
  unpack_char_array(16, buffer, reinterpret_cast<char*>(&this->param_id[0]), 4);
  return 20;
}

std::string MavLinkParamRequestRead::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PARAM_REQUEST_READ\", \"id\": 20, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"param_index\":" << this->param_index;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"param_id\":"
     << "\""
     << char_array_tostring(16, reinterpret_cast<char*>(&this->param_id[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkParamRequestList::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  return 2;
}

int MavLinkParamRequestList::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  return 2;
}

std::string MavLinkParamRequestList::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PARAM_REQUEST_LIST\", \"id\": 21, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkParamValue::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->param_value), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->param_count),
                4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->param_index),
                6);
  pack_char_array(16, buffer, reinterpret_cast<const char*>(&this->param_id[0]),
                  8);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->param_type), 24);
  return 25;
}

int MavLinkParamValue::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->param_value), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->param_count), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->param_index), 6);
  unpack_char_array(16, buffer, reinterpret_cast<char*>(&this->param_id[0]), 8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->param_type), 24);
  return 25;
}

std::string MavLinkParamValue::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PARAM_VALUE\", \"id\": 22, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"param_value\":" << float_tostring(this->param_value);
  ss << ", \"param_count\":" << this->param_count;
  ss << ", \"param_index\":" << this->param_index;
  ss << ", \"param_id\":"
     << "\""
     << char_array_tostring(16, reinterpret_cast<char*>(&this->param_id[0]))
     << "\"";
  ss << ", \"param_type\":" << static_cast<unsigned int>(this->param_type);
  ss << "} },";
  return ss.str();
}

int MavLinkParamSet::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->param_value), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               4);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 5);
  pack_char_array(16, buffer, reinterpret_cast<const char*>(&this->param_id[0]),
                  6);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->param_type), 22);
  return 23;
}

int MavLinkParamSet::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->param_value), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 5);
  unpack_char_array(16, buffer, reinterpret_cast<char*>(&this->param_id[0]), 6);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->param_type), 22);
  return 23;
}

std::string MavLinkParamSet::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PARAM_SET\", \"id\": 23, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"param_value\":" << float_tostring(this->param_value);
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"param_id\":"
     << "\""
     << char_array_tostring(16, reinterpret_cast<char*>(&this->param_id[0]))
     << "\"";
  ss << ", \"param_type\":" << static_cast<unsigned int>(this->param_type);
  ss << "} },";
  return ss.str();
}

int MavLinkGpsRawInt::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt), 16);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->eph), 20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->epv), 22);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->vel), 24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->cog), 26);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->fix_type), 28);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->satellites_visible), 29);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt_ellipsoid),
               30);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->h_acc), 34);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->v_acc), 38);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->vel_acc), 42);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->hdg_acc), 46);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->yaw), 50);
  return 52;
}

int MavLinkGpsRawInt::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt), 16);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->eph), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->epv), 22);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->vel), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->cog), 26);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->fix_type), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->satellites_visible),
                 29);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt_ellipsoid), 30);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->h_acc), 34);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->v_acc), 38);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->vel_acc), 42);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->hdg_acc), 46);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->yaw), 50);
  return 52;
}

std::string MavLinkGpsRawInt::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GPS_RAW_INT\", \"id\": 24, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"alt\":" << this->alt;
  ss << ", \"eph\":" << this->eph;
  ss << ", \"epv\":" << this->epv;
  ss << ", \"vel\":" << this->vel;
  ss << ", \"cog\":" << this->cog;
  ss << ", \"fix_type\":" << static_cast<unsigned int>(this->fix_type);
  ss << ", \"satellites_visible\":"
     << static_cast<unsigned int>(this->satellites_visible);
  ss << ", \"alt_ellipsoid\":" << this->alt_ellipsoid;
  ss << ", \"h_acc\":" << this->h_acc;
  ss << ", \"v_acc\":" << this->v_acc;
  ss << ", \"vel_acc\":" << this->vel_acc;
  ss << ", \"hdg_acc\":" << this->hdg_acc;
  ss << ", \"yaw\":" << this->yaw;
  ss << "} },";
  return ss.str();
}

int MavLinkGpsStatus::pack(char* buffer) const {
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->satellites_visible), 0);
  pack_uint8_t_array(
      20, buffer, reinterpret_cast<const uint8_t*>(&this->satellite_prn[0]), 1);
  pack_uint8_t_array(20, buffer,
                     reinterpret_cast<const uint8_t*>(&this->satellite_used[0]),
                     21);
  pack_uint8_t_array(
      20, buffer,
      reinterpret_cast<const uint8_t*>(&this->satellite_elevation[0]), 41);
  pack_uint8_t_array(
      20, buffer, reinterpret_cast<const uint8_t*>(&this->satellite_azimuth[0]),
      61);
  pack_uint8_t_array(20, buffer,
                     reinterpret_cast<const uint8_t*>(&this->satellite_snr[0]),
                     81);
  return 101;
}

int MavLinkGpsStatus::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->satellites_visible),
                 0);
  unpack_uint8_t_array(20, buffer,
                       reinterpret_cast<uint8_t*>(&this->satellite_prn[0]), 1);
  unpack_uint8_t_array(
      20, buffer, reinterpret_cast<uint8_t*>(&this->satellite_used[0]), 21);
  unpack_uint8_t_array(
      20, buffer, reinterpret_cast<uint8_t*>(&this->satellite_elevation[0]),
      41);
  unpack_uint8_t_array(
      20, buffer, reinterpret_cast<uint8_t*>(&this->satellite_azimuth[0]), 61);
  unpack_uint8_t_array(20, buffer,
                       reinterpret_cast<uint8_t*>(&this->satellite_snr[0]), 81);
  return 101;
}

std::string MavLinkGpsStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GPS_STATUS\", \"id\": 25, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"satellites_visible\":"
     << static_cast<unsigned int>(this->satellites_visible);
  ss << ", \"satellite_prn\":"
     << "["
     << uint8_t_array_tostring(
            20, reinterpret_cast<uint8_t*>(&this->satellite_prn[0]))
     << "]";
  ss << ", \"satellite_used\":"
     << "["
     << uint8_t_array_tostring(
            20, reinterpret_cast<uint8_t*>(&this->satellite_used[0]))
     << "]";
  ss << ", \"satellite_elevation\":"
     << "["
     << uint8_t_array_tostring(
            20, reinterpret_cast<uint8_t*>(&this->satellite_elevation[0]))
     << "]";
  ss << ", \"satellite_azimuth\":"
     << "["
     << uint8_t_array_tostring(
            20, reinterpret_cast<uint8_t*>(&this->satellite_azimuth[0]))
     << "]";
  ss << ", \"satellite_snr\":"
     << "["
     << uint8_t_array_tostring(
            20, reinterpret_cast<uint8_t*>(&this->satellite_snr[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkScaledImu::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xacc), 4);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->yacc), 6);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zacc), 8);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xgyro), 10);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->ygyro), 12);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zgyro), 14);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xmag), 16);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->ymag), 18);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zmag), 20);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature),
               22);
  return 24;
}

int MavLinkScaledImu::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xacc), 4);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->yacc), 6);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zacc), 8);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xgyro), 10);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->ygyro), 12);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zgyro), 14);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xmag), 16);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->ymag), 18);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zmag), 20);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 22);
  return 24;
}

std::string MavLinkScaledImu::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SCALED_IMU\", \"id\": 26, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"xacc\":" << this->xacc;
  ss << ", \"yacc\":" << this->yacc;
  ss << ", \"zacc\":" << this->zacc;
  ss << ", \"xgyro\":" << this->xgyro;
  ss << ", \"ygyro\":" << this->ygyro;
  ss << ", \"zgyro\":" << this->zgyro;
  ss << ", \"xmag\":" << this->xmag;
  ss << ", \"ymag\":" << this->ymag;
  ss << ", \"zmag\":" << this->zmag;
  ss << ", \"temperature\":" << this->temperature;
  ss << "} },";
  return ss.str();
}

int MavLinkRawImu::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xacc), 8);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->yacc), 10);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zacc), 12);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xgyro), 14);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->ygyro), 16);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zgyro), 18);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xmag), 20);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->ymag), 22);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zmag), 24);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->id), 26);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature),
               27);
  return 29;
}

int MavLinkRawImu::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xacc), 8);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->yacc), 10);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zacc), 12);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xgyro), 14);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->ygyro), 16);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zgyro), 18);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xmag), 20);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->ymag), 22);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zmag), 24);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->id), 26);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 27);
  return 29;
}

std::string MavLinkRawImu::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"RAW_IMU\", \"id\": 27, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"xacc\":" << this->xacc;
  ss << ", \"yacc\":" << this->yacc;
  ss << ", \"zacc\":" << this->zacc;
  ss << ", \"xgyro\":" << this->xgyro;
  ss << ", \"ygyro\":" << this->ygyro;
  ss << ", \"zgyro\":" << this->zgyro;
  ss << ", \"xmag\":" << this->xmag;
  ss << ", \"ymag\":" << this->ymag;
  ss << ", \"zmag\":" << this->zmag;
  ss << ", \"id\":" << static_cast<unsigned int>(this->id);
  ss << ", \"temperature\":" << this->temperature;
  ss << "} },";
  return ss.str();
}

int MavLinkRawPressure::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->press_abs), 8);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->press_diff1),
               10);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->press_diff2),
               12);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature),
               14);
  return 16;
}

int MavLinkRawPressure::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->press_abs), 8);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->press_diff1), 10);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->press_diff2), 12);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 14);
  return 16;
}

std::string MavLinkRawPressure::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"RAW_PRESSURE\", \"id\": 28, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"press_abs\":" << this->press_abs;
  ss << ", \"press_diff1\":" << this->press_diff1;
  ss << ", \"press_diff2\":" << this->press_diff2;
  ss << ", \"temperature\":" << this->temperature;
  ss << "} },";
  return ss.str();
}

int MavLinkScaledPressure::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->press_abs), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->press_diff), 8);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature),
               12);
  pack_int16_t(buffer,
               reinterpret_cast<const int16_t*>(&this->temperature_press_diff),
               14);
  return 16;
}

int MavLinkScaledPressure::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->press_abs), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->press_diff), 8);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 12);
  unpack_int16_t(buffer,
                 reinterpret_cast<int16_t*>(&this->temperature_press_diff), 14);
  return 16;
}

std::string MavLinkScaledPressure::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SCALED_PRESSURE\", \"id\": 29, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"press_abs\":" << float_tostring(this->press_abs);
  ss << ", \"press_diff\":" << float_tostring(this->press_diff);
  ss << ", \"temperature\":" << this->temperature;
  ss << ", \"temperature_press_diff\":" << this->temperature_press_diff;
  ss << "} },";
  return ss.str();
}

int MavLinkAttitude::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->rollspeed), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitchspeed), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yawspeed), 24);
  return 28;
}

int MavLinkAttitude::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->rollspeed), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitchspeed), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yawspeed), 24);
  return 28;
}

std::string MavLinkAttitude::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ATTITUDE\", \"id\": 30, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"roll\":" << float_tostring(this->roll);
  ss << ", \"pitch\":" << float_tostring(this->pitch);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"rollspeed\":" << float_tostring(this->rollspeed);
  ss << ", \"pitchspeed\":" << float_tostring(this->pitchspeed);
  ss << ", \"yawspeed\":" << float_tostring(this->yawspeed);
  ss << "} },";
  return ss.str();
}

int MavLinkAttitudeQuaternion::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->q1), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->q2), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->q3), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->q4), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->rollspeed), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitchspeed), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yawspeed), 28);
  pack_float_array(4, buffer,
                   reinterpret_cast<const float*>(&this->repr_offset_q[0]), 32);
  return 48;
}

int MavLinkAttitudeQuaternion::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->q1), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->q2), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->q3), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->q4), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->rollspeed), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitchspeed), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yawspeed), 28);
  unpack_float_array(4, buffer,
                     reinterpret_cast<float*>(&this->repr_offset_q[0]), 32);
  return 48;
}

std::string MavLinkAttitudeQuaternion::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ATTITUDE_QUATERNION\", \"id\": 31, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"q1\":" << float_tostring(this->q1);
  ss << ", \"q2\":" << float_tostring(this->q2);
  ss << ", \"q3\":" << float_tostring(this->q3);
  ss << ", \"q4\":" << float_tostring(this->q4);
  ss << ", \"rollspeed\":" << float_tostring(this->rollspeed);
  ss << ", \"pitchspeed\":" << float_tostring(this->pitchspeed);
  ss << ", \"yawspeed\":" << float_tostring(this->yawspeed);
  ss << ", \"repr_offset_q\":"
     << "["
     << float_array_tostring(4,
                             reinterpret_cast<float*>(&this->repr_offset_q[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkLocalPositionNed::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vx), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vy), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vz), 24);
  return 28;
}

int MavLinkLocalPositionNed::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vx), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vy), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vz), 24);
  return 28;
}

std::string MavLinkLocalPositionNed::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LOCAL_POSITION_NED\", \"id\": 32, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"vx\":" << float_tostring(this->vx);
  ss << ", \"vy\":" << float_tostring(this->vy);
  ss << ", \"vz\":" << float_tostring(this->vz);
  ss << "} },";
  return ss.str();
}

int MavLinkGlobalPositionInt::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt), 12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->relative_alt),
               16);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vx), 20);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vy), 22);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vz), 24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->hdg), 26);
  return 28;
}

int MavLinkGlobalPositionInt::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->relative_alt), 16);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vx), 20);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vy), 22);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vz), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->hdg), 26);
  return 28;
}

std::string MavLinkGlobalPositionInt::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GLOBAL_POSITION_INT\", \"id\": 33, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"alt\":" << this->alt;
  ss << ", \"relative_alt\":" << this->relative_alt;
  ss << ", \"vx\":" << this->vx;
  ss << ", \"vy\":" << this->vy;
  ss << ", \"vz\":" << this->vz;
  ss << ", \"hdg\":" << this->hdg;
  ss << "} },";
  return ss.str();
}

int MavLinkRcChannelsScaled::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->chan1_scaled),
               4);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->chan2_scaled),
               6);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->chan3_scaled),
               8);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->chan4_scaled),
               10);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->chan5_scaled),
               12);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->chan6_scaled),
               14);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->chan7_scaled),
               16);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->chan8_scaled),
               18);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->port), 20);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->rssi), 21);
  return 22;
}

int MavLinkRcChannelsScaled::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->chan1_scaled), 4);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->chan2_scaled), 6);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->chan3_scaled), 8);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->chan4_scaled), 10);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->chan5_scaled), 12);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->chan6_scaled), 14);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->chan7_scaled), 16);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->chan8_scaled), 18);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->port), 20);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rssi), 21);
  return 22;
}

std::string MavLinkRcChannelsScaled::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"RC_CHANNELS_SCALED\", \"id\": 34, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"chan1_scaled\":" << this->chan1_scaled;
  ss << ", \"chan2_scaled\":" << this->chan2_scaled;
  ss << ", \"chan3_scaled\":" << this->chan3_scaled;
  ss << ", \"chan4_scaled\":" << this->chan4_scaled;
  ss << ", \"chan5_scaled\":" << this->chan5_scaled;
  ss << ", \"chan6_scaled\":" << this->chan6_scaled;
  ss << ", \"chan7_scaled\":" << this->chan7_scaled;
  ss << ", \"chan8_scaled\":" << this->chan8_scaled;
  ss << ", \"port\":" << static_cast<unsigned int>(this->port);
  ss << ", \"rssi\":" << static_cast<unsigned int>(this->rssi);
  ss << "} },";
  return ss.str();
}

int MavLinkRcChannelsRaw::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan1_raw), 4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan2_raw), 6);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan3_raw), 8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan4_raw),
                10);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan5_raw),
                12);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan6_raw),
                14);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan7_raw),
                16);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan8_raw),
                18);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->port), 20);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->rssi), 21);
  return 22;
}

int MavLinkRcChannelsRaw::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan1_raw), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan2_raw), 6);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan3_raw), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan4_raw), 10);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan5_raw), 12);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan6_raw), 14);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan7_raw), 16);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan8_raw), 18);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->port), 20);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rssi), 21);
  return 22;
}

std::string MavLinkRcChannelsRaw::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"RC_CHANNELS_RAW\", \"id\": 35, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"chan1_raw\":" << this->chan1_raw;
  ss << ", \"chan2_raw\":" << this->chan2_raw;
  ss << ", \"chan3_raw\":" << this->chan3_raw;
  ss << ", \"chan4_raw\":" << this->chan4_raw;
  ss << ", \"chan5_raw\":" << this->chan5_raw;
  ss << ", \"chan6_raw\":" << this->chan6_raw;
  ss << ", \"chan7_raw\":" << this->chan7_raw;
  ss << ", \"chan8_raw\":" << this->chan8_raw;
  ss << ", \"port\":" << static_cast<unsigned int>(this->port);
  ss << ", \"rssi\":" << static_cast<unsigned int>(this->rssi);
  ss << "} },";
  return ss.str();
}

int MavLinkServoOutputRaw::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_usec), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo1_raw),
                4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo2_raw),
                6);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo3_raw),
                8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo4_raw),
                10);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo5_raw),
                12);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo6_raw),
                14);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo7_raw),
                16);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo8_raw),
                18);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->port), 20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo9_raw),
                21);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo10_raw),
                23);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo11_raw),
                25);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo12_raw),
                27);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo13_raw),
                29);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo14_raw),
                31);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo15_raw),
                33);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->servo16_raw),
                35);
  return 37;
}

int MavLinkServoOutputRaw::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_usec), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo1_raw), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo2_raw), 6);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo3_raw), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo4_raw), 10);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo5_raw), 12);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo6_raw), 14);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo7_raw), 16);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo8_raw), 18);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->port), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo9_raw), 21);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo10_raw), 23);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo11_raw), 25);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo12_raw), 27);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo13_raw), 29);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo14_raw), 31);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo15_raw), 33);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->servo16_raw), 35);
  return 37;
}

std::string MavLinkServoOutputRaw::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SERVO_OUTPUT_RAW\", \"id\": 36, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"servo1_raw\":" << this->servo1_raw;
  ss << ", \"servo2_raw\":" << this->servo2_raw;
  ss << ", \"servo3_raw\":" << this->servo3_raw;
  ss << ", \"servo4_raw\":" << this->servo4_raw;
  ss << ", \"servo5_raw\":" << this->servo5_raw;
  ss << ", \"servo6_raw\":" << this->servo6_raw;
  ss << ", \"servo7_raw\":" << this->servo7_raw;
  ss << ", \"servo8_raw\":" << this->servo8_raw;
  ss << ", \"port\":" << static_cast<unsigned int>(this->port);
  ss << ", \"servo9_raw\":" << this->servo9_raw;
  ss << ", \"servo10_raw\":" << this->servo10_raw;
  ss << ", \"servo11_raw\":" << this->servo11_raw;
  ss << ", \"servo12_raw\":" << this->servo12_raw;
  ss << ", \"servo13_raw\":" << this->servo13_raw;
  ss << ", \"servo14_raw\":" << this->servo14_raw;
  ss << ", \"servo15_raw\":" << this->servo15_raw;
  ss << ", \"servo16_raw\":" << this->servo16_raw;
  ss << "} },";
  return ss.str();
}

int MavLinkMissionRequestPartialList::pack(char* buffer) const {
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->start_index), 0);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->end_index), 2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               4);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 5);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mission_type),
               6);
  return 7;
}

int MavLinkMissionRequestPartialList::unpack(const char* buffer) {
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->start_index), 0);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->end_index), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 5);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mission_type), 6);
  return 7;
}

std::string MavLinkMissionRequestPartialList::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_REQUEST_PARTIAL_LIST\", \"id\": 37, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"start_index\":" << this->start_index;
  ss << ", \"end_index\":" << this->end_index;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"mission_type\":" << static_cast<unsigned int>(this->mission_type);
  ss << "} },";
  return ss.str();
}

int MavLinkMissionWritePartialList::pack(char* buffer) const {
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->start_index), 0);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->end_index), 2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               4);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 5);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mission_type),
               6);
  return 7;
}

int MavLinkMissionWritePartialList::unpack(const char* buffer) {
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->start_index), 0);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->end_index), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 5);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mission_type), 6);
  return 7;
}

std::string MavLinkMissionWritePartialList::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_WRITE_PARTIAL_LIST\", \"id\": 38, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"start_index\":" << this->start_index;
  ss << ", \"end_index\":" << this->end_index;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"mission_type\":" << static_cast<unsigned int>(this->mission_type);
  ss << "} },";
  return ss.str();
}

int MavLinkMissionItem::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->param1), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param2), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param3), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param4), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->seq), 28);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->command), 30);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               32);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 33);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->frame), 34);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->current), 35);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->autocontinue),
               36);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mission_type),
               37);
  return 38;
}

int MavLinkMissionItem::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->param1), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param2), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param3), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param4), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->seq), 28);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->command), 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 33);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->frame), 34);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->current), 35);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->autocontinue), 36);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mission_type), 37);
  return 38;
}

std::string MavLinkMissionItem::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_ITEM\", \"id\": 39, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"param1\":" << float_tostring(this->param1);
  ss << ", \"param2\":" << float_tostring(this->param2);
  ss << ", \"param3\":" << float_tostring(this->param3);
  ss << ", \"param4\":" << float_tostring(this->param4);
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"seq\":" << this->seq;
  ss << ", \"command\":" << this->command;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"frame\":" << static_cast<unsigned int>(this->frame);
  ss << ", \"current\":" << static_cast<unsigned int>(this->current);
  ss << ", \"autocontinue\":" << static_cast<unsigned int>(this->autocontinue);
  ss << ", \"mission_type\":" << static_cast<unsigned int>(this->mission_type);
  ss << "} },";
  return ss.str();
}

int MavLinkMissionRequest::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->seq), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 3);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mission_type),
               4);
  return 5;
}

int MavLinkMissionRequest::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->seq), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 3);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mission_type), 4);
  return 5;
}

std::string MavLinkMissionRequest::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_REQUEST\", \"id\": 40, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"seq\":" << this->seq;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"mission_type\":" << static_cast<unsigned int>(this->mission_type);
  ss << "} },";
  return ss.str();
}

int MavLinkMissionSetCurrent::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->seq), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 3);
  return 4;
}

int MavLinkMissionSetCurrent::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->seq), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 3);
  return 4;
}

std::string MavLinkMissionSetCurrent::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_SET_CURRENT\", \"id\": 41, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"seq\":" << this->seq;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkMissionCurrent::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->seq), 0);
  return 2;
}

int MavLinkMissionCurrent::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->seq), 0);
  return 2;
}

std::string MavLinkMissionCurrent::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_CURRENT\", \"id\": 42, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"seq\":" << this->seq;
  ss << "} },";
  return ss.str();
}

int MavLinkMissionRequestList::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mission_type),
               2);
  return 3;
}

int MavLinkMissionRequestList::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mission_type), 2);
  return 3;
}

std::string MavLinkMissionRequestList::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_REQUEST_LIST\", \"id\": 43, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"mission_type\":" << static_cast<unsigned int>(this->mission_type);
  ss << "} },";
  return ss.str();
}

int MavLinkMissionCount::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->count), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 3);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mission_type),
               4);
  return 5;
}

int MavLinkMissionCount::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->count), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 3);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mission_type), 4);
  return 5;
}

std::string MavLinkMissionCount::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_COUNT\", \"id\": 44, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"count\":" << this->count;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"mission_type\":" << static_cast<unsigned int>(this->mission_type);
  ss << "} },";
  return ss.str();
}

int MavLinkMissionClearAll::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mission_type),
               2);
  return 3;
}

int MavLinkMissionClearAll::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mission_type), 2);
  return 3;
}

std::string MavLinkMissionClearAll::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_CLEAR_ALL\", \"id\": 45, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"mission_type\":" << static_cast<unsigned int>(this->mission_type);
  ss << "} },";
  return ss.str();
}

int MavLinkMissionItemReached::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->seq), 0);
  return 2;
}

int MavLinkMissionItemReached::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->seq), 0);
  return 2;
}

std::string MavLinkMissionItemReached::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_ITEM_REACHED\", \"id\": 46, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"seq\":" << this->seq;
  ss << "} },";
  return ss.str();
}

int MavLinkMissionAck::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mission_type),
               3);
  return 4;
}

int MavLinkMissionAck::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mission_type), 3);
  return 4;
}

std::string MavLinkMissionAck::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_ACK\", \"id\": 47, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"mission_type\":" << static_cast<unsigned int>(this->mission_type);
  ss << "} },";
  return ss.str();
}

int MavLinkSetGpsGlobalOrigin::pack(char* buffer) const {
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->latitude), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->longitude), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->altitude), 8);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               12);
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec),
                13);
  return 21;
}

int MavLinkSetGpsGlobalOrigin::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->latitude), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->longitude), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->altitude), 8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 12);
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 13);
  return 21;
}

std::string MavLinkSetGpsGlobalOrigin::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SET_GPS_GLOBAL_ORIGIN\", \"id\": 48, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"latitude\":" << this->latitude;
  ss << ", \"longitude\":" << this->longitude;
  ss << ", \"altitude\":" << this->altitude;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"time_usec\":" << this->time_usec;
  ss << "} },";
  return ss.str();
}

int MavLinkGpsGlobalOrigin::pack(char* buffer) const {
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->latitude), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->longitude), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->altitude), 8);
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec),
                12);
  return 20;
}

int MavLinkGpsGlobalOrigin::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->latitude), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->longitude), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->altitude), 8);
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 12);
  return 20;
}

std::string MavLinkGpsGlobalOrigin::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GPS_GLOBAL_ORIGIN\", \"id\": 49, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"latitude\":" << this->latitude;
  ss << ", \"longitude\":" << this->longitude;
  ss << ", \"altitude\":" << this->altitude;
  ss << ", \"time_usec\":" << this->time_usec;
  ss << "} },";
  return ss.str();
}

int MavLinkParamMapRc::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->param_value0), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->scale), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param_value_min), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param_value_max),
             12);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->param_index),
               16);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               18);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 19);
  pack_char_array(16, buffer, reinterpret_cast<const char*>(&this->param_id[0]),
                  20);
  pack_uint8_t(
      buffer,
      reinterpret_cast<const uint8_t*>(&this->parameter_rc_channel_index), 36);
  return 37;
}

int MavLinkParamMapRc::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->param_value0), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->scale), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param_value_min), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param_value_max), 12);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->param_index), 16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 18);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 19);
  unpack_char_array(16, buffer, reinterpret_cast<char*>(&this->param_id[0]),
                    20);
  unpack_uint8_t(buffer,
                 reinterpret_cast<uint8_t*>(&this->parameter_rc_channel_index),
                 36);
  return 37;
}

std::string MavLinkParamMapRc::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PARAM_MAP_RC\", \"id\": 50, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"param_value0\":" << float_tostring(this->param_value0);
  ss << ", \"scale\":" << float_tostring(this->scale);
  ss << ", \"param_value_min\":" << float_tostring(this->param_value_min);
  ss << ", \"param_value_max\":" << float_tostring(this->param_value_max);
  ss << ", \"param_index\":" << this->param_index;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"param_id\":"
     << "\""
     << char_array_tostring(16, reinterpret_cast<char*>(&this->param_id[0]))
     << "\"";
  ss << ", \"parameter_rc_channel_index\":"
     << static_cast<unsigned int>(this->parameter_rc_channel_index);
  ss << "} },";
  return ss.str();
}

int MavLinkMissionRequestInt::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->seq), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 3);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mission_type),
               4);
  return 5;
}

int MavLinkMissionRequestInt::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->seq), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 3);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mission_type), 4);
  return 5;
}

std::string MavLinkMissionRequestInt::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_REQUEST_INT\", \"id\": 51, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"seq\":" << this->seq;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"mission_type\":" << static_cast<unsigned int>(this->mission_type);
  ss << "} },";
  return ss.str();
}

int MavLinkSafetySetAllowedArea::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->p1x), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->p1y), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->p1z), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->p2x), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->p2y), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->p2z), 20);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               24);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 25);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->frame), 26);
  return 27;
}

int MavLinkSafetySetAllowedArea::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->p1x), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->p1y), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->p1z), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->p2x), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->p2y), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->p2z), 20);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 24);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 25);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->frame), 26);
  return 27;
}

std::string MavLinkSafetySetAllowedArea::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SAFETY_SET_ALLOWED_AREA\", \"id\": 54, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"p1x\":" << float_tostring(this->p1x);
  ss << ", \"p1y\":" << float_tostring(this->p1y);
  ss << ", \"p1z\":" << float_tostring(this->p1z);
  ss << ", \"p2x\":" << float_tostring(this->p2x);
  ss << ", \"p2y\":" << float_tostring(this->p2y);
  ss << ", \"p2z\":" << float_tostring(this->p2z);
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"frame\":" << static_cast<unsigned int>(this->frame);
  ss << "} },";
  return ss.str();
}

int MavLinkSafetyAllowedArea::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->p1x), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->p1y), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->p1z), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->p2x), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->p2y), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->p2z), 20);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->frame), 24);
  return 25;
}

int MavLinkSafetyAllowedArea::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->p1x), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->p1y), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->p1z), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->p2x), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->p2y), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->p2z), 20);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->frame), 24);
  return 25;
}

std::string MavLinkSafetyAllowedArea::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SAFETY_ALLOWED_AREA\", \"id\": 55, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"p1x\":" << float_tostring(this->p1x);
  ss << ", \"p1y\":" << float_tostring(this->p1y);
  ss << ", \"p1z\":" << float_tostring(this->p1z);
  ss << ", \"p2x\":" << float_tostring(this->p2x);
  ss << ", \"p2y\":" << float_tostring(this->p2y);
  ss << ", \"p2z\":" << float_tostring(this->p2z);
  ss << ", \"frame\":" << static_cast<unsigned int>(this->frame);
  ss << "} },";
  return ss.str();
}

int MavLinkAttitudeQuaternionCov::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->rollspeed), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitchspeed), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yawspeed), 32);
  pack_float_array(9, buffer,
                   reinterpret_cast<const float*>(&this->covariance[0]), 36);
  return 72;
}

int MavLinkAttitudeQuaternionCov::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->rollspeed), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitchspeed), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yawspeed), 32);
  unpack_float_array(9, buffer, reinterpret_cast<float*>(&this->covariance[0]),
                     36);
  return 72;
}

std::string MavLinkAttitudeQuaternionCov::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ATTITUDE_QUATERNION_COV\", \"id\": 61, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"rollspeed\":" << float_tostring(this->rollspeed);
  ss << ", \"pitchspeed\":" << float_tostring(this->pitchspeed);
  ss << ", \"yawspeed\":" << float_tostring(this->yawspeed);
  ss << ", \"covariance\":"
     << "["
     << float_array_tostring(9, reinterpret_cast<float*>(&this->covariance[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkNavControllerOutput::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->nav_roll), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->nav_pitch), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->alt_error), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->aspd_error), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->xtrack_error), 16);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->nav_bearing),
               20);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->target_bearing),
               22);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->wp_dist), 24);
  return 26;
}

int MavLinkNavControllerOutput::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->nav_roll), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->nav_pitch), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->alt_error), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->aspd_error), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->xtrack_error), 16);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->nav_bearing), 20);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->target_bearing), 22);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->wp_dist), 24);
  return 26;
}

std::string MavLinkNavControllerOutput::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"NAV_CONTROLLER_OUTPUT\", \"id\": 62, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"nav_roll\":" << float_tostring(this->nav_roll);
  ss << ", \"nav_pitch\":" << float_tostring(this->nav_pitch);
  ss << ", \"alt_error\":" << float_tostring(this->alt_error);
  ss << ", \"aspd_error\":" << float_tostring(this->aspd_error);
  ss << ", \"xtrack_error\":" << float_tostring(this->xtrack_error);
  ss << ", \"nav_bearing\":" << this->nav_bearing;
  ss << ", \"target_bearing\":" << this->target_bearing;
  ss << ", \"wp_dist\":" << this->wp_dist;
  ss << "} },";
  return ss.str();
}

int MavLinkGlobalPositionIntCov::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt), 16);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->relative_alt),
               20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vx), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vy), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vz), 32);
  pack_float_array(36, buffer,
                   reinterpret_cast<const float*>(&this->covariance[0]), 36);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->estimator_type),
               180);
  return 181;
}

int MavLinkGlobalPositionIntCov::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt), 16);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->relative_alt), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vx), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vy), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vz), 32);
  unpack_float_array(36, buffer, reinterpret_cast<float*>(&this->covariance[0]),
                     36);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->estimator_type),
                 180);
  return 181;
}

std::string MavLinkGlobalPositionIntCov::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GLOBAL_POSITION_INT_COV\", \"id\": 63, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"alt\":" << this->alt;
  ss << ", \"relative_alt\":" << this->relative_alt;
  ss << ", \"vx\":" << float_tostring(this->vx);
  ss << ", \"vy\":" << float_tostring(this->vy);
  ss << ", \"vz\":" << float_tostring(this->vz);
  ss << ", \"covariance\":"
     << "["
     << float_array_tostring(36, reinterpret_cast<float*>(&this->covariance[0]))
     << "]";
  ss << ", \"estimator_type\":"
     << static_cast<unsigned int>(this->estimator_type);
  ss << "} },";
  return ss.str();
}

int MavLinkLocalPositionNedCov::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vx), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vy), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vz), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ax), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ay), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->az), 40);
  pack_float_array(45, buffer,
                   reinterpret_cast<const float*>(&this->covariance[0]), 44);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->estimator_type),
               224);
  return 225;
}

int MavLinkLocalPositionNedCov::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vx), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vy), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vz), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ax), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ay), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->az), 40);
  unpack_float_array(45, buffer, reinterpret_cast<float*>(&this->covariance[0]),
                     44);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->estimator_type),
                 224);
  return 225;
}

std::string MavLinkLocalPositionNedCov::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LOCAL_POSITION_NED_COV\", \"id\": 64, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"vx\":" << float_tostring(this->vx);
  ss << ", \"vy\":" << float_tostring(this->vy);
  ss << ", \"vz\":" << float_tostring(this->vz);
  ss << ", \"ax\":" << float_tostring(this->ax);
  ss << ", \"ay\":" << float_tostring(this->ay);
  ss << ", \"az\":" << float_tostring(this->az);
  ss << ", \"covariance\":"
     << "["
     << float_array_tostring(45, reinterpret_cast<float*>(&this->covariance[0]))
     << "]";
  ss << ", \"estimator_type\":"
     << static_cast<unsigned int>(this->estimator_type);
  ss << "} },";
  return ss.str();
}

int MavLinkRcChannels::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan1_raw), 4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan2_raw), 6);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan3_raw), 8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan4_raw),
                10);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan5_raw),
                12);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan6_raw),
                14);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan7_raw),
                16);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan8_raw),
                18);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan9_raw),
                20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan10_raw),
                22);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan11_raw),
                24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan12_raw),
                26);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan13_raw),
                28);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan14_raw),
                30);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan15_raw),
                32);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan16_raw),
                34);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan17_raw),
                36);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan18_raw),
                38);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->chancount), 40);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->rssi), 41);
  return 42;
}

int MavLinkRcChannels::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan1_raw), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan2_raw), 6);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan3_raw), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan4_raw), 10);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan5_raw), 12);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan6_raw), 14);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan7_raw), 16);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan8_raw), 18);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan9_raw), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan10_raw), 22);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan11_raw), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan12_raw), 26);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan13_raw), 28);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan14_raw), 30);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan15_raw), 32);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan16_raw), 34);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan17_raw), 36);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan18_raw), 38);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->chancount), 40);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rssi), 41);
  return 42;
}

std::string MavLinkRcChannels::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"RC_CHANNELS\", \"id\": 65, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"chan1_raw\":" << this->chan1_raw;
  ss << ", \"chan2_raw\":" << this->chan2_raw;
  ss << ", \"chan3_raw\":" << this->chan3_raw;
  ss << ", \"chan4_raw\":" << this->chan4_raw;
  ss << ", \"chan5_raw\":" << this->chan5_raw;
  ss << ", \"chan6_raw\":" << this->chan6_raw;
  ss << ", \"chan7_raw\":" << this->chan7_raw;
  ss << ", \"chan8_raw\":" << this->chan8_raw;
  ss << ", \"chan9_raw\":" << this->chan9_raw;
  ss << ", \"chan10_raw\":" << this->chan10_raw;
  ss << ", \"chan11_raw\":" << this->chan11_raw;
  ss << ", \"chan12_raw\":" << this->chan12_raw;
  ss << ", \"chan13_raw\":" << this->chan13_raw;
  ss << ", \"chan14_raw\":" << this->chan14_raw;
  ss << ", \"chan15_raw\":" << this->chan15_raw;
  ss << ", \"chan16_raw\":" << this->chan16_raw;
  ss << ", \"chan17_raw\":" << this->chan17_raw;
  ss << ", \"chan18_raw\":" << this->chan18_raw;
  ss << ", \"chancount\":" << static_cast<unsigned int>(this->chancount);
  ss << ", \"rssi\":" << static_cast<unsigned int>(this->rssi);
  ss << "} },";
  return ss.str();
}

int MavLinkRequestDataStream::pack(char* buffer) const {
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->req_message_rate), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 3);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->req_stream_id),
               4);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->start_stop), 5);
  return 6;
}

int MavLinkRequestDataStream::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->req_message_rate),
                  0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 3);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->req_stream_id), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->start_stop), 5);
  return 6;
}

std::string MavLinkRequestDataStream::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"REQUEST_DATA_STREAM\", \"id\": 66, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"req_message_rate\":" << this->req_message_rate;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"req_stream_id\":"
     << static_cast<unsigned int>(this->req_stream_id);
  ss << ", \"start_stop\":" << static_cast<unsigned int>(this->start_stop);
  ss << "} },";
  return ss.str();
}

int MavLinkDataStream::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->message_rate),
                0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->stream_id), 2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->on_off), 3);
  return 4;
}

int MavLinkDataStream::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->message_rate), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->stream_id), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->on_off), 3);
  return 4;
}

std::string MavLinkDataStream::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"DATA_STREAM\", \"id\": 67, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"message_rate\":" << this->message_rate;
  ss << ", \"stream_id\":" << static_cast<unsigned int>(this->stream_id);
  ss << ", \"on_off\":" << static_cast<unsigned int>(this->on_off);
  ss << "} },";
  return ss.str();
}

int MavLinkManualControl::pack(char* buffer) const {
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->x), 0);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->y), 2);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->z), 4);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->r), 6);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->buttons), 8);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target), 10);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->buttons2), 11);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->enabled_extensions), 13);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->s), 14);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->t), 16);
  return 18;
}

int MavLinkManualControl::unpack(const char* buffer) {
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->x), 0);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->y), 2);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->z), 4);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->r), 6);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->buttons), 8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target), 10);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->buttons2), 11);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->enabled_extensions),
                 13);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->s), 14);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->t), 16);
  return 18;
}

std::string MavLinkManualControl::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MANUAL_CONTROL\", \"id\": 69, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"x\":" << this->x;
  ss << ", \"y\":" << this->y;
  ss << ", \"z\":" << this->z;
  ss << ", \"r\":" << this->r;
  ss << ", \"buttons\":" << this->buttons;
  ss << ", \"target\":" << static_cast<unsigned int>(this->target);
  ss << ", \"buttons2\":" << this->buttons2;
  ss << ", \"enabled_extensions\":"
     << static_cast<unsigned int>(this->enabled_extensions);
  ss << ", \"s\":" << this->s;
  ss << ", \"t\":" << this->t;
  ss << "} },";
  return ss.str();
}

int MavLinkRcChannelsOverride::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan1_raw), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan2_raw), 2);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan3_raw), 4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan4_raw), 6);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan5_raw), 8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan6_raw),
                10);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan7_raw),
                12);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan8_raw),
                14);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               16);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 17);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan9_raw),
                18);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan10_raw),
                20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan11_raw),
                22);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan12_raw),
                24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan13_raw),
                26);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan14_raw),
                28);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan15_raw),
                30);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan16_raw),
                32);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan17_raw),
                34);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan18_raw),
                36);
  return 38;
}

int MavLinkRcChannelsOverride::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan1_raw), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan2_raw), 2);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan3_raw), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan4_raw), 6);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan5_raw), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan6_raw), 10);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan7_raw), 12);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan8_raw), 14);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 17);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan9_raw), 18);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan10_raw), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan11_raw), 22);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan12_raw), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan13_raw), 26);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan14_raw), 28);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan15_raw), 30);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan16_raw), 32);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan17_raw), 34);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan18_raw), 36);
  return 38;
}

std::string MavLinkRcChannelsOverride::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"RC_CHANNELS_OVERRIDE\", \"id\": 70, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"chan1_raw\":" << this->chan1_raw;
  ss << ", \"chan2_raw\":" << this->chan2_raw;
  ss << ", \"chan3_raw\":" << this->chan3_raw;
  ss << ", \"chan4_raw\":" << this->chan4_raw;
  ss << ", \"chan5_raw\":" << this->chan5_raw;
  ss << ", \"chan6_raw\":" << this->chan6_raw;
  ss << ", \"chan7_raw\":" << this->chan7_raw;
  ss << ", \"chan8_raw\":" << this->chan8_raw;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"chan9_raw\":" << this->chan9_raw;
  ss << ", \"chan10_raw\":" << this->chan10_raw;
  ss << ", \"chan11_raw\":" << this->chan11_raw;
  ss << ", \"chan12_raw\":" << this->chan12_raw;
  ss << ", \"chan13_raw\":" << this->chan13_raw;
  ss << ", \"chan14_raw\":" << this->chan14_raw;
  ss << ", \"chan15_raw\":" << this->chan15_raw;
  ss << ", \"chan16_raw\":" << this->chan16_raw;
  ss << ", \"chan17_raw\":" << this->chan17_raw;
  ss << ", \"chan18_raw\":" << this->chan18_raw;
  ss << "} },";
  return ss.str();
}

int MavLinkMissionItemInt::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->param1), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param2), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param3), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param4), 12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->x), 16);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->y), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->seq), 28);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->command), 30);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               32);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 33);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->frame), 34);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->current), 35);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->autocontinue),
               36);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mission_type),
               37);
  return 38;
}

int MavLinkMissionItemInt::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->param1), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param2), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param3), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param4), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->x), 16);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->y), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->seq), 28);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->command), 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 33);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->frame), 34);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->current), 35);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->autocontinue), 36);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mission_type), 37);
  return 38;
}

std::string MavLinkMissionItemInt::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MISSION_ITEM_INT\", \"id\": 73, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"param1\":" << float_tostring(this->param1);
  ss << ", \"param2\":" << float_tostring(this->param2);
  ss << ", \"param3\":" << float_tostring(this->param3);
  ss << ", \"param4\":" << float_tostring(this->param4);
  ss << ", \"x\":" << this->x;
  ss << ", \"y\":" << this->y;
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"seq\":" << this->seq;
  ss << ", \"command\":" << this->command;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"frame\":" << static_cast<unsigned int>(this->frame);
  ss << ", \"current\":" << static_cast<unsigned int>(this->current);
  ss << ", \"autocontinue\":" << static_cast<unsigned int>(this->autocontinue);
  ss << ", \"mission_type\":" << static_cast<unsigned int>(this->mission_type);
  ss << "} },";
  return ss.str();
}

int MavLinkVfrHud::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->airspeed), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->groundspeed), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->alt), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->climb), 12);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->heading), 16);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->throttle), 18);
  return 20;
}

int MavLinkVfrHud::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->airspeed), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->groundspeed), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->alt), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->climb), 12);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->heading), 16);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->throttle), 18);
  return 20;
}

std::string MavLinkVfrHud::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"VFR_HUD\", \"id\": 74, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"airspeed\":" << float_tostring(this->airspeed);
  ss << ", \"groundspeed\":" << float_tostring(this->groundspeed);
  ss << ", \"alt\":" << float_tostring(this->alt);
  ss << ", \"climb\":" << float_tostring(this->climb);
  ss << ", \"heading\":" << this->heading;
  ss << ", \"throttle\":" << this->throttle;
  ss << "} },";
  return ss.str();
}

int MavLinkCommandInt::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->param1), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param2), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param3), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param4), 12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->x), 16);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->y), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->command), 28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               30);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 31);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->frame), 32);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->current), 33);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->autocontinue),
               34);
  return 35;
}

int MavLinkCommandInt::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->param1), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param2), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param3), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param4), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->x), 16);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->y), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->command), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 31);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->frame), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->current), 33);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->autocontinue), 34);
  return 35;
}

std::string MavLinkCommandInt::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"COMMAND_INT\", \"id\": 75, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"param1\":" << float_tostring(this->param1);
  ss << ", \"param2\":" << float_tostring(this->param2);
  ss << ", \"param3\":" << float_tostring(this->param3);
  ss << ", \"param4\":" << float_tostring(this->param4);
  ss << ", \"x\":" << this->x;
  ss << ", \"y\":" << this->y;
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"command\":" << this->command;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"frame\":" << static_cast<unsigned int>(this->frame);
  ss << ", \"current\":" << static_cast<unsigned int>(this->current);
  ss << ", \"autocontinue\":" << static_cast<unsigned int>(this->autocontinue);
  ss << "} },";
  return ss.str();
}

int MavLinkCommandLong::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->param1), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param2), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param3), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param4), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param5), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param6), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->param7), 24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->command), 28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               30);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 31);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->confirmation),
               32);
  return 33;
}

int MavLinkCommandLong::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->param1), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param2), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param3), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param4), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param5), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param6), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->param7), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->command), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 31);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->confirmation), 32);
  return 33;
}

std::string MavLinkCommandLong::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"COMMAND_LONG\", \"id\": 76, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"param1\":" << float_tostring(this->param1);
  ss << ", \"param2\":" << float_tostring(this->param2);
  ss << ", \"param3\":" << float_tostring(this->param3);
  ss << ", \"param4\":" << float_tostring(this->param4);
  ss << ", \"param5\":" << float_tostring(this->param5);
  ss << ", \"param6\":" << float_tostring(this->param6);
  ss << ", \"param7\":" << float_tostring(this->param7);
  ss << ", \"command\":" << this->command;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"confirmation\":" << static_cast<unsigned int>(this->confirmation);
  ss << "} },";
  return ss.str();
}

int MavLinkCommandAck::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->command), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->result), 2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->progress), 3);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->result_param2),
               4);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               8);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 9);
  return 10;
}

int MavLinkCommandAck::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->command), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->result), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->progress), 3);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->result_param2), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 9);
  return 10;
}

std::string MavLinkCommandAck::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"COMMAND_ACK\", \"id\": 77, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"command\":" << this->command;
  ss << ", \"result\":" << static_cast<unsigned int>(this->result);
  ss << ", \"progress\":" << static_cast<unsigned int>(this->progress);
  ss << ", \"result_param2\":" << this->result_param2;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkCommandCancel::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->command), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 3);
  return 4;
}

int MavLinkCommandCancel::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->command), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 3);
  return 4;
}

std::string MavLinkCommandCancel::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"COMMAND_CANCEL\", \"id\": 80, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"command\":" << this->command;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkManualSetpoint::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->thrust), 16);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mode_switch),
               20);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->manual_override_switch),
               21);
  return 22;
}

int MavLinkManualSetpoint::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->thrust), 16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mode_switch), 20);
  unpack_uint8_t(buffer,
                 reinterpret_cast<uint8_t*>(&this->manual_override_switch), 21);
  return 22;
}

std::string MavLinkManualSetpoint::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MANUAL_SETPOINT\", \"id\": 81, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"roll\":" << float_tostring(this->roll);
  ss << ", \"pitch\":" << float_tostring(this->pitch);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"thrust\":" << float_tostring(this->thrust);
  ss << ", \"mode_switch\":" << static_cast<unsigned int>(this->mode_switch);
  ss << ", \"manual_override_switch\":"
     << static_cast<unsigned int>(this->manual_override_switch);
  ss << "} },";
  return ss.str();
}

int MavLinkSetAttitudeTarget::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->body_roll_rate), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->body_pitch_rate),
             24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->body_yaw_rate), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->thrust), 32);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               36);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 37);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type_mask), 38);
  pack_float_array(3, buffer,
                   reinterpret_cast<const float*>(&this->thrust_body[0]), 39);
  return 51;
}

int MavLinkSetAttitudeTarget::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->body_roll_rate), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->body_pitch_rate), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->body_yaw_rate), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->thrust), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 36);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 37);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type_mask), 38);
  unpack_float_array(3, buffer, reinterpret_cast<float*>(&this->thrust_body[0]),
                     39);
  return 51;
}

std::string MavLinkSetAttitudeTarget::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SET_ATTITUDE_TARGET\", \"id\": 82, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"body_roll_rate\":" << float_tostring(this->body_roll_rate);
  ss << ", \"body_pitch_rate\":" << float_tostring(this->body_pitch_rate);
  ss << ", \"body_yaw_rate\":" << float_tostring(this->body_yaw_rate);
  ss << ", \"thrust\":" << float_tostring(this->thrust);
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"type_mask\":" << static_cast<unsigned int>(this->type_mask);
  ss << ", \"thrust_body\":"
     << "["
     << float_array_tostring(3, reinterpret_cast<float*>(&this->thrust_body[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkAttitudeTarget::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->body_roll_rate), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->body_pitch_rate),
             24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->body_yaw_rate), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->thrust), 32);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type_mask), 36);
  return 37;
}

int MavLinkAttitudeTarget::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->body_roll_rate), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->body_pitch_rate), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->body_yaw_rate), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->thrust), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type_mask), 36);
  return 37;
}

std::string MavLinkAttitudeTarget::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ATTITUDE_TARGET\", \"id\": 83, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"body_roll_rate\":" << float_tostring(this->body_roll_rate);
  ss << ", \"body_pitch_rate\":" << float_tostring(this->body_pitch_rate);
  ss << ", \"body_yaw_rate\":" << float_tostring(this->body_yaw_rate);
  ss << ", \"thrust\":" << float_tostring(this->thrust);
  ss << ", \"type_mask\":" << static_cast<unsigned int>(this->type_mask);
  ss << "} },";
  return ss.str();
}

int MavLinkSetPositionTargetLocalNed::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vx), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vy), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vz), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->afx), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->afy), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->afz), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_rate), 44);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->type_mask),
                48);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               50);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 51);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->coordinate_frame), 52);
  return 53;
}

int MavLinkSetPositionTargetLocalNed::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vx), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vy), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vz), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->afx), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->afy), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->afz), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_rate), 44);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->type_mask), 48);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 50);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 51);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->coordinate_frame),
                 52);
  return 53;
}

std::string MavLinkSetPositionTargetLocalNed::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SET_POSITION_TARGET_LOCAL_NED\", \"id\": 84, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"vx\":" << float_tostring(this->vx);
  ss << ", \"vy\":" << float_tostring(this->vy);
  ss << ", \"vz\":" << float_tostring(this->vz);
  ss << ", \"afx\":" << float_tostring(this->afx);
  ss << ", \"afy\":" << float_tostring(this->afy);
  ss << ", \"afz\":" << float_tostring(this->afz);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"yaw_rate\":" << float_tostring(this->yaw_rate);
  ss << ", \"type_mask\":" << this->type_mask;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"coordinate_frame\":"
     << static_cast<unsigned int>(this->coordinate_frame);
  ss << "} },";
  return ss.str();
}

int MavLinkPositionTargetLocalNed::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vx), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vy), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vz), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->afx), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->afy), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->afz), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_rate), 44);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->type_mask),
                48);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->coordinate_frame), 50);
  return 51;
}

int MavLinkPositionTargetLocalNed::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vx), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vy), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vz), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->afx), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->afy), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->afz), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_rate), 44);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->type_mask), 48);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->coordinate_frame),
                 50);
  return 51;
}

std::string MavLinkPositionTargetLocalNed::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"POSITION_TARGET_LOCAL_NED\", \"id\": 85, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"vx\":" << float_tostring(this->vx);
  ss << ", \"vy\":" << float_tostring(this->vy);
  ss << ", \"vz\":" << float_tostring(this->vz);
  ss << ", \"afx\":" << float_tostring(this->afx);
  ss << ", \"afy\":" << float_tostring(this->afy);
  ss << ", \"afz\":" << float_tostring(this->afz);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"yaw_rate\":" << float_tostring(this->yaw_rate);
  ss << ", \"type_mask\":" << this->type_mask;
  ss << ", \"coordinate_frame\":"
     << static_cast<unsigned int>(this->coordinate_frame);
  ss << "} },";
  return ss.str();
}

int MavLinkSetPositionTargetGlobalInt::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat_int), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon_int), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->alt), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vx), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vy), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vz), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->afx), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->afy), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->afz), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_rate), 44);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->type_mask),
                48);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               50);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 51);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->coordinate_frame), 52);
  return 53;
}

int MavLinkSetPositionTargetGlobalInt::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat_int), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon_int), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->alt), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vx), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vy), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vz), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->afx), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->afy), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->afz), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_rate), 44);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->type_mask), 48);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 50);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 51);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->coordinate_frame),
                 52);
  return 53;
}

std::string MavLinkSetPositionTargetGlobalInt::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SET_POSITION_TARGET_GLOBAL_INT\", \"id\": 86, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"lat_int\":" << this->lat_int;
  ss << ", \"lon_int\":" << this->lon_int;
  ss << ", \"alt\":" << float_tostring(this->alt);
  ss << ", \"vx\":" << float_tostring(this->vx);
  ss << ", \"vy\":" << float_tostring(this->vy);
  ss << ", \"vz\":" << float_tostring(this->vz);
  ss << ", \"afx\":" << float_tostring(this->afx);
  ss << ", \"afy\":" << float_tostring(this->afy);
  ss << ", \"afz\":" << float_tostring(this->afz);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"yaw_rate\":" << float_tostring(this->yaw_rate);
  ss << ", \"type_mask\":" << this->type_mask;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"coordinate_frame\":"
     << static_cast<unsigned int>(this->coordinate_frame);
  ss << "} },";
  return ss.str();
}

int MavLinkPositionTargetGlobalInt::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat_int), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon_int), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->alt), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vx), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vy), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vz), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->afx), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->afy), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->afz), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_rate), 44);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->type_mask),
                48);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->coordinate_frame), 50);
  return 51;
}

int MavLinkPositionTargetGlobalInt::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat_int), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon_int), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->alt), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vx), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vy), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vz), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->afx), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->afy), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->afz), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_rate), 44);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->type_mask), 48);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->coordinate_frame),
                 50);
  return 51;
}

std::string MavLinkPositionTargetGlobalInt::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"POSITION_TARGET_GLOBAL_INT\", \"id\": 87, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"lat_int\":" << this->lat_int;
  ss << ", \"lon_int\":" << this->lon_int;
  ss << ", \"alt\":" << float_tostring(this->alt);
  ss << ", \"vx\":" << float_tostring(this->vx);
  ss << ", \"vy\":" << float_tostring(this->vy);
  ss << ", \"vz\":" << float_tostring(this->vz);
  ss << ", \"afx\":" << float_tostring(this->afx);
  ss << ", \"afy\":" << float_tostring(this->afy);
  ss << ", \"afz\":" << float_tostring(this->afz);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"yaw_rate\":" << float_tostring(this->yaw_rate);
  ss << ", \"type_mask\":" << this->type_mask;
  ss << ", \"coordinate_frame\":"
     << static_cast<unsigned int>(this->coordinate_frame);
  ss << "} },";
  return ss.str();
}

int MavLinkLocalPositionNedSystemGlobalOffset::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 24);
  return 28;
}

int MavLinkLocalPositionNedSystemGlobalOffset::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 24);
  return 28;
}

std::string MavLinkLocalPositionNedSystemGlobalOffset::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET\", \"id\": 89, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"roll\":" << float_tostring(this->roll);
  ss << ", \"pitch\":" << float_tostring(this->pitch);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << "} },";
  return ss.str();
}

int MavLinkHilState::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->rollspeed), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitchspeed), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yawspeed), 28);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 32);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 36);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt), 40);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vx), 44);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vy), 46);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vz), 48);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xacc), 50);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->yacc), 52);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zacc), 54);
  return 56;
}

int MavLinkHilState::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->rollspeed), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitchspeed), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yawspeed), 28);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 32);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 36);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt), 40);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vx), 44);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vy), 46);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vz), 48);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xacc), 50);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->yacc), 52);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zacc), 54);
  return 56;
}

std::string MavLinkHilState::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HIL_STATE\", \"id\": 90, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"roll\":" << float_tostring(this->roll);
  ss << ", \"pitch\":" << float_tostring(this->pitch);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"rollspeed\":" << float_tostring(this->rollspeed);
  ss << ", \"pitchspeed\":" << float_tostring(this->pitchspeed);
  ss << ", \"yawspeed\":" << float_tostring(this->yawspeed);
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"alt\":" << this->alt;
  ss << ", \"vx\":" << this->vx;
  ss << ", \"vy\":" << this->vy;
  ss << ", \"vz\":" << this->vz;
  ss << ", \"xacc\":" << this->xacc;
  ss << ", \"yacc\":" << this->yacc;
  ss << ", \"zacc\":" << this->zacc;
  ss << "} },";
  return ss.str();
}

int MavLinkHilControls::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll_ailerons), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch_elevator), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_rudder), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->throttle), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->aux1), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->aux2), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->aux3), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->aux4), 36);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mode), 40);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->nav_mode), 41);
  return 42;
}

int MavLinkHilControls::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll_ailerons), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch_elevator), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_rudder), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->throttle), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->aux1), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->aux2), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->aux3), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->aux4), 36);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mode), 40);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->nav_mode), 41);
  return 42;
}

std::string MavLinkHilControls::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HIL_CONTROLS\", \"id\": 91, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"roll_ailerons\":" << float_tostring(this->roll_ailerons);
  ss << ", \"pitch_elevator\":" << float_tostring(this->pitch_elevator);
  ss << ", \"yaw_rudder\":" << float_tostring(this->yaw_rudder);
  ss << ", \"throttle\":" << float_tostring(this->throttle);
  ss << ", \"aux1\":" << float_tostring(this->aux1);
  ss << ", \"aux2\":" << float_tostring(this->aux2);
  ss << ", \"aux3\":" << float_tostring(this->aux3);
  ss << ", \"aux4\":" << float_tostring(this->aux4);
  ss << ", \"mode\":" << static_cast<unsigned int>(this->mode);
  ss << ", \"nav_mode\":" << static_cast<unsigned int>(this->nav_mode);
  ss << "} },";
  return ss.str();
}

int MavLinkHilRcInputsRaw::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan1_raw), 8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan2_raw),
                10);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan3_raw),
                12);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan4_raw),
                14);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan5_raw),
                16);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan6_raw),
                18);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan7_raw),
                20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan8_raw),
                22);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan9_raw),
                24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan10_raw),
                26);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan11_raw),
                28);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->chan12_raw),
                30);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->rssi), 32);
  return 33;
}

int MavLinkHilRcInputsRaw::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan1_raw), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan2_raw), 10);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan3_raw), 12);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan4_raw), 14);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan5_raw), 16);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan6_raw), 18);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan7_raw), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan8_raw), 22);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan9_raw), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan10_raw), 26);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan11_raw), 28);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->chan12_raw), 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rssi), 32);
  return 33;
}

std::string MavLinkHilRcInputsRaw::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HIL_RC_INPUTS_RAW\", \"id\": 92, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"chan1_raw\":" << this->chan1_raw;
  ss << ", \"chan2_raw\":" << this->chan2_raw;
  ss << ", \"chan3_raw\":" << this->chan3_raw;
  ss << ", \"chan4_raw\":" << this->chan4_raw;
  ss << ", \"chan5_raw\":" << this->chan5_raw;
  ss << ", \"chan6_raw\":" << this->chan6_raw;
  ss << ", \"chan7_raw\":" << this->chan7_raw;
  ss << ", \"chan8_raw\":" << this->chan8_raw;
  ss << ", \"chan9_raw\":" << this->chan9_raw;
  ss << ", \"chan10_raw\":" << this->chan10_raw;
  ss << ", \"chan11_raw\":" << this->chan11_raw;
  ss << ", \"chan12_raw\":" << this->chan12_raw;
  ss << ", \"rssi\":" << static_cast<unsigned int>(this->rssi);
  ss << "} },";
  return ss.str();
}

int MavLinkHilActuatorControls::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->flags), 8);
  pack_float_array(16, buffer,
                   reinterpret_cast<const float*>(&this->controls[0]), 16);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mode), 80);
  return 81;
}

int MavLinkHilActuatorControls::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->flags), 8);
  unpack_float_array(16, buffer, reinterpret_cast<float*>(&this->controls[0]),
                     16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mode), 80);
  return 81;
}

std::string MavLinkHilActuatorControls::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HIL_ACTUATOR_CONTROLS\", \"id\": 93, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"flags\":" << this->flags;
  ss << ", \"controls\":"
     << "["
     << float_array_tostring(16, reinterpret_cast<float*>(&this->controls[0]))
     << "]";
  ss << ", \"mode\":" << static_cast<unsigned int>(this->mode);
  ss << "} },";
  return ss.str();
}

int MavLinkOpticalFlow::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->flow_comp_m_x), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->flow_comp_m_y), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ground_distance),
             16);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->flow_x), 20);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->flow_y), 22);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->sensor_id), 24);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->quality), 25);
  pack_float(buffer, reinterpret_cast<const float*>(&this->flow_rate_x), 26);
  pack_float(buffer, reinterpret_cast<const float*>(&this->flow_rate_y), 30);
  return 34;
}

int MavLinkOpticalFlow::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->flow_comp_m_x), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->flow_comp_m_y), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ground_distance), 16);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->flow_x), 20);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->flow_y), 22);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->sensor_id), 24);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->quality), 25);
  unpack_float(buffer, reinterpret_cast<float*>(&this->flow_rate_x), 26);
  unpack_float(buffer, reinterpret_cast<float*>(&this->flow_rate_y), 30);
  return 34;
}

std::string MavLinkOpticalFlow::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"OPTICAL_FLOW\", \"id\": 100, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"flow_comp_m_x\":" << float_tostring(this->flow_comp_m_x);
  ss << ", \"flow_comp_m_y\":" << float_tostring(this->flow_comp_m_y);
  ss << ", \"ground_distance\":" << float_tostring(this->ground_distance);
  ss << ", \"flow_x\":" << this->flow_x;
  ss << ", \"flow_y\":" << this->flow_y;
  ss << ", \"sensor_id\":" << static_cast<unsigned int>(this->sensor_id);
  ss << ", \"quality\":" << static_cast<unsigned int>(this->quality);
  ss << ", \"flow_rate_x\":" << float_tostring(this->flow_rate_x);
  ss << ", \"flow_rate_y\":" << float_tostring(this->flow_rate_y);
  ss << "} },";
  return ss.str();
}

int MavLinkGlobalVisionPositionEstimate::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 28);
  pack_float_array(21, buffer,
                   reinterpret_cast<const float*>(&this->covariance[0]), 32);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->reset_counter),
               116);
  return 117;
}

int MavLinkGlobalVisionPositionEstimate::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 28);
  unpack_float_array(21, buffer, reinterpret_cast<float*>(&this->covariance[0]),
                     32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->reset_counter), 116);
  return 117;
}

std::string MavLinkGlobalVisionPositionEstimate::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GLOBAL_VISION_POSITION_ESTIMATE\", \"id\": 101, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"usec\":" << this->usec;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"roll\":" << float_tostring(this->roll);
  ss << ", \"pitch\":" << float_tostring(this->pitch);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"covariance\":"
     << "["
     << float_array_tostring(21, reinterpret_cast<float*>(&this->covariance[0]))
     << "]";
  ss << ", \"reset_counter\":"
     << static_cast<unsigned int>(this->reset_counter);
  ss << "} },";
  return ss.str();
}

int MavLinkVisionPositionEstimate::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 28);
  pack_float_array(21, buffer,
                   reinterpret_cast<const float*>(&this->covariance[0]), 32);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->reset_counter),
               116);
  return 117;
}

int MavLinkVisionPositionEstimate::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 28);
  unpack_float_array(21, buffer, reinterpret_cast<float*>(&this->covariance[0]),
                     32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->reset_counter), 116);
  return 117;
}

std::string MavLinkVisionPositionEstimate::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"VISION_POSITION_ESTIMATE\", \"id\": 102, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"usec\":" << this->usec;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"roll\":" << float_tostring(this->roll);
  ss << ", \"pitch\":" << float_tostring(this->pitch);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"covariance\":"
     << "["
     << float_array_tostring(21, reinterpret_cast<float*>(&this->covariance[0]))
     << "]";
  ss << ", \"reset_counter\":"
     << static_cast<unsigned int>(this->reset_counter);
  ss << "} },";
  return ss.str();
}

int MavLinkVisionSpeedEstimate::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 16);
  pack_float_array(9, buffer,
                   reinterpret_cast<const float*>(&this->covariance[0]), 20);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->reset_counter),
               56);
  return 57;
}

int MavLinkVisionSpeedEstimate::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 16);
  unpack_float_array(9, buffer, reinterpret_cast<float*>(&this->covariance[0]),
                     20);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->reset_counter), 56);
  return 57;
}

std::string MavLinkVisionSpeedEstimate::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"VISION_SPEED_ESTIMATE\", \"id\": 103, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"usec\":" << this->usec;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"covariance\":"
     << "["
     << float_array_tostring(9, reinterpret_cast<float*>(&this->covariance[0]))
     << "]";
  ss << ", \"reset_counter\":"
     << static_cast<unsigned int>(this->reset_counter);
  ss << "} },";
  return ss.str();
}

int MavLinkViconPositionEstimate::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 28);
  pack_float_array(21, buffer,
                   reinterpret_cast<const float*>(&this->covariance[0]), 32);
  return 116;
}

int MavLinkViconPositionEstimate::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 28);
  unpack_float_array(21, buffer, reinterpret_cast<float*>(&this->covariance[0]),
                     32);
  return 116;
}

std::string MavLinkViconPositionEstimate::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"VICON_POSITION_ESTIMATE\", \"id\": 104, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"usec\":" << this->usec;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"roll\":" << float_tostring(this->roll);
  ss << ", \"pitch\":" << float_tostring(this->pitch);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"covariance\":"
     << "["
     << float_array_tostring(21, reinterpret_cast<float*>(&this->covariance[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkHighresImu::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->xacc), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yacc), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->zacc), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->xgyro), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ygyro), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->zgyro), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->xmag), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ymag), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->zmag), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->abs_pressure), 44);
  pack_float(buffer, reinterpret_cast<const float*>(&this->diff_pressure), 48);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pressure_alt), 52);
  pack_float(buffer, reinterpret_cast<const float*>(&this->temperature), 56);
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->fields_updated), 60);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->id), 62);
  return 63;
}

int MavLinkHighresImu::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->xacc), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yacc), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->zacc), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->xgyro), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ygyro), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->zgyro), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->xmag), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ymag), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->zmag), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->abs_pressure), 44);
  unpack_float(buffer, reinterpret_cast<float*>(&this->diff_pressure), 48);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pressure_alt), 52);
  unpack_float(buffer, reinterpret_cast<float*>(&this->temperature), 56);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->fields_updated),
                  60);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->id), 62);
  return 63;
}

std::string MavLinkHighresImu::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HIGHRES_IMU\", \"id\": 105, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"xacc\":" << float_tostring(this->xacc);
  ss << ", \"yacc\":" << float_tostring(this->yacc);
  ss << ", \"zacc\":" << float_tostring(this->zacc);
  ss << ", \"xgyro\":" << float_tostring(this->xgyro);
  ss << ", \"ygyro\":" << float_tostring(this->ygyro);
  ss << ", \"zgyro\":" << float_tostring(this->zgyro);
  ss << ", \"xmag\":" << float_tostring(this->xmag);
  ss << ", \"ymag\":" << float_tostring(this->ymag);
  ss << ", \"zmag\":" << float_tostring(this->zmag);
  ss << ", \"abs_pressure\":" << float_tostring(this->abs_pressure);
  ss << ", \"diff_pressure\":" << float_tostring(this->diff_pressure);
  ss << ", \"pressure_alt\":" << float_tostring(this->pressure_alt);
  ss << ", \"temperature\":" << float_tostring(this->temperature);
  ss << ", \"fields_updated\":" << this->fields_updated;
  ss << ", \"id\":" << static_cast<unsigned int>(this->id);
  ss << "} },";
  return ss.str();
}

int MavLinkOpticalFlowRad::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint32_t(
      buffer, reinterpret_cast<const uint32_t*>(&this->integration_time_us), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->integrated_x), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->integrated_y), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->integrated_xgyro),
             20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->integrated_ygyro),
             24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->integrated_zgyro),
             28);
  pack_uint32_t(
      buffer, reinterpret_cast<const uint32_t*>(&this->time_delta_distance_us),
      32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->distance), 36);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature),
               40);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->sensor_id), 42);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->quality), 43);
  return 44;
}

int MavLinkOpticalFlowRad::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint32_t(buffer,
                  reinterpret_cast<uint32_t*>(&this->integration_time_us), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->integrated_x), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->integrated_y), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->integrated_xgyro), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->integrated_ygyro), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->integrated_zgyro), 28);
  unpack_uint32_t(
      buffer, reinterpret_cast<uint32_t*>(&this->time_delta_distance_us), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->distance), 36);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 40);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->sensor_id), 42);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->quality), 43);
  return 44;
}

std::string MavLinkOpticalFlowRad::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"OPTICAL_FLOW_RAD\", \"id\": 106, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"integration_time_us\":" << this->integration_time_us;
  ss << ", \"integrated_x\":" << float_tostring(this->integrated_x);
  ss << ", \"integrated_y\":" << float_tostring(this->integrated_y);
  ss << ", \"integrated_xgyro\":" << float_tostring(this->integrated_xgyro);
  ss << ", \"integrated_ygyro\":" << float_tostring(this->integrated_ygyro);
  ss << ", \"integrated_zgyro\":" << float_tostring(this->integrated_zgyro);
  ss << ", \"time_delta_distance_us\":" << this->time_delta_distance_us;
  ss << ", \"distance\":" << float_tostring(this->distance);
  ss << ", \"temperature\":" << this->temperature;
  ss << ", \"sensor_id\":" << static_cast<unsigned int>(this->sensor_id);
  ss << ", \"quality\":" << static_cast<unsigned int>(this->quality);
  ss << "} },";
  return ss.str();
}

int MavLinkHilSensor::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->xacc), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yacc), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->zacc), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->xgyro), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ygyro), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->zgyro), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->xmag), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ymag), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->zmag), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->abs_pressure), 44);
  pack_float(buffer, reinterpret_cast<const float*>(&this->diff_pressure), 48);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pressure_alt), 52);
  pack_float(buffer, reinterpret_cast<const float*>(&this->temperature), 56);
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(&this->fields_updated), 60);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->id), 64);
  return 65;
}

int MavLinkHilSensor::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->xacc), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yacc), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->zacc), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->xgyro), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ygyro), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->zgyro), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->xmag), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ymag), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->zmag), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->abs_pressure), 44);
  unpack_float(buffer, reinterpret_cast<float*>(&this->diff_pressure), 48);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pressure_alt), 52);
  unpack_float(buffer, reinterpret_cast<float*>(&this->temperature), 56);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->fields_updated),
                  60);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->id), 64);
  return 65;
}

std::string MavLinkHilSensor::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HIL_SENSOR\", \"id\": 107, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"xacc\":" << float_tostring(this->xacc);
  ss << ", \"yacc\":" << float_tostring(this->yacc);
  ss << ", \"zacc\":" << float_tostring(this->zacc);
  ss << ", \"xgyro\":" << float_tostring(this->xgyro);
  ss << ", \"ygyro\":" << float_tostring(this->ygyro);
  ss << ", \"zgyro\":" << float_tostring(this->zgyro);
  ss << ", \"xmag\":" << float_tostring(this->xmag);
  ss << ", \"ymag\":" << float_tostring(this->ymag);
  ss << ", \"zmag\":" << float_tostring(this->zmag);
  ss << ", \"abs_pressure\":" << float_tostring(this->abs_pressure);
  ss << ", \"diff_pressure\":" << float_tostring(this->diff_pressure);
  ss << ", \"pressure_alt\":" << float_tostring(this->pressure_alt);
  ss << ", \"temperature\":" << float_tostring(this->temperature);
  ss << ", \"fields_updated\":" << this->fields_updated;
  ss << ", \"id\":" << static_cast<unsigned int>(this->id);
  ss << "} },";
  return ss.str();
}

int MavLinkSimState::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->q1), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->q2), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->q3), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->q4), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->xacc), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yacc), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->zacc), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->xgyro), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ygyro), 44);
  pack_float(buffer, reinterpret_cast<const float*>(&this->zgyro), 48);
  pack_float(buffer, reinterpret_cast<const float*>(&this->lat), 52);
  pack_float(buffer, reinterpret_cast<const float*>(&this->lon), 56);
  pack_float(buffer, reinterpret_cast<const float*>(&this->alt), 60);
  pack_float(buffer, reinterpret_cast<const float*>(&this->std_dev_horz), 64);
  pack_float(buffer, reinterpret_cast<const float*>(&this->std_dev_vert), 68);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vn), 72);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ve), 76);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vd), 80);
  return 84;
}

int MavLinkSimState::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->q1), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->q2), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->q3), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->q4), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->xacc), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yacc), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->zacc), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->xgyro), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ygyro), 44);
  unpack_float(buffer, reinterpret_cast<float*>(&this->zgyro), 48);
  unpack_float(buffer, reinterpret_cast<float*>(&this->lat), 52);
  unpack_float(buffer, reinterpret_cast<float*>(&this->lon), 56);
  unpack_float(buffer, reinterpret_cast<float*>(&this->alt), 60);
  unpack_float(buffer, reinterpret_cast<float*>(&this->std_dev_horz), 64);
  unpack_float(buffer, reinterpret_cast<float*>(&this->std_dev_vert), 68);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vn), 72);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ve), 76);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vd), 80);
  return 84;
}

std::string MavLinkSimState::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SIM_STATE\", \"id\": 108, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"q1\":" << float_tostring(this->q1);
  ss << ", \"q2\":" << float_tostring(this->q2);
  ss << ", \"q3\":" << float_tostring(this->q3);
  ss << ", \"q4\":" << float_tostring(this->q4);
  ss << ", \"roll\":" << float_tostring(this->roll);
  ss << ", \"pitch\":" << float_tostring(this->pitch);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"xacc\":" << float_tostring(this->xacc);
  ss << ", \"yacc\":" << float_tostring(this->yacc);
  ss << ", \"zacc\":" << float_tostring(this->zacc);
  ss << ", \"xgyro\":" << float_tostring(this->xgyro);
  ss << ", \"ygyro\":" << float_tostring(this->ygyro);
  ss << ", \"zgyro\":" << float_tostring(this->zgyro);
  ss << ", \"lat\":" << float_tostring(this->lat);
  ss << ", \"lon\":" << float_tostring(this->lon);
  ss << ", \"alt\":" << float_tostring(this->alt);
  ss << ", \"std_dev_horz\":" << float_tostring(this->std_dev_horz);
  ss << ", \"std_dev_vert\":" << float_tostring(this->std_dev_vert);
  ss << ", \"vn\":" << float_tostring(this->vn);
  ss << ", \"ve\":" << float_tostring(this->ve);
  ss << ", \"vd\":" << float_tostring(this->vd);
  ss << "} },";
  return ss.str();
}

int MavLinkRadioStatus::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->rxerrors), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->fixed), 2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->rssi), 4);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->remrssi), 5);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->txbuf), 6);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->noise), 7);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->remnoise), 8);
  return 9;
}

int MavLinkRadioStatus::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->rxerrors), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->fixed), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rssi), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->remrssi), 5);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->txbuf), 6);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->noise), 7);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->remnoise), 8);
  return 9;
}

std::string MavLinkRadioStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"RADIO_STATUS\", \"id\": 109, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"rxerrors\":" << this->rxerrors;
  ss << ", \"fixed\":" << this->fixed;
  ss << ", \"rssi\":" << static_cast<unsigned int>(this->rssi);
  ss << ", \"remrssi\":" << static_cast<unsigned int>(this->remrssi);
  ss << ", \"txbuf\":" << static_cast<unsigned int>(this->txbuf);
  ss << ", \"noise\":" << static_cast<unsigned int>(this->noise);
  ss << ", \"remnoise\":" << static_cast<unsigned int>(this->remnoise);
  ss << "} },";
  return ss.str();
}

int MavLinkFileTransferProtocol::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_network),
               0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               1);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 2);
  pack_uint8_t_array(251, buffer,
                     reinterpret_cast<const uint8_t*>(&this->payload[0]), 3);
  return 254;
}

int MavLinkFileTransferProtocol::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_network), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 1);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 2);
  unpack_uint8_t_array(251, buffer,
                       reinterpret_cast<uint8_t*>(&this->payload[0]), 3);
  return 254;
}

std::string MavLinkFileTransferProtocol::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"FILE_TRANSFER_PROTOCOL\", \"id\": 110, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_network\":"
     << static_cast<unsigned int>(this->target_network);
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"payload\":"
     << "["
     << uint8_t_array_tostring(251,
                               reinterpret_cast<uint8_t*>(&this->payload[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkTimesync::pack(char* buffer) const {
  pack_int64_t(buffer, reinterpret_cast<const int64_t*>(&this->tc1), 0);
  pack_int64_t(buffer, reinterpret_cast<const int64_t*>(&this->ts1), 8);
  return 16;
}

int MavLinkTimesync::unpack(const char* buffer) {
  unpack_int64_t(buffer, reinterpret_cast<int64_t*>(&this->tc1), 0);
  unpack_int64_t(buffer, reinterpret_cast<int64_t*>(&this->ts1), 8);
  return 16;
}

std::string MavLinkTimesync::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"TIMESYNC\", \"id\": 111, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"tc1\":" << this->tc1;
  ss << ", \"ts1\":" << this->ts1;
  ss << "} },";
  return ss.str();
}

int MavLinkCameraTrigger::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->seq), 8);
  return 12;
}

int MavLinkCameraTrigger::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->seq), 8);
  return 12;
}

std::string MavLinkCameraTrigger::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CAMERA_TRIGGER\", \"id\": 112, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"seq\":" << this->seq;
  ss << "} },";
  return ss.str();
}

int MavLinkHilGps::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt), 16);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->eph), 20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->epv), 22);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->vel), 24);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vn), 26);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->ve), 28);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vd), 30);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->cog), 32);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->fix_type), 34);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->satellites_visible), 35);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->id), 36);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->yaw), 37);
  return 39;
}

int MavLinkHilGps::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt), 16);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->eph), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->epv), 22);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->vel), 24);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vn), 26);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->ve), 28);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vd), 30);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->cog), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->fix_type), 34);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->satellites_visible),
                 35);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->id), 36);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->yaw), 37);
  return 39;
}

std::string MavLinkHilGps::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HIL_GPS\", \"id\": 113, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"alt\":" << this->alt;
  ss << ", \"eph\":" << this->eph;
  ss << ", \"epv\":" << this->epv;
  ss << ", \"vel\":" << this->vel;
  ss << ", \"vn\":" << this->vn;
  ss << ", \"ve\":" << this->ve;
  ss << ", \"vd\":" << this->vd;
  ss << ", \"cog\":" << this->cog;
  ss << ", \"fix_type\":" << static_cast<unsigned int>(this->fix_type);
  ss << ", \"satellites_visible\":"
     << static_cast<unsigned int>(this->satellites_visible);
  ss << ", \"id\":" << static_cast<unsigned int>(this->id);
  ss << ", \"yaw\":" << this->yaw;
  ss << "} },";
  return ss.str();
}

int MavLinkHilOpticalFlow::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint32_t(
      buffer, reinterpret_cast<const uint32_t*>(&this->integration_time_us), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->integrated_x), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->integrated_y), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->integrated_xgyro),
             20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->integrated_ygyro),
             24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->integrated_zgyro),
             28);
  pack_uint32_t(
      buffer, reinterpret_cast<const uint32_t*>(&this->time_delta_distance_us),
      32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->distance), 36);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature),
               40);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->sensor_id), 42);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->quality), 43);
  return 44;
}

int MavLinkHilOpticalFlow::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint32_t(buffer,
                  reinterpret_cast<uint32_t*>(&this->integration_time_us), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->integrated_x), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->integrated_y), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->integrated_xgyro), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->integrated_ygyro), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->integrated_zgyro), 28);
  unpack_uint32_t(
      buffer, reinterpret_cast<uint32_t*>(&this->time_delta_distance_us), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->distance), 36);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 40);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->sensor_id), 42);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->quality), 43);
  return 44;
}

std::string MavLinkHilOpticalFlow::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HIL_OPTICAL_FLOW\", \"id\": 114, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"integration_time_us\":" << this->integration_time_us;
  ss << ", \"integrated_x\":" << float_tostring(this->integrated_x);
  ss << ", \"integrated_y\":" << float_tostring(this->integrated_y);
  ss << ", \"integrated_xgyro\":" << float_tostring(this->integrated_xgyro);
  ss << ", \"integrated_ygyro\":" << float_tostring(this->integrated_ygyro);
  ss << ", \"integrated_zgyro\":" << float_tostring(this->integrated_zgyro);
  ss << ", \"time_delta_distance_us\":" << this->time_delta_distance_us;
  ss << ", \"distance\":" << float_tostring(this->distance);
  ss << ", \"temperature\":" << this->temperature;
  ss << ", \"sensor_id\":" << static_cast<unsigned int>(this->sensor_id);
  ss << ", \"quality\":" << static_cast<unsigned int>(this->quality);
  ss << "} },";
  return ss.str();
}

int MavLinkHilStateQuaternion::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float_array(
      4, buffer, reinterpret_cast<const float*>(&this->attitude_quaternion[0]),
      8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->rollspeed), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitchspeed), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yawspeed), 32);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 36);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 40);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt), 44);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vx), 48);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vy), 50);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vz), 52);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->ind_airspeed),
                54);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->true_airspeed),
                56);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xacc), 58);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->yacc), 60);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zacc), 62);
  return 64;
}

int MavLinkHilStateQuaternion::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float_array(
      4, buffer, reinterpret_cast<float*>(&this->attitude_quaternion[0]), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->rollspeed), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitchspeed), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yawspeed), 32);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 36);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 40);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt), 44);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vx), 48);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vy), 50);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vz), 52);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->ind_airspeed), 54);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->true_airspeed),
                  56);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xacc), 58);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->yacc), 60);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zacc), 62);
  return 64;
}

std::string MavLinkHilStateQuaternion::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HIL_STATE_QUATERNION\", \"id\": 115, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"attitude_quaternion\":"
     << "["
     << float_array_tostring(
            4, reinterpret_cast<float*>(&this->attitude_quaternion[0]))
     << "]";
  ss << ", \"rollspeed\":" << float_tostring(this->rollspeed);
  ss << ", \"pitchspeed\":" << float_tostring(this->pitchspeed);
  ss << ", \"yawspeed\":" << float_tostring(this->yawspeed);
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"alt\":" << this->alt;
  ss << ", \"vx\":" << this->vx;
  ss << ", \"vy\":" << this->vy;
  ss << ", \"vz\":" << this->vz;
  ss << ", \"ind_airspeed\":" << this->ind_airspeed;
  ss << ", \"true_airspeed\":" << this->true_airspeed;
  ss << ", \"xacc\":" << this->xacc;
  ss << ", \"yacc\":" << this->yacc;
  ss << ", \"zacc\":" << this->zacc;
  ss << "} },";
  return ss.str();
}

int MavLinkScaledImu2::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xacc), 4);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->yacc), 6);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zacc), 8);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xgyro), 10);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->ygyro), 12);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zgyro), 14);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xmag), 16);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->ymag), 18);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zmag), 20);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature),
               22);
  return 24;
}

int MavLinkScaledImu2::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xacc), 4);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->yacc), 6);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zacc), 8);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xgyro), 10);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->ygyro), 12);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zgyro), 14);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xmag), 16);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->ymag), 18);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zmag), 20);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 22);
  return 24;
}

std::string MavLinkScaledImu2::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SCALED_IMU2\", \"id\": 116, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"xacc\":" << this->xacc;
  ss << ", \"yacc\":" << this->yacc;
  ss << ", \"zacc\":" << this->zacc;
  ss << ", \"xgyro\":" << this->xgyro;
  ss << ", \"ygyro\":" << this->ygyro;
  ss << ", \"zgyro\":" << this->zgyro;
  ss << ", \"xmag\":" << this->xmag;
  ss << ", \"ymag\":" << this->ymag;
  ss << ", \"zmag\":" << this->zmag;
  ss << ", \"temperature\":" << this->temperature;
  ss << "} },";
  return ss.str();
}

int MavLinkLogRequestList::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->start), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->end), 2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               4);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 5);
  return 6;
}

int MavLinkLogRequestList::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->start), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->end), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 5);
  return 6;
}

std::string MavLinkLogRequestList::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LOG_REQUEST_LIST\", \"id\": 117, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"start\":" << this->start;
  ss << ", \"end\":" << this->end;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkLogEntry::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_utc), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->size), 4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->id), 8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->num_logs), 10);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->last_log_num),
                12);
  return 14;
}

int MavLinkLogEntry::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_utc), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->size), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->id), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->num_logs), 10);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->last_log_num), 12);
  return 14;
}

std::string MavLinkLogEntry::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LOG_ENTRY\", \"id\": 118, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_utc\":" << this->time_utc;
  ss << ", \"size\":" << this->size;
  ss << ", \"id\":" << this->id;
  ss << ", \"num_logs\":" << this->num_logs;
  ss << ", \"last_log_num\":" << this->last_log_num;
  ss << "} },";
  return ss.str();
}

int MavLinkLogRequestData::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->ofs), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->count), 4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->id), 8);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               10);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 11);
  return 12;
}

int MavLinkLogRequestData::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->ofs), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->count), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->id), 8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 10);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 11);
  return 12;
}

std::string MavLinkLogRequestData::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LOG_REQUEST_DATA\", \"id\": 119, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"ofs\":" << this->ofs;
  ss << ", \"count\":" << this->count;
  ss << ", \"id\":" << this->id;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkLogData::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->ofs), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->id), 4);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->count), 6);
  pack_uint8_t_array(90, buffer,
                     reinterpret_cast<const uint8_t*>(&this->data[0]), 7);
  return 97;
}

int MavLinkLogData::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->ofs), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->id), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->count), 6);
  unpack_uint8_t_array(90, buffer, reinterpret_cast<uint8_t*>(&this->data[0]),
                       7);
  return 97;
}

std::string MavLinkLogData::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LOG_DATA\", \"id\": 120, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"ofs\":" << this->ofs;
  ss << ", \"id\":" << this->id;
  ss << ", \"count\":" << static_cast<unsigned int>(this->count);
  ss << ", \"data\":"
     << "["
     << uint8_t_array_tostring(90, reinterpret_cast<uint8_t*>(&this->data[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkLogErase::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  return 2;
}

int MavLinkLogErase::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  return 2;
}

std::string MavLinkLogErase::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LOG_ERASE\", \"id\": 121, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkLogRequestEnd::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  return 2;
}

int MavLinkLogRequestEnd::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  return 2;
}

std::string MavLinkLogRequestEnd::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LOG_REQUEST_END\", \"id\": 122, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkGpsInjectData::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->len), 2);
  pack_uint8_t_array(110, buffer,
                     reinterpret_cast<const uint8_t*>(&this->data[0]), 3);
  return 113;
}

int MavLinkGpsInjectData::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->len), 2);
  unpack_uint8_t_array(110, buffer, reinterpret_cast<uint8_t*>(&this->data[0]),
                       3);
  return 113;
}

std::string MavLinkGpsInjectData::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GPS_INJECT_DATA\", \"id\": 123, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"len\":" << static_cast<unsigned int>(this->len);
  ss << ", \"data\":"
     << "["
     << uint8_t_array_tostring(110, reinterpret_cast<uint8_t*>(&this->data[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkGps2Raw::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt), 16);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->dgps_age), 20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->eph), 24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->epv), 26);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->vel), 28);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->cog), 30);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->fix_type), 32);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->satellites_visible), 33);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->dgps_numch), 34);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->yaw), 35);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt_ellipsoid),
               37);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->h_acc), 41);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->v_acc), 45);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->vel_acc), 49);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->hdg_acc), 53);
  return 57;
}

int MavLinkGps2Raw::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt), 16);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->dgps_age), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->eph), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->epv), 26);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->vel), 28);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->cog), 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->fix_type), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->satellites_visible),
                 33);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->dgps_numch), 34);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->yaw), 35);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt_ellipsoid), 37);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->h_acc), 41);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->v_acc), 45);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->vel_acc), 49);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->hdg_acc), 53);
  return 57;
}

std::string MavLinkGps2Raw::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GPS2_RAW\", \"id\": 124, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"alt\":" << this->alt;
  ss << ", \"dgps_age\":" << this->dgps_age;
  ss << ", \"eph\":" << this->eph;
  ss << ", \"epv\":" << this->epv;
  ss << ", \"vel\":" << this->vel;
  ss << ", \"cog\":" << this->cog;
  ss << ", \"fix_type\":" << static_cast<unsigned int>(this->fix_type);
  ss << ", \"satellites_visible\":"
     << static_cast<unsigned int>(this->satellites_visible);
  ss << ", \"dgps_numch\":" << static_cast<unsigned int>(this->dgps_numch);
  ss << ", \"yaw\":" << this->yaw;
  ss << ", \"alt_ellipsoid\":" << this->alt_ellipsoid;
  ss << ", \"h_acc\":" << this->h_acc;
  ss << ", \"v_acc\":" << this->v_acc;
  ss << ", \"vel_acc\":" << this->vel_acc;
  ss << ", \"hdg_acc\":" << this->hdg_acc;
  ss << "} },";
  return ss.str();
}

int MavLinkPowerStatus::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->Vcc), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->Vservo), 2);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->flags), 4);
  return 6;
}

int MavLinkPowerStatus::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->Vcc), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->Vservo), 2);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->flags), 4);
  return 6;
}

std::string MavLinkPowerStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"POWER_STATUS\", \"id\": 125, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"Vcc\":" << this->Vcc;
  ss << ", \"Vservo\":" << this->Vservo;
  ss << ", \"flags\":" << this->flags;
  ss << "} },";
  return ss.str();
}

int MavLinkSerialControl::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->baudrate), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->timeout), 4);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->device), 6);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->flags), 7);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->count), 8);
  pack_uint8_t_array(70, buffer,
                     reinterpret_cast<const uint8_t*>(&this->data[0]), 9);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               79);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 80);
  return 81;
}

int MavLinkSerialControl::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->baudrate), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->timeout), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->device), 6);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->flags), 7);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->count), 8);
  unpack_uint8_t_array(70, buffer, reinterpret_cast<uint8_t*>(&this->data[0]),
                       9);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 79);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 80);
  return 81;
}

std::string MavLinkSerialControl::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SERIAL_CONTROL\", \"id\": 126, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"baudrate\":" << this->baudrate;
  ss << ", \"timeout\":" << this->timeout;
  ss << ", \"device\":" << static_cast<unsigned int>(this->device);
  ss << ", \"flags\":" << static_cast<unsigned int>(this->flags);
  ss << ", \"count\":" << static_cast<unsigned int>(this->count);
  ss << ", \"data\":"
     << "["
     << uint8_t_array_tostring(70, reinterpret_cast<uint8_t*>(&this->data[0]))
     << "]";
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkGpsRtk::pack(char* buffer) const {
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(&this->time_last_baseline_ms),
                0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->tow), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->baseline_a_mm),
               8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->baseline_b_mm),
               12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->baseline_c_mm),
               16);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->accuracy), 20);
  pack_int32_t(buffer,
               reinterpret_cast<const int32_t*>(&this->iar_num_hypotheses), 24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->wn), 28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->rtk_receiver_id),
               30);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->rtk_health), 31);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->rtk_rate), 32);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->nsats), 33);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->baseline_coords_type),
               34);
  return 35;
}

int MavLinkGpsRtk::unpack(const char* buffer) {
  unpack_uint32_t(buffer,
                  reinterpret_cast<uint32_t*>(&this->time_last_baseline_ms), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->tow), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->baseline_a_mm), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->baseline_b_mm), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->baseline_c_mm), 16);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->accuracy), 20);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->iar_num_hypotheses),
                 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->wn), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rtk_receiver_id),
                 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rtk_health), 31);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rtk_rate), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->nsats), 33);
  unpack_uint8_t(buffer,
                 reinterpret_cast<uint8_t*>(&this->baseline_coords_type), 34);
  return 35;
}

std::string MavLinkGpsRtk::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GPS_RTK\", \"id\": 127, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_last_baseline_ms\":" << this->time_last_baseline_ms;
  ss << ", \"tow\":" << this->tow;
  ss << ", \"baseline_a_mm\":" << this->baseline_a_mm;
  ss << ", \"baseline_b_mm\":" << this->baseline_b_mm;
  ss << ", \"baseline_c_mm\":" << this->baseline_c_mm;
  ss << ", \"accuracy\":" << this->accuracy;
  ss << ", \"iar_num_hypotheses\":" << this->iar_num_hypotheses;
  ss << ", \"wn\":" << this->wn;
  ss << ", \"rtk_receiver_id\":"
     << static_cast<unsigned int>(this->rtk_receiver_id);
  ss << ", \"rtk_health\":" << static_cast<unsigned int>(this->rtk_health);
  ss << ", \"rtk_rate\":" << static_cast<unsigned int>(this->rtk_rate);
  ss << ", \"nsats\":" << static_cast<unsigned int>(this->nsats);
  ss << ", \"baseline_coords_type\":"
     << static_cast<unsigned int>(this->baseline_coords_type);
  ss << "} },";
  return ss.str();
}

int MavLinkGps2Rtk::pack(char* buffer) const {
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(&this->time_last_baseline_ms),
                0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->tow), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->baseline_a_mm),
               8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->baseline_b_mm),
               12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->baseline_c_mm),
               16);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->accuracy), 20);
  pack_int32_t(buffer,
               reinterpret_cast<const int32_t*>(&this->iar_num_hypotheses), 24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->wn), 28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->rtk_receiver_id),
               30);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->rtk_health), 31);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->rtk_rate), 32);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->nsats), 33);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->baseline_coords_type),
               34);
  return 35;
}

int MavLinkGps2Rtk::unpack(const char* buffer) {
  unpack_uint32_t(buffer,
                  reinterpret_cast<uint32_t*>(&this->time_last_baseline_ms), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->tow), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->baseline_a_mm), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->baseline_b_mm), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->baseline_c_mm), 16);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->accuracy), 20);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->iar_num_hypotheses),
                 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->wn), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rtk_receiver_id),
                 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rtk_health), 31);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rtk_rate), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->nsats), 33);
  unpack_uint8_t(buffer,
                 reinterpret_cast<uint8_t*>(&this->baseline_coords_type), 34);
  return 35;
}

std::string MavLinkGps2Rtk::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GPS2_RTK\", \"id\": 128, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_last_baseline_ms\":" << this->time_last_baseline_ms;
  ss << ", \"tow\":" << this->tow;
  ss << ", \"baseline_a_mm\":" << this->baseline_a_mm;
  ss << ", \"baseline_b_mm\":" << this->baseline_b_mm;
  ss << ", \"baseline_c_mm\":" << this->baseline_c_mm;
  ss << ", \"accuracy\":" << this->accuracy;
  ss << ", \"iar_num_hypotheses\":" << this->iar_num_hypotheses;
  ss << ", \"wn\":" << this->wn;
  ss << ", \"rtk_receiver_id\":"
     << static_cast<unsigned int>(this->rtk_receiver_id);
  ss << ", \"rtk_health\":" << static_cast<unsigned int>(this->rtk_health);
  ss << ", \"rtk_rate\":" << static_cast<unsigned int>(this->rtk_rate);
  ss << ", \"nsats\":" << static_cast<unsigned int>(this->nsats);
  ss << ", \"baseline_coords_type\":"
     << static_cast<unsigned int>(this->baseline_coords_type);
  ss << "} },";
  return ss.str();
}

int MavLinkScaledImu3::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xacc), 4);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->yacc), 6);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zacc), 8);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xgyro), 10);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->ygyro), 12);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zgyro), 14);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->xmag), 16);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->ymag), 18);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->zmag), 20);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature),
               22);
  return 24;
}

int MavLinkScaledImu3::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xacc), 4);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->yacc), 6);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zacc), 8);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xgyro), 10);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->ygyro), 12);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zgyro), 14);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->xmag), 16);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->ymag), 18);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->zmag), 20);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 22);
  return 24;
}

std::string MavLinkScaledImu3::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SCALED_IMU3\", \"id\": 129, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"xacc\":" << this->xacc;
  ss << ", \"yacc\":" << this->yacc;
  ss << ", \"zacc\":" << this->zacc;
  ss << ", \"xgyro\":" << this->xgyro;
  ss << ", \"ygyro\":" << this->ygyro;
  ss << ", \"zgyro\":" << this->zgyro;
  ss << ", \"xmag\":" << this->xmag;
  ss << ", \"ymag\":" << this->ymag;
  ss << ", \"zmag\":" << this->zmag;
  ss << ", \"temperature\":" << this->temperature;
  ss << "} },";
  return ss.str();
}

int MavLinkDataTransmissionHandshake::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->size), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->width), 4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->height), 6);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->packets), 8);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 10);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->payload), 11);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->jpg_quality),
               12);
  return 13;
}

int MavLinkDataTransmissionHandshake::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->size), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->width), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->height), 6);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->packets), 8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 10);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->payload), 11);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->jpg_quality), 12);
  return 13;
}

std::string MavLinkDataTransmissionHandshake::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"DATA_TRANSMISSION_HANDSHAKE\", \"id\": 130, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"size\":" << this->size;
  ss << ", \"width\":" << this->width;
  ss << ", \"height\":" << this->height;
  ss << ", \"packets\":" << this->packets;
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"payload\":" << static_cast<unsigned int>(this->payload);
  ss << ", \"jpg_quality\":" << static_cast<unsigned int>(this->jpg_quality);
  ss << "} },";
  return ss.str();
}

int MavLinkEncapsulatedData::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->seqnr), 0);
  pack_uint8_t_array(253, buffer,
                     reinterpret_cast<const uint8_t*>(&this->data[0]), 2);
  return 255;
}

int MavLinkEncapsulatedData::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->seqnr), 0);
  unpack_uint8_t_array(253, buffer, reinterpret_cast<uint8_t*>(&this->data[0]),
                       2);
  return 255;
}

std::string MavLinkEncapsulatedData::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ENCAPSULATED_DATA\", \"id\": 131, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"seqnr\":" << this->seqnr;
  ss << ", \"data\":"
     << "["
     << uint8_t_array_tostring(253, reinterpret_cast<uint8_t*>(&this->data[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkDistanceSensor::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->min_distance),
                4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->max_distance),
                6);
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->current_distance), 8);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 10);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->id), 11);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->orientation),
               12);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->covariance), 13);
  pack_float(buffer, reinterpret_cast<const float*>(&this->horizontal_fov), 14);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vertical_fov), 18);
  pack_float_array(4, buffer,
                   reinterpret_cast<const float*>(&this->quaternion[0]), 22);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->signal_quality),
               38);
  return 39;
}

int MavLinkDistanceSensor::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->min_distance), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->max_distance), 6);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->current_distance),
                  8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 10);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->id), 11);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->orientation), 12);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->covariance), 13);
  unpack_float(buffer, reinterpret_cast<float*>(&this->horizontal_fov), 14);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vertical_fov), 18);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->quaternion[0]),
                     22);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->signal_quality), 38);
  return 39;
}

std::string MavLinkDistanceSensor::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"DISTANCE_SENSOR\", \"id\": 132, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"min_distance\":" << this->min_distance;
  ss << ", \"max_distance\":" << this->max_distance;
  ss << ", \"current_distance\":" << this->current_distance;
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"id\":" << static_cast<unsigned int>(this->id);
  ss << ", \"orientation\":" << static_cast<unsigned int>(this->orientation);
  ss << ", \"covariance\":" << static_cast<unsigned int>(this->covariance);
  ss << ", \"horizontal_fov\":" << float_tostring(this->horizontal_fov);
  ss << ", \"vertical_fov\":" << float_tostring(this->vertical_fov);
  ss << ", \"quaternion\":"
     << "["
     << float_array_tostring(4, reinterpret_cast<float*>(&this->quaternion[0]))
     << "]";
  ss << ", \"signal_quality\":"
     << static_cast<unsigned int>(this->signal_quality);
  ss << "} },";
  return ss.str();
}

int MavLinkTerrainRequest::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->mask), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 12);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->grid_spacing),
                16);
  return 18;
}

int MavLinkTerrainRequest::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->mask), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 12);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->grid_spacing), 16);
  return 18;
}

std::string MavLinkTerrainRequest::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"TERRAIN_REQUEST\", \"id\": 133, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"mask\":" << this->mask;
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"grid_spacing\":" << this->grid_spacing;
  ss << "} },";
  return ss.str();
}

int MavLinkTerrainData::pack(char* buffer) const {
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->grid_spacing),
                8);
  pack_int16_t_array(16, buffer,
                     reinterpret_cast<const int16_t*>(&this->data[0]), 10);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->gridbit), 42);
  return 43;
}

int MavLinkTerrainData::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->grid_spacing), 8);
  unpack_int16_t_array(16, buffer, reinterpret_cast<int16_t*>(&this->data[0]),
                       10);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->gridbit), 42);
  return 43;
}

std::string MavLinkTerrainData::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"TERRAIN_DATA\", \"id\": 134, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"grid_spacing\":" << this->grid_spacing;
  ss << ", \"data\":"
     << "["
     << int16_t_array_tostring(16, reinterpret_cast<int16_t*>(&this->data[0]))
     << "]";
  ss << ", \"gridbit\":" << static_cast<unsigned int>(this->gridbit);
  ss << "} },";
  return ss.str();
}

int MavLinkTerrainCheck::pack(char* buffer) const {
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 4);
  return 8;
}

int MavLinkTerrainCheck::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 4);
  return 8;
}

std::string MavLinkTerrainCheck::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"TERRAIN_CHECK\", \"id\": 135, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << "} },";
  return ss.str();
}

int MavLinkTerrainReport::pack(char* buffer) const {
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->terrain_height), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->current_height), 12);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->spacing), 16);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->pending), 18);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->loaded), 20);
  return 22;
}

int MavLinkTerrainReport::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->terrain_height), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->current_height), 12);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->spacing), 16);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->pending), 18);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->loaded), 20);
  return 22;
}

std::string MavLinkTerrainReport::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"TERRAIN_REPORT\", \"id\": 136, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"terrain_height\":" << float_tostring(this->terrain_height);
  ss << ", \"current_height\":" << float_tostring(this->current_height);
  ss << ", \"spacing\":" << this->spacing;
  ss << ", \"pending\":" << this->pending;
  ss << ", \"loaded\":" << this->loaded;
  ss << "} },";
  return ss.str();
}

int MavLinkScaledPressure2::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->press_abs), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->press_diff), 8);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature),
               12);
  pack_int16_t(buffer,
               reinterpret_cast<const int16_t*>(&this->temperature_press_diff),
               14);
  return 16;
}

int MavLinkScaledPressure2::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->press_abs), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->press_diff), 8);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 12);
  unpack_int16_t(buffer,
                 reinterpret_cast<int16_t*>(&this->temperature_press_diff), 14);
  return 16;
}

std::string MavLinkScaledPressure2::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SCALED_PRESSURE2\", \"id\": 137, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"press_abs\":" << float_tostring(this->press_abs);
  ss << ", \"press_diff\":" << float_tostring(this->press_diff);
  ss << ", \"temperature\":" << this->temperature;
  ss << ", \"temperature_press_diff\":" << this->temperature_press_diff;
  ss << "} },";
  return ss.str();
}

int MavLinkAttPosMocap::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 32);
  pack_float_array(21, buffer,
                   reinterpret_cast<const float*>(&this->covariance[0]), 36);
  return 120;
}

int MavLinkAttPosMocap::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 32);
  unpack_float_array(21, buffer, reinterpret_cast<float*>(&this->covariance[0]),
                     36);
  return 120;
}

std::string MavLinkAttPosMocap::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ATT_POS_MOCAP\", \"id\": 138, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"covariance\":"
     << "["
     << float_array_tostring(21, reinterpret_cast<float*>(&this->covariance[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkSetActuatorControlTarget::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float_array(8, buffer,
                   reinterpret_cast<const float*>(&this->controls[0]), 8);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->group_mlx), 40);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               41);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 42);
  return 43;
}

int MavLinkSetActuatorControlTarget::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float_array(8, buffer, reinterpret_cast<float*>(&this->controls[0]),
                     8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->group_mlx), 40);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 41);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 42);
  return 43;
}

std::string MavLinkSetActuatorControlTarget::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SET_ACTUATOR_CONTROL_TARGET\", \"id\": 139, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"controls\":"
     << "["
     << float_array_tostring(8, reinterpret_cast<float*>(&this->controls[0]))
     << "]";
  ss << ", \"group_mlx\":" << static_cast<unsigned int>(this->group_mlx);
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkActuatorControlTarget::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float_array(8, buffer,
                   reinterpret_cast<const float*>(&this->controls[0]), 8);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->group_mlx), 40);
  return 41;
}

int MavLinkActuatorControlTarget::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float_array(8, buffer, reinterpret_cast<float*>(&this->controls[0]),
                     8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->group_mlx), 40);
  return 41;
}

std::string MavLinkActuatorControlTarget::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ACTUATOR_CONTROL_TARGET\", \"id\": 140, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"controls\":"
     << "["
     << float_array_tostring(8, reinterpret_cast<float*>(&this->controls[0]))
     << "]";
  ss << ", \"group_mlx\":" << static_cast<unsigned int>(this->group_mlx);
  ss << "} },";
  return ss.str();
}

int MavLinkAltitude::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->altitude_monotonic),
             8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->altitude_amsl), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->altitude_local), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->altitude_relative),
             20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->altitude_terrain),
             24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->bottom_clearance),
             28);
  return 32;
}

int MavLinkAltitude::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->altitude_monotonic), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->altitude_amsl), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->altitude_local), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->altitude_relative), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->altitude_terrain), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->bottom_clearance), 28);
  return 32;
}

std::string MavLinkAltitude::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ALTITUDE\", \"id\": 141, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"altitude_monotonic\":" << float_tostring(this->altitude_monotonic);
  ss << ", \"altitude_amsl\":" << float_tostring(this->altitude_amsl);
  ss << ", \"altitude_local\":" << float_tostring(this->altitude_local);
  ss << ", \"altitude_relative\":" << float_tostring(this->altitude_relative);
  ss << ", \"altitude_terrain\":" << float_tostring(this->altitude_terrain);
  ss << ", \"bottom_clearance\":" << float_tostring(this->bottom_clearance);
  ss << "} },";
  return ss.str();
}

int MavLinkResourceRequest::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->request_id), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->uri_type), 1);
  pack_uint8_t_array(120, buffer,
                     reinterpret_cast<const uint8_t*>(&this->uri[0]), 2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->transfer_type),
               122);
  pack_uint8_t_array(120, buffer,
                     reinterpret_cast<const uint8_t*>(&this->storage[0]), 123);
  return 243;
}

int MavLinkResourceRequest::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->request_id), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->uri_type), 1);
  unpack_uint8_t_array(120, buffer, reinterpret_cast<uint8_t*>(&this->uri[0]),
                       2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->transfer_type), 122);
  unpack_uint8_t_array(120, buffer,
                       reinterpret_cast<uint8_t*>(&this->storage[0]), 123);
  return 243;
}

std::string MavLinkResourceRequest::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"RESOURCE_REQUEST\", \"id\": 142, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"request_id\":" << static_cast<unsigned int>(this->request_id);
  ss << ", \"uri_type\":" << static_cast<unsigned int>(this->uri_type);
  ss << ", \"uri\":"
     << "["
     << uint8_t_array_tostring(120, reinterpret_cast<uint8_t*>(&this->uri[0]))
     << "]";
  ss << ", \"transfer_type\":"
     << static_cast<unsigned int>(this->transfer_type);
  ss << ", \"storage\":"
     << "["
     << uint8_t_array_tostring(120,
                               reinterpret_cast<uint8_t*>(&this->storage[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkScaledPressure3::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->press_abs), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->press_diff), 8);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature),
               12);
  pack_int16_t(buffer,
               reinterpret_cast<const int16_t*>(&this->temperature_press_diff),
               14);
  return 16;
}

int MavLinkScaledPressure3::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->press_abs), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->press_diff), 8);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 12);
  unpack_int16_t(buffer,
                 reinterpret_cast<int16_t*>(&this->temperature_press_diff), 14);
  return 16;
}

std::string MavLinkScaledPressure3::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SCALED_PRESSURE3\", \"id\": 143, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"press_abs\":" << float_tostring(this->press_abs);
  ss << ", \"press_diff\":" << float_tostring(this->press_diff);
  ss << ", \"temperature\":" << this->temperature;
  ss << ", \"temperature_press_diff\":" << this->temperature_press_diff;
  ss << "} },";
  return ss.str();
}

int MavLinkFollowTarget::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->timestamp), 0);
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->custom_state),
                8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 16);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->alt), 24);
  pack_float_array(3, buffer, reinterpret_cast<const float*>(&this->vel[0]),
                   28);
  pack_float_array(3, buffer, reinterpret_cast<const float*>(&this->acc[0]),
                   40);
  pack_float_array(4, buffer,
                   reinterpret_cast<const float*>(&this->attitude_q[0]), 52);
  pack_float_array(3, buffer, reinterpret_cast<const float*>(&this->rates[0]),
                   68);
  pack_float_array(3, buffer,
                   reinterpret_cast<const float*>(&this->position_cov[0]), 80);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->est_capabilities), 92);
  return 93;
}

int MavLinkFollowTarget::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->timestamp), 0);
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->custom_state), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 16);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->alt), 24);
  unpack_float_array(3, buffer, reinterpret_cast<float*>(&this->vel[0]), 28);
  unpack_float_array(3, buffer, reinterpret_cast<float*>(&this->acc[0]), 40);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->attitude_q[0]),
                     52);
  unpack_float_array(3, buffer, reinterpret_cast<float*>(&this->rates[0]), 68);
  unpack_float_array(3, buffer,
                     reinterpret_cast<float*>(&this->position_cov[0]), 80);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->est_capabilities),
                 92);
  return 93;
}

std::string MavLinkFollowTarget::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"FOLLOW_TARGET\", \"id\": 144, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"timestamp\":" << this->timestamp;
  ss << ", \"custom_state\":" << this->custom_state;
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"alt\":" << float_tostring(this->alt);
  ss << ", \"vel\":"
     << "[" << float_array_tostring(3, reinterpret_cast<float*>(&this->vel[0]))
     << "]";
  ss << ", \"acc\":"
     << "[" << float_array_tostring(3, reinterpret_cast<float*>(&this->acc[0]))
     << "]";
  ss << ", \"attitude_q\":"
     << "["
     << float_array_tostring(4, reinterpret_cast<float*>(&this->attitude_q[0]))
     << "]";
  ss << ", \"rates\":"
     << "["
     << float_array_tostring(3, reinterpret_cast<float*>(&this->rates[0]))
     << "]";
  ss << ", \"position_cov\":"
     << "["
     << float_array_tostring(3,
                             reinterpret_cast<float*>(&this->position_cov[0]))
     << "]";
  ss << ", \"est_capabilities\":"
     << static_cast<unsigned int>(this->est_capabilities);
  ss << "} },";
  return ss.str();
}

int MavLinkControlSystemState::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x_acc), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y_acc), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z_acc), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x_vel), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y_vel), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z_vel), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x_pos), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y_pos), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z_pos), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->airspeed), 44);
  pack_float_array(3, buffer,
                   reinterpret_cast<const float*>(&this->vel_variance[0]), 48);
  pack_float_array(3, buffer,
                   reinterpret_cast<const float*>(&this->pos_variance[0]), 60);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 72);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll_rate), 88);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch_rate), 92);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_rate), 96);
  return 100;
}

int MavLinkControlSystemState::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x_acc), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y_acc), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z_acc), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x_vel), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y_vel), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z_vel), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x_pos), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y_pos), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z_pos), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->airspeed), 44);
  unpack_float_array(3, buffer,
                     reinterpret_cast<float*>(&this->vel_variance[0]), 48);
  unpack_float_array(3, buffer,
                     reinterpret_cast<float*>(&this->pos_variance[0]), 60);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 72);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll_rate), 88);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch_rate), 92);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_rate), 96);
  return 100;
}

std::string MavLinkControlSystemState::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CONTROL_SYSTEM_STATE\", \"id\": 146, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"x_acc\":" << float_tostring(this->x_acc);
  ss << ", \"y_acc\":" << float_tostring(this->y_acc);
  ss << ", \"z_acc\":" << float_tostring(this->z_acc);
  ss << ", \"x_vel\":" << float_tostring(this->x_vel);
  ss << ", \"y_vel\":" << float_tostring(this->y_vel);
  ss << ", \"z_vel\":" << float_tostring(this->z_vel);
  ss << ", \"x_pos\":" << float_tostring(this->x_pos);
  ss << ", \"y_pos\":" << float_tostring(this->y_pos);
  ss << ", \"z_pos\":" << float_tostring(this->z_pos);
  ss << ", \"airspeed\":" << float_tostring(this->airspeed);
  ss << ", \"vel_variance\":"
     << "["
     << float_array_tostring(3,
                             reinterpret_cast<float*>(&this->vel_variance[0]))
     << "]";
  ss << ", \"pos_variance\":"
     << "["
     << float_array_tostring(3,
                             reinterpret_cast<float*>(&this->pos_variance[0]))
     << "]";
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"roll_rate\":" << float_tostring(this->roll_rate);
  ss << ", \"pitch_rate\":" << float_tostring(this->pitch_rate);
  ss << ", \"yaw_rate\":" << float_tostring(this->yaw_rate);
  ss << "} },";
  return ss.str();
}

int MavLinkBatteryStatus::pack(char* buffer) const {
  pack_int32_t(buffer,
               reinterpret_cast<const int32_t*>(&this->current_consumed), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->energy_consumed),
               4);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature), 8);
  pack_uint16_t_array(
      10, buffer, reinterpret_cast<const uint16_t*>(&this->voltages[0]), 10);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->current_battery),
               30);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->id), 32);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->battery_function), 33);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 34);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->battery_remaining),
              35);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->time_remaining),
               36);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->charge_state),
               40);
  pack_uint16_t_array(
      4, buffer, reinterpret_cast<const uint16_t*>(&this->voltages_ext[0]), 41);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mode), 49);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->fault_bitmask),
                50);
  return 54;
}

int MavLinkBatteryStatus::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->current_consumed),
                 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->energy_consumed), 4);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 8);
  unpack_uint16_t_array(10, buffer,
                        reinterpret_cast<uint16_t*>(&this->voltages[0]), 10);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->current_battery),
                 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->id), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->battery_function),
                 33);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 34);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->battery_remaining),
                35);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->time_remaining), 36);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->charge_state), 40);
  unpack_uint16_t_array(
      4, buffer, reinterpret_cast<uint16_t*>(&this->voltages_ext[0]), 41);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mode), 49);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->fault_bitmask),
                  50);
  return 54;
}

std::string MavLinkBatteryStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"BATTERY_STATUS\", \"id\": 147, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"current_consumed\":" << this->current_consumed;
  ss << ", \"energy_consumed\":" << this->energy_consumed;
  ss << ", \"temperature\":" << this->temperature;
  ss << ", \"voltages\":"
     << "["
     << uint16_t_array_tostring(10,
                                reinterpret_cast<uint16_t*>(&this->voltages[0]))
     << "]";
  ss << ", \"current_battery\":" << this->current_battery;
  ss << ", \"id\":" << static_cast<unsigned int>(this->id);
  ss << ", \"battery_function\":"
     << static_cast<unsigned int>(this->battery_function);
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"battery_remaining\":" << static_cast<int>(this->battery_remaining);
  ss << ", \"time_remaining\":" << this->time_remaining;
  ss << ", \"charge_state\":" << static_cast<unsigned int>(this->charge_state);
  ss << ", \"voltages_ext\":"
     << "["
     << uint16_t_array_tostring(
            4, reinterpret_cast<uint16_t*>(&this->voltages_ext[0]))
     << "]";
  ss << ", \"mode\":" << static_cast<unsigned int>(this->mode);
  ss << ", \"fault_bitmask\":" << this->fault_bitmask;
  ss << "} },";
  return ss.str();
}

int MavLinkAutopilotVersion::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->capabilities),
                0);
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->uid), 8);
  pack_uint32_t(
      buffer, reinterpret_cast<const uint32_t*>(&this->flight_sw_version), 16);
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(&this->middleware_sw_version),
                20);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->os_sw_version),
                24);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->board_version),
                28);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->vendor_id),
                32);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->product_id),
                34);
  pack_uint8_t_array(
      8, buffer,
      reinterpret_cast<const uint8_t*>(&this->flight_custom_version[0]), 36);
  pack_uint8_t_array(
      8, buffer,
      reinterpret_cast<const uint8_t*>(&this->middleware_custom_version[0]),
      44);
  pack_uint8_t_array(
      8, buffer, reinterpret_cast<const uint8_t*>(&this->os_custom_version[0]),
      52);
  pack_uint8_t_array(18, buffer,
                     reinterpret_cast<const uint8_t*>(&this->uid2[0]), 60);
  return 78;
}

int MavLinkAutopilotVersion::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->capabilities), 0);
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->uid), 8);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->flight_sw_version),
                  16);
  unpack_uint32_t(
      buffer, reinterpret_cast<uint32_t*>(&this->middleware_sw_version), 20);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->os_sw_version),
                  24);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->board_version),
                  28);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->vendor_id), 32);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->product_id), 34);
  unpack_uint8_t_array(
      8, buffer, reinterpret_cast<uint8_t*>(&this->flight_custom_version[0]),
      36);
  unpack_uint8_t_array(
      8, buffer,
      reinterpret_cast<uint8_t*>(&this->middleware_custom_version[0]), 44);
  unpack_uint8_t_array(
      8, buffer, reinterpret_cast<uint8_t*>(&this->os_custom_version[0]), 52);
  unpack_uint8_t_array(18, buffer, reinterpret_cast<uint8_t*>(&this->uid2[0]),
                       60);
  return 78;
}

std::string MavLinkAutopilotVersion::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"AUTOPILOT_VERSION\", \"id\": 148, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"capabilities\":" << this->capabilities;
  ss << ", \"uid\":" << this->uid;
  ss << ", \"flight_sw_version\":" << this->flight_sw_version;
  ss << ", \"middleware_sw_version\":" << this->middleware_sw_version;
  ss << ", \"os_sw_version\":" << this->os_sw_version;
  ss << ", \"board_version\":" << this->board_version;
  ss << ", \"vendor_id\":" << this->vendor_id;
  ss << ", \"product_id\":" << this->product_id;
  ss << ", \"flight_custom_version\":"
     << "["
     << uint8_t_array_tostring(
            8, reinterpret_cast<uint8_t*>(&this->flight_custom_version[0]))
     << "]";
  ss << ", \"middleware_custom_version\":"
     << "["
     << uint8_t_array_tostring(
            8, reinterpret_cast<uint8_t*>(&this->middleware_custom_version[0]))
     << "]";
  ss << ", \"os_custom_version\":"
     << "["
     << uint8_t_array_tostring(
            8, reinterpret_cast<uint8_t*>(&this->os_custom_version[0]))
     << "]";
  ss << ", \"uid2\":"
     << "["
     << uint8_t_array_tostring(18, reinterpret_cast<uint8_t*>(&this->uid2[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkLandingTarget::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->angle_x), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->angle_y), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->distance), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->size_x), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->size_y), 24);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_num), 28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->frame), 29);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 30);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 34);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 38);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 42);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 58);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->position_valid),
               59);
  return 60;
}

int MavLinkLandingTarget::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->angle_x), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->angle_y), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->distance), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->size_x), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->size_y), 24);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_num), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->frame), 29);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 30);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 34);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 38);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 42);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 58);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->position_valid), 59);
  return 60;
}

std::string MavLinkLandingTarget::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LANDING_TARGET\", \"id\": 149, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"angle_x\":" << float_tostring(this->angle_x);
  ss << ", \"angle_y\":" << float_tostring(this->angle_y);
  ss << ", \"distance\":" << float_tostring(this->distance);
  ss << ", \"size_x\":" << float_tostring(this->size_x);
  ss << ", \"size_y\":" << float_tostring(this->size_y);
  ss << ", \"target_num\":" << static_cast<unsigned int>(this->target_num);
  ss << ", \"frame\":" << static_cast<unsigned int>(this->frame);
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"position_valid\":"
     << static_cast<unsigned int>(this->position_valid);
  ss << "} },";
  return ss.str();
}

int MavLinkFenceStatus::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->breach_time),
                0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->breach_count),
                4);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->breach_status),
               6);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->breach_type), 7);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->breach_mitigation), 8);
  return 9;
}

int MavLinkFenceStatus::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->breach_time), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->breach_count), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->breach_status), 6);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->breach_type), 7);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->breach_mitigation),
                 8);
  return 9;
}

std::string MavLinkFenceStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"FENCE_STATUS\", \"id\": 162, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"breach_time\":" << this->breach_time;
  ss << ", \"breach_count\":" << this->breach_count;
  ss << ", \"breach_status\":"
     << static_cast<unsigned int>(this->breach_status);
  ss << ", \"breach_type\":" << static_cast<unsigned int>(this->breach_type);
  ss << ", \"breach_mitigation\":"
     << static_cast<unsigned int>(this->breach_mitigation);
  ss << "} },";
  return ss.str();
}

int MavLinkMagCalReport::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->fitness), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ofs_x), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ofs_y), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ofs_z), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->diag_x), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->diag_y), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->diag_z), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->offdiag_x), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->offdiag_y), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->offdiag_z), 36);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->compass_id), 40);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->cal_mask), 41);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->cal_status), 42);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->autosaved), 43);
  pack_float(buffer,
             reinterpret_cast<const float*>(&this->orientation_confidence), 44);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->old_orientation),
               48);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->new_orientation),
               49);
  pack_float(buffer, reinterpret_cast<const float*>(&this->scale_factor), 50);
  return 54;
}

int MavLinkMagCalReport::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->fitness), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ofs_x), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ofs_y), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ofs_z), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->diag_x), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->diag_y), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->diag_z), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->offdiag_x), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->offdiag_y), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->offdiag_z), 36);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->compass_id), 40);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->cal_mask), 41);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->cal_status), 42);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->autosaved), 43);
  unpack_float(buffer, reinterpret_cast<float*>(&this->orientation_confidence),
               44);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->old_orientation),
                 48);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->new_orientation),
                 49);
  unpack_float(buffer, reinterpret_cast<float*>(&this->scale_factor), 50);
  return 54;
}

std::string MavLinkMagCalReport::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MAG_CAL_REPORT\", \"id\": 192, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"fitness\":" << float_tostring(this->fitness);
  ss << ", \"ofs_x\":" << float_tostring(this->ofs_x);
  ss << ", \"ofs_y\":" << float_tostring(this->ofs_y);
  ss << ", \"ofs_z\":" << float_tostring(this->ofs_z);
  ss << ", \"diag_x\":" << float_tostring(this->diag_x);
  ss << ", \"diag_y\":" << float_tostring(this->diag_y);
  ss << ", \"diag_z\":" << float_tostring(this->diag_z);
  ss << ", \"offdiag_x\":" << float_tostring(this->offdiag_x);
  ss << ", \"offdiag_y\":" << float_tostring(this->offdiag_y);
  ss << ", \"offdiag_z\":" << float_tostring(this->offdiag_z);
  ss << ", \"compass_id\":" << static_cast<unsigned int>(this->compass_id);
  ss << ", \"cal_mask\":" << static_cast<unsigned int>(this->cal_mask);
  ss << ", \"cal_status\":" << static_cast<unsigned int>(this->cal_status);
  ss << ", \"autosaved\":" << static_cast<unsigned int>(this->autosaved);
  ss << ", \"orientation_confidence\":"
     << float_tostring(this->orientation_confidence);
  ss << ", \"old_orientation\":"
     << static_cast<unsigned int>(this->old_orientation);
  ss << ", \"new_orientation\":"
     << static_cast<unsigned int>(this->new_orientation);
  ss << ", \"scale_factor\":" << float_tostring(this->scale_factor);
  ss << "} },";
  return ss.str();
}

int MavLinkEfiStatus::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->ecu_index), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->rpm), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->fuel_consumed), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->fuel_flow), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->engine_load), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->throttle_position),
             20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->spark_dwell_time),
             24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->barometric_pressure),
             28);
  pack_float(buffer,
             reinterpret_cast<const float*>(&this->intake_manifold_pressure),
             32);
  pack_float(buffer,
             reinterpret_cast<const float*>(&this->intake_manifold_temperature),
             36);
  pack_float(buffer,
             reinterpret_cast<const float*>(&this->cylinder_head_temperature),
             40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ignition_timing),
             44);
  pack_float(buffer, reinterpret_cast<const float*>(&this->injection_time), 48);
  pack_float(buffer,
             reinterpret_cast<const float*>(&this->exhaust_gas_temperature),
             52);
  pack_float(buffer, reinterpret_cast<const float*>(&this->throttle_out), 56);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pt_compensation),
             60);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->health), 64);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ignition_voltage),
             65);
  return 69;
}

int MavLinkEfiStatus::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->ecu_index), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->rpm), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->fuel_consumed), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->fuel_flow), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->engine_load), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->throttle_position), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->spark_dwell_time), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->barometric_pressure),
               28);
  unpack_float(buffer,
               reinterpret_cast<float*>(&this->intake_manifold_pressure), 32);
  unpack_float(
      buffer, reinterpret_cast<float*>(&this->intake_manifold_temperature), 36);
  unpack_float(buffer,
               reinterpret_cast<float*>(&this->cylinder_head_temperature), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ignition_timing), 44);
  unpack_float(buffer, reinterpret_cast<float*>(&this->injection_time), 48);
  unpack_float(buffer, reinterpret_cast<float*>(&this->exhaust_gas_temperature),
               52);
  unpack_float(buffer, reinterpret_cast<float*>(&this->throttle_out), 56);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pt_compensation), 60);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->health), 64);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ignition_voltage), 65);
  return 69;
}

std::string MavLinkEfiStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"EFI_STATUS\", \"id\": 225, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"ecu_index\":" << float_tostring(this->ecu_index);
  ss << ", \"rpm\":" << float_tostring(this->rpm);
  ss << ", \"fuel_consumed\":" << float_tostring(this->fuel_consumed);
  ss << ", \"fuel_flow\":" << float_tostring(this->fuel_flow);
  ss << ", \"engine_load\":" << float_tostring(this->engine_load);
  ss << ", \"throttle_position\":" << float_tostring(this->throttle_position);
  ss << ", \"spark_dwell_time\":" << float_tostring(this->spark_dwell_time);
  ss << ", \"barometric_pressure\":"
     << float_tostring(this->barometric_pressure);
  ss << ", \"intake_manifold_pressure\":"
     << float_tostring(this->intake_manifold_pressure);
  ss << ", \"intake_manifold_temperature\":"
     << float_tostring(this->intake_manifold_temperature);
  ss << ", \"cylinder_head_temperature\":"
     << float_tostring(this->cylinder_head_temperature);
  ss << ", \"ignition_timing\":" << float_tostring(this->ignition_timing);
  ss << ", \"injection_time\":" << float_tostring(this->injection_time);
  ss << ", \"exhaust_gas_temperature\":"
     << float_tostring(this->exhaust_gas_temperature);
  ss << ", \"throttle_out\":" << float_tostring(this->throttle_out);
  ss << ", \"pt_compensation\":" << float_tostring(this->pt_compensation);
  ss << ", \"health\":" << static_cast<unsigned int>(this->health);
  ss << ", \"ignition_voltage\":" << float_tostring(this->ignition_voltage);
  ss << "} },";
  return ss.str();
}

int MavLinkEstimatorStatus::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vel_ratio), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pos_horiz_ratio),
             12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pos_vert_ratio), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->mag_ratio), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->hagl_ratio), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->tas_ratio), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pos_horiz_accuracy),
             32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pos_vert_accuracy),
             36);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->flags), 40);
  return 42;
}

int MavLinkEstimatorStatus::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vel_ratio), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pos_horiz_ratio), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pos_vert_ratio), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->mag_ratio), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->hagl_ratio), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->tas_ratio), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pos_horiz_accuracy), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pos_vert_accuracy), 36);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->flags), 40);
  return 42;
}

std::string MavLinkEstimatorStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ESTIMATOR_STATUS\", \"id\": 230, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"vel_ratio\":" << float_tostring(this->vel_ratio);
  ss << ", \"pos_horiz_ratio\":" << float_tostring(this->pos_horiz_ratio);
  ss << ", \"pos_vert_ratio\":" << float_tostring(this->pos_vert_ratio);
  ss << ", \"mag_ratio\":" << float_tostring(this->mag_ratio);
  ss << ", \"hagl_ratio\":" << float_tostring(this->hagl_ratio);
  ss << ", \"tas_ratio\":" << float_tostring(this->tas_ratio);
  ss << ", \"pos_horiz_accuracy\":" << float_tostring(this->pos_horiz_accuracy);
  ss << ", \"pos_vert_accuracy\":" << float_tostring(this->pos_vert_accuracy);
  ss << ", \"flags\":" << this->flags;
  ss << "} },";
  return ss.str();
}

int MavLinkWindCov::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->wind_x), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->wind_y), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->wind_z), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->var_horiz), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->var_vert), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->wind_alt), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->horiz_accuracy), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vert_accuracy), 36);
  return 40;
}

int MavLinkWindCov::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->wind_x), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->wind_y), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->wind_z), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->var_horiz), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->var_vert), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->wind_alt), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->horiz_accuracy), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vert_accuracy), 36);
  return 40;
}

std::string MavLinkWindCov::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"WIND_COV\", \"id\": 231, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"wind_x\":" << float_tostring(this->wind_x);
  ss << ", \"wind_y\":" << float_tostring(this->wind_y);
  ss << ", \"wind_z\":" << float_tostring(this->wind_z);
  ss << ", \"var_horiz\":" << float_tostring(this->var_horiz);
  ss << ", \"var_vert\":" << float_tostring(this->var_vert);
  ss << ", \"wind_alt\":" << float_tostring(this->wind_alt);
  ss << ", \"horiz_accuracy\":" << float_tostring(this->horiz_accuracy);
  ss << ", \"vert_accuracy\":" << float_tostring(this->vert_accuracy);
  ss << "} },";
  return ss.str();
}

int MavLinkGpsInput::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_week_ms),
                8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->alt), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->hdop), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vdop), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vn), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->ve), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vd), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->speed_accuracy), 44);
  pack_float(buffer, reinterpret_cast<const float*>(&this->horiz_accuracy), 48);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vert_accuracy), 52);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->ignore_flags),
                56);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->time_week),
                58);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->gps_id), 60);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->fix_type), 61);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->satellites_visible), 62);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->yaw), 63);
  return 65;
}

int MavLinkGpsInput::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_week_ms), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->alt), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->hdop), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vdop), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vn), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->ve), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vd), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->speed_accuracy), 44);
  unpack_float(buffer, reinterpret_cast<float*>(&this->horiz_accuracy), 48);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vert_accuracy), 52);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->ignore_flags), 56);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->time_week), 58);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->gps_id), 60);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->fix_type), 61);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->satellites_visible),
                 62);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->yaw), 63);
  return 65;
}

std::string MavLinkGpsInput::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GPS_INPUT\", \"id\": 232, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"time_week_ms\":" << this->time_week_ms;
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"alt\":" << float_tostring(this->alt);
  ss << ", \"hdop\":" << float_tostring(this->hdop);
  ss << ", \"vdop\":" << float_tostring(this->vdop);
  ss << ", \"vn\":" << float_tostring(this->vn);
  ss << ", \"ve\":" << float_tostring(this->ve);
  ss << ", \"vd\":" << float_tostring(this->vd);
  ss << ", \"speed_accuracy\":" << float_tostring(this->speed_accuracy);
  ss << ", \"horiz_accuracy\":" << float_tostring(this->horiz_accuracy);
  ss << ", \"vert_accuracy\":" << float_tostring(this->vert_accuracy);
  ss << ", \"ignore_flags\":" << this->ignore_flags;
  ss << ", \"time_week\":" << this->time_week;
  ss << ", \"gps_id\":" << static_cast<unsigned int>(this->gps_id);
  ss << ", \"fix_type\":" << static_cast<unsigned int>(this->fix_type);
  ss << ", \"satellites_visible\":"
     << static_cast<unsigned int>(this->satellites_visible);
  ss << ", \"yaw\":" << this->yaw;
  ss << "} },";
  return ss.str();
}

int MavLinkGpsRtcmData::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->flags), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->len), 1);
  pack_uint8_t_array(180, buffer,
                     reinterpret_cast<const uint8_t*>(&this->data[0]), 2);
  return 182;
}

int MavLinkGpsRtcmData::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->flags), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->len), 1);
  unpack_uint8_t_array(180, buffer, reinterpret_cast<uint8_t*>(&this->data[0]),
                       2);
  return 182;
}

std::string MavLinkGpsRtcmData::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GPS_RTCM_DATA\", \"id\": 233, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"flags\":" << static_cast<unsigned int>(this->flags);
  ss << ", \"len\":" << static_cast<unsigned int>(this->len);
  ss << ", \"data\":"
     << "["
     << uint8_t_array_tostring(180, reinterpret_cast<uint8_t*>(&this->data[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkHighLatency::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->custom_mode),
                0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->latitude), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->longitude), 8);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->roll), 12);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->pitch), 14);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->heading), 16);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->heading_sp), 18);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->altitude_amsl),
               20);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->altitude_sp),
               22);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->wp_distance),
                24);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->base_mode), 26);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->landed_state),
               27);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->throttle), 28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->airspeed), 29);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->airspeed_sp),
               30);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->groundspeed),
               31);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->climb_rate), 32);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->gps_nsat), 33);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->gps_fix_type),
               34);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->battery_remaining), 35);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->temperature), 36);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->temperature_air),
              37);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->failsafe), 38);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->wp_num), 39);
  return 40;
}

int MavLinkHighLatency::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->custom_mode), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->latitude), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->longitude), 8);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->roll), 12);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->pitch), 14);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->heading), 16);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->heading_sp), 18);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->altitude_amsl), 20);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->altitude_sp), 22);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->wp_distance), 24);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->base_mode), 26);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->landed_state), 27);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->throttle), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->airspeed), 29);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->airspeed_sp), 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->groundspeed), 31);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->climb_rate), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->gps_nsat), 33);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->gps_fix_type), 34);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->battery_remaining),
                 35);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->temperature), 36);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->temperature_air), 37);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->failsafe), 38);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->wp_num), 39);
  return 40;
}

std::string MavLinkHighLatency::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HIGH_LATENCY\", \"id\": 234, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"custom_mode\":" << this->custom_mode;
  ss << ", \"latitude\":" << this->latitude;
  ss << ", \"longitude\":" << this->longitude;
  ss << ", \"roll\":" << this->roll;
  ss << ", \"pitch\":" << this->pitch;
  ss << ", \"heading\":" << this->heading;
  ss << ", \"heading_sp\":" << this->heading_sp;
  ss << ", \"altitude_amsl\":" << this->altitude_amsl;
  ss << ", \"altitude_sp\":" << this->altitude_sp;
  ss << ", \"wp_distance\":" << this->wp_distance;
  ss << ", \"base_mode\":" << static_cast<unsigned int>(this->base_mode);
  ss << ", \"landed_state\":" << static_cast<unsigned int>(this->landed_state);
  ss << ", \"throttle\":" << static_cast<int>(this->throttle);
  ss << ", \"airspeed\":" << static_cast<unsigned int>(this->airspeed);
  ss << ", \"airspeed_sp\":" << static_cast<unsigned int>(this->airspeed_sp);
  ss << ", \"groundspeed\":" << static_cast<unsigned int>(this->groundspeed);
  ss << ", \"climb_rate\":" << static_cast<int>(this->climb_rate);
  ss << ", \"gps_nsat\":" << static_cast<unsigned int>(this->gps_nsat);
  ss << ", \"gps_fix_type\":" << static_cast<unsigned int>(this->gps_fix_type);
  ss << ", \"battery_remaining\":"
     << static_cast<unsigned int>(this->battery_remaining);
  ss << ", \"temperature\":" << static_cast<int>(this->temperature);
  ss << ", \"temperature_air\":" << static_cast<int>(this->temperature_air);
  ss << ", \"failsafe\":" << static_cast<unsigned int>(this->failsafe);
  ss << ", \"wp_num\":" << static_cast<unsigned int>(this->wp_num);
  ss << "} },";
  return ss.str();
}

int MavLinkHighLatency2::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->timestamp), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->latitude), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->longitude), 8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->custom_mode),
                12);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->altitude), 14);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->target_altitude),
               16);
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->target_distance), 18);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->wp_num), 20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->failure_flags),
                22);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 24);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->autopilot), 25);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->heading), 26);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_heading),
               27);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->throttle), 28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->airspeed), 29);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->airspeed_sp),
               30);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->groundspeed),
               31);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->windspeed), 32);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->wind_heading),
               33);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->eph), 34);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->epv), 35);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->temperature_air),
              36);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->climb_rate), 37);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->battery), 38);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->custom0), 39);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->custom1), 40);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->custom2), 41);
  return 42;
}

int MavLinkHighLatency2::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->timestamp), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->latitude), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->longitude), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->custom_mode), 12);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->altitude), 14);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->target_altitude),
                 16);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->target_distance),
                  18);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->wp_num), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->failure_flags),
                  22);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 24);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->autopilot), 25);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->heading), 26);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_heading), 27);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->throttle), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->airspeed), 29);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->airspeed_sp), 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->groundspeed), 31);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->windspeed), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->wind_heading), 33);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->eph), 34);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->epv), 35);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->temperature_air), 36);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->climb_rate), 37);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->battery), 38);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->custom0), 39);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->custom1), 40);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->custom2), 41);
  return 42;
}

std::string MavLinkHighLatency2::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HIGH_LATENCY2\", \"id\": 235, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"timestamp\":" << this->timestamp;
  ss << ", \"latitude\":" << this->latitude;
  ss << ", \"longitude\":" << this->longitude;
  ss << ", \"custom_mode\":" << this->custom_mode;
  ss << ", \"altitude\":" << this->altitude;
  ss << ", \"target_altitude\":" << this->target_altitude;
  ss << ", \"target_distance\":" << this->target_distance;
  ss << ", \"wp_num\":" << this->wp_num;
  ss << ", \"failure_flags\":" << this->failure_flags;
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"autopilot\":" << static_cast<unsigned int>(this->autopilot);
  ss << ", \"heading\":" << static_cast<unsigned int>(this->heading);
  ss << ", \"target_heading\":"
     << static_cast<unsigned int>(this->target_heading);
  ss << ", \"throttle\":" << static_cast<unsigned int>(this->throttle);
  ss << ", \"airspeed\":" << static_cast<unsigned int>(this->airspeed);
  ss << ", \"airspeed_sp\":" << static_cast<unsigned int>(this->airspeed_sp);
  ss << ", \"groundspeed\":" << static_cast<unsigned int>(this->groundspeed);
  ss << ", \"windspeed\":" << static_cast<unsigned int>(this->windspeed);
  ss << ", \"wind_heading\":" << static_cast<unsigned int>(this->wind_heading);
  ss << ", \"eph\":" << static_cast<unsigned int>(this->eph);
  ss << ", \"epv\":" << static_cast<unsigned int>(this->epv);
  ss << ", \"temperature_air\":" << static_cast<int>(this->temperature_air);
  ss << ", \"climb_rate\":" << static_cast<int>(this->climb_rate);
  ss << ", \"battery\":" << static_cast<int>(this->battery);
  ss << ", \"custom0\":" << static_cast<int>(this->custom0);
  ss << ", \"custom1\":" << static_cast<int>(this->custom1);
  ss << ", \"custom2\":" << static_cast<int>(this->custom2);
  ss << "} },";
  return ss.str();
}

int MavLinkVibration::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vibration_x), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vibration_y), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vibration_z), 16);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->clipping_0),
                20);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->clipping_1),
                24);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->clipping_2),
                28);
  return 32;
}

int MavLinkVibration::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vibration_x), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vibration_y), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vibration_z), 16);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->clipping_0), 20);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->clipping_1), 24);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->clipping_2), 28);
  return 32;
}

std::string MavLinkVibration::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"VIBRATION\", \"id\": 241, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"vibration_x\":" << float_tostring(this->vibration_x);
  ss << ", \"vibration_y\":" << float_tostring(this->vibration_y);
  ss << ", \"vibration_z\":" << float_tostring(this->vibration_z);
  ss << ", \"clipping_0\":" << this->clipping_0;
  ss << ", \"clipping_1\":" << this->clipping_1;
  ss << ", \"clipping_2\":" << this->clipping_2;
  ss << "} },";
  return ss.str();
}

int MavLinkHomePosition::pack(char* buffer) const {
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->latitude), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->longitude), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->altitude), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 20);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->approach_x), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->approach_y), 44);
  pack_float(buffer, reinterpret_cast<const float*>(&this->approach_z), 48);
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec),
                52);
  return 60;
}

int MavLinkHomePosition::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->latitude), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->longitude), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->altitude), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 20);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->approach_x), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->approach_y), 44);
  unpack_float(buffer, reinterpret_cast<float*>(&this->approach_z), 48);
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 52);
  return 60;
}

std::string MavLinkHomePosition::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HOME_POSITION\", \"id\": 242, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"latitude\":" << this->latitude;
  ss << ", \"longitude\":" << this->longitude;
  ss << ", \"altitude\":" << this->altitude;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"approach_x\":" << float_tostring(this->approach_x);
  ss << ", \"approach_y\":" << float_tostring(this->approach_y);
  ss << ", \"approach_z\":" << float_tostring(this->approach_z);
  ss << ", \"time_usec\":" << this->time_usec;
  ss << "} },";
  return ss.str();
}

int MavLinkSetHomePosition::pack(char* buffer) const {
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->latitude), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->longitude), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->altitude), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 20);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->approach_x), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->approach_y), 44);
  pack_float(buffer, reinterpret_cast<const float*>(&this->approach_z), 48);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               52);
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec),
                53);
  return 61;
}

int MavLinkSetHomePosition::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->latitude), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->longitude), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->altitude), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 20);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->approach_x), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->approach_y), 44);
  unpack_float(buffer, reinterpret_cast<float*>(&this->approach_z), 48);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 52);
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 53);
  return 61;
}

std::string MavLinkSetHomePosition::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SET_HOME_POSITION\", \"id\": 243, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"latitude\":" << this->latitude;
  ss << ", \"longitude\":" << this->longitude;
  ss << ", \"altitude\":" << this->altitude;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"approach_x\":" << float_tostring(this->approach_x);
  ss << ", \"approach_y\":" << float_tostring(this->approach_y);
  ss << ", \"approach_z\":" << float_tostring(this->approach_z);
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"time_usec\":" << this->time_usec;
  ss << "} },";
  return ss.str();
}

int MavLinkMessageInterval::pack(char* buffer) const {
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->interval_us), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->message_id),
                4);
  return 6;
}

int MavLinkMessageInterval::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->interval_us), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->message_id), 4);
  return 6;
}

std::string MavLinkMessageInterval::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MESSAGE_INTERVAL\", \"id\": 244, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"interval_us\":" << this->interval_us;
  ss << ", \"message_id\":" << this->message_id;
  ss << "} },";
  return ss.str();
}

int MavLinkExtendedSysState::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->vtol_state), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->landed_state),
               1);
  return 2;
}

int MavLinkExtendedSysState::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->vtol_state), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->landed_state), 1);
  return 2;
}

std::string MavLinkExtendedSysState::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"EXTENDED_SYS_STATE\", \"id\": 245, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"vtol_state\":" << static_cast<unsigned int>(this->vtol_state);
  ss << ", \"landed_state\":" << static_cast<unsigned int>(this->landed_state);
  ss << "} },";
  return ss.str();
}

int MavLinkAdsbVehicle::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->ICAO_address),
                0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->altitude), 12);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->heading), 16);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->hor_velocity),
                18);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->ver_velocity),
               20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->flags), 22);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->squawk), 24);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->altitude_type),
               26);
  pack_char_array(9, buffer, reinterpret_cast<const char*>(&this->callsign[0]),
                  27);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->emitter_type),
               36);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->tslc), 37);
  return 38;
}

int MavLinkAdsbVehicle::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->ICAO_address), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->altitude), 12);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->heading), 16);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->hor_velocity), 18);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->ver_velocity), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->flags), 22);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->squawk), 24);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->altitude_type), 26);
  unpack_char_array(9, buffer, reinterpret_cast<char*>(&this->callsign[0]), 27);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->emitter_type), 36);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->tslc), 37);
  return 38;
}

std::string MavLinkAdsbVehicle::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ADSB_VEHICLE\", \"id\": 246, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"ICAO_address\":" << this->ICAO_address;
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"altitude\":" << this->altitude;
  ss << ", \"heading\":" << this->heading;
  ss << ", \"hor_velocity\":" << this->hor_velocity;
  ss << ", \"ver_velocity\":" << this->ver_velocity;
  ss << ", \"flags\":" << this->flags;
  ss << ", \"squawk\":" << this->squawk;
  ss << ", \"altitude_type\":"
     << static_cast<unsigned int>(this->altitude_type);
  ss << ", \"callsign\":"
     << "\""
     << char_array_tostring(9, reinterpret_cast<char*>(&this->callsign[0]))
     << "\"";
  ss << ", \"emitter_type\":" << static_cast<unsigned int>(this->emitter_type);
  ss << ", \"tslc\":" << static_cast<unsigned int>(this->tslc);
  ss << "} },";
  return ss.str();
}

int MavLinkCollision::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->id), 0);
  pack_float(buffer,
             reinterpret_cast<const float*>(&this->time_to_minimum_delta), 4);
  pack_float(buffer,
             reinterpret_cast<const float*>(&this->altitude_minimum_delta), 8);
  pack_float(buffer,
             reinterpret_cast<const float*>(&this->horizontal_minimum_delta),
             12);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->src), 16);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->action), 17);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->threat_level),
               18);
  return 19;
}

int MavLinkCollision::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->id), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->time_to_minimum_delta),
               4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->altitude_minimum_delta),
               8);
  unpack_float(buffer,
               reinterpret_cast<float*>(&this->horizontal_minimum_delta), 12);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->src), 16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->action), 17);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->threat_level), 18);
  return 19;
}

std::string MavLinkCollision::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"COLLISION\", \"id\": 247, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"id\":" << this->id;
  ss << ", \"time_to_minimum_delta\":"
     << float_tostring(this->time_to_minimum_delta);
  ss << ", \"altitude_minimum_delta\":"
     << float_tostring(this->altitude_minimum_delta);
  ss << ", \"horizontal_minimum_delta\":"
     << float_tostring(this->horizontal_minimum_delta);
  ss << ", \"src\":" << static_cast<unsigned int>(this->src);
  ss << ", \"action\":" << static_cast<unsigned int>(this->action);
  ss << ", \"threat_level\":" << static_cast<unsigned int>(this->threat_level);
  ss << "} },";
  return ss.str();
}

int MavLinkV2Extension::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->message_type),
                0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_network),
               2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               3);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 4);
  pack_uint8_t_array(249, buffer,
                     reinterpret_cast<const uint8_t*>(&this->payload[0]), 5);
  return 254;
}

int MavLinkV2Extension::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->message_type), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_network), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 3);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 4);
  unpack_uint8_t_array(249, buffer,
                       reinterpret_cast<uint8_t*>(&this->payload[0]), 5);
  return 254;
}

std::string MavLinkV2Extension::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"V2_EXTENSION\", \"id\": 248, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"message_type\":" << this->message_type;
  ss << ", \"target_network\":"
     << static_cast<unsigned int>(this->target_network);
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"payload\":"
     << "["
     << uint8_t_array_tostring(249,
                               reinterpret_cast<uint8_t*>(&this->payload[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkMemoryVect::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->address), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->ver), 2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 3);
  pack_int8_t_array(32, buffer,
                    reinterpret_cast<const int8_t*>(&this->value[0]), 4);
  return 36;
}

int MavLinkMemoryVect::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->address), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->ver), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 3);
  unpack_int8_t_array(32, buffer, reinterpret_cast<int8_t*>(&this->value[0]),
                      4);
  return 36;
}

std::string MavLinkMemoryVect::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MEMORY_VECT\", \"id\": 249, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"address\":" << this->address;
  ss << ", \"ver\":" << static_cast<unsigned int>(this->ver);
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"value\":"
     << "["
     << int8_t_array_tostring(32, reinterpret_cast<int8_t*>(&this->value[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkDebugVect::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 16);
  pack_char_array(10, buffer, reinterpret_cast<const char*>(&this->name[0]),
                  20);
  return 30;
}

int MavLinkDebugVect::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 16);
  unpack_char_array(10, buffer, reinterpret_cast<char*>(&this->name[0]), 20);
  return 30;
}

std::string MavLinkDebugVect::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"DEBUG_VECT\", \"id\": 250, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"name\":"
     << "\"" << char_array_tostring(10, reinterpret_cast<char*>(&this->name[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkNamedValueFloat::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->value), 4);
  pack_char_array(10, buffer, reinterpret_cast<const char*>(&this->name[0]), 8);
  return 18;
}

int MavLinkNamedValueFloat::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->value), 4);
  unpack_char_array(10, buffer, reinterpret_cast<char*>(&this->name[0]), 8);
  return 18;
}

std::string MavLinkNamedValueFloat::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"NAMED_VALUE_FLOAT\", \"id\": 251, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"value\":" << float_tostring(this->value);
  ss << ", \"name\":"
     << "\"" << char_array_tostring(10, reinterpret_cast<char*>(&this->name[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkNamedValueInt::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->value), 4);
  pack_char_array(10, buffer, reinterpret_cast<const char*>(&this->name[0]), 8);
  return 18;
}

int MavLinkNamedValueInt::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->value), 4);
  unpack_char_array(10, buffer, reinterpret_cast<char*>(&this->name[0]), 8);
  return 18;
}

std::string MavLinkNamedValueInt::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"NAMED_VALUE_INT\", \"id\": 252, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"value\":" << this->value;
  ss << ", \"name\":"
     << "\"" << char_array_tostring(10, reinterpret_cast<char*>(&this->name[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkStatustext::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->severity), 0);
  pack_char_array(50, buffer, reinterpret_cast<const char*>(&this->text[0]), 1);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->id), 51);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->chunk_seq), 53);
  return 54;
}

int MavLinkStatustext::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->severity), 0);
  unpack_char_array(50, buffer, reinterpret_cast<char*>(&this->text[0]), 1);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->id), 51);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->chunk_seq), 53);
  return 54;
}

std::string MavLinkStatustext::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"STATUSTEXT\", \"id\": 253, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"severity\":" << static_cast<unsigned int>(this->severity);
  ss << ", \"text\":"
     << "\"" << char_array_tostring(50, reinterpret_cast<char*>(&this->text[0]))
     << "\"";
  ss << ", \"id\":" << this->id;
  ss << ", \"chunk_seq\":" << static_cast<unsigned int>(this->chunk_seq);
  ss << "} },";
  return ss.str();
}

int MavLinkDebug::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->value), 4);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->ind), 8);
  return 9;
}

int MavLinkDebug::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->value), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->ind), 8);
  return 9;
}

std::string MavLinkDebug::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"DEBUG\", \"id\": 254, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"value\":" << float_tostring(this->value);
  ss << ", \"ind\":" << static_cast<unsigned int>(this->ind);
  ss << "} },";
  return ss.str();
}

int MavLinkSetupSigning::pack(char* buffer) const {
  pack_uint64_t(buffer,
                reinterpret_cast<const uint64_t*>(&this->initial_timestamp), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               8);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 9);
  pack_uint8_t_array(
      32, buffer, reinterpret_cast<const uint8_t*>(&this->secret_key[0]), 10);
  return 42;
}

int MavLinkSetupSigning::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->initial_timestamp),
                  0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 9);
  unpack_uint8_t_array(32, buffer,
                       reinterpret_cast<uint8_t*>(&this->secret_key[0]), 10);
  return 42;
}

std::string MavLinkSetupSigning::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SETUP_SIGNING\", \"id\": 256, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"initial_timestamp\":" << this->initial_timestamp;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"secret_key\":"
     << "["
     << uint8_t_array_tostring(32,
                               reinterpret_cast<uint8_t*>(&this->secret_key[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkButtonChange::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(&this->last_change_ms), 4);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->state), 8);
  return 9;
}

int MavLinkButtonChange::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->last_change_ms),
                  4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->state), 8);
  return 9;
}

std::string MavLinkButtonChange::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"BUTTON_CHANGE\", \"id\": 257, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"last_change_ms\":" << this->last_change_ms;
  ss << ", \"state\":" << static_cast<unsigned int>(this->state);
  ss << "} },";
  return ss.str();
}

int MavLinkPlayTune::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  pack_char_array(30, buffer, reinterpret_cast<const char*>(&this->tune[0]), 2);
  pack_char_array(200, buffer, reinterpret_cast<const char*>(&this->tune2[0]),
                  32);
  return 232;
}

int MavLinkPlayTune::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  unpack_char_array(30, buffer, reinterpret_cast<char*>(&this->tune[0]), 2);
  unpack_char_array(200, buffer, reinterpret_cast<char*>(&this->tune2[0]), 32);
  return 232;
}

std::string MavLinkPlayTune::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PLAY_TUNE\", \"id\": 258, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"tune\":"
     << "\"" << char_array_tostring(30, reinterpret_cast<char*>(&this->tune[0]))
     << "\"";
  ss << ", \"tune2\":"
     << "\""
     << char_array_tostring(200, reinterpret_cast<char*>(&this->tune2[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkCameraInformation::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(&this->firmware_version), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->focal_length), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->sensor_size_h), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->sensor_size_v), 16);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->flags), 20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->resolution_h),
                24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->resolution_v),
                26);
  pack_uint16_t(
      buffer, reinterpret_cast<const uint16_t*>(&this->cam_definition_version),
      28);
  pack_uint8_t_array(
      32, buffer, reinterpret_cast<const uint8_t*>(&this->vendor_name[0]), 30);
  pack_uint8_t_array(
      32, buffer, reinterpret_cast<const uint8_t*>(&this->model_name[0]), 62);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->lens_id), 94);
  pack_char_array(140, buffer,
                  reinterpret_cast<const char*>(&this->cam_definition_uri[0]),
                  95);
  return 235;
}

int MavLinkCameraInformation::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->firmware_version),
                  4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->focal_length), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->sensor_size_h), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->sensor_size_v), 16);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->flags), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->resolution_h), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->resolution_v), 26);
  unpack_uint16_t(
      buffer, reinterpret_cast<uint16_t*>(&this->cam_definition_version), 28);
  unpack_uint8_t_array(32, buffer,
                       reinterpret_cast<uint8_t*>(&this->vendor_name[0]), 30);
  unpack_uint8_t_array(32, buffer,
                       reinterpret_cast<uint8_t*>(&this->model_name[0]), 62);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->lens_id), 94);
  unpack_char_array(140, buffer,
                    reinterpret_cast<char*>(&this->cam_definition_uri[0]), 95);
  return 235;
}

std::string MavLinkCameraInformation::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CAMERA_INFORMATION\", \"id\": 259, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"firmware_version\":" << this->firmware_version;
  ss << ", \"focal_length\":" << float_tostring(this->focal_length);
  ss << ", \"sensor_size_h\":" << float_tostring(this->sensor_size_h);
  ss << ", \"sensor_size_v\":" << float_tostring(this->sensor_size_v);
  ss << ", \"flags\":" << this->flags;
  ss << ", \"resolution_h\":" << this->resolution_h;
  ss << ", \"resolution_v\":" << this->resolution_v;
  ss << ", \"cam_definition_version\":" << this->cam_definition_version;
  ss << ", \"vendor_name\":"
     << "["
     << uint8_t_array_tostring(
            32, reinterpret_cast<uint8_t*>(&this->vendor_name[0]))
     << "]";
  ss << ", \"model_name\":"
     << "["
     << uint8_t_array_tostring(32,
                               reinterpret_cast<uint8_t*>(&this->model_name[0]))
     << "]";
  ss << ", \"lens_id\":" << static_cast<unsigned int>(this->lens_id);
  ss << ", \"cam_definition_uri\":"
     << "\""
     << char_array_tostring(
            140, reinterpret_cast<char*>(&this->cam_definition_uri[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkCameraSettings::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mode_id), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->zoomLevel), 5);
  pack_float(buffer, reinterpret_cast<const float*>(&this->focusLevel), 9);
  return 13;
}

int MavLinkCameraSettings::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mode_id), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->zoomLevel), 5);
  unpack_float(buffer, reinterpret_cast<float*>(&this->focusLevel), 9);
  return 13;
}

std::string MavLinkCameraSettings::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CAMERA_SETTINGS\", \"id\": 260, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"mode_id\":" << static_cast<unsigned int>(this->mode_id);
  ss << ", \"zoomLevel\":" << float_tostring(this->zoomLevel);
  ss << ", \"focusLevel\":" << float_tostring(this->focusLevel);
  ss << "} },";
  return ss.str();
}

int MavLinkStorageInformation::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->total_capacity), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->used_capacity), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->available_capacity),
             12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->read_speed), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->write_speed), 20);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->storage_id), 24);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->storage_count),
               25);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->status), 26);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 27);
  pack_char_array(32, buffer, reinterpret_cast<const char*>(&this->name[0]),
                  28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->storage_usage),
               60);
  return 61;
}

int MavLinkStorageInformation::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->total_capacity), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->used_capacity), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->available_capacity), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->read_speed), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->write_speed), 20);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->storage_id), 24);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->storage_count), 25);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->status), 26);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 27);
  unpack_char_array(32, buffer, reinterpret_cast<char*>(&this->name[0]), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->storage_usage), 60);
  return 61;
}

std::string MavLinkStorageInformation::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"STORAGE_INFORMATION\", \"id\": 261, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"total_capacity\":" << float_tostring(this->total_capacity);
  ss << ", \"used_capacity\":" << float_tostring(this->used_capacity);
  ss << ", \"available_capacity\":" << float_tostring(this->available_capacity);
  ss << ", \"read_speed\":" << float_tostring(this->read_speed);
  ss << ", \"write_speed\":" << float_tostring(this->write_speed);
  ss << ", \"storage_id\":" << static_cast<unsigned int>(this->storage_id);
  ss << ", \"storage_count\":"
     << static_cast<unsigned int>(this->storage_count);
  ss << ", \"status\":" << static_cast<unsigned int>(this->status);
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"name\":"
     << "\"" << char_array_tostring(32, reinterpret_cast<char*>(&this->name[0]))
     << "\"";
  ss << ", \"storage_usage\":"
     << static_cast<unsigned int>(this->storage_usage);
  ss << "} },";
  return ss.str();
}

int MavLinkCameraCaptureStatus::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->image_interval), 4);
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(&this->recording_time_ms), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->available_capacity),
             12);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->image_status),
               16);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->video_status),
               17);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->image_count),
               18);
  return 22;
}

int MavLinkCameraCaptureStatus::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->image_interval), 4);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->recording_time_ms),
                  8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->available_capacity), 12);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->image_status), 16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->video_status), 17);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->image_count), 18);
  return 22;
}

std::string MavLinkCameraCaptureStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CAMERA_CAPTURE_STATUS\", \"id\": 262, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"image_interval\":" << float_tostring(this->image_interval);
  ss << ", \"recording_time_ms\":" << this->recording_time_ms;
  ss << ", \"available_capacity\":" << float_tostring(this->available_capacity);
  ss << ", \"image_status\":" << static_cast<unsigned int>(this->image_status);
  ss << ", \"video_status\":" << static_cast<unsigned int>(this->video_status);
  ss << ", \"image_count\":" << this->image_count;
  ss << "} },";
  return ss.str();
}

int MavLinkCameraImageCaptured::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_utc), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 16);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt), 20);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->relative_alt),
               24);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 28);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->image_index),
               44);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->camera_id), 48);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->capture_result),
              49);
  pack_char_array(205, buffer,
                  reinterpret_cast<const char*>(&this->file_url[0]), 50);
  return 255;
}

int MavLinkCameraImageCaptured::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_utc), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 16);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt), 20);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->relative_alt), 24);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 28);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->image_index), 44);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->camera_id), 48);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->capture_result), 49);
  unpack_char_array(205, buffer, reinterpret_cast<char*>(&this->file_url[0]),
                    50);
  return 255;
}

std::string MavLinkCameraImageCaptured::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CAMERA_IMAGE_CAPTURED\", \"id\": 263, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_utc\":" << this->time_utc;
  ss << ", \"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"alt\":" << this->alt;
  ss << ", \"relative_alt\":" << this->relative_alt;
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"image_index\":" << this->image_index;
  ss << ", \"camera_id\":" << static_cast<unsigned int>(this->camera_id);
  ss << ", \"capture_result\":" << static_cast<int>(this->capture_result);
  ss << ", \"file_url\":"
     << "\""
     << char_array_tostring(205, reinterpret_cast<char*>(&this->file_url[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkFlightInformation::pack(char* buffer) const {
  pack_uint64_t(buffer,
                reinterpret_cast<const uint64_t*>(&this->arming_time_utc), 0);
  pack_uint64_t(buffer,
                reinterpret_cast<const uint64_t*>(&this->takeoff_time_utc), 8);
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->flight_uuid),
                16);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                24);
  return 28;
}

int MavLinkFlightInformation::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->arming_time_utc),
                  0);
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->takeoff_time_utc),
                  8);
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->flight_uuid), 16);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 24);
  return 28;
}

std::string MavLinkFlightInformation::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"FLIGHT_INFORMATION\", \"id\": 264, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"arming_time_utc\":" << this->arming_time_utc;
  ss << ", \"takeoff_time_utc\":" << this->takeoff_time_utc;
  ss << ", \"flight_uuid\":" << this->flight_uuid;
  ss << ", \"time_boot_ms\":" << this->time_boot_ms;
  ss << "} },";
  return ss.str();
}

int MavLinkMountOrientation::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_absolute), 16);
  return 20;
}

int MavLinkMountOrientation::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_absolute), 16);
  return 20;
}

std::string MavLinkMountOrientation::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"MOUNT_ORIENTATION\", \"id\": 265, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"roll\":" << float_tostring(this->roll);
  ss << ", \"pitch\":" << float_tostring(this->pitch);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"yaw_absolute\":" << float_tostring(this->yaw_absolute);
  ss << "} },";
  return ss.str();
}

int MavLinkLoggingData::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->sequence), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 3);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->length), 4);
  pack_uint8_t(
      buffer, reinterpret_cast<const uint8_t*>(&this->first_message_offset), 5);
  pack_uint8_t_array(249, buffer,
                     reinterpret_cast<const uint8_t*>(&this->data[0]), 6);
  return 255;
}

int MavLinkLoggingData::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->sequence), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 3);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->length), 4);
  unpack_uint8_t(buffer,
                 reinterpret_cast<uint8_t*>(&this->first_message_offset), 5);
  unpack_uint8_t_array(249, buffer, reinterpret_cast<uint8_t*>(&this->data[0]),
                       6);
  return 255;
}

std::string MavLinkLoggingData::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LOGGING_DATA\", \"id\": 266, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"sequence\":" << this->sequence;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"length\":" << static_cast<unsigned int>(this->length);
  ss << ", \"first_message_offset\":"
     << static_cast<unsigned int>(this->first_message_offset);
  ss << ", \"data\":"
     << "["
     << uint8_t_array_tostring(249, reinterpret_cast<uint8_t*>(&this->data[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkLoggingDataAcked::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->sequence), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 3);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->length), 4);
  pack_uint8_t(
      buffer, reinterpret_cast<const uint8_t*>(&this->first_message_offset), 5);
  pack_uint8_t_array(249, buffer,
                     reinterpret_cast<const uint8_t*>(&this->data[0]), 6);
  return 255;
}

int MavLinkLoggingDataAcked::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->sequence), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 3);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->length), 4);
  unpack_uint8_t(buffer,
                 reinterpret_cast<uint8_t*>(&this->first_message_offset), 5);
  unpack_uint8_t_array(249, buffer, reinterpret_cast<uint8_t*>(&this->data[0]),
                       6);
  return 255;
}

std::string MavLinkLoggingDataAcked::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LOGGING_DATA_ACKED\", \"id\": 267, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"sequence\":" << this->sequence;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"length\":" << static_cast<unsigned int>(this->length);
  ss << ", \"first_message_offset\":"
     << static_cast<unsigned int>(this->first_message_offset);
  ss << ", \"data\":"
     << "["
     << uint8_t_array_tostring(249, reinterpret_cast<uint8_t*>(&this->data[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkLoggingAck::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->sequence), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 3);
  return 4;
}

int MavLinkLoggingAck::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->sequence), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 3);
  return 4;
}

std::string MavLinkLoggingAck::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"LOGGING_ACK\", \"id\": 268, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"sequence\":" << this->sequence;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkVideoStreamInformation::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->framerate), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->bitrate), 4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->flags), 8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->resolution_h),
                10);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->resolution_v),
                12);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->rotation), 14);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->hfov), 16);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->stream_id), 18);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->count), 19);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 20);
  pack_char_array(32, buffer, reinterpret_cast<const char*>(&this->name[0]),
                  21);
  pack_char_array(160, buffer, reinterpret_cast<const char*>(&this->uri[0]),
                  53);
  return 213;
}

int MavLinkVideoStreamInformation::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->framerate), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->bitrate), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->flags), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->resolution_h), 10);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->resolution_v), 12);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->rotation), 14);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->hfov), 16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->stream_id), 18);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->count), 19);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 20);
  unpack_char_array(32, buffer, reinterpret_cast<char*>(&this->name[0]), 21);
  unpack_char_array(160, buffer, reinterpret_cast<char*>(&this->uri[0]), 53);
  return 213;
}

std::string MavLinkVideoStreamInformation::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"VIDEO_STREAM_INFORMATION\", \"id\": 269, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"framerate\":" << float_tostring(this->framerate);
  ss << ", \"bitrate\":" << this->bitrate;
  ss << ", \"flags\":" << this->flags;
  ss << ", \"resolution_h\":" << this->resolution_h;
  ss << ", \"resolution_v\":" << this->resolution_v;
  ss << ", \"rotation\":" << this->rotation;
  ss << ", \"hfov\":" << this->hfov;
  ss << ", \"stream_id\":" << static_cast<unsigned int>(this->stream_id);
  ss << ", \"count\":" << static_cast<unsigned int>(this->count);
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"name\":"
     << "\"" << char_array_tostring(32, reinterpret_cast<char*>(&this->name[0]))
     << "\"";
  ss << ", \"uri\":"
     << "\"" << char_array_tostring(160, reinterpret_cast<char*>(&this->uri[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkVideoStreamStatus::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->framerate), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->bitrate), 4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->flags), 8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->resolution_h),
                10);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->resolution_v),
                12);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->rotation), 14);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->hfov), 16);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->stream_id), 18);
  return 19;
}

int MavLinkVideoStreamStatus::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->framerate), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->bitrate), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->flags), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->resolution_h), 10);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->resolution_v), 12);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->rotation), 14);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->hfov), 16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->stream_id), 18);
  return 19;
}

std::string MavLinkVideoStreamStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"VIDEO_STREAM_STATUS\", \"id\": 270, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"framerate\":" << float_tostring(this->framerate);
  ss << ", \"bitrate\":" << this->bitrate;
  ss << ", \"flags\":" << this->flags;
  ss << ", \"resolution_h\":" << this->resolution_h;
  ss << ", \"resolution_v\":" << this->resolution_v;
  ss << ", \"rotation\":" << this->rotation;
  ss << ", \"hfov\":" << this->hfov;
  ss << ", \"stream_id\":" << static_cast<unsigned int>(this->stream_id);
  ss << "} },";
  return ss.str();
}

int MavLinkCameraFovStatus::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat_camera), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon_camera), 8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt_camera), 12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat_image), 16);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon_image), 20);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt_image), 24);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->hfov), 44);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vfov), 48);
  return 52;
}

int MavLinkCameraFovStatus::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat_camera), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon_camera), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt_camera), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat_image), 16);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon_image), 20);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt_image), 24);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->hfov), 44);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vfov), 48);
  return 52;
}

std::string MavLinkCameraFovStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CAMERA_FOV_STATUS\", \"id\": 271, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"lat_camera\":" << this->lat_camera;
  ss << ", \"lon_camera\":" << this->lon_camera;
  ss << ", \"alt_camera\":" << this->alt_camera;
  ss << ", \"lat_image\":" << this->lat_image;
  ss << ", \"lon_image\":" << this->lon_image;
  ss << ", \"alt_image\":" << this->alt_image;
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"hfov\":" << float_tostring(this->hfov);
  ss << ", \"vfov\":" << float_tostring(this->vfov);
  ss << "} },";
  return ss.str();
}

int MavLinkCameraTrackingImageStatus::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->point_x), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->point_y), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->radius), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->rec_top_x), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->rec_top_y), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->rec_bottom_x), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->rec_bottom_y), 24);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->tracking_status),
               28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->tracking_mode),
               29);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_data),
               30);
  return 31;
}

int MavLinkCameraTrackingImageStatus::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->point_x), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->point_y), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->radius), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->rec_top_x), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->rec_top_y), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->rec_bottom_x), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->rec_bottom_y), 24);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->tracking_status),
                 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->tracking_mode), 29);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_data), 30);
  return 31;
}

std::string MavLinkCameraTrackingImageStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CAMERA_TRACKING_IMAGE_STATUS\", \"id\": 275, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"point_x\":" << float_tostring(this->point_x);
  ss << ", \"point_y\":" << float_tostring(this->point_y);
  ss << ", \"radius\":" << float_tostring(this->radius);
  ss << ", \"rec_top_x\":" << float_tostring(this->rec_top_x);
  ss << ", \"rec_top_y\":" << float_tostring(this->rec_top_y);
  ss << ", \"rec_bottom_x\":" << float_tostring(this->rec_bottom_x);
  ss << ", \"rec_bottom_y\":" << float_tostring(this->rec_bottom_y);
  ss << ", \"tracking_status\":"
     << static_cast<unsigned int>(this->tracking_status);
  ss << ", \"tracking_mode\":"
     << static_cast<unsigned int>(this->tracking_mode);
  ss << ", \"target_data\":" << static_cast<unsigned int>(this->target_data);
  ss << "} },";
  return ss.str();
}

int MavLinkCameraTrackingGeoStatus::pack(char* buffer) const {
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->alt), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->h_acc), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->v_acc), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vel_n), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vel_e), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vel_d), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vel_acc), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->dist), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->hdg), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->hdg_acc), 44);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->tracking_status),
               48);
  return 49;
}

int MavLinkCameraTrackingGeoStatus::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->alt), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->h_acc), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->v_acc), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vel_n), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vel_e), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vel_d), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vel_acc), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->dist), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->hdg), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->hdg_acc), 44);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->tracking_status),
                 48);
  return 49;
}

std::string MavLinkCameraTrackingGeoStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CAMERA_TRACKING_GEO_STATUS\", \"id\": 276, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"alt\":" << float_tostring(this->alt);
  ss << ", \"h_acc\":" << float_tostring(this->h_acc);
  ss << ", \"v_acc\":" << float_tostring(this->v_acc);
  ss << ", \"vel_n\":" << float_tostring(this->vel_n);
  ss << ", \"vel_e\":" << float_tostring(this->vel_e);
  ss << ", \"vel_d\":" << float_tostring(this->vel_d);
  ss << ", \"vel_acc\":" << float_tostring(this->vel_acc);
  ss << ", \"dist\":" << float_tostring(this->dist);
  ss << ", \"hdg\":" << float_tostring(this->hdg);
  ss << ", \"hdg_acc\":" << float_tostring(this->hdg_acc);
  ss << ", \"tracking_status\":"
     << static_cast<unsigned int>(this->tracking_status);
  ss << "} },";
  return ss.str();
}

int MavLinkGimbalManagerInformation::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->cap_flags), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll_min), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll_max), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch_min), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch_max), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_min), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_max), 28);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->gimbal_device_id), 32);
  return 33;
}

int MavLinkGimbalManagerInformation::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->cap_flags), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll_min), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll_max), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch_min), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch_max), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_min), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_max), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->gimbal_device_id),
                 32);
  return 33;
}

std::string MavLinkGimbalManagerInformation::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GIMBAL_MANAGER_INFORMATION\", \"id\": 280, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"cap_flags\":" << this->cap_flags;
  ss << ", \"roll_min\":" << float_tostring(this->roll_min);
  ss << ", \"roll_max\":" << float_tostring(this->roll_max);
  ss << ", \"pitch_min\":" << float_tostring(this->pitch_min);
  ss << ", \"pitch_max\":" << float_tostring(this->pitch_max);
  ss << ", \"yaw_min\":" << float_tostring(this->yaw_min);
  ss << ", \"yaw_max\":" << float_tostring(this->yaw_max);
  ss << ", \"gimbal_device_id\":"
     << static_cast<unsigned int>(this->gimbal_device_id);
  ss << "} },";
  return ss.str();
}

int MavLinkGimbalManagerStatus::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->flags), 4);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->gimbal_device_id), 8);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->primary_control_sysid),
               9);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->primary_control_compid),
               10);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->secondary_control_sysid),
               11);
  pack_uint8_t(
      buffer, reinterpret_cast<const uint8_t*>(&this->secondary_control_compid),
      12);
  return 13;
}

int MavLinkGimbalManagerStatus::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->flags), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->gimbal_device_id),
                 8);
  unpack_uint8_t(buffer,
                 reinterpret_cast<uint8_t*>(&this->primary_control_sysid), 9);
  unpack_uint8_t(buffer,
                 reinterpret_cast<uint8_t*>(&this->primary_control_compid), 10);
  unpack_uint8_t(
      buffer, reinterpret_cast<uint8_t*>(&this->secondary_control_sysid), 11);
  unpack_uint8_t(
      buffer, reinterpret_cast<uint8_t*>(&this->secondary_control_compid), 12);
  return 13;
}

std::string MavLinkGimbalManagerStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GIMBAL_MANAGER_STATUS\", \"id\": 281, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"flags\":" << this->flags;
  ss << ", \"gimbal_device_id\":"
     << static_cast<unsigned int>(this->gimbal_device_id);
  ss << ", \"primary_control_sysid\":"
     << static_cast<unsigned int>(this->primary_control_sysid);
  ss << ", \"primary_control_compid\":"
     << static_cast<unsigned int>(this->primary_control_compid);
  ss << ", \"secondary_control_sysid\":"
     << static_cast<unsigned int>(this->secondary_control_sysid);
  ss << ", \"secondary_control_compid\":"
     << static_cast<unsigned int>(this->secondary_control_compid);
  ss << "} },";
  return ss.str();
}

int MavLinkGimbalManagerSetAttitude::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->flags), 0);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->angular_velocity_x),
             20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->angular_velocity_y),
             24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->angular_velocity_z),
             28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               32);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 33);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->gimbal_device_id), 34);
  return 35;
}

int MavLinkGimbalManagerSetAttitude::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->flags), 0);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->angular_velocity_x), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->angular_velocity_y), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->angular_velocity_z), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 33);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->gimbal_device_id),
                 34);
  return 35;
}

std::string MavLinkGimbalManagerSetAttitude::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GIMBAL_MANAGER_SET_ATTITUDE\", \"id\": 282, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"flags\":" << this->flags;
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"angular_velocity_x\":" << float_tostring(this->angular_velocity_x);
  ss << ", \"angular_velocity_y\":" << float_tostring(this->angular_velocity_y);
  ss << ", \"angular_velocity_z\":" << float_tostring(this->angular_velocity_z);
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"gimbal_device_id\":"
     << static_cast<unsigned int>(this->gimbal_device_id);
  ss << "} },";
  return ss.str();
}

int MavLinkGimbalDeviceInformation::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->uid), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                8);
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(&this->firmware_version), 12);
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(&this->hardware_version), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll_min), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->roll_max), 24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch_min), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch_max), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_min), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_max), 40);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->cap_flags),
                44);
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->custom_cap_flags), 46);
  pack_char_array(32, buffer,
                  reinterpret_cast<const char*>(&this->vendor_name[0]), 48);
  pack_char_array(32, buffer,
                  reinterpret_cast<const char*>(&this->model_name[0]), 80);
  pack_char_array(32, buffer,
                  reinterpret_cast<const char*>(&this->custom_name[0]), 112);
  return 144;
}

int MavLinkGimbalDeviceInformation::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->uid), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 8);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->firmware_version),
                  12);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->hardware_version),
                  16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll_min), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->roll_max), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch_min), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch_max), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_min), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_max), 40);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->cap_flags), 44);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->custom_cap_flags),
                  46);
  unpack_char_array(32, buffer, reinterpret_cast<char*>(&this->vendor_name[0]),
                    48);
  unpack_char_array(32, buffer, reinterpret_cast<char*>(&this->model_name[0]),
                    80);
  unpack_char_array(32, buffer, reinterpret_cast<char*>(&this->custom_name[0]),
                    112);
  return 144;
}

std::string MavLinkGimbalDeviceInformation::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GIMBAL_DEVICE_INFORMATION\", \"id\": 283, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"uid\":" << this->uid;
  ss << ", \"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"firmware_version\":" << this->firmware_version;
  ss << ", \"hardware_version\":" << this->hardware_version;
  ss << ", \"roll_min\":" << float_tostring(this->roll_min);
  ss << ", \"roll_max\":" << float_tostring(this->roll_max);
  ss << ", \"pitch_min\":" << float_tostring(this->pitch_min);
  ss << ", \"pitch_max\":" << float_tostring(this->pitch_max);
  ss << ", \"yaw_min\":" << float_tostring(this->yaw_min);
  ss << ", \"yaw_max\":" << float_tostring(this->yaw_max);
  ss << ", \"cap_flags\":" << this->cap_flags;
  ss << ", \"custom_cap_flags\":" << this->custom_cap_flags;
  ss << ", \"vendor_name\":"
     << "\""
     << char_array_tostring(32, reinterpret_cast<char*>(&this->vendor_name[0]))
     << "\"";
  ss << ", \"model_name\":"
     << "\""
     << char_array_tostring(32, reinterpret_cast<char*>(&this->model_name[0]))
     << "\"";
  ss << ", \"custom_name\":"
     << "\""
     << char_array_tostring(32, reinterpret_cast<char*>(&this->custom_name[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkGimbalDeviceSetAttitude::pack(char* buffer) const {
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->angular_velocity_x),
             16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->angular_velocity_y),
             20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->angular_velocity_z),
             24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->flags), 28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               30);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 31);
  return 32;
}

int MavLinkGimbalDeviceSetAttitude::unpack(const char* buffer) {
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->angular_velocity_x), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->angular_velocity_y), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->angular_velocity_z), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->flags), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 31);
  return 32;
}

std::string MavLinkGimbalDeviceSetAttitude::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GIMBAL_DEVICE_SET_ATTITUDE\", \"id\": 284, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"angular_velocity_x\":" << float_tostring(this->angular_velocity_x);
  ss << ", \"angular_velocity_y\":" << float_tostring(this->angular_velocity_y);
  ss << ", \"angular_velocity_z\":" << float_tostring(this->angular_velocity_z);
  ss << ", \"flags\":" << this->flags;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkGimbalDeviceAttitudeStatus::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->angular_velocity_x),
             20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->angular_velocity_y),
             24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->angular_velocity_z),
             28);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->failure_flags),
                32);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->flags), 36);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               38);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 39);
  return 40;
}

int MavLinkGimbalDeviceAttitudeStatus::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->angular_velocity_x), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->angular_velocity_y), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->angular_velocity_z), 28);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->failure_flags),
                  32);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->flags), 36);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 38);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 39);
  return 40;
}

std::string MavLinkGimbalDeviceAttitudeStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GIMBAL_DEVICE_ATTITUDE_STATUS\", \"id\": 285, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"angular_velocity_x\":" << float_tostring(this->angular_velocity_x);
  ss << ", \"angular_velocity_y\":" << float_tostring(this->angular_velocity_y);
  ss << ", \"angular_velocity_z\":" << float_tostring(this->angular_velocity_z);
  ss << ", \"failure_flags\":" << this->failure_flags;
  ss << ", \"flags\":" << this->flags;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkAutopilotStateForGimbalDevice::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_boot_us),
                0);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 8);
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(&this->q_estimated_delay_us),
                24);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vx), 28);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vy), 32);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vz), 36);
  pack_uint32_t(buffer,
                reinterpret_cast<const uint32_t*>(&this->v_estimated_delay_us),
                40);
  pack_float(
      buffer,
      reinterpret_cast<const float*>(&this->feed_forward_angular_velocity_z),
      44);
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->estimator_status), 48);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               50);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 51);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->landed_state),
               52);
  return 53;
}

int MavLinkAutopilotStateForGimbalDevice::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_boot_us), 0);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 8);
  unpack_uint32_t(buffer,
                  reinterpret_cast<uint32_t*>(&this->q_estimated_delay_us), 24);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vx), 28);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vy), 32);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vz), 36);
  unpack_uint32_t(buffer,
                  reinterpret_cast<uint32_t*>(&this->v_estimated_delay_us), 40);
  unpack_float(buffer,
               reinterpret_cast<float*>(&this->feed_forward_angular_velocity_z),
               44);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->estimator_status),
                  48);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 50);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 51);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->landed_state), 52);
  return 53;
}

std::string MavLinkAutopilotStateForGimbalDevice::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"AUTOPILOT_STATE_FOR_GIMBAL_DEVICE\", \"id\": 286, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_us\":" << this->time_boot_us;
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"q_estimated_delay_us\":" << this->q_estimated_delay_us;
  ss << ", \"vx\":" << float_tostring(this->vx);
  ss << ", \"vy\":" << float_tostring(this->vy);
  ss << ", \"vz\":" << float_tostring(this->vz);
  ss << ", \"v_estimated_delay_us\":" << this->v_estimated_delay_us;
  ss << ", \"feed_forward_angular_velocity_z\":"
     << float_tostring(this->feed_forward_angular_velocity_z);
  ss << ", \"estimator_status\":" << this->estimator_status;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"landed_state\":" << static_cast<unsigned int>(this->landed_state);
  ss << "} },";
  return ss.str();
}

int MavLinkGimbalManagerSetPitchyaw::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->flags), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch_rate), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_rate), 16);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               20);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 21);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->gimbal_device_id), 22);
  return 23;
}

int MavLinkGimbalManagerSetPitchyaw::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->flags), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch_rate), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_rate), 16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 20);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 21);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->gimbal_device_id),
                 22);
  return 23;
}

std::string MavLinkGimbalManagerSetPitchyaw::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GIMBAL_MANAGER_SET_PITCHYAW\", \"id\": 287, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"flags\":" << this->flags;
  ss << ", \"pitch\":" << float_tostring(this->pitch);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"pitch_rate\":" << float_tostring(this->pitch_rate);
  ss << ", \"yaw_rate\":" << float_tostring(this->yaw_rate);
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"gimbal_device_id\":"
     << static_cast<unsigned int>(this->gimbal_device_id);
  ss << "} },";
  return ss.str();
}

int MavLinkGimbalManagerSetManualControl::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->flags), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitch_rate), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yaw_rate), 16);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               20);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 21);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->gimbal_device_id), 22);
  return 23;
}

int MavLinkGimbalManagerSetManualControl::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->flags), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitch_rate), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yaw_rate), 16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 20);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 21);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->gimbal_device_id),
                 22);
  return 23;
}

std::string MavLinkGimbalManagerSetManualControl::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GIMBAL_MANAGER_SET_MANUAL_CONTROL\", \"id\": 288, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"flags\":" << this->flags;
  ss << ", \"pitch\":" << float_tostring(this->pitch);
  ss << ", \"yaw\":" << float_tostring(this->yaw);
  ss << ", \"pitch_rate\":" << float_tostring(this->pitch_rate);
  ss << ", \"yaw_rate\":" << float_tostring(this->yaw_rate);
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"gimbal_device_id\":"
     << static_cast<unsigned int>(this->gimbal_device_id);
  ss << "} },";
  return ss.str();
}

int MavLinkEscInfo::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint32_t_array(
      4, buffer, reinterpret_cast<const uint32_t*>(&this->error_count[0]), 8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->counter), 24);
  pack_uint16_t_array(
      4, buffer, reinterpret_cast<const uint16_t*>(&this->failure_flags[0]),
      26);
  pack_int16_t_array(
      4, buffer, reinterpret_cast<const int16_t*>(&this->temperature[0]), 34);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->index), 42);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->count), 43);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->connection_type),
               44);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->info), 45);
  return 46;
}

int MavLinkEscInfo::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint32_t_array(4, buffer,
                        reinterpret_cast<uint32_t*>(&this->error_count[0]), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->counter), 24);
  unpack_uint16_t_array(
      4, buffer, reinterpret_cast<uint16_t*>(&this->failure_flags[0]), 26);
  unpack_int16_t_array(4, buffer,
                       reinterpret_cast<int16_t*>(&this->temperature[0]), 34);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->index), 42);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->count), 43);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->connection_type),
                 44);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->info), 45);
  return 46;
}

std::string MavLinkEscInfo::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ESC_INFO\", \"id\": 290, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"error_count\":"
     << "["
     << uint32_t_array_tostring(
            4, reinterpret_cast<uint32_t*>(&this->error_count[0]))
     << "]";
  ss << ", \"counter\":" << this->counter;
  ss << ", \"failure_flags\":"
     << "["
     << uint16_t_array_tostring(
            4, reinterpret_cast<uint16_t*>(&this->failure_flags[0]))
     << "]";
  ss << ", \"temperature\":"
     << "["
     << int16_t_array_tostring(
            4, reinterpret_cast<int16_t*>(&this->temperature[0]))
     << "]";
  ss << ", \"index\":" << static_cast<unsigned int>(this->index);
  ss << ", \"count\":" << static_cast<unsigned int>(this->count);
  ss << ", \"connection_type\":"
     << static_cast<unsigned int>(this->connection_type);
  ss << ", \"info\":" << static_cast<unsigned int>(this->info);
  ss << "} },";
  return ss.str();
}

int MavLinkEscStatus::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_int32_t_array(4, buffer, reinterpret_cast<const int32_t*>(&this->rpm[0]),
                     8);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->voltage[0]),
                   24);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->current[0]),
                   40);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->index), 56);
  return 57;
}

int MavLinkEscStatus::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_int32_t_array(4, buffer, reinterpret_cast<int32_t*>(&this->rpm[0]), 8);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->voltage[0]),
                     24);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->current[0]),
                     40);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->index), 56);
  return 57;
}

std::string MavLinkEscStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ESC_STATUS\", \"id\": 291, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"rpm\":"
     << "["
     << int32_t_array_tostring(4, reinterpret_cast<int32_t*>(&this->rpm[0]))
     << "]";
  ss << ", \"voltage\":"
     << "["
     << float_array_tostring(4, reinterpret_cast<float*>(&this->voltage[0]))
     << "]";
  ss << ", \"current\":"
     << "["
     << float_array_tostring(4, reinterpret_cast<float*>(&this->current[0]))
     << "]";
  ss << ", \"index\":" << static_cast<unsigned int>(this->index);
  ss << "} },";
  return ss.str();
}

int MavLinkWifiConfigAp::pack(char* buffer) const {
  pack_char_array(32, buffer, reinterpret_cast<const char*>(&this->ssid[0]), 0);
  pack_char_array(64, buffer, reinterpret_cast<const char*>(&this->password[0]),
                  32);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->mode), 96);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->response), 97);
  return 98;
}

int MavLinkWifiConfigAp::unpack(const char* buffer) {
  unpack_char_array(32, buffer, reinterpret_cast<char*>(&this->ssid[0]), 0);
  unpack_char_array(64, buffer, reinterpret_cast<char*>(&this->password[0]),
                    32);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->mode), 96);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->response), 97);
  return 98;
}

std::string MavLinkWifiConfigAp::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"WIFI_CONFIG_AP\", \"id\": 299, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"ssid\":"
     << "\"" << char_array_tostring(32, reinterpret_cast<char*>(&this->ssid[0]))
     << "\"";
  ss << ", \"password\":"
     << "\""
     << char_array_tostring(64, reinterpret_cast<char*>(&this->password[0]))
     << "\"";
  ss << ", \"mode\":" << static_cast<int>(this->mode);
  ss << ", \"response\":" << static_cast<int>(this->response);
  ss << "} },";
  return ss.str();
}

int MavLinkProtocolVersion::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->version), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->min_version),
                2);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->max_version),
                4);
  pack_uint8_t_array(
      8, buffer, reinterpret_cast<const uint8_t*>(&this->spec_version_hash[0]),
      6);
  pack_uint8_t_array(
      8, buffer,
      reinterpret_cast<const uint8_t*>(&this->library_version_hash[0]), 14);
  return 22;
}

int MavLinkProtocolVersion::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->version), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->min_version), 2);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->max_version), 4);
  unpack_uint8_t_array(
      8, buffer, reinterpret_cast<uint8_t*>(&this->spec_version_hash[0]), 6);
  unpack_uint8_t_array(
      8, buffer, reinterpret_cast<uint8_t*>(&this->library_version_hash[0]),
      14);
  return 22;
}

std::string MavLinkProtocolVersion::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PROTOCOL_VERSION\", \"id\": 300, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"version\":" << this->version;
  ss << ", \"min_version\":" << this->min_version;
  ss << ", \"max_version\":" << this->max_version;
  ss << ", \"spec_version_hash\":"
     << "["
     << uint8_t_array_tostring(
            8, reinterpret_cast<uint8_t*>(&this->spec_version_hash[0]))
     << "]";
  ss << ", \"library_version_hash\":"
     << "["
     << uint8_t_array_tostring(
            8, reinterpret_cast<uint8_t*>(&this->library_version_hash[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkAisVessel::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->MMSI), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 4);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->COG), 12);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->heading), 14);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->velocity), 16);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->dimension_bow),
                18);
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->dimension_stern), 20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->tslc), 22);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->flags), 24);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->turn_rate), 26);
  pack_uint8_t(
      buffer, reinterpret_cast<const uint8_t*>(&this->navigational_status), 27);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->dimension_port),
               29);
  pack_uint8_t(
      buffer, reinterpret_cast<const uint8_t*>(&this->dimension_starboard), 30);
  pack_char_array(7, buffer, reinterpret_cast<const char*>(&this->callsign[0]),
                  31);
  pack_char_array(20, buffer, reinterpret_cast<const char*>(&this->name[0]),
                  38);
  return 58;
}

int MavLinkAisVessel::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->MMSI), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->COG), 12);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->heading), 14);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->velocity), 16);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->dimension_bow),
                  18);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->dimension_stern),
                  20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->tslc), 22);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->flags), 24);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->turn_rate), 26);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->navigational_status),
                 27);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->dimension_port), 29);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->dimension_starboard),
                 30);
  unpack_char_array(7, buffer, reinterpret_cast<char*>(&this->callsign[0]), 31);
  unpack_char_array(20, buffer, reinterpret_cast<char*>(&this->name[0]), 38);
  return 58;
}

std::string MavLinkAisVessel::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"AIS_VESSEL\", \"id\": 301, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"MMSI\":" << this->MMSI;
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"COG\":" << this->COG;
  ss << ", \"heading\":" << this->heading;
  ss << ", \"velocity\":" << this->velocity;
  ss << ", \"dimension_bow\":" << this->dimension_bow;
  ss << ", \"dimension_stern\":" << this->dimension_stern;
  ss << ", \"tslc\":" << this->tslc;
  ss << ", \"flags\":" << this->flags;
  ss << ", \"turn_rate\":" << static_cast<int>(this->turn_rate);
  ss << ", \"navigational_status\":"
     << static_cast<unsigned int>(this->navigational_status);
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"dimension_port\":"
     << static_cast<unsigned int>(this->dimension_port);
  ss << ", \"dimension_starboard\":"
     << static_cast<unsigned int>(this->dimension_starboard);
  ss << ", \"callsign\":"
     << "\""
     << char_array_tostring(7, reinterpret_cast<char*>(&this->callsign[0]))
     << "\"";
  ss << ", \"name\":"
     << "\"" << char_array_tostring(20, reinterpret_cast<char*>(&this->name[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkUavcanNodeStatus::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->uptime_sec),
                8);
  pack_uint16_t(
      buffer,
      reinterpret_cast<const uint16_t*>(&this->vendor_specific_status_code),
      12);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->health), 14);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->mode), 15);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->sub_mode), 16);
  return 17;
}

int MavLinkUavcanNodeStatus::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->uptime_sec), 8);
  unpack_uint16_t(
      buffer, reinterpret_cast<uint16_t*>(&this->vendor_specific_status_code),
      12);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->health), 14);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->mode), 15);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->sub_mode), 16);
  return 17;
}

std::string MavLinkUavcanNodeStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"UAVCAN_NODE_STATUS\", \"id\": 310, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"uptime_sec\":" << this->uptime_sec;
  ss << ", \"vendor_specific_status_code\":"
     << this->vendor_specific_status_code;
  ss << ", \"health\":" << static_cast<unsigned int>(this->health);
  ss << ", \"mode\":" << static_cast<unsigned int>(this->mode);
  ss << ", \"sub_mode\":" << static_cast<unsigned int>(this->sub_mode);
  ss << "} },";
  return ss.str();
}

int MavLinkUavcanNodeInfo::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->uptime_sec),
                8);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->sw_vcs_commit),
                12);
  pack_char_array(80, buffer, reinterpret_cast<const char*>(&this->name[0]),
                  16);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->hw_version_major), 96);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->hw_version_minor), 97);
  pack_uint8_t_array(
      16, buffer, reinterpret_cast<const uint8_t*>(&this->hw_unique_id[0]), 98);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->sw_version_major), 114);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->sw_version_minor), 115);
  return 116;
}

int MavLinkUavcanNodeInfo::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->uptime_sec), 8);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->sw_vcs_commit),
                  12);
  unpack_char_array(80, buffer, reinterpret_cast<char*>(&this->name[0]), 16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->hw_version_major),
                 96);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->hw_version_minor),
                 97);
  unpack_uint8_t_array(16, buffer,
                       reinterpret_cast<uint8_t*>(&this->hw_unique_id[0]), 98);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->sw_version_major),
                 114);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->sw_version_minor),
                 115);
  return 116;
}

std::string MavLinkUavcanNodeInfo::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"UAVCAN_NODE_INFO\", \"id\": 311, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"uptime_sec\":" << this->uptime_sec;
  ss << ", \"sw_vcs_commit\":" << this->sw_vcs_commit;
  ss << ", \"name\":"
     << "\"" << char_array_tostring(80, reinterpret_cast<char*>(&this->name[0]))
     << "\"";
  ss << ", \"hw_version_major\":"
     << static_cast<unsigned int>(this->hw_version_major);
  ss << ", \"hw_version_minor\":"
     << static_cast<unsigned int>(this->hw_version_minor);
  ss << ", \"hw_unique_id\":"
     << "["
     << uint8_t_array_tostring(
            16, reinterpret_cast<uint8_t*>(&this->hw_unique_id[0]))
     << "]";
  ss << ", \"sw_version_major\":"
     << static_cast<unsigned int>(this->sw_version_major);
  ss << ", \"sw_version_minor\":"
     << static_cast<unsigned int>(this->sw_version_minor);
  ss << "} },";
  return ss.str();
}

int MavLinkParamExtRequestRead::pack(char* buffer) const {
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->param_index), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 3);
  pack_char_array(16, buffer, reinterpret_cast<const char*>(&this->param_id[0]),
                  4);
  return 20;
}

int MavLinkParamExtRequestRead::unpack(const char* buffer) {
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->param_index), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 3);
  unpack_char_array(16, buffer, reinterpret_cast<char*>(&this->param_id[0]), 4);
  return 20;
}

std::string MavLinkParamExtRequestRead::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PARAM_EXT_REQUEST_READ\", \"id\": 320, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"param_index\":" << this->param_index;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"param_id\":"
     << "\""
     << char_array_tostring(16, reinterpret_cast<char*>(&this->param_id[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkParamExtRequestList::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  return 2;
}

int MavLinkParamExtRequestList::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  return 2;
}

std::string MavLinkParamExtRequestList::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PARAM_EXT_REQUEST_LIST\", \"id\": 321, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkParamExtValue::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->param_count),
                0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->param_index),
                2);
  pack_char_array(16, buffer, reinterpret_cast<const char*>(&this->param_id[0]),
                  4);
  pack_char_array(128, buffer,
                  reinterpret_cast<const char*>(&this->param_value[0]), 20);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->param_type),
               148);
  return 149;
}

int MavLinkParamExtValue::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->param_count), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->param_index), 2);
  unpack_char_array(16, buffer, reinterpret_cast<char*>(&this->param_id[0]), 4);
  unpack_char_array(128, buffer, reinterpret_cast<char*>(&this->param_value[0]),
                    20);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->param_type), 148);
  return 149;
}

std::string MavLinkParamExtValue::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PARAM_EXT_VALUE\", \"id\": 322, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"param_count\":" << this->param_count;
  ss << ", \"param_index\":" << this->param_index;
  ss << ", \"param_id\":"
     << "\""
     << char_array_tostring(16, reinterpret_cast<char*>(&this->param_id[0]))
     << "\"";
  ss << ", \"param_value\":"
     << "\""
     << char_array_tostring(128, reinterpret_cast<char*>(&this->param_value[0]))
     << "\"";
  ss << ", \"param_type\":" << static_cast<unsigned int>(this->param_type);
  ss << "} },";
  return ss.str();
}

int MavLinkParamExtSet::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  pack_char_array(16, buffer, reinterpret_cast<const char*>(&this->param_id[0]),
                  2);
  pack_char_array(128, buffer,
                  reinterpret_cast<const char*>(&this->param_value[0]), 18);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->param_type),
               146);
  return 147;
}

int MavLinkParamExtSet::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  unpack_char_array(16, buffer, reinterpret_cast<char*>(&this->param_id[0]), 2);
  unpack_char_array(128, buffer, reinterpret_cast<char*>(&this->param_value[0]),
                    18);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->param_type), 146);
  return 147;
}

std::string MavLinkParamExtSet::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PARAM_EXT_SET\", \"id\": 323, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"param_id\":"
     << "\""
     << char_array_tostring(16, reinterpret_cast<char*>(&this->param_id[0]))
     << "\"";
  ss << ", \"param_value\":"
     << "\""
     << char_array_tostring(128, reinterpret_cast<char*>(&this->param_value[0]))
     << "\"";
  ss << ", \"param_type\":" << static_cast<unsigned int>(this->param_type);
  ss << "} },";
  return ss.str();
}

int MavLinkParamExtAck::pack(char* buffer) const {
  pack_char_array(16, buffer, reinterpret_cast<const char*>(&this->param_id[0]),
                  0);
  pack_char_array(128, buffer,
                  reinterpret_cast<const char*>(&this->param_value[0]), 16);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->param_type),
               144);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->param_result),
               145);
  return 146;
}

int MavLinkParamExtAck::unpack(const char* buffer) {
  unpack_char_array(16, buffer, reinterpret_cast<char*>(&this->param_id[0]), 0);
  unpack_char_array(128, buffer, reinterpret_cast<char*>(&this->param_value[0]),
                    16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->param_type), 144);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->param_result), 145);
  return 146;
}

std::string MavLinkParamExtAck::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PARAM_EXT_ACK\", \"id\": 324, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"param_id\":"
     << "\""
     << char_array_tostring(16, reinterpret_cast<char*>(&this->param_id[0]))
     << "\"";
  ss << ", \"param_value\":"
     << "\""
     << char_array_tostring(128, reinterpret_cast<char*>(&this->param_value[0]))
     << "\"";
  ss << ", \"param_type\":" << static_cast<unsigned int>(this->param_type);
  ss << ", \"param_result\":" << static_cast<unsigned int>(this->param_result);
  ss << "} },";
  return ss.str();
}

int MavLinkObstacleDistance::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint16_t_array(
      72, buffer, reinterpret_cast<const uint16_t*>(&this->distances[0]), 8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->min_distance),
                152);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->max_distance),
                154);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->sensor_type),
               156);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->increment), 157);
  pack_float(buffer, reinterpret_cast<const float*>(&this->increment_f), 158);
  pack_float(buffer, reinterpret_cast<const float*>(&this->angle_offset), 162);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->frame), 166);
  return 167;
}

int MavLinkObstacleDistance::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint16_t_array(72, buffer,
                        reinterpret_cast<uint16_t*>(&this->distances[0]), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->min_distance),
                  152);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->max_distance),
                  154);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->sensor_type), 156);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->increment), 157);
  unpack_float(buffer, reinterpret_cast<float*>(&this->increment_f), 158);
  unpack_float(buffer, reinterpret_cast<float*>(&this->angle_offset), 162);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->frame), 166);
  return 167;
}

std::string MavLinkObstacleDistance::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"OBSTACLE_DISTANCE\", \"id\": 330, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"distances\":"
     << "["
     << uint16_t_array_tostring(
            72, reinterpret_cast<uint16_t*>(&this->distances[0]))
     << "]";
  ss << ", \"min_distance\":" << this->min_distance;
  ss << ", \"max_distance\":" << this->max_distance;
  ss << ", \"sensor_type\":" << static_cast<unsigned int>(this->sensor_type);
  ss << ", \"increment\":" << static_cast<unsigned int>(this->increment);
  ss << ", \"increment_f\":" << float_tostring(this->increment_f);
  ss << ", \"angle_offset\":" << float_tostring(this->angle_offset);
  ss << ", \"frame\":" << static_cast<unsigned int>(this->frame);
  ss << "} },";
  return ss.str();
}

int MavLinkOdometry::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->x), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->y), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 16);
  pack_float_array(4, buffer, reinterpret_cast<const float*>(&this->q[0]), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vx), 36);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vy), 40);
  pack_float(buffer, reinterpret_cast<const float*>(&this->vz), 44);
  pack_float(buffer, reinterpret_cast<const float*>(&this->rollspeed), 48);
  pack_float(buffer, reinterpret_cast<const float*>(&this->pitchspeed), 52);
  pack_float(buffer, reinterpret_cast<const float*>(&this->yawspeed), 56);
  pack_float_array(21, buffer,
                   reinterpret_cast<const float*>(&this->pose_covariance[0]),
                   60);
  pack_float_array(
      21, buffer, reinterpret_cast<const float*>(&this->velocity_covariance[0]),
      144);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->frame_id), 228);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->child_frame_id),
               229);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->reset_counter),
               230);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->estimator_type),
               231);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->quality), 232);
  return 233;
}

int MavLinkOdometry::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->x), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->y), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 16);
  unpack_float_array(4, buffer, reinterpret_cast<float*>(&this->q[0]), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vx), 36);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vy), 40);
  unpack_float(buffer, reinterpret_cast<float*>(&this->vz), 44);
  unpack_float(buffer, reinterpret_cast<float*>(&this->rollspeed), 48);
  unpack_float(buffer, reinterpret_cast<float*>(&this->pitchspeed), 52);
  unpack_float(buffer, reinterpret_cast<float*>(&this->yawspeed), 56);
  unpack_float_array(21, buffer,
                     reinterpret_cast<float*>(&this->pose_covariance[0]), 60);
  unpack_float_array(
      21, buffer, reinterpret_cast<float*>(&this->velocity_covariance[0]), 144);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->frame_id), 228);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->child_frame_id),
                 229);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->reset_counter), 230);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->estimator_type),
                 231);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->quality), 232);
  return 233;
}

std::string MavLinkOdometry::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ODOMETRY\", \"id\": 331, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"x\":" << float_tostring(this->x);
  ss << ", \"y\":" << float_tostring(this->y);
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"q\":"
     << "[" << float_array_tostring(4, reinterpret_cast<float*>(&this->q[0]))
     << "]";
  ss << ", \"vx\":" << float_tostring(this->vx);
  ss << ", \"vy\":" << float_tostring(this->vy);
  ss << ", \"vz\":" << float_tostring(this->vz);
  ss << ", \"rollspeed\":" << float_tostring(this->rollspeed);
  ss << ", \"pitchspeed\":" << float_tostring(this->pitchspeed);
  ss << ", \"yawspeed\":" << float_tostring(this->yawspeed);
  ss << ", \"pose_covariance\":"
     << "["
     << float_array_tostring(
            21, reinterpret_cast<float*>(&this->pose_covariance[0]))
     << "]";
  ss << ", \"velocity_covariance\":"
     << "["
     << float_array_tostring(
            21, reinterpret_cast<float*>(&this->velocity_covariance[0]))
     << "]";
  ss << ", \"frame_id\":" << static_cast<unsigned int>(this->frame_id);
  ss << ", \"child_frame_id\":"
     << static_cast<unsigned int>(this->child_frame_id);
  ss << ", \"reset_counter\":"
     << static_cast<unsigned int>(this->reset_counter);
  ss << ", \"estimator_type\":"
     << static_cast<unsigned int>(this->estimator_type);
  ss << ", \"quality\":" << static_cast<int>(this->quality);
  ss << "} },";
  return ss.str();
}

int MavLinkTrajectoryRepresentationWaypoints::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->pos_x[0]),
                   8);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->pos_y[0]),
                   28);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->pos_z[0]),
                   48);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->vel_x[0]),
                   68);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->vel_y[0]),
                   88);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->vel_z[0]),
                   108);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->acc_x[0]),
                   128);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->acc_y[0]),
                   148);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->acc_z[0]),
                   168);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->pos_yaw[0]),
                   188);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->vel_yaw[0]),
                   208);
  pack_uint16_t_array(
      5, buffer, reinterpret_cast<const uint16_t*>(&this->command[0]), 228);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->valid_points),
               238);
  return 239;
}

int MavLinkTrajectoryRepresentationWaypoints::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->pos_x[0]), 8);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->pos_y[0]), 28);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->pos_z[0]), 48);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->vel_x[0]), 68);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->vel_y[0]), 88);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->vel_z[0]), 108);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->acc_x[0]), 128);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->acc_y[0]), 148);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->acc_z[0]), 168);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->pos_yaw[0]),
                     188);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->vel_yaw[0]),
                     208);
  unpack_uint16_t_array(5, buffer,
                        reinterpret_cast<uint16_t*>(&this->command[0]), 228);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->valid_points), 238);
  return 239;
}

std::string MavLinkTrajectoryRepresentationWaypoints::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"TRAJECTORY_REPRESENTATION_WAYPOINTS\", \"id\": 332, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"pos_x\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->pos_x[0]))
     << "]";
  ss << ", \"pos_y\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->pos_y[0]))
     << "]";
  ss << ", \"pos_z\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->pos_z[0]))
     << "]";
  ss << ", \"vel_x\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->vel_x[0]))
     << "]";
  ss << ", \"vel_y\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->vel_y[0]))
     << "]";
  ss << ", \"vel_z\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->vel_z[0]))
     << "]";
  ss << ", \"acc_x\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->acc_x[0]))
     << "]";
  ss << ", \"acc_y\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->acc_y[0]))
     << "]";
  ss << ", \"acc_z\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->acc_z[0]))
     << "]";
  ss << ", \"pos_yaw\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->pos_yaw[0]))
     << "]";
  ss << ", \"vel_yaw\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->vel_yaw[0]))
     << "]";
  ss << ", \"command\":"
     << "["
     << uint16_t_array_tostring(5,
                                reinterpret_cast<uint16_t*>(&this->command[0]))
     << "]";
  ss << ", \"valid_points\":" << static_cast<unsigned int>(this->valid_points);
  ss << "} },";
  return ss.str();
}

int MavLinkTrajectoryRepresentationBezier::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->pos_x[0]),
                   8);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->pos_y[0]),
                   28);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->pos_z[0]),
                   48);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->delta[0]),
                   68);
  pack_float_array(5, buffer, reinterpret_cast<const float*>(&this->pos_yaw[0]),
                   88);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->valid_points),
               108);
  return 109;
}

int MavLinkTrajectoryRepresentationBezier::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->pos_x[0]), 8);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->pos_y[0]), 28);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->pos_z[0]), 48);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->delta[0]), 68);
  unpack_float_array(5, buffer, reinterpret_cast<float*>(&this->pos_yaw[0]),
                     88);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->valid_points), 108);
  return 109;
}

std::string MavLinkTrajectoryRepresentationBezier::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"TRAJECTORY_REPRESENTATION_BEZIER\", \"id\": 333, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"pos_x\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->pos_x[0]))
     << "]";
  ss << ", \"pos_y\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->pos_y[0]))
     << "]";
  ss << ", \"pos_z\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->pos_z[0]))
     << "]";
  ss << ", \"delta\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->delta[0]))
     << "]";
  ss << ", \"pos_yaw\":"
     << "["
     << float_array_tostring(5, reinterpret_cast<float*>(&this->pos_yaw[0]))
     << "]";
  ss << ", \"valid_points\":" << static_cast<unsigned int>(this->valid_points);
  ss << "} },";
  return ss.str();
}

int MavLinkCellularStatus::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->mcc), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->mnc), 2);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->lac), 4);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->status), 6);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->failure_reason),
               7);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 8);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->quality), 9);
  return 10;
}

int MavLinkCellularStatus::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->mcc), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->mnc), 2);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->lac), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->status), 6);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->failure_reason), 7);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->quality), 9);
  return 10;
}

std::string MavLinkCellularStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CELLULAR_STATUS\", \"id\": 334, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"mcc\":" << this->mcc;
  ss << ", \"mnc\":" << this->mnc;
  ss << ", \"lac\":" << this->lac;
  ss << ", \"status\":" << static_cast<unsigned int>(this->status);
  ss << ", \"failure_reason\":"
     << static_cast<unsigned int>(this->failure_reason);
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"quality\":" << static_cast<unsigned int>(this->quality);
  ss << "} },";
  return ss.str();
}

int MavLinkIsbdLinkStatus::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->timestamp), 0);
  pack_uint64_t(buffer,
                reinterpret_cast<const uint64_t*>(&this->last_heartbeat), 8);
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->failed_sessions), 16);
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->successful_sessions),
                18);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->signal_quality),
               20);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->ring_pending),
               21);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->tx_session_pending), 22);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->rx_session_pending), 23);
  return 24;
}

int MavLinkIsbdLinkStatus::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->timestamp), 0);
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->last_heartbeat),
                  8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->failed_sessions),
                  16);
  unpack_uint16_t(buffer,
                  reinterpret_cast<uint16_t*>(&this->successful_sessions), 18);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->signal_quality), 20);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->ring_pending), 21);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->tx_session_pending),
                 22);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->rx_session_pending),
                 23);
  return 24;
}

std::string MavLinkIsbdLinkStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ISBD_LINK_STATUS\", \"id\": 335, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"timestamp\":" << this->timestamp;
  ss << ", \"last_heartbeat\":" << this->last_heartbeat;
  ss << ", \"failed_sessions\":" << this->failed_sessions;
  ss << ", \"successful_sessions\":" << this->successful_sessions;
  ss << ", \"signal_quality\":"
     << static_cast<unsigned int>(this->signal_quality);
  ss << ", \"ring_pending\":" << static_cast<unsigned int>(this->ring_pending);
  ss << ", \"tx_session_pending\":"
     << static_cast<unsigned int>(this->tx_session_pending);
  ss << ", \"rx_session_pending\":"
     << static_cast<unsigned int>(this->rx_session_pending);
  ss << "} },";
  return ss.str();
}

int MavLinkCellularConfig::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->enable_lte), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->enable_pin), 1);
  pack_char_array(16, buffer, reinterpret_cast<const char*>(&this->pin[0]), 2);
  pack_char_array(16, buffer, reinterpret_cast<const char*>(&this->new_pin[0]),
                  18);
  pack_char_array(32, buffer, reinterpret_cast<const char*>(&this->apn[0]), 34);
  pack_char_array(16, buffer, reinterpret_cast<const char*>(&this->puk[0]), 66);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->roaming), 82);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->response), 83);
  return 84;
}

int MavLinkCellularConfig::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->enable_lte), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->enable_pin), 1);
  unpack_char_array(16, buffer, reinterpret_cast<char*>(&this->pin[0]), 2);
  unpack_char_array(16, buffer, reinterpret_cast<char*>(&this->new_pin[0]), 18);
  unpack_char_array(32, buffer, reinterpret_cast<char*>(&this->apn[0]), 34);
  unpack_char_array(16, buffer, reinterpret_cast<char*>(&this->puk[0]), 66);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->roaming), 82);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->response), 83);
  return 84;
}

std::string MavLinkCellularConfig::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CELLULAR_CONFIG\", \"id\": 336, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"enable_lte\":" << static_cast<unsigned int>(this->enable_lte);
  ss << ", \"enable_pin\":" << static_cast<unsigned int>(this->enable_pin);
  ss << ", \"pin\":"
     << "\"" << char_array_tostring(16, reinterpret_cast<char*>(&this->pin[0]))
     << "\"";
  ss << ", \"new_pin\":"
     << "\""
     << char_array_tostring(16, reinterpret_cast<char*>(&this->new_pin[0]))
     << "\"";
  ss << ", \"apn\":"
     << "\"" << char_array_tostring(32, reinterpret_cast<char*>(&this->apn[0]))
     << "\"";
  ss << ", \"puk\":"
     << "\"" << char_array_tostring(16, reinterpret_cast<char*>(&this->puk[0]))
     << "\"";
  ss << ", \"roaming\":" << static_cast<unsigned int>(this->roaming);
  ss << ", \"response\":" << static_cast<unsigned int>(this->response);
  ss << "} },";
  return ss.str();
}

int MavLinkRawRpm::pack(char* buffer) const {
  pack_float(buffer, reinterpret_cast<const float*>(&this->frequency), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->index), 4);
  return 5;
}

int MavLinkRawRpm::unpack(const char* buffer) {
  unpack_float(buffer, reinterpret_cast<float*>(&this->frequency), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->index), 4);
  return 5;
}

std::string MavLinkRawRpm::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"RAW_RPM\", \"id\": 339, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"frequency\":" << float_tostring(this->frequency);
  ss << ", \"index\":" << static_cast<unsigned int>(this->index);
  ss << "} },";
  return ss.str();
}

int MavLinkUtmGlobalPosition::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lat), 8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->lon), 12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->alt), 16);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->relative_alt),
               20);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->next_lat), 24);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->next_lon), 28);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->next_alt), 32);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vx), 36);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vy), 38);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->vz), 40);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->h_acc), 42);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->v_acc), 44);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->vel_acc), 46);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->update_rate),
                48);
  pack_uint8_t_array(18, buffer,
                     reinterpret_cast<const uint8_t*>(&this->uas_id[0]), 50);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->flight_state),
               68);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->flags), 69);
  return 70;
}

int MavLinkUtmGlobalPosition::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lat), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->lon), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->alt), 16);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->relative_alt), 20);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->next_lat), 24);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->next_lon), 28);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->next_alt), 32);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vx), 36);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vy), 38);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->vz), 40);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->h_acc), 42);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->v_acc), 44);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->vel_acc), 46);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->update_rate), 48);
  unpack_uint8_t_array(18, buffer, reinterpret_cast<uint8_t*>(&this->uas_id[0]),
                       50);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->flight_state), 68);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->flags), 69);
  return 70;
}

std::string MavLinkUtmGlobalPosition::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"UTM_GLOBAL_POSITION\", \"id\": 340, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time\":" << this->time;
  ss << ", \"lat\":" << this->lat;
  ss << ", \"lon\":" << this->lon;
  ss << ", \"alt\":" << this->alt;
  ss << ", \"relative_alt\":" << this->relative_alt;
  ss << ", \"next_lat\":" << this->next_lat;
  ss << ", \"next_lon\":" << this->next_lon;
  ss << ", \"next_alt\":" << this->next_alt;
  ss << ", \"vx\":" << this->vx;
  ss << ", \"vy\":" << this->vy;
  ss << ", \"vz\":" << this->vz;
  ss << ", \"h_acc\":" << this->h_acc;
  ss << ", \"v_acc\":" << this->v_acc;
  ss << ", \"vel_acc\":" << this->vel_acc;
  ss << ", \"update_rate\":" << this->update_rate;
  ss << ", \"uas_id\":"
     << "["
     << uint8_t_array_tostring(18, reinterpret_cast<uint8_t*>(&this->uas_id[0]))
     << "]";
  ss << ", \"flight_state\":" << static_cast<unsigned int>(this->flight_state);
  ss << ", \"flags\":" << static_cast<unsigned int>(this->flags);
  ss << "} },";
  return ss.str();
}

int MavLinkDebugFloatArray::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->array_id), 8);
  pack_char_array(10, buffer, reinterpret_cast<const char*>(&this->name[0]),
                  10);
  pack_float_array(58, buffer, reinterpret_cast<const float*>(&this->data[0]),
                   20);
  return 252;
}

int MavLinkDebugFloatArray::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->array_id), 8);
  unpack_char_array(10, buffer, reinterpret_cast<char*>(&this->name[0]), 10);
  unpack_float_array(58, buffer, reinterpret_cast<float*>(&this->data[0]), 20);
  return 252;
}

std::string MavLinkDebugFloatArray::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"DEBUG_FLOAT_ARRAY\", \"id\": 350, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"array_id\":" << this->array_id;
  ss << ", \"name\":"
     << "\"" << char_array_tostring(10, reinterpret_cast<char*>(&this->name[0]))
     << "\"";
  ss << ", \"data\":"
     << "["
     << float_array_tostring(58, reinterpret_cast<float*>(&this->data[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkOrbitExecutionStatus::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->radius), 8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->x), 12);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->y), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->z), 20);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->frame), 24);
  return 25;
}

int MavLinkOrbitExecutionStatus::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->radius), 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->x), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->y), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->z), 20);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->frame), 24);
  return 25;
}

std::string MavLinkOrbitExecutionStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ORBIT_EXECUTION_STATUS\", \"id\": 360, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"radius\":" << float_tostring(this->radius);
  ss << ", \"x\":" << this->x;
  ss << ", \"y\":" << this->y;
  ss << ", \"z\":" << float_tostring(this->z);
  ss << ", \"frame\":" << static_cast<unsigned int>(this->frame);
  ss << "} },";
  return ss.str();
}

int MavLinkSmartBatteryInfo::pack(char* buffer) const {
  pack_int32_t(
      buffer,
      reinterpret_cast<const int32_t*>(&this->capacity_full_specification), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->capacity_full),
               4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->cycle_count),
                8);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->weight), 10);
  pack_uint16_t(
      buffer,
      reinterpret_cast<const uint16_t*>(&this->discharge_minimum_voltage), 12);
  pack_uint16_t(
      buffer,
      reinterpret_cast<const uint16_t*>(&this->charging_minimum_voltage), 14);
  pack_uint16_t(
      buffer, reinterpret_cast<const uint16_t*>(&this->resting_minimum_voltage),
      16);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->id), 18);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->battery_function), 19);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 20);
  pack_char_array(16, buffer,
                  reinterpret_cast<const char*>(&this->serial_number[0]), 21);
  pack_char_array(50, buffer,
                  reinterpret_cast<const char*>(&this->device_name[0]), 37);
  pack_uint16_t(
      buffer,
      reinterpret_cast<const uint16_t*>(&this->charging_maximum_voltage), 87);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->cells_in_series),
               89);
  pack_uint32_t(
      buffer,
      reinterpret_cast<const uint32_t*>(&this->discharge_maximum_current), 90);
  pack_uint32_t(
      buffer,
      reinterpret_cast<const uint32_t*>(&this->discharge_maximum_burst_current),
      94);
  pack_char_array(11, buffer,
                  reinterpret_cast<const char*>(&this->manufacture_date[0]),
                  98);
  return 109;
}

int MavLinkSmartBatteryInfo::unpack(const char* buffer) {
  unpack_int32_t(buffer,
                 reinterpret_cast<int32_t*>(&this->capacity_full_specification),
                 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->capacity_full), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->cycle_count), 8);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->weight), 10);
  unpack_uint16_t(buffer,
                  reinterpret_cast<uint16_t*>(&this->discharge_minimum_voltage),
                  12);
  unpack_uint16_t(
      buffer, reinterpret_cast<uint16_t*>(&this->charging_minimum_voltage), 14);
  unpack_uint16_t(
      buffer, reinterpret_cast<uint16_t*>(&this->resting_minimum_voltage), 16);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->id), 18);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->battery_function),
                 19);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 20);
  unpack_char_array(16, buffer,
                    reinterpret_cast<char*>(&this->serial_number[0]), 21);
  unpack_char_array(50, buffer, reinterpret_cast<char*>(&this->device_name[0]),
                    37);
  unpack_uint16_t(
      buffer, reinterpret_cast<uint16_t*>(&this->charging_maximum_voltage), 87);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->cells_in_series),
                 89);
  unpack_uint32_t(buffer,
                  reinterpret_cast<uint32_t*>(&this->discharge_maximum_current),
                  90);
  unpack_uint32_t(
      buffer,
      reinterpret_cast<uint32_t*>(&this->discharge_maximum_burst_current), 94);
  unpack_char_array(11, buffer,
                    reinterpret_cast<char*>(&this->manufacture_date[0]), 98);
  return 109;
}

std::string MavLinkSmartBatteryInfo::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SMART_BATTERY_INFO\", \"id\": 370, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"capacity_full_specification\":" << this->capacity_full_specification;
  ss << ", \"capacity_full\":" << this->capacity_full;
  ss << ", \"cycle_count\":" << this->cycle_count;
  ss << ", \"weight\":" << this->weight;
  ss << ", \"discharge_minimum_voltage\":" << this->discharge_minimum_voltage;
  ss << ", \"charging_minimum_voltage\":" << this->charging_minimum_voltage;
  ss << ", \"resting_minimum_voltage\":" << this->resting_minimum_voltage;
  ss << ", \"id\":" << static_cast<unsigned int>(this->id);
  ss << ", \"battery_function\":"
     << static_cast<unsigned int>(this->battery_function);
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"serial_number\":"
     << "\""
     << char_array_tostring(16,
                            reinterpret_cast<char*>(&this->serial_number[0]))
     << "\"";
  ss << ", \"device_name\":"
     << "\""
     << char_array_tostring(50, reinterpret_cast<char*>(&this->device_name[0]))
     << "\"";
  ss << ", \"charging_maximum_voltage\":" << this->charging_maximum_voltage;
  ss << ", \"cells_in_series\":"
     << static_cast<unsigned int>(this->cells_in_series);
  ss << ", \"discharge_maximum_current\":" << this->discharge_maximum_current;
  ss << ", \"discharge_maximum_burst_current\":"
     << this->discharge_maximum_burst_current;
  ss << ", \"manufacture_date\":"
     << "\""
     << char_array_tostring(11,
                            reinterpret_cast<char*>(&this->manufacture_date[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkGeneratorStatus::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->status), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->battery_current), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->load_current), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->power_generated),
             16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->bus_voltage), 20);
  pack_float(buffer,
             reinterpret_cast<const float*>(&this->bat_current_setpoint), 24);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->runtime), 28);
  pack_int32_t(buffer,
               reinterpret_cast<const int32_t*>(&this->time_until_maintenance),
               32);
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->generator_speed), 36);
  pack_int16_t(buffer,
               reinterpret_cast<const int16_t*>(&this->rectifier_temperature),
               38);
  pack_int16_t(buffer,
               reinterpret_cast<const int16_t*>(&this->generator_temperature),
               40);
  return 42;
}

int MavLinkGeneratorStatus::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->status), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->battery_current), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->load_current), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->power_generated), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->bus_voltage), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->bat_current_setpoint),
               24);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->runtime), 28);
  unpack_int32_t(buffer,
                 reinterpret_cast<int32_t*>(&this->time_until_maintenance), 32);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->generator_speed),
                  36);
  unpack_int16_t(buffer,
                 reinterpret_cast<int16_t*>(&this->rectifier_temperature), 38);
  unpack_int16_t(buffer,
                 reinterpret_cast<int16_t*>(&this->generator_temperature), 40);
  return 42;
}

std::string MavLinkGeneratorStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"GENERATOR_STATUS\", \"id\": 373, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"status\":" << this->status;
  ss << ", \"battery_current\":" << float_tostring(this->battery_current);
  ss << ", \"load_current\":" << float_tostring(this->load_current);
  ss << ", \"power_generated\":" << float_tostring(this->power_generated);
  ss << ", \"bus_voltage\":" << float_tostring(this->bus_voltage);
  ss << ", \"bat_current_setpoint\":"
     << float_tostring(this->bat_current_setpoint);
  ss << ", \"runtime\":" << this->runtime;
  ss << ", \"time_until_maintenance\":" << this->time_until_maintenance;
  ss << ", \"generator_speed\":" << this->generator_speed;
  ss << ", \"rectifier_temperature\":" << this->rectifier_temperature;
  ss << ", \"generator_temperature\":" << this->generator_temperature;
  ss << "} },";
  return ss.str();
}

int MavLinkActuatorOutputStatus::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->active), 8);
  pack_float_array(32, buffer,
                   reinterpret_cast<const float*>(&this->actuator[0]), 12);
  return 140;
}

int MavLinkActuatorOutputStatus::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->active), 8);
  unpack_float_array(32, buffer, reinterpret_cast<float*>(&this->actuator[0]),
                     12);
  return 140;
}

std::string MavLinkActuatorOutputStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ACTUATOR_OUTPUT_STATUS\", \"id\": 375, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"active\":" << this->active;
  ss << ", \"actuator\":"
     << "["
     << float_array_tostring(32, reinterpret_cast<float*>(&this->actuator[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkTimeEstimateToTarget::pack(char* buffer) const {
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->safe_return), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->land), 4);
  pack_int32_t(buffer,
               reinterpret_cast<const int32_t*>(&this->mission_next_item), 8);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->mission_end),
               12);
  pack_int32_t(buffer,
               reinterpret_cast<const int32_t*>(&this->commanded_action), 16);
  return 20;
}

int MavLinkTimeEstimateToTarget::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->safe_return), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->land), 4);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->mission_next_item),
                 8);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->mission_end), 12);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->commanded_action),
                 16);
  return 20;
}

std::string MavLinkTimeEstimateToTarget::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"TIME_ESTIMATE_TO_TARGET\", \"id\": 380, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"safe_return\":" << this->safe_return;
  ss << ", \"land\":" << this->land;
  ss << ", \"mission_next_item\":" << this->mission_next_item;
  ss << ", \"mission_end\":" << this->mission_end;
  ss << ", \"commanded_action\":" << this->commanded_action;
  ss << "} },";
  return ss.str();
}

int MavLinkTunnel::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->payload_type),
                0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 3);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->payload_length),
               4);
  pack_uint8_t_array(128, buffer,
                     reinterpret_cast<const uint8_t*>(&this->payload[0]), 5);
  return 133;
}

int MavLinkTunnel::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->payload_type), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 3);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->payload_length), 4);
  unpack_uint8_t_array(128, buffer,
                       reinterpret_cast<uint8_t*>(&this->payload[0]), 5);
  return 133;
}

std::string MavLinkTunnel::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"TUNNEL\", \"id\": 385, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"payload_type\":" << this->payload_type;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"payload_length\":"
     << static_cast<unsigned int>(this->payload_length);
  ss << ", \"payload\":"
     << "["
     << uint8_t_array_tostring(128,
                               reinterpret_cast<uint8_t*>(&this->payload[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkCanFrame::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->id), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               4);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 5);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->bus), 6);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->len), 7);
  pack_uint8_t_array(8, buffer,
                     reinterpret_cast<const uint8_t*>(&this->data[0]), 8);
  return 16;
}

int MavLinkCanFrame::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->id), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 5);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->bus), 6);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->len), 7);
  unpack_uint8_t_array(8, buffer, reinterpret_cast<uint8_t*>(&this->data[0]),
                       8);
  return 16;
}

std::string MavLinkCanFrame::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CAN_FRAME\", \"id\": 386, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"id\":" << this->id;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"bus\":" << static_cast<unsigned int>(this->bus);
  ss << ", \"len\":" << static_cast<unsigned int>(this->len);
  ss << ", \"data\":"
     << "["
     << uint8_t_array_tostring(8, reinterpret_cast<uint8_t*>(&this->data[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkOnboardComputerStatus::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->uptime), 8);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->ram_usage),
                12);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->ram_total),
                16);
  pack_uint32_t_array(
      4, buffer, reinterpret_cast<const uint32_t*>(&this->storage_type[0]), 20);
  pack_uint32_t_array(
      4, buffer, reinterpret_cast<const uint32_t*>(&this->storage_usage[0]),
      36);
  pack_uint32_t_array(
      4, buffer, reinterpret_cast<const uint32_t*>(&this->storage_total[0]),
      52);
  pack_uint32_t_array(
      6, buffer, reinterpret_cast<const uint32_t*>(&this->link_type[0]), 68);
  pack_uint32_t_array(
      6, buffer, reinterpret_cast<const uint32_t*>(&this->link_tx_rate[0]), 92);
  pack_uint32_t_array(6, buffer,
                      reinterpret_cast<const uint32_t*>(&this->link_rx_rate[0]),
                      116);
  pack_uint32_t_array(
      6, buffer, reinterpret_cast<const uint32_t*>(&this->link_tx_max[0]), 140);
  pack_uint32_t_array(
      6, buffer, reinterpret_cast<const uint32_t*>(&this->link_rx_max[0]), 164);
  pack_int16_t_array(
      4, buffer, reinterpret_cast<const int16_t*>(&this->fan_speed[0]), 188);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->type), 196);
  pack_uint8_t_array(
      8, buffer, reinterpret_cast<const uint8_t*>(&this->cpu_cores[0]), 197);
  pack_uint8_t_array(10, buffer,
                     reinterpret_cast<const uint8_t*>(&this->cpu_combined[0]),
                     205);
  pack_uint8_t_array(
      4, buffer, reinterpret_cast<const uint8_t*>(&this->gpu_cores[0]), 215);
  pack_uint8_t_array(10, buffer,
                     reinterpret_cast<const uint8_t*>(&this->gpu_combined[0]),
                     219);
  pack_int8_t(buffer, reinterpret_cast<const int8_t*>(&this->temperature_board),
              229);
  pack_int8_t_array(8, buffer,
                    reinterpret_cast<const int8_t*>(&this->temperature_core[0]),
                    230);
  return 238;
}

int MavLinkOnboardComputerStatus::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->uptime), 8);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->ram_usage), 12);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->ram_total), 16);
  unpack_uint32_t_array(
      4, buffer, reinterpret_cast<uint32_t*>(&this->storage_type[0]), 20);
  unpack_uint32_t_array(
      4, buffer, reinterpret_cast<uint32_t*>(&this->storage_usage[0]), 36);
  unpack_uint32_t_array(
      4, buffer, reinterpret_cast<uint32_t*>(&this->storage_total[0]), 52);
  unpack_uint32_t_array(6, buffer,
                        reinterpret_cast<uint32_t*>(&this->link_type[0]), 68);
  unpack_uint32_t_array(
      6, buffer, reinterpret_cast<uint32_t*>(&this->link_tx_rate[0]), 92);
  unpack_uint32_t_array(
      6, buffer, reinterpret_cast<uint32_t*>(&this->link_rx_rate[0]), 116);
  unpack_uint32_t_array(
      6, buffer, reinterpret_cast<uint32_t*>(&this->link_tx_max[0]), 140);
  unpack_uint32_t_array(
      6, buffer, reinterpret_cast<uint32_t*>(&this->link_rx_max[0]), 164);
  unpack_int16_t_array(4, buffer,
                       reinterpret_cast<int16_t*>(&this->fan_speed[0]), 188);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->type), 196);
  unpack_uint8_t_array(8, buffer,
                       reinterpret_cast<uint8_t*>(&this->cpu_cores[0]), 197);
  unpack_uint8_t_array(10, buffer,
                       reinterpret_cast<uint8_t*>(&this->cpu_combined[0]), 205);
  unpack_uint8_t_array(4, buffer,
                       reinterpret_cast<uint8_t*>(&this->gpu_cores[0]), 215);
  unpack_uint8_t_array(10, buffer,
                       reinterpret_cast<uint8_t*>(&this->gpu_combined[0]), 219);
  unpack_int8_t(buffer, reinterpret_cast<int8_t*>(&this->temperature_board),
                229);
  unpack_int8_t_array(
      8, buffer, reinterpret_cast<int8_t*>(&this->temperature_core[0]), 230);
  return 238;
}

std::string MavLinkOnboardComputerStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"ONBOARD_COMPUTER_STATUS\", \"id\": 390, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"uptime\":" << this->uptime;
  ss << ", \"ram_usage\":" << this->ram_usage;
  ss << ", \"ram_total\":" << this->ram_total;
  ss << ", \"storage_type\":"
     << "["
     << uint32_t_array_tostring(
            4, reinterpret_cast<uint32_t*>(&this->storage_type[0]))
     << "]";
  ss << ", \"storage_usage\":"
     << "["
     << uint32_t_array_tostring(
            4, reinterpret_cast<uint32_t*>(&this->storage_usage[0]))
     << "]";
  ss << ", \"storage_total\":"
     << "["
     << uint32_t_array_tostring(
            4, reinterpret_cast<uint32_t*>(&this->storage_total[0]))
     << "]";
  ss << ", \"link_type\":"
     << "["
     << uint32_t_array_tostring(
            6, reinterpret_cast<uint32_t*>(&this->link_type[0]))
     << "]";
  ss << ", \"link_tx_rate\":"
     << "["
     << uint32_t_array_tostring(
            6, reinterpret_cast<uint32_t*>(&this->link_tx_rate[0]))
     << "]";
  ss << ", \"link_rx_rate\":"
     << "["
     << uint32_t_array_tostring(
            6, reinterpret_cast<uint32_t*>(&this->link_rx_rate[0]))
     << "]";
  ss << ", \"link_tx_max\":"
     << "["
     << uint32_t_array_tostring(
            6, reinterpret_cast<uint32_t*>(&this->link_tx_max[0]))
     << "]";
  ss << ", \"link_rx_max\":"
     << "["
     << uint32_t_array_tostring(
            6, reinterpret_cast<uint32_t*>(&this->link_rx_max[0]))
     << "]";
  ss << ", \"fan_speed\":"
     << "["
     << int16_t_array_tostring(4,
                               reinterpret_cast<int16_t*>(&this->fan_speed[0]))
     << "]";
  ss << ", \"type\":" << static_cast<unsigned int>(this->type);
  ss << ", \"cpu_cores\":"
     << "["
     << uint8_t_array_tostring(8,
                               reinterpret_cast<uint8_t*>(&this->cpu_cores[0]))
     << "]";
  ss << ", \"cpu_combined\":"
     << "["
     << uint8_t_array_tostring(
            10, reinterpret_cast<uint8_t*>(&this->cpu_combined[0]))
     << "]";
  ss << ", \"gpu_cores\":"
     << "["
     << uint8_t_array_tostring(4,
                               reinterpret_cast<uint8_t*>(&this->gpu_cores[0]))
     << "]";
  ss << ", \"gpu_combined\":"
     << "["
     << uint8_t_array_tostring(
            10, reinterpret_cast<uint8_t*>(&this->gpu_combined[0]))
     << "]";
  ss << ", \"temperature_board\":" << static_cast<int>(this->temperature_board);
  ss << ", \"temperature_core\":"
     << "["
     << int8_t_array_tostring(
            8, reinterpret_cast<int8_t*>(&this->temperature_core[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkComponentInformation::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_uint32_t(
      buffer,
      reinterpret_cast<const uint32_t*>(&this->general_metadata_file_crc), 4);
  pack_uint32_t(
      buffer,
      reinterpret_cast<const uint32_t*>(&this->peripherals_metadata_file_crc),
      8);
  pack_char_array(100, buffer,
                  reinterpret_cast<const char*>(&this->general_metadata_uri[0]),
                  12);
  pack_char_array(
      100, buffer,
      reinterpret_cast<const char*>(&this->peripherals_metadata_uri[0]), 112);
  return 212;
}

int MavLinkComponentInformation::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_uint32_t(
      buffer, reinterpret_cast<uint32_t*>(&this->general_metadata_file_crc), 4);
  unpack_uint32_t(
      buffer, reinterpret_cast<uint32_t*>(&this->peripherals_metadata_file_crc),
      8);
  unpack_char_array(
      100, buffer, reinterpret_cast<char*>(&this->general_metadata_uri[0]), 12);
  unpack_char_array(100, buffer,
                    reinterpret_cast<char*>(&this->peripherals_metadata_uri[0]),
                    112);
  return 212;
}

std::string MavLinkComponentInformation::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"COMPONENT_INFORMATION\", \"id\": 395, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"general_metadata_file_crc\":" << this->general_metadata_file_crc;
  ss << ", \"peripherals_metadata_file_crc\":"
     << this->peripherals_metadata_file_crc;
  ss << ", \"general_metadata_uri\":"
     << "\""
     << char_array_tostring(
            100, reinterpret_cast<char*>(&this->general_metadata_uri[0]))
     << "\"";
  ss << ", \"peripherals_metadata_uri\":"
     << "\""
     << char_array_tostring(
            100, reinterpret_cast<char*>(&this->peripherals_metadata_uri[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkComponentMetadata::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->time_boot_ms),
                0);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->file_crc), 4);
  pack_char_array(100, buffer, reinterpret_cast<const char*>(&this->uri[0]), 8);
  return 108;
}

int MavLinkComponentMetadata::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->time_boot_ms), 0);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->file_crc), 4);
  unpack_char_array(100, buffer, reinterpret_cast<char*>(&this->uri[0]), 8);
  return 108;
}

std::string MavLinkComponentMetadata::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"COMPONENT_METADATA\", \"id\": 397, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_boot_ms\":" << this->time_boot_ms;
  ss << ", \"file_crc\":" << this->file_crc;
  ss << ", \"uri\":"
     << "\"" << char_array_tostring(100, reinterpret_cast<char*>(&this->uri[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkPlayTuneV2::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->format), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               4);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 5);
  pack_char_array(248, buffer, reinterpret_cast<const char*>(&this->tune[0]),
                  6);
  return 254;
}

int MavLinkPlayTuneV2::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->format), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 5);
  unpack_char_array(248, buffer, reinterpret_cast<char*>(&this->tune[0]), 6);
  return 254;
}

std::string MavLinkPlayTuneV2::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"PLAY_TUNE_V2\", \"id\": 400, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"format\":" << this->format;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"tune\":"
     << "\""
     << char_array_tostring(248, reinterpret_cast<char*>(&this->tune[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkSupportedTunes::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->format), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               4);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 5);
  return 6;
}

int MavLinkSupportedTunes::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->format), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 5);
  return 6;
}

std::string MavLinkSupportedTunes::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"SUPPORTED_TUNES\", \"id\": 401, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"format\":" << this->format;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkEvent::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->id), 0);
  pack_uint32_t(
      buffer, reinterpret_cast<const uint32_t*>(&this->event_time_boot_ms), 4);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->sequence), 8);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->destination_component),
               10);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->destination_system), 11);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->log_levels), 12);
  pack_uint8_t_array(40, buffer,
                     reinterpret_cast<const uint8_t*>(&this->arguments[0]), 13);
  return 53;
}

int MavLinkEvent::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->id), 0);
  unpack_uint32_t(buffer,
                  reinterpret_cast<uint32_t*>(&this->event_time_boot_ms), 4);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->sequence), 8);
  unpack_uint8_t(buffer,
                 reinterpret_cast<uint8_t*>(&this->destination_component), 10);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->destination_system),
                 11);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->log_levels), 12);
  unpack_uint8_t_array(40, buffer,
                       reinterpret_cast<uint8_t*>(&this->arguments[0]), 13);
  return 53;
}

std::string MavLinkEvent::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"EVENT\", \"id\": 410, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"id\":" << this->id;
  ss << ", \"event_time_boot_ms\":" << this->event_time_boot_ms;
  ss << ", \"sequence\":" << this->sequence;
  ss << ", \"destination_component\":"
     << static_cast<unsigned int>(this->destination_component);
  ss << ", \"destination_system\":"
     << static_cast<unsigned int>(this->destination_system);
  ss << ", \"log_levels\":" << static_cast<unsigned int>(this->log_levels);
  ss << ", \"arguments\":"
     << "["
     << uint8_t_array_tostring(40,
                               reinterpret_cast<uint8_t*>(&this->arguments[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkCurrentEventSequence::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->sequence), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->flags), 2);
  return 3;
}

int MavLinkCurrentEventSequence::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->sequence), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->flags), 2);
  return 3;
}

std::string MavLinkCurrentEventSequence::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CURRENT_EVENT_SEQUENCE\", \"id\": 411, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"sequence\":" << this->sequence;
  ss << ", \"flags\":" << static_cast<unsigned int>(this->flags);
  ss << "} },";
  return ss.str();
}

int MavLinkRequestEvent::pack(char* buffer) const {
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->first_sequence), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->last_sequence),
                2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               4);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 5);
  return 6;
}

int MavLinkRequestEvent::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->first_sequence),
                  0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->last_sequence), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 5);
  return 6;
}

std::string MavLinkRequestEvent::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"REQUEST_EVENT\", \"id\": 412, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"first_sequence\":" << this->first_sequence;
  ss << ", \"last_sequence\":" << this->last_sequence;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << "} },";
  return ss.str();
}

int MavLinkResponseEventError::pack(char* buffer) const {
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->sequence), 0);
  pack_uint16_t(
      buffer,
      reinterpret_cast<const uint16_t*>(&this->sequence_oldest_available), 2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               4);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 5);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->reason), 6);
  return 7;
}

int MavLinkResponseEventError::unpack(const char* buffer) {
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->sequence), 0);
  unpack_uint16_t(
      buffer, reinterpret_cast<uint16_t*>(&this->sequence_oldest_available), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 5);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->reason), 6);
  return 7;
}

std::string MavLinkResponseEventError::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"RESPONSE_EVENT_ERROR\", \"id\": 413, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"sequence\":" << this->sequence;
  ss << ", \"sequence_oldest_available\":" << this->sequence_oldest_available;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"reason\":" << static_cast<unsigned int>(this->reason);
  ss << "} },";
  return ss.str();
}

int MavLinkCanfdFrame::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->id), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               4);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 5);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->bus), 6);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->len), 7);
  pack_uint8_t_array(64, buffer,
                     reinterpret_cast<const uint8_t*>(&this->data[0]), 8);
  return 72;
}

int MavLinkCanfdFrame::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->id), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 5);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->bus), 6);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->len), 7);
  unpack_uint8_t_array(64, buffer, reinterpret_cast<uint8_t*>(&this->data[0]),
                       8);
  return 72;
}

std::string MavLinkCanfdFrame::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CANFD_FRAME\", \"id\": 387, \"timestamp\":" << timestamp
     << ", \"msg\": {";
  ss << "\"id\":" << this->id;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"bus\":" << static_cast<unsigned int>(this->bus);
  ss << ", \"len\":" << static_cast<unsigned int>(this->len);
  ss << ", \"data\":"
     << "["
     << uint8_t_array_tostring(64, reinterpret_cast<uint8_t*>(&this->data[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkCanFilterModify::pack(char* buffer) const {
  pack_uint16_t_array(16, buffer,
                      reinterpret_cast<const uint16_t*>(&this->ids[0]), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               32);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 33);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->bus), 34);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->operation), 35);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->num_ids), 36);
  return 37;
}

int MavLinkCanFilterModify::unpack(const char* buffer) {
  unpack_uint16_t_array(16, buffer, reinterpret_cast<uint16_t*>(&this->ids[0]),
                        0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 33);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->bus), 34);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->operation), 35);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->num_ids), 36);
  return 37;
}

std::string MavLinkCanFilterModify::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"CAN_FILTER_MODIFY\", \"id\": 388, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"ids\":"
     << "["
     << uint16_t_array_tostring(16, reinterpret_cast<uint16_t*>(&this->ids[0]))
     << "]";
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"bus\":" << static_cast<unsigned int>(this->bus);
  ss << ", \"operation\":" << static_cast<unsigned int>(this->operation);
  ss << ", \"num_ids\":" << static_cast<unsigned int>(this->num_ids);
  ss << "} },";
  return ss.str();
}

int MavLinkWheelDistance::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_double_array(16, buffer,
                    reinterpret_cast<const double*>(&this->distance[0]), 8);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->count), 136);
  return 137;
}

int MavLinkWheelDistance::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_double_array(16, buffer, reinterpret_cast<double*>(&this->distance[0]),
                      8);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->count), 136);
  return 137;
}

std::string MavLinkWheelDistance::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"WHEEL_DISTANCE\", \"id\": 9000, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"distance\":"
     << "["
     << double_array_tostring(16, reinterpret_cast<double*>(&this->distance[0]))
     << "]";
  ss << ", \"count\":" << static_cast<unsigned int>(this->count);
  ss << "} },";
  return ss.str();
}

int MavLinkWinchStatus::pack(char* buffer) const {
  pack_uint64_t(buffer, reinterpret_cast<const uint64_t*>(&this->time_usec), 0);
  pack_float(buffer, reinterpret_cast<const float*>(&this->line_length), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->speed), 12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->tension), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->voltage), 20);
  pack_float(buffer, reinterpret_cast<const float*>(&this->current), 24);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->status), 28);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature),
               32);
  return 34;
}

int MavLinkWinchStatus::unpack(const char* buffer) {
  unpack_uint64_t(buffer, reinterpret_cast<uint64_t*>(&this->time_usec), 0);
  unpack_float(buffer, reinterpret_cast<float*>(&this->line_length), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->speed), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->tension), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->voltage), 20);
  unpack_float(buffer, reinterpret_cast<float*>(&this->current), 24);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->status), 28);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 32);
  return 34;
}

std::string MavLinkWinchStatus::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"WINCH_STATUS\", \"id\": 9005, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"time_usec\":" << this->time_usec;
  ss << ", \"line_length\":" << float_tostring(this->line_length);
  ss << ", \"speed\":" << float_tostring(this->speed);
  ss << ", \"tension\":" << float_tostring(this->tension);
  ss << ", \"voltage\":" << float_tostring(this->voltage);
  ss << ", \"current\":" << float_tostring(this->current);
  ss << ", \"status\":" << this->status;
  ss << ", \"temperature\":" << this->temperature;
  ss << "} },";
  return ss.str();
}

int MavLinkOpenDroneIdBasicId::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  pack_uint8_t_array(20, buffer,
                     reinterpret_cast<const uint8_t*>(&this->id_or_mac[0]), 2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->id_type), 22);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->ua_type), 23);
  pack_uint8_t_array(20, buffer,
                     reinterpret_cast<const uint8_t*>(&this->uas_id[0]), 24);
  return 44;
}

int MavLinkOpenDroneIdBasicId::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  unpack_uint8_t_array(20, buffer,
                       reinterpret_cast<uint8_t*>(&this->id_or_mac[0]), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->id_type), 22);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->ua_type), 23);
  unpack_uint8_t_array(20, buffer, reinterpret_cast<uint8_t*>(&this->uas_id[0]),
                       24);
  return 44;
}

std::string MavLinkOpenDroneIdBasicId::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"OPEN_DRONE_ID_BASIC_ID\", \"id\": 12900, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"id_or_mac\":"
     << "["
     << uint8_t_array_tostring(20,
                               reinterpret_cast<uint8_t*>(&this->id_or_mac[0]))
     << "]";
  ss << ", \"id_type\":" << static_cast<unsigned int>(this->id_type);
  ss << ", \"ua_type\":" << static_cast<unsigned int>(this->ua_type);
  ss << ", \"uas_id\":"
     << "["
     << uint8_t_array_tostring(20, reinterpret_cast<uint8_t*>(&this->uas_id[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkOpenDroneIdLocation::pack(char* buffer) const {
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->latitude), 0);
  pack_int32_t(buffer, reinterpret_cast<const int32_t*>(&this->longitude), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->altitude_barometric),
             8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->altitude_geodetic),
             12);
  pack_float(buffer, reinterpret_cast<const float*>(&this->height), 16);
  pack_float(buffer, reinterpret_cast<const float*>(&this->timestamp), 20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->direction),
                24);
  pack_uint16_t(buffer,
                reinterpret_cast<const uint16_t*>(&this->speed_horizontal), 26);
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->speed_vertical),
               28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               30);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 31);
  pack_uint8_t_array(20, buffer,
                     reinterpret_cast<const uint8_t*>(&this->id_or_mac[0]), 32);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->status), 52);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->height_reference), 53);
  pack_uint8_t(
      buffer, reinterpret_cast<const uint8_t*>(&this->horizontal_accuracy), 54);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->vertical_accuracy), 55);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->barometer_accuracy), 56);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->speed_accuracy),
               57);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->timestamp_accuracy), 58);
  return 59;
}

int MavLinkOpenDroneIdLocation::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->latitude), 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->longitude), 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->altitude_barometric), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->altitude_geodetic), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->height), 16);
  unpack_float(buffer, reinterpret_cast<float*>(&this->timestamp), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->direction), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->speed_horizontal),
                  26);
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->speed_vertical), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 30);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 31);
  unpack_uint8_t_array(20, buffer,
                       reinterpret_cast<uint8_t*>(&this->id_or_mac[0]), 32);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->status), 52);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->height_reference),
                 53);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->horizontal_accuracy),
                 54);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->vertical_accuracy),
                 55);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->barometer_accuracy),
                 56);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->speed_accuracy), 57);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->timestamp_accuracy),
                 58);
  return 59;
}

std::string MavLinkOpenDroneIdLocation::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"OPEN_DRONE_ID_LOCATION\", \"id\": 12901, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"latitude\":" << this->latitude;
  ss << ", \"longitude\":" << this->longitude;
  ss << ", \"altitude_barometric\":"
     << float_tostring(this->altitude_barometric);
  ss << ", \"altitude_geodetic\":" << float_tostring(this->altitude_geodetic);
  ss << ", \"height\":" << float_tostring(this->height);
  ss << ", \"timestamp\":" << float_tostring(this->timestamp);
  ss << ", \"direction\":" << this->direction;
  ss << ", \"speed_horizontal\":" << this->speed_horizontal;
  ss << ", \"speed_vertical\":" << this->speed_vertical;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"id_or_mac\":"
     << "["
     << uint8_t_array_tostring(20,
                               reinterpret_cast<uint8_t*>(&this->id_or_mac[0]))
     << "]";
  ss << ", \"status\":" << static_cast<unsigned int>(this->status);
  ss << ", \"height_reference\":"
     << static_cast<unsigned int>(this->height_reference);
  ss << ", \"horizontal_accuracy\":"
     << static_cast<unsigned int>(this->horizontal_accuracy);
  ss << ", \"vertical_accuracy\":"
     << static_cast<unsigned int>(this->vertical_accuracy);
  ss << ", \"barometer_accuracy\":"
     << static_cast<unsigned int>(this->barometer_accuracy);
  ss << ", \"speed_accuracy\":"
     << static_cast<unsigned int>(this->speed_accuracy);
  ss << ", \"timestamp_accuracy\":"
     << static_cast<unsigned int>(this->timestamp_accuracy);
  ss << "} },";
  return ss.str();
}

int MavLinkOpenDroneIdAuthentication::pack(char* buffer) const {
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->timestamp), 0);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               4);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 5);
  pack_uint8_t_array(20, buffer,
                     reinterpret_cast<const uint8_t*>(&this->id_or_mac[0]), 6);
  pack_uint8_t(
      buffer, reinterpret_cast<const uint8_t*>(&this->authentication_type), 26);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->data_page), 27);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->last_page_index),
               28);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->length), 29);
  pack_uint8_t_array(
      23, buffer,
      reinterpret_cast<const uint8_t*>(&this->authentication_data[0]), 30);
  return 53;
}

int MavLinkOpenDroneIdAuthentication::unpack(const char* buffer) {
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->timestamp), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 4);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 5);
  unpack_uint8_t_array(20, buffer,
                       reinterpret_cast<uint8_t*>(&this->id_or_mac[0]), 6);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->authentication_type),
                 26);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->data_page), 27);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->last_page_index),
                 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->length), 29);
  unpack_uint8_t_array(
      23, buffer, reinterpret_cast<uint8_t*>(&this->authentication_data[0]),
      30);
  return 53;
}

std::string MavLinkOpenDroneIdAuthentication::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"OPEN_DRONE_ID_AUTHENTICATION\", \"id\": 12902, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"timestamp\":" << this->timestamp;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"id_or_mac\":"
     << "["
     << uint8_t_array_tostring(20,
                               reinterpret_cast<uint8_t*>(&this->id_or_mac[0]))
     << "]";
  ss << ", \"authentication_type\":"
     << static_cast<unsigned int>(this->authentication_type);
  ss << ", \"data_page\":" << static_cast<unsigned int>(this->data_page);
  ss << ", \"last_page_index\":"
     << static_cast<unsigned int>(this->last_page_index);
  ss << ", \"length\":" << static_cast<unsigned int>(this->length);
  ss << ", \"authentication_data\":"
     << "["
     << uint8_t_array_tostring(
            23, reinterpret_cast<uint8_t*>(&this->authentication_data[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkOpenDroneIdSelfId::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  pack_uint8_t_array(20, buffer,
                     reinterpret_cast<const uint8_t*>(&this->id_or_mac[0]), 2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->description_type), 22);
  pack_char_array(23, buffer,
                  reinterpret_cast<const char*>(&this->description[0]), 23);
  return 46;
}

int MavLinkOpenDroneIdSelfId::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  unpack_uint8_t_array(20, buffer,
                       reinterpret_cast<uint8_t*>(&this->id_or_mac[0]), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->description_type),
                 22);
  unpack_char_array(23, buffer, reinterpret_cast<char*>(&this->description[0]),
                    23);
  return 46;
}

std::string MavLinkOpenDroneIdSelfId::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"OPEN_DRONE_ID_SELF_ID\", \"id\": 12903, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"id_or_mac\":"
     << "["
     << uint8_t_array_tostring(20,
                               reinterpret_cast<uint8_t*>(&this->id_or_mac[0]))
     << "]";
  ss << ", \"description_type\":"
     << static_cast<unsigned int>(this->description_type);
  ss << ", \"description\":"
     << "\""
     << char_array_tostring(23, reinterpret_cast<char*>(&this->description[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkOpenDroneIdSystem::pack(char* buffer) const {
  pack_int32_t(buffer,
               reinterpret_cast<const int32_t*>(&this->operator_latitude), 0);
  pack_int32_t(buffer,
               reinterpret_cast<const int32_t*>(&this->operator_longitude), 4);
  pack_float(buffer, reinterpret_cast<const float*>(&this->area_ceiling), 8);
  pack_float(buffer, reinterpret_cast<const float*>(&this->area_floor), 12);
  pack_float(buffer,
             reinterpret_cast<const float*>(&this->operator_altitude_geo), 16);
  pack_uint32_t(buffer, reinterpret_cast<const uint32_t*>(&this->timestamp),
                20);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->area_count),
                24);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->area_radius),
                26);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               28);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 29);
  pack_uint8_t_array(20, buffer,
                     reinterpret_cast<const uint8_t*>(&this->id_or_mac[0]), 30);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->operator_location_type),
               50);
  pack_uint8_t(
      buffer, reinterpret_cast<const uint8_t*>(&this->classification_type), 51);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->category_eu),
               52);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->class_eu), 53);
  return 54;
}

int MavLinkOpenDroneIdSystem::unpack(const char* buffer) {
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->operator_latitude),
                 0);
  unpack_int32_t(buffer, reinterpret_cast<int32_t*>(&this->operator_longitude),
                 4);
  unpack_float(buffer, reinterpret_cast<float*>(&this->area_ceiling), 8);
  unpack_float(buffer, reinterpret_cast<float*>(&this->area_floor), 12);
  unpack_float(buffer, reinterpret_cast<float*>(&this->operator_altitude_geo),
               16);
  unpack_uint32_t(buffer, reinterpret_cast<uint32_t*>(&this->timestamp), 20);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->area_count), 24);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->area_radius), 26);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 28);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 29);
  unpack_uint8_t_array(20, buffer,
                       reinterpret_cast<uint8_t*>(&this->id_or_mac[0]), 30);
  unpack_uint8_t(buffer,
                 reinterpret_cast<uint8_t*>(&this->operator_location_type), 50);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->classification_type),
                 51);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->category_eu), 52);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->class_eu), 53);
  return 54;
}

std::string MavLinkOpenDroneIdSystem::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"OPEN_DRONE_ID_SYSTEM\", \"id\": 12904, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"operator_latitude\":" << this->operator_latitude;
  ss << ", \"operator_longitude\":" << this->operator_longitude;
  ss << ", \"area_ceiling\":" << float_tostring(this->area_ceiling);
  ss << ", \"area_floor\":" << float_tostring(this->area_floor);
  ss << ", \"operator_altitude_geo\":"
     << float_tostring(this->operator_altitude_geo);
  ss << ", \"timestamp\":" << this->timestamp;
  ss << ", \"area_count\":" << this->area_count;
  ss << ", \"area_radius\":" << this->area_radius;
  ss << ", \"target_system\":"
     << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"id_or_mac\":"
     << "["
     << uint8_t_array_tostring(20,
                               reinterpret_cast<uint8_t*>(&this->id_or_mac[0]))
     << "]";
  ss << ", \"operator_location_type\":"
     << static_cast<unsigned int>(this->operator_location_type);
  ss << ", \"classification_type\":"
     << static_cast<unsigned int>(this->classification_type);
  ss << ", \"category_eu\":" << static_cast<unsigned int>(this->category_eu);
  ss << ", \"class_eu\":" << static_cast<unsigned int>(this->class_eu);
  ss << "} },";
  return ss.str();
}

int MavLinkOpenDroneIdOperatorId::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  pack_uint8_t_array(20, buffer,
                     reinterpret_cast<const uint8_t*>(&this->id_or_mac[0]), 2);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->operator_id_type), 22);
  pack_char_array(20, buffer,
                  reinterpret_cast<const char*>(&this->operator_id[0]), 23);
  return 43;
}

int MavLinkOpenDroneIdOperatorId::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  unpack_uint8_t_array(20, buffer,
                       reinterpret_cast<uint8_t*>(&this->id_or_mac[0]), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->operator_id_type),
                 22);
  unpack_char_array(20, buffer, reinterpret_cast<char*>(&this->operator_id[0]),
                    23);
  return 43;
}

std::string MavLinkOpenDroneIdOperatorId::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"OPEN_DRONE_ID_OPERATOR_ID\", \"id\": 12905, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"id_or_mac\":"
     << "["
     << uint8_t_array_tostring(20,
                               reinterpret_cast<uint8_t*>(&this->id_or_mac[0]))
     << "]";
  ss << ", \"operator_id_type\":"
     << static_cast<unsigned int>(this->operator_id_type);
  ss << ", \"operator_id\":"
     << "\""
     << char_array_tostring(20, reinterpret_cast<char*>(&this->operator_id[0]))
     << "\"";
  ss << "} },";
  return ss.str();
}

int MavLinkOpenDroneIdMessagePack::pack(char* buffer) const {
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->target_system),
               0);
  pack_uint8_t(buffer,
               reinterpret_cast<const uint8_t*>(&this->target_component), 1);
  pack_uint8_t_array(20, buffer,
                     reinterpret_cast<const uint8_t*>(&this->id_or_mac[0]), 2);
  pack_uint8_t(
      buffer, reinterpret_cast<const uint8_t*>(&this->single_message_size), 22);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->msg_pack_size),
               23);
  pack_uint8_t_array(225, buffer,
                     reinterpret_cast<const uint8_t*>(&this->messages[0]), 24);
  return 249;
}

int MavLinkOpenDroneIdMessagePack::unpack(const char* buffer) {
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_system), 0);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->target_component),
                 1);
  unpack_uint8_t_array(20, buffer,
                       reinterpret_cast<uint8_t*>(&this->id_or_mac[0]), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->single_message_size),
                 22);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->msg_pack_size), 23);
  unpack_uint8_t_array(225, buffer,
                       reinterpret_cast<uint8_t*>(&this->messages[0]), 24);
  return 249;
}

std::string MavLinkOpenDroneIdMessagePack::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"OPEN_DRONE_ID_MESSAGE_PACK\", \"id\": 12915, "
        "\"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"target_system\":" << static_cast<unsigned int>(this->target_system);
  ss << ", \"target_component\":"
     << static_cast<unsigned int>(this->target_component);
  ss << ", \"id_or_mac\":"
     << "["
     << uint8_t_array_tostring(20,
                               reinterpret_cast<uint8_t*>(&this->id_or_mac[0]))
     << "]";
  ss << ", \"single_message_size\":"
     << static_cast<unsigned int>(this->single_message_size);
  ss << ", \"msg_pack_size\":"
     << static_cast<unsigned int>(this->msg_pack_size);
  ss << ", \"messages\":"
     << "["
     << uint8_t_array_tostring(225,
                               reinterpret_cast<uint8_t*>(&this->messages[0]))
     << "]";
  ss << "} },";
  return ss.str();
}

int MavLinkHygrometerSensor::pack(char* buffer) const {
  pack_int16_t(buffer, reinterpret_cast<const int16_t*>(&this->temperature), 0);
  pack_uint16_t(buffer, reinterpret_cast<const uint16_t*>(&this->humidity), 2);
  pack_uint8_t(buffer, reinterpret_cast<const uint8_t*>(&this->id), 4);
  return 5;
}

int MavLinkHygrometerSensor::unpack(const char* buffer) {
  unpack_int16_t(buffer, reinterpret_cast<int16_t*>(&this->temperature), 0);
  unpack_uint16_t(buffer, reinterpret_cast<uint16_t*>(&this->humidity), 2);
  unpack_uint8_t(buffer, reinterpret_cast<uint8_t*>(&this->id), 4);
  return 5;
}

std::string MavLinkHygrometerSensor::toJSon() {
  std::ostringstream ss;
  ss << "{ \"name\": \"HYGROMETER_SENSOR\", \"id\": 12920, \"timestamp\":"
     << timestamp << ", \"msg\": {";
  ss << "\"temperature\":" << this->temperature;
  ss << ", \"humidity\":" << this->humidity;
  ss << ", \"id\":" << static_cast<unsigned int>(this->id);
  ss << "} },";
  return ss.str();
}

void MavCmdNavWaypoint::pack() {
  param1 = Hold;
  param2 = AcceptRadius;
  param3 = PassRadius;
  param4 = Yaw;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdNavWaypoint::unpack() {
  Hold = param1;
  AcceptRadius = param2;
  PassRadius = param3;
  Yaw = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdNavLoiterUnlim::pack() {
  param3 = Radius;
  param4 = Yaw;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdNavLoiterUnlim::unpack() {
  Radius = param3;
  Yaw = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdNavLoiterTurns::pack() {
  param1 = Turns;
  param2 = HeadingRequired;
  param3 = Radius;
  param4 = XtrackLocation;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdNavLoiterTurns::unpack() {
  Turns = param1;
  HeadingRequired = param2;
  Radius = param3;
  XtrackLocation = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdNavLoiterTime::pack() {
  param1 = Time;
  param2 = HeadingRequired;
  param3 = Radius;
  param4 = XtrackLocation;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdNavLoiterTime::unpack() {
  Time = param1;
  HeadingRequired = param2;
  Radius = param3;
  XtrackLocation = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdNavReturnToLaunch::pack() {}
void MavCmdNavReturnToLaunch::unpack() {}
void MavCmdNavLand::pack() {
  param1 = AbortAlt;
  param2 = LandMode;
  param4 = YawAngle;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdNavLand::unpack() {
  AbortAlt = param1;
  LandMode = param2;
  YawAngle = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdNavTakeoff::pack() {
  param1 = Pitch;
  param4 = Yaw;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdNavTakeoff::unpack() {
  Pitch = param1;
  Yaw = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdNavLandLocal::pack() {
  param1 = Target;
  param2 = Offset;
  param3 = DescendRate;
  param4 = Yaw;
  param5 = YPosition;
  param6 = XPosition;
  param7 = ZPosition;
}
void MavCmdNavLandLocal::unpack() {
  Target = param1;
  Offset = param2;
  DescendRate = param3;
  Yaw = param4;
  YPosition = param5;
  XPosition = param6;
  ZPosition = param7;
}
void MavCmdNavTakeoffLocal::pack() {
  param1 = Pitch;
  param3 = AscendRate;
  param4 = Yaw;
  param5 = YPosition;
  param6 = XPosition;
  param7 = ZPosition;
}
void MavCmdNavTakeoffLocal::unpack() {
  Pitch = param1;
  AscendRate = param3;
  Yaw = param4;
  YPosition = param5;
  XPosition = param6;
  ZPosition = param7;
}
void MavCmdNavFollow::pack() {
  param1 = Following;
  param2 = GroundSpeed;
  param3 = Radius;
  param4 = Yaw;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdNavFollow::unpack() {
  Following = param1;
  GroundSpeed = param2;
  Radius = param3;
  Yaw = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdNavContinueAndChangeAlt::pack() {
  param1 = Action;
  param7 = Altitude;
}
void MavCmdNavContinueAndChangeAlt::unpack() {
  Action = param1;
  Altitude = param7;
}
void MavCmdNavLoiterToAlt::pack() {
  param1 = HeadingRequired;
  param2 = Radius;
  param4 = XtrackLocation;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdNavLoiterToAlt::unpack() {
  HeadingRequired = param1;
  Radius = param2;
  XtrackLocation = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdDoFollow::pack() {
  param1 = SystemId;
  param4 = AltitudeMode;
  param5 = Altitude;
  param7 = TimeToLand;
}
void MavCmdDoFollow::unpack() {
  SystemId = param1;
  AltitudeMode = param4;
  Altitude = param5;
  TimeToLand = param7;
}
void MavCmdDoFollowReposition::pack() {
  param1 = CameraQp1;
  param2 = CameraQp2;
  param3 = CameraQp3;
  param4 = CameraQp4;
  param5 = AltitudeOffset;
  param6 = XOffset;
  param7 = YOffset;
}
void MavCmdDoFollowReposition::unpack() {
  CameraQp1 = param1;
  CameraQp2 = param2;
  CameraQp3 = param3;
  CameraQp4 = param4;
  AltitudeOffset = param5;
  XOffset = param6;
  YOffset = param7;
}
void MavCmdDoOrbit::pack() {
  param1 = Radius;
  param2 = Velocity;
  param3 = YawBehavior;
  param4 = Orbits;
  param5 = Latitudepx;
  param6 = Longitudepy;
  param7 = Altitudepz;
}
void MavCmdDoOrbit::unpack() {
  Radius = param1;
  Velocity = param2;
  YawBehavior = param3;
  Orbits = param4;
  Latitudepx = param5;
  Longitudepy = param6;
  Altitudepz = param7;
}
void MavCmdNavRoi::pack() {
  param1 = RoiMode;
  param2 = WpIndex;
  param3 = RoiIndex;
  param5 = X;
  param6 = Y;
  param7 = Z;
}
void MavCmdNavRoi::unpack() {
  RoiMode = param1;
  WpIndex = param2;
  RoiIndex = param3;
  X = param5;
  Y = param6;
  Z = param7;
}
void MavCmdNavPathplanning::pack() {
  param1 = LocalCtrl;
  param2 = GlobalCtrl;
  param4 = Yaw;
  param5 = Latitudepx;
  param6 = Longitudepy;
  param7 = Altitudepz;
}
void MavCmdNavPathplanning::unpack() {
  LocalCtrl = param1;
  GlobalCtrl = param2;
  Yaw = param4;
  Latitudepx = param5;
  Longitudepy = param6;
  Altitudepz = param7;
}
void MavCmdNavSplineWaypoint::pack() {
  param1 = Hold;
  param5 = Latitudepx;
  param6 = Longitudepy;
  param7 = Altitudepz;
}
void MavCmdNavSplineWaypoint::unpack() {
  Hold = param1;
  Latitudepx = param5;
  Longitudepy = param6;
  Altitudepz = param7;
}
void MavCmdNavVtolTakeoff::pack() {
  param2 = TransitionHeading;
  param4 = YawAngle;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdNavVtolTakeoff::unpack() {
  TransitionHeading = param2;
  YawAngle = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdNavVtolLand::pack() {
  param1 = LandOptions;
  param3 = ApproachAltitude;
  param4 = Yaw;
  param5 = Latitude;
  param6 = Longitude;
  param7 = GroundAltitude;
}
void MavCmdNavVtolLand::unpack() {
  LandOptions = param1;
  ApproachAltitude = param3;
  Yaw = param4;
  Latitude = param5;
  Longitude = param6;
  GroundAltitude = param7;
}
void MavCmdNavGuidedEnable::pack() { param1 = Enable; }
void MavCmdNavGuidedEnable::unpack() { Enable = param1; }
void MavCmdNavDelay::pack() {
  param1 = Delay;
  param2 = Hour;
  param3 = Minute;
  param4 = Second;
}
void MavCmdNavDelay::unpack() {
  Delay = param1;
  Hour = param2;
  Minute = param3;
  Second = param4;
}
void MavCmdNavPayloadPlace::pack() {
  param1 = MaxDescent;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdNavPayloadPlace::unpack() {
  MaxDescent = param1;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdNavLast::pack() {}
void MavCmdNavLast::unpack() {}
void MavCmdConditionDelay::pack() { param1 = Delay; }
void MavCmdConditionDelay::unpack() { Delay = param1; }
void MavCmdConditionChangeAlt::pack() {
  param1 = Rate;
  param7 = Altitude;
}
void MavCmdConditionChangeAlt::unpack() {
  Rate = param1;
  Altitude = param7;
}
void MavCmdConditionDistance::pack() { param1 = Distance; }
void MavCmdConditionDistance::unpack() { Distance = param1; }
void MavCmdConditionYaw::pack() {
  param1 = Angle;
  param2 = AngularSpeed;
  param3 = Direction;
  param4 = Relative;
}
void MavCmdConditionYaw::unpack() {
  Angle = param1;
  AngularSpeed = param2;
  Direction = param3;
  Relative = param4;
}
void MavCmdConditionLast::pack() {}
void MavCmdConditionLast::unpack() {}
void MavCmdDoSetMode::pack() {
  param1 = Mode;
  param2 = CustomMode;
  param3 = CustomSubmode;
}
void MavCmdDoSetMode::unpack() {
  Mode = param1;
  CustomMode = param2;
  CustomSubmode = param3;
}
void MavCmdDoJump::pack() {
  param1 = Number;
  param2 = Repeat;
}
void MavCmdDoJump::unpack() {
  Number = param1;
  Repeat = param2;
}
void MavCmdDoChangeSpeed::pack() {
  param1 = SpeedType;
  param2 = Speed;
  param3 = Throttle;
}
void MavCmdDoChangeSpeed::unpack() {
  SpeedType = param1;
  Speed = param2;
  Throttle = param3;
}
void MavCmdDoSetHome::pack() {
  param1 = UseCurrent;
  param4 = Yaw;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdDoSetHome::unpack() {
  UseCurrent = param1;
  Yaw = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdDoSetParameter::pack() {
  param1 = Number;
  param2 = Value;
}
void MavCmdDoSetParameter::unpack() {
  Number = param1;
  Value = param2;
}
void MavCmdDoSetRelay::pack() {
  param1 = Instance;
  param2 = Setting;
}
void MavCmdDoSetRelay::unpack() {
  Instance = param1;
  Setting = param2;
}
void MavCmdDoRepeatRelay::pack() {
  param1 = Instance;
  param2 = Count;
  param3 = Time;
}
void MavCmdDoRepeatRelay::unpack() {
  Instance = param1;
  Count = param2;
  Time = param3;
}
void MavCmdDoSetServo::pack() {
  param1 = Instance;
  param2 = Pwm;
}
void MavCmdDoSetServo::unpack() {
  Instance = param1;
  Pwm = param2;
}
void MavCmdDoRepeatServo::pack() {
  param1 = Instance;
  param2 = Pwm;
  param3 = Count;
  param4 = Time;
}
void MavCmdDoRepeatServo::unpack() {
  Instance = param1;
  Pwm = param2;
  Count = param3;
  Time = param4;
}
void MavCmdDoFlighttermination::pack() { param1 = Terminate; }
void MavCmdDoFlighttermination::unpack() { Terminate = param1; }
void MavCmdDoChangeAltitude::pack() {
  param1 = Altitude;
  param2 = Frame;
}
void MavCmdDoChangeAltitude::unpack() {
  Altitude = param1;
  Frame = param2;
}
void MavCmdDoSetActuator::pack() {
  param1 = ActuatorP1;
  param2 = ActuatorP2;
  param3 = ActuatorP3;
  param4 = ActuatorP4;
  param5 = ActuatorP5;
  param6 = ActuatorP6;
  param7 = Index;
}
void MavCmdDoSetActuator::unpack() {
  ActuatorP1 = param1;
  ActuatorP2 = param2;
  ActuatorP3 = param3;
  ActuatorP4 = param4;
  ActuatorP5 = param5;
  ActuatorP6 = param6;
  Index = param7;
}
void MavCmdDoLandStart::pack() {
  param5 = Latitude;
  param6 = Longitude;
}
void MavCmdDoLandStart::unpack() {
  Latitude = param5;
  Longitude = param6;
}
void MavCmdDoRallyLand::pack() {
  param1 = Altitude;
  param2 = Speed;
}
void MavCmdDoRallyLand::unpack() {
  Altitude = param1;
  Speed = param2;
}
void MavCmdDoGoAround::pack() { param1 = Altitude; }
void MavCmdDoGoAround::unpack() { Altitude = param1; }
void MavCmdDoReposition::pack() {
  param1 = Speed;
  param2 = Bitmask;
  param3 = Radius;
  param4 = Yaw;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdDoReposition::unpack() {
  Speed = param1;
  Bitmask = param2;
  Radius = param3;
  Yaw = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdDoPauseContinue::pack() { param1 = Continue; }
void MavCmdDoPauseContinue::unpack() { Continue = param1; }
void MavCmdDoSetReverse::pack() { param1 = Reverse; }
void MavCmdDoSetReverse::unpack() { Reverse = param1; }
void MavCmdDoSetRoiLocation::pack() {
  param1 = GimbalDeviceId;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdDoSetRoiLocation::unpack() {
  GimbalDeviceId = param1;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdDoSetRoiWpnextOffset::pack() {
  param1 = GimbalDeviceId;
  param5 = PitchOffset;
  param6 = RollOffset;
  param7 = YawOffset;
}
void MavCmdDoSetRoiWpnextOffset::unpack() {
  GimbalDeviceId = param1;
  PitchOffset = param5;
  RollOffset = param6;
  YawOffset = param7;
}
void MavCmdDoSetRoiNone::pack() { param1 = GimbalDeviceId; }
void MavCmdDoSetRoiNone::unpack() { GimbalDeviceId = param1; }
void MavCmdDoSetRoiSysid::pack() {
  param1 = SystemId;
  param2 = GimbalDeviceId;
}
void MavCmdDoSetRoiSysid::unpack() {
  SystemId = param1;
  GimbalDeviceId = param2;
}
void MavCmdDoControlVideo::pack() {
  param1 = Id;
  param2 = Transmission;
  param3 = Interval;
  param4 = Recording;
}
void MavCmdDoControlVideo::unpack() {
  Id = param1;
  Transmission = param2;
  Interval = param3;
  Recording = param4;
}
void MavCmdDoSetRoi::pack() {
  param1 = RoiMode;
  param2 = WpIndex;
  param3 = RoiIndex;
  param5 = MavRoiWpnext;
  param6 = MavRoiWpnext2;
  param7 = MavRoiWpnext3;
}
void MavCmdDoSetRoi::unpack() {
  RoiMode = param1;
  WpIndex = param2;
  RoiIndex = param3;
  MavRoiWpnext = param5;
  MavRoiWpnext2 = param6;
  MavRoiWpnext3 = param7;
}
void MavCmdDoDigicamConfigure::pack() {
  param1 = Mode;
  param2 = ShutterSpeed;
  param3 = Aperture;
  param4 = Iso;
  param5 = Exposure;
  param6 = CommandIdentity;
  param7 = EngineCutpoff;
}
void MavCmdDoDigicamConfigure::unpack() {
  Mode = param1;
  ShutterSpeed = param2;
  Aperture = param3;
  Iso = param4;
  Exposure = param5;
  CommandIdentity = param6;
  EngineCutpoff = param7;
}
void MavCmdDoDigicamControl::pack() {
  param1 = SessionControl;
  param2 = ZoomAbsolute;
  param3 = ZoomRelative;
  param4 = Focus;
  param5 = ShootCommand;
  param6 = CommandIdentity;
  param7 = ShotId;
}
void MavCmdDoDigicamControl::unpack() {
  SessionControl = param1;
  ZoomAbsolute = param2;
  ZoomRelative = param3;
  Focus = param4;
  ShootCommand = param5;
  CommandIdentity = param6;
  ShotId = param7;
}
void MavCmdDoMountConfigure::pack() {
  param1 = Mode;
  param2 = StabilizeRoll;
  param3 = StabilizePitch;
  param4 = StabilizeYaw;
  param5 = RollInputMode;
  param6 = PitchInputMode;
  param7 = YawInputMode;
}
void MavCmdDoMountConfigure::unpack() {
  Mode = param1;
  StabilizeRoll = param2;
  StabilizePitch = param3;
  StabilizeYaw = param4;
  RollInputMode = param5;
  PitchInputMode = param6;
  YawInputMode = param7;
}
void MavCmdDoMountControl::pack() {
  param1 = Pitch;
  param2 = Roll;
  param3 = Yaw;
  param4 = Altitude;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Mode;
}
void MavCmdDoMountControl::unpack() {
  Pitch = param1;
  Roll = param2;
  Yaw = param3;
  Altitude = param4;
  Latitude = param5;
  Longitude = param6;
  Mode = param7;
}
void MavCmdDoSetCamTriggDist::pack() {
  param1 = Distance;
  param2 = Shutter;
  param3 = Trigger;
}
void MavCmdDoSetCamTriggDist::unpack() {
  Distance = param1;
  Shutter = param2;
  Trigger = param3;
}
void MavCmdDoFenceEnable::pack() { param1 = Enable; }
void MavCmdDoFenceEnable::unpack() { Enable = param1; }
void MavCmdDoParachute::pack() { param1 = Action; }
void MavCmdDoParachute::unpack() { Action = param1; }
void MavCmdDoMotorTest::pack() {
  param1 = Instance;
  param2 = ThrottleType;
  param3 = Throttle;
  param4 = Timeout;
  param5 = MotorCount;
  param6 = TestOrder;
}
void MavCmdDoMotorTest::unpack() {
  Instance = param1;
  ThrottleType = param2;
  Throttle = param3;
  Timeout = param4;
  MotorCount = param5;
  TestOrder = param6;
}
void MavCmdDoInvertedFlight::pack() { param1 = Inverted; }
void MavCmdDoInvertedFlight::unpack() { Inverted = param1; }
void MavCmdDoGripper::pack() {
  param1 = Instance;
  param2 = Action;
}
void MavCmdDoGripper::unpack() {
  Instance = param1;
  Action = param2;
}
void MavCmdDoAutotuneEnable::pack() {
  param1 = Enable;
  param2 = Axis;
}
void MavCmdDoAutotuneEnable::unpack() {
  Enable = param1;
  Axis = param2;
}
void MavCmdNavSetYawSpeed::pack() {
  param1 = Yaw;
  param2 = Speed;
  param3 = Angle;
}
void MavCmdNavSetYawSpeed::unpack() {
  Yaw = param1;
  Speed = param2;
  Angle = param3;
}
void MavCmdDoSetCamTriggInterval::pack() {
  param1 = TriggerCycle;
  param2 = ShutterIntegration;
}
void MavCmdDoSetCamTriggInterval::unpack() {
  TriggerCycle = param1;
  ShutterIntegration = param2;
}
void MavCmdDoMountControlQuat::pack() {
  param1 = Qp1;
  param2 = Qp2;
  param3 = Qp3;
  param4 = Qp4;
}
void MavCmdDoMountControlQuat::unpack() {
  Qp1 = param1;
  Qp2 = param2;
  Qp3 = param3;
  Qp4 = param4;
}
void MavCmdDoGuidedMaster::pack() {
  param1 = SystemId;
  param2 = ComponentId;
}
void MavCmdDoGuidedMaster::unpack() {
  SystemId = param1;
  ComponentId = param2;
}
void MavCmdDoGuidedLimits::pack() {
  param1 = Timeout;
  param2 = MinAltitude;
  param3 = MaxAltitude;
  param4 = HorizpMoveLimit;
}
void MavCmdDoGuidedLimits::unpack() {
  Timeout = param1;
  MinAltitude = param2;
  MaxAltitude = param3;
  HorizpMoveLimit = param4;
}
void MavCmdDoEngineControl::pack() {
  param1 = StartEngine;
  param2 = ColdStart;
  param3 = HeightDelay;
}
void MavCmdDoEngineControl::unpack() {
  StartEngine = param1;
  ColdStart = param2;
  HeightDelay = param3;
}
void MavCmdDoSetMissionCurrent::pack() { param1 = Number; }
void MavCmdDoSetMissionCurrent::unpack() { Number = param1; }
void MavCmdDoLast::pack() {}
void MavCmdDoLast::unpack() {}
void MavCmdPreflightCalibration::pack() {
  param1 = GyroTemperature;
  param2 = Magnetometer;
  param3 = GroundPressure;
  param4 = RemoteControl;
  param5 = Accelerometer;
  param6 = CompmotOrAirspeed;
  param7 = EscOrBaro;
}
void MavCmdPreflightCalibration::unpack() {
  GyroTemperature = param1;
  Magnetometer = param2;
  GroundPressure = param3;
  RemoteControl = param4;
  Accelerometer = param5;
  CompmotOrAirspeed = param6;
  EscOrBaro = param7;
}
void MavCmdPreflightSetSensorOffsets::pack() {
  param1 = SensorType;
  param2 = XOffset;
  param3 = YOffset;
  param4 = ZOffset;
  param5 = P4thDimension;
  param6 = P5thDimension;
  param7 = P6thDimension;
}
void MavCmdPreflightSetSensorOffsets::unpack() {
  SensorType = param1;
  XOffset = param2;
  YOffset = param3;
  ZOffset = param4;
  P4thDimension = param5;
  P5thDimension = param6;
  P6thDimension = param7;
}
void MavCmdPreflightUavcan::pack() { param1 = ActuatorId; }
void MavCmdPreflightUavcan::unpack() { ActuatorId = param1; }
void MavCmdPreflightStorage::pack() {
  param1 = ParameterStorage;
  param2 = MissionStorage;
  param3 = LoggingRate;
}
void MavCmdPreflightStorage::unpack() {
  ParameterStorage = param1;
  MissionStorage = param2;
  LoggingRate = param3;
}
void MavCmdPreflightRebootShutdown::pack() {
  param1 = Autopilot;
  param2 = Companion;
  param3 = ComponentAction;
  param4 = ComponentId;
  param7 = Wip;
}
void MavCmdPreflightRebootShutdown::unpack() {
  Autopilot = param1;
  Companion = param2;
  ComponentAction = param3;
  ComponentId = param4;
  Wip = param7;
}
void MavCmdOverrideGoto::pack() {
  param1 = Continue;
  param2 = Position;
  param3 = Frame;
  param4 = Yaw;
  param5 = Latitudepx;
  param6 = Longitudepy;
  param7 = Altitudepz;
}
void MavCmdOverrideGoto::unpack() {
  Continue = param1;
  Position = param2;
  Frame = param3;
  Yaw = param4;
  Latitudepx = param5;
  Longitudepy = param6;
  Altitudepz = param7;
}
void MavCmdObliqueSurvey::pack() {
  param1 = Distance;
  param2 = Shutter;
  param3 = MinInterval;
  param4 = Positions;
  param5 = RollAngle;
  param6 = PitchAngle;
}
void MavCmdObliqueSurvey::unpack() {
  Distance = param1;
  Shutter = param2;
  MinInterval = param3;
  Positions = param4;
  RollAngle = param5;
  PitchAngle = param6;
}
void MavCmdMissionStart::pack() {
  param1 = FirstItem;
  param2 = LastItem;
}
void MavCmdMissionStart::unpack() {
  FirstItem = param1;
  LastItem = param2;
}
void MavCmdActuatorTest::pack() {
  param1 = Value;
  param2 = Timeout;
  param5 = OutputFunction;
}
void MavCmdActuatorTest::unpack() {
  Value = param1;
  Timeout = param2;
  OutputFunction = param5;
}
void MavCmdConfigureActuator::pack() {
  param1 = Configuration;
  param5 = OutputFunction;
}
void MavCmdConfigureActuator::unpack() {
  Configuration = param1;
  OutputFunction = param5;
}
void MavCmdComponentArmDisarm::pack() {
  param1 = Arm;
  param2 = Force;
}
void MavCmdComponentArmDisarm::unpack() {
  Arm = param1;
  Force = param2;
}
void MavCmdRunPrearmChecks::pack() {}
void MavCmdRunPrearmChecks::unpack() {}
void MavCmdIlluminatorOnOff::pack() { param1 = Enable; }
void MavCmdIlluminatorOnOff::unpack() { Enable = param1; }
void MavCmdGetHomePosition::pack() {}
void MavCmdGetHomePosition::unpack() {}
void MavCmdInjectFailure::pack() {
  param1 = FailureUnit;
  param2 = FailureType;
  param3 = Instance;
}
void MavCmdInjectFailure::unpack() {
  FailureUnit = param1;
  FailureType = param2;
  Instance = param3;
}
void MavCmdStartRxPair::pack() {
  param1 = Spektrum;
  param2 = RcType;
}
void MavCmdStartRxPair::unpack() {
  Spektrum = param1;
  RcType = param2;
}
void MavCmdGetMessageInterval::pack() { param1 = MessageId; }
void MavCmdGetMessageInterval::unpack() { MessageId = param1; }
void MavCmdSetMessageInterval::pack() {
  param1 = MessageId;
  param2 = Interval;
  param3 = ResponseTarget;
}
void MavCmdSetMessageInterval::unpack() {
  MessageId = param1;
  Interval = param2;
  ResponseTarget = param3;
}
void MavCmdRequestMessage::pack() {
  param1 = MessageId;
  param2 = ReqParamP1;
  param3 = ReqParamP2;
  param4 = ReqParamP3;
  param5 = ReqParamP4;
  param6 = ReqParamP5;
  param7 = ResponseTarget;
}
void MavCmdRequestMessage::unpack() {
  MessageId = param1;
  ReqParamP1 = param2;
  ReqParamP2 = param3;
  ReqParamP3 = param4;
  ReqParamP4 = param5;
  ReqParamP5 = param6;
  ResponseTarget = param7;
}
void MavCmdRequestProtocolVersion::pack() { param1 = Protocol; }
void MavCmdRequestProtocolVersion::unpack() { Protocol = param1; }
void MavCmdRequestAutopilotCapabilities::pack() { param1 = Version; }
void MavCmdRequestAutopilotCapabilities::unpack() { Version = param1; }
void MavCmdRequestCameraInformation::pack() { param1 = Capabilities; }
void MavCmdRequestCameraInformation::unpack() { Capabilities = param1; }
void MavCmdRequestCameraSettings::pack() { param1 = Settings; }
void MavCmdRequestCameraSettings::unpack() { Settings = param1; }
void MavCmdRequestStorageInformation::pack() {
  param1 = StorageId;
  param2 = Information;
}
void MavCmdRequestStorageInformation::unpack() {
  StorageId = param1;
  Information = param2;
}
void MavCmdStorageFormat::pack() {
  param1 = StorageId;
  param2 = Format;
  param3 = ResetImageLog;
}
void MavCmdStorageFormat::unpack() {
  StorageId = param1;
  Format = param2;
  ResetImageLog = param3;
}
void MavCmdRequestCameraCaptureStatus::pack() { param1 = CaptureStatus; }
void MavCmdRequestCameraCaptureStatus::unpack() { CaptureStatus = param1; }
void MavCmdRequestFlightInformation::pack() { param1 = FlightInformation; }
void MavCmdRequestFlightInformation::unpack() { FlightInformation = param1; }
void MavCmdResetCameraSettings::pack() { param1 = Reset; }
void MavCmdResetCameraSettings::unpack() { Reset = param1; }
void MavCmdSetCameraMode::pack() { param2 = CameraMode; }
void MavCmdSetCameraMode::unpack() { CameraMode = param2; }
void MavCmdSetCameraZoom::pack() {
  param1 = ZoomType;
  param2 = ZoomValue;
}
void MavCmdSetCameraZoom::unpack() {
  ZoomType = param1;
  ZoomValue = param2;
}
void MavCmdSetCameraFocus::pack() {
  param1 = FocusType;
  param2 = FocusValue;
}
void MavCmdSetCameraFocus::unpack() {
  FocusType = param1;
  FocusValue = param2;
}
void MavCmdSetStorageUsage::pack() {
  param1 = StorageId;
  param2 = Usage;
}
void MavCmdSetStorageUsage::unpack() {
  StorageId = param1;
  Usage = param2;
}
void MavCmdJumpTag::pack() { param1 = Tag; }
void MavCmdJumpTag::unpack() { Tag = param1; }
void MavCmdDoJumpTag::pack() {
  param1 = Tag;
  param2 = Repeat;
}
void MavCmdDoJumpTag::unpack() {
  Tag = param1;
  Repeat = param2;
}
void MavCmdDoGimbalManagerPitchyaw::pack() {
  param1 = PitchAngle;
  param2 = YawAngle;
  param3 = PitchRate;
  param4 = YawRate;
  param5 = GimbalManagerFlags;
  param6 = GimbalDeviceId;
}
void MavCmdDoGimbalManagerPitchyaw::unpack() {
  PitchAngle = param1;
  YawAngle = param2;
  PitchRate = param3;
  YawRate = param4;
  GimbalManagerFlags = param5;
  GimbalDeviceId = param6;
}
void MavCmdDoGimbalManagerConfigure::pack() {
  param1 = SysidPrimaryControl;
  param2 = CompidPrimaryControl;
  param3 = SysidSecondaryControl;
  param4 = CompidSecondaryControl;
  param5 = GimbalDeviceId;
}
void MavCmdDoGimbalManagerConfigure::unpack() {
  SysidPrimaryControl = param1;
  CompidPrimaryControl = param2;
  SysidSecondaryControl = param3;
  CompidSecondaryControl = param4;
  GimbalDeviceId = param5;
}
void MavCmdImageStartCapture::pack() {
  param2 = Interval;
  param3 = TotalImages;
  param4 = SequenceNumber;
}
void MavCmdImageStartCapture::unpack() {
  Interval = param2;
  TotalImages = param3;
  SequenceNumber = param4;
}
void MavCmdImageStopCapture::pack() {}
void MavCmdImageStopCapture::unpack() {}
void MavCmdRequestCameraImageCapture::pack() { param1 = Number; }
void MavCmdRequestCameraImageCapture::unpack() { Number = param1; }
void MavCmdDoTriggerControl::pack() {
  param1 = Enable;
  param2 = Reset;
  param3 = Pause;
}
void MavCmdDoTriggerControl::unpack() {
  Enable = param1;
  Reset = param2;
  Pause = param3;
}
void MavCmdCameraTrackPoint::pack() {
  param1 = PointX;
  param2 = PointY;
  param3 = Radius;
}
void MavCmdCameraTrackPoint::unpack() {
  PointX = param1;
  PointY = param2;
  Radius = param3;
}
void MavCmdCameraTrackRectangle::pack() {
  param1 = TopLeftCorner;
  param2 = TopLeftCorner2;
  param3 = BottomRightCorner;
  param4 = BottomRightCorner2;
}
void MavCmdCameraTrackRectangle::unpack() {
  TopLeftCorner = param1;
  TopLeftCorner2 = param2;
  BottomRightCorner = param3;
  BottomRightCorner2 = param4;
}
void MavCmdCameraStopTracking::pack() {}
void MavCmdCameraStopTracking::unpack() {}
void MavCmdVideoStartCapture::pack() {
  param1 = StreamId;
  param2 = StatusFrequency;
}
void MavCmdVideoStartCapture::unpack() {
  StreamId = param1;
  StatusFrequency = param2;
}
void MavCmdVideoStopCapture::pack() { param1 = StreamId; }
void MavCmdVideoStopCapture::unpack() { StreamId = param1; }
void MavCmdVideoStartStreaming::pack() { param1 = StreamId; }
void MavCmdVideoStartStreaming::unpack() { StreamId = param1; }
void MavCmdVideoStopStreaming::pack() { param1 = StreamId; }
void MavCmdVideoStopStreaming::unpack() { StreamId = param1; }
void MavCmdRequestVideoStreamInformation::pack() { param1 = StreamId; }
void MavCmdRequestVideoStreamInformation::unpack() { StreamId = param1; }
void MavCmdRequestVideoStreamStatus::pack() { param1 = StreamId; }
void MavCmdRequestVideoStreamStatus::unpack() { StreamId = param1; }
void MavCmdLoggingStart::pack() { param1 = Format; }
void MavCmdLoggingStart::unpack() { Format = param1; }
void MavCmdLoggingStop::pack() {}
void MavCmdLoggingStop::unpack() {}
void MavCmdAirframeConfiguration::pack() {
  param1 = LandingGearId;
  param2 = LandingGearPosition;
}
void MavCmdAirframeConfiguration::unpack() {
  LandingGearId = param1;
  LandingGearPosition = param2;
}
void MavCmdControlHighLatency::pack() { param1 = Enable; }
void MavCmdControlHighLatency::unpack() { Enable = param1; }
void MavCmdPanoramaCreate::pack() {
  param1 = HorizontalAngle;
  param2 = VerticalAngle;
  param3 = HorizontalSpeed;
  param4 = VerticalSpeed;
}
void MavCmdPanoramaCreate::unpack() {
  HorizontalAngle = param1;
  VerticalAngle = param2;
  HorizontalSpeed = param3;
  VerticalSpeed = param4;
}
void MavCmdDoVtolTransition::pack() {
  param1 = State;
  param2 = Immediate;
}
void MavCmdDoVtolTransition::unpack() {
  State = param1;
  Immediate = param2;
}
void MavCmdArmAuthorizationRequest::pack() { param1 = SystemId; }
void MavCmdArmAuthorizationRequest::unpack() { SystemId = param1; }
void MavCmdSetGuidedSubmodeStandard::pack() {}
void MavCmdSetGuidedSubmodeStandard::unpack() {}
void MavCmdSetGuidedSubmodeCircle::pack() {
  param1 = Radius;
  param2 = UserDefined;
  param3 = UserDefined2;
  param4 = UserDefined3;
  param5 = Latitude;
  param6 = Longitude;
}
void MavCmdSetGuidedSubmodeCircle::unpack() {
  Radius = param1;
  UserDefined = param2;
  UserDefined2 = param3;
  UserDefined3 = param4;
  Latitude = param5;
  Longitude = param6;
}
void MavCmdConditionGate::pack() {
  param1 = Geometry;
  param2 = Usealtitude;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdConditionGate::unpack() {
  Geometry = param1;
  Usealtitude = param2;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdNavFenceReturnPoint::pack() {
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdNavFenceReturnPoint::unpack() {
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdNavFencePolygonVertexInclusion::pack() {
  param1 = VertexCount;
  param2 = InclusionGroup;
  param5 = Latitude;
  param6 = Longitude;
}
void MavCmdNavFencePolygonVertexInclusion::unpack() {
  VertexCount = param1;
  InclusionGroup = param2;
  Latitude = param5;
  Longitude = param6;
}
void MavCmdNavFencePolygonVertexExclusion::pack() {
  param1 = VertexCount;
  param5 = Latitude;
  param6 = Longitude;
}
void MavCmdNavFencePolygonVertexExclusion::unpack() {
  VertexCount = param1;
  Latitude = param5;
  Longitude = param6;
}
void MavCmdNavFenceCircleInclusion::pack() {
  param1 = Radius;
  param2 = InclusionGroup;
  param5 = Latitude;
  param6 = Longitude;
}
void MavCmdNavFenceCircleInclusion::unpack() {
  Radius = param1;
  InclusionGroup = param2;
  Latitude = param5;
  Longitude = param6;
}
void MavCmdNavFenceCircleExclusion::pack() {
  param1 = Radius;
  param5 = Latitude;
  param6 = Longitude;
}
void MavCmdNavFenceCircleExclusion::unpack() {
  Radius = param1;
  Latitude = param5;
  Longitude = param6;
}
void MavCmdNavRallyPoint::pack() {
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdNavRallyPoint::unpack() {
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdUavcanGetNodeInfo::pack() {}
void MavCmdUavcanGetNodeInfo::unpack() {}
void MavCmdDoAdsbOutIdent::pack() {}
void MavCmdDoAdsbOutIdent::unpack() {}
void MavCmdPayloadPrepareDeploy::pack() {
  param1 = OperationMode;
  param2 = ApproachVector;
  param3 = GroundSpeed;
  param4 = AltitudeClearance;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdPayloadPrepareDeploy::unpack() {
  OperationMode = param1;
  ApproachVector = param2;
  GroundSpeed = param3;
  AltitudeClearance = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdPayloadControlDeploy::pack() { param1 = OperationMode; }
void MavCmdPayloadControlDeploy::unpack() { OperationMode = param1; }
void MavCmdFixedMagCalYaw::pack() {
  param1 = Yaw;
  param2 = Compassmask;
  param3 = Latitude;
  param4 = Longitude;
}
void MavCmdFixedMagCalYaw::unpack() {
  Yaw = param1;
  Compassmask = param2;
  Latitude = param3;
  Longitude = param4;
}
void MavCmdDoWinch::pack() {
  param1 = Instance;
  param2 = Action;
  param3 = Length;
  param4 = Rate;
}
void MavCmdDoWinch::unpack() {
  Instance = param1;
  Action = param2;
  Length = param3;
  Rate = param4;
}
void MavCmdWaypointUser1::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdWaypointUser1::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdWaypointUser2::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdWaypointUser2::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdWaypointUser3::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdWaypointUser3::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdWaypointUser4::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdWaypointUser4::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdWaypointUser5::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdWaypointUser5::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdSpatialUser1::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdSpatialUser1::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdSpatialUser2::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdSpatialUser2::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdSpatialUser3::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdSpatialUser3::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdSpatialUser4::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdSpatialUser4::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdSpatialUser5::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = Latitude;
  param6 = Longitude;
  param7 = Altitude;
}
void MavCmdSpatialUser5::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  Latitude = param5;
  Longitude = param6;
  Altitude = param7;
}
void MavCmdUser1::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = UserDefined5;
  param6 = UserDefined6;
  param7 = UserDefined7;
}
void MavCmdUser1::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  UserDefined5 = param5;
  UserDefined6 = param6;
  UserDefined7 = param7;
}
void MavCmdUser2::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = UserDefined5;
  param6 = UserDefined6;
  param7 = UserDefined7;
}
void MavCmdUser2::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  UserDefined5 = param5;
  UserDefined6 = param6;
  UserDefined7 = param7;
}
void MavCmdUser3::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = UserDefined5;
  param6 = UserDefined6;
  param7 = UserDefined7;
}
void MavCmdUser3::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  UserDefined5 = param5;
  UserDefined6 = param6;
  UserDefined7 = param7;
}
void MavCmdUser4::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = UserDefined5;
  param6 = UserDefined6;
  param7 = UserDefined7;
}
void MavCmdUser4::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  UserDefined5 = param5;
  UserDefined6 = param6;
  UserDefined7 = param7;
}
void MavCmdUser5::pack() {
  param1 = UserDefined;
  param2 = UserDefined2;
  param3 = UserDefined3;
  param4 = UserDefined4;
  param5 = UserDefined5;
  param6 = UserDefined6;
  param7 = UserDefined7;
}
void MavCmdUser5::unpack() {
  UserDefined = param1;
  UserDefined2 = param2;
  UserDefined3 = param3;
  UserDefined4 = param4;
  UserDefined5 = param5;
  UserDefined6 = param6;
  UserDefined7 = param7;
}
void MavCmdCanForward::pack() { param1 = Bus; }
void MavCmdCanForward::unpack() { Bus = param1; }
MavLinkMessageBase* MavLinkMessageBase::lookup(const MavLinkMessage& msg) {
  MavLinkMessageBase* result = nullptr;
  switch (static_cast<MavLinkMessageIds>(msg.msgid)) {
    case MavLinkMessageIds::MAVLINK_MSG_ID_HEARTBEAT:
      result = new MavLinkHeartbeat();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SYS_STATUS:
      result = new MavLinkSysStatus();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SYSTEM_TIME:
      result = new MavLinkSystemTime();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_PING:
      result = new MavLinkPing();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_CHANGE_OPERATOR_CONTROL:
      result = new MavLinkChangeOperatorControl();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_CHANGE_OPERATOR_CONTROL_ACK:
      result = new MavLinkChangeOperatorControlAck();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_AUTH_KEY:
      result = new MavLinkAuthKey();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_LINK_NODE_STATUS:
      result = new MavLinkLinkNodeStatus();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SET_MODE:
      result = new MavLinkSetMode();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_PARAM_REQUEST_READ:
      result = new MavLinkParamRequestRead();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
      result = new MavLinkParamRequestList();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_PARAM_VALUE:
      result = new MavLinkParamValue();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_PARAM_SET:
      result = new MavLinkParamSet();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_GPS_RAW_INT:
      result = new MavLinkGpsRawInt();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_GPS_STATUS:
      result = new MavLinkGpsStatus();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SCALED_IMU:
      result = new MavLinkScaledImu();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_RAW_IMU:
      result = new MavLinkRawImu();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_RAW_PRESSURE:
      result = new MavLinkRawPressure();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SCALED_PRESSURE:
      result = new MavLinkScaledPressure();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_ATTITUDE:
      result = new MavLinkAttitude();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_ATTITUDE_QUATERNION:
      result = new MavLinkAttitudeQuaternion();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_LOCAL_POSITION_NED:
      result = new MavLinkLocalPositionNed();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
      result = new MavLinkGlobalPositionInt();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_RC_CHANNELS_SCALED:
      result = new MavLinkRcChannelsScaled();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_RC_CHANNELS_RAW:
      result = new MavLinkRcChannelsRaw();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SERVO_OUTPUT_RAW:
      result = new MavLinkServoOutputRaw();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_REQUEST_PARTIAL_LIST:
      result = new MavLinkMissionRequestPartialList();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_WRITE_PARTIAL_LIST:
      result = new MavLinkMissionWritePartialList();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_ITEM:
      result = new MavLinkMissionItem();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_REQUEST:
      result = new MavLinkMissionRequest();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_SET_CURRENT:
      result = new MavLinkMissionSetCurrent();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_CURRENT:
      result = new MavLinkMissionCurrent();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
      result = new MavLinkMissionRequestList();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_COUNT:
      result = new MavLinkMissionCount();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_CLEAR_ALL:
      result = new MavLinkMissionClearAll();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_ITEM_REACHED:
      result = new MavLinkMissionItemReached();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_ACK:
      result = new MavLinkMissionAck();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SET_GPS_GLOBAL_ORIGIN:
      result = new MavLinkSetGpsGlobalOrigin();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_GPS_GLOBAL_ORIGIN:
      result = new MavLinkGpsGlobalOrigin();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_PARAM_MAP_RC:
      result = new MavLinkParamMapRc();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_REQUEST_INT:
      result = new MavLinkMissionRequestInt();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SAFETY_SET_ALLOWED_AREA:
      result = new MavLinkSafetySetAllowedArea();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SAFETY_ALLOWED_AREA:
      result = new MavLinkSafetyAllowedArea();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_ATTITUDE_QUATERNION_COV:
      result = new MavLinkAttitudeQuaternionCov();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:
      result = new MavLinkNavControllerOutput();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_GLOBAL_POSITION_INT_COV:
      result = new MavLinkGlobalPositionIntCov();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_LOCAL_POSITION_NED_COV:
      result = new MavLinkLocalPositionNedCov();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_RC_CHANNELS:
      result = new MavLinkRcChannels();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_REQUEST_DATA_STREAM:
      result = new MavLinkRequestDataStream();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_DATA_STREAM:
      result = new MavLinkDataStream();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MANUAL_CONTROL:
      result = new MavLinkManualControl();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
      result = new MavLinkRcChannelsOverride();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MISSION_ITEM_INT:
      result = new MavLinkMissionItemInt();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_VFR_HUD:
      result = new MavLinkVfrHud();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_COMMAND_INT:
      result = new MavLinkCommandInt();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_COMMAND_LONG:
      result = new MavLinkCommandLong();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_COMMAND_ACK:
      result = new MavLinkCommandAck();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_COMMAND_CANCEL:
      result = new MavLinkCommandCancel();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MANUAL_SETPOINT:
      result = new MavLinkManualSetpoint();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SET_ATTITUDE_TARGET:
      result = new MavLinkSetAttitudeTarget();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_ATTITUDE_TARGET:
      result = new MavLinkAttitudeTarget();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SET_POSITION_TARGET_LOCAL_NED:
      result = new MavLinkSetPositionTargetLocalNed();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED:
      result = new MavLinkPositionTargetLocalNed();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT:
      result = new MavLinkSetPositionTargetGlobalInt();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT:
      result = new MavLinkPositionTargetGlobalInt();
      break;
    case MavLinkMessageIds::
        MAVLINK_MSG_ID_LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET:
      result = new MavLinkLocalPositionNedSystemGlobalOffset();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_HIL_STATE:
      result = new MavLinkHilState();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_HIL_CONTROLS:
      result = new MavLinkHilControls();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_HIL_RC_INPUTS_RAW:
      result = new MavLinkHilRcInputsRaw();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_HIL_ACTUATOR_CONTROLS:
      result = new MavLinkHilActuatorControls();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_OPTICAL_FLOW:
      result = new MavLinkOpticalFlow();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_GLOBAL_VISION_POSITION_ESTIMATE:
      result = new MavLinkGlobalVisionPositionEstimate();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_VISION_POSITION_ESTIMATE:
      result = new MavLinkVisionPositionEstimate();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_VISION_SPEED_ESTIMATE:
      result = new MavLinkVisionSpeedEstimate();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_VICON_POSITION_ESTIMATE:
      result = new MavLinkViconPositionEstimate();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_HIGHRES_IMU:
      result = new MavLinkHighresImu();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_OPTICAL_FLOW_RAD:
      result = new MavLinkOpticalFlowRad();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_HIL_SENSOR:
      result = new MavLinkHilSensor();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SIM_STATE:
      result = new MavLinkSimState();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_RADIO_STATUS:
      result = new MavLinkRadioStatus();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_FILE_TRANSFER_PROTOCOL:
      result = new MavLinkFileTransferProtocol();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_TIMESYNC:
      result = new MavLinkTimesync();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_CAMERA_TRIGGER:
      result = new MavLinkCameraTrigger();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_HIL_GPS:
      result = new MavLinkHilGps();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_HIL_OPTICAL_FLOW:
      result = new MavLinkHilOpticalFlow();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_HIL_STATE_QUATERNION:
      result = new MavLinkHilStateQuaternion();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SCALED_IMU2:
      result = new MavLinkScaledImu2();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_LOG_REQUEST_LIST:
      result = new MavLinkLogRequestList();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_LOG_ENTRY:
      result = new MavLinkLogEntry();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_LOG_REQUEST_DATA:
      result = new MavLinkLogRequestData();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_LOG_DATA:
      result = new MavLinkLogData();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_LOG_ERASE:
      result = new MavLinkLogErase();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_LOG_REQUEST_END:
      result = new MavLinkLogRequestEnd();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_GPS_INJECT_DATA:
      result = new MavLinkGpsInjectData();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_GPS2_RAW:
      result = new MavLinkGps2Raw();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_POWER_STATUS:
      result = new MavLinkPowerStatus();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SERIAL_CONTROL:
      result = new MavLinkSerialControl();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_GPS_RTK:
      result = new MavLinkGpsRtk();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_GPS2_RTK:
      result = new MavLinkGps2Rtk();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SCALED_IMU3:
      result = new MavLinkScaledImu3();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_DATA_TRANSMISSION_HANDSHAKE:
      result = new MavLinkDataTransmissionHandshake();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_ENCAPSULATED_DATA:
      result = new MavLinkEncapsulatedData();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_DISTANCE_SENSOR:
      result = new MavLinkDistanceSensor();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_TERRAIN_REQUEST:
      result = new MavLinkTerrainRequest();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_TERRAIN_DATA:
      result = new MavLinkTerrainData();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_TERRAIN_CHECK:
      result = new MavLinkTerrainCheck();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_TERRAIN_REPORT:
      result = new MavLinkTerrainReport();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SCALED_PRESSURE2:
      result = new MavLinkScaledPressure2();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_ATT_POS_MOCAP:
      result = new MavLinkAttPosMocap();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SET_ACTUATOR_CONTROL_TARGET:
      result = new MavLinkSetActuatorControlTarget();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_ACTUATOR_CONTROL_TARGET:
      result = new MavLinkActuatorControlTarget();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_ALTITUDE:
      result = new MavLinkAltitude();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_RESOURCE_REQUEST:
      result = new MavLinkResourceRequest();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SCALED_PRESSURE3:
      result = new MavLinkScaledPressure3();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_FOLLOW_TARGET:
      result = new MavLinkFollowTarget();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_CONTROL_SYSTEM_STATE:
      result = new MavLinkControlSystemState();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_BATTERY_STATUS:
      result = new MavLinkBatteryStatus();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_AUTOPILOT_VERSION:
      result = new MavLinkAutopilotVersion();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_LANDING_TARGET:
      result = new MavLinkLandingTarget();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_FENCE_STATUS:
      result = new MavLinkFenceStatus();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MAG_CAL_REPORT:
      result = new MavLinkMagCalReport();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_EFI_STATUS:
      result = new MavLinkEfiStatus();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_ESTIMATOR_STATUS:
      result = new MavLinkEstimatorStatus();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_WIND_COV:
      result = new MavLinkWindCov();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_GPS_INPUT:
      result = new MavLinkGpsInput();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_GPS_RTCM_DATA:
      result = new MavLinkGpsRtcmData();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_HIGH_LATENCY:
      result = new MavLinkHighLatency();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_HIGH_LATENCY2:
      result = new MavLinkHighLatency2();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_VIBRATION:
      result = new MavLinkVibration();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_HOME_POSITION:
      result = new MavLinkHomePosition();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_SET_HOME_POSITION:
      result = new MavLinkSetHomePosition();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MESSAGE_INTERVAL:
      result = new MavLinkMessageInterval();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_EXTENDED_SYS_STATE:
      result = new MavLinkExtendedSysState();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_ADSB_VEHICLE:
      result = new MavLinkAdsbVehicle();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_COLLISION:
      result = new MavLinkCollision();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_V2_EXTENSION:
      result = new MavLinkV2Extension();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_MEMORY_VECT:
      result = new MavLinkMemoryVect();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_DEBUG_VECT:
      result = new MavLinkDebugVect();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_NAMED_VALUE_FLOAT:
      result = new MavLinkNamedValueFloat();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_NAMED_VALUE_INT:
      result = new MavLinkNamedValueInt();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_STATUSTEXT:
      result = new MavLinkStatustext();
      break;
    case MavLinkMessageIds::MAVLINK_MSG_ID_DEBUG:
      result = new MavLinkDebug();
      break;
    default:
      break;
  }
  if (result != nullptr) {
    result->decode(msg);
  }
  return result;
}
