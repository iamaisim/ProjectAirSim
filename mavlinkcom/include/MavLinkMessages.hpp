// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef MavLinkCom_MavLinkMessages_hpp
#define MavLinkCom_MavLinkMessages_hpp

#include <stdint.h>

#include <string>

#include "MavLinkMessageBase.hpp"

namespace mavlinkcom {

enum class MavLinkMessageIds {
  MAVLINK_MSG_ID_HEARTBEAT = 0,
  MAVLINK_MSG_ID_SYS_STATUS = 1,
  MAVLINK_MSG_ID_SYSTEM_TIME = 2,
  MAVLINK_MSG_ID_PING = 4,
  MAVLINK_MSG_ID_CHANGE_OPERATOR_CONTROL = 5,
  MAVLINK_MSG_ID_CHANGE_OPERATOR_CONTROL_ACK = 6,
  MAVLINK_MSG_ID_AUTH_KEY = 7,
  MAVLINK_MSG_ID_LINK_NODE_STATUS = 8,
  MAVLINK_MSG_ID_SET_MODE = 11,
  MAVLINK_MSG_ID_PARAM_REQUEST_READ = 20,
  MAVLINK_MSG_ID_PARAM_REQUEST_LIST = 21,
  MAVLINK_MSG_ID_PARAM_VALUE = 22,
  MAVLINK_MSG_ID_PARAM_SET = 23,
  MAVLINK_MSG_ID_GPS_RAW_INT = 24,
  MAVLINK_MSG_ID_GPS_STATUS = 25,
  MAVLINK_MSG_ID_SCALED_IMU = 26,
  MAVLINK_MSG_ID_RAW_IMU = 27,
  MAVLINK_MSG_ID_RAW_PRESSURE = 28,
  MAVLINK_MSG_ID_SCALED_PRESSURE = 29,
  MAVLINK_MSG_ID_ATTITUDE = 30,
  MAVLINK_MSG_ID_ATTITUDE_QUATERNION = 31,
  MAVLINK_MSG_ID_LOCAL_POSITION_NED = 32,
  MAVLINK_MSG_ID_GLOBAL_POSITION_INT = 33,
  MAVLINK_MSG_ID_RC_CHANNELS_SCALED = 34,
  MAVLINK_MSG_ID_RC_CHANNELS_RAW = 35,
  MAVLINK_MSG_ID_SERVO_OUTPUT_RAW = 36,
  MAVLINK_MSG_ID_MISSION_REQUEST_PARTIAL_LIST = 37,
  MAVLINK_MSG_ID_MISSION_WRITE_PARTIAL_LIST = 38,
  MAVLINK_MSG_ID_MISSION_ITEM = 39,
  MAVLINK_MSG_ID_MISSION_REQUEST = 40,
  MAVLINK_MSG_ID_MISSION_SET_CURRENT = 41,
  MAVLINK_MSG_ID_MISSION_CURRENT = 42,
  MAVLINK_MSG_ID_MISSION_REQUEST_LIST = 43,
  MAVLINK_MSG_ID_MISSION_COUNT = 44,
  MAVLINK_MSG_ID_MISSION_CLEAR_ALL = 45,
  MAVLINK_MSG_ID_MISSION_ITEM_REACHED = 46,
  MAVLINK_MSG_ID_MISSION_ACK = 47,
  MAVLINK_MSG_ID_SET_GPS_GLOBAL_ORIGIN = 48,
  MAVLINK_MSG_ID_GPS_GLOBAL_ORIGIN = 49,
  MAVLINK_MSG_ID_PARAM_MAP_RC = 50,
  MAVLINK_MSG_ID_MISSION_REQUEST_INT = 51,
  MAVLINK_MSG_ID_SAFETY_SET_ALLOWED_AREA = 54,
  MAVLINK_MSG_ID_SAFETY_ALLOWED_AREA = 55,
  MAVLINK_MSG_ID_ATTITUDE_QUATERNION_COV = 61,
  MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT = 62,
  MAVLINK_MSG_ID_GLOBAL_POSITION_INT_COV = 63,
  MAVLINK_MSG_ID_LOCAL_POSITION_NED_COV = 64,
  MAVLINK_MSG_ID_RC_CHANNELS = 65,
  MAVLINK_MSG_ID_REQUEST_DATA_STREAM = 66,
  MAVLINK_MSG_ID_DATA_STREAM = 67,
  MAVLINK_MSG_ID_MANUAL_CONTROL = 69,
  MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE = 70,
  MAVLINK_MSG_ID_MISSION_ITEM_INT = 73,
  MAVLINK_MSG_ID_VFR_HUD = 74,
  MAVLINK_MSG_ID_COMMAND_INT = 75,
  MAVLINK_MSG_ID_COMMAND_LONG = 76,
  MAVLINK_MSG_ID_COMMAND_ACK = 77,
  MAVLINK_MSG_ID_COMMAND_CANCEL = 80,
  MAVLINK_MSG_ID_MANUAL_SETPOINT = 81,
  MAVLINK_MSG_ID_SET_ATTITUDE_TARGET = 82,
  MAVLINK_MSG_ID_ATTITUDE_TARGET = 83,
  MAVLINK_MSG_ID_SET_POSITION_TARGET_LOCAL_NED = 84,
  MAVLINK_MSG_ID_POSITION_TARGET_LOCAL_NED = 85,
  MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT = 86,
  MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT = 87,
  MAVLINK_MSG_ID_LOCAL_POSITION_NED_SYSTEM_GLOBAL_OFFSET = 89,
  MAVLINK_MSG_ID_HIL_STATE = 90,
  MAVLINK_MSG_ID_HIL_CONTROLS = 91,
  MAVLINK_MSG_ID_HIL_RC_INPUTS_RAW = 92,
  MAVLINK_MSG_ID_HIL_ACTUATOR_CONTROLS = 93,
  MAVLINK_MSG_ID_OPTICAL_FLOW = 100,
  MAVLINK_MSG_ID_GLOBAL_VISION_POSITION_ESTIMATE = 101,
  MAVLINK_MSG_ID_VISION_POSITION_ESTIMATE = 102,
  MAVLINK_MSG_ID_VISION_SPEED_ESTIMATE = 103,
  MAVLINK_MSG_ID_VICON_POSITION_ESTIMATE = 104,
  MAVLINK_MSG_ID_HIGHRES_IMU = 105,
  MAVLINK_MSG_ID_OPTICAL_FLOW_RAD = 106,
  MAVLINK_MSG_ID_HIL_SENSOR = 107,
  MAVLINK_MSG_ID_SIM_STATE = 108,
  MAVLINK_MSG_ID_RADIO_STATUS = 109,
  MAVLINK_MSG_ID_FILE_TRANSFER_PROTOCOL = 110,
  MAVLINK_MSG_ID_TIMESYNC = 111,
  MAVLINK_MSG_ID_CAMERA_TRIGGER = 112,
  MAVLINK_MSG_ID_HIL_GPS = 113,
  MAVLINK_MSG_ID_HIL_OPTICAL_FLOW = 114,
  MAVLINK_MSG_ID_HIL_STATE_QUATERNION = 115,
  MAVLINK_MSG_ID_SCALED_IMU2 = 116,
  MAVLINK_MSG_ID_LOG_REQUEST_LIST = 117,
  MAVLINK_MSG_ID_LOG_ENTRY = 118,
  MAVLINK_MSG_ID_LOG_REQUEST_DATA = 119,
  MAVLINK_MSG_ID_LOG_DATA = 120,
  MAVLINK_MSG_ID_LOG_ERASE = 121,
  MAVLINK_MSG_ID_LOG_REQUEST_END = 122,
  MAVLINK_MSG_ID_GPS_INJECT_DATA = 123,
  MAVLINK_MSG_ID_GPS2_RAW = 124,
  MAVLINK_MSG_ID_POWER_STATUS = 125,
  MAVLINK_MSG_ID_SERIAL_CONTROL = 126,
  MAVLINK_MSG_ID_GPS_RTK = 127,
  MAVLINK_MSG_ID_GPS2_RTK = 128,
  MAVLINK_MSG_ID_SCALED_IMU3 = 129,
  MAVLINK_MSG_ID_DATA_TRANSMISSION_HANDSHAKE = 130,
  MAVLINK_MSG_ID_ENCAPSULATED_DATA = 131,
  MAVLINK_MSG_ID_DISTANCE_SENSOR = 132,
  MAVLINK_MSG_ID_TERRAIN_REQUEST = 133,
  MAVLINK_MSG_ID_TERRAIN_DATA = 134,
  MAVLINK_MSG_ID_TERRAIN_CHECK = 135,
  MAVLINK_MSG_ID_TERRAIN_REPORT = 136,
  MAVLINK_MSG_ID_SCALED_PRESSURE2 = 137,
  MAVLINK_MSG_ID_ATT_POS_MOCAP = 138,
  MAVLINK_MSG_ID_SET_ACTUATOR_CONTROL_TARGET = 139,
  MAVLINK_MSG_ID_ACTUATOR_CONTROL_TARGET = 140,
  MAVLINK_MSG_ID_ALTITUDE = 141,
  MAVLINK_MSG_ID_RESOURCE_REQUEST = 142,
  MAVLINK_MSG_ID_SCALED_PRESSURE3 = 143,
  MAVLINK_MSG_ID_FOLLOW_TARGET = 144,
  MAVLINK_MSG_ID_CONTROL_SYSTEM_STATE = 146,
  MAVLINK_MSG_ID_BATTERY_STATUS = 147,
  MAVLINK_MSG_ID_AUTOPILOT_VERSION = 148,
  MAVLINK_MSG_ID_LANDING_TARGET = 149,
  MAVLINK_MSG_ID_FENCE_STATUS = 162,
  MAVLINK_MSG_ID_MAG_CAL_REPORT = 192,
  MAVLINK_MSG_ID_EFI_STATUS = 225,
  MAVLINK_MSG_ID_ESTIMATOR_STATUS = 230,
  MAVLINK_MSG_ID_WIND_COV = 231,
  MAVLINK_MSG_ID_GPS_INPUT = 232,
  MAVLINK_MSG_ID_GPS_RTCM_DATA = 233,
  MAVLINK_MSG_ID_HIGH_LATENCY = 234,
  MAVLINK_MSG_ID_HIGH_LATENCY2 = 235,
  MAVLINK_MSG_ID_VIBRATION = 241,
  MAVLINK_MSG_ID_HOME_POSITION = 242,
  MAVLINK_MSG_ID_SET_HOME_POSITION = 243,
  MAVLINK_MSG_ID_MESSAGE_INTERVAL = 244,
  MAVLINK_MSG_ID_EXTENDED_SYS_STATE = 245,
  MAVLINK_MSG_ID_ADSB_VEHICLE = 246,
  MAVLINK_MSG_ID_COLLISION = 247,
  MAVLINK_MSG_ID_V2_EXTENSION = 248,
  MAVLINK_MSG_ID_MEMORY_VECT = 249,
  MAVLINK_MSG_ID_DEBUG_VECT = 250,
  MAVLINK_MSG_ID_NAMED_VALUE_FLOAT = 251,
  MAVLINK_MSG_ID_NAMED_VALUE_INT = 252,
  MAVLINK_MSG_ID_STATUSTEXT = 253,
  MAVLINK_MSG_ID_DEBUG = 254,
  MAVLINK_MSG_ID_SETUP_SIGNING = 256,
  MAVLINK_MSG_ID_BUTTON_CHANGE = 257,
  MAVLINK_MSG_ID_PLAY_TUNE = 258,
  MAVLINK_MSG_ID_CAMERA_INFORMATION = 259,
  MAVLINK_MSG_ID_CAMERA_SETTINGS = 260,
  MAVLINK_MSG_ID_STORAGE_INFORMATION = 261,
  MAVLINK_MSG_ID_CAMERA_CAPTURE_STATUS = 262,
  MAVLINK_MSG_ID_CAMERA_IMAGE_CAPTURED = 263,
  MAVLINK_MSG_ID_FLIGHT_INFORMATION = 264,
  MAVLINK_MSG_ID_MOUNT_ORIENTATION = 265,
  MAVLINK_MSG_ID_LOGGING_DATA = 266,
  MAVLINK_MSG_ID_LOGGING_DATA_ACKED = 267,
  MAVLINK_MSG_ID_LOGGING_ACK = 268,
  MAVLINK_MSG_ID_VIDEO_STREAM_INFORMATION = 269,
  MAVLINK_MSG_ID_VIDEO_STREAM_STATUS = 270,
  MAVLINK_MSG_ID_CAMERA_FOV_STATUS = 271,
  MAVLINK_MSG_ID_CAMERA_TRACKING_IMAGE_STATUS = 275,
  MAVLINK_MSG_ID_CAMERA_TRACKING_GEO_STATUS = 276,
  MAVLINK_MSG_ID_GIMBAL_MANAGER_INFORMATION = 280,
  MAVLINK_MSG_ID_GIMBAL_MANAGER_STATUS = 281,
  MAVLINK_MSG_ID_GIMBAL_MANAGER_SET_ATTITUDE = 282,
  MAVLINK_MSG_ID_GIMBAL_DEVICE_INFORMATION = 283,
  MAVLINK_MSG_ID_GIMBAL_DEVICE_SET_ATTITUDE = 284,
  MAVLINK_MSG_ID_GIMBAL_DEVICE_ATTITUDE_STATUS = 285,
  MAVLINK_MSG_ID_AUTOPILOT_STATE_FOR_GIMBAL_DEVICE = 286,
  MAVLINK_MSG_ID_GIMBAL_MANAGER_SET_PITCHYAW = 287,
  MAVLINK_MSG_ID_GIMBAL_MANAGER_SET_MANUAL_CONTROL = 288,
  MAVLINK_MSG_ID_ESC_INFO = 290,
  MAVLINK_MSG_ID_ESC_STATUS = 291,
  MAVLINK_MSG_ID_WIFI_CONFIG_AP = 299,
  MAVLINK_MSG_ID_PROTOCOL_VERSION = 300,
  MAVLINK_MSG_ID_AIS_VESSEL = 301,
  MAVLINK_MSG_ID_UAVCAN_NODE_STATUS = 310,
  MAVLINK_MSG_ID_UAVCAN_NODE_INFO = 311,
  MAVLINK_MSG_ID_PARAM_EXT_REQUEST_READ = 320,
  MAVLINK_MSG_ID_PARAM_EXT_REQUEST_LIST = 321,
  MAVLINK_MSG_ID_PARAM_EXT_VALUE = 322,
  MAVLINK_MSG_ID_PARAM_EXT_SET = 323,
  MAVLINK_MSG_ID_PARAM_EXT_ACK = 324,
  MAVLINK_MSG_ID_OBSTACLE_DISTANCE = 330,
  MAVLINK_MSG_ID_ODOMETRY = 331,
  MAVLINK_MSG_ID_TRAJECTORY_REPRESENTATION_WAYPOINTS = 332,
  MAVLINK_MSG_ID_TRAJECTORY_REPRESENTATION_BEZIER = 333,
  MAVLINK_MSG_ID_CELLULAR_STATUS = 334,
  MAVLINK_MSG_ID_ISBD_LINK_STATUS = 335,
  MAVLINK_MSG_ID_CELLULAR_CONFIG = 336,
  MAVLINK_MSG_ID_RAW_RPM = 339,
  MAVLINK_MSG_ID_UTM_GLOBAL_POSITION = 340,
  MAVLINK_MSG_ID_DEBUG_FLOAT_ARRAY = 350,
  MAVLINK_MSG_ID_ORBIT_EXECUTION_STATUS = 360,
  MAVLINK_MSG_ID_SMART_BATTERY_INFO = 370,
  MAVLINK_MSG_ID_GENERATOR_STATUS = 373,
  MAVLINK_MSG_ID_ACTUATOR_OUTPUT_STATUS = 375,
  MAVLINK_MSG_ID_TIME_ESTIMATE_TO_TARGET = 380,
  MAVLINK_MSG_ID_TUNNEL = 385,
  MAVLINK_MSG_ID_CAN_FRAME = 386,
  MAVLINK_MSG_ID_ONBOARD_COMPUTER_STATUS = 390,
  MAVLINK_MSG_ID_COMPONENT_INFORMATION = 395,
  MAVLINK_MSG_ID_COMPONENT_METADATA = 397,
  MAVLINK_MSG_ID_PLAY_TUNE_V2 = 400,
  MAVLINK_MSG_ID_SUPPORTED_TUNES = 401,
  MAVLINK_MSG_ID_EVENT = 410,
  MAVLINK_MSG_ID_CURRENT_EVENT_SEQUENCE = 411,
  MAVLINK_MSG_ID_REQUEST_EVENT = 412,
  MAVLINK_MSG_ID_RESPONSE_EVENT_ERROR = 413,
  MAVLINK_MSG_ID_CANFD_FRAME = 387,
  MAVLINK_MSG_ID_CAN_FILTER_MODIFY = 388,
  MAVLINK_MSG_ID_WHEEL_DISTANCE = 9000,
  MAVLINK_MSG_ID_WINCH_STATUS = 9005,
  MAVLINK_MSG_ID_OPEN_DRONE_ID_BASIC_ID = 12900,
  MAVLINK_MSG_ID_OPEN_DRONE_ID_LOCATION = 12901,
  MAVLINK_MSG_ID_OPEN_DRONE_ID_AUTHENTICATION = 12902,
  MAVLINK_MSG_ID_OPEN_DRONE_ID_SELF_ID = 12903,
  MAVLINK_MSG_ID_OPEN_DRONE_ID_SYSTEM = 12904,
  MAVLINK_MSG_ID_OPEN_DRONE_ID_OPERATOR_ID = 12905,
  MAVLINK_MSG_ID_OPEN_DRONE_ID_MESSAGE_PACK = 12915,
  MAVLINK_MSG_ID_HYGROMETER_SENSOR = 12920
};
// Micro air vehicle / autopilot classes. This identifies the individual model.
enum class MAV_AUTOPILOT {
  // Generic autopilot, full support for everything
  MAV_AUTOPILOT_GENERIC = 0,
  // Reserved for future use.
  MAV_AUTOPILOT_RESERVED = 1,
  // SLUGS autopilot, http://slugsuav.soe.ucsc.edu
  MAV_AUTOPILOT_SLUGS = 2,
  // ArduPilot - Plane/Copter/Rover/Sub/Tracker, https://ardupilot.org
  MAV_AUTOPILOT_ARDUPILOTMEGA = 3,
  // OpenPilot, http://openpilot.org
  MAV_AUTOPILOT_OPENPILOT = 4,
  // Generic autopilot only supporting simple waypoints
  MAV_AUTOPILOT_GENERIC_WAYPOINTS_ONLY = 5,
  // Generic autopilot supporting waypoints and other simple navigation commands
  MAV_AUTOPILOT_GENERIC_WAYPOINTS_AND_SIMPLE_NAVIGATION_ONLY = 6,
  // Generic autopilot supporting the full mission command set
  MAV_AUTOPILOT_GENERIC_MISSION_FULL = 7,
  // No valid autopilot, e.g. a GCS or other MAVLink component
  MAV_AUTOPILOT_INVALID = 8,
  // PPZ UAV - http://nongnu.org/paparazzi
  MAV_AUTOPILOT_PPZ = 9,
  // UAV Dev Board
  MAV_AUTOPILOT_UDB = 10,
  // FlexiPilot
  MAV_AUTOPILOT_FP = 11,
  // PX4 Autopilot - http://px4.io/
  MAV_AUTOPILOT_PX4 = 12,
  // SMACCMPilot - http://smaccmpilot.org
  MAV_AUTOPILOT_SMACCMPILOT = 13,
  // AutoQuad -- http://autoquad.org
  MAV_AUTOPILOT_AUTOQUAD = 14,
  // Armazila -- http://armazila.com
  MAV_AUTOPILOT_ARMAZILA = 15,
  // Aerob -- http://aerob.ru
  MAV_AUTOPILOT_AEROB = 16,
  // ASLUAV autopilot -- http://www.asl.ethz.ch
  MAV_AUTOPILOT_ASLUAV = 17,
  // SmartAP Autopilot - http://sky-drones.com
  MAV_AUTOPILOT_SMARTAP = 18,
  // AirRails - http://uaventure.com
  MAV_AUTOPILOT_AIRRAILS = 19,
  // Fusion Reflex - https://fusion.engineering
  MAV_AUTOPILOT_REFLEX = 20
};

// MAVLINK component type reported in HEARTBEAT message. Flight controllers must
// report the type of the vehicle on which they are mounted (e.g.
// MAV_TYPE_OCTOROTOR). All other components must report a value appropriate for
// their type (e.g. a camera must use MAV_TYPE_CAMERA).
enum class MAV_TYPE {
  // Generic micro air vehicle
  MAV_TYPE_GENERIC = 0,
  // Fixed wing aircraft.
  MAV_TYPE_FIXED_WING = 1,
  // Quadrotor
  MAV_TYPE_QUADROTOR = 2,
  // Coaxial helicopter
  MAV_TYPE_COAXIAL = 3,
  // Normal helicopter with tail rotor.
  MAV_TYPE_HELICOPTER = 4,
  // Ground installation
  MAV_TYPE_ANTENNA_TRACKER = 5,
  // Operator control unit / ground control station
  MAV_TYPE_GCS = 6,
  // Airship, controlled
  MAV_TYPE_AIRSHIP = 7,
  // Free balloon, uncontrolled
  MAV_TYPE_FREE_BALLOON = 8,
  // Rocket
  MAV_TYPE_ROCKET = 9,
  // Ground rover
  MAV_TYPE_GROUND_ROVER = 10,
  // Surface vessel, boat, ship
  MAV_TYPE_SURFACE_BOAT = 11,
  // Submarine
  MAV_TYPE_SUBMARINE = 12,
  // Hexarotor
  MAV_TYPE_HEXAROTOR = 13,
  // Octorotor
  MAV_TYPE_OCTOROTOR = 14,
  // Tricopter
  MAV_TYPE_TRICOPTER = 15,
  // Flapping wing
  MAV_TYPE_FLAPPING_WING = 16,
  // Kite
  MAV_TYPE_KITE = 17,
  // Onboard companion controller
  MAV_TYPE_ONBOARD_CONTROLLER = 18,
  // Two-rotor Tailsitter VTOL that additionally uses control surfaces in
  // vertical
  // operation. Note, value previously named MAV_TYPE_VTOL_DUOROTOR.
  MAV_TYPE_VTOL_TAILSITTER_DUOROTOR = 19,
  // Quad-rotor Tailsitter VTOL using a V-shaped quad config in vertical
  // operation.
  // Note: value previously named MAV_TYPE_VTOL_QUADROTOR.
  MAV_TYPE_VTOL_TAILSITTER_QUADROTOR = 20,
  // Tiltrotor VTOL. Fuselage and wings stay (nominally) horizontal in all
  // flight
  // phases. It able to tilt (some) rotors to provide thrust in cruise flight.
  MAV_TYPE_VTOL_TILTROTOR = 21,
  // VTOL with separate fixed rotors for hover and cruise flight. Fuselage and
  // wings
  // stay (nominally) horizontal in all flight phases.
  MAV_TYPE_VTOL_FIXEDROTOR = 22,
  // Tailsitter VTOL. Fuselage and wings orientation changes depending on flight
  // phase: vertical for hover, horizontal for cruise. Use more specific VTOL
  // MAV_TYPE_VTOL_DUOROTOR
  // or MAV_TYPE_VTOL_QUADROTOR if appropriate.
  MAV_TYPE_VTOL_TAILSITTER = 23,
  // VTOL reserved 4
  MAV_TYPE_VTOL_RESERVED4 = 24,
  // VTOL reserved 5
  MAV_TYPE_VTOL_RESERVED5 = 25,
  // Gimbal
  MAV_TYPE_GIMBAL = 26,
  // ADSB system
  MAV_TYPE_ADSB = 27,
  // Steerable, nonrigid airfoil
  MAV_TYPE_PARAFOIL = 28,
  // Dodecarotor
  MAV_TYPE_DODECAROTOR = 29,
  // Camera
  MAV_TYPE_CAMERA = 30,
  // Charging station
  MAV_TYPE_CHARGING_STATION = 31,
  // FLARM collision avoidance system
  MAV_TYPE_FLARM = 32,
  // Servo
  MAV_TYPE_SERVO = 33,
  // Open Drone ID. See https://mavlink.io/en/services/opendroneid.html.
  MAV_TYPE_ODID = 34,
  // Decarotor
  MAV_TYPE_DECAROTOR = 35,
  // Battery
  MAV_TYPE_BATTERY = 36,
  // Parachute
  MAV_TYPE_PARACHUTE = 37,
  // Log
  MAV_TYPE_LOG = 38,
  // OSD
  MAV_TYPE_OSD = 39,
  // IMU
  MAV_TYPE_IMU = 40,
  // GPS
  MAV_TYPE_GPS = 41,
  // Winch
  MAV_TYPE_WINCH = 42
};

// These flags encode the MAV mode.
enum class MAV_MODE_FLAG {
  // 0b10000000 MAV safety set to armed. Motors are enabled / running / can
  // start.
  // Ready to fly. Additional note: this flag is to be ignore when sent in the
  // command
  // MAV_CMD_DO_SET_MODE and MAV_CMD_COMPONENT_ARM_DISARM shall be used instead.
  // The flag can still be used to report the armed state.
  MAV_MODE_FLAG_SAFETY_ARMED = 128,
  // 0b01000000 remote control input is enabled.
  MAV_MODE_FLAG_MANUAL_INPUT_ENABLED = 64,
  // 0b00100000 hardware in the loop simulation. All motors / actuators are
  // blocked,
  // but internal software is full operational.
  MAV_MODE_FLAG_HIL_ENABLED = 32,
  // 0b00010000 system stabilizes electronically its attitude (and optionally
  // position).
  // It needs however further control inputs to move around.
  MAV_MODE_FLAG_STABILIZE_ENABLED = 16,
  // 0b00001000 guided mode enabled, system flies waypoints / mission items.
  MAV_MODE_FLAG_GUIDED_ENABLED = 8,
  // 0b00000100 autonomous mode enabled, system finds its own goal positions.
  // Guided
  // flag can be set or not, depends on the actual implementation.
  MAV_MODE_FLAG_AUTO_ENABLED = 4,
  // 0b00000010 system has a test mode enabled. This flag is intended for
  // temporary
  // system tests and should not be used for stable implementations.
  MAV_MODE_FLAG_TEST_ENABLED = 2,
  // 0b00000001 Reserved for future use.
  MAV_MODE_FLAG_CUSTOM_MODE_ENABLED = 1
};

// These values encode the bit positions of the decode position. These values
// can be used to read the value of a flag bit by combining the base_mode
// variable with AND with the flag position value. The result will be either 0
// or 1, depending on if the flag is set or not.
enum class MAV_MODE_FLAG_DECODE_POSITION {
  // First bit: 10000000
  MAV_MODE_FLAG_DECODE_POSITION_SAFETY = 128,
  // Second bit: 01000000
  MAV_MODE_FLAG_DECODE_POSITION_MANUAL = 64,
  // Third bit: 00100000
  MAV_MODE_FLAG_DECODE_POSITION_HIL = 32,
  // Fourth bit: 00010000
  MAV_MODE_FLAG_DECODE_POSITION_STABILIZE = 16,
  // Fifth bit: 00001000
  MAV_MODE_FLAG_DECODE_POSITION_GUIDED = 8,
  // Sixth bit: 00000100
  MAV_MODE_FLAG_DECODE_POSITION_AUTO = 4,
  // Seventh bit: 00000010
  MAV_MODE_FLAG_DECODE_POSITION_TEST = 2,
  // Eighth bit: 00000001
  MAV_MODE_FLAG_DECODE_POSITION_CUSTOM_MODE = 1
};

enum class MAV_STATE {
  // Uninitialized system, state is unknown.
  MAV_STATE_UNINIT = 0,
  // System is booting up.
  MAV_STATE_BOOT = 1,
  // System is calibrating and not flight-ready.
  MAV_STATE_CALIBRATING = 2,
  // System is grounded and on standby. It can be launched any time.
  MAV_STATE_STANDBY = 3,
  // System is active and might be already airborne. Motors are engaged.
  MAV_STATE_ACTIVE = 4,
  // System is in a non-normal flight mode. It can however still navigate.
  MAV_STATE_CRITICAL = 5,
  // System is in a non-normal flight mode. It lost control over parts or over
  // the
  // whole airframe. It is in mayday and going down.
  MAV_STATE_EMERGENCY = 6,
  // System just initialized its power-down sequence, will shut down now.
  MAV_STATE_POWEROFF = 7,
  // System is terminating itself.
  MAV_STATE_FLIGHT_TERMINATION = 8
};

// Component ids (values) for the different types and instances of onboard
// hardware/software that might make up a MAVLink system (autopilot, cameras,
// servos, GPS systems, avoidance systems etc.). Components must use the
// appropriate ID in their source address when sending messages. Components can
// also use IDs to determine if they are the intended recipient of an incoming
// message. The MAV_COMP_ID_ALL value is used to indicate messages that must be
// processed by all components. When creating new entries, components that can
// have multiple instances (e.g. cameras, servos etc.) should be allocated
// sequential values. An appropriate number of values should be left free after
// these components to allow the number of instances to be expanded.
enum class MAV_COMPONENT {
  // Target id (target_component) used to broadcast messages to all components
  // of
  // the receiving system. Components should attempt to process messages with
  // this
  // component ID and forward to components on any other interfaces. Note: This
  // is not a valid *source* component id for a message.
  MAV_COMP_ID_ALL = 0,
  // System flight controller component ("autopilot"). Only one autopilot is
  // expected
  // in a particular system.
  MAV_COMP_ID_AUTOPILOT1 = 1,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER1 = 25,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER2 = 26,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER3 = 27,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER4 = 28,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER5 = 29,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER6 = 30,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER7 = 31,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER8 = 32,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER9 = 33,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER10 = 34,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER11 = 35,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER12 = 36,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER13 = 37,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER14 = 38,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER15 = 39,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER16 = 40,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER17 = 41,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER18 = 42,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER19 = 43,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER20 = 44,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER21 = 45,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER22 = 46,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER23 = 47,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER24 = 48,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER25 = 49,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER26 = 50,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER27 = 51,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER28 = 52,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER29 = 53,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER30 = 54,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER31 = 55,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER32 = 56,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER33 = 57,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER34 = 58,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER35 = 59,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER36 = 60,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER37 = 61,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER38 = 62,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER39 = 63,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER40 = 64,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER41 = 65,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER42 = 66,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER43 = 67,
  // Telemetry radio (e.g. SiK radio, or other component that emits RADIO_STATUS
  // messages).
  MAV_COMP_ID_TELEMETRY_RADIO = 68,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER45 = 69,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER46 = 70,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER47 = 71,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER48 = 72,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER49 = 73,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER50 = 74,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER51 = 75,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER52 = 76,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER53 = 77,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER54 = 78,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER55 = 79,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER56 = 80,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER57 = 81,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER58 = 82,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER59 = 83,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER60 = 84,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER61 = 85,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER62 = 86,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER63 = 87,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER64 = 88,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER65 = 89,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER66 = 90,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER67 = 91,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER68 = 92,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER69 = 93,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER70 = 94,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER71 = 95,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER72 = 96,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER73 = 97,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER74 = 98,
  // Id for a component on privately managed MAVLink network. Can be used for
  // any
  // purpose but may not be published by components outside of the private
  // network.
  MAV_COMP_ID_USER75 = 99,
  // Camera #1.
  MAV_COMP_ID_CAMERA = 100,
  // Camera #2.
  MAV_COMP_ID_CAMERA2 = 101,
  // Camera #3.
  MAV_COMP_ID_CAMERA3 = 102,
  // Camera #4.
  MAV_COMP_ID_CAMERA4 = 103,
  // Camera #5.
  MAV_COMP_ID_CAMERA5 = 104,
  // Camera #6.
  MAV_COMP_ID_CAMERA6 = 105,
  // Servo #1.
  MAV_COMP_ID_SERVO1 = 140,
  // Servo #2.
  MAV_COMP_ID_SERVO2 = 141,
  // Servo #3.
  MAV_COMP_ID_SERVO3 = 142,
  // Servo #4.
  MAV_COMP_ID_SERVO4 = 143,
  // Servo #5.
  MAV_COMP_ID_SERVO5 = 144,
  // Servo #6.
  MAV_COMP_ID_SERVO6 = 145,
  // Servo #7.
  MAV_COMP_ID_SERVO7 = 146,
  // Servo #8.
  MAV_COMP_ID_SERVO8 = 147,
  // Servo #9.
  MAV_COMP_ID_SERVO9 = 148,
  // Servo #10.
  MAV_COMP_ID_SERVO10 = 149,
  // Servo #11.
  MAV_COMP_ID_SERVO11 = 150,
  // Servo #12.
  MAV_COMP_ID_SERVO12 = 151,
  // Servo #13.
  MAV_COMP_ID_SERVO13 = 152,
  // Servo #14.
  MAV_COMP_ID_SERVO14 = 153,
  // Gimbal #1.
  MAV_COMP_ID_GIMBAL = 154,
  // Logging component.
  MAV_COMP_ID_LOG = 155,
  // Automatic Dependent Surveillance-Broadcast (ADS-B) component.
  MAV_COMP_ID_ADSB = 156,
  // On Screen Display (OSD) devices for video links.
  MAV_COMP_ID_OSD = 157,
  // Generic autopilot peripheral component ID. Meant for devices that do not
  // implement
  // the parameter microservice.
  MAV_COMP_ID_PERIPHERAL = 158,
  // Gimbal ID for QX1.
  MAV_COMP_ID_QX1_GIMBAL = 159,
  // FLARM collision alert component.
  MAV_COMP_ID_FLARM = 160,
  // Parachute component.
  MAV_COMP_ID_PARACHUTE = 161,
  // Gimbal #2.
  MAV_COMP_ID_GIMBAL2 = 171,
  // Gimbal #3.
  MAV_COMP_ID_GIMBAL3 = 172,
  // Gimbal #4
  MAV_COMP_ID_GIMBAL4 = 173,
  // Gimbal #5.
  MAV_COMP_ID_GIMBAL5 = 174,
  // Gimbal #6.
  MAV_COMP_ID_GIMBAL6 = 175,
  // Battery #1.
  MAV_COMP_ID_BATTERY = 180,
  // Battery #2.
  MAV_COMP_ID_BATTERY2 = 181,
  // CAN over MAVLink client.
  MAV_COMP_ID_MAVCAN = 189,
  // Component that can generate/supply a mission flight plan (e.g. GCS or
  // developer
  // API).
  MAV_COMP_ID_MISSIONPLANNER = 190,
  // Component that lives on the onboard computer (companion computer) and has
  // some
  // generic functionalities, such as settings system parameters and monitoring
  // the status of some processes that don't directly speak mavlink and so on.
  MAV_COMP_ID_ONBOARD_COMPUTER = 191,
  // Component that lives on the onboard computer (companion computer) and has
  // some
  // generic functionalities, such as settings system parameters and monitoring
  // the status of some processes that don't directly speak mavlink and so on.
  MAV_COMP_ID_ONBOARD_COMPUTER2 = 192,
  // Component that lives on the onboard computer (companion computer) and has
  // some
  // generic functionalities, such as settings system parameters and monitoring
  // the status of some processes that don't directly speak mavlink and so on.
  MAV_COMP_ID_ONBOARD_COMPUTER3 = 193,
  // Component that lives on the onboard computer (companion computer) and has
  // some
  // generic functionalities, such as settings system parameters and monitoring
  // the status of some processes that don't directly speak mavlink and so on.
  MAV_COMP_ID_ONBOARD_COMPUTER4 = 194,
  // Component that finds an optimal path between points based on a certain
  // constraint
  // (e.g. minimum snap, shortest path, cost, etc.).
  MAV_COMP_ID_PATHPLANNER = 195,
  // Component that plans a collision free path between two points.
  MAV_COMP_ID_OBSTACLE_AVOIDANCE = 196,
  // Component that provides position estimates using VIO techniques.
  MAV_COMP_ID_VISUAL_INERTIAL_ODOMETRY = 197,
  // Component that manages pairing of vehicle and GCS.
  MAV_COMP_ID_PAIRING_MANAGER = 198,
  // Inertial Measurement Unit (IMU) #1.
  MAV_COMP_ID_IMU = 200,
  // Inertial Measurement Unit (IMU) #2.
  MAV_COMP_ID_IMU_2 = 201,
  // Inertial Measurement Unit (IMU) #3.
  MAV_COMP_ID_IMU_3 = 202,
  // GPS #1.
  MAV_COMP_ID_GPS = 220,
  // GPS #2.
  MAV_COMP_ID_GPS2 = 221,
  // Open Drone ID transmitter/receiver (Bluetooth/WiFi/Internet).
  MAV_COMP_ID_ODID_TXRX_1 = 236,
  // Open Drone ID transmitter/receiver (Bluetooth/WiFi/Internet).
  MAV_COMP_ID_ODID_TXRX_2 = 237,
  // Open Drone ID transmitter/receiver (Bluetooth/WiFi/Internet).
  MAV_COMP_ID_ODID_TXRX_3 = 238,
  // Component to bridge MAVLink to UDP (i.e. from a UART).
  MAV_COMP_ID_UDP_BRIDGE = 240,
  // Component to bridge to UART (i.e. from UDP).
  MAV_COMP_ID_UART_BRIDGE = 241,
  // Component handling TUNNEL messages (e.g. vendor specific GUI of a
  // component).
  MAV_COMP_ID_TUNNEL_NODE = 242,
  // Component for handling system messages (e.g. to ARM, takeoff, etc.).
  MAV_COMP_ID_SYSTEM_CONTROL = 250
};

// These values define the type of firmware release. These values indicate the
// first version or release of this type. For example the first alpha release
// would be 64, the second would be 65.
enum class FIRMWARE_VERSION_TYPE {
  // development release
  FIRMWARE_VERSION_TYPE_DEV = 0,
  // alpha release
  FIRMWARE_VERSION_TYPE_ALPHA = 64,
  // beta release
  FIRMWARE_VERSION_TYPE_BETA = 128,
  // release candidate
  FIRMWARE_VERSION_TYPE_RC = 192,
  // official stable release
  FIRMWARE_VERSION_TYPE_OFFICIAL = 255
};

// Flags to report failure cases over the high latency telemtry.
enum class HL_FAILURE_FLAG {
  // GPS failure.
  HL_FAILURE_FLAG_GPS = 1,
  // Differential pressure sensor failure.
  HL_FAILURE_FLAG_DIFFERENTIAL_PRESSURE = 2,
  // Absolute pressure sensor failure.
  HL_FAILURE_FLAG_ABSOLUTE_PRESSURE = 4,
  // Accelerometer sensor failure.
  HL_FAILURE_FLAG_3D_ACCEL = 8,
  // Gyroscope sensor failure.
  HL_FAILURE_FLAG_3D_GYRO = 16,
  // Magnetometer sensor failure.
  HL_FAILURE_FLAG_3D_MAG = 32,
  // Terrain subsystem failure.
  HL_FAILURE_FLAG_TERRAIN = 64,
  // Battery failure/critical low battery.
  HL_FAILURE_FLAG_BATTERY = 128,
  // RC receiver failure/no rc connection.
  HL_FAILURE_FLAG_RC_RECEIVER = 256,
  // Offboard link failure.
  HL_FAILURE_FLAG_OFFBOARD_LINK = 512,
  // Engine failure.
  HL_FAILURE_FLAG_ENGINE = 1024,
  // Geofence violation.
  HL_FAILURE_FLAG_GEOFENCE = 2048,
  // Estimator failure, for example measurement rejection or large variances.
  HL_FAILURE_FLAG_ESTIMATOR = 4096,
  // Mission failure.
  HL_FAILURE_FLAG_MISSION = 8192
};

// Actions that may be specified in MAV_CMD_OVERRIDE_GOTO to override mission
// execution.
enum class MAV_GOTO {
  // Hold at the current position.
  MAV_GOTO_DO_HOLD = 0,
  // Continue with the next item in mission execution.
  MAV_GOTO_DO_CONTINUE = 1,
  // Hold at the current position of the system
  MAV_GOTO_HOLD_AT_CURRENT_POSITION = 2,
  // Hold at the position specified in the parameters of the DO_HOLD action
  MAV_GOTO_HOLD_AT_SPECIFIED_POSITION = 3
};

// These defines are predefined OR-combined mode flags. There is no need to use
// values from this enum, but it simplifies the use of the mode flags. Note that
// manual input is enabled in all modes as a safety override.
enum class MAV_MODE {
  // System is not ready to fly, booting, calibrating, etc. No flag is set.
  MAV_MODE_PREFLIGHT = 0,
  // System is allowed to be active, under assisted RC control.
  MAV_MODE_STABILIZE_DISARMED = 80,
  // System is allowed to be active, under assisted RC control.
  MAV_MODE_STABILIZE_ARMED = 208,
  // System is allowed to be active, under manual (RC) control, no stabilization
  MAV_MODE_MANUAL_DISARMED = 64,
  // System is allowed to be active, under manual (RC) control, no stabilization
  MAV_MODE_MANUAL_ARMED = 192,
  // System is allowed to be active, under autonomous control, manual setpoint
  MAV_MODE_GUIDED_DISARMED = 88,
  // System is allowed to be active, under autonomous control, manual setpoint
  MAV_MODE_GUIDED_ARMED = 216,
  // System is allowed to be active, under autonomous control and navigation
  // (the
  // trajectory is decided onboard and not pre-programmed by waypoints)
  MAV_MODE_AUTO_DISARMED = 92,
  // System is allowed to be active, under autonomous control and navigation
  // (the
  // trajectory is decided onboard and not pre-programmed by waypoints)
  MAV_MODE_AUTO_ARMED = 220,
  // UNDEFINED mode. This solely depends on the autopilot - use with caution,
  // intended
  // for developers only.
  MAV_MODE_TEST_DISARMED = 66,
  // UNDEFINED mode. This solely depends on the autopilot - use with caution,
  // intended
  // for developers only.
  MAV_MODE_TEST_ARMED = 194
};

// These encode the sensors whose status is sent as part of the SYS_STATUS
// message.
enum class MAV_SYS_STATUS_SENSOR {
  // 0x01 3D gyro
  MAV_SYS_STATUS_SENSOR_3D_GYRO = 1,
  // 0x02 3D accelerometer
  MAV_SYS_STATUS_SENSOR_3D_ACCEL = 2,
  // 0x04 3D magnetometer
  MAV_SYS_STATUS_SENSOR_3D_MAG = 4,
  // 0x08 absolute pressure
  MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE = 8,
  // 0x10 differential pressure
  MAV_SYS_STATUS_SENSOR_DIFFERENTIAL_PRESSURE = 16,
  // 0x20 GPS
  MAV_SYS_STATUS_SENSOR_GPS = 32,
  // 0x40 optical flow
  MAV_SYS_STATUS_SENSOR_OPTICAL_FLOW = 64,
  // 0x80 computer vision position
  MAV_SYS_STATUS_SENSOR_VISION_POSITION = 128,
  // 0x100 laser based position
  MAV_SYS_STATUS_SENSOR_LASER_POSITION = 256,
  // 0x200 external ground truth (Vicon or Leica)
  MAV_SYS_STATUS_SENSOR_EXTERNAL_GROUND_TRUTH = 512,
  // 0x400 3D angular rate control
  MAV_SYS_STATUS_SENSOR_ANGULAR_RATE_CONTROL = 1024,
  // 0x800 attitude stabilization
  MAV_SYS_STATUS_SENSOR_ATTITUDE_STABILIZATION = 2048,
  // 0x1000 yaw position
  MAV_SYS_STATUS_SENSOR_YAW_POSITION = 4096,
  // 0x2000 z/altitude control
  MAV_SYS_STATUS_SENSOR_Z_ALTITUDE_CONTROL = 8192,
  // 0x4000 x/y position control
  MAV_SYS_STATUS_SENSOR_XY_POSITION_CONTROL = 16384,
  // 0x8000 motor outputs / control
  MAV_SYS_STATUS_SENSOR_MOTOR_OUTPUTS = 32768,
  // 0x10000 rc receiver
  MAV_SYS_STATUS_SENSOR_RC_RECEIVER = 65536,
  // 0x20000 2nd 3D gyro
  MAV_SYS_STATUS_SENSOR_3D_GYRO2 = 131072,
  // 0x40000 2nd 3D accelerometer
  MAV_SYS_STATUS_SENSOR_3D_ACCEL2 = 262144,
  // 0x80000 2nd 3D magnetometer
  MAV_SYS_STATUS_SENSOR_3D_MAG2 = 524288,
  // 0x100000 geofence
  MAV_SYS_STATUS_GEOFENCE = 1048576,
  // 0x200000 AHRS subsystem health
  MAV_SYS_STATUS_AHRS = 2097152,
  // 0x400000 Terrain subsystem health
  MAV_SYS_STATUS_TERRAIN = 4194304,
  // 0x800000 Motors are reversed
  MAV_SYS_STATUS_REVERSE_MOTOR = 8388608,
  // 0x1000000 Logging
  MAV_SYS_STATUS_LOGGING = 16777216,
  // 0x2000000 Battery
  MAV_SYS_STATUS_SENSOR_BATTERY = 33554432,
  // 0x4000000 Proximity
  MAV_SYS_STATUS_SENSOR_PROXIMITY = 67108864,
  // 0x8000000 Satellite Communication
  MAV_SYS_STATUS_SENSOR_SATCOM = 134217728,
  // 0x10000000 pre-arm check status. Always healthy when armed
  MAV_SYS_STATUS_PREARM_CHECK = 268435456,
  // 0x20000000 Avoidance/collision prevention
  MAV_SYS_STATUS_OBSTACLE_AVOIDANCE = 536870912,
  // 0x40000000 propulsion (actuator, esc, motor or propellor)
  MAV_SYS_STATUS_SENSOR_PROPULSION = 1073741824,
  // 0x80000000 Extended bit-field are used for further sensor status bits
  // (needs
  // to be set in onboard_control_sensors_present only)
  // MAV_SYS_STATUS_EXTENSION_USED = 2147483648
};

// These encode the sensors whose status is sent as part of the SYS_STATUS
// message in the extended fields.
enum class MAV_SYS_STATUS_SENSOR_EXTENDED {
  // 0x01 Recovery system (parachute, balloon, retracts etc)
  MAV_SYS_STATUS_RECOVERY_SYSTEM = 1
};

// Co-ordinate frames used by MAVLink. Not all frames are supported by all
// commands, messages, or vehicles. Global frames use the following naming
// conventions: - "GLOBAL": Global co-ordinate frame with WGS84
// latitude/longitude and altitude positive over mean sea level (MSL) by
// default. The following modifiers may be used with "GLOBAL":
// - "RELATIVE_ALT": Altitude is relative to the vehicle home position rather
// than MSL. - "TERRAIN_ALT": Altitude is relative to ground level rather than
// MSL. - "INT": Latitude/longitude (in degrees) are scaled by multiplying by
// 1E7. Local frames use the following naming conventions: - "LOCAL": Origin of
// local frame is fixed relative to earth. Unless otherwise specified this
// origin is the origin of the vehicle position-estimator ("EKF"). - "BODY":
// Origin of local frame travels with the vehicle. NOTE, "BODY" does NOT
// indicate alignment of frame axis with vehicle attitude. - "OFFSET":
// Deprecated synonym for "BODY" (origin travels with the vehicle). Not to be
// used for new frames. Some deprecated frames do not follow these conventions
// (e.g. MAV_FRAME_BODY_NED and MAV_FRAME_BODY_OFFSET_NED).
enum class MAV_FRAME {
  // Global (WGS84) coordinate frame + MSL altitude. First value / x: latitude,
  // second value / y: longitude, third value / z: positive altitude over mean
  // sea
  // level (MSL).
  MAV_FRAME_GLOBAL = 0,
  // NED local tangent frame (x: North, y: East, z: Down) with origin fixed
  // relative
  // to earth.
  MAV_FRAME_LOCAL_NED = 1,
  // NOT a coordinate frame, indicates a mission command.
  MAV_FRAME_MISSION = 2,
  // Global (WGS84) coordinate frame + altitude relative to the home position.
  // First
  // value / x: latitude, second value / y: longitude, third value / z: positive
  // altitude with 0 being at the altitude of the home position.
  MAV_FRAME_GLOBAL_RELATIVE_ALT = 3,
  // ENU local tangent frame (x: East, y: North, z: Up) with origin fixed
  // relative
  // to earth.
  MAV_FRAME_LOCAL_ENU = 4,
  // Global (WGS84) coordinate frame (scaled) + MSL altitude. First value / x:
  // latitude
  // in degrees*1E7, second value / y: longitude in degrees*1E7, third value /
  // z:
  // positive altitude over mean sea level (MSL).
  MAV_FRAME_GLOBAL_INT = 5,
  // Global (WGS84) coordinate frame (scaled) + altitude relative to the home
  // position.
  // First value / x: latitude in degrees*1E7, second value / y: longitude in
  // degrees*1E7,
  // third value / z: positive altitude with 0 being at the altitude of the home
  // position.
  MAV_FRAME_GLOBAL_RELATIVE_ALT_INT = 6,
  // NED local tangent frame (x: North, y: East, z: Down) with origin that
  // travels
  // with the vehicle.
  MAV_FRAME_LOCAL_OFFSET_NED = 7,
  // Same as MAV_FRAME_LOCAL_NED when used to represent position values. Same as
  // MAV_FRAME_BODY_FRD when used with velocity/accelaration values.
  MAV_FRAME_BODY_NED = 8,
  // This is the same as MAV_FRAME_BODY_FRD.
  MAV_FRAME_BODY_OFFSET_NED = 9,
  // Global (WGS84) coordinate frame with AGL altitude (at the waypoint
  // coordinate).
  // First value / x: latitude in degrees, second value / y: longitude in
  // degrees,
  // third value / z: positive altitude in meters with 0 being at ground level
  // in
  // terrain model.
  MAV_FRAME_GLOBAL_TERRAIN_ALT = 10,
  // Global (WGS84) coordinate frame (scaled) with AGL altitude (at the waypoint
  // coordinate). First value / x: latitude in degrees*1E7, second value / y:
  // longitude
  // in degrees*1E7, third value / z: positive altitude in meters with 0 being
  // at
  // ground level in terrain model.
  MAV_FRAME_GLOBAL_TERRAIN_ALT_INT = 11,
  // FRD local tangent frame (x: Forward, y: Right, z: Down) with origin that
  // travels
  // with vehicle. The forward axis is aligned to the front of the vehicle in
  // the
  // horizontal plane.
  MAV_FRAME_BODY_FRD = 12,
  // MAV_FRAME_BODY_FLU - Body fixed frame of reference, Z-up (x: Forward, y:
  // Left,
  // z: Up).
  MAV_FRAME_RESERVED_13 = 13,
  // MAV_FRAME_MOCAP_NED - Odometry local coordinate frame of data given by a
  // motion
  // capture system, Z-down (x: North, y: East, z: Down).
  MAV_FRAME_RESERVED_14 = 14,
  // MAV_FRAME_MOCAP_ENU - Odometry local coordinate frame of data given by a
  // motion
  // capture system, Z-up (x: East, y: North, z: Up).
  MAV_FRAME_RESERVED_15 = 15,
  // MAV_FRAME_VISION_NED - Odometry local coordinate frame of data given by a
  // vision
  // estimation system, Z-down (x: North, y: East, z: Down).
  MAV_FRAME_RESERVED_16 = 16,
  // MAV_FRAME_VISION_ENU - Odometry local coordinate frame of data given by a
  // vision
  // estimation system, Z-up (x: East, y: North, z: Up).
  MAV_FRAME_RESERVED_17 = 17,
  // MAV_FRAME_ESTIM_NED - Odometry local coordinate frame of data given by an
  // estimator
  // running onboard the vehicle, Z-down (x: North, y: East, z: Down).
  MAV_FRAME_RESERVED_18 = 18,
  // MAV_FRAME_ESTIM_ENU - Odometry local coordinate frame of data given by an
  // estimator
  // running onboard the vehicle, Z-up (x: East, y: North, z: Up).
  MAV_FRAME_RESERVED_19 = 19,
  // FRD local tangent frame (x: Forward, y: Right, z: Down) with origin fixed
  // relative
  // to earth. The forward axis is aligned to the front of the vehicle in the
  // horizontal
  // plane.
  MAV_FRAME_LOCAL_FRD = 20,
  // FLU local tangent frame (x: Forward, y: Left, z: Up) with origin fixed
  // relative
  // to earth. The forward axis is aligned to the front of the vehicle in the
  // horizontal
  // plane.
  MAV_FRAME_LOCAL_FLU = 21
};

enum class MAVLINK_DATA_STREAM_TYPE {
  MAVLINK_DATA_STREAM_IMG_JPEG = 0,
  MAVLINK_DATA_STREAM_IMG_BMP = 1,
  MAVLINK_DATA_STREAM_IMG_RAW8U = 2,
  MAVLINK_DATA_STREAM_IMG_RAW32U = 3,
  MAVLINK_DATA_STREAM_IMG_PGM = 4,
  MAVLINK_DATA_STREAM_IMG_PNG = 5
};

// Actions following geofence breach.
enum class FENCE_ACTION {
  // Disable fenced mode. If used in a plan this would mean the next fence is
  // disabled.
  FENCE_ACTION_NONE = 0,
  // Fly to geofence MAV_CMD_NAV_FENCE_RETURN_POINT in GUIDED mode. Note: This
  // action
  // is only supported by ArduPlane, and may not be supported in all versions.
  FENCE_ACTION_GUIDED = 1,
  // Report fence breach, but don't take action
  FENCE_ACTION_REPORT = 2,
  // Fly to geofence MAV_CMD_NAV_FENCE_RETURN_POINT with manual throttle control
  // in GUIDED mode. Note: This action is only supported by ArduPlane, and may
  // not
  // be supported in all versions.
  FENCE_ACTION_GUIDED_THR_PASS = 3,
  // Return/RTL mode.
  FENCE_ACTION_RTL = 4,
  // Hold at current location.
  FENCE_ACTION_HOLD = 5,
  // Termination failsafe. Motors are shut down (some flight stacks may trigger
  // other failsafe actions).
  FENCE_ACTION_TERMINATE = 6,
  // Land at current location.
  FENCE_ACTION_LAND = 7
};

enum class FENCE_BREACH {
  // No last fence breach
  FENCE_BREACH_NONE = 0,
  // Breached minimum altitude
  FENCE_BREACH_MINALT = 1,
  // Breached maximum altitude
  FENCE_BREACH_MAXALT = 2,
  // Breached fence boundary
  FENCE_BREACH_BOUNDARY = 3
};

// Actions being taken to mitigate/prevent fence breach
enum class FENCE_MITIGATE {
  // Unknown
  FENCE_MITIGATE_UNKNOWN = 0,
  // No actions being taken
  FENCE_MITIGATE_NONE = 1,
  // Velocity limiting active to prevent breach
  FENCE_MITIGATE_VEL_LIMIT = 2
};

// Enumeration of possible mount operation modes. This message is used by
// obsolete/deprecated gimbal messages.
enum class MAV_MOUNT_MODE {
  // Load and keep safe position (Roll,Pitch,Yaw) from permant memory and stop
  // stabilization
  MAV_MOUNT_MODE_RETRACT = 0,
  // Load and keep neutral position (Roll,Pitch,Yaw) from permanent memory.
  MAV_MOUNT_MODE_NEUTRAL = 1,
  // Load neutral position and start MAVLink Roll,Pitch,Yaw control with
  // stabilization
  MAV_MOUNT_MODE_MAVLINK_TARGETING = 2,
  // Load neutral position and start RC Roll,Pitch,Yaw control with
  // stabilization
  MAV_MOUNT_MODE_RC_TARGETING = 3,
  // Load neutral position and start to point to Lat,Lon,Alt
  MAV_MOUNT_MODE_GPS_POINT = 4,
  // Gimbal tracks system with specified system ID
  MAV_MOUNT_MODE_SYSID_TARGET = 5,
  // Gimbal tracks home position
  MAV_MOUNT_MODE_HOME_LOCATION = 6
};

// Gimbal device (low level) capability flags (bitmap)
enum class GIMBAL_DEVICE_CAP_FLAGS {
  // Gimbal device supports a retracted position
  GIMBAL_DEVICE_CAP_FLAGS_HAS_RETRACT = 1,
  // Gimbal device supports a horizontal, forward looking position, stabilized
  GIMBAL_DEVICE_CAP_FLAGS_HAS_NEUTRAL = 2,
  // Gimbal device supports rotating around roll axis.
  GIMBAL_DEVICE_CAP_FLAGS_HAS_ROLL_AXIS = 4,
  // Gimbal device supports to follow a roll angle relative to the vehicle
  GIMBAL_DEVICE_CAP_FLAGS_HAS_ROLL_FOLLOW = 8,
  // Gimbal device supports locking to an roll angle (generally that's the
  // default
  // with roll stabilized)
  GIMBAL_DEVICE_CAP_FLAGS_HAS_ROLL_LOCK = 16,
  // Gimbal device supports rotating around pitch axis.
  GIMBAL_DEVICE_CAP_FLAGS_HAS_PITCH_AXIS = 32,
  // Gimbal device supports to follow a pitch angle relative to the vehicle
  GIMBAL_DEVICE_CAP_FLAGS_HAS_PITCH_FOLLOW = 64,
  // Gimbal device supports locking to an pitch angle (generally that's the
  // default
  // with pitch stabilized)
  GIMBAL_DEVICE_CAP_FLAGS_HAS_PITCH_LOCK = 128,
  // Gimbal device supports rotating around yaw axis.
  GIMBAL_DEVICE_CAP_FLAGS_HAS_YAW_AXIS = 256,
  // Gimbal device supports to follow a yaw angle relative to the vehicle
  // (generally
  // that's the default)
  GIMBAL_DEVICE_CAP_FLAGS_HAS_YAW_FOLLOW = 512,
  // Gimbal device supports locking to an absolute heading (often this is an
  // option
  // available)
  GIMBAL_DEVICE_CAP_FLAGS_HAS_YAW_LOCK = 1024,
  // Gimbal device supports yawing/panning infinetely (e.g. using slip disk).
  GIMBAL_DEVICE_CAP_FLAGS_SUPPORTS_INFINITE_YAW = 2048
};

// Gimbal manager high level capability flags (bitmap). The first 16 bits are
// identical to the GIMBAL_DEVICE_CAP_FLAGS. However, the gimbal manager does
// not need to copy the flags from the gimbal but can also enhance the
// capabilities and thus add flags.
enum class GIMBAL_MANAGER_CAP_FLAGS {
  // Based on GIMBAL_DEVICE_CAP_FLAGS_HAS_RETRACT.
  GIMBAL_MANAGER_CAP_FLAGS_HAS_RETRACT = 1,
  // Based on GIMBAL_DEVICE_CAP_FLAGS_HAS_NEUTRAL.
  GIMBAL_MANAGER_CAP_FLAGS_HAS_NEUTRAL = 2,
  // Based on GIMBAL_DEVICE_CAP_FLAGS_HAS_ROLL_AXIS.
  GIMBAL_MANAGER_CAP_FLAGS_HAS_ROLL_AXIS = 4,
  // Based on GIMBAL_DEVICE_CAP_FLAGS_HAS_ROLL_FOLLOW.
  GIMBAL_MANAGER_CAP_FLAGS_HAS_ROLL_FOLLOW = 8,
  // Based on GIMBAL_DEVICE_CAP_FLAGS_HAS_ROLL_LOCK.
  GIMBAL_MANAGER_CAP_FLAGS_HAS_ROLL_LOCK = 16,
  // Based on GIMBAL_DEVICE_CAP_FLAGS_HAS_PITCH_AXIS.
  GIMBAL_MANAGER_CAP_FLAGS_HAS_PITCH_AXIS = 32,
  // Based on GIMBAL_DEVICE_CAP_FLAGS_HAS_PITCH_FOLLOW.
  GIMBAL_MANAGER_CAP_FLAGS_HAS_PITCH_FOLLOW = 64,
  // Based on GIMBAL_DEVICE_CAP_FLAGS_HAS_PITCH_LOCK.
  GIMBAL_MANAGER_CAP_FLAGS_HAS_PITCH_LOCK = 128,
  // Based on GIMBAL_DEVICE_CAP_FLAGS_HAS_YAW_AXIS.
  GIMBAL_MANAGER_CAP_FLAGS_HAS_YAW_AXIS = 256,
  // Based on GIMBAL_DEVICE_CAP_FLAGS_HAS_YAW_FOLLOW.
  GIMBAL_MANAGER_CAP_FLAGS_HAS_YAW_FOLLOW = 512,
  // Based on GIMBAL_DEVICE_CAP_FLAGS_HAS_YAW_LOCK.
  GIMBAL_MANAGER_CAP_FLAGS_HAS_YAW_LOCK = 1024,
  // Based on GIMBAL_DEVICE_CAP_FLAGS_SUPPORTS_INFINITE_YAW.
  GIMBAL_MANAGER_CAP_FLAGS_SUPPORTS_INFINITE_YAW = 2048,
  // Gimbal manager supports to point to a local position.
  GIMBAL_MANAGER_CAP_FLAGS_CAN_POINT_LOCATION_LOCAL = 65536,
  // Gimbal manager supports to point to a global latitude, longitude, altitude
  // position.
  GIMBAL_MANAGER_CAP_FLAGS_CAN_POINT_LOCATION_GLOBAL = 131072
};

// Flags for gimbal device (lower level) operation.
enum class GIMBAL_DEVICE_FLAGS {
  // Set to retracted safe position (no stabilization), takes presedence over
  // all
  // other flags.
  GIMBAL_DEVICE_FLAGS_RETRACT = 1,
  // Set to neutral/default position, taking precedence over all other flags
  // except
  // RETRACT. Neutral is commonly forward-facing and horizontal (pitch=yaw=0)
  // but
  // may be any orientation.
  GIMBAL_DEVICE_FLAGS_NEUTRAL = 2,
  // Lock roll angle to absolute angle relative to horizon (not relative to
  // drone).
  // This is generally the default with a stabilizing gimbal.
  GIMBAL_DEVICE_FLAGS_ROLL_LOCK = 4,
  // Lock pitch angle to absolute angle relative to horizon (not relative to
  // drone).
  // This is generally the default.
  GIMBAL_DEVICE_FLAGS_PITCH_LOCK = 8,
  // Lock yaw angle to absolute angle relative to North (not relative to drone).
  // If this flag is set, the quaternion is in the Earth frame with the x-axis
  // pointing
  // North (yaw absolute). If this flag is not set, the quaternion frame is in
  // the
  // Earth frame rotated so that the x-axis is pointing forward (yaw relative to
  // vehicle).
  GIMBAL_DEVICE_FLAGS_YAW_LOCK = 16
};

// Flags for high level gimbal manager operation The first 16 bits are identical
// to the GIMBAL_DEVICE_FLAGS.
enum class GIMBAL_MANAGER_FLAGS {
  // Based on GIMBAL_DEVICE_FLAGS_RETRACT
  GIMBAL_MANAGER_FLAGS_RETRACT = 1,
  // Based on GIMBAL_DEVICE_FLAGS_NEUTRAL
  GIMBAL_MANAGER_FLAGS_NEUTRAL = 2,
  // Based on GIMBAL_DEVICE_FLAGS_ROLL_LOCK
  GIMBAL_MANAGER_FLAGS_ROLL_LOCK = 4,
  // Based on GIMBAL_DEVICE_FLAGS_PITCH_LOCK
  GIMBAL_MANAGER_FLAGS_PITCH_LOCK = 8,
  // Based on GIMBAL_DEVICE_FLAGS_YAW_LOCK
  GIMBAL_MANAGER_FLAGS_YAW_LOCK = 16
};

// Gimbal device (low level) error flags (bitmap, 0 means no error)
enum class GIMBAL_DEVICE_ERROR_FLAGS {
  // Gimbal device is limited by hardware roll limit.
  GIMBAL_DEVICE_ERROR_FLAGS_AT_ROLL_LIMIT = 1,
  // Gimbal device is limited by hardware pitch limit.
  GIMBAL_DEVICE_ERROR_FLAGS_AT_PITCH_LIMIT = 2,
  // Gimbal device is limited by hardware yaw limit.
  GIMBAL_DEVICE_ERROR_FLAGS_AT_YAW_LIMIT = 4,
  // There is an error with the gimbal encoders.
  GIMBAL_DEVICE_ERROR_FLAGS_ENCODER_ERROR = 8,
  // There is an error with the gimbal power source.
  GIMBAL_DEVICE_ERROR_FLAGS_POWER_ERROR = 16,
  // There is an error with the gimbal motor's.
  GIMBAL_DEVICE_ERROR_FLAGS_MOTOR_ERROR = 32,
  // There is an error with the gimbal's software.
  GIMBAL_DEVICE_ERROR_FLAGS_SOFTWARE_ERROR = 64,
  // There is an error with the gimbal's communication.
  GIMBAL_DEVICE_ERROR_FLAGS_COMMS_ERROR = 128,
  // Gimbal is currently calibrating.
  GIMBAL_DEVICE_ERROR_FLAGS_CALIBRATION_RUNNING = 256
};

// Gripper actions.
enum class GRIPPER_ACTIONS {
  // Gripper release cargo.
  GRIPPER_ACTION_RELEASE = 0,
  // Gripper grab onto cargo.
  GRIPPER_ACTION_GRAB = 1
};

// Winch actions.
enum class WINCH_ACTIONS {
  // Allow motor to freewheel.
  WINCH_RELAXED = 0,
  // Wind or unwind specified length of line, optionally using specified rate.
  WINCH_RELATIVE_LENGTH_CONTROL = 1,
  // Wind or unwind line at specified rate.
  WINCH_RATE_CONTROL = 2,
  // Perform the locking sequence to relieve motor while in the fully retracted
  // position. Only action and instance command parameters are used, others are
  // ignored.
  WINCH_LOCK = 3,
  // Sequence of drop, slow down, touch down, reel up, lock. Only action and
  // instance
  // command parameters are used, others are ignored.
  WINCH_DELIVER = 4,
  // Engage motor and hold current position. Only action and instance command
  // parameters
  // are used, others are ignored.
  WINCH_HOLD = 5,
  // Return the reel to the fully retracted position. Only action and instance
  // command
  // parameters are used, others are ignored.
  WINCH_RETRACT = 6,
  // Load the reel with line. The winch will calculate the total loaded length
  // and
  // stop when the tension exceeds a threshold. Only action and instance command
  // parameters are used, others are ignored.
  WINCH_LOAD_LINE = 7,
  // Spool out the entire length of the line. Only action and instance command
  // parameters
  // are used, others are ignored.
  WINCH_ABANDON_LINE = 8
};

// Generalized UAVCAN node health
enum class UAVCAN_NODE_HEALTH {
  // The node is functioning properly.
  UAVCAN_NODE_HEALTH_OK = 0,
  // A critical parameter went out of range or the node has encountered a minor
  // failure.
  UAVCAN_NODE_HEALTH_WARNING = 1,
  // The node has encountered a major failure.
  UAVCAN_NODE_HEALTH_ERROR = 2,
  // The node has suffered a fatal malfunction.
  UAVCAN_NODE_HEALTH_CRITICAL = 3
};

// Generalized UAVCAN node mode
enum class UAVCAN_NODE_MODE {
  // The node is performing its primary functions.
  UAVCAN_NODE_MODE_OPERATIONAL = 0,
  // The node is initializing; this mode is entered immediately after startup.
  UAVCAN_NODE_MODE_INITIALIZATION = 1,
  // The node is under maintenance.
  UAVCAN_NODE_MODE_MAINTENANCE = 2,
  // The node is in the process of updating its software.
  UAVCAN_NODE_MODE_SOFTWARE_UPDATE = 3,
  // The node is no longer available online.
  UAVCAN_NODE_MODE_OFFLINE = 7
};

// Indicates the ESC connection type.
enum class ESC_CONNECTION_TYPE {
  // Traditional PPM ESC.
  ESC_CONNECTION_TYPE_PPM = 0,
  // Serial Bus connected ESC.
  ESC_CONNECTION_TYPE_SERIAL = 1,
  // One Shot PPM ESC.
  ESC_CONNECTION_TYPE_ONESHOT = 2,
  // I2C ESC.
  ESC_CONNECTION_TYPE_I2C = 3,
  // CAN-Bus ESC.
  ESC_CONNECTION_TYPE_CAN = 4,
  // DShot ESC.
  ESC_CONNECTION_TYPE_DSHOT = 5
};

// Flags to report ESC failures.
enum class ESC_FAILURE_FLAGS {
  // No ESC failure.
  ESC_FAILURE_NONE = 0,
  // Over current failure.
  ESC_FAILURE_OVER_CURRENT = 1,
  // Over voltage failure.
  ESC_FAILURE_OVER_VOLTAGE = 2,
  // Over temperature failure.
  ESC_FAILURE_OVER_TEMPERATURE = 4,
  // Over RPM failure.
  ESC_FAILURE_OVER_RPM = 8,
  // Inconsistent command failure i.e. out of bounds.
  ESC_FAILURE_INCONSISTENT_CMD = 16,
  // Motor stuck failure.
  ESC_FAILURE_MOTOR_STUCK = 32,
  // Generic ESC failure.
  ESC_FAILURE_GENERIC = 64
};

// Flags to indicate the status of camera storage.
enum class STORAGE_STATUS {
  // Storage is missing (no microSD card loaded for example.)
  STORAGE_STATUS_EMPTY = 0,
  // Storage present but unformatted.
  STORAGE_STATUS_UNFORMATTED = 1,
  // Storage present and ready.
  STORAGE_STATUS_READY = 2,
  // Camera does not supply storage status information. Capacity information in
  // STORAGE_INFORMATION fields will be ignored.
  STORAGE_STATUS_NOT_SUPPORTED = 3
};

// Flags to indicate the type of storage.
enum class STORAGE_TYPE {
  // Storage type is not known.
  STORAGE_TYPE_UNKNOWN = 0,
  // Storage type is USB device.
  STORAGE_TYPE_USB_STICK = 1,
  // Storage type is SD card.
  STORAGE_TYPE_SD = 2,
  // Storage type is microSD card.
  STORAGE_TYPE_MICROSD = 3,
  // Storage type is CFast.
  STORAGE_TYPE_CF = 4,
  // Storage type is CFexpress.
  STORAGE_TYPE_CFE = 5,
  // Storage type is XQD.
  STORAGE_TYPE_XQD = 6,
  // Storage type is HD mass storage type.
  STORAGE_TYPE_HD = 7,
  // Storage type is other, not listed type.
  STORAGE_TYPE_OTHER = 254
};

// Flags to indicate usage for a particular storage (see
// STORAGE_INFORMATION.storage_usage and MAV_CMD_SET_STORAGE_USAGE).
enum class STORAGE_USAGE_FLAG {
  // Always set to 1 (indicates STORAGE_INFORMATION.storage_usage is supported).
  STORAGE_USAGE_FLAG_SET = 1,
  // Storage for saving photos.
  STORAGE_USAGE_FLAG_PHOTO = 2,
  // Storage for saving videos.
  STORAGE_USAGE_FLAG_VIDEO = 4,
  // Storage for saving logs.
  STORAGE_USAGE_FLAG_LOGS = 8
};

// Yaw behaviour during orbit flight.
enum class ORBIT_YAW_BEHAVIOUR {
  // Vehicle front points to the center (default).
  ORBIT_YAW_BEHAVIOUR_HOLD_FRONT_TO_CIRCLE_CENTER = 0,
  // Vehicle front holds heading when message received.
  ORBIT_YAW_BEHAVIOUR_HOLD_INITIAL_HEADING = 1,
  // Yaw uncontrolled.
  ORBIT_YAW_BEHAVIOUR_UNCONTROLLED = 2,
  // Vehicle front follows flight path (tangential to circle).
  ORBIT_YAW_BEHAVIOUR_HOLD_FRONT_TANGENT_TO_CIRCLE = 3,
  // Yaw controlled by RC input.
  ORBIT_YAW_BEHAVIOUR_RC_CONTROLLED = 4
};

// Possible responses from a WIFI_CONFIG_AP message.
enum class WIFI_CONFIG_AP_RESPONSE {
  // Undefined response. Likely an indicative of a system that doesn't support
  // this
  // request.
  WIFI_CONFIG_AP_RESPONSE_UNDEFINED = 0,
  // Changes accepted.
  WIFI_CONFIG_AP_RESPONSE_ACCEPTED = 1,
  // Changes rejected.
  WIFI_CONFIG_AP_RESPONSE_REJECTED = 2,
  // Invalid Mode.
  WIFI_CONFIG_AP_RESPONSE_MODE_ERROR = 3,
  // Invalid SSID.
  WIFI_CONFIG_AP_RESPONSE_SSID_ERROR = 4,
  // Invalid Password.
  WIFI_CONFIG_AP_RESPONSE_PASSWORD_ERROR = 5
};

// Possible responses from a CELLULAR_CONFIG message.
enum class CELLULAR_CONFIG_RESPONSE {
  // Changes accepted.
  CELLULAR_CONFIG_RESPONSE_ACCEPTED = 0,
  // Invalid APN.
  CELLULAR_CONFIG_RESPONSE_APN_ERROR = 1,
  // Invalid PIN.
  CELLULAR_CONFIG_RESPONSE_PIN_ERROR = 2,
  // Changes rejected.
  CELLULAR_CONFIG_RESPONSE_REJECTED = 3,
  // PUK is required to unblock SIM card.
  CELLULAR_CONFIG_BLOCKED_PUK_REQUIRED = 4
};

// WiFi Mode.
enum class WIFI_CONFIG_AP_MODE {
  // WiFi mode is undefined.
  WIFI_CONFIG_AP_MODE_UNDEFINED = 0,
  // WiFi configured as an access point.
  WIFI_CONFIG_AP_MODE_AP = 1,
  // WiFi configured as a station connected to an existing local WiFi network.
  WIFI_CONFIG_AP_MODE_STATION = 2,
  // WiFi disabled.
  WIFI_CONFIG_AP_MODE_DISABLED = 3
};

// Supported component metadata types. These are used in the "general" metadata
// file returned by COMPONENT_METADATA to provide information about supported
// metadata types. The types are not used directly in MAVLink messages.
enum class COMP_METADATA_TYPE {
  // General information about the component. General metadata includes
  // information
  // about other metadata types supported by the component. Files of this type
  // must
  // be supported, and must be downloadable from vehicle using a MAVLink FTP
  // URI.
  COMP_METADATA_TYPE_GENERAL = 0,
  // Parameter meta data.
  COMP_METADATA_TYPE_PARAMETER = 1,
  // Meta data that specifies which commands and command parameters the vehicle
  // supports. (WIP)
  COMP_METADATA_TYPE_COMMANDS = 2,
  // Meta data that specifies external non-MAVLink peripherals.
  COMP_METADATA_TYPE_PERIPHERALS = 3,
  // Meta data for the events interface.
  COMP_METADATA_TYPE_EVENTS = 4,
  // Meta data for actuator configuration (motors, servos and vehicle geometry)
  // and testing.
  COMP_METADATA_TYPE_ACTUATORS = 5
};

// Actuator configuration, used to change a setting on an actuator. Component
// information metadata can be used to know which outputs support which
// commands.
enum class ACTUATOR_CONFIGURATION {
  // Do nothing.
  ACTUATOR_CONFIGURATION_NONE = 0,
  // Command the actuator to beep now.
  ACTUATOR_CONFIGURATION_BEEP = 1,
  // Permanently set the actuator (ESC) to 3D mode (reversible thrust).
  ACTUATOR_CONFIGURATION_3D_MODE_ON = 2,
  // Permanently set the actuator (ESC) to non 3D mode (non-reversible thrust).
  ACTUATOR_CONFIGURATION_3D_MODE_OFF = 3,
  // Permanently set the actuator (ESC) to spin direction 1 (which can be
  // clockwise
  // or counter-clockwise).
  ACTUATOR_CONFIGURATION_SPIN_DIRECTION1 = 4,
  // Permanently set the actuator (ESC) to spin direction 2 (opposite of
  // direction
  // 1).
  ACTUATOR_CONFIGURATION_SPIN_DIRECTION2 = 5
};

// Actuator output function. Values greater or equal to 1000 are
// autopilot-specific.
enum class ACTUATOR_OUTPUT_FUNCTION {
  // No function (disabled).
  ACTUATOR_OUTPUT_FUNCTION_NONE = 0,
  // Motor 1
  ACTUATOR_OUTPUT_FUNCTION_MOTOR1 = 1,
  // Motor 2
  ACTUATOR_OUTPUT_FUNCTION_MOTOR2 = 2,
  // Motor 3
  ACTUATOR_OUTPUT_FUNCTION_MOTOR3 = 3,
  // Motor 4
  ACTUATOR_OUTPUT_FUNCTION_MOTOR4 = 4,
  // Motor 5
  ACTUATOR_OUTPUT_FUNCTION_MOTOR5 = 5,
  // Motor 6
  ACTUATOR_OUTPUT_FUNCTION_MOTOR6 = 6,
  // Motor 7
  ACTUATOR_OUTPUT_FUNCTION_MOTOR7 = 7,
  // Motor 8
  ACTUATOR_OUTPUT_FUNCTION_MOTOR8 = 8,
  // Motor 9
  ACTUATOR_OUTPUT_FUNCTION_MOTOR9 = 9,
  // Motor 10
  ACTUATOR_OUTPUT_FUNCTION_MOTOR10 = 10,
  // Motor 11
  ACTUATOR_OUTPUT_FUNCTION_MOTOR11 = 11,
  // Motor 12
  ACTUATOR_OUTPUT_FUNCTION_MOTOR12 = 12,
  // Motor 13
  ACTUATOR_OUTPUT_FUNCTION_MOTOR13 = 13,
  // Motor 14
  ACTUATOR_OUTPUT_FUNCTION_MOTOR14 = 14,
  // Motor 15
  ACTUATOR_OUTPUT_FUNCTION_MOTOR15 = 15,
  // Motor 16
  ACTUATOR_OUTPUT_FUNCTION_MOTOR16 = 16,
  // Servo 1
  ACTUATOR_OUTPUT_FUNCTION_SERVO1 = 33,
  // Servo 2
  ACTUATOR_OUTPUT_FUNCTION_SERVO2 = 34,
  // Servo 3
  ACTUATOR_OUTPUT_FUNCTION_SERVO3 = 35,
  // Servo 4
  ACTUATOR_OUTPUT_FUNCTION_SERVO4 = 36,
  // Servo 5
  ACTUATOR_OUTPUT_FUNCTION_SERVO5 = 37,
  // Servo 6
  ACTUATOR_OUTPUT_FUNCTION_SERVO6 = 38,
  // Servo 7
  ACTUATOR_OUTPUT_FUNCTION_SERVO7 = 39,
  // Servo 8
  ACTUATOR_OUTPUT_FUNCTION_SERVO8 = 40,
  // Servo 9
  ACTUATOR_OUTPUT_FUNCTION_SERVO9 = 41,
  // Servo 10
  ACTUATOR_OUTPUT_FUNCTION_SERVO10 = 42,
  // Servo 11
  ACTUATOR_OUTPUT_FUNCTION_SERVO11 = 43,
  // Servo 12
  ACTUATOR_OUTPUT_FUNCTION_SERVO12 = 44,
  // Servo 13
  ACTUATOR_OUTPUT_FUNCTION_SERVO13 = 45,
  // Servo 14
  ACTUATOR_OUTPUT_FUNCTION_SERVO14 = 46,
  // Servo 15
  ACTUATOR_OUTPUT_FUNCTION_SERVO15 = 47,
  // Servo 16
  ACTUATOR_OUTPUT_FUNCTION_SERVO16 = 48
};

// Enable axes that will be tuned via autotuning. Used in
// MAV_CMD_DO_AUTOTUNE_ENABLE.
enum class AUTOTUNE_AXIS {
  // Flight stack tunes axis according to its default settings.
  AUTOTUNE_AXIS_DEFAULT = 0,
  // Autotune roll axis.
  AUTOTUNE_AXIS_ROLL = 1,
  // Autotune pitch axis.
  AUTOTUNE_AXIS_PITCH = 2,
  // Autotune yaw axis.
  AUTOTUNE_AXIS_YAW = 4
};

// Actions for reading/writing parameters between persistent and volatile
// storage when using MAV_CMD_PREFLIGHT_STORAGE. (Commonly parameters are loaded
// from persistent storage (flash/EEPROM) into volatile storage (RAM) on startup
// and written back when they are changed.)
enum class PREFLIGHT_STORAGE_PARAMETER_ACTION {
  // Read all parameters from persistent storage. Replaces values in volatile
  // storage.
  PARAM_READ_PERSISTENT = 0,
  // Write all parameter values to persistent storage (flash/EEPROM)
  PARAM_WRITE_PERSISTENT = 1,
  // Reset all user configurable parameters to their default value (including
  // airframe
  // selection, sensor calibration data, safety settings, and so on). Does not
  // reset
  // values that contain operation counters and vehicle computed statistics.
  PARAM_RESET_CONFIG_DEFAULT = 2,
  // Reset only sensor calibration parameters to factory defaults (or firmware
  // default
  // if not available)
  PARAM_RESET_SENSOR_DEFAULT = 3,
  // Reset all parameters, including operation counters, to default values
  PARAM_RESET_ALL_DEFAULT = 4
};

// Actions for reading and writing plan information (mission, rally points,
// geofence) between persistent and volatile storage when using
// MAV_CMD_PREFLIGHT_STORAGE. (Commonly missions are loaded from persistent
// storage (flash/EEPROM) into volatile storage (RAM) on startup and written
// back when they are changed.)
enum class PREFLIGHT_STORAGE_MISSION_ACTION {
  // Read current mission data from persistent storage
  MISSION_READ_PERSISTENT = 0,
  // Write current mission data to persistent storage
  MISSION_WRITE_PERSISTENT = 1,
  // Erase all mission data stored on the vehicle (both persistent and volatile
  // storage)
  MISSION_RESET_DEFAULT = 2
};

// Commands to be executed by the MAV. They can be executed on user request, or
// as part of a mission script. If the action is used in a mission, the
// parameter mapping to the waypoint/mission message is as follows: Param 1,
// Param 2, Param 3, Param 4, X: Param 5, Y:Param 6, Z:Param 7. This command
// list is similar what ARINC 424 is for commercial aircraft: A data format how
// to interpret waypoint/mission data. NaN and INT32_MAX may be used in
// float/integer params (respectively) to indicate optional/default values (e.g.
// to use the component's current yaw or latitude rather than a specific value).
// See https://mavlink.io/en/guide/xml_schema.html#MAV_CMD for information about
// the structure of the MAV_CMD entries
enum class MAV_CMD {
  // Navigate to waypoint.
  MAV_CMD_NAV_WAYPOINT = 16,
  // Loiter around this waypoint an unlimited amount of time
  MAV_CMD_NAV_LOITER_UNLIM = 17,
  // Loiter around this waypoint for X turns
  MAV_CMD_NAV_LOITER_TURNS = 18,
  // Loiter at the specified latitude, longitude and altitude for a certain
  // amount
  // of time. Multicopter vehicles stop at the point (within a vehicle-specific
  // acceptance radius). Forward-only moving vehicles (e.g. fixed-wing) circle
  // the
  // point with the specified radius/direction. If the Heading Required
  // parameter
  // (2) is non-zero forward moving aircraft will only leave the loiter circle
  // once
  // heading towards the next waypoint.
  MAV_CMD_NAV_LOITER_TIME = 19,
  // Return to launch location
  MAV_CMD_NAV_RETURN_TO_LAUNCH = 20,
  // Land at location.
  MAV_CMD_NAV_LAND = 21,
  // Takeoff from ground / hand. Vehicles that support multiple takeoff modes
  // (e.g.
  // VTOL quadplane) should take off using the currently configured mode.
  MAV_CMD_NAV_TAKEOFF = 22,
  // Land at local position (local frame only)
  MAV_CMD_NAV_LAND_LOCAL = 23,
  // Takeoff from local position (local frame only)
  MAV_CMD_NAV_TAKEOFF_LOCAL = 24,
  // Vehicle following, i.e. this waypoint represents the position of a moving
  // vehicle
  MAV_CMD_NAV_FOLLOW = 25,
  // Continue on the current course and climb/descend to specified altitude.
  // When
  // the altitude is reached continue to the next command (i.e., don't proceed
  // to
  // the next command until the desired altitude is reached.
  MAV_CMD_NAV_CONTINUE_AND_CHANGE_ALT = 30,
  // Begin loiter at the specified Latitude and Longitude. If Lat=Lon=0, then
  // loiter
  // at the current position. Don't consider the navigation command complete
  // (don't
  // leave loiter) until the altitude has been reached. Additionally, if the
  // Heading
  // Required parameter is non-zero the aircraft will not leave the loiter until
  // heading toward the next waypoint.
  MAV_CMD_NAV_LOITER_TO_ALT = 31,
  // Begin following a target
  MAV_CMD_DO_FOLLOW = 32,
  // Reposition the MAV after a follow target command has been sent
  MAV_CMD_DO_FOLLOW_REPOSITION = 33,
  // Start orbiting on the circumference of a circle defined by the parameters.
  // Setting values to NaN/INT32_MAX (as appropriate) results in using defaults.
  MAV_CMD_DO_ORBIT = 34,
  // Sets the region of interest (ROI) for a sensor set or the vehicle itself.
  // This
  // can then be used by the vehicle's control system to control the vehicle
  // attitude
  // and the attitude of various sensors such as cameras.
  MAV_CMD_NAV_ROI = 80,
  // Control autonomous path planning on the MAV.
  MAV_CMD_NAV_PATHPLANNING = 81,
  // Navigate to waypoint using a spline path.
  MAV_CMD_NAV_SPLINE_WAYPOINT = 82,
  // Takeoff from ground using VTOL mode, and transition to forward flight with
  // specified heading. The command should be ignored by vehicles that dont
  // support
  // both VTOL and fixed-wing flight (multicopters, boats,etc.).
  MAV_CMD_NAV_VTOL_TAKEOFF = 84,
  // Land using VTOL mode
  MAV_CMD_NAV_VTOL_LAND = 85,
  // hand control over to an external controller
  MAV_CMD_NAV_GUIDED_ENABLE = 92,
  // Delay the next navigation command a number of seconds or until a specified
  // time
  MAV_CMD_NAV_DELAY = 93,
  // Descend and place payload. Vehicle moves to specified location, descends
  // until
  // it detects a hanging payload has reached the ground, and then releases the
  // payload. If ground is not detected before the reaching the maximum descent
  // value (param1), the command will complete without releasing the payload.
  MAV_CMD_NAV_PAYLOAD_PLACE = 94,
  // NOP - This command is only used to mark the upper limit of the NAV/ACTION
  // commands
  // in the enumeration
  MAV_CMD_NAV_LAST = 95,
  // Delay mission state machine.
  MAV_CMD_CONDITION_DELAY = 112,
  // Ascend/descend to target altitude at specified rate. Delay mission state
  // machine
  // until desired altitude reached.
  MAV_CMD_CONDITION_CHANGE_ALT = 113,
  // Delay mission state machine until within desired distance of next NAV
  // point.
  MAV_CMD_CONDITION_DISTANCE = 114,
  // Reach a certain target angle.
  MAV_CMD_CONDITION_YAW = 115,
  // NOP - This command is only used to mark the upper limit of the CONDITION
  // commands
  // in the enumeration
  MAV_CMD_CONDITION_LAST = 159,
  // Set system mode.
  MAV_CMD_DO_SET_MODE = 176,
  // Jump to the desired command in the mission list. Repeat this action only
  // the
  // specified number of times
  MAV_CMD_DO_JUMP = 177,
  // Change speed and/or throttle set points.
  MAV_CMD_DO_CHANGE_SPEED = 178,
  // Sets the home position to either to the current position or a specified
  // position.
  // The home position is the default position that the system will return to
  // and
  // land on. The position is set automatically by the system during the takeoff
  // (and may also be set using this command). Note: the current home position
  // may
  // be emitted in a HOME_POSITION message on request (using
  // MAV_CMD_REQUEST_MESSAGE
  // with param1=242).
  MAV_CMD_DO_SET_HOME = 179,
  // Set a system parameter. Caution! Use of this command requires knowledge of
  // the numeric enumeration value of the parameter.
  MAV_CMD_DO_SET_PARAMETER = 180,
  // Set a relay to a condition.
  MAV_CMD_DO_SET_RELAY = 181,
  // Cycle a relay on and off for a desired number of cycles with a desired
  // period.
  MAV_CMD_DO_REPEAT_RELAY = 182,
  // Set a servo to a desired PWM value.
  MAV_CMD_DO_SET_SERVO = 183,
  // Cycle a between its nominal setting and a desired PWM for a desired number
  // of cycles with a desired period.
  MAV_CMD_DO_REPEAT_SERVO = 184,
  // Terminate flight immediately. Flight termination immediately and
  // irreversably
  // terminates the current flight, returning the vehicle to ground. The vehicle
  // will ignore RC or other input until it has been power-cycled. Termination
  // may
  // trigger safety measures, including: disabling motors and deployment of
  // parachute
  // on multicopters, and setting flight surfaces to initiate a landing pattern
  // on fixed-wing). On multicopters without a parachute it may trigger a crash
  // landing. Support for this command can be tested using the protocol bit:
  // MAV_PROTOCOL_CAPABILITY_FLIGHT_TERMINATION.
  // Support for this command can also be tested by sending the command with
  // param1=0
  // (< 0.5); the ACK should be either MAV_RESULT_FAILED or
  // MAV_RESULT_UNSUPPORTED.
  MAV_CMD_DO_FLIGHTTERMINATION = 185,
  // Change altitude set point.
  MAV_CMD_DO_CHANGE_ALTITUDE = 186,
  // Sets actuators (e.g. servos) to a desired value. The actuator numbers are
  // mapped
  // to specific outputs (e.g. on any MAIN or AUX PWM or UAVCAN) using a
  // flight-stack
  // specific mechanism (i.e. a parameter).
  MAV_CMD_DO_SET_ACTUATOR = 187,
  // Mission command to perform a landing. This is used as a marker in a mission
  // to tell the autopilot where a sequence of mission items that represents a
  // landing
  // starts. It may also be sent via a COMMAND_LONG to trigger a landing, in
  // which
  // case the nearest (geographically) landing sequence in the mission will be
  // used.
  // The Latitude/Longitude is optional, and may be set to 0 if not needed. If
  // specified
  // then it will be used to help find the closest landing sequence.
  MAV_CMD_DO_LAND_START = 189,
  // Mission command to perform a landing from a rally point.
  MAV_CMD_DO_RALLY_LAND = 190,
  // Mission command to safely abort an autonomous landing.
  MAV_CMD_DO_GO_AROUND = 191,
  // Reposition the vehicle to a specific WGS84 global position.
  MAV_CMD_DO_REPOSITION = 192,
  // If in a GPS controlled position mode, hold the current position or
  // continue.
  MAV_CMD_DO_PAUSE_CONTINUE = 193,
  // Set moving direction to forward or reverse.
  MAV_CMD_DO_SET_REVERSE = 194,
  // Sets the region of interest (ROI) to a location. This can then be used by
  // the
  // vehicle's control system to control the vehicle attitude and the attitude
  // of
  // various sensors such as cameras. This command can be sent to a gimbal
  // manager
  // but not to a gimbal device. A gimbal is not to react to this message.
  MAV_CMD_DO_SET_ROI_LOCATION = 195,
  // Sets the region of interest (ROI) to be toward next waypoint, with optional
  // pitch/roll/yaw offset. This can then be used by the vehicle's control
  // system
  // to control the vehicle attitude and the attitude of various sensors such as
  // cameras. This command can be sent to a gimbal manager but not to a gimbal
  // device.
  // A gimbal device is not to react to this message.
  MAV_CMD_DO_SET_ROI_WPNEXT_OFFSET = 196,
  // Cancels any previous ROI command returning the vehicle/sensors to default
  // flight
  // characteristics. This can then be used by the vehicle's control system to
  // control
  // the vehicle attitude and the attitude of various sensors such as cameras.
  // This
  // command can be sent to a gimbal manager but not to a gimbal device. A
  // gimbal
  // device is not to react to this message. After this command the gimbal
  // manager
  // should go back to manual input if available, and otherwise assume a neutral
  // position.
  MAV_CMD_DO_SET_ROI_NONE = 197,
  // Mount tracks system with specified system ID. Determination of target
  // vehicle
  // position may be done with GLOBAL_POSITION_INT or any other means. This
  // command
  // can be sent to a gimbal manager but not to a gimbal device. A gimbal device
  // is not to react to this message.
  MAV_CMD_DO_SET_ROI_SYSID = 198,
  // Control onboard camera system.
  MAV_CMD_DO_CONTROL_VIDEO = 200,
  // Sets the region of interest (ROI) for a sensor set or the vehicle itself.
  // This
  // can then be used by the vehicle's control system to control the vehicle
  // attitude
  // and the attitude of various sensors such as cameras.
  MAV_CMD_DO_SET_ROI = 201,
  // Configure digital camera. This is a fallback message for systems that have
  // not yet implemented PARAM_EXT_XXX messages and camera definition files (see
  // https://mavlink.io/en/services/camera_def.html ).
  MAV_CMD_DO_DIGICAM_CONFIGURE = 202,
  // Control digital camera. This is a fallback message for systems that have
  // not
  // yet implemented PARAM_EXT_XXX messages and camera definition files (see
  // https://mavlink.io/en/services/camera_def.html
  // ).
  MAV_CMD_DO_DIGICAM_CONTROL = 203,
  // Mission command to configure a camera or antenna mount
  MAV_CMD_DO_MOUNT_CONFIGURE = 204,
  // Mission command to control a camera or antenna mount
  MAV_CMD_DO_MOUNT_CONTROL = 205,
  // Mission command to set camera trigger distance for this flight. The camera
  // is triggered each time this distance is exceeded. This command can also be
  // used to set the shutter integration time for the camera.
  MAV_CMD_DO_SET_CAM_TRIGG_DIST = 206,
  // Mission command to enable the geofence
  MAV_CMD_DO_FENCE_ENABLE = 207,
  // Mission item/command to release a parachute or enable/disable auto release.
  MAV_CMD_DO_PARACHUTE = 208,
  // Command to perform motor test.
  MAV_CMD_DO_MOTOR_TEST = 209,
  // Change to/from inverted flight.
  MAV_CMD_DO_INVERTED_FLIGHT = 210,
  // Mission command to operate a gripper.
  MAV_CMD_DO_GRIPPER = 211,
  // Enable/disable autotune.
  MAV_CMD_DO_AUTOTUNE_ENABLE = 212,
  // Sets a desired vehicle turn angle and speed change.
  MAV_CMD_NAV_SET_YAW_SPEED = 213,
  // Mission command to set camera trigger interval for this flight. If
  // triggering
  // is enabled, the camera is triggered each time this interval expires. This
  // command
  // can also be used to set the shutter integration time for the camera.
  MAV_CMD_DO_SET_CAM_TRIGG_INTERVAL = 214,
  // Mission command to control a camera or antenna mount, using a quaternion as
  // reference.
  MAV_CMD_DO_MOUNT_CONTROL_QUAT = 220,
  // set id of master controller
  MAV_CMD_DO_GUIDED_MASTER = 221,
  // Set limits for external control
  MAV_CMD_DO_GUIDED_LIMITS = 222,
  // Control vehicle engine. This is interpreted by the vehicles engine
  // controller
  // to change the target engine state. It is intended for vehicles with
  // internal
  // combustion engines
  MAV_CMD_DO_ENGINE_CONTROL = 223,
  // Set the mission item with sequence number seq as current item. This means
  // that
  // the MAV will continue to this mission item on the shortest path (not
  // following
  // the mission items in-between).
  MAV_CMD_DO_SET_MISSION_CURRENT = 224,
  // NOP - This command is only used to mark the upper limit of the DO commands
  // in the enumeration
  MAV_CMD_DO_LAST = 240,
  // Trigger calibration. This command will be only accepted if in pre-flight
  // mode.
  // Except for Temperature Calibration, only one sensor should be set in a
  // single
  // message and all others should be zero.
  MAV_CMD_PREFLIGHT_CALIBRATION = 241,
  // Set sensor offsets. This command will be only accepted if in pre-flight
  // mode.
  MAV_CMD_PREFLIGHT_SET_SENSOR_OFFSETS = 242,
  // Trigger UAVCAN configuration (actuator ID assignment and direction
  // mapping).
  // Note that this maps to the legacy UAVCAN v0 function UAVCAN_ENUMERATE,
  // which
  // is intended to be executed just once during initial vehicle configuration
  // (it
  // is not a normal pre-flight command and has been poorly named).
  MAV_CMD_PREFLIGHT_UAVCAN = 243,
  // Request storage of different parameter values and logs. This command will
  // be
  // only accepted if in pre-flight mode.
  MAV_CMD_PREFLIGHT_STORAGE = 245,
  // Request the reboot or shutdown of system components.
  MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN = 246,
  // Override current mission with command to pause mission, pause mission and
  // move
  // to position, continue/resume mission. When param 1 indicates that the
  // mission
  // is paused (MAV_GOTO_DO_HOLD), param 2 defines whether it holds in place or
  // moves to another position.
  MAV_CMD_OVERRIDE_GOTO = 252,
  // Mission command to set a Camera Auto Mount Pivoting Oblique Survey
  // (Replaces
  // CAM_TRIGG_DIST for this purpose). The camera is triggered each time this
  // distance
  // is exceeded, then the mount moves to the next position. Params 4~6 set-up
  // the
  // angle limits and number of positions for oblique survey, where
  // mount-enabled
  // vehicles automatically roll the camera between shots to emulate an oblique
  // camera setup (providing an increased HFOV). This command can also be used
  // to
  // set the shutter integration time for the camera.
  MAV_CMD_OBLIQUE_SURVEY = 260,
  // start running a mission
  MAV_CMD_MISSION_START = 300,
  // Actuator testing command. This is similar to MAV_CMD_DO_MOTOR_TEST but
  // operates
  // on the level of output functions, i.e. it is possible to test Motor1
  // independent
  // from which output it is configured on. Autopilots typically refuse this
  // command
  // while armed.
  MAV_CMD_ACTUATOR_TEST = 310,
  // Actuator configuration command.
  MAV_CMD_CONFIGURE_ACTUATOR = 311,
  // Arms / Disarms a component
  MAV_CMD_COMPONENT_ARM_DISARM = 400,
  // Instructs a target system to run pre-arm checks. This allows preflight
  // checks
  // to be run on demand, which may be useful on systems that normally run them
  // at low rate, or which do not trigger checks when the armable state might
  // have
  // changed. This command should return MAV_RESULT_ACCEPTED if it will run the
  // checks. The results of the checks are usually then reported in SYS_STATUS
  // messages
  // (this is system-specific). The command should return
  // MAV_RESULT_TEMPORARILY_REJECTED
  // if the system is already armed.
  MAV_CMD_RUN_PREARM_CHECKS = 401,
  // Turns illuminators ON/OFF. An illuminator is a light source that is used
  // for
  // lighting up dark areas external to the sytstem: e.g. a torch or searchlight
  // (as opposed to a light source for illuminating the system itself, e.g. an
  // indicator
  // light).
  MAV_CMD_ILLUMINATOR_ON_OFF = 405,
  // Request the home position from the vehicle. The vehicle will ACK the
  // command
  // and then emit the HOME_POSITION message.
  MAV_CMD_GET_HOME_POSITION = 410,
  // Inject artificial failure for testing purposes. Note that autopilots should
  // implement an additional protection before accepting this command such as a
  // specific param setting.
  MAV_CMD_INJECT_FAILURE = 420,
  // Starts receiver pairing.
  MAV_CMD_START_RX_PAIR = 500,
  // Request the interval between messages for a particular MAVLink message ID.
  // The receiver should ACK the command and then emit its response in a
  // MESSAGE_INTERVAL
  // message.
  MAV_CMD_GET_MESSAGE_INTERVAL = 510,
  // Set the interval between messages for a particular MAVLink message ID. This
  // interface replaces REQUEST_DATA_STREAM.
  MAV_CMD_SET_MESSAGE_INTERVAL = 511,
  // Request the target system(s) emit a single instance of a specified message
  // (i.e. a "one-shot" version of MAV_CMD_SET_MESSAGE_INTERVAL).
  MAV_CMD_REQUEST_MESSAGE = 512,
  // Request MAVLink protocol version compatibility. All receivers should ACK
  // the
  // command and then emit their capabilities in an PROTOCOL_VERSION message
  MAV_CMD_REQUEST_PROTOCOL_VERSION = 519,
  // Request autopilot capabilities. The receiver should ACK the command and
  // then
  // emit its capabilities in an AUTOPILOT_VERSION message
  MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES = 520,
  // Request camera information (CAMERA_INFORMATION).
  MAV_CMD_REQUEST_CAMERA_INFORMATION = 521,
  // Request camera settings (CAMERA_SETTINGS).
  MAV_CMD_REQUEST_CAMERA_SETTINGS = 522,
  // Request storage information (STORAGE_INFORMATION). Use the command's
  // target_component
  // to target a specific component's storage.
  MAV_CMD_REQUEST_STORAGE_INFORMATION = 525,
  // Format a storage medium. Once format is complete, a STORAGE_INFORMATION
  // message
  // is sent. Use the command's target_component to target a specific
  // component's
  // storage.
  MAV_CMD_STORAGE_FORMAT = 526,
  // Request camera capture status (CAMERA_CAPTURE_STATUS)
  MAV_CMD_REQUEST_CAMERA_CAPTURE_STATUS = 527,
  // Request flight information (FLIGHT_INFORMATION)
  MAV_CMD_REQUEST_FLIGHT_INFORMATION = 528,
  // Reset all camera settings to Factory Default
  MAV_CMD_RESET_CAMERA_SETTINGS = 529,
  // Set camera running mode. Use NaN for reserved values. GCS will send a
  // MAV_CMD_REQUEST_VIDEO_STREAM_STATUS
  // command after a mode change if the camera supports video streaming.
  MAV_CMD_SET_CAMERA_MODE = 530,
  // Set camera zoom. Camera must respond with a CAMERA_SETTINGS message (on
  // success).
  MAV_CMD_SET_CAMERA_ZOOM = 531,
  // Set camera focus. Camera must respond with a CAMERA_SETTINGS message (on
  // success).
  MAV_CMD_SET_CAMERA_FOCUS = 532,
  // Set that a particular storage is the preferred location for saving photos,
  // videos, and/or other media (e.g. to set that an SD card is used for storing
  // videos). There can only be one preferred save location for each particular
  // media type: setting a media usage flag will clear/reset that same flag if
  // set
  // on any other storage. If no flag is set the system should use its default
  // storage.
  // A target system can choose to always use default storage, in which case it
  // should ACK the command with MAV_RESULT_UNSUPPORTED. A target system can
  // choose
  // to not allow a particular storage to be set as preferred storage, in which
  // case it should ACK the command with MAV_RESULT_DENIED.
  MAV_CMD_SET_STORAGE_USAGE = 533,
  // Tagged jump target. Can be jumped to with MAV_CMD_DO_JUMP_TAG.
  MAV_CMD_JUMP_TAG = 600,
  // Jump to the matching tag in the mission list. Repeat this action for the
  // specified
  // number of times. A mission should contain a single matching tag for each
  // jump.
  // If this is not the case then a jump to a missing tag should complete the
  // mission,
  // and a jump where there are multiple matching tags should always select the
  // one with the lowest mission sequence number.
  MAV_CMD_DO_JUMP_TAG = 601,
  // High level setpoint to be sent to a gimbal manager to set a gimbal
  // attitude.
  // It is possible to set combinations of the values below. E.g. an angle as
  // well
  // as a desired angular rate can be used to get to this angle at a certain
  // angular
  // rate, or an angular rate only will result in continuous turning. NaN is to
  // be used to signal unset. Note: a gimbal is never to react to this command
  // but
  // only the gimbal manager.
  MAV_CMD_DO_GIMBAL_MANAGER_PITCHYAW = 1000,
  // Gimbal configuration to set which sysid/compid is in primary and secondary
  // control.
  MAV_CMD_DO_GIMBAL_MANAGER_CONFIGURE = 1001,
  // Start image capture sequence. Sends CAMERA_IMAGE_CAPTURED after each
  // capture.
  // Use NaN for reserved values.
  MAV_CMD_IMAGE_START_CAPTURE = 2000,
  // Stop image capture sequence Use NaN for reserved values.
  MAV_CMD_IMAGE_STOP_CAPTURE = 2001,
  // Re-request a CAMERA_IMAGE_CAPTURED message.
  MAV_CMD_REQUEST_CAMERA_IMAGE_CAPTURE = 2002,
  // Enable or disable on-board camera triggering system.
  MAV_CMD_DO_TRIGGER_CONTROL = 2003,
  // If the camera supports point visual tracking
  // (CAMERA_CAP_FLAGS_HAS_TRACKING_POINT
  // is set), this command allows to initiate the tracking.
  MAV_CMD_CAMERA_TRACK_POINT = 2004,
  // If the camera supports rectangle visual tracking
  // (CAMERA_CAP_FLAGS_HAS_TRACKING_RECTANGLE
  // is set), this command allows to initiate the tracking.
  MAV_CMD_CAMERA_TRACK_RECTANGLE = 2005,
  // Stops ongoing tracking.
  MAV_CMD_CAMERA_STOP_TRACKING = 2010,
  // Starts video capture (recording).
  MAV_CMD_VIDEO_START_CAPTURE = 2500,
  // Stop the current video capture (recording).
  MAV_CMD_VIDEO_STOP_CAPTURE = 2501,
  // Start video streaming
  MAV_CMD_VIDEO_START_STREAMING = 2502,
  // Stop the given video stream
  MAV_CMD_VIDEO_STOP_STREAMING = 2503,
  // Request video stream information (VIDEO_STREAM_INFORMATION)
  MAV_CMD_REQUEST_VIDEO_STREAM_INFORMATION = 2504,
  // Request video stream status (VIDEO_STREAM_STATUS)
  MAV_CMD_REQUEST_VIDEO_STREAM_STATUS = 2505,
  // Request to start streaming logging data over MAVLink (see also LOGGING_DATA
  // message)
  MAV_CMD_LOGGING_START = 2510,
  // Request to stop streaming log data over MAVLink
  MAV_CMD_LOGGING_STOP = 2511,
  MAV_CMD_AIRFRAME_CONFIGURATION = 2520,
  // Request to start/stop transmitting over the high latency telemetry
  MAV_CMD_CONTROL_HIGH_LATENCY = 2600,
  // Create a panorama at the current position
  MAV_CMD_PANORAMA_CREATE = 2800,
  // Request VTOL transition
  MAV_CMD_DO_VTOL_TRANSITION = 3000,
  // Request authorization to arm the vehicle to a external entity, the arm
  // authorizer
  // is responsible to request all data that is needs from the vehicle before
  // authorize
  // or deny the request. If approved the progress of command_ack message should
  // be set with period of time that this authorization is valid in seconds or
  // in
  // case it was denied it should be set with one of the reasons in
  // ARM_AUTH_DENIED_REASON.
  MAV_CMD_ARM_AUTHORIZATION_REQUEST = 3001,
  // This command sets the submode to standard guided when vehicle is in guided
  // mode. The vehicle holds position and altitude and the user can input the
  // desired
  // velocities along all three axes.
  MAV_CMD_SET_GUIDED_SUBMODE_STANDARD = 4000,
  // This command sets submode circle when vehicle is in guided mode. Vehicle
  // flies
  // along a circle facing the center of the circle. The user can input the
  // velocity
  // along the circle and change the radius. If no input is given the vehicle
  // will
  // hold position.
  MAV_CMD_SET_GUIDED_SUBMODE_CIRCLE = 4001,
  // Delay mission state machine until gate has been reached.
  MAV_CMD_CONDITION_GATE = 4501,
  // Fence return point (there can only be one such point in a geofence
  // definition).
  // If rally points are supported they should be used instead.
  MAV_CMD_NAV_FENCE_RETURN_POINT = 5000,
  // Fence vertex for an inclusion polygon (the polygon must not be
  // self-intersecting).
  // The vehicle must stay within this area. Minimum of 3 vertices required.
  MAV_CMD_NAV_FENCE_POLYGON_VERTEX_INCLUSION = 5001,
  // Fence vertex for an exclusion polygon (the polygon must not be
  // self-intersecting).
  // The vehicle must stay outside this area. Minimum of 3 vertices required.
  MAV_CMD_NAV_FENCE_POLYGON_VERTEX_EXCLUSION = 5002,
  // Circular fence area. The vehicle must stay inside this area.
  MAV_CMD_NAV_FENCE_CIRCLE_INCLUSION = 5003,
  // Circular fence area. The vehicle must stay outside this area.
  MAV_CMD_NAV_FENCE_CIRCLE_EXCLUSION = 5004,
  // Rally point. You can have multiple rally points defined.
  MAV_CMD_NAV_RALLY_POINT = 5100,
  // Commands the vehicle to respond with a sequence of messages
  // UAVCAN_NODE_INFO,
  // one message per every UAVCAN node that is online. Note that some of the
  // response
  // messages can be lost, which the receiver can detect easily by checking
  // whether
  // every received UAVCAN_NODE_STATUS has a matching message UAVCAN_NODE_INFO
  // received
  // earlier; if not, this command should be sent again in order to request
  // re-transmission
  // of the node information messages.
  MAV_CMD_UAVCAN_GET_NODE_INFO = 5200,
  // Trigger the start of an ADSB-out IDENT. This should only be used when
  // requested
  // to do so by an Air Traffic Controller in controlled airspace. This starts
  // the
  // IDENT which is then typically held for 18 seconds by the hardware per the
  // Mode
  // A, C, and S transponder spec.
  MAV_CMD_DO_ADSB_OUT_IDENT = 10001,
  // Deploy payload on a Lat / Lon / Alt position. This includes the navigation
  // to reach the required release position and velocity.
  MAV_CMD_PAYLOAD_PREPARE_DEPLOY = 30001,
  // Control the payload deployment.
  MAV_CMD_PAYLOAD_CONTROL_DEPLOY = 30002,
  // Magnetometer calibration based on provided known yaw. This allows for fast
  // calibration using WMM field tables in the vehicle, given only the known yaw
  // of the vehicle. If Latitude and longitude are both zero then use the
  // current
  // vehicle location.
  MAV_CMD_FIXED_MAG_CAL_YAW = 42006,
  // Command to operate winch.
  MAV_CMD_DO_WINCH = 42600,
  // User defined waypoint item. Ground Station will show the Vehicle as flying
  // through this item.
  MAV_CMD_WAYPOINT_USER_1 = 31000,
  // User defined waypoint item. Ground Station will show the Vehicle as flying
  // through this item.
  MAV_CMD_WAYPOINT_USER_2 = 31001,
  // User defined waypoint item. Ground Station will show the Vehicle as flying
  // through this item.
  MAV_CMD_WAYPOINT_USER_3 = 31002,
  // User defined waypoint item. Ground Station will show the Vehicle as flying
  // through this item.
  MAV_CMD_WAYPOINT_USER_4 = 31003,
  // User defined waypoint item. Ground Station will show the Vehicle as flying
  // through this item.
  MAV_CMD_WAYPOINT_USER_5 = 31004,
  // User defined spatial item. Ground Station will not show the Vehicle as
  // flying
  // through this item. Example: ROI item.
  MAV_CMD_SPATIAL_USER_1 = 31005,
  // User defined spatial item. Ground Station will not show the Vehicle as
  // flying
  // through this item. Example: ROI item.
  MAV_CMD_SPATIAL_USER_2 = 31006,
  // User defined spatial item. Ground Station will not show the Vehicle as
  // flying
  // through this item. Example: ROI item.
  MAV_CMD_SPATIAL_USER_3 = 31007,
  // User defined spatial item. Ground Station will not show the Vehicle as
  // flying
  // through this item. Example: ROI item.
  MAV_CMD_SPATIAL_USER_4 = 31008,
  // User defined spatial item. Ground Station will not show the Vehicle as
  // flying
  // through this item. Example: ROI item.
  MAV_CMD_SPATIAL_USER_5 = 31009,
  // User defined command. Ground Station will not show the Vehicle as flying
  // through
  // this item. Example: MAV_CMD_DO_SET_PARAMETER item.
  MAV_CMD_USER_1 = 31010,
  // User defined command. Ground Station will not show the Vehicle as flying
  // through
  // this item. Example: MAV_CMD_DO_SET_PARAMETER item.
  MAV_CMD_USER_2 = 31011,
  // User defined command. Ground Station will not show the Vehicle as flying
  // through
  // this item. Example: MAV_CMD_DO_SET_PARAMETER item.
  MAV_CMD_USER_3 = 31012,
  // User defined command. Ground Station will not show the Vehicle as flying
  // through
  // this item. Example: MAV_CMD_DO_SET_PARAMETER item.
  MAV_CMD_USER_4 = 31013,
  // User defined command. Ground Station will not show the Vehicle as flying
  // through
  // this item. Example: MAV_CMD_DO_SET_PARAMETER item.
  MAV_CMD_USER_5 = 31014,
  // Request forwarding of CAN packets from the given CAN bus to this component.
  // CAN Frames are sent using CAN_FRAME and CANFD_FRAME messages
  MAV_CMD_CAN_FORWARD = 32000
};

// A data stream is not a fixed set of messages, but rather a recommendation to
// the autopilot software. Individual autopilots may or may not obey the
// recommended messages.
enum class MAV_DATA_STREAM {
  // Enable all data streams
  MAV_DATA_STREAM_ALL = 0,
  // Enable IMU_RAW, GPS_RAW, GPS_STATUS packets.
  MAV_DATA_STREAM_RAW_SENSORS = 1,
  // Enable GPS_STATUS, CONTROL_STATUS, AUX_STATUS
  MAV_DATA_STREAM_EXTENDED_STATUS = 2,
  // Enable RC_CHANNELS_SCALED, RC_CHANNELS_RAW, SERVO_OUTPUT_RAW
  MAV_DATA_STREAM_RC_CHANNELS = 3,
  // Enable ATTITUDE_CONTROLLER_OUTPUT, POSITION_CONTROLLER_OUTPUT,
  // NAV_CONTROLLER_OUTPUT.
  MAV_DATA_STREAM_RAW_CONTROLLER = 4,
  // Enable LOCAL_POSITION, GLOBAL_POSITION_INT messages.
  MAV_DATA_STREAM_POSITION = 6,
  // Dependent on the autopilot
  MAV_DATA_STREAM_EXTRA1 = 10,
  // Dependent on the autopilot
  MAV_DATA_STREAM_EXTRA2 = 11,
  // Dependent on the autopilot
  MAV_DATA_STREAM_EXTRA3 = 12
};

// The ROI (region of interest) for the vehicle. This can be be used by the
// vehicle for camera/vehicle attitude alignment (see MAV_CMD_NAV_ROI).
enum class MAV_ROI {
  // No region of interest.
  MAV_ROI_NONE = 0,
  // Point toward next waypoint, with optional pitch/roll/yaw offset.
  MAV_ROI_WPNEXT = 1,
  // Point toward given waypoint.
  MAV_ROI_WPINDEX = 2,
  // Point toward fixed location.
  MAV_ROI_LOCATION = 3,
  // Point toward of given id.
  MAV_ROI_TARGET = 4
};

// ACK / NACK / ERROR values as a result of MAV_CMDs and for mission item
// transmission.
enum class MAV_CMD_ACK {
  // Command / mission item is ok.
  MAV_CMD_ACK_OK = 0,
  // Generic error message if none of the other reasons fails or if no detailed
  // error reporting is implemented.
  MAV_CMD_ACK_ERR_FAIL = 1,
  // The system is refusing to accept this command from this source /
  // communication
  // partner.
  MAV_CMD_ACK_ERR_ACCESS_DENIED = 2,
  // Command or mission item is not supported, other commands would be accepted.
  MAV_CMD_ACK_ERR_NOT_SUPPORTED = 3,
  // The coordinate frame of this command / mission item is not supported.
  MAV_CMD_ACK_ERR_COORDINATE_FRAME_NOT_SUPPORTED = 4,
  // The coordinate frame of this command is ok, but he coordinate values exceed
  // the safety limits of this system. This is a generic error, please use the
  // more
  // specific error messages below if possible.
  MAV_CMD_ACK_ERR_COORDINATES_OUT_OF_RANGE = 5,
  // The X or latitude value is out of range.
  MAV_CMD_ACK_ERR_X_LAT_OUT_OF_RANGE = 6,
  // The Y or longitude value is out of range.
  MAV_CMD_ACK_ERR_Y_LON_OUT_OF_RANGE = 7,
  // The Z or altitude value is out of range.
  MAV_CMD_ACK_ERR_Z_ALT_OUT_OF_RANGE = 8
};

// Specifies the datatype of a MAVLink parameter.
enum class MAV_PARAM_TYPE {
  // 8-bit unsigned integer
  MAV_PARAM_TYPE_UINT8 = 1,
  // 8-bit signed integer
  MAV_PARAM_TYPE_INT8 = 2,
  // 16-bit unsigned integer
  MAV_PARAM_TYPE_UINT16 = 3,
  // 16-bit signed integer
  MAV_PARAM_TYPE_INT16 = 4,
  // 32-bit unsigned integer
  MAV_PARAM_TYPE_UINT32 = 5,
  // 32-bit signed integer
  MAV_PARAM_TYPE_INT32 = 6,
  // 64-bit unsigned integer
  MAV_PARAM_TYPE_UINT64 = 7,
  // 64-bit signed integer
  MAV_PARAM_TYPE_INT64 = 8,
  // 32-bit floating-point
  MAV_PARAM_TYPE_REAL32 = 9,
  // 64-bit floating-point
  MAV_PARAM_TYPE_REAL64 = 10
};

// Specifies the datatype of a MAVLink extended parameter.
enum class MAV_PARAM_EXT_TYPE {
  // 8-bit unsigned integer
  MAV_PARAM_EXT_TYPE_UINT8 = 1,
  // 8-bit signed integer
  MAV_PARAM_EXT_TYPE_INT8 = 2,
  // 16-bit unsigned integer
  MAV_PARAM_EXT_TYPE_UINT16 = 3,
  // 16-bit signed integer
  MAV_PARAM_EXT_TYPE_INT16 = 4,
  // 32-bit unsigned integer
  MAV_PARAM_EXT_TYPE_UINT32 = 5,
  // 32-bit signed integer
  MAV_PARAM_EXT_TYPE_INT32 = 6,
  // 64-bit unsigned integer
  MAV_PARAM_EXT_TYPE_UINT64 = 7,
  // 64-bit signed integer
  MAV_PARAM_EXT_TYPE_INT64 = 8,
  // 32-bit floating-point
  MAV_PARAM_EXT_TYPE_REAL32 = 9,
  // 64-bit floating-point
  MAV_PARAM_EXT_TYPE_REAL64 = 10,
  // Custom Type
  MAV_PARAM_EXT_TYPE_CUSTOM = 11
};

// Result from a MAVLink command (MAV_CMD)
enum class MAV_RESULT {
  // Command is valid (is supported and has valid parameters), and was executed.
  MAV_RESULT_ACCEPTED = 0,
  // Command is valid, but cannot be executed at this time. This is used to
  // indicate
  // a problem that should be fixed just by waiting (e.g. a state machine is
  // busy,
  // can't arm because have not got GPS lock, etc.). Retrying later should work.
  MAV_RESULT_TEMPORARILY_REJECTED = 1,
  // Command is invalid (is supported but has invalid parameters). Retrying same
  // command and parameters will not work.
  MAV_RESULT_DENIED = 2,
  // Command is not supported (unknown).
  MAV_RESULT_UNSUPPORTED = 3,
  // Command is valid, but execution has failed. This is used to indicate any
  // non-temporary
  // or unexpected problem, i.e. any problem that must be fixed before the
  // command
  // can succeed/be retried. For example, attempting to write a file when out of
  // memory, attempting to arm when sensors are not calibrated, etc.
  MAV_RESULT_FAILED = 4,
  // Command is valid and is being executed. This will be followed by further
  // progress
  // updates, i.e. the component may send further COMMAND_ACK messages with
  // result
  // MAV_RESULT_IN_PROGRESS (at a rate decided by the implementation), and must
  // terminate by sending a COMMAND_ACK message with final result of the
  // operation.
  // The COMMAND_ACK.progress field can be used to indicate the progress of the
  // operation.
  MAV_RESULT_IN_PROGRESS = 5,
  // Command has been cancelled (as a result of receiving a COMMAND_CANCEL
  // message).
  MAV_RESULT_CANCELLED = 6
};

// Result of mission operation (in a MISSION_ACK message).
enum class MAV_MISSION_RESULT {
  // mission accepted OK
  MAV_MISSION_ACCEPTED = 0,
  // Generic error / not accepting mission commands at all right now.
  MAV_MISSION_ERROR = 1,
  // Coordinate frame is not supported.
  MAV_MISSION_UNSUPPORTED_FRAME = 2,
  // Command is not supported.
  MAV_MISSION_UNSUPPORTED = 3,
  // Mission items exceed storage space.
  MAV_MISSION_NO_SPACE = 4,
  // One of the parameters has an invalid value.
  MAV_MISSION_INVALID = 5,
  // param1 has an invalid value.
  MAV_MISSION_INVALID_PARAM1 = 6,
  // param2 has an invalid value.
  MAV_MISSION_INVALID_PARAM2 = 7,
  // param3 has an invalid value.
  MAV_MISSION_INVALID_PARAM3 = 8,
  // param4 has an invalid value.
  MAV_MISSION_INVALID_PARAM4 = 9,
  // x / param5 has an invalid value.
  MAV_MISSION_INVALID_PARAM5_X = 10,
  // y / param6 has an invalid value.
  MAV_MISSION_INVALID_PARAM6_Y = 11,
  // z / param7 has an invalid value.
  MAV_MISSION_INVALID_PARAM7 = 12,
  // Mission item received out of sequence
  MAV_MISSION_INVALID_SEQUENCE = 13,
  // Not accepting any mission commands from this communication partner.
  MAV_MISSION_DENIED = 14,
  // Current mission operation cancelled (e.g. mission upload, mission
  // download).
  MAV_MISSION_OPERATION_CANCELLED = 15
};

// Indicates the severity level, generally used for status messages to indicate
// their relative urgency. Based on RFC-5424 using expanded definitions at:
// http://www.kiwisyslog.com/kb/info:-syslog-message-levels/.
enum class MAV_SEVERITY {
  // System is unusable. This is a "panic" condition.
  MAV_SEVERITY_EMERGENCY = 0,
  // Action should be taken immediately. Indicates error in non-critical
  // systems.
  MAV_SEVERITY_ALERT = 1,
  // Action must be taken immediately. Indicates failure in a primary system.
  MAV_SEVERITY_CRITICAL = 2,
  // Indicates an error in secondary/redundant systems.
  MAV_SEVERITY_ERROR = 3,
  // Indicates about a possible future error if this is not resolved within a
  // given
  // timeframe. Example would be a low battery warning.
  MAV_SEVERITY_WARNING = 4,
  // An unusual event has occurred, though not an error condition. This should
  // be
  // investigated for the root cause.
  MAV_SEVERITY_NOTICE = 5,
  // Normal operational messages. Useful for logging. No action is required for
  // these messages.
  MAV_SEVERITY_INFO = 6,
  // Useful non-operational messages that can assist in debugging. These should
  // not occur during normal operation.
  MAV_SEVERITY_DEBUG = 7
};

// Power supply status flags (bitmask)
enum class MAV_POWER_STATUS {
  // main brick power supply valid
  MAV_POWER_STATUS_BRICK_VALID = 1,
  // main servo power supply valid for FMU
  MAV_POWER_STATUS_SERVO_VALID = 2,
  // USB power is connected
  MAV_POWER_STATUS_USB_CONNECTED = 4,
  // peripheral supply is in over-current state
  MAV_POWER_STATUS_PERIPH_OVERCURRENT = 8,
  // hi-power peripheral supply is in over-current state
  MAV_POWER_STATUS_PERIPH_HIPOWER_OVERCURRENT = 16,
  // Power status has changed since boot
  MAV_POWER_STATUS_CHANGED = 32
};

// SERIAL_CONTROL device types
enum class SERIAL_CONTROL_DEV {
  // First telemetry port
  SERIAL_CONTROL_DEV_TELEM1 = 0,
  // Second telemetry port
  SERIAL_CONTROL_DEV_TELEM2 = 1,
  // First GPS port
  SERIAL_CONTROL_DEV_GPS1 = 2,
  // Second GPS port
  SERIAL_CONTROL_DEV_GPS2 = 3,
  // system shell
  SERIAL_CONTROL_DEV_SHELL = 10,
  // SERIAL0
  SERIAL_CONTROL_SERIAL0 = 100,
  // SERIAL1
  SERIAL_CONTROL_SERIAL1 = 101,
  // SERIAL2
  SERIAL_CONTROL_SERIAL2 = 102,
  // SERIAL3
  SERIAL_CONTROL_SERIAL3 = 103,
  // SERIAL4
  SERIAL_CONTROL_SERIAL4 = 104,
  // SERIAL5
  SERIAL_CONTROL_SERIAL5 = 105,
  // SERIAL6
  SERIAL_CONTROL_SERIAL6 = 106,
  // SERIAL7
  SERIAL_CONTROL_SERIAL7 = 107,
  // SERIAL8
  SERIAL_CONTROL_SERIAL8 = 108,
  // SERIAL9
  SERIAL_CONTROL_SERIAL9 = 109
};

// SERIAL_CONTROL flags (bitmask)
enum class SERIAL_CONTROL_FLAG {
  // Set if this is a reply
  SERIAL_CONTROL_FLAG_REPLY = 1,
  // Set if the sender wants the receiver to send a response as another
  // SERIAL_CONTROL
  // message
  SERIAL_CONTROL_FLAG_RESPOND = 2,
  // Set if access to the serial port should be removed from whatever driver is
  // currently using it, giving exclusive access to the SERIAL_CONTROL protocol.
  // The port can be handed back by sending a request without this flag set
  SERIAL_CONTROL_FLAG_EXCLUSIVE = 4,
  // Block on writes to the serial port
  SERIAL_CONTROL_FLAG_BLOCKING = 8,
  // Send multiple replies until port is drained
  SERIAL_CONTROL_FLAG_MULTI = 16
};

// Enumeration of distance sensor types
enum class MAV_DISTANCE_SENSOR {
  // Laser rangefinder, e.g. LightWare SF02/F or PulsedLight units
  MAV_DISTANCE_SENSOR_LASER = 0,
  // Ultrasound rangefinder, e.g. MaxBotix units
  MAV_DISTANCE_SENSOR_ULTRASOUND = 1,
  // Infrared rangefinder, e.g. Sharp units
  MAV_DISTANCE_SENSOR_INFRARED = 2,
  // Radar type, e.g. uLanding units
  MAV_DISTANCE_SENSOR_RADAR = 3,
  // Broken or unknown type, e.g. analog units
  MAV_DISTANCE_SENSOR_UNKNOWN = 4
};

// Enumeration of sensor orientation, according to its rotations
enum class MAV_SENSOR_ORIENTATION {
  // Roll: 0, Pitch: 0, Yaw: 0
  MAV_SENSOR_ROTATION_NONE = 0,
  // Roll: 0, Pitch: 0, Yaw: 45
  MAV_SENSOR_ROTATION_YAW_45 = 1,
  // Roll: 0, Pitch: 0, Yaw: 90
  MAV_SENSOR_ROTATION_YAW_90 = 2,
  // Roll: 0, Pitch: 0, Yaw: 135
  MAV_SENSOR_ROTATION_YAW_135 = 3,
  // Roll: 0, Pitch: 0, Yaw: 180
  MAV_SENSOR_ROTATION_YAW_180 = 4,
  // Roll: 0, Pitch: 0, Yaw: 225
  MAV_SENSOR_ROTATION_YAW_225 = 5,
  // Roll: 0, Pitch: 0, Yaw: 270
  MAV_SENSOR_ROTATION_YAW_270 = 6,
  // Roll: 0, Pitch: 0, Yaw: 315
  MAV_SENSOR_ROTATION_YAW_315 = 7,
  // Roll: 180, Pitch: 0, Yaw: 0
  MAV_SENSOR_ROTATION_ROLL_180 = 8,
  // Roll: 180, Pitch: 0, Yaw: 45
  MAV_SENSOR_ROTATION_ROLL_180_YAW_45 = 9,
  // Roll: 180, Pitch: 0, Yaw: 90
  MAV_SENSOR_ROTATION_ROLL_180_YAW_90 = 10,
  // Roll: 180, Pitch: 0, Yaw: 135
  MAV_SENSOR_ROTATION_ROLL_180_YAW_135 = 11,
  // Roll: 0, Pitch: 180, Yaw: 0
  MAV_SENSOR_ROTATION_PITCH_180 = 12,
  // Roll: 180, Pitch: 0, Yaw: 225
  MAV_SENSOR_ROTATION_ROLL_180_YAW_225 = 13,
  // Roll: 180, Pitch: 0, Yaw: 270
  MAV_SENSOR_ROTATION_ROLL_180_YAW_270 = 14,
  // Roll: 180, Pitch: 0, Yaw: 315
  MAV_SENSOR_ROTATION_ROLL_180_YAW_315 = 15,
  // Roll: 90, Pitch: 0, Yaw: 0
  MAV_SENSOR_ROTATION_ROLL_90 = 16,
  // Roll: 90, Pitch: 0, Yaw: 45
  MAV_SENSOR_ROTATION_ROLL_90_YAW_45 = 17,
  // Roll: 90, Pitch: 0, Yaw: 90
  MAV_SENSOR_ROTATION_ROLL_90_YAW_90 = 18,
  // Roll: 90, Pitch: 0, Yaw: 135
  MAV_SENSOR_ROTATION_ROLL_90_YAW_135 = 19,
  // Roll: 270, Pitch: 0, Yaw: 0
  MAV_SENSOR_ROTATION_ROLL_270 = 20,
  // Roll: 270, Pitch: 0, Yaw: 45
  MAV_SENSOR_ROTATION_ROLL_270_YAW_45 = 21,
  // Roll: 270, Pitch: 0, Yaw: 90
  MAV_SENSOR_ROTATION_ROLL_270_YAW_90 = 22,
  // Roll: 270, Pitch: 0, Yaw: 135
  MAV_SENSOR_ROTATION_ROLL_270_YAW_135 = 23,
  // Roll: 0, Pitch: 90, Yaw: 0
  MAV_SENSOR_ROTATION_PITCH_90 = 24,
  // Roll: 0, Pitch: 270, Yaw: 0
  MAV_SENSOR_ROTATION_PITCH_270 = 25,
  // Roll: 0, Pitch: 180, Yaw: 90
  MAV_SENSOR_ROTATION_PITCH_180_YAW_90 = 26,
  // Roll: 0, Pitch: 180, Yaw: 270
  MAV_SENSOR_ROTATION_PITCH_180_YAW_270 = 27,
  // Roll: 90, Pitch: 90, Yaw: 0
  MAV_SENSOR_ROTATION_ROLL_90_PITCH_90 = 28,
  // Roll: 180, Pitch: 90, Yaw: 0
  MAV_SENSOR_ROTATION_ROLL_180_PITCH_90 = 29,
  // Roll: 270, Pitch: 90, Yaw: 0
  MAV_SENSOR_ROTATION_ROLL_270_PITCH_90 = 30,
  // Roll: 90, Pitch: 180, Yaw: 0
  MAV_SENSOR_ROTATION_ROLL_90_PITCH_180 = 31,
  // Roll: 270, Pitch: 180, Yaw: 0
  MAV_SENSOR_ROTATION_ROLL_270_PITCH_180 = 32,
  // Roll: 90, Pitch: 270, Yaw: 0
  MAV_SENSOR_ROTATION_ROLL_90_PITCH_270 = 33,
  // Roll: 180, Pitch: 270, Yaw: 0
  MAV_SENSOR_ROTATION_ROLL_180_PITCH_270 = 34,
  // Roll: 270, Pitch: 270, Yaw: 0
  MAV_SENSOR_ROTATION_ROLL_270_PITCH_270 = 35,
  // Roll: 90, Pitch: 180, Yaw: 90
  MAV_SENSOR_ROTATION_ROLL_90_PITCH_180_YAW_90 = 36,
  // Roll: 90, Pitch: 0, Yaw: 270
  MAV_SENSOR_ROTATION_ROLL_90_YAW_270 = 37,
  // Roll: 90, Pitch: 68, Yaw: 293
  MAV_SENSOR_ROTATION_ROLL_90_PITCH_68_YAW_293 = 38,
  // Pitch: 315
  MAV_SENSOR_ROTATION_PITCH_315 = 39,
  // Roll: 90, Pitch: 315
  MAV_SENSOR_ROTATION_ROLL_90_PITCH_315 = 40,
  // Custom orientation
  MAV_SENSOR_ROTATION_CUSTOM = 100
};

// Bitmask of (optional) autopilot capabilities (64 bit). If a bit is set, the
// autopilot supports this capability.
enum class MAV_PROTOCOL_CAPABILITY {
  // Autopilot supports the MISSION_ITEM float message type. Note that
  // MISSION_ITEM
  // is deprecated, and autopilots should use MISSION_INT instead.
  MAV_PROTOCOL_CAPABILITY_MISSION_FLOAT = 1,
  // Autopilot supports the new param float message type.
  MAV_PROTOCOL_CAPABILITY_PARAM_FLOAT = 2,
  // Autopilot supports MISSION_ITEM_INT scaled integer message type. Note that
  // this flag must always be set if missions are supported, because missions
  // must
  // always use MISSION_ITEM_INT (rather than MISSION_ITEM, which is
  // deprecated).
  MAV_PROTOCOL_CAPABILITY_MISSION_INT = 4,
  // Autopilot supports COMMAND_INT scaled integer message type.
  MAV_PROTOCOL_CAPABILITY_COMMAND_INT = 8,
  // Parameter protocol uses byte-wise encoding of parameter values into
  // param_value
  // (float) fields:
  // https://mavlink.io/en/services/parameter.html#parameter-encoding.
  // Note that either this flag or MAV_PROTOCOL_CAPABILITY_PARAM_ENCODE_BYTEWISE
  // should be set if the parameter protocol is supported.
  MAV_PROTOCOL_CAPABILITY_PARAM_ENCODE_BYTEWISE = 16,
  // Autopilot supports the File Transfer Protocol v1:
  // https://mavlink.io/en/services/ftp.html.
  MAV_PROTOCOL_CAPABILITY_FTP = 32,
  // Autopilot supports commanding attitude offboard.
  MAV_PROTOCOL_CAPABILITY_SET_ATTITUDE_TARGET = 64,
  // Autopilot supports commanding position and velocity targets in local NED
  // frame.
  MAV_PROTOCOL_CAPABILITY_SET_POSITION_TARGET_LOCAL_NED = 128,
  // Autopilot supports commanding position and velocity targets in global
  // scaled
  // integers.
  MAV_PROTOCOL_CAPABILITY_SET_POSITION_TARGET_GLOBAL_INT = 256,
  // Autopilot supports terrain protocol / data handling.
  MAV_PROTOCOL_CAPABILITY_TERRAIN = 512,
  // Autopilot supports direct actuator control.
  MAV_PROTOCOL_CAPABILITY_SET_ACTUATOR_TARGET = 1024,
  // Autopilot supports the MAV_CMD_DO_FLIGHTTERMINATION command (flight
  // termination).
  MAV_PROTOCOL_CAPABILITY_FLIGHT_TERMINATION = 2048,
  // Autopilot supports onboard compass calibration.
  MAV_PROTOCOL_CAPABILITY_COMPASS_CALIBRATION = 4096,
  // Autopilot supports MAVLink version 2.
  MAV_PROTOCOL_CAPABILITY_MAVLINK2 = 8192,
  // Autopilot supports mission fence protocol.
  MAV_PROTOCOL_CAPABILITY_MISSION_FENCE = 16384,
  // Autopilot supports mission rally point protocol.
  MAV_PROTOCOL_CAPABILITY_MISSION_RALLY = 32768,
  // Reserved for future use.
  MAV_PROTOCOL_CAPABILITY_RESERVED2 = 65536,
  // Parameter protocol uses C-cast of parameter values to set the param_value
  // (float)
  // fields: https://mavlink.io/en/services/parameter.html#parameter-encoding.
  // Note
  // that either this flag or MAV_PROTOCOL_CAPABILITY_PARAM_ENCODE_BYTEWISE
  // should
  // be set if the parameter protocol is supported.
  MAV_PROTOCOL_CAPABILITY_PARAM_ENCODE_C_CAST = 131072
};

// Type of mission items being requested/sent in mission protocol.
enum class MAV_MISSION_TYPE {
  // Items are mission commands for main mission.
  MAV_MISSION_TYPE_MISSION = 0,
  // Specifies GeoFence area(s). Items are MAV_CMD_NAV_FENCE_ GeoFence items.
  MAV_MISSION_TYPE_FENCE = 1,
  // Specifies the rally points for the vehicle. Rally points are alternative
  // RTL
  // points. Items are MAV_CMD_NAV_RALLY_POINT rally point items.
  MAV_MISSION_TYPE_RALLY = 2,
  // Only used in MISSION_CLEAR_ALL to clear all mission types.
  MAV_MISSION_TYPE_ALL = 255
};

// Enumeration of estimator types
enum class MAV_ESTIMATOR_TYPE {
  // Unknown type of the estimator.
  MAV_ESTIMATOR_TYPE_UNKNOWN = 0,
  // This is a naive estimator without any real covariance feedback.
  MAV_ESTIMATOR_TYPE_NAIVE = 1,
  // Computer vision based estimate. Might be up to scale.
  MAV_ESTIMATOR_TYPE_VISION = 2,
  // Visual-inertial estimate.
  MAV_ESTIMATOR_TYPE_VIO = 3,
  // Plain GPS estimate.
  MAV_ESTIMATOR_TYPE_GPS = 4,
  // Estimator integrating GPS and inertial sensing.
  MAV_ESTIMATOR_TYPE_GPS_INS = 5,
  // Estimate from external motion capturing system.
  MAV_ESTIMATOR_TYPE_MOCAP = 6,
  // Estimator based on lidar sensor input.
  MAV_ESTIMATOR_TYPE_LIDAR = 7,
  // Estimator on autopilot.
  MAV_ESTIMATOR_TYPE_AUTOPILOT = 8
};

// Enumeration of battery types
enum class MAV_BATTERY_TYPE {
  // Not specified.
  MAV_BATTERY_TYPE_UNKNOWN = 0,
  // Lithium polymer battery
  MAV_BATTERY_TYPE_LIPO = 1,
  // Lithium-iron-phosphate battery
  MAV_BATTERY_TYPE_LIFE = 2,
  // Lithium-ION battery
  MAV_BATTERY_TYPE_LION = 3,
  // Nickel metal hydride battery
  MAV_BATTERY_TYPE_NIMH = 4
};

// Enumeration of battery functions
enum class MAV_BATTERY_FUNCTION {
  // Battery function is unknown
  MAV_BATTERY_FUNCTION_UNKNOWN = 0,
  // Battery supports all flight systems
  MAV_BATTERY_FUNCTION_ALL = 1,
  // Battery for the propulsion system
  MAV_BATTERY_FUNCTION_PROPULSION = 2,
  // Avionics battery
  MAV_BATTERY_FUNCTION_AVIONICS = 3,
  // Payload battery
  MAV_BATTERY_TYPE_PAYLOAD = 4
};

// Enumeration for battery charge states.
enum class MAV_BATTERY_CHARGE_STATE {
  // Low battery state is not provided
  MAV_BATTERY_CHARGE_STATE_UNDEFINED = 0,
  // Battery is not in low state. Normal operation.
  MAV_BATTERY_CHARGE_STATE_OK = 1,
  // Battery state is low, warn and monitor close.
  MAV_BATTERY_CHARGE_STATE_LOW = 2,
  // Battery state is critical, return or abort immediately.
  MAV_BATTERY_CHARGE_STATE_CRITICAL = 3,
  // Battery state is too low for ordinary abort sequence. Perform fastest
  // possible
  // emergency stop to prevent damage.
  MAV_BATTERY_CHARGE_STATE_EMERGENCY = 4,
  // Battery failed, damage unavoidable. Possible causes (faults) are listed in
  // MAV_BATTERY_FAULT.
  MAV_BATTERY_CHARGE_STATE_FAILED = 5,
  // Battery is diagnosed to be defective or an error occurred, usage is
  // discouraged
  // / prohibited. Possible causes (faults) are listed in MAV_BATTERY_FAULT.
  MAV_BATTERY_CHARGE_STATE_UNHEALTHY = 6,
  // Battery is charging.
  MAV_BATTERY_CHARGE_STATE_CHARGING = 7
};

// Battery mode. Note, the normal operation mode (i.e. when flying) should be
// reported as MAV_BATTERY_MODE_UNKNOWN to allow message trimming in normal
// flight.
enum class MAV_BATTERY_MODE {
  // Battery mode not supported/unknown battery mode/normal operation.
  MAV_BATTERY_MODE_UNKNOWN = 0,
  // Battery is auto discharging (towards storage level).
  MAV_BATTERY_MODE_AUTO_DISCHARGING = 1,
  // Battery in hot-swap mode (current limited to prevent spikes that might
  // damage
  // sensitive electrical circuits).
  MAV_BATTERY_MODE_HOT_SWAP = 2
};

// Smart battery supply status/fault flags (bitmask) for health indication. The
// battery must also report either MAV_BATTERY_CHARGE_STATE_FAILED or
// MAV_BATTERY_CHARGE_STATE_UNHEALTHY if any of these are set.
enum class MAV_BATTERY_FAULT {
  // Battery has deep discharged.
  MAV_BATTERY_FAULT_DEEP_DISCHARGE = 1,
  // Voltage spikes.
  MAV_BATTERY_FAULT_SPIKES = 2,
  // One or more cells have failed. Battery should also report
  // MAV_BATTERY_CHARGE_STATE_FAILE
  // (and should not be used).
  MAV_BATTERY_FAULT_CELL_FAIL = 4,
  // Over-current fault.
  MAV_BATTERY_FAULT_OVER_CURRENT = 8,
  // Over-temperature fault.
  MAV_BATTERY_FAULT_OVER_TEMPERATURE = 16,
  // Under-temperature fault.
  MAV_BATTERY_FAULT_UNDER_TEMPERATURE = 32,
  // Vehicle voltage is not compatible with this battery (batteries on same
  // power
  // rail should have similar voltage).
  MAV_BATTERY_FAULT_INCOMPATIBLE_VOLTAGE = 64,
  // Battery firmware is not compatible with current autopilot firmware.
  MAV_BATTERY_FAULT_INCOMPATIBLE_FIRMWARE = 128,
  // Battery is not compatible due to cell configuration (e.g. 5s1p when vehicle
  // requires 6s).
  BATTERY_FAULT_INCOMPATIBLE_CELLS_CONFIGURATION = 256
};

// Flags to report status/failure cases for a power generator (used in
// GENERATOR_STATUS). Note that FAULTS are conditions that cause the generator
// to fail. Warnings are conditions that require attention before the next use
// (they indicate the system is not operating properly).
enum class MAV_GENERATOR_STATUS_FLAG {
  // Generator is off.
  MAV_GENERATOR_STATUS_FLAG_OFF = 1,
  // Generator is ready to start generating power.
  MAV_GENERATOR_STATUS_FLAG_READY = 2,
  // Generator is generating power.
  MAV_GENERATOR_STATUS_FLAG_GENERATING = 4,
  // Generator is charging the batteries (generating enough power to charge and
  // provide the load).
  MAV_GENERATOR_STATUS_FLAG_CHARGING = 8,
  // Generator is operating at a reduced maximum power.
  MAV_GENERATOR_STATUS_FLAG_REDUCED_POWER = 16,
  // Generator is providing the maximum output.
  MAV_GENERATOR_STATUS_FLAG_MAXPOWER = 32,
  // Generator is near the maximum operating temperature, cooling is
  // insufficient.
  MAV_GENERATOR_STATUS_FLAG_OVERTEMP_WARNING = 64,
  // Generator hit the maximum operating temperature and shutdown.
  MAV_GENERATOR_STATUS_FLAG_OVERTEMP_FAULT = 128,
  // Power electronics are near the maximum operating temperature, cooling is
  // insufficient.
  MAV_GENERATOR_STATUS_FLAG_ELECTRONICS_OVERTEMP_WARNING = 256,
  // Power electronics hit the maximum operating temperature and shutdown.
  MAV_GENERATOR_STATUS_FLAG_ELECTRONICS_OVERTEMP_FAULT = 512,
  // Power electronics experienced a fault and shutdown.
  MAV_GENERATOR_STATUS_FLAG_ELECTRONICS_FAULT = 1024,
  // The power source supplying the generator failed e.g. mechanical generator
  // stopped,
  // tether is no longer providing power, solar cell is in shade, hydrogen
  // reaction
  // no longer happening.
  MAV_GENERATOR_STATUS_FLAG_POWERSOURCE_FAULT = 2048,
  // Generator controller having communication problems.
  MAV_GENERATOR_STATUS_FLAG_COMMUNICATION_WARNING = 4096,
  // Power electronic or generator cooling system error.
  MAV_GENERATOR_STATUS_FLAG_COOLING_WARNING = 8192,
  // Generator controller power rail experienced a fault.
  MAV_GENERATOR_STATUS_FLAG_POWER_RAIL_FAULT = 16384,
  // Generator controller exceeded the overcurrent threshold and shutdown to
  // prevent
  // damage.
  MAV_GENERATOR_STATUS_FLAG_OVERCURRENT_FAULT = 32768,
  // Generator controller detected a high current going into the batteries and
  // shutdown
  // to prevent battery damage.
  MAV_GENERATOR_STATUS_FLAG_BATTERY_OVERCHARGE_CURRENT_FAULT = 65536,
  // Generator controller exceeded it's overvoltage threshold and shutdown to
  // prevent
  // it exceeding the voltage rating.
  MAV_GENERATOR_STATUS_FLAG_OVERVOLTAGE_FAULT = 131072,
  // Batteries are under voltage (generator will not start).
  MAV_GENERATOR_STATUS_FLAG_BATTERY_UNDERVOLT_FAULT = 262144,
  // Generator start is inhibited by e.g. a safety switch.
  MAV_GENERATOR_STATUS_FLAG_START_INHIBITED = 524288,
  // Generator requires maintenance.
  MAV_GENERATOR_STATUS_FLAG_MAINTENANCE_REQUIRED = 1048576,
  // Generator is not ready to generate yet.
  MAV_GENERATOR_STATUS_FLAG_WARMING_UP = 2097152,
  // Generator is idle.
  MAV_GENERATOR_STATUS_FLAG_IDLE = 4194304
};

// Enumeration of VTOL states
enum class MAV_VTOL_STATE {
  // MAV is not configured as VTOL
  MAV_VTOL_STATE_UNDEFINED = 0,
  // VTOL is in transition from multicopter to fixed-wing
  MAV_VTOL_STATE_TRANSITION_TO_FW = 1,
  // VTOL is in transition from fixed-wing to multicopter
  MAV_VTOL_STATE_TRANSITION_TO_MC = 2,
  // VTOL is in multicopter state
  MAV_VTOL_STATE_MC = 3,
  // VTOL is in fixed-wing state
  MAV_VTOL_STATE_FW = 4
};

// Enumeration of landed detector states
enum class MAV_LANDED_STATE {
  // MAV landed state is unknown
  MAV_LANDED_STATE_UNDEFINED = 0,
  // MAV is landed (on ground)
  MAV_LANDED_STATE_ON_GROUND = 1,
  // MAV is in air
  MAV_LANDED_STATE_IN_AIR = 2,
  // MAV currently taking off
  MAV_LANDED_STATE_TAKEOFF = 3,
  // MAV currently landing
  MAV_LANDED_STATE_LANDING = 4
};

// Enumeration of the ADSB altimeter types
enum class ADSB_ALTITUDE_TYPE {
  // Altitude reported from a Baro source using QNH reference
  ADSB_ALTITUDE_TYPE_PRESSURE_QNH = 0,
  // Altitude reported from a GNSS source
  ADSB_ALTITUDE_TYPE_GEOMETRIC = 1
};

// ADSB classification for the type of vehicle emitting the transponder signal
enum class ADSB_EMITTER_TYPE {
  ADSB_EMITTER_TYPE_NO_INFO = 0,
  ADSB_EMITTER_TYPE_LIGHT = 1,
  ADSB_EMITTER_TYPE_SMALL = 2,
  ADSB_EMITTER_TYPE_LARGE = 3,
  ADSB_EMITTER_TYPE_HIGH_VORTEX_LARGE = 4,
  ADSB_EMITTER_TYPE_HEAVY = 5,
  ADSB_EMITTER_TYPE_HIGHLY_MANUV = 6,
  ADSB_EMITTER_TYPE_ROTOCRAFT = 7,
  ADSB_EMITTER_TYPE_UNASSIGNED = 8,
  ADSB_EMITTER_TYPE_GLIDER = 9,
  ADSB_EMITTER_TYPE_LIGHTER_AIR = 10,
  ADSB_EMITTER_TYPE_PARACHUTE = 11,
  ADSB_EMITTER_TYPE_ULTRA_LIGHT = 12,
  ADSB_EMITTER_TYPE_UNASSIGNED2 = 13,
  ADSB_EMITTER_TYPE_UAV = 14,
  ADSB_EMITTER_TYPE_SPACE = 15,
  ADSB_EMITTER_TYPE_UNASSGINED3 = 16,
  ADSB_EMITTER_TYPE_EMERGENCY_SURFACE = 17,
  ADSB_EMITTER_TYPE_SERVICE_SURFACE = 18,
  ADSB_EMITTER_TYPE_POINT_OBSTACLE = 19
};

// These flags indicate status such as data validity of each data source. Set =
// data valid
enum class ADSB_FLAGS {
  ADSB_FLAGS_VALID_COORDS = 1,
  ADSB_FLAGS_VALID_ALTITUDE = 2,
  ADSB_FLAGS_VALID_HEADING = 4,
  ADSB_FLAGS_VALID_VELOCITY = 8,
  ADSB_FLAGS_VALID_CALLSIGN = 16,
  ADSB_FLAGS_VALID_SQUAWK = 32,
  ADSB_FLAGS_SIMULATED = 64,
  ADSB_FLAGS_VERTICAL_VELOCITY_VALID = 128,
  ADSB_FLAGS_BARO_VALID = 256,
  ADSB_FLAGS_SOURCE_UAT = 32768
};

// Bitmap of options for the MAV_CMD_DO_REPOSITION
enum class MAV_DO_REPOSITION_FLAGS {
  // The aircraft should immediately transition into guided. This should not be
  // set for follow me applications
  MAV_DO_REPOSITION_FLAGS_CHANGE_MODE = 1
};

// Flags in ESTIMATOR_STATUS message
enum class ESTIMATOR_STATUS_FLAGS {
  // True if the attitude estimate is good
  ESTIMATOR_ATTITUDE = 1,
  // True if the horizontal velocity estimate is good
  ESTIMATOR_VELOCITY_HORIZ = 2,
  // True if the vertical velocity estimate is good
  ESTIMATOR_VELOCITY_VERT = 4,
  // True if the horizontal position (relative) estimate is good
  ESTIMATOR_POS_HORIZ_REL = 8,
  // True if the horizontal position (absolute) estimate is good
  ESTIMATOR_POS_HORIZ_ABS = 16,
  // True if the vertical position (absolute) estimate is good
  ESTIMATOR_POS_VERT_ABS = 32,
  // True if the vertical position (above ground) estimate is good
  ESTIMATOR_POS_VERT_AGL = 64,
  // True if the EKF is in a constant position mode and is not using external
  // measurements
  // (eg GPS or optical flow)
  ESTIMATOR_CONST_POS_MODE = 128,
  // True if the EKF has sufficient data to enter a mode that will provide a
  // (relative)
  // position estimate
  ESTIMATOR_PRED_POS_HORIZ_REL = 256,
  // True if the EKF has sufficient data to enter a mode that will provide a
  // (absolute)
  // position estimate
  ESTIMATOR_PRED_POS_HORIZ_ABS = 512,
  // True if the EKF has detected a GPS glitch
  ESTIMATOR_GPS_GLITCH = 1024,
  // True if the EKF has detected bad accelerometer data
  ESTIMATOR_ACCEL_ERROR = 2048
};

// Sequence that motors are tested when using MAV_CMD_DO_MOTOR_TEST.
enum class MOTOR_TEST_ORDER {
  // Default autopilot motor test method.
  MOTOR_TEST_ORDER_DEFAULT = 0,
  // Motor numbers are specified as their index in a predefined vehicle-specific
  // sequence.
  MOTOR_TEST_ORDER_SEQUENCE = 1,
  // Motor numbers are specified as the output as labeled on the board.
  MOTOR_TEST_ORDER_BOARD = 2
};

// Defines how throttle value is represented in MAV_CMD_DO_MOTOR_TEST.
enum class MOTOR_TEST_THROTTLE_TYPE {
  // Throttle as a percentage (0 ~ 100)
  MOTOR_TEST_THROTTLE_PERCENT = 0,
  // Throttle as an absolute PWM value (normally in range of 1000~2000).
  MOTOR_TEST_THROTTLE_PWM = 1,
  // Throttle pass-through from pilot's transmitter.
  MOTOR_TEST_THROTTLE_PILOT = 2,
  // Per-motor compass calibration test.
  MOTOR_TEST_COMPASS_CAL = 3
};

enum class GPS_INPUT_IGNORE_FLAGS {
  // ignore altitude field
  GPS_INPUT_IGNORE_FLAG_ALT = 1,
  // ignore hdop field
  GPS_INPUT_IGNORE_FLAG_HDOP = 2,
  // ignore vdop field
  GPS_INPUT_IGNORE_FLAG_VDOP = 4,
  // ignore horizontal velocity field (vn and ve)
  GPS_INPUT_IGNORE_FLAG_VEL_HORIZ = 8,
  // ignore vertical velocity field (vd)
  GPS_INPUT_IGNORE_FLAG_VEL_VERT = 16,
  // ignore speed accuracy field
  GPS_INPUT_IGNORE_FLAG_SPEED_ACCURACY = 32,
  // ignore horizontal accuracy field
  GPS_INPUT_IGNORE_FLAG_HORIZONTAL_ACCURACY = 64,
  // ignore vertical accuracy field
  GPS_INPUT_IGNORE_FLAG_VERTICAL_ACCURACY = 128
};

// Possible actions an aircraft can take to avoid a collision.
enum class MAV_COLLISION_ACTION {
  // Ignore any potential collisions
  MAV_COLLISION_ACTION_NONE = 0,
  // Report potential collision
  MAV_COLLISION_ACTION_REPORT = 1,
  // Ascend or Descend to avoid threat
  MAV_COLLISION_ACTION_ASCEND_OR_DESCEND = 2,
  // Move horizontally to avoid threat
  MAV_COLLISION_ACTION_MOVE_HORIZONTALLY = 3,
  // Aircraft to move perpendicular to the collision's velocity vector
  MAV_COLLISION_ACTION_MOVE_PERPENDICULAR = 4,
  // Aircraft to fly directly back to its launch point
  MAV_COLLISION_ACTION_RTL = 5,
  // Aircraft to stop in place
  MAV_COLLISION_ACTION_HOVER = 6
};

// Aircraft-rated danger from this threat.
enum class MAV_COLLISION_THREAT_LEVEL {
  // Not a threat
  MAV_COLLISION_THREAT_LEVEL_NONE = 0,
  // Craft is mildly concerned about this threat
  MAV_COLLISION_THREAT_LEVEL_LOW = 1,
  // Craft is panicking, and may take actions to avoid threat
  MAV_COLLISION_THREAT_LEVEL_HIGH = 2
};

// Source of information about this collision.
enum class MAV_COLLISION_SRC {
  // ID field references ADSB_VEHICLE packets
  MAV_COLLISION_SRC_ADSB = 0,
  // ID field references MAVLink SRC ID
  MAV_COLLISION_SRC_MAVLINK_GPS_GLOBAL_INT = 1
};

// Type of GPS fix
enum class GPS_FIX_TYPE {
  // No GPS connected
  GPS_FIX_TYPE_NO_GPS = 0,
  // No position information, GPS is connected
  GPS_FIX_TYPE_NO_FIX = 1,
  // 2D position
  GPS_FIX_TYPE_2D_FIX = 2,
  // 3D position
  GPS_FIX_TYPE_3D_FIX = 3,
  // DGPS/SBAS aided 3D position
  GPS_FIX_TYPE_DGPS = 4,
  // RTK float, 3D position
  GPS_FIX_TYPE_RTK_FLOAT = 5,
  // RTK Fixed, 3D position
  GPS_FIX_TYPE_RTK_FIXED = 6,
  // Static fixed, typically used for base stations
  GPS_FIX_TYPE_STATIC = 7,
  // PPP, 3D position.
  GPS_FIX_TYPE_PPP = 8
};

// RTK GPS baseline coordinate system, used for RTK corrections
enum class RTK_BASELINE_COORDINATE_SYSTEM {
  // Earth-centered, Earth-fixed
  RTK_BASELINE_COORDINATE_SYSTEM_ECEF = 0,
  // RTK basestation centered, north, east, down
  RTK_BASELINE_COORDINATE_SYSTEM_NED = 1
};

// Type of landing target
enum class LANDING_TARGET_TYPE {
  // Landing target signaled by light beacon (ex: IR-LOCK)
  LANDING_TARGET_TYPE_LIGHT_BEACON = 0,
  // Landing target signaled by radio beacon (ex: ILS, NDB)
  LANDING_TARGET_TYPE_RADIO_BEACON = 1,
  // Landing target represented by a fiducial marker (ex: ARTag)
  LANDING_TARGET_TYPE_VISION_FIDUCIAL = 2,
  // Landing target represented by a pre-defined visual shape/feature (ex:
  // X-marker,
  // H-marker, square)
  LANDING_TARGET_TYPE_VISION_OTHER = 3
};

// Direction of VTOL transition
enum class VTOL_TRANSITION_HEADING {
  // Respect the heading configuration of the vehicle.
  VTOL_TRANSITION_HEADING_VEHICLE_DEFAULT = 0,
  // Use the heading pointing towards the next waypoint.
  VTOL_TRANSITION_HEADING_NEXT_WAYPOINT = 1,
  // Use the heading on takeoff (while sitting on the ground).
  VTOL_TRANSITION_HEADING_TAKEOFF = 2,
  // Use the specified heading in parameter 4.
  VTOL_TRANSITION_HEADING_SPECIFIED = 3,
  // Use the current heading when reaching takeoff altitude (potentially facing
  // the wind when weather-vaning is active).
  VTOL_TRANSITION_HEADING_ANY = 4
};

// Camera capability flags (Bitmap)
enum class CAMERA_CAP_FLAGS {
  // Camera is able to record video
  CAMERA_CAP_FLAGS_CAPTURE_VIDEO = 1,
  // Camera is able to capture images
  CAMERA_CAP_FLAGS_CAPTURE_IMAGE = 2,
  // Camera has separate Video and Image/Photo modes (MAV_CMD_SET_CAMERA_MODE)
  CAMERA_CAP_FLAGS_HAS_MODES = 4,
  // Camera can capture images while in video mode
  CAMERA_CAP_FLAGS_CAN_CAPTURE_IMAGE_IN_VIDEO_MODE = 8,
  // Camera can capture videos while in Photo/Image mode
  CAMERA_CAP_FLAGS_CAN_CAPTURE_VIDEO_IN_IMAGE_MODE = 16,
  // Camera has image survey mode (MAV_CMD_SET_CAMERA_MODE)
  CAMERA_CAP_FLAGS_HAS_IMAGE_SURVEY_MODE = 32,
  // Camera has basic zoom control (MAV_CMD_SET_CAMERA_ZOOM)
  CAMERA_CAP_FLAGS_HAS_BASIC_ZOOM = 64,
  // Camera has basic focus control (MAV_CMD_SET_CAMERA_FOCUS)
  CAMERA_CAP_FLAGS_HAS_BASIC_FOCUS = 128,
  // Camera has video streaming capabilities (request VIDEO_STREAM_INFORMATION
  // with
  // MAV_CMD_REQUEST_MESSAGE for video streaming info)
  CAMERA_CAP_FLAGS_HAS_VIDEO_STREAM = 256,
  // Camera supports tracking of a point on the camera view.
  CAMERA_CAP_FLAGS_HAS_TRACKING_POINT = 512,
  // Camera supports tracking of a selection rectangle on the camera view.
  CAMERA_CAP_FLAGS_HAS_TRACKING_RECTANGLE = 1024,
  // Camera supports tracking geo status (CAMERA_TRACKING_GEO_STATUS).
  CAMERA_CAP_FLAGS_HAS_TRACKING_GEO_STATUS = 2048
};

// Stream status flags (Bitmap)
enum class VIDEO_STREAM_STATUS_FLAGS {
  // Stream is active (running)
  VIDEO_STREAM_STATUS_FLAGS_RUNNING = 1,
  // Stream is thermal imaging
  VIDEO_STREAM_STATUS_FLAGS_THERMAL = 2
};

// Video stream types
enum class VIDEO_STREAM_TYPE {
  // Stream is RTSP
  VIDEO_STREAM_TYPE_RTSP = 0,
  // Stream is RTP UDP (URI gives the port number)
  VIDEO_STREAM_TYPE_RTPUDP = 1,
  // Stream is MPEG on TCP
  VIDEO_STREAM_TYPE_TCP_MPEG = 2,
  // Stream is h.264 on MPEG TS (URI gives the port number)
  VIDEO_STREAM_TYPE_MPEG_TS_H264 = 3
};

// Camera tracking status flags
enum class CAMERA_TRACKING_STATUS_FLAGS {
  // Camera is not tracking
  CAMERA_TRACKING_STATUS_FLAGS_IDLE = 0,
  // Camera is tracking
  CAMERA_TRACKING_STATUS_FLAGS_ACTIVE = 1,
  // Camera tracking in error state
  CAMERA_TRACKING_STATUS_FLAGS_ERROR = 2
};

// Camera tracking modes
enum class CAMERA_TRACKING_MODE {
  // Not tracking
  CAMERA_TRACKING_MODE_NONE = 0,
  // Target is a point
  CAMERA_TRACKING_MODE_POINT = 1,
  // Target is a rectangle
  CAMERA_TRACKING_MODE_RECTANGLE = 2
};

// Camera tracking target data (shows where tracked target is within image)
enum class CAMERA_TRACKING_TARGET_DATA {
  // No target data
  CAMERA_TRACKING_TARGET_DATA_NONE = 0,
  // Target data embedded in image data (proprietary)
  CAMERA_TRACKING_TARGET_DATA_EMBEDDED = 1,
  // Target data rendered in image
  CAMERA_TRACKING_TARGET_DATA_RENDERED = 2,
  // Target data within status message (Point or Rectangle)
  CAMERA_TRACKING_TARGET_DATA_IN_STATUS = 4
};

// Zoom types for MAV_CMD_SET_CAMERA_ZOOM
enum class CAMERA_ZOOM_TYPE {
  // Zoom one step increment (-1 for wide, 1 for tele)
  ZOOM_TYPE_STEP = 0,
  // Continuous zoom up/down until stopped (-1 for wide, 1 for tele, 0 to stop
  // zooming)
  ZOOM_TYPE_CONTINUOUS = 1,
  // Zoom value as proportion of full camera range (a value between 0.0 and
  // 100.0)
  ZOOM_TYPE_RANGE = 2,
  // Zoom value/variable focal length in milimetres. Note that there is no
  // message
  // to get the valid zoom range of the camera, so this can type can only be
  // used
  // for cameras where the zoom range is known (implying that this cannot
  // reliably
  // be used in a GCS for an arbitrary camera)
  ZOOM_TYPE_FOCAL_LENGTH = 3
};

// Focus types for MAV_CMD_SET_CAMERA_FOCUS
enum class SET_FOCUS_TYPE {
  // Focus one step increment (-1 for focusing in, 1 for focusing out towards
  // infinity).
  FOCUS_TYPE_STEP = 0,
  // Continuous focus up/down until stopped (-1 for focusing in, 1 for focusing
  // out towards infinity, 0 to stop focusing)
  FOCUS_TYPE_CONTINUOUS = 1,
  // Focus value as proportion of full camera focus range (a value between 0.0
  // and
  // 100.0)
  FOCUS_TYPE_RANGE = 2,
  // Focus value in metres. Note that there is no message to get the valid focus
  // range of the camera, so this can type can only be used for cameras where
  // the
  // range is known (implying that this cannot reliably be used in a GCS for an
  // arbitrary camera).
  FOCUS_TYPE_METERS = 3,
  // Focus automatically.
  FOCUS_TYPE_AUTO = 4,
  // Single auto focus. Mainly used for still pictures. Usually abbreviated as
  // AF-S.
  FOCUS_TYPE_AUTO_SINGLE = 5,
  // Continuous auto focus. Mainly used for dynamic scenes. Abbreviated as AF-C.
  FOCUS_TYPE_AUTO_CONTINUOUS = 6
};

// Result from PARAM_EXT_SET message (or a PARAM_SET within a transaction).
enum class PARAM_ACK {
  // Parameter value ACCEPTED and SET
  PARAM_ACK_ACCEPTED = 0,
  // Parameter value UNKNOWN/UNSUPPORTED
  PARAM_ACK_VALUE_UNSUPPORTED = 1,
  // Parameter failed to set
  PARAM_ACK_FAILED = 2,
  // Parameter value received but not yet set/accepted. A subsequent
  // PARAM_ACK_TRANSACTION
  // or PARAM_EXT_ACK with the final result will follow once operation is
  // completed.
  // This is returned immediately for parameters that take longer to set,
  // indicating
  // taht the the parameter was recieved and does not need to be resent.
  PARAM_ACK_IN_PROGRESS = 3
};

// Camera Modes.
enum class CAMERA_MODE {
  // Camera is in image/photo capture mode.
  CAMERA_MODE_IMAGE = 0,
  // Camera is in video capture mode.
  CAMERA_MODE_VIDEO = 1,
  // Camera is in image survey capture mode. It allows for camera controller to
  // do specific settings for surveys.
  CAMERA_MODE_IMAGE_SURVEY = 2
};

enum class MAV_ARM_AUTH_DENIED_REASON {
  // Not a specific reason
  MAV_ARM_AUTH_DENIED_REASON_GENERIC = 0,
  // Authorizer will send the error as string to GCS
  MAV_ARM_AUTH_DENIED_REASON_NONE = 1,
  // At least one waypoint have a invalid value
  MAV_ARM_AUTH_DENIED_REASON_INVALID_WAYPOINT = 2,
  // Timeout in the authorizer process(in case it depends on network)
  MAV_ARM_AUTH_DENIED_REASON_TIMEOUT = 3,
  // Airspace of the mission in use by another vehicle, second result parameter
  // can have the waypoint id that caused it to be denied.
  MAV_ARM_AUTH_DENIED_REASON_AIRSPACE_IN_USE = 4,
  // Weather is not good to fly
  MAV_ARM_AUTH_DENIED_REASON_BAD_WEATHER = 5
};

// RC type
enum class RC_TYPE {
  // Spektrum DSM2
  RC_TYPE_SPEKTRUM_DSM2 = 0,
  // Spektrum DSMX
  RC_TYPE_SPEKTRUM_DSMX = 1
};

// Bitmap to indicate which dimensions should be ignored by the vehicle: a value
// of 0b0000000000000000 or 0b0000001000000000 indicates that none of the
// setpoint dimensions should be ignored. If bit 9 is set the floats afx afy afz
// should be interpreted as force instead of acceleration.
enum class POSITION_TARGET_TYPEMASK {
  // Ignore position x
  POSITION_TARGET_TYPEMASK_X_IGNORE = 1,
  // Ignore position y
  POSITION_TARGET_TYPEMASK_Y_IGNORE = 2,
  // Ignore position z
  POSITION_TARGET_TYPEMASK_Z_IGNORE = 4,
  // Ignore velocity x
  POSITION_TARGET_TYPEMASK_VX_IGNORE = 8,
  // Ignore velocity y
  POSITION_TARGET_TYPEMASK_VY_IGNORE = 16,
  // Ignore velocity z
  POSITION_TARGET_TYPEMASK_VZ_IGNORE = 32,
  // Ignore acceleration x
  POSITION_TARGET_TYPEMASK_AX_IGNORE = 64,
  // Ignore acceleration y
  POSITION_TARGET_TYPEMASK_AY_IGNORE = 128,
  // Ignore acceleration z
  POSITION_TARGET_TYPEMASK_AZ_IGNORE = 256,
  // Use force instead of acceleration
  POSITION_TARGET_TYPEMASK_FORCE_SET = 512,
  // Ignore yaw
  POSITION_TARGET_TYPEMASK_YAW_IGNORE = 1024,
  // Ignore yaw rate
  POSITION_TARGET_TYPEMASK_YAW_RATE_IGNORE = 2048
};

// Bitmap to indicate which dimensions should be ignored by the vehicle: a value
// of 0b00000000 indicates that none of the setpoint dimensions should be
// ignored.
enum class ATTITUDE_TARGET_TYPEMASK {
  // Ignore body roll rate
  ATTITUDE_TARGET_TYPEMASK_BODY_ROLL_RATE_IGNORE = 1,
  // Ignore body pitch rate
  ATTITUDE_TARGET_TYPEMASK_BODY_PITCH_RATE_IGNORE = 2,
  // Ignore body yaw rate
  ATTITUDE_TARGET_TYPEMASK_BODY_YAW_RATE_IGNORE = 4,
  // Use 3D body thrust setpoint instead of throttle
  ATTITUDE_TARGET_TYPEMASK_THRUST_BODY_SET = 32,
  // Ignore throttle
  ATTITUDE_TARGET_TYPEMASK_THROTTLE_IGNORE = 64,
  // Ignore attitude
  ATTITUDE_TARGET_TYPEMASK_ATTITUDE_IGNORE = 128
};

// Airborne status of UAS.
enum class UTM_FLIGHT_STATE {
  // The flight state can't be determined.
  UTM_FLIGHT_STATE_UNKNOWN = 1,
  // UAS on ground.
  UTM_FLIGHT_STATE_GROUND = 2,
  // UAS airborne.
  UTM_FLIGHT_STATE_AIRBORNE = 3,
  // UAS is in an emergency flight state.
  UTM_FLIGHT_STATE_EMERGENCY = 16,
  // UAS has no active controls.
  UTM_FLIGHT_STATE_NOCTRL = 32
};

// Flags for the global position report.
enum class UTM_DATA_AVAIL_FLAGS {
  // The field time contains valid data.
  UTM_DATA_AVAIL_FLAGS_TIME_VALID = 1,
  // The field uas_id contains valid data.
  UTM_DATA_AVAIL_FLAGS_UAS_ID_AVAILABLE = 2,
  // The fields lat, lon and h_acc contain valid data.
  UTM_DATA_AVAIL_FLAGS_POSITION_AVAILABLE = 4,
  // The fields alt and v_acc contain valid data.
  UTM_DATA_AVAIL_FLAGS_ALTITUDE_AVAILABLE = 8,
  // The field relative_alt contains valid data.
  UTM_DATA_AVAIL_FLAGS_RELATIVE_ALTITUDE_AVAILABLE = 16,
  // The fields vx and vy contain valid data.
  UTM_DATA_AVAIL_FLAGS_HORIZONTAL_VELO_AVAILABLE = 32,
  // The field vz contains valid data.
  UTM_DATA_AVAIL_FLAGS_VERTICAL_VELO_AVAILABLE = 64,
  // The fields next_lat, next_lon and next_alt contain valid data.
  UTM_DATA_AVAIL_FLAGS_NEXT_WAYPOINT_AVAILABLE = 128
};

// These flags encode the cellular network status
enum class CELLULAR_STATUS_FLAG {
  // State unknown or not reportable.
  CELLULAR_STATUS_FLAG_UNKNOWN = 0,
  // Modem is unusable
  CELLULAR_STATUS_FLAG_FAILED = 1,
  // Modem is being initialized
  CELLULAR_STATUS_FLAG_INITIALIZING = 2,
  // Modem is locked
  CELLULAR_STATUS_FLAG_LOCKED = 3,
  // Modem is not enabled and is powered down
  CELLULAR_STATUS_FLAG_DISABLED = 4,
  // Modem is currently transitioning to the CELLULAR_STATUS_FLAG_DISABLED state
  CELLULAR_STATUS_FLAG_DISABLING = 5,
  // Modem is currently transitioning to the CELLULAR_STATUS_FLAG_ENABLED state
  CELLULAR_STATUS_FLAG_ENABLING = 6,
  // Modem is enabled and powered on but not registered with a network provider
  // and not available for data connections
  CELLULAR_STATUS_FLAG_ENABLED = 7,
  // Modem is searching for a network provider to register
  CELLULAR_STATUS_FLAG_SEARCHING = 8,
  // Modem is registered with a network provider, and data connections and
  // messaging
  // may be available for use
  CELLULAR_STATUS_FLAG_REGISTERED = 9,
  // Modem is disconnecting and deactivating the last active packet data bearer.
  // This state will not be entered if more than one packet data bearer is
  // active
  // and one of the active bearers is deactivated
  CELLULAR_STATUS_FLAG_DISCONNECTING = 10,
  // Modem is activating and connecting the first packet data bearer. Subsequent
  // bearer activations when another bearer is already active do not cause this
  // state to be entered
  CELLULAR_STATUS_FLAG_CONNECTING = 11,
  // One or more packet data bearers is active and connected
  CELLULAR_STATUS_FLAG_CONNECTED = 12
};

// These flags are used to diagnose the failure state of CELLULAR_STATUS
enum class CELLULAR_NETWORK_FAILED_REASON {
  // No error
  CELLULAR_NETWORK_FAILED_REASON_NONE = 0,
  // Error state is unknown
  CELLULAR_NETWORK_FAILED_REASON_UNKNOWN = 1,
  // SIM is required for the modem but missing
  CELLULAR_NETWORK_FAILED_REASON_SIM_MISSING = 2,
  // SIM is available, but not usuable for connection
  CELLULAR_NETWORK_FAILED_REASON_SIM_ERROR = 3
};

// Cellular network radio type
enum class CELLULAR_NETWORK_RADIO_TYPE {
  CELLULAR_NETWORK_RADIO_TYPE_NONE = 0,
  CELLULAR_NETWORK_RADIO_TYPE_GSM = 1,
  CELLULAR_NETWORK_RADIO_TYPE_CDMA = 2,
  CELLULAR_NETWORK_RADIO_TYPE_WCDMA = 3,
  CELLULAR_NETWORK_RADIO_TYPE_LTE = 4
};

// Precision land modes (used in MAV_CMD_NAV_LAND).
enum class PRECISION_LAND_MODE {
  // Normal (non-precision) landing.
  PRECISION_LAND_MODE_DISABLED = 0,
  // Use precision landing if beacon detected when land command accepted,
  // otherwise
  // land normally.
  PRECISION_LAND_MODE_OPPORTUNISTIC = 1,
  // Use precision landing, searching for beacon if not found when land command
  // accepted (land normally if beacon cannot be found).
  PRECISION_LAND_MODE_REQUIRED = 2
};

// Parachute actions. Trigger release and enable/disable auto-release.
enum class PARACHUTE_ACTION {
  // Disable auto-release of parachute (i.e. release triggered by crash
  // detectors).
  PARACHUTE_DISABLE = 0,
  // Enable auto-release of parachute.
  PARACHUTE_ENABLE = 1,
  // Release parachute and kill motors.
  PARACHUTE_RELEASE = 2
};

enum class MAV_TUNNEL_PAYLOAD_TYPE {
  // Encoding of payload unknown.
  MAV_TUNNEL_PAYLOAD_TYPE_UNKNOWN = 0,
  // Registered for STorM32 gimbal controller.
  MAV_TUNNEL_PAYLOAD_TYPE_STORM32_RESERVED0 = 200,
  // Registered for STorM32 gimbal controller.
  MAV_TUNNEL_PAYLOAD_TYPE_STORM32_RESERVED1 = 201,
  // Registered for STorM32 gimbal controller.
  MAV_TUNNEL_PAYLOAD_TYPE_STORM32_RESERVED2 = 202,
  // Registered for STorM32 gimbal controller.
  MAV_TUNNEL_PAYLOAD_TYPE_STORM32_RESERVED3 = 203,
  // Registered for STorM32 gimbal controller.
  MAV_TUNNEL_PAYLOAD_TYPE_STORM32_RESERVED4 = 204,
  // Registered for STorM32 gimbal controller.
  MAV_TUNNEL_PAYLOAD_TYPE_STORM32_RESERVED5 = 205,
  // Registered for STorM32 gimbal controller.
  MAV_TUNNEL_PAYLOAD_TYPE_STORM32_RESERVED6 = 206,
  // Registered for STorM32 gimbal controller.
  MAV_TUNNEL_PAYLOAD_TYPE_STORM32_RESERVED7 = 207,
  // Registered for STorM32 gimbal controller.
  MAV_TUNNEL_PAYLOAD_TYPE_STORM32_RESERVED8 = 208,
  // Registered for STorM32 gimbal controller.
  MAV_TUNNEL_PAYLOAD_TYPE_STORM32_RESERVED9 = 209
};

enum class MAV_ODID_ID_TYPE {
  // No type defined.
  MAV_ODID_ID_TYPE_NONE = 0,
  // Manufacturer Serial Number (ANSI/CTA-2063 format).
  MAV_ODID_ID_TYPE_SERIAL_NUMBER = 1,
  // CAA (Civil Aviation Authority) registered ID. Format: [ICAO Country
  // Code].[CAA
  // Assigned ID].
  MAV_ODID_ID_TYPE_CAA_REGISTRATION_ID = 2,
  // UTM (Unmanned Traffic Management) assigned UUID (RFC4122).
  MAV_ODID_ID_TYPE_UTM_ASSIGNED_UUID = 3,
  // A 20 byte ID for a specific flight/session. The exact ID type is indicated
  // by the first byte of uas_id and these type values are managed by ICAO.
  MAV_ODID_ID_TYPE_SPECIFIC_SESSION_ID = 4
};

enum class MAV_ODID_UA_TYPE {
  // No UA (Unmanned Aircraft) type defined.
  MAV_ODID_UA_TYPE_NONE = 0,
  // Aeroplane/Airplane. Fixed wing.
  MAV_ODID_UA_TYPE_AEROPLANE = 1,
  // Helicopter or multirotor.
  MAV_ODID_UA_TYPE_HELICOPTER_OR_MULTIROTOR = 2,
  // Gyroplane.
  MAV_ODID_UA_TYPE_GYROPLANE = 3,
  // VTOL (Vertical Take-Off and Landing). Fixed wing aircraft that can take off
  // vertically.
  MAV_ODID_UA_TYPE_HYBRID_LIFT = 4,
  // Ornithopter.
  MAV_ODID_UA_TYPE_ORNITHOPTER = 5,
  // Glider.
  MAV_ODID_UA_TYPE_GLIDER = 6,
  // Kite.
  MAV_ODID_UA_TYPE_KITE = 7,
  // Free Balloon.
  MAV_ODID_UA_TYPE_FREE_BALLOON = 8,
  // Captive Balloon.
  MAV_ODID_UA_TYPE_CAPTIVE_BALLOON = 9,
  // Airship. E.g. a blimp.
  MAV_ODID_UA_TYPE_AIRSHIP = 10,
  // Free Fall/Parachute (unpowered).
  MAV_ODID_UA_TYPE_FREE_FALL_PARACHUTE = 11,
  // Rocket.
  MAV_ODID_UA_TYPE_ROCKET = 12,
  // Tethered powered aircraft.
  MAV_ODID_UA_TYPE_TETHERED_POWERED_AIRCRAFT = 13,
  // Ground Obstacle.
  MAV_ODID_UA_TYPE_GROUND_OBSTACLE = 14,
  // Other type of aircraft not listed earlier.
  MAV_ODID_UA_TYPE_OTHER = 15
};

enum class MAV_ODID_STATUS {
  // The status of the (UA) Unmanned Aircraft is undefined.
  MAV_ODID_STATUS_UNDECLARED = 0,
  // The UA is on the ground.
  MAV_ODID_STATUS_GROUND = 1,
  // The UA is in the air.
  MAV_ODID_STATUS_AIRBORNE = 2,
  // The UA is having an emergency.
  MAV_ODID_STATUS_EMERGENCY = 3,
  // The remote ID system is failing or unreliable in some way.
  MAV_ODID_STATUS_REMOTE_ID_SYSTEM_FAILURE = 4
};

enum class MAV_ODID_HEIGHT_REF {
  // The height field is relative to the take-off location.
  MAV_ODID_HEIGHT_REF_OVER_TAKEOFF = 0,
  // The height field is relative to ground.
  MAV_ODID_HEIGHT_REF_OVER_GROUND = 1
};

enum class MAV_ODID_HOR_ACC {
  // The horizontal accuracy is unknown.
  MAV_ODID_HOR_ACC_UNKNOWN = 0,
  // The horizontal accuracy is smaller than 10 Nautical Miles. 18.52 km.
  MAV_ODID_HOR_ACC_10NM = 1,
  // The horizontal accuracy is smaller than 4 Nautical Miles. 7.408 km.
  MAV_ODID_HOR_ACC_4NM = 2,
  // The horizontal accuracy is smaller than 2 Nautical Miles. 3.704 km.
  MAV_ODID_HOR_ACC_2NM = 3,
  // The horizontal accuracy is smaller than 1 Nautical Miles. 1.852 km.
  MAV_ODID_HOR_ACC_1NM = 4,
  // The horizontal accuracy is smaller than 0.5 Nautical Miles. 926 m.
  MAV_ODID_HOR_ACC_0_5NM = 5,
  // The horizontal accuracy is smaller than 0.3 Nautical Miles. 555.6 m.
  MAV_ODID_HOR_ACC_0_3NM = 6,
  // The horizontal accuracy is smaller than 0.1 Nautical Miles. 185.2 m.
  MAV_ODID_HOR_ACC_0_1NM = 7,
  // The horizontal accuracy is smaller than 0.05 Nautical Miles. 92.6 m.
  MAV_ODID_HOR_ACC_0_05NM = 8,
  // The horizontal accuracy is smaller than 30 meter.
  MAV_ODID_HOR_ACC_30_METER = 9,
  // The horizontal accuracy is smaller than 10 meter.
  MAV_ODID_HOR_ACC_10_METER = 10,
  // The horizontal accuracy is smaller than 3 meter.
  MAV_ODID_HOR_ACC_3_METER = 11,
  // The horizontal accuracy is smaller than 1 meter.
  MAV_ODID_HOR_ACC_1_METER = 12
};

enum class MAV_ODID_VER_ACC {
  // The vertical accuracy is unknown.
  MAV_ODID_VER_ACC_UNKNOWN = 0,
  // The vertical accuracy is smaller than 150 meter.
  MAV_ODID_VER_ACC_150_METER = 1,
  // The vertical accuracy is smaller than 45 meter.
  MAV_ODID_VER_ACC_45_METER = 2,
  // The vertical accuracy is smaller than 25 meter.
  MAV_ODID_VER_ACC_25_METER = 3,
  // The vertical accuracy is smaller than 10 meter.
  MAV_ODID_VER_ACC_10_METER = 4,
  // The vertical accuracy is smaller than 3 meter.
  MAV_ODID_VER_ACC_3_METER = 5,
  // The vertical accuracy is smaller than 1 meter.
  MAV_ODID_VER_ACC_1_METER = 6
};

enum class MAV_ODID_SPEED_ACC {
  // The speed accuracy is unknown.
  MAV_ODID_SPEED_ACC_UNKNOWN = 0,
  // The speed accuracy is smaller than 10 meters per second.
  MAV_ODID_SPEED_ACC_10_METERS_PER_SECOND = 1,
  // The speed accuracy is smaller than 3 meters per second.
  MAV_ODID_SPEED_ACC_3_METERS_PER_SECOND = 2,
  // The speed accuracy is smaller than 1 meters per second.
  MAV_ODID_SPEED_ACC_1_METERS_PER_SECOND = 3,
  // The speed accuracy is smaller than 0.3 meters per second.
  MAV_ODID_SPEED_ACC_0_3_METERS_PER_SECOND = 4
};

enum class MAV_ODID_TIME_ACC {
  // The timestamp accuracy is unknown.
  MAV_ODID_TIME_ACC_UNKNOWN = 0,
  // The timestamp accuracy is smaller than or equal to 0.1 second.
  MAV_ODID_TIME_ACC_0_1_SECOND = 1,
  // The timestamp accuracy is smaller than or equal to 0.2 second.
  MAV_ODID_TIME_ACC_0_2_SECOND = 2,
  // The timestamp accuracy is smaller than or equal to 0.3 second.
  MAV_ODID_TIME_ACC_0_3_SECOND = 3,
  // The timestamp accuracy is smaller than or equal to 0.4 second.
  MAV_ODID_TIME_ACC_0_4_SECOND = 4,
  // The timestamp accuracy is smaller than or equal to 0.5 second.
  MAV_ODID_TIME_ACC_0_5_SECOND = 5,
  // The timestamp accuracy is smaller than or equal to 0.6 second.
  MAV_ODID_TIME_ACC_0_6_SECOND = 6,
  // The timestamp accuracy is smaller than or equal to 0.7 second.
  MAV_ODID_TIME_ACC_0_7_SECOND = 7,
  // The timestamp accuracy is smaller than or equal to 0.8 second.
  MAV_ODID_TIME_ACC_0_8_SECOND = 8,
  // The timestamp accuracy is smaller than or equal to 0.9 second.
  MAV_ODID_TIME_ACC_0_9_SECOND = 9,
  // The timestamp accuracy is smaller than or equal to 1.0 second.
  MAV_ODID_TIME_ACC_1_0_SECOND = 10,
  // The timestamp accuracy is smaller than or equal to 1.1 second.
  MAV_ODID_TIME_ACC_1_1_SECOND = 11,
  // The timestamp accuracy is smaller than or equal to 1.2 second.
  MAV_ODID_TIME_ACC_1_2_SECOND = 12,
  // The timestamp accuracy is smaller than or equal to 1.3 second.
  MAV_ODID_TIME_ACC_1_3_SECOND = 13,
  // The timestamp accuracy is smaller than or equal to 1.4 second.
  MAV_ODID_TIME_ACC_1_4_SECOND = 14,
  // The timestamp accuracy is smaller than or equal to 1.5 second.
  MAV_ODID_TIME_ACC_1_5_SECOND = 15
};

enum class MAV_ODID_AUTH_TYPE {
  // No authentication type is specified.
  MAV_ODID_AUTH_TYPE_NONE = 0,
  // Signature for the UAS (Unmanned Aircraft System) ID.
  MAV_ODID_AUTH_TYPE_UAS_ID_SIGNATURE = 1,
  // Signature for the Operator ID.
  MAV_ODID_AUTH_TYPE_OPERATOR_ID_SIGNATURE = 2,
  // Signature for the entire message set.
  MAV_ODID_AUTH_TYPE_MESSAGE_SET_SIGNATURE = 3,
  // Authentication is provided by Network Remote ID.
  MAV_ODID_AUTH_TYPE_NETWORK_REMOTE_ID = 4,
  // The exact authentication type is indicated by the first byte of
  // authentication_data
  // and these type values are managed by ICAO.
  MAV_ODID_AUTH_TYPE_SPECIFIC_AUTHENTICATION = 5
};

enum class MAV_ODID_DESC_TYPE {
  // Optional free-form text description of the purpose of the flight.
  MAV_ODID_DESC_TYPE_TEXT = 0,
  // Optional additional clarification when status == MAV_ODID_STATUS_EMERGENCY.
  MAV_ODID_DESC_TYPE_EMERGENCY = 1,
  // Optional additional clarification when status != MAV_ODID_STATUS_EMERGENCY.
  MAV_ODID_DESC_TYPE_EXTENDED_STATUS = 2
};

enum class MAV_ODID_OPERATOR_LOCATION_TYPE {
  // The location/altitude of the operator is the same as the take-off location.
  MAV_ODID_OPERATOR_LOCATION_TYPE_TAKEOFF = 0,
  // The location/altitude of the operator is dynamic. E.g. based on live GNSS
  // data.
  MAV_ODID_OPERATOR_LOCATION_TYPE_LIVE_GNSS = 1,
  // The location/altitude of the operator are fixed values.
  MAV_ODID_OPERATOR_LOCATION_TYPE_FIXED = 2
};

enum class MAV_ODID_CLASSIFICATION_TYPE {
  // The classification type for the UA is undeclared.
  MAV_ODID_CLASSIFICATION_TYPE_UNDECLARED = 0,
  // The classification type for the UA follows EU (European Union)
  // specifications.
  MAV_ODID_CLASSIFICATION_TYPE_EU = 1
};

enum class MAV_ODID_CATEGORY_EU {
  // The category for the UA, according to the EU specification, is undeclared.
  MAV_ODID_CATEGORY_EU_UNDECLARED = 0,
  // The category for the UA, according to the EU specification, is the Open
  // category.
  MAV_ODID_CATEGORY_EU_OPEN = 1,
  // The category for the UA, according to the EU specification, is the Specific
  // category.
  MAV_ODID_CATEGORY_EU_SPECIFIC = 2,
  // The category for the UA, according to the EU specification, is the
  // Certified
  // category.
  MAV_ODID_CATEGORY_EU_CERTIFIED = 3
};

enum class MAV_ODID_CLASS_EU {
  // The class for the UA, according to the EU specification, is undeclared.
  MAV_ODID_CLASS_EU_UNDECLARED = 0,
  // The class for the UA, according to the EU specification, is Class 0.
  MAV_ODID_CLASS_EU_CLASS_0 = 1,
  // The class for the UA, according to the EU specification, is Class 1.
  MAV_ODID_CLASS_EU_CLASS_1 = 2,
  // The class for the UA, according to the EU specification, is Class 2.
  MAV_ODID_CLASS_EU_CLASS_2 = 3,
  // The class for the UA, according to the EU specification, is Class 3.
  MAV_ODID_CLASS_EU_CLASS_3 = 4,
  // The class for the UA, according to the EU specification, is Class 4.
  MAV_ODID_CLASS_EU_CLASS_4 = 5,
  // The class for the UA, according to the EU specification, is Class 5.
  MAV_ODID_CLASS_EU_CLASS_5 = 6,
  // The class for the UA, according to the EU specification, is Class 6.
  MAV_ODID_CLASS_EU_CLASS_6 = 7
};

enum class MAV_ODID_OPERATOR_ID_TYPE {
  // CAA (Civil Aviation Authority) registered operator ID.
  MAV_ODID_OPERATOR_ID_TYPE_CAA = 0
};

// Tune formats (used for vehicle buzzer/tone generation).
enum class TUNE_FORMAT {
  // Format is QBasic 1.1 Play:
  // https://www.qbasic.net/en/reference/qb11/Statement/PLAY-006.htm.
  TUNE_FORMAT_QBASIC1_1 = 1,
  // Format is Modern Music Markup Language (MML):
  // https://en.wikipedia.org/wiki/Music_Macro_Language#Modern_MML.
  TUNE_FORMAT_MML_MODERN = 2
};

// Type of AIS vessel, enum duplicated from AIS standard,
// https://gpsd.gitlab.io/gpsd/AIVDM.html
enum class AIS_TYPE {
  // Not available (default).
  AIS_TYPE_UNKNOWN = 0,
  AIS_TYPE_RESERVED_1 = 1,
  AIS_TYPE_RESERVED_2 = 2,
  AIS_TYPE_RESERVED_3 = 3,
  AIS_TYPE_RESERVED_4 = 4,
  AIS_TYPE_RESERVED_5 = 5,
  AIS_TYPE_RESERVED_6 = 6,
  AIS_TYPE_RESERVED_7 = 7,
  AIS_TYPE_RESERVED_8 = 8,
  AIS_TYPE_RESERVED_9 = 9,
  AIS_TYPE_RESERVED_10 = 10,
  AIS_TYPE_RESERVED_11 = 11,
  AIS_TYPE_RESERVED_12 = 12,
  AIS_TYPE_RESERVED_13 = 13,
  AIS_TYPE_RESERVED_14 = 14,
  AIS_TYPE_RESERVED_15 = 15,
  AIS_TYPE_RESERVED_16 = 16,
  AIS_TYPE_RESERVED_17 = 17,
  AIS_TYPE_RESERVED_18 = 18,
  AIS_TYPE_RESERVED_19 = 19,
  // Wing In Ground effect.
  AIS_TYPE_WIG = 20,
  AIS_TYPE_WIG_HAZARDOUS_A = 21,
  AIS_TYPE_WIG_HAZARDOUS_B = 22,
  AIS_TYPE_WIG_HAZARDOUS_C = 23,
  AIS_TYPE_WIG_HAZARDOUS_D = 24,
  AIS_TYPE_WIG_RESERVED_1 = 25,
  AIS_TYPE_WIG_RESERVED_2 = 26,
  AIS_TYPE_WIG_RESERVED_3 = 27,
  AIS_TYPE_WIG_RESERVED_4 = 28,
  AIS_TYPE_WIG_RESERVED_5 = 29,
  AIS_TYPE_FISHING = 30,
  AIS_TYPE_TOWING = 31,
  // Towing: length exceeds 200m or breadth exceeds 25m.
  AIS_TYPE_TOWING_LARGE = 32,
  // Dredging or other underwater ops.
  AIS_TYPE_DREDGING = 33,
  AIS_TYPE_DIVING = 34,
  AIS_TYPE_MILITARY = 35,
  AIS_TYPE_SAILING = 36,
  AIS_TYPE_PLEASURE = 37,
  AIS_TYPE_RESERVED_20 = 38,
  AIS_TYPE_RESERVED_21 = 39,
  // High Speed Craft.
  AIS_TYPE_HSC = 40,
  AIS_TYPE_HSC_HAZARDOUS_A = 41,
  AIS_TYPE_HSC_HAZARDOUS_B = 42,
  AIS_TYPE_HSC_HAZARDOUS_C = 43,
  AIS_TYPE_HSC_HAZARDOUS_D = 44,
  AIS_TYPE_HSC_RESERVED_1 = 45,
  AIS_TYPE_HSC_RESERVED_2 = 46,
  AIS_TYPE_HSC_RESERVED_3 = 47,
  AIS_TYPE_HSC_RESERVED_4 = 48,
  AIS_TYPE_HSC_UNKNOWN = 49,
  AIS_TYPE_PILOT = 50,
  // Search And Rescue vessel.
  AIS_TYPE_SAR = 51,
  AIS_TYPE_TUG = 52,
  AIS_TYPE_PORT_TENDER = 53,
  // Anti-pollution equipment.
  AIS_TYPE_ANTI_POLLUTION = 54,
  AIS_TYPE_LAW_ENFORCEMENT = 55,
  AIS_TYPE_SPARE_LOCAL_1 = 56,
  AIS_TYPE_SPARE_LOCAL_2 = 57,
  AIS_TYPE_MEDICAL_TRANSPORT = 58,
  // Noncombatant ship according to RR Resolution No. 18.
  AIS_TYPE_NONECOMBATANT = 59,
  AIS_TYPE_PASSENGER = 60,
  AIS_TYPE_PASSENGER_HAZARDOUS_A = 61,
  AIS_TYPE_PASSENGER_HAZARDOUS_B = 62,
  AIS_TYPE_AIS_TYPE_PASSENGER_HAZARDOUS_C = 63,
  AIS_TYPE_PASSENGER_HAZARDOUS_D = 64,
  AIS_TYPE_PASSENGER_RESERVED_1 = 65,
  AIS_TYPE_PASSENGER_RESERVED_2 = 66,
  AIS_TYPE_PASSENGER_RESERVED_3 = 67,
  AIS_TYPE_AIS_TYPE_PASSENGER_RESERVED_4 = 68,
  AIS_TYPE_PASSENGER_UNKNOWN = 69,
  AIS_TYPE_CARGO = 70,
  AIS_TYPE_CARGO_HAZARDOUS_A = 71,
  AIS_TYPE_CARGO_HAZARDOUS_B = 72,
  AIS_TYPE_CARGO_HAZARDOUS_C = 73,
  AIS_TYPE_CARGO_HAZARDOUS_D = 74,
  AIS_TYPE_CARGO_RESERVED_1 = 75,
  AIS_TYPE_CARGO_RESERVED_2 = 76,
  AIS_TYPE_CARGO_RESERVED_3 = 77,
  AIS_TYPE_CARGO_RESERVED_4 = 78,
  AIS_TYPE_CARGO_UNKNOWN = 79,
  AIS_TYPE_TANKER = 80,
  AIS_TYPE_TANKER_HAZARDOUS_A = 81,
  AIS_TYPE_TANKER_HAZARDOUS_B = 82,
  AIS_TYPE_TANKER_HAZARDOUS_C = 83,
  AIS_TYPE_TANKER_HAZARDOUS_D = 84,
  AIS_TYPE_TANKER_RESERVED_1 = 85,
  AIS_TYPE_TANKER_RESERVED_2 = 86,
  AIS_TYPE_TANKER_RESERVED_3 = 87,
  AIS_TYPE_TANKER_RESERVED_4 = 88,
  AIS_TYPE_TANKER_UNKNOWN = 89,
  AIS_TYPE_OTHER = 90,
  AIS_TYPE_OTHER_HAZARDOUS_A = 91,
  AIS_TYPE_OTHER_HAZARDOUS_B = 92,
  AIS_TYPE_OTHER_HAZARDOUS_C = 93,
  AIS_TYPE_OTHER_HAZARDOUS_D = 94,
  AIS_TYPE_OTHER_RESERVED_1 = 95,
  AIS_TYPE_OTHER_RESERVED_2 = 96,
  AIS_TYPE_OTHER_RESERVED_3 = 97,
  AIS_TYPE_OTHER_RESERVED_4 = 98,
  AIS_TYPE_OTHER_UNKNOWN = 99
};

// Navigational status of AIS vessel, enum duplicated from AIS standard,
// https://gpsd.gitlab.io/gpsd/AIVDM.html
enum class AIS_NAV_STATUS {
  // Under way using engine.
  UNDER_WAY = 0,
  AIS_NAV_ANCHORED = 1,
  AIS_NAV_UN_COMMANDED = 2,
  AIS_NAV_RESTRICTED_MANOEUVERABILITY = 3,
  AIS_NAV_DRAUGHT_CONSTRAINED = 4,
  AIS_NAV_MOORED = 5,
  AIS_NAV_AGROUND = 6,
  AIS_NAV_FISHING = 7,
  AIS_NAV_SAILING = 8,
  AIS_NAV_RESERVED_HSC = 9,
  AIS_NAV_RESERVED_WIG = 10,
  AIS_NAV_RESERVED_1 = 11,
  AIS_NAV_RESERVED_2 = 12,
  AIS_NAV_RESERVED_3 = 13,
  // Search And Rescue Transponder.
  AIS_NAV_AIS_SART = 14,
  // Not available (default).
  AIS_NAV_UNKNOWN = 15
};

// These flags are used in the AIS_VESSEL.fields bitmask to indicate validity of
// data in the other message fields. When set, the data is valid.
enum class AIS_FLAGS {
  // 1 = Position accuracy less than 10m, 0 = position accuracy greater than
  // 10m.
  AIS_FLAGS_POSITION_ACCURACY = 1,
  AIS_FLAGS_VALID_COG = 2,
  AIS_FLAGS_VALID_VELOCITY = 4,
  // 1 = Velocity over 52.5765m/s (102.2 knots)
  AIS_FLAGS_HIGH_VELOCITY = 8,
  AIS_FLAGS_VALID_TURN_RATE = 16,
  // Only the sign of the returned turn rate value is valid, either greater than
  // 5deg/30s or less than -5deg/30s
  AIS_FLAGS_TURN_RATE_SIGN_ONLY = 32,
  AIS_FLAGS_VALID_DIMENSIONS = 64,
  // Distance to bow is larger than 511m
  AIS_FLAGS_LARGE_BOW_DIMENSION = 128,
  // Distance to stern is larger than 511m
  AIS_FLAGS_LARGE_STERN_DIMENSION = 256,
  // Distance to port side is larger than 63m
  AIS_FLAGS_LARGE_PORT_DIMENSION = 512,
  // Distance to starboard side is larger than 63m
  AIS_FLAGS_LARGE_STARBOARD_DIMENSION = 1024,
  AIS_FLAGS_VALID_CALLSIGN = 2048,
  AIS_FLAGS_VALID_NAME = 4096
};

// List of possible units where failures can be injected.
enum class FAILURE_UNIT {
  FAILURE_UNIT_SENSOR_GYRO = 0,
  FAILURE_UNIT_SENSOR_ACCEL = 1,
  FAILURE_UNIT_SENSOR_MAG = 2,
  FAILURE_UNIT_SENSOR_BARO = 3,
  FAILURE_UNIT_SENSOR_GPS = 4,
  FAILURE_UNIT_SENSOR_OPTICAL_FLOW = 5,
  FAILURE_UNIT_SENSOR_VIO = 6,
  FAILURE_UNIT_SENSOR_DISTANCE_SENSOR = 7,
  FAILURE_UNIT_SENSOR_AIRSPEED = 8,
  FAILURE_UNIT_SYSTEM_BATTERY = 100,
  FAILURE_UNIT_SYSTEM_MOTOR = 101,
  FAILURE_UNIT_SYSTEM_SERVO = 102,
  FAILURE_UNIT_SYSTEM_AVOIDANCE = 103,
  FAILURE_UNIT_SYSTEM_RC_SIGNAL = 104,
  FAILURE_UNIT_SYSTEM_MAVLINK_SIGNAL = 105
};

// List of possible failure type to inject.
enum class FAILURE_TYPE {
  // No failure injected, used to reset a previous failure.
  FAILURE_TYPE_OK = 0,
  // Sets unit off, so completely non-responsive.
  FAILURE_TYPE_OFF = 1,
  // Unit is stuck e.g. keeps reporting the same value.
  FAILURE_TYPE_STUCK = 2,
  // Unit is reporting complete garbage.
  FAILURE_TYPE_GARBAGE = 3,
  // Unit is consistently wrong.
  FAILURE_TYPE_WRONG = 4,
  // Unit is slow, so e.g. reporting at slower than expected rate.
  FAILURE_TYPE_SLOW = 5,
  // Data of unit is delayed in time.
  FAILURE_TYPE_DELAYED = 6,
  // Unit is sometimes working, sometimes not.
  FAILURE_TYPE_INTERMITTENT = 7
};

enum class NAV_VTOL_LAND_OPTIONS {
  // Default autopilot landing behaviour.
  NAV_VTOL_LAND_OPTIONS_DEFAULT = 0,
  // Descend in fixed wing mode, transitioning to multicopter mode for vertical
  // landing when close to the ground. The fixed wing descent pattern is at the
  // discretion of the vehicle (e.g. transition altitude, loiter direction,
  // radius,
  // and speed, etc.).
  NAV_VTOL_LAND_OPTIONS_FW_DESCENT = 1,
  // Land in multicopter mode on reaching the landing co-ordinates (the whole
  // landing
  // is by "hover descent").
  NAV_VTOL_LAND_OPTIONS_HOVER_DESCENT = 2
};

// Winch status flags used in WINCH_STATUS
enum class MAV_WINCH_STATUS_FLAG {
  // Winch is healthy
  MAV_WINCH_STATUS_HEALTHY = 1,
  // Winch line is fully retracted
  MAV_WINCH_STATUS_FULLY_RETRACTED = 2,
  // Winch motor is moving
  MAV_WINCH_STATUS_MOVING = 4,
  // Winch clutch is engaged allowing motor to move freely.
  MAV_WINCH_STATUS_CLUTCH_ENGAGED = 8,
  // Winch is locked by locking mechanism.
  MAV_WINCH_STATUS_LOCKED = 16,
  // Winch is gravity dropping payload.
  MAV_WINCH_STATUS_DROPPING = 32,
  // Winch is arresting payload descent.
  MAV_WINCH_STATUS_ARRESTING = 64,
  // Winch is using torque measurements to sense the ground.
  MAV_WINCH_STATUS_GROUND_SENSE = 128,
  // Winch is returning to the fully retracted position.
  MAV_WINCH_STATUS_RETRACTING = 256,
  // Winch is redelivering the payload. This is a failover state if the line
  // tension
  // goes above a threshold during RETRACTING.
  MAV_WINCH_STATUS_REDELIVER = 512,
  // Winch is abandoning the line and possibly payload. Winch unspools the
  // entire
  // calculated line length. This is a failover state from REDELIVER if the
  // number
  // of attemps exceeds a threshold.
  MAV_WINCH_STATUS_ABANDON_LINE = 1024
};

enum class MAG_CAL_STATUS {
  MAG_CAL_NOT_STARTED = 0,
  MAG_CAL_WAITING_TO_START = 1,
  MAG_CAL_RUNNING_STEP_ONE = 2,
  MAG_CAL_RUNNING_STEP_TWO = 3,
  MAG_CAL_SUCCESS = 4,
  MAG_CAL_FAILED = 5,
  MAG_CAL_BAD_ORIENTATION = 6,
  MAG_CAL_BAD_RADIUS = 7
};

// Reason for an event error response.
enum class MAV_EVENT_ERROR_REASON {
  // The requested event is not available (anymore).
  MAV_EVENT_ERROR_REASON_UNAVAILABLE = 0
};

// Flags for CURRENT_EVENT_SEQUENCE.
enum class MAV_EVENT_CURRENT_SEQUENCE_FLAGS {
  // A sequence reset has happened (e.g. vehicle reboot).
  MAV_EVENT_CURRENT_SEQUENCE_FLAGS_RESET = 1
};

// Flags in the HIL_SENSOR message indicate which fields have updated since the
// last message
enum class HIL_SENSOR_UPDATED_FLAGS {
  // None of the fields in HIL_SENSOR have been updated
  HIL_SENSOR_UPDATED_NONE = 0,
  // The value in the xacc field has been updated
  HIL_SENSOR_UPDATED_XACC = 1,
  // The value in the yacc field has been updated
  HIL_SENSOR_UPDATED_YACC = 2,
  // The value in the zacc field has been updated
  HIL_SENSOR_UPDATED_ZACC = 4,
  // The value in the xgyro field has been updated
  HIL_SENSOR_UPDATED_XGYRO = 8,
  // The value in the ygyro field has been updated
  HIL_SENSOR_UPDATED_YGYRO = 16,
  // The value in the zgyro field has been updated
  HIL_SENSOR_UPDATED_ZGYRO = 32,
  // The value in the xmag field has been updated
  HIL_SENSOR_UPDATED_XMAG = 64,
  // The value in the ymag field has been updated
  HIL_SENSOR_UPDATED_YMAG = 128,
  // The value in the zmag field has been updated
  HIL_SENSOR_UPDATED_ZMAG = 256,
  // The value in the abs_pressure field has been updated
  HIL_SENSOR_UPDATED_ABS_PRESSURE = 512,
  // The value in the diff_pressure field has been updated
  HIL_SENSOR_UPDATED_DIFF_PRESSURE = 1024,
  // The value in the pressure_alt field has been updated
  HIL_SENSOR_UPDATED_PRESSURE_ALT = 2048,
  // The value in the temperature field has been updated
  HIL_SENSOR_UPDATED_TEMPERATURE = 4096,
  // Full reset of attitude/position/velocities/etc was performed in sim (Bit
  // 31).
  // HIL_SENSOR_UPDATED_RESET = 2147483648
};

// Flags in the HIGHRES_IMU message indicate which fields have updated since the
// last message
enum class HIGHRES_IMU_UPDATED_FLAGS {
  // None of the fields in HIGHRES_IMU have been updated
  HIGHRES_IMU_UPDATED_NONE = 0,
  // The value in the xacc field has been updated
  HIGHRES_IMU_UPDATED_XACC = 1,
  // The value in the yacc field has been updated
  HIGHRES_IMU_UPDATED_YACC = 2,
  // The value in the zacc field has been updated since
  HIGHRES_IMU_UPDATED_ZACC = 4,
  // The value in the xgyro field has been updated
  HIGHRES_IMU_UPDATED_XGYRO = 8,
  // The value in the ygyro field has been updated
  HIGHRES_IMU_UPDATED_YGYRO = 16,
  // The value in the zgyro field has been updated
  HIGHRES_IMU_UPDATED_ZGYRO = 32,
  // The value in the xmag field has been updated
  HIGHRES_IMU_UPDATED_XMAG = 64,
  // The value in the ymag field has been updated
  HIGHRES_IMU_UPDATED_YMAG = 128,
  // The value in the zmag field has been updated
  HIGHRES_IMU_UPDATED_ZMAG = 256,
  // The value in the abs_pressure field has been updated
  HIGHRES_IMU_UPDATED_ABS_PRESSURE = 512,
  // The value in the diff_pressure field has been updated
  HIGHRES_IMU_UPDATED_DIFF_PRESSURE = 1024,
  // The value in the pressure_alt field has been updated
  HIGHRES_IMU_UPDATED_PRESSURE_ALT = 2048,
  // The value in the temperature field has been updated
  HIGHRES_IMU_UPDATED_TEMPERATURE = 4096,
  // All fields in HIGHRES_IMU have been updated.
  HIGHRES_IMU_UPDATED_ALL = 65535
};

enum class CAN_FILTER_OP {
  CAN_FILTER_REPLACE = 0,
  CAN_FILTER_ADD = 1,
  CAN_FILTER_REMOVE = 2
};

// MAV FTP error codes (https://mavlink.io/en/services/ftp.html)
enum class MAV_FTP_ERR {
  // None: No error
  MAV_FTP_ERR_NONE = 0,
  // Fail: Unknown failure
  MAV_FTP_ERR_FAIL = 1,
  // FailErrno: Command failed, Err number sent back in PayloadHeader.data[1].
  // This
  // is a file-system error number understood by the server operating system.
  MAV_FTP_ERR_FAILERRNO = 2,
  // InvalidDataSize: Payload size is invalid
  MAV_FTP_ERR_INVALIDDATASIZE = 3,
  // InvalidSession: Session is not currently open
  MAV_FTP_ERR_INVALIDSESSION = 4,
  // NoSessionsAvailable: All available sessions are already in use
  MAV_FTP_ERR_NOSESSIONSAVAILABLE = 5,
  // EOF: Offset past end of file for ListDirectory and ReadFile commands
  MAV_FTP_ERR_EOF = 6,
  // UnknownCommand: Unknown command / opcode
  MAV_FTP_ERR_UNKNOWNCOMMAND = 7,
  // FileExists: File/directory already exists
  MAV_FTP_ERR_FILEEXISTS = 8,
  // FileProtected: File/directory is write protected
  MAV_FTP_ERR_FILEPROTECTED = 9,
  // FileNotFound: File/directory not found
  MAV_FTP_ERR_FILENOTFOUND = 10
};

// MAV FTP opcodes: https://mavlink.io/en/services/ftp.html
enum class MAV_FTP_OPCODE {
  // None. Ignored, always ACKed
  MAV_FTP_OPCODE_NONE = 0,
  // TerminateSession: Terminates open Read session
  MAV_FTP_OPCODE_TERMINATESESSION = 1,
  // ResetSessions: Terminates all open read sessions
  MAV_FTP_OPCODE_RESETSESSION = 2,
  // ListDirectory. List files and directories in path from offset
  MAV_FTP_OPCODE_LISTDIRECTORY = 3,
  // OpenFileRO: Opens file at path for reading, returns session
  MAV_FTP_OPCODE_OPENFILERO = 4,
  // ReadFile: Reads size bytes from offset in session
  MAV_FTP_OPCODE_READFILE = 5,
  // CreateFile: Creates file at path for writing, returns session
  MAV_FTP_OPCODE_CREATEFILE = 6,
  // WriteFile: Writes size bytes to offset in session
  MAV_FTP_OPCODE_WRITEFILE = 7,
  // RemoveFile: Remove file at path
  MAV_FTP_OPCODE_REMOVEFILE = 8,
  // CreateDirectory: Creates directory at path
  MAV_FTP_OPCODE_CREATEDIRECTORY = 9,
  // RemoveDirectory: Removes directory at path. The directory must be empty.
  MAV_FTP_OPCODE_REMOVEDIRECTORY = 10,
  // OpenFileWO: Opens file at path for writing, returns session
  MAV_FTP_OPCODE_OPENFILEWO = 11,
  // TruncateFile: Truncate file at path to offset length
  MAV_FTP_OPCODE_TRUNCATEFILE = 12,
  // Rename: Rename path1 to path2
  MAV_FTP_OPCODE_RENAME = 13,
  // CalcFileCRC32: Calculate CRC32 for file at path
  MAV_FTP_OPCODE_CALCFILECRC = 14,
  // BurstReadFile: Burst download session file
  MAV_FTP_OPCODE_BURSTREADFILE = 15,
  // ACK: ACK response
  MAV_FTP_OPCODE_ACK = 128,
  // NAK: NAK response
  MAV_FTP_OPCODE_NAK = 129
};

// The heartbeat message shows that a system or component is present and
// responding. The type and autopilot fields (along with the message component
// id), allow the receiving system to treat further messages from this system
// appropriately (e.g. by laying out the user interface based on the autopilot).
// This microservice is documented at
// https://mavlink.io/en/services/heartbeat.html
class MavLinkHeartbeat : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 0;
  MavLinkHeartbeat() { msgid = kMessageId; }
  // Vehicle or component type. For a flight controller component the vehicle
  // type (quadrotor, helicopter, etc.). For other components the component type
  // (e.g. camera, gimbal, etc.). This should be used in preference to component
  // id for identifying the component type.
  uint8_t type = 0;
  // Autopilot type / class. Use MAV_AUTOPILOT_INVALID for components that are
  // not flight controllers.
  uint8_t autopilot = 0;
  // System mode bitmap.
  uint8_t base_mode = 0;
  // A bitfield for use for autopilot-specific flags
  uint32_t custom_mode = 0;
  // System status flag.
  uint8_t system_status = 0;
  // MAVLink version, not writable by user, gets added by protocol because of
  // magic data type: uint8_t_mavlink_version
  uint8_t mavlink_version = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The general system state. If the system is following the MAVLink standard,
// the system state is mainly defined by three orthogonal states/modes: The
// system mode, which is either LOCKED (motors shut down and locked), MANUAL
// (system under RC control), GUIDED (system with autonomous position control,
// position setpoint controlled manually) or AUTO (system guided by
// path/waypoint planner). The NAV_MODE defined the current flight state:
// LIFTOFF (often an open-loop maneuver), LANDING, WAYPOINTS or VECTOR. This
// represents the internal navigation state machine. The system status shows
// whether the system is currently active or not and if an emergency occurred.
// During the CRITICAL and EMERGENCY states the MAV is still considered to be
// active, but should start emergency procedures autonomously. After a failure
// occurred it should first move from active to critical to allow manual
// intervention and then move to emergency after a certain timeout.
class MavLinkSysStatus : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 1;
  MavLinkSysStatus() { msgid = kMessageId; }
  // Bitmap showing which onboard controllers and sensors are present. Value of
  // 0: not present. Value of 1: present.
  uint32_t onboard_control_sensors_present = 0;
  // Bitmap showing which onboard controllers and sensors are enabled: Value of
  // 0: not enabled. Value of 1: enabled.
  uint32_t onboard_control_sensors_enabled = 0;
  // Bitmap showing which onboard controllers and sensors have an error (or are
  // operational). Value of 0: error. Value of 1: healthy.
  uint32_t onboard_control_sensors_health = 0;
  // Maximum usage in percent of the mainloop time. Values: [0-1000] - should
  // always be below 1000
  uint16_t load = 0;
  // Battery voltage, UINT16_MAX: Voltage not sent by autopilot
  uint16_t voltage_battery = 0;
  // Battery current, -1: Current not sent by autopilot
  int16_t current_battery = 0;
  // Battery energy remaining, -1: Battery remaining energy not sent by
  // autopilot
  int8_t battery_remaining = 0;
  // Communication drop rate, (UART, I2C, SPI, CAN), dropped packets on all
  // links (packets that were corrupted on reception on the MAV)
  uint16_t drop_rate_comm = 0;
  // Communication errors (UART, I2C, SPI, CAN), dropped packets on all links
  // (packets that were corrupted on reception on the MAV)
  uint16_t errors_comm = 0;
  // Autopilot-specific errors
  uint16_t errors_count1 = 0;
  // Autopilot-specific errors
  uint16_t errors_count2 = 0;
  // Autopilot-specific errors
  uint16_t errors_count3 = 0;
  // Autopilot-specific errors
  uint16_t errors_count4 = 0;
  // Bitmap showing which onboard controllers and sensors are present. Value of
  // 0: not present. Value of 1: present.
  uint32_t onboard_control_sensors_present_extended = 0;
  // Bitmap showing which onboard controllers and sensors are enabled: Value of
  // 0: not enabled. Value of 1: enabled.
  uint32_t onboard_control_sensors_enabled_extended = 0;
  // Bitmap showing which onboard controllers and sensors have an error (or are
  // operational). Value of 0: error. Value of 1: healthy.
  uint32_t onboard_control_sensors_health_extended = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The system time is the time of the master clock, typically the computer clock
// of the main onboard computer.
class MavLinkSystemTime : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 2;
  MavLinkSystemTime() { msgid = kMessageId; }
  // Timestamp (UNIX epoch time).
  uint64_t time_unix_usec = 0;
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// A ping message either requesting or responding to a ping. This allows to
// measure the system latencies, including serial port, radio modem and UDP
// connections. The ping microservice is documented at
// https://mavlink.io/en/services/ping.html
class MavLinkPing : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 4;
  MavLinkPing() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // PING sequence
  uint32_t seq = 0;
  // 0: request ping from all receiving systems. If greater than 0: message is a
  // ping response and number is the system id of the requesting system
  uint8_t target_system = 0;
  // 0: request ping from all receiving components. If greater than 0: message
  // is a ping response and number is the component id of the requesting
  // component.
  uint8_t target_component = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request to control this MAV
class MavLinkChangeOperatorControl : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 5;
  MavLinkChangeOperatorControl() { msgid = kMessageId; }
  // System the GCS requests control for
  uint8_t target_system = 0;
  // 0: request control of this MAV, 1: Release control of this MAV
  uint8_t control_request = 0;
  // 0: key as plaintext, 1-255: future, different hashing/encryption variants.
  // The GCS should in general use the safest mode possible initially and then
  // gradually move down the encryption level if it gets a NACK message
  // indicating an encryption mismatch.
  uint8_t version = 0;
  // Password / Key, depending on version plaintext or encrypted. 25 or less
  // characters, NULL terminated. The characters may involve A-Z, a-z, 0-9, and
  // "!?,.-"
  char passkey[25] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Accept / deny control of this MAV
class MavLinkChangeOperatorControlAck : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 6;
  MavLinkChangeOperatorControlAck() { msgid = kMessageId; }
  // ID of the GCS this message
  uint8_t gcs_system_id = 0;
  // 0: request control of this MAV, 1: Release control of this MAV
  uint8_t control_request = 0;
  // 0: ACK, 1: NACK: Wrong passkey, 2: NACK: Unsupported passkey encryption
  // method, 3: NACK: Already under control
  uint8_t ack = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Emit an encrypted signature / key identifying this system. PLEASE NOTE: This
// protocol has been kept simple, so transmitting the key requires an encrypted
// channel for true safety.
class MavLinkAuthKey : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 7;
  MavLinkAuthKey() { msgid = kMessageId; }
  // key
  char key[32] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Status generated in each node in the communication chain and injected into
// MAVLink stream.
class MavLinkLinkNodeStatus : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 8;
  MavLinkLinkNodeStatus() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint64_t timestamp = 0;
  // Remaining free transmit buffer space
  uint8_t tx_buf = 0;
  // Remaining free receive buffer space
  uint8_t rx_buf = 0;
  // Transmit rate
  uint32_t tx_rate = 0;
  // Receive rate
  uint32_t rx_rate = 0;
  // Number of bytes that could not be parsed correctly.
  uint16_t rx_parse_err = 0;
  // Transmit buffer overflows. This number wraps around as it reaches
  // UINT16_MAX
  uint16_t tx_overflows = 0;
  // Receive buffer overflows. This number wraps around as it reaches UINT16_MAX
  uint16_t rx_overflows = 0;
  // Messages sent
  uint32_t messages_sent = 0;
  // Messages received (estimated from counting seq)
  uint32_t messages_received = 0;
  // Messages lost (estimated from counting seq)
  uint32_t messages_lost = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Set the system mode, as defined by enum MAV_MODE. There is no target
// component id as the mode is by definition for the overall aircraft, not only
// for one component.
class MavLinkSetMode : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 11;
  MavLinkSetMode() { msgid = kMessageId; }
  // The system setting the mode
  uint8_t target_system = 0;
  // The new base mode.
  uint8_t base_mode = 0;
  // The new autopilot-specific mode. This field can be ignored by an autopilot.
  uint32_t custom_mode = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request to read the onboard parameter with the param_id string id. Onboard
// parameters are stored as key[const char*] -> value[float]. This allows to
// send a parameter to any other component (such as the GCS) without the need of
// previous knowledge of possible parameter names. Thus the same GCS can store
// different parameters for different autopilots. See also
// https://mavlink.io/en/services/parameter.html for a full documentation of
// QGroundControl and IMU code.
class MavLinkParamRequestRead : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 20;
  MavLinkParamRequestRead() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Onboard parameter id, terminated by NULL if the length is less than 16
  // human-readable chars and WITHOUT null termination (NULL) byte if the length
  // is exactly 16 chars - applications have to provide 16+1 bytes storage if
  // the ID is stored as string
  char param_id[16] = {0};
  // Parameter index. Send -1 to use the param ID field as identifier (else the
  // param id will be ignored)
  int16_t param_index = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request all parameters of this component. After this request, all parameters
// are emitted. The parameter microservice is documented at
// https://mavlink.io/en/services/parameter.html
class MavLinkParamRequestList : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 21;
  MavLinkParamRequestList() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Emit the value of a onboard parameter. The inclusion of param_count and
// param_index in the message allows the recipient to keep track of received
// parameters and allows him to re-request missing parameters after a loss or
// timeout. The parameter microservice is documented at
// https://mavlink.io/en/services/parameter.html
class MavLinkParamValue : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 22;
  MavLinkParamValue() { msgid = kMessageId; }
  // Onboard parameter id, terminated by NULL if the length is less than 16
  // human-readable chars and WITHOUT null termination (NULL) byte if the length
  // is exactly 16 chars - applications have to provide 16+1 bytes storage if
  // the ID is stored as string
  char param_id[16] = {0};
  // Onboard parameter value
  float param_value = 0;
  // Onboard parameter type.
  uint8_t param_type = 0;
  // Total number of onboard parameters
  uint16_t param_count = 0;
  // Index of this onboard parameter
  uint16_t param_index = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Set a parameter value (write new value to permanent storage). The receiving
// component should acknowledge the new parameter value by broadcasting a
// PARAM_VALUE message (broadcasting ensures that multiple GCS all have an
// up-to-date list of all parameters). If the sending GCS did not receive a
// PARAM_VALUE within its timeout time, it should re-send the PARAM_SET message.
// The parameter microservice is documented at
// https://mavlink.io/en/services/parameter.html. PARAM_SET may also be called
// within the context of a transaction (started with MAV_CMD_PARAM_TRANSACTION).
// Within a transaction the receiving component should respond with
// PARAM_ACK_TRANSACTION to the setter component (instead of broadcasting
// PARAM_VALUE), and PARAM_SET should be re-sent if this is ACK not received.
class MavLinkParamSet : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 23;
  MavLinkParamSet() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Onboard parameter id, terminated by NULL if the length is less than 16
  // human-readable chars and WITHOUT null termination (NULL) byte if the length
  // is exactly 16 chars - applications have to provide 16+1 bytes storage if
  // the ID is stored as string
  char param_id[16] = {0};
  // Onboard parameter value
  float param_value = 0;
  // Onboard parameter type.
  uint8_t param_type = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The global position, as returned by the Global Positioning System (GPS). This
// is NOT the global position estimate of the system, but rather a RAW sensor
// value. See message GLOBAL_POSITION_INT for the global position estimate.
class MavLinkGpsRawInt : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 24;
  MavLinkGpsRawInt() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // GPS fix type.
  uint8_t fix_type = 0;
  // Latitude (WGS84, EGM96 ellipsoid)
  int32_t lat = 0;
  // Longitude (WGS84, EGM96 ellipsoid)
  int32_t lon = 0;
  // Altitude (MSL). Positive for up. Note that virtually all GPS modules
  // provide the MSL altitude in addition to the WGS84 altitude.
  int32_t alt = 0;
  // GPS HDOP horizontal dilution of position (unitless * 100). If unknown, set
  // to: UINT16_MAX
  uint16_t eph = 0;
  // GPS VDOP vertical dilution of position (unitless * 100). If unknown, set
  // to: UINT16_MAX
  uint16_t epv = 0;
  // GPS ground speed. If unknown, set to: UINT16_MAX
  uint16_t vel = 0;
  // Course over ground (NOT heading, but direction of movement) in degrees *
  // 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX
  uint16_t cog = 0;
  // Number of satellites visible. If unknown, set to UINT8_MAX
  uint8_t satellites_visible = 0;
  // Altitude (above WGS84, EGM96 ellipsoid). Positive for up.
  int32_t alt_ellipsoid = 0;
  // Position uncertainty.
  uint32_t h_acc = 0;
  // Altitude uncertainty.
  uint32_t v_acc = 0;
  // Speed uncertainty.
  uint32_t vel_acc = 0;
  // Heading / track uncertainty
  uint32_t hdg_acc = 0;
  // Yaw in earth frame from north. Use 0 if this GPS does not provide yaw. Use
  // UINT16_MAX if this GPS is configured to provide yaw and is currently unable
  // to provide it. Use 36000 for north.
  uint16_t yaw = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The positioning status, as reported by GPS. This message is intended to
// display status information about each satellite visible to the receiver. See
// message GLOBAL_POSITION_INT for the global position estimate. This message
// can contain information for up to 20 satellites.
class MavLinkGpsStatus : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 25;
  MavLinkGpsStatus() { msgid = kMessageId; }
  // Number of satellites visible
  uint8_t satellites_visible = 0;
  // Global satellite ID
  uint8_t satellite_prn[20] = {0};
  // 0: Satellite not used, 1: used for localization
  uint8_t satellite_used[20] = {0};
  // Elevation (0: right on top of receiver, 90: on the horizon) of satellite
  uint8_t satellite_elevation[20] = {0};
  // Direction of satellite, 0: 0 deg, 255: 360 deg.
  uint8_t satellite_azimuth[20] = {0};
  // Signal to noise ratio of satellite
  uint8_t satellite_snr[20] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The RAW IMU readings for the usual 9DOF sensor setup. This message should
// contain the scaled values to the described units
class MavLinkScaledImu : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 26;
  MavLinkScaledImu() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // X acceleration
  int16_t xacc = 0;
  // Y acceleration
  int16_t yacc = 0;
  // Z acceleration
  int16_t zacc = 0;
  // Angular speed around X axis
  int16_t xgyro = 0;
  // Angular speed around Y axis
  int16_t ygyro = 0;
  // Angular speed around Z axis
  int16_t zgyro = 0;
  // X Magnetic field
  int16_t xmag = 0;
  // Y Magnetic field
  int16_t ymag = 0;
  // Z Magnetic field
  int16_t zmag = 0;
  // Temperature, 0: IMU does not provide temperature values. If the IMU is at
  // 0C it must send 1 (0.01C).
  int16_t temperature = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The RAW IMU readings for a 9DOF sensor, which is identified by the id
// (default IMU1). This message should always contain the true raw values
// without any scaling to allow data capture and system debugging.
class MavLinkRawImu : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 27;
  MavLinkRawImu() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // X acceleration (raw)
  int16_t xacc = 0;
  // Y acceleration (raw)
  int16_t yacc = 0;
  // Z acceleration (raw)
  int16_t zacc = 0;
  // Angular speed around X axis (raw)
  int16_t xgyro = 0;
  // Angular speed around Y axis (raw)
  int16_t ygyro = 0;
  // Angular speed around Z axis (raw)
  int16_t zgyro = 0;
  // X Magnetic field (raw)
  int16_t xmag = 0;
  // Y Magnetic field (raw)
  int16_t ymag = 0;
  // Z Magnetic field (raw)
  int16_t zmag = 0;
  // Id. Ids are numbered from 0 and map to IMUs numbered from 1 (e.g. IMU1 will
  // have a message with id=0)
  uint8_t id = 0;
  // Temperature, 0: IMU does not provide temperature values. If the IMU is at
  // 0C it must send 1 (0.01C).
  int16_t temperature = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The RAW pressure readings for the typical setup of one absolute pressure and
// one differential pressure sensor. The sensor values should be the raw,
// UNSCALED ADC values.
class MavLinkRawPressure : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 28;
  MavLinkRawPressure() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Absolute pressure (raw)
  int16_t press_abs = 0;
  // Differential pressure 1 (raw, 0 if nonexistent)
  int16_t press_diff1 = 0;
  // Differential pressure 2 (raw, 0 if nonexistent)
  int16_t press_diff2 = 0;
  // Raw Temperature measurement (raw)
  int16_t temperature = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The pressure readings for the typical setup of one absolute and differential
// pressure sensor. The units are as specified in each field.
class MavLinkScaledPressure : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 29;
  MavLinkScaledPressure() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Absolute pressure
  float press_abs = 0;
  // Differential pressure 1
  float press_diff = 0;
  // Absolute pressure temperature
  int16_t temperature = 0;
  // Differential pressure temperature (0, if not available). Report values of 0
  // (or 1) as 1 cdegC.
  int16_t temperature_press_diff = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The attitude in the aeronautical frame (right-handed, Z-down, X-front,
// Y-right).
class MavLinkAttitude : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 30;
  MavLinkAttitude() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Roll angle (-pi..+pi)
  float roll = 0;
  // Pitch angle (-pi..+pi)
  float pitch = 0;
  // Yaw angle (-pi..+pi)
  float yaw = 0;
  // Roll angular speed
  float rollspeed = 0;
  // Pitch angular speed
  float pitchspeed = 0;
  // Yaw angular speed
  float yawspeed = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The attitude in the aeronautical frame (right-handed, Z-down, X-front,
// Y-right), expressed as quaternion. Quaternion order is w, x, y, z and a zero
// rotation would be expressed as (1 0 0 0).
class MavLinkAttitudeQuaternion : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 31;
  MavLinkAttitudeQuaternion() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Quaternion component 1, w (1 in null-rotation)
  float q1 = 0;
  // Quaternion component 2, x (0 in null-rotation)
  float q2 = 0;
  // Quaternion component 3, y (0 in null-rotation)
  float q3 = 0;
  // Quaternion component 4, z (0 in null-rotation)
  float q4 = 0;
  // Roll angular speed
  float rollspeed = 0;
  // Pitch angular speed
  float pitchspeed = 0;
  // Yaw angular speed
  float yawspeed = 0;
  // Rotation offset by which the attitude quaternion and angular speed vector
  // should be rotated for user display (quaternion with [w, x, y, z] order,
  // zero-rotation is [1, 0, 0, 0], send [0, 0, 0, 0] if field not supported).
  // This field is intended for systems in which the reference attitude may
  // change during flight. For example, tailsitters VTOLs rotate their reference
  // attitude by 90 degrees between hover mode and fixed wing mode, thus
  // repr_offset_q is equal to [1, 0, 0, 0] in hover mode and equal to [0.7071,
  // 0, 0.7071, 0] in fixed wing mode.
  float repr_offset_q[4] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The filtered local position (e.g. fused computer vision and accelerometers).
// Coordinate frame is right-handed, Z-axis down (aeronautical frame, NED /
// north-east-down convention)
class MavLinkLocalPositionNed : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 32;
  MavLinkLocalPositionNed() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // X Position
  float x = 0;
  // Y Position
  float y = 0;
  // Z Position
  float z = 0;
  // X Speed
  float vx = 0;
  // Y Speed
  float vy = 0;
  // Z Speed
  float vz = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The filtered global position (e.g. fused GPS and accelerometers). The
// position is in GPS-frame (right-handed, Z-up). It is designed as scaled
// integer message since the resolution of float is not sufficient.
class MavLinkGlobalPositionInt : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 33;
  MavLinkGlobalPositionInt() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Latitude, expressed
  int32_t lat = 0;
  // Longitude, expressed
  int32_t lon = 0;
  // Altitude (MSL). Note that virtually all GPS modules provide both WGS84 and
  // MSL.
  int32_t alt = 0;
  // Altitude above ground
  int32_t relative_alt = 0;
  // Ground X Speed (Latitude, positive north)
  int16_t vx = 0;
  // Ground Y Speed (Longitude, positive east)
  int16_t vy = 0;
  // Ground Z Speed (Altitude, positive down)
  int16_t vz = 0;
  // Vehicle heading (yaw angle), 0.0..359.99 degrees. If unknown, set to:
  // UINT16_MAX
  uint16_t hdg = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The scaled values of the RC channels received: (-100%) -10000, (0%) 0, (100%)
// 10000. Channels that are inactive should be set to UINT16_MAX.
class MavLinkRcChannelsScaled : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 34;
  MavLinkRcChannelsScaled() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Servo output port (set of 8 outputs = 1 port). Flight stacks running on
  // Pixhawk should use: 0 = MAIN, 1 = AUX.
  uint8_t port = 0;
  // RC channel 1 value scaled.
  int16_t chan1_scaled = 0;
  // RC channel 2 value scaled.
  int16_t chan2_scaled = 0;
  // RC channel 3 value scaled.
  int16_t chan3_scaled = 0;
  // RC channel 4 value scaled.
  int16_t chan4_scaled = 0;
  // RC channel 5 value scaled.
  int16_t chan5_scaled = 0;
  // RC channel 6 value scaled.
  int16_t chan6_scaled = 0;
  // RC channel 7 value scaled.
  int16_t chan7_scaled = 0;
  // RC channel 8 value scaled.
  int16_t chan8_scaled = 0;
  // Receive signal strength indicator in device-dependent units/scale. Values:
  // [0-254], UINT8_MAX: invalid/unknown.
  uint8_t rssi = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The RAW values of the RC channels received. The standard PPM modulation is as
// follows: 1000 microseconds: 0%, 2000 microseconds: 100%. A value of
// UINT16_MAX implies the channel is unused. Individual receivers/transmitters
// might violate this specification.
class MavLinkRcChannelsRaw : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 35;
  MavLinkRcChannelsRaw() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Servo output port (set of 8 outputs = 1 port). Flight stacks running on
  // Pixhawk should use: 0 = MAIN, 1 = AUX.
  uint8_t port = 0;
  // RC channel 1 value.
  uint16_t chan1_raw = 0;
  // RC channel 2 value.
  uint16_t chan2_raw = 0;
  // RC channel 3 value.
  uint16_t chan3_raw = 0;
  // RC channel 4 value.
  uint16_t chan4_raw = 0;
  // RC channel 5 value.
  uint16_t chan5_raw = 0;
  // RC channel 6 value.
  uint16_t chan6_raw = 0;
  // RC channel 7 value.
  uint16_t chan7_raw = 0;
  // RC channel 8 value.
  uint16_t chan8_raw = 0;
  // Receive signal strength indicator in device-dependent units/scale. Values:
  // [0-254], UINT8_MAX: invalid/unknown.
  uint8_t rssi = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Superseded by ACTUATOR_OUTPUT_STATUS. The RAW values of the servo outputs
// (for RC input from the remote, use the RC_CHANNELS messages). The standard
// PPM modulation is as follows: 1000 microseconds: 0%, 2000 microseconds: 100%.
class MavLinkServoOutputRaw : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 36;
  MavLinkServoOutputRaw() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint32_t time_usec = 0;
  // Servo output port (set of 8 outputs = 1 port). Flight stacks running on
  // Pixhawk should use: 0 = MAIN, 1 = AUX.
  uint8_t port = 0;
  // Servo output 1 value
  uint16_t servo1_raw = 0;
  // Servo output 2 value
  uint16_t servo2_raw = 0;
  // Servo output 3 value
  uint16_t servo3_raw = 0;
  // Servo output 4 value
  uint16_t servo4_raw = 0;
  // Servo output 5 value
  uint16_t servo5_raw = 0;
  // Servo output 6 value
  uint16_t servo6_raw = 0;
  // Servo output 7 value
  uint16_t servo7_raw = 0;
  // Servo output 8 value
  uint16_t servo8_raw = 0;
  // Servo output 9 value
  uint16_t servo9_raw = 0;
  // Servo output 10 value
  uint16_t servo10_raw = 0;
  // Servo output 11 value
  uint16_t servo11_raw = 0;
  // Servo output 12 value
  uint16_t servo12_raw = 0;
  // Servo output 13 value
  uint16_t servo13_raw = 0;
  // Servo output 14 value
  uint16_t servo14_raw = 0;
  // Servo output 15 value
  uint16_t servo15_raw = 0;
  // Servo output 16 value
  uint16_t servo16_raw = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request a partial list of mission items from the system/component.
// https://mavlink.io/en/services/mission.html. If start and end index are the
// same, just send one waypoint.
class MavLinkMissionRequestPartialList : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 37;
  MavLinkMissionRequestPartialList() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Start index
  int16_t start_index = 0;
  // End index, -1 by default (-1: send list to end). Else a valid index of the
  // list
  int16_t end_index = 0;
  // Mission type.
  uint8_t mission_type = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// This message is sent to the MAV to write a partial list. If start index ==
// end index, only one item will be transmitted / updated. If the start index is
// NOT 0 and above the current list size, this request should be REJECTED!
class MavLinkMissionWritePartialList : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 38;
  MavLinkMissionWritePartialList() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Start index. Must be smaller / equal to the largest index of the current
  // onboard list.
  int16_t start_index = 0;
  // End index, equal or greater than start index.
  int16_t end_index = 0;
  // Mission type.
  uint8_t mission_type = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Message encoding a mission item. This message is emitted to announce the
// presence of a mission item and to set a mission item on the system. The
// mission item can be either in x, y, z meters (type: LOCAL) or x:lat, y:lon,
// z:altitude. Local frame is Z-down, right handed (NED), global frame is Z-up,
// right handed (ENU). NaN may be used to indicate an optional/default value
// (e.g. to use the system's current latitude or yaw rather than a specific
// value). See also https://mavlink.io/en/services/mission.html.
class MavLinkMissionItem : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 39;
  MavLinkMissionItem() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Sequence
  uint16_t seq = 0;
  // The coordinate system of the waypoint.
  uint8_t frame = 0;
  // The scheduled action for the waypoint.
  uint16_t command = 0;
  // false:0, true:1
  uint8_t current = 0;
  // Autocontinue to next waypoint
  uint8_t autocontinue = 0;
  // PARAM1, see MAV_CMD enum
  float param1 = 0;
  // PARAM2, see MAV_CMD enum
  float param2 = 0;
  // PARAM3, see MAV_CMD enum
  float param3 = 0;
  // PARAM4, see MAV_CMD enum
  float param4 = 0;
  // PARAM5 / local: X coordinate, global: latitude
  float x = 0;
  // PARAM6 / local: Y coordinate, global: longitude
  float y = 0;
  // PARAM7 / local: Z coordinate, global: altitude (relative or absolute,
  // depending on frame).
  float z = 0;
  // Mission type.
  uint8_t mission_type = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request the information of the mission item with the sequence number seq. The
// response of the system to this message should be a MISSION_ITEM message.
// https://mavlink.io/en/services/mission.html
class MavLinkMissionRequest : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 40;
  MavLinkMissionRequest() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Sequence
  uint16_t seq = 0;
  // Mission type.
  uint8_t mission_type = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Set the mission item with sequence number seq as current item. This means
// that the MAV will continue to this mission item on the shortest path (not
// following the mission items in-between).
class MavLinkMissionSetCurrent : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 41;
  MavLinkMissionSetCurrent() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Sequence
  uint16_t seq = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Message that announces the sequence number of the current active mission
// item. The MAV will fly towards this mission item.
class MavLinkMissionCurrent : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 42;
  MavLinkMissionCurrent() { msgid = kMessageId; }
  // Sequence
  uint16_t seq = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request the overall list of mission items from the system/component.
class MavLinkMissionRequestList : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 43;
  MavLinkMissionRequestList() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Mission type.
  uint8_t mission_type = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// This message is emitted as response to MISSION_REQUEST_LIST by the MAV and to
// initiate a write transaction. The GCS can then request the individual mission
// item based on the knowledge of the total number of waypoints.
class MavLinkMissionCount : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 44;
  MavLinkMissionCount() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Number of mission items in the sequence
  uint16_t count = 0;
  // Mission type.
  uint8_t mission_type = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Delete all mission items at once.
class MavLinkMissionClearAll : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 45;
  MavLinkMissionClearAll() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Mission type.
  uint8_t mission_type = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// A certain mission item has been reached. The system will either hold this
// position (or circle on the orbit) or (if the autocontinue on the WP was set)
// continue to the next waypoint.
class MavLinkMissionItemReached : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 46;
  MavLinkMissionItemReached() { msgid = kMessageId; }
  // Sequence
  uint16_t seq = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Acknowledgment message during waypoint handling. The type field states if
// this message is a positive ack (type=0) or if an error happened
// (type=non-zero).
class MavLinkMissionAck : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 47;
  MavLinkMissionAck() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Mission result.
  uint8_t type = 0;
  // Mission type.
  uint8_t mission_type = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Sets the GPS co-ordinates of the vehicle local origin (0,0,0) position.
// Vehicle should emit GPS_GLOBAL_ORIGIN irrespective of whether the origin is
// changed. This enables transform between the local coordinate frame and the
// global (GPS) coordinate frame, which may be necessary when (for example)
// indoor and outdoor settings are connected and the MAV should move from in- to
// outdoor.
class MavLinkSetGpsGlobalOrigin : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 48;
  MavLinkSetGpsGlobalOrigin() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Latitude (WGS84)
  int32_t latitude = 0;
  // Longitude (WGS84)
  int32_t longitude = 0;
  // Altitude (MSL). Positive for up.
  int32_t altitude = 0;
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Publishes the GPS co-ordinates of the vehicle local origin (0,0,0) position.
// Emitted whenever a new GPS-Local position mapping is requested or set - e.g.
// following SET_GPS_GLOBAL_ORIGIN message.
class MavLinkGpsGlobalOrigin : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 49;
  MavLinkGpsGlobalOrigin() { msgid = kMessageId; }
  // Latitude (WGS84)
  int32_t latitude = 0;
  // Longitude (WGS84)
  int32_t longitude = 0;
  // Altitude (MSL). Positive for up.
  int32_t altitude = 0;
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Bind a RC channel to a parameter. The parameter should change according to
// the RC channel value.
class MavLinkParamMapRc : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 50;
  MavLinkParamMapRc() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Onboard parameter id, terminated by NULL if the length is less than 16
  // human-readable chars and WITHOUT null termination (NULL) byte if the length
  // is exactly 16 chars - applications have to provide 16+1 bytes storage if
  // the ID is stored as string
  char param_id[16] = {0};
  // Parameter index. Send -1 to use the param ID field as identifier (else the
  // param id will be ignored), send -2 to disable any existing map for this
  // rc_channel_index.
  int16_t param_index = 0;
  // Index of parameter RC channel. Not equal to the RC channel id. Typically
  // corresponds to a potentiometer-knob on the RC.
  uint8_t parameter_rc_channel_index = 0;
  // Initial parameter value
  float param_value0 = 0;
  // Scale, maps the RC range [-1, 1] to a parameter value
  float scale = 0;
  // Minimum param value. The protocol does not define if this overwrites an
  // onboard minimum value. (Depends on implementation)
  float param_value_min = 0;
  // Maximum param value. The protocol does not define if this overwrites an
  // onboard maximum value. (Depends on implementation)
  float param_value_max = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request the information of the mission item with the sequence number seq. The
// response of the system to this message should be a MISSION_ITEM_INT message.
// https://mavlink.io/en/services/mission.html
class MavLinkMissionRequestInt : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 51;
  MavLinkMissionRequestInt() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Sequence
  uint16_t seq = 0;
  // Mission type.
  uint8_t mission_type = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Set a safety zone (volume), which is defined by two corners of a cube. This
// message can be used to tell the MAV which setpoints/waypoints to accept and
// which to reject. Safety areas are often enforced by national or competition
// regulations.
class MavLinkSafetySetAllowedArea : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 54;
  MavLinkSafetySetAllowedArea() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Coordinate frame. Can be either global, GPS, right-handed with Z axis up or
  // local, right handed, Z axis down.
  uint8_t frame = 0;
  // x position 1 / Latitude 1
  float p1x = 0;
  // y position 1 / Longitude 1
  float p1y = 0;
  // z position 1 / Altitude 1
  float p1z = 0;
  // x position 2 / Latitude 2
  float p2x = 0;
  // y position 2 / Longitude 2
  float p2y = 0;
  // z position 2 / Altitude 2
  float p2z = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Read out the safety zone the MAV currently assumes.
class MavLinkSafetyAllowedArea : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 55;
  MavLinkSafetyAllowedArea() { msgid = kMessageId; }
  // Coordinate frame. Can be either global, GPS, right-handed with Z axis up or
  // local, right handed, Z axis down.
  uint8_t frame = 0;
  // x position 1 / Latitude 1
  float p1x = 0;
  // y position 1 / Longitude 1
  float p1y = 0;
  // z position 1 / Altitude 1
  float p1z = 0;
  // x position 2 / Latitude 2
  float p2x = 0;
  // y position 2 / Longitude 2
  float p2y = 0;
  // z position 2 / Altitude 2
  float p2z = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The attitude in the aeronautical frame (right-handed, Z-down, X-front,
// Y-right), expressed as quaternion. Quaternion order is w, x, y, z and a zero
// rotation would be expressed as (1 0 0 0).
class MavLinkAttitudeQuaternionCov : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 61;
  MavLinkAttitudeQuaternionCov() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Quaternion components, w, x, y, z (1 0 0 0 is the null-rotation)
  float q[4] = {0};
  // Roll angular speed
  float rollspeed = 0;
  // Pitch angular speed
  float pitchspeed = 0;
  // Yaw angular speed
  float yawspeed = 0;
  // Row-major representation of a 3x3 attitude covariance matrix (states: roll,
  // pitch, yaw; first three entries are the first ROW, next three entries are
  // the second row, etc.). If unknown, assign NaN value to first element in the
  // array.
  float covariance[9] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The state of the navigation and position controller.
class MavLinkNavControllerOutput : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 62;
  MavLinkNavControllerOutput() { msgid = kMessageId; }
  // Current desired roll
  float nav_roll = 0;
  // Current desired pitch
  float nav_pitch = 0;
  // Current desired heading
  int16_t nav_bearing = 0;
  // Bearing to current waypoint/target
  int16_t target_bearing = 0;
  // Distance to active waypoint
  uint16_t wp_dist = 0;
  // Current altitude error
  float alt_error = 0;
  // Current airspeed error
  float aspd_error = 0;
  // Current crosstrack error on x-y plane
  float xtrack_error = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The filtered global position (e.g. fused GPS and accelerometers). The
// position is in GPS-frame (right-handed, Z-up). It is designed as scaled
// integer message since the resolution of float is not sufficient. NOTE: This
// message is intended for onboard networks / companion computers and
// higher-bandwidth links and optimized for accuracy and completeness. Please
// use the GLOBAL_POSITION_INT message for a minimal subset.
class MavLinkGlobalPositionIntCov : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 63;
  MavLinkGlobalPositionIntCov() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Class id of the estimator this estimate originated from.
  uint8_t estimator_type = 0;
  // Latitude
  int32_t lat = 0;
  // Longitude
  int32_t lon = 0;
  // Altitude in meters above MSL
  int32_t alt = 0;
  // Altitude above ground
  int32_t relative_alt = 0;
  // Ground X Speed (Latitude)
  float vx = 0;
  // Ground Y Speed (Longitude)
  float vy = 0;
  // Ground Z Speed (Altitude)
  float vz = 0;
  // Row-major representation of a 6x6 position and velocity 6x6
  // cross-covariance matrix (states: lat, lon, alt, vx, vy, vz; first six
  // entries are the first ROW, next six entries are the second row, etc.). If
  // unknown, assign NaN value to first element in the array.
  float covariance[36] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The filtered local position (e.g. fused computer vision and accelerometers).
// Coordinate frame is right-handed, Z-axis down (aeronautical frame, NED /
// north-east-down convention)
class MavLinkLocalPositionNedCov : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 64;
  MavLinkLocalPositionNedCov() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Class id of the estimator this estimate originated from.
  uint8_t estimator_type = 0;
  // X Position
  float x = 0;
  // Y Position
  float y = 0;
  // Z Position
  float z = 0;
  // X Speed
  float vx = 0;
  // Y Speed
  float vy = 0;
  // Z Speed
  float vz = 0;
  // X Acceleration
  float ax = 0;
  // Y Acceleration
  float ay = 0;
  // Z Acceleration
  float az = 0;
  // Row-major representation of position, velocity and acceleration 9x9
  // cross-covariance matrix upper right triangle (states: x, y, z, vx, vy, vz,
  // ax, ay, az; first nine entries are the first ROW, next eight entries are
  // the second row, etc.). If unknown, assign NaN value to first element in the
  // array.
  float covariance[45] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The PPM values of the RC channels received. The standard PPM modulation is as
// follows: 1000 microseconds: 0%, 2000 microseconds: 100%. A value of
// UINT16_MAX implies the channel is unused. Individual receivers/transmitters
// might violate this specification.
class MavLinkRcChannels : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 65;
  MavLinkRcChannels() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Total number of RC channels being received. This can be larger than 18,
  // indicating that more channels are available but not given in this message.
  // This value should be 0 when no RC channels are available.
  uint8_t chancount = 0;
  // RC channel 1 value.
  uint16_t chan1_raw = 0;
  // RC channel 2 value.
  uint16_t chan2_raw = 0;
  // RC channel 3 value.
  uint16_t chan3_raw = 0;
  // RC channel 4 value.
  uint16_t chan4_raw = 0;
  // RC channel 5 value.
  uint16_t chan5_raw = 0;
  // RC channel 6 value.
  uint16_t chan6_raw = 0;
  // RC channel 7 value.
  uint16_t chan7_raw = 0;
  // RC channel 8 value.
  uint16_t chan8_raw = 0;
  // RC channel 9 value.
  uint16_t chan9_raw = 0;
  // RC channel 10 value.
  uint16_t chan10_raw = 0;
  // RC channel 11 value.
  uint16_t chan11_raw = 0;
  // RC channel 12 value.
  uint16_t chan12_raw = 0;
  // RC channel 13 value.
  uint16_t chan13_raw = 0;
  // RC channel 14 value.
  uint16_t chan14_raw = 0;
  // RC channel 15 value.
  uint16_t chan15_raw = 0;
  // RC channel 16 value.
  uint16_t chan16_raw = 0;
  // RC channel 17 value.
  uint16_t chan17_raw = 0;
  // RC channel 18 value.
  uint16_t chan18_raw = 0;
  // Receive signal strength indicator in device-dependent units/scale. Values:
  // [0-254], UINT8_MAX: invalid/unknown.
  uint8_t rssi = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request a data stream.
class MavLinkRequestDataStream : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 66;
  MavLinkRequestDataStream() { msgid = kMessageId; }
  // The target requested to send the message stream.
  uint8_t target_system = 0;
  // The target requested to send the message stream.
  uint8_t target_component = 0;
  // The ID of the requested data stream
  uint8_t req_stream_id = 0;
  // The requested message rate
  uint16_t req_message_rate = 0;
  // 1 to start sending, 0 to stop sending.
  uint8_t start_stop = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Data stream status information.
class MavLinkDataStream : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 67;
  MavLinkDataStream() { msgid = kMessageId; }
  // The ID of the requested data stream
  uint8_t stream_id = 0;
  // The message rate
  uint16_t message_rate = 0;
  // 1 stream is enabled, 0 stream is stopped.
  uint8_t on_off = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// This message provides an API for manually controlling the vehicle using
// standard joystick axes nomenclature, along with a joystick-like input device.
// Unused axes can be disabled and buttons states are transmitted as individual
// on/off bits of a bitmask
class MavLinkManualControl : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 69;
  MavLinkManualControl() { msgid = kMessageId; }
  // The system to be controlled.
  uint8_t target = 0;
  // X-axis, normalized to the range [-1000,1000]. A value of INT16_MAX
  // indicates that this axis is invalid. Generally corresponds to
  // forward(1000)-backward(-1000) movement on a joystick and the pitch of a
  // vehicle.
  int16_t x = 0;
  // Y-axis, normalized to the range [-1000,1000]. A value of INT16_MAX
  // indicates that this axis is invalid. Generally corresponds to
  // left(-1000)-right(1000) movement on a joystick and the roll of a vehicle.
  int16_t y = 0;
  // Z-axis, normalized to the range [-1000,1000]. A value of INT16_MAX
  // indicates that this axis is invalid. Generally corresponds to a separate
  // slider movement with maximum being 1000 and minimum being -1000 on a
  // joystick and the thrust of a vehicle. Positive values are positive thrust,
  // negative values are negative thrust.
  int16_t z = 0;
  // R-axis, normalized to the range [-1000,1000]. A value of INT16_MAX
  // indicates that this axis is invalid. Generally corresponds to a twisting of
  // the joystick, with counter-clockwise being 1000 and clockwise being -1000,
  // and the yaw of a vehicle.
  int16_t r = 0;
  // A bitfield corresponding to the joystick buttons' 0-15 current state, 1 for
  // pressed, 0 for released. The lowest bit corresponds to Button 1.
  uint16_t buttons = 0;
  // A bitfield corresponding to the joystick buttons' 16-31 current state, 1
  // for pressed, 0 for released. The lowest bit corresponds to Button 16.
  uint16_t buttons2 = 0;
  // Set bits to 1 to indicate which of the following extension fields contain
  // valid data: bit 0: pitch, bit 1: roll.
  uint8_t enabled_extensions = 0;
  // Pitch-only-axis, normalized to the range [-1000,1000]. Generally
  // corresponds to pitch on vehicles with additional degrees of freedom. Valid
  // if bit 0 of enabled_extensions field is set. Set to 0 if invalid.
  int16_t s = 0;
  // Roll-only-axis, normalized to the range [-1000,1000]. Generally corresponds
  // to roll on vehicles with additional degrees of freedom. Valid if bit 1 of
  // enabled_extensions field is set. Set to 0 if invalid.
  int16_t t = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The RAW values of the RC channels sent to the MAV to override info received
// from the RC radio. The standard PPM modulation is as follows: 1000
// microseconds: 0%, 2000 microseconds: 100%. Individual receivers/transmitters
// might violate this specification. Note carefully the semantic differences
// between the first 8 channels and the subsequent channels
class MavLinkRcChannelsOverride : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 70;
  MavLinkRcChannelsOverride() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // RC channel 1 value. A value of UINT16_MAX means to ignore this field. A
  // value of 0 means to release this channel back to the RC radio.
  uint16_t chan1_raw = 0;
  // RC channel 2 value. A value of UINT16_MAX means to ignore this field. A
  // value of 0 means to release this channel back to the RC radio.
  uint16_t chan2_raw = 0;
  // RC channel 3 value. A value of UINT16_MAX means to ignore this field. A
  // value of 0 means to release this channel back to the RC radio.
  uint16_t chan3_raw = 0;
  // RC channel 4 value. A value of UINT16_MAX means to ignore this field. A
  // value of 0 means to release this channel back to the RC radio.
  uint16_t chan4_raw = 0;
  // RC channel 5 value. A value of UINT16_MAX means to ignore this field. A
  // value of 0 means to release this channel back to the RC radio.
  uint16_t chan5_raw = 0;
  // RC channel 6 value. A value of UINT16_MAX means to ignore this field. A
  // value of 0 means to release this channel back to the RC radio.
  uint16_t chan6_raw = 0;
  // RC channel 7 value. A value of UINT16_MAX means to ignore this field. A
  // value of 0 means to release this channel back to the RC radio.
  uint16_t chan7_raw = 0;
  // RC channel 8 value. A value of UINT16_MAX means to ignore this field. A
  // value of 0 means to release this channel back to the RC radio.
  uint16_t chan8_raw = 0;
  // RC channel 9 value. A value of 0 or UINT16_MAX means to ignore this field.
  // A value of UINT16_MAX-1 means to release this channel back to the RC radio.
  uint16_t chan9_raw = 0;
  // RC channel 10 value. A value of 0 or UINT16_MAX means to ignore this field.
  // A value of UINT16_MAX-1 means to release this channel back to the RC radio.
  uint16_t chan10_raw = 0;
  // RC channel 11 value. A value of 0 or UINT16_MAX means to ignore this field.
  // A value of UINT16_MAX-1 means to release this channel back to the RC radio.
  uint16_t chan11_raw = 0;
  // RC channel 12 value. A value of 0 or UINT16_MAX means to ignore this field.
  // A value of UINT16_MAX-1 means to release this channel back to the RC radio.
  uint16_t chan12_raw = 0;
  // RC channel 13 value. A value of 0 or UINT16_MAX means to ignore this field.
  // A value of UINT16_MAX-1 means to release this channel back to the RC radio.
  uint16_t chan13_raw = 0;
  // RC channel 14 value. A value of 0 or UINT16_MAX means to ignore this field.
  // A value of UINT16_MAX-1 means to release this channel back to the RC radio.
  uint16_t chan14_raw = 0;
  // RC channel 15 value. A value of 0 or UINT16_MAX means to ignore this field.
  // A value of UINT16_MAX-1 means to release this channel back to the RC radio.
  uint16_t chan15_raw = 0;
  // RC channel 16 value. A value of 0 or UINT16_MAX means to ignore this field.
  // A value of UINT16_MAX-1 means to release this channel back to the RC radio.
  uint16_t chan16_raw = 0;
  // RC channel 17 value. A value of 0 or UINT16_MAX means to ignore this field.
  // A value of UINT16_MAX-1 means to release this channel back to the RC radio.
  uint16_t chan17_raw = 0;
  // RC channel 18 value. A value of 0 or UINT16_MAX means to ignore this field.
  // A value of UINT16_MAX-1 means to release this channel back to the RC radio.
  uint16_t chan18_raw = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Message encoding a mission item. This message is emitted to announce the
// presence of a mission item and to set a mission item on the system. The
// mission item can be either in x, y, z meters (type: LOCAL) or x:lat, y:lon,
// z:altitude. Local frame is Z-down, right handed (NED), global frame is Z-up,
// right handed (ENU). NaN or INT32_MAX may be used in float/integer params
// (respectively) to indicate optional/default values (e.g. to use the
// component's current latitude, yaw rather than a specific value). See also
// https://mavlink.io/en/services/mission.html.
class MavLinkMissionItemInt : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 73;
  MavLinkMissionItemInt() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Waypoint ID (sequence number). Starts at zero. Increases monotonically for
  // each waypoint, no gaps in the sequence (0,1,2,3,4).
  uint16_t seq = 0;
  // The coordinate system of the waypoint.
  uint8_t frame = 0;
  // The scheduled action for the waypoint.
  uint16_t command = 0;
  // false:0, true:1
  uint8_t current = 0;
  // Autocontinue to next waypoint
  uint8_t autocontinue = 0;
  // PARAM1, see MAV_CMD enum
  float param1 = 0;
  // PARAM2, see MAV_CMD enum
  float param2 = 0;
  // PARAM3, see MAV_CMD enum
  float param3 = 0;
  // PARAM4, see MAV_CMD enum
  float param4 = 0;
  // PARAM5 / local: x position in meters * 1e4, global: latitude in degrees *
  // 10^7
  int32_t x = 0;
  // PARAM6 / y position: local: x position in meters * 1e4, global: longitude
  // in degrees *10^7
  int32_t y = 0;
  // PARAM7 / z position: global: altitude in meters (relative or absolute,
  // depending on frame.
  float z = 0;
  // Mission type.
  uint8_t mission_type = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Metrics typically displayed on a HUD for fixed wing aircraft.
class MavLinkVfrHud : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 74;
  MavLinkVfrHud() { msgid = kMessageId; }
  // Vehicle speed in form appropriate for vehicle type. For standard aircraft
  // this is typically calibrated airspeed (CAS) or indicated airspeed (IAS) -
  // either of which can be used by a pilot to estimate stall speed.
  float airspeed = 0;
  // Current ground speed.
  float groundspeed = 0;
  // Current heading in compass units (0-360, 0=north).
  int16_t heading = 0;
  // Current throttle setting (0 to 100).
  uint16_t throttle = 0;
  // Current altitude (MSL).
  float alt = 0;
  // Current climb rate.
  float climb = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Message encoding a command with parameters as scaled integers. Scaling
// depends on the actual command value. NaN or INT32_MAX may be used in
// float/integer params (respectively) to indicate optional/default values (e.g.
// to use the component's current latitude, yaw rather than a specific value).
// The command microservice is documented at
// https://mavlink.io/en/services/command.html
class MavLinkCommandInt : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 75;
  MavLinkCommandInt() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // The coordinate system of the COMMAND.
  uint8_t frame = 0;
  // The scheduled action for the mission item.
  uint16_t command = 0;
  // Not used.
  uint8_t current = 0;
  // Not used (set 0).
  uint8_t autocontinue = 0;
  // PARAM1, see MAV_CMD enum
  float param1 = 0;
  // PARAM2, see MAV_CMD enum
  float param2 = 0;
  // PARAM3, see MAV_CMD enum
  float param3 = 0;
  // PARAM4, see MAV_CMD enum
  float param4 = 0;
  // PARAM5 / local: x position in meters * 1e4, global: latitude in degrees *
  // 10^7
  int32_t x = 0;
  // PARAM6 / local: y position in meters * 1e4, global: longitude in degrees *
  // 10^7
  int32_t y = 0;
  // PARAM7 / z position: global: altitude in meters (relative or absolute,
  // depending on frame).
  float z = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Send a command with up to seven parameters to the MAV. The command
// microservice is documented at https://mavlink.io/en/services/command.html
class MavLinkCommandLong : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 76;
  MavLinkCommandLong() { msgid = kMessageId; }
  // System which should execute the command
  uint8_t target_system = 0;
  // Component which should execute the command, 0 for all components
  uint8_t target_component = 0;
  // Command ID (of command to send).
  uint16_t command = 0;
  // 0: First transmission of this command. 1-255: Confirmation transmissions
  // (e.g. for kill command)
  uint8_t confirmation = 0;
  // Parameter 1 (for the specific command).
  float param1 = 0;
  // Parameter 2 (for the specific command).
  float param2 = 0;
  // Parameter 3 (for the specific command).
  float param3 = 0;
  // Parameter 4 (for the specific command).
  float param4 = 0;
  // Parameter 5 (for the specific command).
  float param5 = 0;
  // Parameter 6 (for the specific command).
  float param6 = 0;
  // Parameter 7 (for the specific command).
  float param7 = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Report status of a command. Includes feedback whether the command was
// executed. The command microservice is documented at
// https://mavlink.io/en/services/command.html
class MavLinkCommandAck : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 77;
  MavLinkCommandAck() { msgid = kMessageId; }
  // Command ID (of acknowledged command).
  uint16_t command = 0;
  // Result of command.
  uint8_t result = 0;
  // Also used as result_param1, it can be set with an enum containing the
  // errors reasons of why the command was denied, or the progress percentage
  // when result is MAV_RESULT_IN_PROGRESS (UINT8_MAX if the progress is
  // unknown).
  uint8_t progress = 0;
  // Additional parameter of the result, example: which parameter of
  // MAV_CMD_NAV_WAYPOINT caused it to be denied.
  int32_t result_param2 = 0;
  // System ID of the target recipient. This is the ID of the system that sent
  // the command for which this COMMAND_ACK is an acknowledgement.
  uint8_t target_system = 0;
  // Component ID of the target recipient. This is the ID of the system that
  // sent the command for which this COMMAND_ACK is an acknowledgement.
  uint8_t target_component = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Cancel a long running command. The target system should respond with a
// COMMAND_ACK to the original command with result=MAV_RESULT_CANCELLED if the
// long running process was cancelled. If it has already completed, the cancel
// action can be ignored. The cancel action can be retried until some sort of
// acknowledgement to the original command has been received. The command
// microservice is documented at https://mavlink.io/en/services/command.html
class MavLinkCommandCancel : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 80;
  MavLinkCommandCancel() { msgid = kMessageId; }
  // System executing long running command. Should not be broadcast (0).
  uint8_t target_system = 0;
  // Component executing long running command.
  uint8_t target_component = 0;
  // Command ID (of command to cancel).
  uint16_t command = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Setpoint in roll, pitch, yaw and thrust from the operator
class MavLinkManualSetpoint : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 81;
  MavLinkManualSetpoint() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Desired roll rate
  float roll = 0;
  // Desired pitch rate
  float pitch = 0;
  // Desired yaw rate
  float yaw = 0;
  // Collective thrust, normalized to 0 .. 1
  float thrust = 0;
  // Flight mode switch position, 0.. 255
  uint8_t mode_switch = 0;
  // Override mode switch position, 0.. 255
  uint8_t manual_override_switch = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Sets a desired vehicle attitude. Used by an external controller to command
// the vehicle (manual controller or other system).
class MavLinkSetAttitudeTarget : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 82;
  MavLinkSetAttitudeTarget() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Bitmap to indicate which dimensions should be ignored by the vehicle.
  uint8_t type_mask = 0;
  // Attitude quaternion (w, x, y, z order, zero-rotation is 1, 0, 0, 0)
  float q[4] = {0};
  // Body roll rate
  float body_roll_rate = 0;
  // Body pitch rate
  float body_pitch_rate = 0;
  // Body yaw rate
  float body_yaw_rate = 0;
  // Collective thrust, normalized to 0 .. 1 (-1 .. 1 for vehicles capable of
  // reverse trust)
  float thrust = 0;
  // 3D thrust setpoint in the body NED frame, normalized to -1 .. 1
  float thrust_body[3] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Reports the current commanded attitude of the vehicle as specified by the
// autopilot. This should match the commands sent in a SET_ATTITUDE_TARGET
// message if the vehicle is being controlled this way.
class MavLinkAttitudeTarget : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 83;
  MavLinkAttitudeTarget() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Bitmap to indicate which dimensions should be ignored by the vehicle.
  uint8_t type_mask = 0;
  // Attitude quaternion (w, x, y, z order, zero-rotation is 1, 0, 0, 0)
  float q[4] = {0};
  // Body roll rate
  float body_roll_rate = 0;
  // Body pitch rate
  float body_pitch_rate = 0;
  // Body yaw rate
  float body_yaw_rate = 0;
  // Collective thrust, normalized to 0 .. 1 (-1 .. 1 for vehicles capable of
  // reverse trust)
  float thrust = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Sets a desired vehicle position in a local north-east-down coordinate frame.
// Used by an external controller to command the vehicle (manual controller or
// other system).
class MavLinkSetPositionTargetLocalNed : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 84;
  MavLinkSetPositionTargetLocalNed() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Valid options are: MAV_FRAME_LOCAL_NED = 1, MAV_FRAME_LOCAL_OFFSET_NED = 7,
  // MAV_FRAME_BODY_NED = 8, MAV_FRAME_BODY_OFFSET_NED = 9
  uint8_t coordinate_frame = 0;
  // Bitmap to indicate which dimensions should be ignored by the vehicle.
  uint16_t type_mask = 0;
  // X Position in NED frame
  float x = 0;
  // Y Position in NED frame
  float y = 0;
  // Z Position in NED frame (note, altitude is negative in NED)
  float z = 0;
  // X velocity in NED frame
  float vx = 0;
  // Y velocity in NED frame
  float vy = 0;
  // Z velocity in NED frame
  float vz = 0;
  // X acceleration or force (if bit 10 of type_mask is set) in NED frame in
  // meter / s^2 or N
  float afx = 0;
  // Y acceleration or force (if bit 10 of type_mask is set) in NED frame in
  // meter / s^2 or N
  float afy = 0;
  // Z acceleration or force (if bit 10 of type_mask is set) in NED frame in
  // meter / s^2 or N
  float afz = 0;
  // yaw setpoint
  float yaw = 0;
  // yaw rate setpoint
  float yaw_rate = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Reports the current commanded vehicle position, velocity, and acceleration as
// specified by the autopilot. This should match the commands sent in
// SET_POSITION_TARGET_LOCAL_NED if the vehicle is being controlled this way.
class MavLinkPositionTargetLocalNed : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 85;
  MavLinkPositionTargetLocalNed() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Valid options are: MAV_FRAME_LOCAL_NED = 1, MAV_FRAME_LOCAL_OFFSET_NED = 7,
  // MAV_FRAME_BODY_NED = 8, MAV_FRAME_BODY_OFFSET_NED = 9
  uint8_t coordinate_frame = 0;
  // Bitmap to indicate which dimensions should be ignored by the vehicle.
  uint16_t type_mask = 0;
  // X Position in NED frame
  float x = 0;
  // Y Position in NED frame
  float y = 0;
  // Z Position in NED frame (note, altitude is negative in NED)
  float z = 0;
  // X velocity in NED frame
  float vx = 0;
  // Y velocity in NED frame
  float vy = 0;
  // Z velocity in NED frame
  float vz = 0;
  // X acceleration or force (if bit 10 of type_mask is set) in NED frame in
  // meter / s^2 or N
  float afx = 0;
  // Y acceleration or force (if bit 10 of type_mask is set) in NED frame in
  // meter / s^2 or N
  float afy = 0;
  // Z acceleration or force (if bit 10 of type_mask is set) in NED frame in
  // meter / s^2 or N
  float afz = 0;
  // yaw setpoint
  float yaw = 0;
  // yaw rate setpoint
  float yaw_rate = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Sets a desired vehicle position, velocity, and/or acceleration in a global
// coordinate system (WGS84). Used by an external controller to command the
// vehicle (manual controller or other system).
class MavLinkSetPositionTargetGlobalInt : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 86;
  MavLinkSetPositionTargetGlobalInt() { msgid = kMessageId; }
  // Timestamp (time since system boot). The rationale for the timestamp in the
  // setpoint is to allow the system to compensate for the transport delay of
  // the setpoint. This allows the system to compensate processing latency.
  uint32_t time_boot_ms = 0;
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Valid options are: MAV_FRAME_GLOBAL_INT = 5,
  // MAV_FRAME_GLOBAL_RELATIVE_ALT_INT = 6, MAV_FRAME_GLOBAL_TERRAIN_ALT_INT =
  // 11
  uint8_t coordinate_frame = 0;
  // Bitmap to indicate which dimensions should be ignored by the vehicle.
  uint16_t type_mask = 0;
  // X Position in WGS84 frame
  int32_t lat_int = 0;
  // Y Position in WGS84 frame
  int32_t lon_int = 0;
  // Altitude (MSL, Relative to home, or AGL - depending on frame)
  float alt = 0;
  // X velocity in NED frame
  float vx = 0;
  // Y velocity in NED frame
  float vy = 0;
  // Z velocity in NED frame
  float vz = 0;
  // X acceleration or force (if bit 10 of type_mask is set) in NED frame in
  // meter / s^2 or N
  float afx = 0;
  // Y acceleration or force (if bit 10 of type_mask is set) in NED frame in
  // meter / s^2 or N
  float afy = 0;
  // Z acceleration or force (if bit 10 of type_mask is set) in NED frame in
  // meter / s^2 or N
  float afz = 0;
  // yaw setpoint
  float yaw = 0;
  // yaw rate setpoint
  float yaw_rate = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Reports the current commanded vehicle position, velocity, and acceleration as
// specified by the autopilot. This should match the commands sent in
// SET_POSITION_TARGET_GLOBAL_INT if the vehicle is being controlled this way.
class MavLinkPositionTargetGlobalInt : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 87;
  MavLinkPositionTargetGlobalInt() { msgid = kMessageId; }
  // Timestamp (time since system boot). The rationale for the timestamp in the
  // setpoint is to allow the system to compensate for the transport delay of
  // the setpoint. This allows the system to compensate processing latency.
  uint32_t time_boot_ms = 0;
  // Valid options are: MAV_FRAME_GLOBAL_INT = 5,
  // MAV_FRAME_GLOBAL_RELATIVE_ALT_INT = 6, MAV_FRAME_GLOBAL_TERRAIN_ALT_INT =
  // 11
  uint8_t coordinate_frame = 0;
  // Bitmap to indicate which dimensions should be ignored by the vehicle.
  uint16_t type_mask = 0;
  // X Position in WGS84 frame
  int32_t lat_int = 0;
  // Y Position in WGS84 frame
  int32_t lon_int = 0;
  // Altitude (MSL, AGL or relative to home altitude, depending on frame)
  float alt = 0;
  // X velocity in NED frame
  float vx = 0;
  // Y velocity in NED frame
  float vy = 0;
  // Z velocity in NED frame
  float vz = 0;
  // X acceleration or force (if bit 10 of type_mask is set) in NED frame in
  // meter / s^2 or N
  float afx = 0;
  // Y acceleration or force (if bit 10 of type_mask is set) in NED frame in
  // meter / s^2 or N
  float afy = 0;
  // Z acceleration or force (if bit 10 of type_mask is set) in NED frame in
  // meter / s^2 or N
  float afz = 0;
  // yaw setpoint
  float yaw = 0;
  // yaw rate setpoint
  float yaw_rate = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The offset in X, Y, Z and yaw between the LOCAL_POSITION_NED messages of MAV
// X and the global coordinate frame in NED coordinates. Coordinate frame is
// right-handed, Z-axis down (aeronautical frame, NED / north-east-down
// convention)
class MavLinkLocalPositionNedSystemGlobalOffset : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 89;
  MavLinkLocalPositionNedSystemGlobalOffset() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // X Position
  float x = 0;
  // Y Position
  float y = 0;
  // Z Position
  float z = 0;
  // Roll
  float roll = 0;
  // Pitch
  float pitch = 0;
  // Yaw
  float yaw = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Sent from simulation to autopilot. This packet is useful for high throughput
// applications such as hardware in the loop simulations.
class MavLinkHilState : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 90;
  MavLinkHilState() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Roll angle
  float roll = 0;
  // Pitch angle
  float pitch = 0;
  // Yaw angle
  float yaw = 0;
  // Body frame roll / phi angular speed
  float rollspeed = 0;
  // Body frame pitch / theta angular speed
  float pitchspeed = 0;
  // Body frame yaw / psi angular speed
  float yawspeed = 0;
  // Latitude
  int32_t lat = 0;
  // Longitude
  int32_t lon = 0;
  // Altitude
  int32_t alt = 0;
  // Ground X Speed (Latitude)
  int16_t vx = 0;
  // Ground Y Speed (Longitude)
  int16_t vy = 0;
  // Ground Z Speed (Altitude)
  int16_t vz = 0;
  // X acceleration
  int16_t xacc = 0;
  // Y acceleration
  int16_t yacc = 0;
  // Z acceleration
  int16_t zacc = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Sent from autopilot to simulation. Hardware in the loop control outputs
class MavLinkHilControls : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 91;
  MavLinkHilControls() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Control output -1 .. 1
  float roll_ailerons = 0;
  // Control output -1 .. 1
  float pitch_elevator = 0;
  // Control output -1 .. 1
  float yaw_rudder = 0;
  // Throttle 0 .. 1
  float throttle = 0;
  // Aux 1, -1 .. 1
  float aux1 = 0;
  // Aux 2, -1 .. 1
  float aux2 = 0;
  // Aux 3, -1 .. 1
  float aux3 = 0;
  // Aux 4, -1 .. 1
  float aux4 = 0;
  // System mode.
  uint8_t mode = 0;
  // Navigation mode (MAV_NAV_MODE)
  uint8_t nav_mode = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Sent from simulation to autopilot. The RAW values of the RC channels
// received. The standard PPM modulation is as follows: 1000 microseconds: 0%,
// 2000 microseconds: 100%. Individual receivers/transmitters might violate this
// specification.
class MavLinkHilRcInputsRaw : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 92;
  MavLinkHilRcInputsRaw() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // RC channel 1 value
  uint16_t chan1_raw = 0;
  // RC channel 2 value
  uint16_t chan2_raw = 0;
  // RC channel 3 value
  uint16_t chan3_raw = 0;
  // RC channel 4 value
  uint16_t chan4_raw = 0;
  // RC channel 5 value
  uint16_t chan5_raw = 0;
  // RC channel 6 value
  uint16_t chan6_raw = 0;
  // RC channel 7 value
  uint16_t chan7_raw = 0;
  // RC channel 8 value
  uint16_t chan8_raw = 0;
  // RC channel 9 value
  uint16_t chan9_raw = 0;
  // RC channel 10 value
  uint16_t chan10_raw = 0;
  // RC channel 11 value
  uint16_t chan11_raw = 0;
  // RC channel 12 value
  uint16_t chan12_raw = 0;
  // Receive signal strength indicator in device-dependent units/scale. Values:
  // [0-254], UINT8_MAX: invalid/unknown.
  uint8_t rssi = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Sent from autopilot to simulation. Hardware in the loop control outputs
// (replacement for HIL_CONTROLS)
class MavLinkHilActuatorControls : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 93;
  MavLinkHilActuatorControls() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Control outputs -1 .. 1. Channel assignment depends on the simulated
  // hardware.
  float controls[16] = {0};
  // System mode. Includes arming state.
  uint8_t mode = 0;
  // Flags as bitfield, 1: indicate simulation using lockstep.
  uint64_t flags = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Optical flow from a flow sensor (e.g. optical mouse sensor)
class MavLinkOpticalFlow : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 100;
  MavLinkOpticalFlow() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Sensor ID
  uint8_t sensor_id = 0;
  // Flow in x-sensor direction
  int16_t flow_x = 0;
  // Flow in y-sensor direction
  int16_t flow_y = 0;
  // Flow in x-sensor direction, angular-speed compensated
  float flow_comp_m_x = 0;
  // Flow in y-sensor direction, angular-speed compensated
  float flow_comp_m_y = 0;
  // Optical flow quality / confidence. 0: bad, 255: maximum quality
  uint8_t quality = 0;
  // Ground distance. Positive value: distance known. Negative value: Unknown
  // distance
  float ground_distance = 0;
  // Flow rate about X axis
  float flow_rate_x = 0;
  // Flow rate about Y axis
  float flow_rate_y = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Global position/attitude estimate from a vision source.
class MavLinkGlobalVisionPositionEstimate : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 101;
  MavLinkGlobalVisionPositionEstimate() { msgid = kMessageId; }
  // Timestamp (UNIX time or since system boot)
  uint64_t usec = 0;
  // Global X position
  float x = 0;
  // Global Y position
  float y = 0;
  // Global Z position
  float z = 0;
  // Roll angle
  float roll = 0;
  // Pitch angle
  float pitch = 0;
  // Yaw angle
  float yaw = 0;
  // Row-major representation of pose 6x6 cross-covariance matrix upper right
  // triangle (states: x_global, y_global, z_global, roll, pitch, yaw; first six
  // entries are the first ROW, next five entries are the second ROW, etc.). If
  // unknown, assign NaN value to first element in the array.
  float covariance[21] = {0};
  // Estimate reset counter. This should be incremented when the estimate resets
  // in any of the dimensions (position, velocity, attitude, angular speed).
  // This is designed to be used when e.g an external SLAM system detects a
  // loop-closure and the estimate jumps.
  uint8_t reset_counter = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Local position/attitude estimate from a vision source.
class MavLinkVisionPositionEstimate : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 102;
  MavLinkVisionPositionEstimate() { msgid = kMessageId; }
  // Timestamp (UNIX time or time since system boot)
  uint64_t usec = 0;
  // Local X position
  float x = 0;
  // Local Y position
  float y = 0;
  // Local Z position
  float z = 0;
  // Roll angle
  float roll = 0;
  // Pitch angle
  float pitch = 0;
  // Yaw angle
  float yaw = 0;
  // Row-major representation of pose 6x6 cross-covariance matrix upper right
  // triangle (states: x, y, z, roll, pitch, yaw; first six entries are the
  // first ROW, next five entries are the second ROW, etc.). If unknown, assign
  // NaN value to first element in the array.
  float covariance[21] = {0};
  // Estimate reset counter. This should be incremented when the estimate resets
  // in any of the dimensions (position, velocity, attitude, angular speed).
  // This is designed to be used when e.g an external SLAM system detects a
  // loop-closure and the estimate jumps.
  uint8_t reset_counter = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Speed estimate from a vision source.
class MavLinkVisionSpeedEstimate : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 103;
  MavLinkVisionSpeedEstimate() { msgid = kMessageId; }
  // Timestamp (UNIX time or time since system boot)
  uint64_t usec = 0;
  // Global X speed
  float x = 0;
  // Global Y speed
  float y = 0;
  // Global Z speed
  float z = 0;
  // Row-major representation of 3x3 linear velocity covariance matrix (states:
  // vx, vy, vz; 1st three entries - 1st row, etc.). If unknown, assign NaN
  // value to first element in the array.
  float covariance[9] = {0};
  // Estimate reset counter. This should be incremented when the estimate resets
  // in any of the dimensions (position, velocity, attitude, angular speed).
  // This is designed to be used when e.g an external SLAM system detects a
  // loop-closure and the estimate jumps.
  uint8_t reset_counter = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Global position estimate from a Vicon motion system source.
class MavLinkViconPositionEstimate : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 104;
  MavLinkViconPositionEstimate() { msgid = kMessageId; }
  // Timestamp (UNIX time or time since system boot)
  uint64_t usec = 0;
  // Global X position
  float x = 0;
  // Global Y position
  float y = 0;
  // Global Z position
  float z = 0;
  // Roll angle
  float roll = 0;
  // Pitch angle
  float pitch = 0;
  // Yaw angle
  float yaw = 0;
  // Row-major representation of 6x6 pose cross-covariance matrix upper right
  // triangle (states: x, y, z, roll, pitch, yaw; first six entries are the
  // first ROW, next five entries are the second ROW, etc.). If unknown, assign
  // NaN value to first element in the array.
  float covariance[21] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The IMU readings in SI units in NED body frame
class MavLinkHighresImu : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 105;
  MavLinkHighresImu() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // X acceleration
  float xacc = 0;
  // Y acceleration
  float yacc = 0;
  // Z acceleration
  float zacc = 0;
  // Angular speed around X axis
  float xgyro = 0;
  // Angular speed around Y axis
  float ygyro = 0;
  // Angular speed around Z axis
  float zgyro = 0;
  // X Magnetic field
  float xmag = 0;
  // Y Magnetic field
  float ymag = 0;
  // Z Magnetic field
  float zmag = 0;
  // Absolute pressure
  float abs_pressure = 0;
  // Differential pressure
  float diff_pressure = 0;
  // Altitude calculated from pressure
  float pressure_alt = 0;
  // Temperature
  float temperature = 0;
  // Bitmap for fields that have updated since last message
  uint16_t fields_updated = 0;
  // Id. Ids are numbered from 0 and map to IMUs numbered from 1 (e.g. IMU1 will
  // have a message with id=0)
  uint8_t id = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Optical flow from an angular rate flow sensor (e.g. PX4FLOW or mouse sensor)
class MavLinkOpticalFlowRad : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 106;
  MavLinkOpticalFlowRad() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Sensor ID
  uint8_t sensor_id = 0;
  // Integration time. Divide integrated_x and integrated_y by the integration
  // time to obtain average flow. The integration time also indicates the.
  uint32_t integration_time_us = 0;
  // Flow around X axis (Sensor RH rotation about the X axis induces a positive
  // flow. Sensor linear motion along the positive Y axis induces a negative
  // flow.)
  float integrated_x = 0;
  // Flow around Y axis (Sensor RH rotation about the Y axis induces a positive
  // flow. Sensor linear motion along the positive X axis induces a positive
  // flow.)
  float integrated_y = 0;
  // RH rotation around X axis
  float integrated_xgyro = 0;
  // RH rotation around Y axis
  float integrated_ygyro = 0;
  // RH rotation around Z axis
  float integrated_zgyro = 0;
  // Temperature
  int16_t temperature = 0;
  // Optical flow quality / confidence. 0: no valid flow, 255: maximum quality
  uint8_t quality = 0;
  // Time since the distance was sampled.
  uint32_t time_delta_distance_us = 0;
  // Distance to the center of the flow field. Positive value (including zero):
  // distance known. Negative value: Unknown distance.
  float distance = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The IMU readings in SI units in NED body frame
class MavLinkHilSensor : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 107;
  MavLinkHilSensor() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // X acceleration
  float xacc = 0;
  // Y acceleration
  float yacc = 0;
  // Z acceleration
  float zacc = 0;
  // Angular speed around X axis in body frame
  float xgyro = 0;
  // Angular speed around Y axis in body frame
  float ygyro = 0;
  // Angular speed around Z axis in body frame
  float zgyro = 0;
  // X Magnetic field
  float xmag = 0;
  // Y Magnetic field
  float ymag = 0;
  // Z Magnetic field
  float zmag = 0;
  // Absolute pressure
  float abs_pressure = 0;
  // Differential pressure (airspeed)
  float diff_pressure = 0;
  // Altitude calculated from pressure
  float pressure_alt = 0;
  // Temperature
  float temperature = 0;
  // Bitmap for fields that have updated since last message
  uint32_t fields_updated = 0;
  // Sensor ID (zero indexed). Used for multiple sensor inputs
  uint8_t id = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Status of simulation environment, if used
class MavLinkSimState : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 108;
  MavLinkSimState() { msgid = kMessageId; }
  // True attitude quaternion component 1, w (1 in null-rotation)
  float q1 = 0;
  // True attitude quaternion component 2, x (0 in null-rotation)
  float q2 = 0;
  // True attitude quaternion component 3, y (0 in null-rotation)
  float q3 = 0;
  // True attitude quaternion component 4, z (0 in null-rotation)
  float q4 = 0;
  // Attitude roll expressed as Euler angles, not recommended except for
  // human-readable outputs
  float roll = 0;
  // Attitude pitch expressed as Euler angles, not recommended except for
  // human-readable outputs
  float pitch = 0;
  // Attitude yaw expressed as Euler angles, not recommended except for
  // human-readable outputs
  float yaw = 0;
  // X acceleration
  float xacc = 0;
  // Y acceleration
  float yacc = 0;
  // Z acceleration
  float zacc = 0;
  // Angular speed around X axis
  float xgyro = 0;
  // Angular speed around Y axis
  float ygyro = 0;
  // Angular speed around Z axis
  float zgyro = 0;
  // Latitude
  float lat = 0;
  // Longitude
  float lon = 0;
  // Altitude
  float alt = 0;
  // Horizontal position standard deviation
  float std_dev_horz = 0;
  // Vertical position standard deviation
  float std_dev_vert = 0;
  // True velocity in north direction in earth-fixed NED frame
  float vn = 0;
  // True velocity in east direction in earth-fixed NED frame
  float ve = 0;
  // True velocity in down direction in earth-fixed NED frame
  float vd = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Status generated by radio and injected into MAVLink stream.
class MavLinkRadioStatus : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 109;
  MavLinkRadioStatus() { msgid = kMessageId; }
  // Local (message sender) recieved signal strength indication in
  // device-dependent units/scale. Values: [0-254], UINT8_MAX: invalid/unknown.
  uint8_t rssi = 0;
  // Remote (message receiver) signal strength indication in device-dependent
  // units/scale. Values: [0-254], UINT8_MAX: invalid/unknown.
  uint8_t remrssi = 0;
  // Remaining free transmitter buffer space.
  uint8_t txbuf = 0;
  // Local background noise level. These are device dependent RSSI values (scale
  // as approx 2x dB on SiK radios). Values: [0-254], UINT8_MAX:
  // invalid/unknown.
  uint8_t noise = 0;
  // Remote background noise level. These are device dependent RSSI values
  // (scale as approx 2x dB on SiK radios). Values: [0-254], UINT8_MAX:
  // invalid/unknown.
  uint8_t remnoise = 0;
  // Count of radio packet receive errors (since boot).
  uint16_t rxerrors = 0;
  // Count of error corrected radio packets (since boot).
  uint16_t fixed = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// File transfer protocol message: https://mavlink.io/en/services/ftp.html.
class MavLinkFileTransferProtocol : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 110;
  MavLinkFileTransferProtocol() { msgid = kMessageId; }
  // Network ID (0 for broadcast)
  uint8_t target_network = 0;
  // System ID (0 for broadcast)
  uint8_t target_system = 0;
  // Component ID (0 for broadcast)
  uint8_t target_component = 0;
  // Variable length payload. The length is defined by the remaining message
  // length when subtracting the header and other fields. The content/format of
  // this block is defined in https://mavlink.io/en/services/ftp.html.
  uint8_t payload[251] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Time synchronization message.
class MavLinkTimesync : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 111;
  MavLinkTimesync() { msgid = kMessageId; }
  // Time sync timestamp 1
  int64_t tc1 = 0;
  // Time sync timestamp 2
  int64_t ts1 = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Camera-IMU triggering and synchronisation message.
class MavLinkCameraTrigger : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 112;
  MavLinkCameraTrigger() { msgid = kMessageId; }
  // Timestamp for image frame (UNIX Epoch time or time since system boot). The
  // receiving end can infer timestamp format (since 1.1.1970 or since system
  // boot) by checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Image frame sequence
  uint32_t seq = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The global position, as returned by the Global Positioning System (GPS). This
// is NOT the global position estimate of the sytem, but rather a RAW sensor
// value. See message GLOBAL_POSITION_INT for the global position estimate.
class MavLinkHilGps : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 113;
  MavLinkHilGps() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // 0-1: no fix, 2: 2D fix, 3: 3D fix. Some applications will not use the value
  // of this field unless it is at least two, so always correctly fill in the
  // fix.
  uint8_t fix_type = 0;
  // Latitude (WGS84)
  int32_t lat = 0;
  // Longitude (WGS84)
  int32_t lon = 0;
  // Altitude (MSL). Positive for up.
  int32_t alt = 0;
  // GPS HDOP horizontal dilution of position (unitless * 100). If unknown, set
  // to: UINT16_MAX
  uint16_t eph = 0;
  // GPS VDOP vertical dilution of position (unitless * 100). If unknown, set
  // to: UINT16_MAX
  uint16_t epv = 0;
  // GPS ground speed. If unknown, set to: UINT16_MAX
  uint16_t vel = 0;
  // GPS velocity in north direction in earth-fixed NED frame
  int16_t vn = 0;
  // GPS velocity in east direction in earth-fixed NED frame
  int16_t ve = 0;
  // GPS velocity in down direction in earth-fixed NED frame
  int16_t vd = 0;
  // Course over ground (NOT heading, but direction of movement), 0.0..359.99
  // degrees. If unknown, set to: UINT16_MAX
  uint16_t cog = 0;
  // Number of satellites visible. If unknown, set to UINT8_MAX
  uint8_t satellites_visible = 0;
  // GPS ID (zero indexed). Used for multiple GPS inputs
  uint8_t id = 0;
  // Yaw of vehicle relative to Earth's North, zero means not available, use
  // 36000 for north
  uint16_t yaw = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Simulated optical flow from a flow sensor (e.g. PX4FLOW or optical mouse
// sensor)
class MavLinkHilOpticalFlow : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 114;
  MavLinkHilOpticalFlow() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Sensor ID
  uint8_t sensor_id = 0;
  // Integration time. Divide integrated_x and integrated_y by the integration
  // time to obtain average flow. The integration time also indicates the.
  uint32_t integration_time_us = 0;
  // Flow in radians around X axis (Sensor RH rotation about the X axis induces
  // a positive flow. Sensor linear motion along the positive Y axis induces a
  // negative flow.)
  float integrated_x = 0;
  // Flow in radians around Y axis (Sensor RH rotation about the Y axis induces
  // a positive flow. Sensor linear motion along the positive X axis induces a
  // positive flow.)
  float integrated_y = 0;
  // RH rotation around X axis
  float integrated_xgyro = 0;
  // RH rotation around Y axis
  float integrated_ygyro = 0;
  // RH rotation around Z axis
  float integrated_zgyro = 0;
  // Temperature
  int16_t temperature = 0;
  // Optical flow quality / confidence. 0: no valid flow, 255: maximum quality
  uint8_t quality = 0;
  // Time since the distance was sampled.
  uint32_t time_delta_distance_us = 0;
  // Distance to the center of the flow field. Positive value (including zero):
  // distance known. Negative value: Unknown distance.
  float distance = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Sent from simulation to autopilot, avoids in contrast to HIL_STATE
// singularities. This packet is useful for high throughput applications such as
// hardware in the loop simulations.
class MavLinkHilStateQuaternion : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 115;
  MavLinkHilStateQuaternion() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Vehicle attitude expressed as normalized quaternion in w, x, y, z order
  // (with 1 0 0 0 being the null-rotation)
  float attitude_quaternion[4] = {0};
  // Body frame roll / phi angular speed
  float rollspeed = 0;
  // Body frame pitch / theta angular speed
  float pitchspeed = 0;
  // Body frame yaw / psi angular speed
  float yawspeed = 0;
  // Latitude
  int32_t lat = 0;
  // Longitude
  int32_t lon = 0;
  // Altitude
  int32_t alt = 0;
  // Ground X Speed (Latitude)
  int16_t vx = 0;
  // Ground Y Speed (Longitude)
  int16_t vy = 0;
  // Ground Z Speed (Altitude)
  int16_t vz = 0;
  // Indicated airspeed
  uint16_t ind_airspeed = 0;
  // True airspeed
  uint16_t true_airspeed = 0;
  // X acceleration
  int16_t xacc = 0;
  // Y acceleration
  int16_t yacc = 0;
  // Z acceleration
  int16_t zacc = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The RAW IMU readings for secondary 9DOF sensor setup. This message should
// contain the scaled values to the described units
class MavLinkScaledImu2 : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 116;
  MavLinkScaledImu2() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // X acceleration
  int16_t xacc = 0;
  // Y acceleration
  int16_t yacc = 0;
  // Z acceleration
  int16_t zacc = 0;
  // Angular speed around X axis
  int16_t xgyro = 0;
  // Angular speed around Y axis
  int16_t ygyro = 0;
  // Angular speed around Z axis
  int16_t zgyro = 0;
  // X Magnetic field
  int16_t xmag = 0;
  // Y Magnetic field
  int16_t ymag = 0;
  // Z Magnetic field
  int16_t zmag = 0;
  // Temperature, 0: IMU does not provide temperature values. If the IMU is at
  // 0C it must send 1 (0.01C).
  int16_t temperature = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request a list of available logs. On some systems calling this may stop
// on-board logging until LOG_REQUEST_END is called. If there are no log files
// available this request shall be answered with one LOG_ENTRY message with id =
// 0 and num_logs = 0.
class MavLinkLogRequestList : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 117;
  MavLinkLogRequestList() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // First log id (0 for first available)
  uint16_t start = 0;
  // Last log id (0xffff for last available)
  uint16_t end = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Reply to LOG_REQUEST_LIST
class MavLinkLogEntry : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 118;
  MavLinkLogEntry() { msgid = kMessageId; }
  // Log id
  uint16_t id = 0;
  // Total number of logs
  uint16_t num_logs = 0;
  // High log number
  uint16_t last_log_num = 0;
  // UTC timestamp of log since 1970, or 0 if not available
  uint32_t time_utc = 0;
  // Size of the log (may be approximate)
  uint32_t size = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request a chunk of a log
class MavLinkLogRequestData : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 119;
  MavLinkLogRequestData() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Log id (from LOG_ENTRY reply)
  uint16_t id = 0;
  // Offset into the log
  uint32_t ofs = 0;
  // Number of bytes
  uint32_t count = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Reply to LOG_REQUEST_DATA
class MavLinkLogData : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 120;
  MavLinkLogData() { msgid = kMessageId; }
  // Log id (from LOG_ENTRY reply)
  uint16_t id = 0;
  // Offset into the log
  uint32_t ofs = 0;
  // Number of bytes (zero for end of log)
  uint8_t count = 0;
  // log data
  uint8_t data[90] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Erase all logs
class MavLinkLogErase : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 121;
  MavLinkLogErase() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Stop log transfer and resume normal logging
class MavLinkLogRequestEnd : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 122;
  MavLinkLogRequestEnd() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Data for injecting into the onboard GPS (used for DGPS)
class MavLinkGpsInjectData : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 123;
  MavLinkGpsInjectData() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Data length
  uint8_t len = 0;
  // Raw data (110 is enough for 12 satellites of RTCMv2)
  uint8_t data[110] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Second GPS data.
class MavLinkGps2Raw : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 124;
  MavLinkGps2Raw() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // GPS fix type.
  uint8_t fix_type = 0;
  // Latitude (WGS84)
  int32_t lat = 0;
  // Longitude (WGS84)
  int32_t lon = 0;
  // Altitude (MSL). Positive for up.
  int32_t alt = 0;
  // GPS HDOP horizontal dilution of position (unitless * 100). If unknown, set
  // to: UINT16_MAX
  uint16_t eph = 0;
  // GPS VDOP vertical dilution of position (unitless * 100). If unknown, set
  // to: UINT16_MAX
  uint16_t epv = 0;
  // GPS ground speed. If unknown, set to: UINT16_MAX
  uint16_t vel = 0;
  // Course over ground (NOT heading, but direction of movement): 0.0..359.99
  // degrees. If unknown, set to: UINT16_MAX
  uint16_t cog = 0;
  // Number of satellites visible. If unknown, set to UINT8_MAX
  uint8_t satellites_visible = 0;
  // Number of DGPS satellites
  uint8_t dgps_numch = 0;
  // Age of DGPS info
  uint32_t dgps_age = 0;
  // Yaw in earth frame from north. Use 0 if this GPS does not provide yaw. Use
  // UINT16_MAX if this GPS is configured to provide yaw and is currently unable
  // to provide it. Use 36000 for north.
  uint16_t yaw = 0;
  // Altitude (above WGS84, EGM96 ellipsoid). Positive for up.
  int32_t alt_ellipsoid = 0;
  // Position uncertainty.
  uint32_t h_acc = 0;
  // Altitude uncertainty.
  uint32_t v_acc = 0;
  // Speed uncertainty.
  uint32_t vel_acc = 0;
  // Heading / track uncertainty
  uint32_t hdg_acc = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Power supply status
class MavLinkPowerStatus : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 125;
  MavLinkPowerStatus() { msgid = kMessageId; }
  // 5V rail voltage.
  uint16_t Vcc = 0;
  // Servo rail voltage.
  uint16_t Vservo = 0;
  // Bitmap of power supply status flags.
  uint16_t flags = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Control a serial port. This can be used for raw access to an onboard serial
// peripheral such as a GPS or telemetry radio. It is designed to make it
// possible to update the devices firmware via MAVLink messages or change the
// devices settings. A message with zero bytes can be used to change just the
// baudrate.
class MavLinkSerialControl : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 126;
  MavLinkSerialControl() { msgid = kMessageId; }
  // Serial control device type.
  uint8_t device = 0;
  // Bitmap of serial control flags.
  uint8_t flags = 0;
  // Timeout for reply data
  uint16_t timeout = 0;
  // Baudrate of transfer. Zero means no change.
  uint32_t baudrate = 0;
  // how many bytes in this transfer
  uint8_t count = 0;
  // serial data
  uint8_t data[70] = {0};
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// RTK GPS data. Gives information on the relative baseline calculation the GPS
// is reporting
class MavLinkGpsRtk : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 127;
  MavLinkGpsRtk() { msgid = kMessageId; }
  // Time since boot of last baseline message received.
  uint32_t time_last_baseline_ms = 0;
  // Identification of connected RTK receiver.
  uint8_t rtk_receiver_id = 0;
  // GPS Week Number of last baseline
  uint16_t wn = 0;
  // GPS Time of Week of last baseline
  uint32_t tow = 0;
  // GPS-specific health report for RTK data.
  uint8_t rtk_health = 0;
  // Rate of baseline messages being received by GPS
  uint8_t rtk_rate = 0;
  // Current number of sats used for RTK calculation.
  uint8_t nsats = 0;
  // Coordinate system of baseline
  uint8_t baseline_coords_type = 0;
  // Current baseline in ECEF x or NED north component.
  int32_t baseline_a_mm = 0;
  // Current baseline in ECEF y or NED east component.
  int32_t baseline_b_mm = 0;
  // Current baseline in ECEF z or NED down component.
  int32_t baseline_c_mm = 0;
  // Current estimate of baseline accuracy.
  uint32_t accuracy = 0;
  // Current number of integer ambiguity hypotheses.
  int32_t iar_num_hypotheses = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// RTK GPS data. Gives information on the relative baseline calculation the GPS
// is reporting
class MavLinkGps2Rtk : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 128;
  MavLinkGps2Rtk() { msgid = kMessageId; }
  // Time since boot of last baseline message received.
  uint32_t time_last_baseline_ms = 0;
  // Identification of connected RTK receiver.
  uint8_t rtk_receiver_id = 0;
  // GPS Week Number of last baseline
  uint16_t wn = 0;
  // GPS Time of Week of last baseline
  uint32_t tow = 0;
  // GPS-specific health report for RTK data.
  uint8_t rtk_health = 0;
  // Rate of baseline messages being received by GPS
  uint8_t rtk_rate = 0;
  // Current number of sats used for RTK calculation.
  uint8_t nsats = 0;
  // Coordinate system of baseline
  uint8_t baseline_coords_type = 0;
  // Current baseline in ECEF x or NED north component.
  int32_t baseline_a_mm = 0;
  // Current baseline in ECEF y or NED east component.
  int32_t baseline_b_mm = 0;
  // Current baseline in ECEF z or NED down component.
  int32_t baseline_c_mm = 0;
  // Current estimate of baseline accuracy.
  uint32_t accuracy = 0;
  // Current number of integer ambiguity hypotheses.
  int32_t iar_num_hypotheses = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The RAW IMU readings for 3rd 9DOF sensor setup. This message should contain
// the scaled values to the described units
class MavLinkScaledImu3 : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 129;
  MavLinkScaledImu3() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // X acceleration
  int16_t xacc = 0;
  // Y acceleration
  int16_t yacc = 0;
  // Z acceleration
  int16_t zacc = 0;
  // Angular speed around X axis
  int16_t xgyro = 0;
  // Angular speed around Y axis
  int16_t ygyro = 0;
  // Angular speed around Z axis
  int16_t zgyro = 0;
  // X Magnetic field
  int16_t xmag = 0;
  // Y Magnetic field
  int16_t ymag = 0;
  // Z Magnetic field
  int16_t zmag = 0;
  // Temperature, 0: IMU does not provide temperature values. If the IMU is at
  // 0C it must send 1 (0.01C).
  int16_t temperature = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Handshake message to initiate, control and stop image streaming when using
// the Image Transmission Protocol:
// https://mavlink.io/en/services/image_transmission.html.
class MavLinkDataTransmissionHandshake : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 130;
  MavLinkDataTransmissionHandshake() { msgid = kMessageId; }
  // Type of requested/acknowledged data.
  uint8_t type = 0;
  // total data size (set on ACK only).
  uint32_t size = 0;
  // Width of a matrix or image.
  uint16_t width = 0;
  // Height of a matrix or image.
  uint16_t height = 0;
  // Number of packets being sent (set on ACK only).
  uint16_t packets = 0;
  // Payload size per packet (normally 253 byte, see DATA field size in message
  // ENCAPSULATED_DATA) (set on ACK only).
  uint8_t payload = 0;
  // JPEG quality. Values: [1-100].
  uint8_t jpg_quality = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Data packet for images sent using the Image Transmission Protocol:
// https://mavlink.io/en/services/image_transmission.html.
class MavLinkEncapsulatedData : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 131;
  MavLinkEncapsulatedData() { msgid = kMessageId; }
  // sequence number (starting with 0 on every transmission)
  uint16_t seqnr = 0;
  // image data bytes
  uint8_t data[253] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Distance sensor information for an onboard rangefinder.
class MavLinkDistanceSensor : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 132;
  MavLinkDistanceSensor() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Minimum distance the sensor can measure
  uint16_t min_distance = 0;
  // Maximum distance the sensor can measure
  uint16_t max_distance = 0;
  // Current distance reading
  uint16_t current_distance = 0;
  // Type of distance sensor.
  uint8_t type = 0;
  // Onboard ID of the sensor
  uint8_t id = 0;
  // Direction the sensor faces. downward-facing: ROTATION_PITCH_270,
  // upward-facing: ROTATION_PITCH_90, backward-facing: ROTATION_PITCH_180,
  // forward-facing: ROTATION_NONE, left-facing: ROTATION_YAW_90, right-facing:
  // ROTATION_YAW_270
  uint8_t orientation = 0;
  // Measurement variance. Max standard deviation is 6cm. UINT8_MAX if unknown.
  uint8_t covariance = 0;
  // Horizontal Field of View (angle) where the distance measurement is valid
  // and the field of view is known. Otherwise this is set to 0.
  float horizontal_fov = 0;
  // Vertical Field of View (angle) where the distance measurement is valid and
  // the field of view is known. Otherwise this is set to 0.
  float vertical_fov = 0;
  // Quaternion of the sensor orientation in vehicle body frame (w, x, y, z
  // order, zero-rotation is 1, 0, 0, 0). Zero-rotation is along the vehicle
  // body x-axis. This field is required if the orientation is set to
  // MAV_SENSOR_ROTATION_CUSTOM. Set it to 0 if invalid."
  float quaternion[4] = {0};
  // Signal quality of the sensor. Specific to each sensor type, representing
  // the relation of the signal strength with the target reflectivity, distance,
  // size or aspect, but normalised as a percentage. 0 = unknown/unset signal
  // quality, 1 = invalid signal, 100 = perfect signal.
  uint8_t signal_quality = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request for terrain data and terrain status. See terrain protocol docs:
// https://mavlink.io/en/services/terrain.html
class MavLinkTerrainRequest : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 133;
  MavLinkTerrainRequest() { msgid = kMessageId; }
  // Latitude of SW corner of first grid
  int32_t lat = 0;
  // Longitude of SW corner of first grid
  int32_t lon = 0;
  // Grid spacing
  uint16_t grid_spacing = 0;
  // Bitmask of requested 4x4 grids (row major 8x7 array of grids, 56 bits)
  uint64_t mask = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Terrain data sent from GCS. The lat/lon and grid_spacing must be the same as
// a lat/lon from a TERRAIN_REQUEST. See terrain protocol docs:
// https://mavlink.io/en/services/terrain.html
class MavLinkTerrainData : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 134;
  MavLinkTerrainData() { msgid = kMessageId; }
  // Latitude of SW corner of first grid
  int32_t lat = 0;
  // Longitude of SW corner of first grid
  int32_t lon = 0;
  // Grid spacing
  uint16_t grid_spacing = 0;
  // bit within the terrain request mask
  uint8_t gridbit = 0;
  // Terrain data MSL
  int16_t data[16] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request that the vehicle report terrain height at the given location
// (expected response is a TERRAIN_REPORT). Used by GCS to check if vehicle has
// all terrain data needed for a mission.
class MavLinkTerrainCheck : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 135;
  MavLinkTerrainCheck() { msgid = kMessageId; }
  // Latitude
  int32_t lat = 0;
  // Longitude
  int32_t lon = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Streamed from drone to report progress of terrain map download (initiated by
// TERRAIN_REQUEST), or sent as a response to a TERRAIN_CHECK request. See
// terrain protocol docs: https://mavlink.io/en/services/terrain.html
class MavLinkTerrainReport : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 136;
  MavLinkTerrainReport() { msgid = kMessageId; }
  // Latitude
  int32_t lat = 0;
  // Longitude
  int32_t lon = 0;
  // grid spacing (zero if terrain at this location unavailable)
  uint16_t spacing = 0;
  // Terrain height MSL
  float terrain_height = 0;
  // Current vehicle height above lat/lon terrain height
  float current_height = 0;
  // Number of 4x4 terrain blocks waiting to be received or read from disk
  uint16_t pending = 0;
  // Number of 4x4 terrain blocks in memory
  uint16_t loaded = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Barometer readings for 2nd barometer
class MavLinkScaledPressure2 : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 137;
  MavLinkScaledPressure2() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Absolute pressure
  float press_abs = 0;
  // Differential pressure
  float press_diff = 0;
  // Absolute pressure temperature
  int16_t temperature = 0;
  // Differential pressure temperature (0, if not available). Report values of 0
  // (or 1) as 1 cdegC.
  int16_t temperature_press_diff = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Motion capture attitude and position
class MavLinkAttPosMocap : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 138;
  MavLinkAttPosMocap() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Attitude quaternion (w, x, y, z order, zero-rotation is 1, 0, 0, 0)
  float q[4] = {0};
  // X position (NED)
  float x = 0;
  // Y position (NED)
  float y = 0;
  // Z position (NED)
  float z = 0;
  // Row-major representation of a pose 6x6 cross-covariance matrix upper right
  // triangle (states: x, y, z, roll, pitch, yaw; first six entries are the
  // first ROW, next five entries are the second ROW, etc.). If unknown, assign
  // NaN value to first element in the array.
  float covariance[21] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Set the vehicle attitude and body angular rates.
class MavLinkSetActuatorControlTarget : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 139;
  MavLinkSetActuatorControlTarget() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Actuator group. The "_mlx" indicates this is a multi-instance message and a
  // MAVLink parser should use this field to difference between instances.
  uint8_t group_mlx = 0;
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Actuator controls. Normed to -1..+1 where 0 is neutral position. Throttle
  // for single rotation direction motors is 0..1, negative range for reverse
  // direction. Standard mapping for attitude controls (group 0): (index 0-7):
  // roll, pitch, yaw, throttle, flaps, spoilers, airbrakes, landing gear. Load
  // a pass-through mixer to repurpose them as generic outputs.
  float controls[8] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Set the vehicle attitude and body angular rates.
class MavLinkActuatorControlTarget : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 140;
  MavLinkActuatorControlTarget() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Actuator group. The "_mlx" indicates this is a multi-instance message and a
  // MAVLink parser should use this field to difference between instances.
  uint8_t group_mlx = 0;
  // Actuator controls. Normed to -1..+1 where 0 is neutral position. Throttle
  // for single rotation direction motors is 0..1, negative range for reverse
  // direction. Standard mapping for attitude controls (group 0): (index 0-7):
  // roll, pitch, yaw, throttle, flaps, spoilers, airbrakes, landing gear. Load
  // a pass-through mixer to repurpose them as generic outputs.
  float controls[8] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The current system altitude.
class MavLinkAltitude : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 141;
  MavLinkAltitude() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // This altitude measure is initialized on system boot and monotonic (it is
  // never reset, but represents the local altitude change). The only guarantee
  // on this field is that it will never be reset and is consistent within a
  // flight. The recommended value for this field is the uncorrected barometric
  // altitude at boot time. This altitude will also drift and vary between
  // flights.
  float altitude_monotonic = 0;
  // This altitude measure is strictly above mean sea level and might be
  // non-monotonic (it might reset on events like GPS lock or when a new QNH
  // value is set). It should be the altitude to which global altitude waypoints
  // are compared to. Note that it is *not* the GPS altitude, however, most GPS
  // modules already output MSL by default and not the WGS84 altitude.
  float altitude_amsl = 0;
  // This is the local altitude in the local coordinate frame. It is not the
  // altitude above home, but in reference to the coordinate origin (0, 0, 0).
  // It is up-positive.
  float altitude_local = 0;
  // This is the altitude above the home position. It resets on each change of
  // the current home position.
  float altitude_relative = 0;
  // This is the altitude above terrain. It might be fed by a terrain database
  // or an altimeter. Values smaller than -1000 should be interpreted as
  // unknown.
  float altitude_terrain = 0;
  // This is not the altitude, but the clear space below the system according to
  // the fused clearance estimate. It generally should max out at the maximum
  // range of e.g. the laser altimeter. It is generally a moving target. A
  // negative value indicates no measurement available.
  float bottom_clearance = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The autopilot is requesting a resource (file, binary, other type of data)
class MavLinkResourceRequest : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 142;
  MavLinkResourceRequest() { msgid = kMessageId; }
  // Request ID. This ID should be re-used when sending back URI contents
  uint8_t request_id = 0;
  // The type of requested URI. 0 = a file via URL. 1 = a UAVCAN binary
  uint8_t uri_type = 0;
  // The requested unique resource identifier (URI). It is not necessarily a
  // straight domain name (depends on the URI type enum)
  uint8_t uri[120] = {0};
  // The way the autopilot wants to receive the URI. 0 = MAVLink FTP. 1 = binary
  // stream.
  uint8_t transfer_type = 0;
  // The storage path the autopilot wants the URI to be stored in. Will only be
  // valid if the transfer_type has a storage associated (e.g. MAVLink FTP).
  uint8_t storage[120] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Barometer readings for 3rd barometer
class MavLinkScaledPressure3 : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 143;
  MavLinkScaledPressure3() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Absolute pressure
  float press_abs = 0;
  // Differential pressure
  float press_diff = 0;
  // Absolute pressure temperature
  int16_t temperature = 0;
  // Differential pressure temperature (0, if not available). Report values of 0
  // (or 1) as 1 cdegC.
  int16_t temperature_press_diff = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Current motion information from a designated system
class MavLinkFollowTarget : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 144;
  MavLinkFollowTarget() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint64_t timestamp = 0;
  // bit positions for tracker reporting capabilities (POS = 0, VEL = 1, ACCEL =
  // 2, ATT + RATES = 3)
  uint8_t est_capabilities = 0;
  // Latitude (WGS84)
  int32_t lat = 0;
  // Longitude (WGS84)
  int32_t lon = 0;
  // Altitude (MSL)
  float alt = 0;
  // target velocity (0,0,0) for unknown
  float vel[3] = {0};
  // linear target acceleration (0,0,0) for unknown
  float acc[3] = {0};
  // (0 0 0 0 for unknown)
  float attitude_q[4] = {0};
  // (0 0 0 for unknown)
  float rates[3] = {0};
  // eph epv
  float position_cov[3] = {0};
  // button states or switches of a tracker device
  uint64_t custom_state = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The smoothed, monotonic system state used to feed the control loops of the
// system.
class MavLinkControlSystemState : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 146;
  MavLinkControlSystemState() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // X acceleration in body frame
  float x_acc = 0;
  // Y acceleration in body frame
  float y_acc = 0;
  // Z acceleration in body frame
  float z_acc = 0;
  // X velocity in body frame
  float x_vel = 0;
  // Y velocity in body frame
  float y_vel = 0;
  // Z velocity in body frame
  float z_vel = 0;
  // X position in local frame
  float x_pos = 0;
  // Y position in local frame
  float y_pos = 0;
  // Z position in local frame
  float z_pos = 0;
  // Airspeed, set to -1 if unknown
  float airspeed = 0;
  // Variance of body velocity estimate
  float vel_variance[3] = {0};
  // Variance in local position
  float pos_variance[3] = {0};
  // The attitude, represented as Quaternion
  float q[4] = {0};
  // Angular rate in roll axis
  float roll_rate = 0;
  // Angular rate in pitch axis
  float pitch_rate = 0;
  // Angular rate in yaw axis
  float yaw_rate = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Battery information. Updates GCS with flight controller battery status. Smart
// batteries also use this message, but may additionally send
// SMART_BATTERY_INFO.
class MavLinkBatteryStatus : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 147;
  MavLinkBatteryStatus() { msgid = kMessageId; }
  // Battery ID
  uint8_t id = 0;
  // Function of the battery
  uint8_t battery_function = 0;
  // Type (chemistry) of the battery
  uint8_t type = 0;
  // Temperature of the battery. INT16_MAX for unknown temperature.
  int16_t temperature = 0;
  // Battery voltage of cells 1 to 10 (see voltages_ext for cells 11-14). Cells
  // in this field above the valid cell count for this battery should have the
  // UINT16_MAX value. If individual cell voltages are unknown or not measured
  // for this battery, then the overall battery voltage should be filled in cell
  // 0, with all others set to UINT16_MAX. If the voltage of the battery is
  // greater than (UINT16_MAX
  // - 1), then cell 0 should be set to (UINT16_MAX - 1), and cell 1 to the
  // remaining voltage. This can be extended to multiple cells if the total
  // voltage is greater than 2 * (UINT16_MAX - 1).
  uint16_t voltages[10] = {0};
  // Battery current, -1: autopilot does not measure the current
  int16_t current_battery = 0;
  // Consumed charge, -1: autopilot does not provide consumption estimate
  int32_t current_consumed = 0;
  // Consumed energy, -1: autopilot does not provide energy consumption estimate
  int32_t energy_consumed = 0;
  // Remaining battery energy. Values: [0-100], -1: autopilot does not estimate
  // the remaining battery.
  int8_t battery_remaining = 0;
  // Remaining battery time, 0: autopilot does not provide remaining battery
  // time estimate
  int32_t time_remaining = 0;
  // State for extent of discharge, provided by autopilot for warning or
  // external reactions
  uint8_t charge_state = 0;
  // Battery voltages for cells 11 to 14. Cells above the valid cell count for
  // this battery should have a value of 0, where zero indicates not supported
  // (note, this is different than for the voltages field and allows empty byte
  // truncation). If the measured value is 0 then 1 should be sent instead.
  uint16_t voltages_ext[4] = {0};
  // Battery mode. Default (0) is that battery mode reporting is not supported
  // or battery is in normal-use mode.
  uint8_t mode = 0;
  // Fault/health indications. These should be set when charge_state is
  // MAV_BATTERY_CHARGE_STATE_FAILED or MAV_BATTERY_CHARGE_STATE_UNHEALTHY (if
  // not, fault reporting is not supported).
  uint32_t fault_bitmask = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Version and capability of autopilot software. This should be emitted in
// response to a request with MAV_CMD_REQUEST_MESSAGE.
class MavLinkAutopilotVersion : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 148;
  MavLinkAutopilotVersion() { msgid = kMessageId; }
  // Bitmap of capabilities
  uint64_t capabilities = 0;
  // Firmware version number
  uint32_t flight_sw_version = 0;
  // Middleware version number
  uint32_t middleware_sw_version = 0;
  // Operating system version number
  uint32_t os_sw_version = 0;
  // HW / board version (last 8 bits should be silicon ID, if any). The first 16
  // bits of this field specify
  // https://github.com/PX4/PX4-Bootloader/blob/master/board_types.txt
  uint32_t board_version = 0;
  // Custom version field, commonly the first 8 bytes of the git hash. This is
  // not an unique identifier, but should allow to identify the commit using the
  // main version number even for very large code bases.
  uint8_t flight_custom_version[8] = {0};
  // Custom version field, commonly the first 8 bytes of the git hash. This is
  // not an unique identifier, but should allow to identify the commit using the
  // main version number even for very large code bases.
  uint8_t middleware_custom_version[8] = {0};
  // Custom version field, commonly the first 8 bytes of the git hash. This is
  // not an unique identifier, but should allow to identify the commit using the
  // main version number even for very large code bases.
  uint8_t os_custom_version[8] = {0};
  // ID of the board vendor
  uint16_t vendor_id = 0;
  // ID of the product
  uint16_t product_id = 0;
  // UID if provided by hardware (see uid2)
  uint64_t uid = 0;
  // UID if provided by hardware (supersedes the uid field. If this is non-zero,
  // use this field, otherwise use uid)
  uint8_t uid2[18] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The location of a landing target. See:
// https://mavlink.io/en/services/landing_target.html
class MavLinkLandingTarget : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 149;
  MavLinkLandingTarget() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // The ID of the target if multiple targets are present
  uint8_t target_num = 0;
  // Coordinate frame used for following fields.
  uint8_t frame = 0;
  // X-axis angular offset of the target from the center of the image
  float angle_x = 0;
  // Y-axis angular offset of the target from the center of the image
  float angle_y = 0;
  // Distance to the target from the vehicle
  float distance = 0;
  // Size of target along x-axis
  float size_x = 0;
  // Size of target along y-axis
  float size_y = 0;
  // X Position of the landing target in MAV_FRAME
  float x = 0;
  // Y Position of the landing target in MAV_FRAME
  float y = 0;
  // Z Position of the landing target in MAV_FRAME
  float z = 0;
  // Quaternion of landing target orientation (w, x, y, z order, zero-rotation
  // is 1, 0, 0, 0)
  float q[4] = {0};
  // Type of landing target
  uint8_t type = 0;
  // Boolean indicating whether the position fields (x, y, z, q, type) contain
  // valid target position information (valid: 1, invalid: 0). Default is 0
  // (invalid).
  uint8_t position_valid = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Status of geo-fencing. Sent in extended status stream when fencing enabled.
class MavLinkFenceStatus : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 162;
  MavLinkFenceStatus() { msgid = kMessageId; }
  // Breach status (0 if currently inside fence, 1 if outside).
  uint8_t breach_status = 0;
  // Number of fence breaches.
  uint16_t breach_count = 0;
  // Last breach type.
  uint8_t breach_type = 0;
  // Time (since boot) of last breach.
  uint32_t breach_time = 0;
  // Active action to prevent fence breach
  uint8_t breach_mitigation = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Reports results of completed compass calibration. Sent until MAG_CAL_ACK
// received.
class MavLinkMagCalReport : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 192;
  MavLinkMagCalReport() { msgid = kMessageId; }
  // Compass being calibrated.
  uint8_t compass_id = 0;
  // Bitmask of compasses being calibrated.
  uint8_t cal_mask = 0;
  // Calibration Status.
  uint8_t cal_status = 0;
  // 0=requires a MAV_CMD_DO_ACCEPT_MAG_CAL, 1=saved to parameters.
  uint8_t autosaved = 0;
  // RMS milligauss residuals.
  float fitness = 0;
  // X offset.
  float ofs_x = 0;
  // Y offset.
  float ofs_y = 0;
  // Z offset.
  float ofs_z = 0;
  // X diagonal (matrix 11).
  float diag_x = 0;
  // Y diagonal (matrix 22).
  float diag_y = 0;
  // Z diagonal (matrix 33).
  float diag_z = 0;
  // X off-diagonal (matrix 12 and 21).
  float offdiag_x = 0;
  // Y off-diagonal (matrix 13 and 31).
  float offdiag_y = 0;
  // Z off-diagonal (matrix 32 and 23).
  float offdiag_z = 0;
  // Confidence in orientation (higher is better).
  float orientation_confidence = 0;
  // orientation before calibration.
  uint8_t old_orientation = 0;
  // orientation after calibration.
  uint8_t new_orientation = 0;
  // field radius correction factor
  float scale_factor = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// EFI status output
class MavLinkEfiStatus : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 225;
  MavLinkEfiStatus() { msgid = kMessageId; }
  // EFI health status
  uint8_t health = 0;
  // ECU index
  float ecu_index = 0;
  // RPM
  float rpm = 0;
  // Fuel consumed
  float fuel_consumed = 0;
  // Fuel flow rate
  float fuel_flow = 0;
  // Engine load
  float engine_load = 0;
  // Throttle position
  float throttle_position = 0;
  // Spark dwell time
  float spark_dwell_time = 0;
  // Barometric pressure
  float barometric_pressure = 0;
  // Intake manifold pressure(
  float intake_manifold_pressure = 0;
  // Intake manifold temperature
  float intake_manifold_temperature = 0;
  // Cylinder head temperature
  float cylinder_head_temperature = 0;
  // Ignition timing (Crank angle degrees)
  float ignition_timing = 0;
  // Injection time
  float injection_time = 0;
  // Exhaust gas temperature
  float exhaust_gas_temperature = 0;
  // Output throttle
  float throttle_out = 0;
  // Pressure/temperature compensation
  float pt_compensation = 0;
  // Supply voltage to EFI sparking system. Zero in this value means "unknown",
  // so if the supply voltage really is zero volts use 0.0001 instead.
  float ignition_voltage = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Estimator status message including flags, innovation test ratios and
// estimated accuracies. The flags message is an integer bitmask containing
// information on which EKF outputs are valid. See the ESTIMATOR_STATUS_FLAGS
// enum definition for further information. The innovation test ratios show the
// magnitude of the sensor innovation divided by the innovation check threshold.
// Under normal operation the innovation test ratios should be below 0.5 with
// occasional values up to 1.0. Values greater than 1.0 should be rare under
// normal operation and indicate that a measurement has been rejected by the
// filter. The user should be notified if an innovation test ratio greater
// than 1.0 is recorded. Notifications for values in the range between 0.5
// and 1.0 should be optional and controllable by the user.
class MavLinkEstimatorStatus : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 230;
  MavLinkEstimatorStatus() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Bitmap indicating which EKF outputs are valid.
  uint16_t flags = 0;
  // Velocity innovation test ratio
  float vel_ratio = 0;
  // Horizontal position innovation test ratio
  float pos_horiz_ratio = 0;
  // Vertical position innovation test ratio
  float pos_vert_ratio = 0;
  // Magnetometer innovation test ratio
  float mag_ratio = 0;
  // Height above terrain innovation test ratio
  float hagl_ratio = 0;
  // True airspeed innovation test ratio
  float tas_ratio = 0;
  // Horizontal position 1-STD accuracy relative to the EKF local origin
  float pos_horiz_accuracy = 0;
  // Vertical position 1-STD accuracy relative to the EKF local origin
  float pos_vert_accuracy = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Wind estimate from vehicle. Note that despite the name, this message does not
// actually contain any covariances but instead variability and accuracy fields
// in terms of standard deviation (1-STD).
class MavLinkWindCov : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 231;
  MavLinkWindCov() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Wind in North (NED) direction (NAN if unknown)
  float wind_x = 0;
  // Wind in East (NED) direction (NAN if unknown)
  float wind_y = 0;
  // Wind in down (NED) direction (NAN if unknown)
  float wind_z = 0;
  // Variability of wind in XY, 1-STD estimated from a 1 Hz lowpassed wind
  // estimate (NAN if unknown)
  float var_horiz = 0;
  // Variability of wind in Z, 1-STD estimated from a 1 Hz lowpassed wind
  // estimate (NAN if unknown)
  float var_vert = 0;
  // Altitude (MSL) that this measurement was taken at (NAN if unknown)
  float wind_alt = 0;
  // Horizontal speed 1-STD accuracy (0 if unknown)
  float horiz_accuracy = 0;
  // Vertical speed 1-STD accuracy (0 if unknown)
  float vert_accuracy = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// GPS sensor input message. This is a raw sensor value sent by the GPS. This is
// NOT the global position estimate of the system.
class MavLinkGpsInput : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 232;
  MavLinkGpsInput() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // ID of the GPS for multiple GPS inputs
  uint8_t gps_id = 0;
  // Bitmap indicating which GPS input flags fields to ignore. All other fields
  // must be provided.
  uint16_t ignore_flags = 0;
  // GPS time (from start of GPS week)
  uint32_t time_week_ms = 0;
  // GPS week number
  uint16_t time_week = 0;
  // 0-1: no fix, 2: 2D fix, 3: 3D fix. 4: 3D with DGPS. 5: 3D with RTK
  uint8_t fix_type = 0;
  // Latitude (WGS84)
  int32_t lat = 0;
  // Longitude (WGS84)
  int32_t lon = 0;
  // Altitude (MSL). Positive for up.
  float alt = 0;
  // GPS HDOP horizontal dilution of position (unitless). If unknown, set to:
  // UINT16_MAX
  float hdop = 0;
  // GPS VDOP vertical dilution of position (unitless). If unknown, set to:
  // UINT16_MAX
  float vdop = 0;
  // GPS velocity in north direction in earth-fixed NED frame
  float vn = 0;
  // GPS velocity in east direction in earth-fixed NED frame
  float ve = 0;
  // GPS velocity in down direction in earth-fixed NED frame
  float vd = 0;
  // GPS speed accuracy
  float speed_accuracy = 0;
  // GPS horizontal accuracy
  float horiz_accuracy = 0;
  // GPS vertical accuracy
  float vert_accuracy = 0;
  // Number of satellites visible.
  uint8_t satellites_visible = 0;
  // Yaw of vehicle relative to Earth's North, zero means not available, use
  // 36000 for north
  uint16_t yaw = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// RTCM message for injecting into the onboard GPS (used for DGPS)
class MavLinkGpsRtcmData : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 233;
  MavLinkGpsRtcmData() { msgid = kMessageId; }
  // LSB: 1 means message is fragmented, next 2 bits are the fragment ID, the
  // remaining 5 bits are used for the sequence ID. Messages are only to be
  // flushed to the GPS when the entire message has been reconstructed on the
  // autopilot. The fragment ID specifies which order the fragments should be
  // assembled into a buffer, while the sequence ID is used to detect a mismatch
  // between different buffers. The buffer is considered fully reconstructed
  // when either all 4 fragments are present, or all the fragments before the
  // first fragment with a non full payload is received. This management is used
  // to ensure that normal GPS operation doesn't corrupt RTCM data, and to
  // recover from a unreliable transport delivery order.
  uint8_t flags = 0;
  // data length
  uint8_t len = 0;
  // RTCM message (may be fragmented)
  uint8_t data[180] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Message appropriate for high latency connections like Iridium
class MavLinkHighLatency : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 234;
  MavLinkHighLatency() { msgid = kMessageId; }
  // Bitmap of enabled system modes.
  uint8_t base_mode = 0;
  // A bitfield for use for autopilot-specific flags.
  uint32_t custom_mode = 0;
  // The landed state. Is set to MAV_LANDED_STATE_UNDEFINED if landed state is
  // unknown.
  uint8_t landed_state = 0;
  // roll
  int16_t roll = 0;
  // pitch
  int16_t pitch = 0;
  // heading
  uint16_t heading = 0;
  // throttle (percentage)
  int8_t throttle = 0;
  // heading setpoint
  int16_t heading_sp = 0;
  // Latitude
  int32_t latitude = 0;
  // Longitude
  int32_t longitude = 0;
  // Altitude above mean sea level
  int16_t altitude_amsl = 0;
  // Altitude setpoint relative to the home position
  int16_t altitude_sp = 0;
  // airspeed
  uint8_t airspeed = 0;
  // airspeed setpoint
  uint8_t airspeed_sp = 0;
  // groundspeed
  uint8_t groundspeed = 0;
  // climb rate
  int8_t climb_rate = 0;
  // Number of satellites visible. If unknown, set to UINT8_MAX
  uint8_t gps_nsat = 0;
  // GPS Fix type.
  uint8_t gps_fix_type = 0;
  // Remaining battery (percentage)
  uint8_t battery_remaining = 0;
  // Autopilot temperature (degrees C)
  int8_t temperature = 0;
  // Air temperature (degrees C) from airspeed sensor
  int8_t temperature_air = 0;
  // failsafe (each bit represents a failsafe where 0=ok, 1=failsafe active
  // (bit0:RC, bit1:batt, bit2:GPS, bit3:GCS, bit4:fence)
  uint8_t failsafe = 0;
  // current waypoint number
  uint8_t wp_num = 0;
  // distance to target
  uint16_t wp_distance = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Message appropriate for high latency connections like Iridium (version 2)
class MavLinkHighLatency2 : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 235;
  MavLinkHighLatency2() { msgid = kMessageId; }
  // Timestamp (milliseconds since boot or Unix epoch)
  uint32_t timestamp = 0;
  // Type of the MAV (quadrotor, helicopter, etc.)
  uint8_t type = 0;
  // Autopilot type / class. Use MAV_AUTOPILOT_INVALID for components that are
  // not flight controllers.
  uint8_t autopilot = 0;
  // A bitfield for use for autopilot-specific flags (2 byte version).
  uint16_t custom_mode = 0;
  // Latitude
  int32_t latitude = 0;
  // Longitude
  int32_t longitude = 0;
  // Altitude above mean sea level
  int16_t altitude = 0;
  // Altitude setpoint
  int16_t target_altitude = 0;
  // Heading
  uint8_t heading = 0;
  // Heading setpoint
  uint8_t target_heading = 0;
  // Distance to target waypoint or position
  uint16_t target_distance = 0;
  // Throttle
  uint8_t throttle = 0;
  // Airspeed
  uint8_t airspeed = 0;
  // Airspeed setpoint
  uint8_t airspeed_sp = 0;
  // Groundspeed
  uint8_t groundspeed = 0;
  // Windspeed
  uint8_t windspeed = 0;
  // Wind heading
  uint8_t wind_heading = 0;
  // Maximum error horizontal position since last message
  uint8_t eph = 0;
  // Maximum error vertical position since last message
  uint8_t epv = 0;
  // Air temperature from airspeed sensor
  int8_t temperature_air = 0;
  // Maximum climb rate magnitude since last message
  int8_t climb_rate = 0;
  // Battery level (-1 if field not provided).
  int8_t battery = 0;
  // Current waypoint number
  uint16_t wp_num = 0;
  // Bitmap of failure flags.
  uint16_t failure_flags = 0;
  // Field for custom payload.
  int8_t custom0 = 0;
  // Field for custom payload.
  int8_t custom1 = 0;
  // Field for custom payload.
  int8_t custom2 = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Vibration levels and accelerometer clipping
class MavLinkVibration : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 241;
  MavLinkVibration() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Vibration levels on X-axis
  float vibration_x = 0;
  // Vibration levels on Y-axis
  float vibration_y = 0;
  // Vibration levels on Z-axis
  float vibration_z = 0;
  // first accelerometer clipping count
  uint32_t clipping_0 = 0;
  // second accelerometer clipping count
  uint32_t clipping_1 = 0;
  // third accelerometer clipping count
  uint32_t clipping_2 = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Contains the home position. The home position is the default position that
// the system will return to and land on. The position must be set automatically
// by the system during the takeoff, and may also be explicitly set using
// MAV_CMD_DO_SET_HOME. The global and local positions encode the position in
// the respective coordinate frames, while the q parameter encodes the
// orientation of the surface. Under normal conditions it describes the heading
// and terrain slope, which can be used by the aircraft to adjust the approach.
// The approach 3D vector describes the point to which the system should fly in
// normal flight mode and then perform a landing sequence along the vector.
// Note: this message can be requested by sending the MAV_CMD_REQUEST_MESSAGE
// with param1=242 (or the deprecated MAV_CMD_GET_HOME_POSITION command).
class MavLinkHomePosition : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 242;
  MavLinkHomePosition() { msgid = kMessageId; }
  // Latitude (WGS84)
  int32_t latitude = 0;
  // Longitude (WGS84)
  int32_t longitude = 0;
  // Altitude (MSL). Positive for up.
  int32_t altitude = 0;
  // Local X position of this position in the local coordinate frame
  float x = 0;
  // Local Y position of this position in the local coordinate frame
  float y = 0;
  // Local Z position of this position in the local coordinate frame
  float z = 0;
  // World to surface normal and heading transformation of the takeoff position.
  // Used to indicate the heading and slope of the ground
  float q[4] = {0};
  // Local X position of the end of the approach vector. Multicopters should set
  // this position based on their takeoff path. Grass-landing fixed wing
  // aircraft should set it the same way as multicopters. Runway-landing fixed
  // wing aircraft should set it to the opposite direction of the takeoff,
  // assuming the takeoff happened from the threshold / touchdown zone.
  float approach_x = 0;
  // Local Y position of the end of the approach vector. Multicopters should set
  // this position based on their takeoff path. Grass-landing fixed wing
  // aircraft should set it the same way as multicopters. Runway-landing fixed
  // wing aircraft should set it to the opposite direction of the takeoff,
  // assuming the takeoff happened from the threshold / touchdown zone.
  float approach_y = 0;
  // Local Z position of the end of the approach vector. Multicopters should set
  // this position based on their takeoff path. Grass-landing fixed wing
  // aircraft should set it the same way as multicopters. Runway-landing fixed
  // wing aircraft should set it to the opposite direction of the takeoff,
  // assuming the takeoff happened from the threshold / touchdown zone.
  float approach_z = 0;
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Sets the home position. The home position is the default position that the
// system will return to and land on. The position is set automatically by the
// system during the takeoff (and may also be set using this message). The
// global and local positions encode the position in the respective coordinate
// frames, while the q parameter encodes the orientation of the surface. Under
// normal conditions it describes the heading and terrain slope, which can be
// used by the aircraft to adjust the approach. The approach 3D vector describes
// the point to which the system should fly in normal flight mode and then
// perform a landing sequence along the vector. Note: the current home position
// may be emitted in a HOME_POSITION message on request (using
// MAV_CMD_REQUEST_MESSAGE with param1=242).
class MavLinkSetHomePosition : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 243;
  MavLinkSetHomePosition() { msgid = kMessageId; }
  // System ID.
  uint8_t target_system = 0;
  // Latitude (WGS84)
  int32_t latitude = 0;
  // Longitude (WGS84)
  int32_t longitude = 0;
  // Altitude (MSL). Positive for up.
  int32_t altitude = 0;
  // Local X position of this position in the local coordinate frame
  float x = 0;
  // Local Y position of this position in the local coordinate frame
  float y = 0;
  // Local Z position of this position in the local coordinate frame
  float z = 0;
  // World to surface normal and heading transformation of the takeoff position.
  // Used to indicate the heading and slope of the ground
  float q[4] = {0};
  // Local X position of the end of the approach vector. Multicopters should set
  // this position based on their takeoff path. Grass-landing fixed wing
  // aircraft should set it the same way as multicopters. Runway-landing fixed
  // wing aircraft should set it to the opposite direction of the takeoff,
  // assuming the takeoff happened from the threshold / touchdown zone.
  float approach_x = 0;
  // Local Y position of the end of the approach vector. Multicopters should set
  // this position based on their takeoff path. Grass-landing fixed wing
  // aircraft should set it the same way as multicopters. Runway-landing fixed
  // wing aircraft should set it to the opposite direction of the takeoff,
  // assuming the takeoff happened from the threshold / touchdown zone.
  float approach_y = 0;
  // Local Z position of the end of the approach vector. Multicopters should set
  // this position based on their takeoff path. Grass-landing fixed wing
  // aircraft should set it the same way as multicopters. Runway-landing fixed
  // wing aircraft should set it to the opposite direction of the takeoff,
  // assuming the takeoff happened from the threshold / touchdown zone.
  float approach_z = 0;
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The interval between messages for a particular MAVLink message ID. This
// message is sent in response to the MAV_CMD_REQUEST_MESSAGE command with
// param1=244 (this message) and param2=message_id (the id of the message for
// which the interval is required). It may also be sent in response to
// MAV_CMD_GET_MESSAGE_INTERVAL. This interface replaces DATA_STREAM.
class MavLinkMessageInterval : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 244;
  MavLinkMessageInterval() { msgid = kMessageId; }
  // The ID of the requested MAVLink message. v1.0 is limited to 254 messages.
  uint16_t message_id = 0;
  // The interval between two messages. A value of -1 indicates this stream is
  // disabled, 0 indicates it is not available, > 0 indicates the interval at
  // which it is sent.
  int32_t interval_us = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Provides state for additional features
class MavLinkExtendedSysState : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 245;
  MavLinkExtendedSysState() { msgid = kMessageId; }
  // The VTOL state if applicable. Is set to MAV_VTOL_STATE_UNDEFINED if UAV is
  // not in VTOL configuration.
  uint8_t vtol_state = 0;
  // The landed state. Is set to MAV_LANDED_STATE_UNDEFINED if landed state is
  // unknown.
  uint8_t landed_state = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The location and information of an ADSB vehicle
class MavLinkAdsbVehicle : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 246;
  MavLinkAdsbVehicle() { msgid = kMessageId; }
  // ICAO address
  uint32_t ICAO_address = 0;
  // Latitude
  int32_t lat = 0;
  // Longitude
  int32_t lon = 0;
  // ADSB altitude type.
  uint8_t altitude_type = 0;
  // Altitude(ASL)
  int32_t altitude = 0;
  // Course over ground
  uint16_t heading = 0;
  // The horizontal velocity
  uint16_t hor_velocity = 0;
  // The vertical velocity. Positive is up
  int16_t ver_velocity = 0;
  // The callsign, 8+null
  char callsign[9] = {0};
  // ADSB emitter type.
  uint8_t emitter_type = 0;
  // Time since last communication in seconds
  uint8_t tslc = 0;
  // Bitmap to indicate various statuses including valid data fields
  uint16_t flags = 0;
  // Squawk code
  uint16_t squawk = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Information about a potential collision
class MavLinkCollision : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 247;
  MavLinkCollision() { msgid = kMessageId; }
  // Collision data source
  uint8_t src = 0;
  // Unique identifier, domain based on src field
  uint32_t id = 0;
  // Action that is being taken to avoid this collision
  uint8_t action = 0;
  // How concerned the aircraft is about this collision
  uint8_t threat_level = 0;
  // Estimated time until collision occurs
  float time_to_minimum_delta = 0;
  // Closest vertical distance between vehicle and object
  float altitude_minimum_delta = 0;
  // Closest horizontal distance between vehicle and object
  float horizontal_minimum_delta = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Message implementing parts of the V2 payload specs in V1 frames for
// transitional support.
class MavLinkV2Extension : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 248;
  MavLinkV2Extension() { msgid = kMessageId; }
  // Network ID (0 for broadcast)
  uint8_t target_network = 0;
  // System ID (0 for broadcast)
  uint8_t target_system = 0;
  // Component ID (0 for broadcast)
  uint8_t target_component = 0;
  // A code that identifies the software component that understands this message
  // (analogous to USB device classes or mime type strings). If this code is
  // less than 32768, it is considered a 'registered' protocol extension and the
  // corresponding entry should be added to
  // https://github.com/mavlink/mavlink/definition_files/extension_message_ids.xml.
  // Software creators can register blocks of message IDs as needed (useful for
  // GCS specific metadata, etc...). Message_types greater than 32767 are
  // considered local experiments and should not be checked in to any widely
  // distributed codebase.
  uint16_t message_type = 0;
  // Variable length payload. The length must be encoded in the payload as part
  // of the message_type protocol, e.g. by including the length as payload data,
  // or by terminating the payload data with a non-zero marker. This is required
  // in order to reconstruct zero-terminated payloads that are (or otherwise
  // would be) trimmed by MAVLink 2 empty-byte truncation. The entire content of
  // the payload block is opaque unless you understand the encoding
  // message_type. The particular encoding used can be extension specific and
  // might not always be documented as part of the MAVLink specification.
  uint8_t payload[249] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Send raw controller memory. The use of this message is discouraged for normal
// packets, but a quite efficient way for testing new messages and getting
// experimental debug output.
class MavLinkMemoryVect : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 249;
  MavLinkMemoryVect() { msgid = kMessageId; }
  // Starting address of the debug variables
  uint16_t address = 0;
  // Version code of the type variable. 0=unknown, type ignored and assumed
  // int16_t. 1=as below
  uint8_t ver = 0;
  // Type code of the memory variables. for ver = 1: 0=16 x int16_t, 1=16 x
  // uint16_t, 2=16 x Q15, 3=16 x 1Q14
  uint8_t type = 0;
  // Memory contents at specified address
  int8_t value[32] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// To debug something using a named 3D vector.
class MavLinkDebugVect : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 250;
  MavLinkDebugVect() { msgid = kMessageId; }
  // Name
  char name[10] = {0};
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // x
  float x = 0;
  // y
  float y = 0;
  // z
  float z = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Send a key-value pair as float. The use of this message is discouraged for
// normal packets, but a quite efficient way for testing new messages and
// getting experimental debug output.
class MavLinkNamedValueFloat : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 251;
  MavLinkNamedValueFloat() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Name of the debug variable
  char name[10] = {0};
  // Floating point value
  float value = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Send a key-value pair as integer. The use of this message is discouraged for
// normal packets, but a quite efficient way for testing new messages and
// getting experimental debug output.
class MavLinkNamedValueInt : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 252;
  MavLinkNamedValueInt() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Name of the debug variable
  char name[10] = {0};
  // Signed integer value
  int32_t value = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Status text message. These messages are printed in yellow in the COMM console
// of QGroundControl. WARNING: They consume quite some bandwidth, so use only
// for important status and error messages. If implemented wisely, these
// messages are buffered on the MCU and sent only at a limited rate (e.g. 10
// Hz).
class MavLinkStatustext : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 253;
  MavLinkStatustext() { msgid = kMessageId; }
  // Severity of status. Relies on the definitions within RFC-5424.
  uint8_t severity = 0;
  // Status text message, without null termination character
  char text[50] = {0};
  // Unique (opaque) identifier for this statustext message. May be used to
  // reassemble a logical long-statustext message from a sequence of chunks. A
  // value of zero indicates this is the only chunk in the sequence and the
  // message can be emitted immediately.
  uint16_t id = 0;
  // This chunk's sequence number; indexing is from zero. Any null character in
  // the text field is taken to mean this was the last chunk.
  uint8_t chunk_seq = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Send a debug value. The index is used to discriminate between values. These
// values show up in the plot of QGroundControl as DEBUG N.
class MavLinkDebug : public MavLinkMessageBase {
 public:
  const static uint8_t kMessageId = 254;
  MavLinkDebug() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // index of debug variable
  uint8_t ind = 0;
  // DEBUG value
  float value = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Setup a MAVLink2 signing key. If called with secret_key of all zero and zero
// initial_timestamp will disable signing
class MavLinkSetupSigning : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 256;
  MavLinkSetupSigning() { msgid = kMessageId; }
  // system id of the target
  uint8_t target_system = 0;
  // component ID of the target
  uint8_t target_component = 0;
  // signing key
  uint8_t secret_key[32] = {0};
  // initial timestamp
  uint64_t initial_timestamp = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Report button state change.
class MavLinkButtonChange : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 257;
  MavLinkButtonChange() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Time of last change of button state.
  uint32_t last_change_ms = 0;
  // Bitmap for state of buttons.
  uint8_t state = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Control vehicle tone generation (buzzer).
class MavLinkPlayTune : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 258;
  MavLinkPlayTune() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // tune in board specific format
  char tune[30] = {0};
  // tune extension (appended to tune)
  char tune2[200] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Information about a camera. Can be requested with a MAV_CMD_REQUEST_MESSAGE
// command.
class MavLinkCameraInformation : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 259;
  MavLinkCameraInformation() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Name of the camera vendor
  uint8_t vendor_name[32] = {0};
  // Name of the camera model
  uint8_t model_name[32] = {0};
  // Version of the camera firmware, encoded as: (Dev & 0xff) << 24 | (Patch &
  // 0xff)
  // << 16 | (Minor & 0xff) << 8 | (Major & 0xff)
  uint32_t firmware_version = 0;
  // Focal length
  float focal_length = 0;
  // Image sensor size horizontal
  float sensor_size_h = 0;
  // Image sensor size vertical
  float sensor_size_v = 0;
  // Horizontal image resolution
  uint16_t resolution_h = 0;
  // Vertical image resolution
  uint16_t resolution_v = 0;
  // Reserved for a lens ID
  uint8_t lens_id = 0;
  // Bitmap of camera capability flags.
  uint32_t flags = 0;
  // Camera definition version (iteration)
  uint16_t cam_definition_version = 0;
  // Camera definition URI (if any, otherwise only basic functions will be
  // available). HTTP- (http://) and MAVLink FTP- (mavlinkftp://) formatted URIs
  // are allowed (and both must be supported by any GCS that implements the
  // Camera Protocol). The definition file may be xz compressed, which will be
  // indicated by the file extension .xml.xz (a GCS that implements the protocol
  // must support decompressing the file). The string needs to be zero
  // terminated.
  char cam_definition_uri[140] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Settings of a camera. Can be requested with a MAV_CMD_REQUEST_MESSAGE
// command.
class MavLinkCameraSettings : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 260;
  MavLinkCameraSettings() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Camera mode
  uint8_t mode_id = 0;
  // Current zoom level (0.0 to 100.0, NaN if not known)
  float zoomLevel = 0;
  // Current focus level (0.0 to 100.0, NaN if not known)
  float focusLevel = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Information about a storage medium. This message is sent in response to a
// request with MAV_CMD_REQUEST_MESSAGE and whenever the status of the storage
// changes (STORAGE_STATUS). Use MAV_CMD_REQUEST_MESSAGE.param2 to indicate the
// index/id of requested storage: 0 for all, 1 for first, 2 for second, etc.
class MavLinkStorageInformation : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 261;
  MavLinkStorageInformation() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Storage ID (1 for first, 2 for second, etc.)
  uint8_t storage_id = 0;
  // Number of storage devices
  uint8_t storage_count = 0;
  // Status of storage
  uint8_t status = 0;
  // Total capacity. If storage is not ready (STORAGE_STATUS_READY) value will
  // be ignored.
  float total_capacity = 0;
  // Used capacity. If storage is not ready (STORAGE_STATUS_READY) value will be
  // ignored.
  float used_capacity = 0;
  // Available storage capacity. If storage is not ready (STORAGE_STATUS_READY)
  // value will be ignored.
  float available_capacity = 0;
  // Read speed.
  float read_speed = 0;
  // Write speed.
  float write_speed = 0;
  // Type of storage
  uint8_t type = 0;
  // Textual storage name to be used in UI (microSD 1, Internal Memory, etc.)
  // This is a NULL terminated string. If it is exactly 32 characters long, add
  // a terminating NULL. If this string is empty, the generic type is shown to
  // the user.
  char name[32] = {0};
  // Flags indicating whether this instance is preferred storage for photos,
  // videos, etc. Note: Implementations should initially set the flags on the
  // system-default storage id used for saving media (if possible/supported).
  // This setting can then be overridden using MAV_CMD_SET_STORAGE_USAGE. If the
  // media usage flags are not set, a GCS may assume storage ID 1 is the default
  // storage for all media types.
  uint8_t storage_usage = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Information about the status of a capture. Can be requested with a
// MAV_CMD_REQUEST_MESSAGE command.
class MavLinkCameraCaptureStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 262;
  MavLinkCameraCaptureStatus() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Current status of image capturing (0: idle, 1: capture in progress, 2:
  // interval set but idle, 3: interval set and capture in progress)
  uint8_t image_status = 0;
  // Current status of video capturing (0: idle, 1: capture in progress)
  uint8_t video_status = 0;
  // Image capture interval
  float image_interval = 0;
  // Elapsed time since recording started (0: Not supported/available). A GCS
  // should compute recording time and use non-zero values of this field to
  // correct any discrepancy.
  uint32_t recording_time_ms = 0;
  // Available storage capacity.
  float available_capacity = 0;
  // Total number of images captured ('forever', or until reset using
  // MAV_CMD_STORAGE_FORMAT).
  int32_t image_count = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Information about a captured image. This is emitted every time a message is
// captured. MAV_CMD_REQUEST_MESSAGE can be used to (re)request this message for
// a specific sequence number or range of sequence numbers:
// MAV_CMD_REQUEST_MESSAGE.param2 indicates the sequence number the first image
// to send, or set to -1 to send the message for all sequence numbers.
// MAV_CMD_REQUEST_MESSAGE.param3 is used to specify a range of messages to
// send: set to 0 (default) to send just the the message for the sequence number
// in param 2, set to -1 to send the message for the sequence number in param 2
// and all the following sequence numbers, set to the sequence number of the
// final message in the range.
class MavLinkCameraImageCaptured : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 263;
  MavLinkCameraImageCaptured() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Timestamp (time since UNIX epoch) in UTC. 0 for unknown.
  uint64_t time_utc = 0;
  // Deprecated/unused. Component IDs are used to differentiate multiple
  // cameras.
  uint8_t camera_id = 0;
  // Latitude where image was taken
  int32_t lat = 0;
  // Longitude where capture was taken
  int32_t lon = 0;
  // Altitude (MSL) where image was taken
  int32_t alt = 0;
  // Altitude above ground
  int32_t relative_alt = 0;
  // Quaternion of camera orientation (w, x, y, z order, zero-rotation is 1, 0,
  // 0, 0)
  float q[4] = {0};
  // Zero based index of this image (i.e. a new image will have index
  // CAMERA_CAPTURE_STATUS.image count -1)
  int32_t image_index = 0;
  // Boolean indicating success (1) or failure (0) while capturing this image.
  int8_t capture_result = 0;
  // URL of image taken. Either local storage or http://foo.jpg if camera
  // provides an HTTP interface.
  char file_url[205] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Information about flight since last arming. This can be requested using
// MAV_CMD_REQUEST_MESSAGE.
class MavLinkFlightInformation : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 264;
  MavLinkFlightInformation() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Timestamp at arming (time since UNIX epoch) in UTC, 0 for unknown
  uint64_t arming_time_utc = 0;
  // Timestamp at takeoff (time since UNIX epoch) in UTC, 0 for unknown
  uint64_t takeoff_time_utc = 0;
  // Universally unique identifier (UUID) of flight, should correspond to name
  // of log files
  uint64_t flight_uuid = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Orientation of a mount
class MavLinkMountOrientation : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 265;
  MavLinkMountOrientation() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Roll in global frame (set to NaN for invalid).
  float roll = 0;
  // Pitch in global frame (set to NaN for invalid).
  float pitch = 0;
  // Yaw relative to vehicle (set to NaN for invalid).
  float yaw = 0;
  // Yaw in absolute frame relative to Earth's North, north is 0 (set to NaN for
  // invalid).
  float yaw_absolute = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// A message containing logged data (see also MAV_CMD_LOGGING_START)
class MavLinkLoggingData : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 266;
  MavLinkLoggingData() { msgid = kMessageId; }
  // system ID of the target
  uint8_t target_system = 0;
  // component ID of the target
  uint8_t target_component = 0;
  // sequence number (can wrap)
  uint16_t sequence = 0;
  // data length
  uint8_t length = 0;
  // offset into data where first message starts. This can be used for recovery,
  // when a previous message got lost (set to UINT8_MAX if no start exists).
  uint8_t first_message_offset = 0;
  // logged data
  uint8_t data[249] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// A message containing logged data which requires a LOGGING_ACK to be sent back
class MavLinkLoggingDataAcked : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 267;
  MavLinkLoggingDataAcked() { msgid = kMessageId; }
  // system ID of the target
  uint8_t target_system = 0;
  // component ID of the target
  uint8_t target_component = 0;
  // sequence number (can wrap)
  uint16_t sequence = 0;
  // data length
  uint8_t length = 0;
  // offset into data where first message starts. This can be used for recovery,
  // when a previous message got lost (set to UINT8_MAX if no start exists).
  uint8_t first_message_offset = 0;
  // logged data
  uint8_t data[249] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// An ack for a LOGGING_DATA_ACKED message
class MavLinkLoggingAck : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 268;
  MavLinkLoggingAck() { msgid = kMessageId; }
  // system ID of the target
  uint8_t target_system = 0;
  // component ID of the target
  uint8_t target_component = 0;
  // sequence number (must match the one in LOGGING_DATA_ACKED)
  uint16_t sequence = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Information about video stream. It may be requested using
// MAV_CMD_REQUEST_MESSAGE, where param2 indicates the video stream id: 0 for
// all streams, 1 for first, 2 for second, etc.
class MavLinkVideoStreamInformation : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 269;
  MavLinkVideoStreamInformation() { msgid = kMessageId; }
  // Video Stream ID (1 for first, 2 for second, etc.)
  uint8_t stream_id = 0;
  // Number of streams available.
  uint8_t count = 0;
  // Type of stream.
  uint8_t type = 0;
  // Bitmap of stream status flags.
  uint16_t flags = 0;
  // Frame rate.
  float framerate = 0;
  // Horizontal resolution.
  uint16_t resolution_h = 0;
  // Vertical resolution.
  uint16_t resolution_v = 0;
  // Bit rate.
  uint32_t bitrate = 0;
  // Video image rotation clockwise.
  uint16_t rotation = 0;
  // Horizontal Field of view.
  uint16_t hfov = 0;
  // Stream name.
  char name[32] = {0};
  // Video stream URI (TCP or RTSP URI ground station should connect to) or port
  // number (UDP port ground station should listen to).
  char uri[160] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Information about the status of a video stream. It may be requested using
// MAV_CMD_REQUEST_MESSAGE.
class MavLinkVideoStreamStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 270;
  MavLinkVideoStreamStatus() { msgid = kMessageId; }
  // Video Stream ID (1 for first, 2 for second, etc.)
  uint8_t stream_id = 0;
  // Bitmap of stream status flags
  uint16_t flags = 0;
  // Frame rate
  float framerate = 0;
  // Horizontal resolution
  uint16_t resolution_h = 0;
  // Vertical resolution
  uint16_t resolution_v = 0;
  // Bit rate
  uint32_t bitrate = 0;
  // Video image rotation clockwise
  uint16_t rotation = 0;
  // Horizontal Field of view
  uint16_t hfov = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Information about the field of view of a camera. Can be requested with a
// MAV_CMD_REQUEST_MESSAGE command.
class MavLinkCameraFovStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 271;
  MavLinkCameraFovStatus() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Latitude of camera (INT32_MAX if unknown).
  int32_t lat_camera = 0;
  // Longitude of camera (INT32_MAX if unknown).
  int32_t lon_camera = 0;
  // Altitude (MSL) of camera (INT32_MAX if unknown).
  int32_t alt_camera = 0;
  // Latitude of center of image (INT32_MAX if unknown, INT32_MIN if at
  // infinity, not intersecting with horizon).
  int32_t lat_image = 0;
  // Longitude of center of image (INT32_MAX if unknown, INT32_MIN if at
  // infinity, not intersecting with horizon).
  int32_t lon_image = 0;
  // Altitude (MSL) of center of image (INT32_MAX if unknown, INT32_MIN if at
  // infinity, not intersecting with horizon).
  int32_t alt_image = 0;
  // Quaternion of camera orientation (w, x, y, z order, zero-rotation is 1, 0,
  // 0, 0)
  float q[4] = {0};
  // Horizontal field of view (NaN if unknown).
  float hfov = 0;
  // Vertical field of view (NaN if unknown).
  float vfov = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Camera tracking status, sent while in active tracking. Use
// MAV_CMD_SET_MESSAGE_INTERVAL to define message interval.
class MavLinkCameraTrackingImageStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 275;
  MavLinkCameraTrackingImageStatus() { msgid = kMessageId; }
  // Current tracking status
  uint8_t tracking_status = 0;
  // Current tracking mode
  uint8_t tracking_mode = 0;
  // Defines location of target data
  uint8_t target_data = 0;
  // Current tracked point x value if CAMERA_TRACKING_MODE_POINT (normalized
  // 0..1, 0 is left, 1 is right), NAN if unknown
  float point_x = 0;
  // Current tracked point y value if CAMERA_TRACKING_MODE_POINT (normalized
  // 0..1, 0 is top, 1 is bottom), NAN if unknown
  float point_y = 0;
  // Current tracked radius if CAMERA_TRACKING_MODE_POINT (normalized 0..1, 0 is
  // image left, 1 is image right), NAN if unknown
  float radius = 0;
  // Current tracked rectangle top x value if CAMERA_TRACKING_MODE_RECTANGLE
  // (normalized 0..1, 0 is left, 1 is right), NAN if unknown
  float rec_top_x = 0;
  // Current tracked rectangle top y value if CAMERA_TRACKING_MODE_RECTANGLE
  // (normalized 0..1, 0 is top, 1 is bottom), NAN if unknown
  float rec_top_y = 0;
  // Current tracked rectangle bottom x value if CAMERA_TRACKING_MODE_RECTANGLE
  // (normalized 0..1, 0 is left, 1 is right), NAN if unknown
  float rec_bottom_x = 0;
  // Current tracked rectangle bottom y value if CAMERA_TRACKING_MODE_RECTANGLE
  // (normalized 0..1, 0 is top, 1 is bottom), NAN if unknown
  float rec_bottom_y = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Camera tracking status, sent while in active tracking. Use
// MAV_CMD_SET_MESSAGE_INTERVAL to define message interval.
class MavLinkCameraTrackingGeoStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 276;
  MavLinkCameraTrackingGeoStatus() { msgid = kMessageId; }
  // Current tracking status
  uint8_t tracking_status = 0;
  // Latitude of tracked object
  int32_t lat = 0;
  // Longitude of tracked object
  int32_t lon = 0;
  // Altitude of tracked object(AMSL, WGS84)
  float alt = 0;
  // Horizontal accuracy. NAN if unknown
  float h_acc = 0;
  // Vertical accuracy. NAN if unknown
  float v_acc = 0;
  // North velocity of tracked object. NAN if unknown
  float vel_n = 0;
  // East velocity of tracked object. NAN if unknown
  float vel_e = 0;
  // Down velocity of tracked object. NAN if unknown
  float vel_d = 0;
  // Velocity accuracy. NAN if unknown
  float vel_acc = 0;
  // Distance between camera and tracked object. NAN if unknown
  float dist = 0;
  // Heading in radians, in NED. NAN if unknown
  float hdg = 0;
  // Accuracy of heading, in NED. NAN if unknown
  float hdg_acc = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Information about a high level gimbal manager. This message should be
// requested by a ground station using MAV_CMD_REQUEST_MESSAGE.
class MavLinkGimbalManagerInformation : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 280;
  MavLinkGimbalManagerInformation() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Bitmap of gimbal capability flags.
  uint32_t cap_flags = 0;
  // Gimbal device ID that this gimbal manager is responsible for.
  uint8_t gimbal_device_id = 0;
  // Minimum hardware roll angle (positive: rolling to the right, negative:
  // rolling to the left)
  float roll_min = 0;
  // Maximum hardware roll angle (positive: rolling to the right, negative:
  // rolling to the left)
  float roll_max = 0;
  // Minimum pitch angle (positive: up, negative: down)
  float pitch_min = 0;
  // Maximum pitch angle (positive: up, negative: down)
  float pitch_max = 0;
  // Minimum yaw angle (positive: to the right, negative: to the left)
  float yaw_min = 0;
  // Maximum yaw angle (positive: to the right, negative: to the left)
  float yaw_max = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Current status about a high level gimbal manager. This message should be
// broadcast at a low regular rate (e.g. 5Hz).
class MavLinkGimbalManagerStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 281;
  MavLinkGimbalManagerStatus() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // High level gimbal manager flags currently applied.
  uint32_t flags = 0;
  // Gimbal device ID that this gimbal manager is responsible for.
  uint8_t gimbal_device_id = 0;
  // System ID of MAVLink component with primary control, 0 for none.
  uint8_t primary_control_sysid = 0;
  // Component ID of MAVLink component with primary control, 0 for none.
  uint8_t primary_control_compid = 0;
  // System ID of MAVLink component with secondary control, 0 for none.
  uint8_t secondary_control_sysid = 0;
  // Component ID of MAVLink component with secondary control, 0 for none.
  uint8_t secondary_control_compid = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// High level message to control a gimbal's attitude. This message is to be sent
// to the gimbal manager (e.g. from a ground station). Angles and rates can be
// set to NaN according to use case.
class MavLinkGimbalManagerSetAttitude : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 282;
  MavLinkGimbalManagerSetAttitude() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // High level gimbal manager flags to use.
  uint32_t flags = 0;
  // Component ID of gimbal device to address (or 1-6 for non-MAVLink gimbal), 0
  // for all gimbal device components. Send command multiple times for more than
  // one gimbal (but not all gimbals).
  uint8_t gimbal_device_id = 0;
  // Quaternion components, w, x, y, z (1 0 0 0 is the null-rotation, the frame
  // is depends on whether the flag GIMBAL_MANAGER_FLAGS_YAW_LOCK is set)
  float q[4] = {0};
  // X component of angular velocity, positive is rolling to the right, NaN to
  // be ignored.
  float angular_velocity_x = 0;
  // Y component of angular velocity, positive is pitching up, NaN to be
  // ignored.
  float angular_velocity_y = 0;
  // Z component of angular velocity, positive is yawing to the right, NaN to be
  // ignored.
  float angular_velocity_z = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Information about a low level gimbal. This message should be requested by the
// gimbal manager or a ground station using MAV_CMD_REQUEST_MESSAGE. The maximum
// angles and rates are the limits by hardware. However, the limits by software
// used are likely different/smaller and dependent on mode/settings/etc..
class MavLinkGimbalDeviceInformation : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 283;
  MavLinkGimbalDeviceInformation() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Name of the gimbal vendor.
  char vendor_name[32] = {0};
  // Name of the gimbal model.
  char model_name[32] = {0};
  // Custom name of the gimbal given to it by the user.
  char custom_name[32] = {0};
  // Version of the gimbal firmware, encoded as: (Dev & 0xff) << 24 | (Patch &
  // 0xff)
  // << 16 | (Minor & 0xff) << 8 | (Major & 0xff).
  uint32_t firmware_version = 0;
  // Version of the gimbal hardware, encoded as: (Dev & 0xff) << 24 | (Patch &
  // 0xff)
  // << 16 | (Minor & 0xff) << 8 | (Major & 0xff).
  uint32_t hardware_version = 0;
  // UID of gimbal hardware (0 if unknown).
  uint64_t uid = 0;
  // Bitmap of gimbal capability flags.
  uint16_t cap_flags = 0;
  // Bitmap for use for gimbal-specific capability flags.
  uint16_t custom_cap_flags = 0;
  // Minimum hardware roll angle (positive: rolling to the right, negative:
  // rolling to the left)
  float roll_min = 0;
  // Maximum hardware roll angle (positive: rolling to the right, negative:
  // rolling to the left)
  float roll_max = 0;
  // Minimum hardware pitch angle (positive: up, negative: down)
  float pitch_min = 0;
  // Maximum hardware pitch angle (positive: up, negative: down)
  float pitch_max = 0;
  // Minimum hardware yaw angle (positive: to the right, negative: to the left)
  float yaw_min = 0;
  // Maximum hardware yaw angle (positive: to the right, negative: to the left)
  float yaw_max = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Low level message to control a gimbal device's attitude. This message is to
// be sent from the gimbal manager to the gimbal device component. Angles and
// rates can be set to NaN according to use case.
class MavLinkGimbalDeviceSetAttitude : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 284;
  MavLinkGimbalDeviceSetAttitude() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Low level gimbal flags.
  uint16_t flags = 0;
  // Quaternion components, w, x, y, z (1 0 0 0 is the null-rotation, the frame
  // is depends on whether the flag GIMBAL_DEVICE_FLAGS_YAW_LOCK is set, set all
  // fields to NaN if only angular velocity should be used)
  float q[4] = {0};
  // X component of angular velocity, positive is rolling to the right, NaN to
  // be ignored.
  float angular_velocity_x = 0;
  // Y component of angular velocity, positive is pitching up, NaN to be
  // ignored.
  float angular_velocity_y = 0;
  // Z component of angular velocity, positive is yawing to the right, NaN to be
  // ignored.
  float angular_velocity_z = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Message reporting the status of a gimbal device. This message should be
// broadcasted by a gimbal device component. The angles encoded in the
// quaternion are relative to absolute North if the flag
// GIMBAL_DEVICE_FLAGS_YAW_LOCK is set (roll: positive is rolling to the right,
// pitch: positive is pitching up, yaw is turn to the right) or relative to the
// vehicle heading if the flag is not set. This message should be broadcast at a
// low regular rate (e.g. 10Hz).
class MavLinkGimbalDeviceAttitudeStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId= 285;
  MavLinkGimbalDeviceAttitudeStatus() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // Current gimbal flags set.
  uint16_t flags = 0;
  // Quaternion components, w, x, y, z (1 0 0 0 is the null-rotation, the frame
  // is depends on whether the flag GIMBAL_DEVICE_FLAGS_YAW_LOCK is set)
  float q[4] = {0};
  // X component of angular velocity (NaN if unknown)
  float angular_velocity_x = 0;
  // Y component of angular velocity (NaN if unknown)
  float angular_velocity_y = 0;
  // Z component of angular velocity (NaN if unknown)
  float angular_velocity_z = 0;
  // Failure flags (0 for no failure)
  uint32_t failure_flags = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Low level message containing autopilot state relevant for a gimbal device.
// This message is to be sent from the gimbal manager to the gimbal device
// component. The data of this message server for the gimbal's estimator
// corrections in particular horizon compensation, as well as the autopilot's
// control intention e.g. feed forward angular control in z-axis.
class MavLinkAutopilotStateForGimbalDevice : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId= 286;
  MavLinkAutopilotStateForGimbalDevice() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Timestamp (time since system boot).
  uint64_t time_boot_us = 0;
  // Quaternion components of autopilot attitude: w, x, y, z (1 0 0 0 is the
  // null-rotation, Hamilton convention).
  float q[4] = {0};
  // Estimated delay of the attitude data.
  uint32_t q_estimated_delay_us = 0;
  // X Speed in NED (North, East, Down).
  float vx = 0;
  // Y Speed in NED (North, East, Down).
  float vy = 0;
  // Z Speed in NED (North, East, Down).
  float vz = 0;
  // Estimated delay of the speed data.
  uint32_t v_estimated_delay_us = 0;
  // Feed forward Z component of angular velocity, positive is yawing to the
  // right, NaN to be ignored. This is to indicate if the autopilot is actively
  // yawing.
  float feed_forward_angular_velocity_z = 0;
  // Bitmap indicating which estimator outputs are valid.
  uint16_t estimator_status = 0;
  // The landed state. Is set to MAV_LANDED_STATE_UNDEFINED if landed state is
  // unknown.
  uint8_t landed_state = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// High level message to control a gimbal's pitch and yaw angles. This message
// is to be sent to the gimbal manager (e.g. from a ground station). Angles and
// rates can be set to NaN according to use case.
class MavLinkGimbalManagerSetPitchyaw : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 287;
  MavLinkGimbalManagerSetPitchyaw() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // High level gimbal manager flags to use.
  uint32_t flags = 0;
  // Component ID of gimbal device to address (or 1-6 for non-MAVLink gimbal), 0
  // for all gimbal device components. Send command multiple times for more than
  // one gimbal (but not all gimbals).
  uint8_t gimbal_device_id = 0;
  // Pitch angle (positive: up, negative: down, NaN to be ignored).
  float pitch = 0;
  // Yaw angle (positive: to the right, negative: to the left, NaN to be
  // ignored).
  float yaw = 0;
  // Pitch angular rate (positive: up, negative: down, NaN to be ignored).
  float pitch_rate = 0;
  // Yaw angular rate (positive: to the right, negative: to the left, NaN to be
  // ignored).
  float yaw_rate = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// High level message to control a gimbal manually. The angles or angular rates
// are unitless; the actual rates will depend on internal gimbal manager
// settings/configuration (e.g. set by parameters). This message is to be sent
// to the gimbal manager (e.g. from a ground station). Angles and rates can be
// set to NaN according to use case.
class MavLinkGimbalManagerSetManualControl : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 288;
  MavLinkGimbalManagerSetManualControl() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // High level gimbal manager flags.
  uint32_t flags = 0;
  // Component ID of gimbal device to address (or 1-6 for non-MAVLink gimbal), 0
  // for all gimbal device components. Send command multiple times for more than
  // one gimbal (but not all gimbals).
  uint8_t gimbal_device_id = 0;
  // Pitch angle unitless (-1..1, positive: up, negative: down, NaN to be
  // ignored).
  float pitch = 0;
  // Yaw angle unitless (-1..1, positive: to the right, negative: to the left,
  // NaN to be ignored).
  float yaw = 0;
  // Pitch angular rate unitless (-1..1, positive: up, negative: down, NaN to be
  // ignored).
  float pitch_rate = 0;
  // Yaw angular rate unitless (-1..1, positive: to the right, negative: to the
  // left, NaN to be ignored).
  float yaw_rate = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// ESC information for lower rate streaming. Recommended streaming rate 1Hz. See
// ESC_STATUS for higher-rate ESC data.
class MavLinkEscInfo : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 290;
  MavLinkEscInfo() { msgid = kMessageId; }
  // Index of the first ESC in this message. minValue = 0, maxValue = 60,
  // increment = 4.
  uint8_t index = 0;
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude the number.
  uint64_t time_usec = 0;
  // Counter of data packets received.
  uint16_t counter = 0;
  // Total number of ESCs in all messages of this type. Message fields with an
  // index higher than this should be ignored because they contain invalid data.
  uint8_t count = 0;
  // Connection type protocol for all ESC.
  uint8_t connection_type = 0;
  // Information regarding online/offline status of each ESC.
  uint8_t info = 0;
  // Bitmap of ESC failure flags.
  uint16_t failure_flags[4] = {0};
  // Number of reported errors by each ESC since boot.
  uint32_t error_count[4] = {0};
  // Temperature of each ESC. INT16_MAX: if data not supplied by ESC.
  int16_t temperature[4] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// ESC information for higher rate streaming. Recommended streaming rate is ~10
// Hz. Information that changes more slowly is sent in ESC_INFO. It should
// typically only be streamed on high-bandwidth links (i.e. to a companion
// computer).
class MavLinkEscStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 291;
  MavLinkEscStatus() { msgid = kMessageId; }
  // Index of the first ESC in this message. minValue = 0, maxValue = 60,
  // increment = 4.
  uint8_t index = 0;
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude the number.
  uint64_t time_usec = 0;
  // Reported motor RPM from each ESC (negative for reverse rotation).
  int32_t rpm[4] = {0};
  // Voltage measured from each ESC.
  float voltage[4] = {0};
  // Current measured from each ESC.
  float current[4] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Configure WiFi AP SSID, password, and mode. This message is re-emitted as an
// acknowledgement by the AP. The message may also be explicitly requested using
// MAV_CMD_REQUEST_MESSAGE
class MavLinkWifiConfigAp : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 299;
  MavLinkWifiConfigAp() { msgid = kMessageId; }
  // Name of Wi-Fi network (SSID). Blank to leave it unchanged when setting.
  // Current SSID when sent back as a response.
  char ssid[32] = {0};
  // Password. Blank for an open AP. MD5 hash when message is sent back as a
  // response.
  char password[64] = {0};
  // WiFi Mode.
  int8_t mode = 0;
  // Message acceptance response (sent back to GS).
  int8_t response = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Version and capability of protocol version. This message can be requested
// with MAV_CMD_REQUEST_MESSAGE and is used as part of the handshaking to
// establish which MAVLink version should be used on the network. Every node
// should respond to a request for PROTOCOL_VERSION to enable the handshaking.
// Library implementers should consider adding this into the default decoding
// state machine to allow the protocol core to respond directly.
class MavLinkProtocolVersion : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 300;
  MavLinkProtocolVersion() { msgid = kMessageId; }
  // Currently active MAVLink version number * 100: v1.0 is 100, v2.0 is 200,
  // etc.
  uint16_t version = 0;
  // Minimum MAVLink version supported
  uint16_t min_version = 0;
  // Maximum MAVLink version supported (set to the same value as version by
  // default)
  uint16_t max_version = 0;
  // The first 8 bytes (not characters printed in hex!) of the git hash.
  uint8_t spec_version_hash[8] = {0};
  // The first 8 bytes (not characters printed in hex!) of the git hash.
  uint8_t library_version_hash[8] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The location and information of an AIS vessel
class MavLinkAisVessel : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 301;
  MavLinkAisVessel() { msgid = kMessageId; }
  // Mobile Marine Service Identifier, 9 decimal digits
  uint32_t MMSI = 0;
  // Latitude
  int32_t lat = 0;
  // Longitude
  int32_t lon = 0;
  // Course over ground
  uint16_t COG = 0;
  // True heading
  uint16_t heading = 0;
  // Speed over ground
  uint16_t velocity = 0;
  // Turn rate
  int8_t turn_rate = 0;
  // Navigational status
  uint8_t navigational_status = 0;
  // Type of vessels
  uint8_t type = 0;
  // Distance from lat/lon location to bow
  uint16_t dimension_bow = 0;
  // Distance from lat/lon location to stern
  uint16_t dimension_stern = 0;
  // Distance from lat/lon location to port side
  uint8_t dimension_port = 0;
  // Distance from lat/lon location to starboard side
  uint8_t dimension_starboard = 0;
  // The vessel callsign
  char callsign[7] = {0};
  // The vessel name
  char name[20] = {0};
  // Time since last communication in seconds
  uint16_t tslc = 0;
  // Bitmask to indicate various statuses including valid data fields
  uint16_t flags = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// General status information of an UAVCAN node. Please refer to the definition
// of the UAVCAN message "uavcan.protocol.NodeStatus" for the background
// information. The UAVCAN specification is available at http://uavcan.org.
class MavLinkUavcanNodeStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 310;
  MavLinkUavcanNodeStatus() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Time since the start-up of the node.
  uint32_t uptime_sec = 0;
  // Generalized node health status.
  uint8_t health = 0;
  // Generalized operating mode.
  uint8_t mode = 0;
  // Not used currently.
  uint8_t sub_mode = 0;
  // Vendor-specific status information.
  uint16_t vendor_specific_status_code = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// General information describing a particular UAVCAN node. Please refer to the
// definition of the UAVCAN service "uavcan.protocol.GetNodeInfo" for the
// background information. This message should be emitted by the system whenever
// a new node appears online, or an existing node reboots. Additionally, it can
// be emitted upon request from the other end of the MAVLink channel (see
// MAV_CMD_UAVCAN_GET_NODE_INFO). It is also not prohibited to emit this message
// unconditionally at a low frequency. The UAVCAN specification is available at
// http://uavcan.org.
class MavLinkUavcanNodeInfo : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 311;
  MavLinkUavcanNodeInfo() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Time since the start-up of the node.
  uint32_t uptime_sec = 0;
  // Node name string. For example, "sapog.px4.io".
  char name[80] = {0};
  // Hardware major version number.
  uint8_t hw_version_major = 0;
  // Hardware minor version number.
  uint8_t hw_version_minor = 0;
  // Hardware unique 128-bit ID.
  uint8_t hw_unique_id[16] = {0};
  // Software major version number.
  uint8_t sw_version_major = 0;
  // Software minor version number.
  uint8_t sw_version_minor = 0;
  // Version control system (VCS) revision identifier (e.g. git short commit
  // hash). 0 if unknown.
  uint32_t sw_vcs_commit = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request to read the value of a parameter with either the param_id string id
// or param_index. PARAM_EXT_VALUE should be emitted in response.
class MavLinkParamExtRequestRead : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 320;
  MavLinkParamExtRequestRead() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Parameter id, terminated by NULL if the length is less than 16
  // human-readable chars and WITHOUT null termination (NULL) byte if the length
  // is exactly 16 chars - applications have to provide 16+1 bytes storage if
  // the ID is stored as string
  char param_id[16] = {0};
  // Parameter index. Set to -1 to use the Parameter ID field as identifier
  // (else param_id will be ignored)
  int16_t param_index = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request all parameters of this component. All parameters should be emitted in
// response as PARAM_EXT_VALUE.
class MavLinkParamExtRequestList : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 321;
  MavLinkParamExtRequestList() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Emit the value of a parameter. The inclusion of param_count and param_index
// in the message allows the recipient to keep track of received parameters and
// allows them to re-request missing parameters after a loss or timeout.
class MavLinkParamExtValue : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 322;
  MavLinkParamExtValue() { msgid = kMessageId; }
  // Parameter id, terminated by NULL if the length is less than 16
  // human-readable chars and WITHOUT null termination (NULL) byte if the length
  // is exactly 16 chars - applications have to provide 16+1 bytes storage if
  // the ID is stored as string
  char param_id[16] = {0};
  // Parameter value
  char param_value[128] = {0};
  // Parameter type.
  uint8_t param_type = 0;
  // Total number of parameters
  uint16_t param_count = 0;
  // Index of this parameter
  uint16_t param_index = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Set a parameter value. In order to deal with message loss (and retransmission
// of PARAM_EXT_SET), when setting a parameter value and the new value is the
// same as the current value, you will immediately get a PARAM_ACK_ACCEPTED
// response. If the current state is PARAM_ACK_IN_PROGRESS, you will accordingly
// receive a PARAM_ACK_IN_PROGRESS in response.
class MavLinkParamExtSet : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 323;
  MavLinkParamExtSet() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Parameter id, terminated by NULL if the length is less than 16
  // human-readable chars and WITHOUT null termination (NULL) byte if the length
  // is exactly 16 chars - applications have to provide 16+1 bytes storage if
  // the ID is stored as string
  char param_id[16] = {0};
  // Parameter value
  char param_value[128] = {0};
  // Parameter type.
  uint8_t param_type = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Response from a PARAM_EXT_SET message.
class MavLinkParamExtAck : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 324;
  MavLinkParamExtAck() { msgid = kMessageId; }
  // Parameter id, terminated by NULL if the length is less than 16
  // human-readable chars and WITHOUT null termination (NULL) byte if the length
  // is exactly 16 chars - applications have to provide 16+1 bytes storage if
  // the ID is stored as string
  char param_id[16] = {0};
  // Parameter value (new value if PARAM_ACK_ACCEPTED, current value otherwise)
  char param_value[128] = {0};
  // Parameter type.
  uint8_t param_type = 0;
  // Result code.
  uint8_t param_result = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Obstacle distances in front of the sensor, starting from the left in
// increment degrees to the right
class MavLinkObstacleDistance : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 330;
  MavLinkObstacleDistance() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Class id of the distance sensor type.
  uint8_t sensor_type = 0;
  // Distance of obstacles around the vehicle with index 0 corresponding to
  // north
  // + angle_offset, unless otherwise specified in the frame. A value of 0 is
  // valid and means that the obstacle is practically touching the sensor. A
  // value of max_distance +1 means no obstacle is present. A value of
  // UINT16_MAX for unknown/not used. In a array element, one unit corresponds
  // to 1cm.
  uint16_t distances[72] = {0};
  // Angular width in degrees of each array element. Increment direction is
  // clockwise. This field is ignored if increment_f is non-zero.
  uint8_t increment = 0;
  // Minimum distance the sensor can measure.
  uint16_t min_distance = 0;
  // Maximum distance the sensor can measure.
  uint16_t max_distance = 0;
  // Angular width in degrees of each array element as a float. If non-zero then
  // this value is used instead of the uint8_t increment field. Positive is
  // clockwise direction, negative is counter-clockwise.
  float increment_f = 0;
  // Relative angle offset of the 0-index element in the distances array. Value
  // of 0 corresponds to forward. Positive is clockwise direction, negative is
  // counter-clockwise.
  float angle_offset = 0;
  // Coordinate frame of reference for the yaw rotation and offset of the sensor
  // data. Defaults to MAV_FRAME_GLOBAL, which is north aligned. For
  // body-mounted sensors use MAV_FRAME_BODY_FRD, which is vehicle front
  // aligned.
  uint8_t frame = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Odometry message to communicate odometry information with an external
// interface. Fits ROS REP 147 standard for aerial vehicles
// (http://www.ros.org/reps/rep-0147.html).
class MavLinkOdometry : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 331;
  MavLinkOdometry() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Coordinate frame of reference for the pose data.
  uint8_t frame_id = 0;
  // Coordinate frame of reference for the velocity in free space (twist) data.
  uint8_t child_frame_id = 0;
  // X Position
  float x = 0;
  // Y Position
  float y = 0;
  // Z Position
  float z = 0;
  // Quaternion components, w, x, y, z (1 0 0 0 is the null-rotation)
  float q[4] = {0};
  // X linear speed
  float vx = 0;
  // Y linear speed
  float vy = 0;
  // Z linear speed
  float vz = 0;
  // Roll angular speed
  float rollspeed = 0;
  // Pitch angular speed
  float pitchspeed = 0;
  // Yaw angular speed
  float yawspeed = 0;
  // Row-major representation of a 6x6 pose cross-covariance matrix upper right
  // triangle (states: x, y, z, roll, pitch, yaw; first six entries are the
  // first ROW, next five entries are the second ROW, etc.). If unknown, assign
  // NaN value to first element in the array.
  float pose_covariance[21] = {0};
  // Row-major representation of a 6x6 velocity cross-covariance matrix upper
  // right triangle (states: vx, vy, vz, rollspeed, pitchspeed, yawspeed; first
  // six entries are the first ROW, next five entries are the second ROW, etc.).
  // If unknown, assign NaN value to first element in the array.
  float velocity_covariance[21] = {0};
  // Estimate reset counter. This should be incremented when the estimate resets
  // in any of the dimensions (position, velocity, attitude, angular speed).
  // This is designed to be used when e.g an external SLAM system detects a
  // loop-closure and the estimate jumps.
  uint8_t reset_counter = 0;
  // Type of estimator that is providing the odometry.
  uint8_t estimator_type = 0;
  // Optional odometry quality metric as a percentage. -1 = odometry has failed,
  // 0 = unknown/unset quality, 1 = worst quality, 100 = best quality
  int8_t quality = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Describe a trajectory using an array of up-to 5 waypoints in the local frame
// (MAV_FRAME_LOCAL_NED).
class MavLinkTrajectoryRepresentationWaypoints : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 332;
  MavLinkTrajectoryRepresentationWaypoints() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Number of valid points (up-to 5 waypoints are possible)
  uint8_t valid_points = 0;
  // X-coordinate of waypoint, set to NaN if not being used
  float pos_x[5] = {0};
  // Y-coordinate of waypoint, set to NaN if not being used
  float pos_y[5] = {0};
  // Z-coordinate of waypoint, set to NaN if not being used
  float pos_z[5] = {0};
  // X-velocity of waypoint, set to NaN if not being used
  float vel_x[5] = {0};
  // Y-velocity of waypoint, set to NaN if not being used
  float vel_y[5] = {0};
  // Z-velocity of waypoint, set to NaN if not being used
  float vel_z[5] = {0};
  // X-acceleration of waypoint, set to NaN if not being used
  float acc_x[5] = {0};
  // Y-acceleration of waypoint, set to NaN if not being used
  float acc_y[5] = {0};
  // Z-acceleration of waypoint, set to NaN if not being used
  float acc_z[5] = {0};
  // Yaw angle, set to NaN if not being used
  float pos_yaw[5] = {0};
  // Yaw rate, set to NaN if not being used
  float vel_yaw[5] = {0};
  // MAV_CMD command id of waypoint, set to UINT16_MAX if not being used.
  uint16_t command[5] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Describe a trajectory using an array of up-to 5 bezier control points in the
// local frame (MAV_FRAME_LOCAL_NED).
class MavLinkTrajectoryRepresentationBezier : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 333;
  MavLinkTrajectoryRepresentationBezier() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Number of valid control points (up-to 5 points are possible)
  uint8_t valid_points = 0;
  // X-coordinate of bezier control points. Set to NaN if not being used
  float pos_x[5] = {0};
  // Y-coordinate of bezier control points. Set to NaN if not being used
  float pos_y[5] = {0};
  // Z-coordinate of bezier control points. Set to NaN if not being used
  float pos_z[5] = {0};
  // Bezier time horizon. Set to NaN if velocity/acceleration should not be
  // incorporated
  float delta[5] = {0};
  // Yaw. Set to NaN for unchanged
  float pos_yaw[5] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Report current used cellular network status
class MavLinkCellularStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 334;
  MavLinkCellularStatus() { msgid = kMessageId; }
  // Cellular modem status
  uint8_t status = 0;
  // Failure reason when status in in CELLUAR_STATUS_FAILED
  uint8_t failure_reason = 0;
  // Cellular network radio type: gsm, cdma, lte...
  uint8_t type = 0;
  // Signal quality in percent. If unknown, set to UINT8_MAX
  uint8_t quality = 0;
  // Mobile country code. If unknown, set to UINT16_MAX
  uint16_t mcc = 0;
  // Mobile network code. If unknown, set to UINT16_MAX
  uint16_t mnc = 0;
  // Location area code. If unknown, set to 0
  uint16_t lac = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Status of the Iridium SBD link.
class MavLinkIsbdLinkStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 335;
  MavLinkIsbdLinkStatus() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t timestamp = 0;
  // Timestamp of the last successful sbd session. The receiving end can infer
  // timestamp format (since 1.1.1970 or since system boot) by checking for the
  // magnitude of the number.
  uint64_t last_heartbeat = 0;
  // Number of failed SBD sessions.
  uint16_t failed_sessions = 0;
  // Number of successful SBD sessions.
  uint16_t successful_sessions = 0;
  // Signal quality equal to the number of bars displayed on the ISU signal
  // strength indicator. Range is 0 to 5, where 0 indicates no signal and 5
  // indicates maximum signal strength.
  uint8_t signal_quality = 0;
  // 1: Ring call pending, 0: No call pending.
  uint8_t ring_pending = 0;
  // 1: Transmission session pending, 0: No transmission session pending.
  uint8_t tx_session_pending = 0;
  // 1: Receiving session pending, 0: No receiving session pending.
  uint8_t rx_session_pending = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Configure cellular modems. This message is re-emitted as an acknowledgement
// by the modem. The message may also be explicitly requested using
// MAV_CMD_REQUEST_MESSAGE.
class MavLinkCellularConfig : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 336;
  MavLinkCellularConfig() { msgid = kMessageId; }
  // Enable/disable LTE. 0: setting unchanged, 1: disabled, 2: enabled. Current
  // setting when sent back as a response.
  uint8_t enable_lte = 0;
  // Enable/disable PIN on the SIM card. 0: setting unchanged, 1: disabled, 2:
  // enabled. Current setting when sent back as a response.
  uint8_t enable_pin = 0;
  // PIN sent to the SIM card. Blank when PIN is disabled. Empty when message is
  // sent back as a response.
  char pin[16] = {0};
  // New PIN when changing the PIN. Blank to leave it unchanged. Empty when
  // message is sent back as a response.
  char new_pin[16] = {0};
  // Name of the cellular APN. Blank to leave it unchanged. Current APN when
  // sent back as a response.
  char apn[32] = {0};
  // Required PUK code in case the user failed to authenticate 3 times with the
  // PIN. Empty when message is sent back as a response.
  char puk[16] = {0};
  // Enable/disable roaming. 0: setting unchanged, 1: disabled, 2: enabled.
  // Current setting when sent back as a response.
  uint8_t roaming = 0;
  // Message acceptance response (sent back to GS).
  uint8_t response = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// RPM sensor data message.
class MavLinkRawRpm : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 339;
  MavLinkRawRpm() { msgid = kMessageId; }
  // Index of this RPM sensor (0-indexed)
  uint8_t index = 0;
  // Indicated rate
  float frequency = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The global position resulting from GPS and sensor fusion.
class MavLinkUtmGlobalPosition : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 340;
  MavLinkUtmGlobalPosition() { msgid = kMessageId; }
  // Time of applicability of position (microseconds since UNIX epoch).
  uint64_t time = 0;
  // Unique UAS ID.
  uint8_t uas_id[18] = {0};
  // Latitude (WGS84)
  int32_t lat = 0;
  // Longitude (WGS84)
  int32_t lon = 0;
  // Altitude (WGS84)
  int32_t alt = 0;
  // Altitude above ground
  int32_t relative_alt = 0;
  // Ground X speed (latitude, positive north)
  int16_t vx = 0;
  // Ground Y speed (longitude, positive east)
  int16_t vy = 0;
  // Ground Z speed (altitude, positive down)
  int16_t vz = 0;
  // Horizontal position uncertainty (standard deviation)
  uint16_t h_acc = 0;
  // Altitude uncertainty (standard deviation)
  uint16_t v_acc = 0;
  // Speed uncertainty (standard deviation)
  uint16_t vel_acc = 0;
  // Next waypoint, latitude (WGS84)
  int32_t next_lat = 0;
  // Next waypoint, longitude (WGS84)
  int32_t next_lon = 0;
  // Next waypoint, altitude (WGS84)
  int32_t next_alt = 0;
  // Time until next update. Set to 0 if unknown or in data driven mode.
  uint16_t update_rate = 0;
  // Flight state
  uint8_t flight_state = 0;
  // Bitwise OR combination of the data available flags.
  uint8_t flags = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Large debug/prototyping array. The message uses the maximum available payload
// for data. The array_id and name fields are used to discriminate between
// messages in code and in user interfaces (respectively). Do not use in
// production code.
class MavLinkDebugFloatArray : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 350;
  MavLinkDebugFloatArray() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Name, for human-friendly display in a Ground Control Station
  char name[10] = {0};
  // Unique ID used to discriminate between arrays
  uint16_t array_id = 0;
  // data
  float data[58] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Vehicle status report that is sent out while orbit execution is in progress
// (see MAV_CMD_DO_ORBIT).
class MavLinkOrbitExecutionStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 360;
  MavLinkOrbitExecutionStatus() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Radius of the orbit circle. Positive values orbit clockwise, negative
  // values orbit counter-clockwise.
  float radius = 0;
  // The coordinate system of the fields: x, y, z.
  uint8_t frame = 0;
  // X coordinate of center point. Coordinate system depends on frame field:
  // local = x position in meters * 1e4, global = latitude in degrees * 1e7.
  int32_t x = 0;
  // Y coordinate of center point. Coordinate system depends on frame field:
  // local = x position in meters * 1e4, global = latitude in degrees * 1e7.
  int32_t y = 0;
  // Altitude of center point. Coordinate system depends on frame field.
  float z = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Smart Battery information (static/infrequent update). Use for updates from:
// smart battery to flight stack, flight stack to GCS. Use BATTERY_STATUS for
// smart battery frequent updates.
class MavLinkSmartBatteryInfo : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 370;
  MavLinkSmartBatteryInfo() { msgid = kMessageId; }
  // Battery ID
  uint8_t id = 0;
  // Function of the battery
  uint8_t battery_function = 0;
  // Type (chemistry) of the battery
  uint8_t type = 0;
  // Capacity when full according to manufacturer, -1: field not provided.
  int32_t capacity_full_specification = 0;
  // Capacity when full (accounting for battery degradation), -1: field not
  // provided.
  int32_t capacity_full = 0;
  // Charge/discharge cycle count. UINT16_MAX: field not provided.
  uint16_t cycle_count = 0;
  // Serial number in ASCII characters, 0 terminated. All 0: field not provided.
  char serial_number[16] = {0};
  // Static device name in ASCII characters, 0 terminated. All 0: field not
  // provided. Encode as manufacturer name then product name separated using an
  // underscore.
  char device_name[50] = {0};
  // Battery weight. 0: field not provided.
  uint16_t weight = 0;
  // Minimum per-cell voltage when discharging. If not supplied set to
  // UINT16_MAX value.
  uint16_t discharge_minimum_voltage = 0;
  // Minimum per-cell voltage when charging. If not supplied set to UINT16_MAX
  // value.
  uint16_t charging_minimum_voltage = 0;
  // Minimum per-cell voltage when resting. If not supplied set to UINT16_MAX
  // value.
  uint16_t resting_minimum_voltage = 0;
  // Maximum per-cell voltage when charged. 0: field not provided.
  uint16_t charging_maximum_voltage = 0;
  // Number of battery cells in series. 0: field not provided.
  uint8_t cells_in_series = 0;
  // Maximum pack discharge current. 0: field not provided.
  uint32_t discharge_maximum_current = 0;
  // Maximum pack discharge burst current. 0: field not provided.
  uint32_t discharge_maximum_burst_current = 0;
  // Manufacture date (DD/MM/YYYY) in ASCII characters, 0 terminated. All 0:
  // field not provided.
  char manufacture_date[11] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Telemetry of power generation system. Alternator or mechanical generator.
class MavLinkGeneratorStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 373;
  MavLinkGeneratorStatus() { msgid = kMessageId; }
  // Status flags.
  uint64_t status = 0;
  // Speed of electrical generator or alternator. UINT16_MAX: field not
  // provided.
  uint16_t generator_speed = 0;
  // Current into/out of battery. Positive for out. Negative for in. NaN: field
  // not provided.
  float battery_current = 0;
  // Current going to the UAV. If battery current not available this is the DC
  // current from the generator. Positive for out. Negative for in. NaN: field
  // not provided
  float load_current = 0;
  // The power being generated. NaN: field not provided
  float power_generated = 0;
  // Voltage of the bus seen at the generator, or battery bus if battery bus is
  // controlled by generator and at a different voltage to main bus.
  float bus_voltage = 0;
  // The temperature of the rectifier or power converter. INT16_MAX: field not
  // provided.
  int16_t rectifier_temperature = 0;
  // The target battery current. Positive for out. Negative for in. NaN: field
  // not provided
  float bat_current_setpoint = 0;
  // The temperature of the mechanical motor, fuel cell core or generator.
  // INT16_MAX: field not provided.
  int16_t generator_temperature = 0;
  // Seconds this generator has run since it was rebooted. UINT32_MAX: field not
  // provided.
  uint32_t runtime = 0;
  // Seconds until this generator requires maintenance. A negative value
  // indicates maintenance is past-due. INT32_MAX: field not provided.
  int32_t time_until_maintenance = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// The raw values of the actuator outputs (e.g. on Pixhawk, from MAIN, AUX
// ports). This message supersedes SERVO_OUTPUT_RAW.
class MavLinkActuatorOutputStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 375;
  MavLinkActuatorOutputStatus() { msgid = kMessageId; }
  // Timestamp (since system boot).
  uint64_t time_usec = 0;
  // Active outputs
  uint32_t active = 0;
  // Servo / motor output array values. Zero values indicate unused channels.
  float actuator[32] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Time/duration estimates for various events and actions given the current
// vehicle state and position.
class MavLinkTimeEstimateToTarget : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 380;
  MavLinkTimeEstimateToTarget() { msgid = kMessageId; }
  // Estimated time to complete the vehicle's configured "safe return" action
  // from its current position (e.g. RTL, Smart RTL, etc.). -1 indicates that
  // the vehicle is landed, or that no time estimate available.
  int32_t safe_return = 0;
  // Estimated time for vehicle to complete the LAND action from its current
  // position. -1 indicates that the vehicle is landed, or that no time estimate
  // available.
  int32_t land = 0;
  // Estimated time for reaching/completing the currently active mission item.
  // -1 means no time estimate available.
  int32_t mission_next_item = 0;
  // Estimated time for completing the current mission. -1 means no mission
  // active and/or no estimate available.
  int32_t mission_end = 0;
  // Estimated time for completing the current commanded action (i.e. Go To,
  // Takeoff, Land, etc.). -1 means no action active and/or no estimate
  // available.
  int32_t commanded_action = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Message for transporting "arbitrary" variable-length data from one component
// to another (broadcast is not forbidden, but discouraged). The encoding of the
// data is usually extension specific, i.e. determined by the source, and is
// usually not documented as part of the MAVLink specification.
class MavLinkTunnel : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 385;
  MavLinkTunnel() { msgid = kMessageId; }
  // System ID (can be 0 for broadcast, but this is discouraged)
  uint8_t target_system = 0;
  // Component ID (can be 0 for broadcast, but this is discouraged)
  uint8_t target_component = 0;
  // A code that identifies the content of the payload (0 for unknown, which is
  // the default). If this code is less than 32768, it is a 'registered' payload
  // type and the corresponding code should be added to the
  // MAV_TUNNEL_PAYLOAD_TYPE enum. Software creators can register blocks of
  // types as needed. Codes greater than 32767 are considered local experiments
  // and should not be checked in to any widely distributed codebase.
  uint16_t payload_type = 0;
  // Length of the data transported in payload
  uint8_t payload_length = 0;
  // Variable length payload. The payload length is defined by payload_length.
  // The entire content of this block is opaque unless you understand the
  // encoding specified by payload_type.
  uint8_t payload[128] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// A forwarded CAN frame as requested by MAV_CMD_CAN_FORWARD.
class MavLinkCanFrame : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 386;
  MavLinkCanFrame() { msgid = kMessageId; }
  // System ID.
  uint8_t target_system = 0;
  // Component ID.
  uint8_t target_component = 0;
  // Bus number
  uint8_t bus = 0;
  // Frame length
  uint8_t len = 0;
  // Frame ID
  uint32_t id = 0;
  // Frame data
  uint8_t data[8] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Hardware status sent by an onboard computer.
class MavLinkOnboardComputerStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 390;
  MavLinkOnboardComputerStatus() { msgid = kMessageId; }
  // Timestamp (UNIX Epoch time or time since system boot). The receiving end
  // can infer timestamp format (since 1.1.1970 or since system boot) by
  // checking for the magnitude of the number.
  uint64_t time_usec = 0;
  // Time since system boot.
  uint32_t uptime = 0;
  // Type of the onboard computer: 0: Mission computer primary, 1: Mission
  // computer backup 1, 2: Mission computer backup 2, 3: Compute node, 4-5:
  // Compute spares, 6-9: Payload computers.
  uint8_t type = 0;
  // CPU usage on the component in percent (100 - idle). A value of UINT8_MAX
  // implies the field is unused.
  uint8_t cpu_cores[8] = {0};
  // Combined CPU usage as the last 10 slices of 100 MS (a histogram). This
  // allows to identify spikes in load that max out the system, but only for a
  // short amount of time. A value of UINT8_MAX implies the field is unused.
  uint8_t cpu_combined[10] = {0};
  // GPU usage on the component in percent (100 - idle). A value of UINT8_MAX
  // implies the field is unused.
  uint8_t gpu_cores[4] = {0};
  // Combined GPU usage as the last 10 slices of 100 MS (a histogram). This
  // allows to identify spikes in load that max out the system, but only for a
  // short amount of time. A value of UINT8_MAX implies the field is unused.
  uint8_t gpu_combined[10] = {0};
  // Temperature of the board. A value of INT8_MAX implies the field is unused.
  int8_t temperature_board = 0;
  // Temperature of the CPU core. A value of INT8_MAX implies the field is
  // unused.
  int8_t temperature_core[8] = {0};
  // Fan speeds. A value of INT16_MAX implies the field is unused.
  int16_t fan_speed[4] = {0};
  // Amount of used RAM on the component system. A value of UINT32_MAX implies
  // the field is unused.
  uint32_t ram_usage = 0;
  // Total amount of RAM on the component system. A value of UINT32_MAX implies
  // the field is unused.
  uint32_t ram_total = 0;
  // Storage type: 0: HDD, 1: SSD, 2: EMMC, 3: SD card (non-removable), 4: SD
  // card (removable). A value of UINT32_MAX implies the field is unused.
  uint32_t storage_type[4] = {0};
  // Amount of used storage space on the component system. A value of UINT32_MAX
  // implies the field is unused.
  uint32_t storage_usage[4] = {0};
  // Total amount of storage space on the component system. A value of
  // UINT32_MAX implies the field is unused.
  uint32_t storage_total[4] = {0};
  // Link type: 0-9: UART, 10-19: Wired network, 20-29: Wifi, 30-39:
  // Point-to-point proprietary, 40-49: Mesh proprietary
  uint32_t link_type[6] = {0};
  // Network traffic from the component system. A value of UINT32_MAX implies
  // the field is unused.
  uint32_t link_tx_rate[6] = {0};
  // Network traffic to the component system. A value of UINT32_MAX implies the
  // field is unused.
  uint32_t link_rx_rate[6] = {0};
  // Network capacity from the component system. A value of UINT32_MAX implies
  // the field is unused.
  uint32_t link_tx_max[6] = {0};
  // Network capacity to the component system. A value of UINT32_MAX implies the
  // field is unused.
  uint32_t link_rx_max[6] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Component information message, which may be requested using
// MAV_CMD_REQUEST_MESSAGE.
class MavLinkComponentInformation : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 395;
  MavLinkComponentInformation() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // CRC32 of the general metadata file (general_metadata_uri).
  uint32_t general_metadata_file_crc = 0;
  // MAVLink FTP URI for the general metadata file (COMP_METADATA_TYPE_GENERAL),
  // which may be compressed with xz. The file contains general component
  // metadata, and may contain URI links for additional metadata (see
  // COMP_METADATA_TYPE). The information is static from boot, and may be
  // generated at compile time. The string needs to be zero terminated.
  char general_metadata_uri[100] = {0};
  // CRC32 of peripherals metadata file (peripherals_metadata_uri).
  uint32_t peripherals_metadata_file_crc = 0;
  // (Optional) MAVLink FTP URI for the peripherals metadata file
  // (COMP_METADATA_TYPE_PERIPHERALS), which may be compressed with xz. This
  // contains data about "attached components" such as UAVCAN nodes. The
  // peripherals are in a separate file because the information must be
  // generated dynamically at runtime. The string needs to be zero terminated.
  char peripherals_metadata_uri[100] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Component metadata message, which may be requested using
// MAV_CMD_REQUEST_MESSAGE. This contains the MAVLink FTP URI and CRC for the
// component's general metadata file. The file must be hosted on the component,
// and may be xz compressed. The file CRC can be used for file caching. The
// general metadata file can be read to get the locations of other metadata
// files (COMP_METADATA_TYPE) and translations, which may be hosted either on
// the vehicle or the internet. For more information see:
// https://mavlink.io/en/services/component_information.html. Note: Camera
// components should use CAMERA_INFORMATION instead, and autopilots may use both
// this message and AUTOPILOT_VERSION.
class MavLinkComponentMetadata : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 397;
  MavLinkComponentMetadata() { msgid = kMessageId; }
  // Timestamp (time since system boot).
  uint32_t time_boot_ms = 0;
  // CRC32 of the general metadata file.
  uint32_t file_crc = 0;
  // MAVLink FTP URI for the general metadata file (COMP_METADATA_TYPE_GENERAL),
  // which may be compressed with xz. The file contains general component
  // metadata, and may contain URI links for additional metadata (see
  // COMP_METADATA_TYPE). The information is static from boot, and may be
  // generated at compile time. The string needs to be zero terminated.
  char uri[100] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Play vehicle tone/tune (buzzer). Supersedes message PLAY_TUNE.
class MavLinkPlayTuneV2 : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 400;
  MavLinkPlayTuneV2() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Tune format
  uint32_t format = 0;
  // Tune definition as a NULL-terminated string.
  char tune[248] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Tune formats supported by vehicle. This should be emitted as response to
// MAV_CMD_REQUEST_MESSAGE.
class MavLinkSupportedTunes : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 401;
  MavLinkSupportedTunes() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Bitfield of supported tune formats.
  uint32_t format = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Event message. Each new event from a particular component gets a new sequence
// number. The same message might be sent multiple times if (re-)requested. Most
// events are broadcast, some can be specific to a target component (as
// receivers keep track of the sequence for missed events, all events need to be
// broadcast. Thus we use destination_component instead of target_component).
class MavLinkEvent : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 410;
  MavLinkEvent() { msgid = kMessageId; }
  // Component ID
  uint8_t destination_component = 0;
  // System ID
  uint8_t destination_system = 0;
  // Event ID (as defined in the component metadata)
  uint32_t id = 0;
  // Timestamp (time since system boot when the event happened).
  uint32_t event_time_boot_ms = 0;
  // Sequence number.
  uint16_t sequence = 0;
  // Log levels: 4 bits MSB: internal (for logging purposes), 4 bits LSB:
  // external. Levels: Emergency = 0, Alert = 1, Critical = 2, Error = 3,
  // Warning = 4, Notice = 5, Info = 6, Debug = 7, Protocol = 8, Disabled = 9
  uint8_t log_levels = 0;
  // Arguments (depend on event ID).
  uint8_t arguments[40] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Regular broadcast for the current latest event sequence number for a
// component. This is used to check for dropped events.
class MavLinkCurrentEventSequence : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 411;
  MavLinkCurrentEventSequence() { msgid = kMessageId; }
  // Sequence number.
  uint16_t sequence = 0;
  // Flag bitset.
  uint8_t flags = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Request one or more events to be (re-)sent. If first_sequence==last_sequence,
// only a single event is requested. Note that first_sequence can be larger than
// last_sequence (because the sequence number can wrap). Each sequence will
// trigger an EVENT or EVENT_ERROR response.
class MavLinkRequestEvent : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 412;
  MavLinkRequestEvent() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // First sequence number of the requested event.
  uint16_t first_sequence = 0;
  // Last sequence number of the requested event.
  uint16_t last_sequence = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Response to a REQUEST_EVENT in case of an error (e.g. the event is not
// available anymore).
class MavLinkResponseEventError : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 413;
  MavLinkResponseEventError() { msgid = kMessageId; }
  // System ID
  uint8_t target_system = 0;
  // Component ID
  uint8_t target_component = 0;
  // Sequence number.
  uint16_t sequence = 0;
  // Oldest Sequence number that is still available after the sequence set in
  // REQUEST_EVENT.
  uint16_t sequence_oldest_available = 0;
  // Error reason.
  uint8_t reason = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// A forwarded CANFD frame as requested by MAV_CMD_CAN_FORWARD. These are
// separated from CAN_FRAME as they need different handling (eg. TAO handling)
class MavLinkCanfdFrame : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 387;
  MavLinkCanfdFrame() { msgid = kMessageId; }
  // System ID.
  uint8_t target_system = 0;
  // Component ID.
  uint8_t target_component = 0;
  // bus number
  uint8_t bus = 0;
  // Frame length
  uint8_t len = 0;
  // Frame ID
  uint32_t id = 0;
  // Frame data
  uint8_t data[64] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Modify the filter of what CAN messages to forward over the mavlink. This can
// be used to make CAN forwarding work well on low bandwith links. The filtering
// is applied on bits 8 to 24 of the CAN id (2nd and 3rd bytes) which
// corresponds to the DroneCAN message ID for DroneCAN. Filters with more than
// 16 IDs can be constructed by sending multiple CAN_FILTER_MODIFY messages.
class MavLinkCanFilterModify : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 388;
  MavLinkCanFilterModify() { msgid = kMessageId; }
  // System ID.
  uint8_t target_system = 0;
  // Component ID.
  uint8_t target_component = 0;
  // bus number
  uint8_t bus = 0;
  // what operation to perform on the filter list. See CAN_FILTER_OP enum.
  uint8_t operation = 0;
  // number of IDs in filter list
  uint8_t num_ids = 0;
  // filter IDs, length num_ids
  uint16_t ids[16] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Cumulative distance traveled for each reported wheel.
class MavLinkWheelDistance : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 9000;
  MavLinkWheelDistance() { msgid = kMessageId; }
  // Timestamp (synced to UNIX time or since system boot).
  uint64_t time_usec = 0;
  // Number of wheels reported.
  uint8_t count = 0;
  // Distance reported by individual wheel encoders. Forward rotations increase
  // values, reverse rotations decrease them. Not all wheels will necessarily
  // have wheel encoders; the mapping of encoders to wheel positions must be
  // agreed/understood by the endpoints.
  double distance[16] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Winch status.
class MavLinkWinchStatus : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 9005;
  MavLinkWinchStatus() { msgid = kMessageId; }
  // Timestamp (synced to UNIX time or since system boot).
  uint64_t time_usec = 0;
  // Length of line released. NaN if unknown
  float line_length = 0;
  // Speed line is being released or retracted. Positive values if being
  // released, negative values if being retracted, NaN if unknown
  float speed = 0;
  // Tension on the line. NaN if unknown
  float tension = 0;
  // Voltage of the battery supplying the winch. NaN if unknown
  float voltage = 0;
  // Current draw from the winch. NaN if unknown
  float current = 0;
  // Temperature of the motor. INT16_MAX if unknown
  int16_t temperature = 0;
  // Status flags
  uint32_t status = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Data for filling the OpenDroneID Basic ID message. This and the below
// messages are primarily meant for feeding data to/from an OpenDroneID
// implementation. E.g. https://github.com/opendroneid/opendroneid-core-c. These
// messages are compatible with the ASTM F3411 Remote ID standard and the
// ASD-STAN prEN 4709-002 Direct Remote ID standard. Additional information and
// usage of these messages is documented at
// https://mavlink.io/en/services/opendroneid.html.
class MavLinkOpenDroneIdBasicId : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 12900;
  MavLinkOpenDroneIdBasicId() { msgid = kMessageId; }
  // System ID (0 for broadcast).
  uint8_t target_system = 0;
  // Component ID (0 for broadcast).
  uint8_t target_component = 0;
  // Only used for drone ID data received from other UAs. See detailed
  // description at https://mavlink.io/en/services/opendroneid.html.
  uint8_t id_or_mac[20] = {0};
  // Indicates the format for the uas_id field of this message.
  uint8_t id_type = 0;
  // Indicates the type of UA (Unmanned Aircraft).
  uint8_t ua_type = 0;
  // UAS (Unmanned Aircraft System) ID following the format specified by
  // id_type. Shall be filled with nulls in the unused portion of the field.
  uint8_t uas_id[20] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Data for filling the OpenDroneID Location message. The float data types are
// 32-bit IEEE 754. The Location message provides the location, altitude,
// direction and speed of the aircraft.
class MavLinkOpenDroneIdLocation : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 12901;
  MavLinkOpenDroneIdLocation() { msgid = kMessageId; }
  // System ID (0 for broadcast).
  uint8_t target_system = 0;
  // Component ID (0 for broadcast).
  uint8_t target_component = 0;
  // Only used for drone ID data received from other UAs. See detailed
  // description at https://mavlink.io/en/services/opendroneid.html.
  uint8_t id_or_mac[20] = {0};
  // Indicates whether the unmanned aircraft is on the ground or in the air.
  uint8_t status = 0;
  // Direction over ground (not heading, but direction of movement) measured
  // clockwise from true North: 0 - 35999 centi-degrees. If unknown: 36100
  // centi-degrees.
  uint16_t direction = 0;
  // Ground speed. Positive only. If unknown: 25500 cm/s. If speed is larger
  // than 25425 cm/s, use 25425 cm/s.
  uint16_t speed_horizontal = 0;
  // The vertical speed. Up is positive. If unknown: 6300 cm/s. If speed is
  // larger than 6200 cm/s, use 6200 cm/s. If lower than -6200 cm/s, use -6200
  // cm/s.
  int16_t speed_vertical = 0;
  // Current latitude of the unmanned aircraft. If unknown: 0 (both Lat/Lon).
  int32_t latitude = 0;
  // Current longitude of the unmanned aircraft. If unknown: 0 (both Lat/Lon).
  int32_t longitude = 0;
  // The altitude calculated from the barometric pressue. Reference is
  // against 29.92inHg or 1013.2mb. If unknown: -1000 m.
  float altitude_barometric = 0;
  // The geodetic altitude as defined by WGS84. If unknown: -1000 m.
  float altitude_geodetic = 0;
  // Indicates the reference point for the height field.
  uint8_t height_reference = 0;
  // The current height of the unmanned aircraft above the take-off location or
  // the ground as indicated by height_reference. If unknown: -1000 m.
  float height = 0;
  // The accuracy of the horizontal position.
  uint8_t horizontal_accuracy = 0;
  // The accuracy of the vertical position.
  uint8_t vertical_accuracy = 0;
  // The accuracy of the barometric altitude.
  uint8_t barometer_accuracy = 0;
  // The accuracy of the horizontal and vertical speed.
  uint8_t speed_accuracy = 0;
  // Seconds after the full hour with reference to UTC time. Typically the GPS
  // outputs a time-of-week value in milliseconds. First convert that to UTC and
  // then convert for this field using ((float) (time_week_ms % (60*60*1000))) /
  // 1000. If unknown: 0xFFFF.
  float timestamp = 0;
  // The accuracy of the timestamps.
  uint8_t timestamp_accuracy = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Data for filling the OpenDroneID Authentication message. The Authentication
// Message defines a field that can provide a means of authenticity for the
// identity of the UAS (Unmanned Aircraft System). The Authentication message
// can have two different formats. For data page 0, the fields PageCount, Length
// and TimeStamp are present and AuthData is only 17 bytes. For data page 1
// through 15, PageCount, Length and TimeStamp are not present and the size of
// AuthData is 23 bytes.
class MavLinkOpenDroneIdAuthentication : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 12902;
  MavLinkOpenDroneIdAuthentication() { msgid = kMessageId; }
  // System ID (0 for broadcast).
  uint8_t target_system = 0;
  // Component ID (0 for broadcast).
  uint8_t target_component = 0;
  // Only used for drone ID data received from other UAs. See detailed
  // description at https://mavlink.io/en/services/opendroneid.html.
  uint8_t id_or_mac[20] = {0};
  // Indicates the type of authentication.
  uint8_t authentication_type = 0;
  // Allowed range is 0 - 15.
  uint8_t data_page = 0;
  // This field is only present for page 0. Allowed range is 0 - 15. See the
  // description of struct ODID_Auth_data at
  // https://github.com/opendroneid/opendroneid-core-c/blob/master/libopendroneid/opendroneid.h.
  uint8_t last_page_index = 0;
  // This field is only present for page 0. Total bytes of authentication_data
  // from all data pages. See the description of struct ODID_Auth_data at
  // https://github.com/opendroneid/opendroneid-core-c/blob/master/libopendroneid/opendroneid.h.
  uint8_t length = 0;
  // This field is only present for page 0. 32 bit Unix Timestamp in seconds
  // since 00:00:00 01/01/2019.
  uint32_t timestamp = 0;
  // Opaque authentication data. For page 0, the size is only 17 bytes. For
  // other pages, the size is 23 bytes. Shall be filled with nulls in the unused
  // portion of the field.
  uint8_t authentication_data[23] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Data for filling the OpenDroneID Self ID message. The Self ID Message is an
// opportunity for the operator to (optionally) declare their identity and
// purpose of the flight. This message can provide additional information that
// could reduce the threat profile of a UA (Unmanned Aircraft) flying in a
// particular area or manner. This message can also be used to provide optional
// additional clarification in an emergency/remote ID system failure situation.
class MavLinkOpenDroneIdSelfId : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 12903;
  MavLinkOpenDroneIdSelfId() { msgid = kMessageId; }
  // System ID (0 for broadcast).
  uint8_t target_system = 0;
  // Component ID (0 for broadcast).
  uint8_t target_component = 0;
  // Only used for drone ID data received from other UAs. See detailed
  // description at https://mavlink.io/en/services/opendroneid.html.
  uint8_t id_or_mac[20] = {0};
  // Indicates the type of the description field.
  uint8_t description_type = 0;
  // Text description or numeric value expressed as ASCII characters. Shall be
  // filled with nulls in the unused portion of the field.
  char description[23] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Data for filling the OpenDroneID System message. The System Message contains
// general system information including the operator location/altitude and
// possible aircraft group and/or category/class information.
class MavLinkOpenDroneIdSystem : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 12904;
  MavLinkOpenDroneIdSystem() { msgid = kMessageId; }
  // System ID (0 for broadcast).
  uint8_t target_system = 0;
  // Component ID (0 for broadcast).
  uint8_t target_component = 0;
  // Only used for drone ID data received from other UAs. See detailed
  // description at https://mavlink.io/en/services/opendroneid.html.
  uint8_t id_or_mac[20] = {0};
  // Specifies the operator location type.
  uint8_t operator_location_type = 0;
  // Specifies the classification type of the UA.
  uint8_t classification_type = 0;
  // Latitude of the operator. If unknown: 0 (both Lat/Lon).
  int32_t operator_latitude = 0;
  // Longitude of the operator. If unknown: 0 (both Lat/Lon).
  int32_t operator_longitude = 0;
  // Number of aircraft in the area, group or formation (default 1).
  uint16_t area_count = 0;
  // Radius of the cylindrical area of the group or formation (default 0).
  uint16_t area_radius = 0;
  // Area Operations Ceiling relative to WGS84. If unknown: -1000 m.
  float area_ceiling = 0;
  // Area Operations Floor relative to WGS84. If unknown: -1000 m.
  float area_floor = 0;
  // When classification_type is MAV_ODID_CLASSIFICATION_TYPE_EU, specifies the
  // category of the UA.
  uint8_t category_eu = 0;
  // When classification_type is MAV_ODID_CLASSIFICATION_TYPE_EU, specifies the
  // class of the UA.
  uint8_t class_eu = 0;
  // Geodetic altitude of the operator relative to WGS84. If unknown: -1000 m.
  float operator_altitude_geo = 0;
  // 32 bit Unix Timestamp in seconds since 00:00:00 01/01/2019.
  uint32_t timestamp = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Data for filling the OpenDroneID Operator ID message, which contains the CAA
// (Civil Aviation Authority) issued operator ID.
class MavLinkOpenDroneIdOperatorId : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 12905;
  MavLinkOpenDroneIdOperatorId() { msgid = kMessageId; }
  // System ID (0 for broadcast).
  uint8_t target_system = 0;
  // Component ID (0 for broadcast).
  uint8_t target_component = 0;
  // Only used for drone ID data received from other UAs. See detailed
  // description at https://mavlink.io/en/services/opendroneid.html.
  uint8_t id_or_mac[20] = {0};
  // Indicates the type of the operator_id field.
  uint8_t operator_id_type = 0;
  // Text description or numeric value expressed as ASCII characters. Shall be
  // filled with nulls in the unused portion of the field.
  char operator_id[20] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// An OpenDroneID message pack is a container for multiple encoded OpenDroneID
// messages (i.e. not in the format given for the above message descriptions but
// after encoding into the compressed OpenDroneID byte format). Used e.g. when
// transmitting on Bluetooth 5.0 Long Range/Extended Advertising or on WiFi
// Neighbor Aware Networking or on WiFi Beacon.
class MavLinkOpenDroneIdMessagePack : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 12915;
  MavLinkOpenDroneIdMessagePack() { msgid = kMessageId; }
  // System ID (0 for broadcast).
  uint8_t target_system = 0;
  // Component ID (0 for broadcast).
  uint8_t target_component = 0;
  // Only used for drone ID data received from other UAs. See detailed
  // description at https://mavlink.io/en/services/opendroneid.html.
  uint8_t id_or_mac[20] = {0};
  // This field must currently always be equal to 25 (bytes), since all encoded
  // OpenDroneID messages are specificed to have this length.
  uint8_t single_message_size = 0;
  // Number of encoded messages in the pack (not the number of bytes). Allowed
  // range is 1 - 9.
  uint8_t msg_pack_size = 0;
  // Concatenation of encoded OpenDroneID messages. Shall be filled with nulls
  // in the unused portion of the field.
  uint8_t messages[225] = {0};
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Temperature and humidity from hygrometer.
class MavLinkHygrometerSensor : public MavLinkMessageBase {
 public:
  const static uint32_t kMessageId = 12920;
  MavLinkHygrometerSensor() { msgid = kMessageId; }
  // Hygrometer ID
  uint8_t id = 0;
  // Temperature
  int16_t temperature = 0;
  // Humidity
  uint16_t humidity = 0;
  virtual std::string toJSon();

 protected:
  virtual int pack(char* buffer) const;
  virtual int unpack(const char* buffer);
};

// Navigate to waypoint.
class MavCmdNavWaypoint : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 16;
  MavCmdNavWaypoint() { command = kCommandId; }
  // Hold time. (ignored by fixed wing, time to stay at waypoint for rotary
  // wing)
  float Hold = 0;
  // Acceptance radius (if the sphere with this radius is hit, the waypoint
  // counts as reached)
  float AcceptRadius = 0;
  // 0 to pass through the WP, if > 0 radius to pass by WP. Positive value for
  // clockwise orbit, negative value for counter-clockwise orbit. Allows
  // trajectory control.
  float PassRadius = 0;
  // Desired yaw angle at waypoint (rotary wing). NaN to use the current system
  // yaw heading mode (e.g. yaw towards next waypoint, yaw to home, etc.).
  float Yaw = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Loiter around this waypoint an unlimited amount of time
class MavCmdNavLoiterUnlim : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 17;
  MavCmdNavLoiterUnlim() { command = kCommandId; }
  // Loiter radius around waypoint for forward-only moving vehicles (not
  // multicopters). If positive loiter clockwise, else counter-clockwise
  float Radius = 0;
  // Desired yaw angle. NaN to use the current system yaw heading mode (e.g. yaw
  // towards next waypoint, yaw to home, etc.).
  float Yaw = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Loiter around this waypoint for X turns
class MavCmdNavLoiterTurns : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 18;
  MavCmdNavLoiterTurns() { command = kCommandId; }
  // Number of turns.
  float Turns = 0;
  // Leave loiter circle only once heading towards the next waypoint (0 = False)
  float HeadingRequired = 0;
  // Loiter radius around waypoint for forward-only moving vehicles (not
  // multicopters). If positive loiter clockwise, else counter-clockwise
  float Radius = 0;
  // Loiter circle exit location and/or path to next waypoint ("xtrack") for
  // forward-only moving vehicles (not multicopters). 0 for the vehicle to
  // converge towards the center xtrack when it leaves the loiter (the line
  // between the centers of the current and next waypoint), 1 to converge to the
  // direct line between the location that the vehicle exits the loiter radius
  // and the next waypoint. Otherwise the angle (in degrees) between the tangent
  // of the loiter circle and the center xtrack at which the vehicle must leave
  // the loiter (and converge to the center xtrack). NaN to use the current
  // system default xtrack behaviour.
  float XtrackLocation = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Loiter at the specified latitude, longitude and altitude for a certain amount
// of time. Multicopter vehicles stop at the point (within a vehicle-specific
// acceptance radius). Forward-only moving vehicles (e.g. fixed-wing) circle the
// point with the specified radius/direction. If the Heading Required parameter
// (2) is non-zero forward moving aircraft will only leave the loiter circle
// once heading towards the next waypoint.
class MavCmdNavLoiterTime : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 19;
  MavCmdNavLoiterTime() { command = kCommandId; }
  // Loiter time (only starts once Lat, Lon and Alt is reached).
  float Time = 0;
  // Leave loiter circle only once heading towards the next waypoint (0 = False)
  float HeadingRequired = 0;
  // Loiter radius around waypoint for forward-only moving vehicles (not
  // multicopters). If positive loiter clockwise, else counter-clockwise.
  float Radius = 0;
  // Loiter circle exit location and/or path to next waypoint ("xtrack") for
  // forward-only moving vehicles (not multicopters). 0 for the vehicle to
  // converge towards the center xtrack when it leaves the loiter (the line
  // between the centers of the current and next waypoint), 1 to converge to the
  // direct line between the location that the vehicle exits the loiter radius
  // and the next waypoint. Otherwise the angle (in degrees) between the tangent
  // of the loiter circle and the center xtrack at which the vehicle must leave
  // the loiter (and converge to the center xtrack). NaN to use the current
  // system default xtrack behaviour.
  float XtrackLocation = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Return to launch location
class MavCmdNavReturnToLaunch : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 20;
  MavCmdNavReturnToLaunch() { command = kCommandId; }

 protected:
  virtual void pack();
  virtual void unpack();
};
// Land at location.
class MavCmdNavLand : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 21;
  MavCmdNavLand() { command = kCommandId; }
  // Minimum target altitude if landing is aborted (0 = undefined/use system
  // default).
  float AbortAlt = 0;
  // Precision land mode.
  float LandMode = 0;
  // Desired yaw angle. NaN to use the current system yaw heading mode (e.g. yaw
  // towards next waypoint, yaw to home, etc.).
  float YawAngle = 0;
  // Latitude.
  float Latitude = 0;
  // Longitude.
  float Longitude = 0;
  // Landing altitude (ground level in current frame).
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Takeoff from ground / hand. Vehicles that support multiple takeoff modes
// (e.g. VTOL quadplane) should take off using the currently configured mode.
class MavCmdNavTakeoff : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 22;
  MavCmdNavTakeoff() { command = kCommandId; }
  // Minimum pitch (if airspeed sensor present), desired pitch without sensor
  float Pitch = 0;
  // Yaw angle (if magnetometer present), ignored without magnetometer. NaN to
  // use the current system yaw heading mode (e.g. yaw towards next waypoint,
  // yaw to home, etc.).
  float Yaw = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Land at local position (local frame only)
class MavCmdNavLandLocal : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 23;
  MavCmdNavLandLocal() { command = kCommandId; }
  // Landing target number (if available)
  float Target = 0;
  // Maximum accepted offset from desired landing position - computed magnitude
  // from spherical coordinates: d = sqrt(x^2 + y^2 + z^2), which gives the
  // maximum accepted distance between the desired landing position and the
  // position where the vehicle is about to land
  float Offset = 0;
  // Landing descend rate
  float DescendRate = 0;
  // Desired yaw angle
  float Yaw = 0;
  // Y-axis position
  float YPosition = 0;
  // X-axis position
  float XPosition = 0;
  // Z-axis / ground level position
  float ZPosition = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Takeoff from local position (local frame only)
class MavCmdNavTakeoffLocal : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 24;
  MavCmdNavTakeoffLocal() { command = kCommandId; }
  // Minimum pitch (if airspeed sensor present), desired pitch without sensor
  float Pitch = 0;
  // Takeoff ascend rate
  float AscendRate = 0;
  // Yaw angle (if magnetometer or another yaw estimation source present),
  // ignored without one of these
  float Yaw = 0;
  // Y-axis position
  float YPosition = 0;
  // X-axis position
  float XPosition = 0;
  // Z-axis position
  float ZPosition = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Vehicle following, i.e. this waypoint represents the position of a moving
// vehicle
class MavCmdNavFollow : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 25;
  MavCmdNavFollow() { command = kCommandId; }
  // Following logic to use (e.g. loitering or sinusoidal following) - depends
  // on specific autopilot implementation
  float Following = 0;
  // Ground speed of vehicle to be followed
  float GroundSpeed = 0;
  // Radius around waypoint. If positive loiter clockwise, else
  // counter-clockwise
  float Radius = 0;
  // Desired yaw angle.
  float Yaw = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Continue on the current course and climb/descend to specified altitude. When
// the altitude is reached continue to the next command (i.e., don't proceed to
// the next command until the desired altitude is reached.
class MavCmdNavContinueAndChangeAlt : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 30;
  MavCmdNavContinueAndChangeAlt() { command = kCommandId; }
  // Climb or Descend (0 = Neutral, command completes when within 5m of this
  // command's altitude, 1 = Climbing, command completes when at or above this
  // command's altitude, 2 = Descending, command completes when at or below this
  // command's altitude.
  float Action = 0;
  // Desired altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Begin loiter at the specified Latitude and Longitude. If Lat=Lon=0, then
// loiter at the current position. Don't consider the navigation command
// complete (don't leave loiter) until the altitude has been reached.
// Additionally, if the Heading Required parameter is non-zero the aircraft will
// not leave the loiter until heading toward the next waypoint.
class MavCmdNavLoiterToAlt : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31;
  MavCmdNavLoiterToAlt() { command = kCommandId; }
  // Leave loiter circle only once heading towards the next waypoint (0 = False)
  float HeadingRequired = 0;
  // Loiter radius around waypoint for forward-only moving vehicles (not
  // multicopters). If positive loiter clockwise, negative counter-clockwise, 0
  // means no change to standard loiter.
  float Radius = 0;
  // Loiter circle exit location and/or path to next waypoint ("xtrack") for
  // forward-only moving vehicles (not multicopters). 0 for the vehicle to
  // converge towards the center xtrack when it leaves the loiter (the line
  // between the centers of the current and next waypoint), 1 to converge to the
  // direct line between the location that the vehicle exits the loiter radius
  // and the next waypoint. Otherwise the angle (in degrees) between the tangent
  // of the loiter circle and the center xtrack at which the vehicle must leave
  // the loiter (and converge to the center xtrack). NaN to use the current
  // system default xtrack behaviour.
  float XtrackLocation = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Begin following a target
class MavCmdDoFollow : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 32;
  MavCmdDoFollow() { command = kCommandId; }
  // System ID (of the FOLLOW_TARGET beacon). Send 0 to disable follow-me and
  // return to the default position hold mode.
  float SystemId = 0;
  // Altitude mode: 0: Keep current altitude, 1: keep altitude difference to
  // target, 2: go to a fixed altitude above home.
  float AltitudeMode = 0;
  // Altitude above home. (used if mode=2)
  float Altitude = 0;
  // Time to land in which the MAV should go to the default position hold mode
  // after a message RX timeout.
  float TimeToLand = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Reposition the MAV after a follow target command has been sent
class MavCmdDoFollowReposition : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 33;
  MavCmdDoFollowReposition() { command = kCommandId; }
  // Camera q1 (where 0 is on the ray from the camera to the tracking device)
  float CameraQp1 = 0;
  // Camera q2
  float CameraQp2 = 0;
  // Camera q3
  float CameraQp3 = 0;
  // Camera q4
  float CameraQp4 = 0;
  // altitude offset from target
  float AltitudeOffset = 0;
  // X offset from target
  float XOffset = 0;
  // Y offset from target
  float YOffset = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Start orbiting on the circumference of a circle defined by the parameters.
// Setting values to NaN/INT32_MAX (as appropriate) results in using defaults.
class MavCmdDoOrbit : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 34;
  MavCmdDoOrbit() { command = kCommandId; }
  // Radius of the circle. Positive: orbit clockwise. Negative: orbit
  // counter-clockwise. NaN: Use vehicle default radius, or current radius if
  // already orbiting.
  float Radius = 0;
  // Tangential Velocity. NaN: Use vehicle default velocity, or current velocity
  // if already orbiting.
  float Velocity = 0;
  // Yaw behavior of the vehicle.
  float YawBehavior = 0;
  // Orbit around the centre point for this many radians (i.e. for a
  // three-quarter orbit set 270*Pi/180). 0: Orbit forever. NaN: Use vehicle
  // default, or current value if already orbiting.
  float Orbits = 0;
  // Center point latitude (if no MAV_FRAME specified) / X coordinate according
  // to MAV_FRAME. INT32_MAX (or NaN if sent in COMMAND_LONG): Use current
  // vehicle position, or current center if already orbiting.
  float Latitudepx = 0;
  // Center point longitude (if no MAV_FRAME specified) / Y coordinate according
  // to MAV_FRAME. INT32_MAX (or NaN if sent in COMMAND_LONG): Use current
  // vehicle position, or current center if already orbiting.
  float Longitudepy = 0;
  // Center point altitude (MSL) (if no MAV_FRAME specified) / Z coordinate
  // according to MAV_FRAME. NaN: Use current vehicle altitude.
  float Altitudepz = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Sets the region of interest (ROI) for a sensor set or the vehicle itself.
// This can then be used by the vehicle's control system to control the vehicle
// attitude and the attitude of various sensors such as cameras.
class MavCmdNavRoi : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 80;
  MavCmdNavRoi() { command = kCommandId; }
  // Region of interest mode.
  float RoiMode = 0;
  // Waypoint index/ target ID. (see MAV_ROI enum)
  float WpIndex = 0;
  // ROI index (allows a vehicle to manage multiple ROI's)
  float RoiIndex = 0;
  // x the location of the fixed ROI (see MAV_FRAME)
  float X = 0;
  // y
  float Y = 0;
  // z
  float Z = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Control autonomous path planning on the MAV.
class MavCmdNavPathplanning : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 81;
  MavCmdNavPathplanning() { command = kCommandId; }
  // 0: Disable local obstacle avoidance / local path planning (without
  // resetting map), 1: Enable local path planning, 2: Enable and reset local
  // path planning
  float LocalCtrl = 0;
  // 0: Disable full path planning (without resetting map), 1: Enable, 2: Enable
  // and reset map/occupancy grid, 3: Enable and reset planned route, but not
  // occupancy grid
  float GlobalCtrl = 0;
  // Yaw angle at goal
  float Yaw = 0;
  // Latitude/X of goal
  float Latitudepx = 0;
  // Longitude/Y of goal
  float Longitudepy = 0;
  // Altitude/Z of goal
  float Altitudepz = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Navigate to waypoint using a spline path.
class MavCmdNavSplineWaypoint : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 82;
  MavCmdNavSplineWaypoint() { command = kCommandId; }
  // Hold time. (ignored by fixed wing, time to stay at waypoint for rotary
  // wing)
  float Hold = 0;
  // Latitude/X of goal
  float Latitudepx = 0;
  // Longitude/Y of goal
  float Longitudepy = 0;
  // Altitude/Z of goal
  float Altitudepz = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Takeoff from ground using VTOL mode, and transition to forward flight with
// specified heading. The command should be ignored by vehicles that dont
// support both VTOL and fixed-wing flight (multicopters, boats,etc.).
class MavCmdNavVtolTakeoff : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 84;
  MavCmdNavVtolTakeoff() { command = kCommandId; }
  // Front transition heading.
  float TransitionHeading = 0;
  // Yaw angle. NaN to use the current system yaw heading mode (e.g. yaw towards
  // next waypoint, yaw to home, etc.).
  float YawAngle = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Land using VTOL mode
class MavCmdNavVtolLand : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 85;
  MavCmdNavVtolLand() { command = kCommandId; }
  // Landing behaviour.
  float LandOptions = 0;
  // Approach altitude (with the same reference as the Altitude field). NaN if
  // unspecified.
  float ApproachAltitude = 0;
  // Yaw angle. NaN to use the current system yaw heading mode (e.g. yaw towards
  // next waypoint, yaw to home, etc.).
  float Yaw = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude (ground level) relative to the current coordinate frame. NaN to
  // use system default landing altitude (ignore value).
  float GroundAltitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// hand control over to an external controller
class MavCmdNavGuidedEnable : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 92;
  MavCmdNavGuidedEnable() { command = kCommandId; }
  // On / Off (> 0.5f on)
  float Enable = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Delay the next navigation command a number of seconds or until a specified
// time
class MavCmdNavDelay : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 93;
  MavCmdNavDelay() { command = kCommandId; }
  // Delay (-1 to enable time-of-day fields)
  float Delay = 0;
  // hour (24h format, UTC, -1 to ignore)
  float Hour = 0;
  // minute (24h format, UTC, -1 to ignore)
  float Minute = 0;
  // second (24h format, UTC, -1 to ignore)
  float Second = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Descend and place payload. Vehicle moves to specified location, descends
// until it detects a hanging payload has reached the ground, and then releases
// the payload. If ground is not detected before the reaching the maximum
// descent value (param1), the command will complete without releasing the
// payload.
class MavCmdNavPayloadPlace : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 94;
  MavCmdNavPayloadPlace() { command = kCommandId; }
  // Maximum distance to descend.
  float MaxDescent = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// NOP - This command is only used to mark the upper limit of the NAV/ACTION
// commands in the enumeration
class MavCmdNavLast : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 95;
  MavCmdNavLast() { command = kCommandId; }

 protected:
  virtual void pack();
  virtual void unpack();
};
// Delay mission state machine.
class MavCmdConditionDelay : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 112;
  MavCmdConditionDelay() { command = kCommandId; }
  // Delay
  float Delay = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Ascend/descend to target altitude at specified rate. Delay mission state
// machine until desired altitude reached.
class MavCmdConditionChangeAlt : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 113;
  MavCmdConditionChangeAlt() { command = kCommandId; }
  // Descent / Ascend rate.
  float Rate = 0;
  // Target Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Delay mission state machine until within desired distance of next NAV point.
class MavCmdConditionDistance : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 114;
  MavCmdConditionDistance() { command = kCommandId; }
  // Distance.
  float Distance = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Reach a certain target angle.
class MavCmdConditionYaw : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 115;
  MavCmdConditionYaw() { command = kCommandId; }
  // target angle, 0 is north
  float Angle = 0;
  // angular speed
  float AngularSpeed = 0;
  // direction: -1: counter clockwise, 1: clockwise
  float Direction = 0;
  // 0: absolute angle, 1: relative offset
  float Relative = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// NOP - This command is only used to mark the upper limit of the CONDITION
// commands in the enumeration
class MavCmdConditionLast : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 159;
  MavCmdConditionLast() { command = kCommandId; }

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set system mode.
class MavCmdDoSetMode : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 176;
  MavCmdDoSetMode() { command = kCommandId; }
  // Mode
  float Mode = 0;
  // Custom mode - this is system specific, please refer to the individual
  // autopilot specifications for details.
  float CustomMode = 0;
  // Custom sub mode - this is system specific, please refer to the individual
  // autopilot specifications for details.
  float CustomSubmode = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Jump to the desired command in the mission list. Repeat this action only the
// specified number of times
class MavCmdDoJump : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 177;
  MavCmdDoJump() { command = kCommandId; }
  // Sequence number
  float Number = 0;
  // Repeat count
  float Repeat = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Change speed and/or throttle set points.
class MavCmdDoChangeSpeed : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 178;
  MavCmdDoChangeSpeed() { command = kCommandId; }
  // Speed type (0=Airspeed, 1=Ground Speed, 2=Climb Speed, 3=Descent Speed)
  float SpeedType = 0;
  // Speed (-1 indicates no change)
  float Speed = 0;
  // Throttle (-1 indicates no change)
  float Throttle = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Sets the home position to either to the current position or a specified
// position. The home position is the default position that the system will
// return to and land on. The position is set automatically by the system during
// the takeoff (and may also be set using this command). Note: the current home
// position may be emitted in a HOME_POSITION message on request (using
// MAV_CMD_REQUEST_MESSAGE with param1=242).
class MavCmdDoSetHome : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 179;
  MavCmdDoSetHome() { command = kCommandId; }
  // Use current (1=use current location, 0=use specified location)
  float UseCurrent = 0;
  // Yaw angle. NaN to use default heading
  float Yaw = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set a system parameter. Caution! Use of this command requires knowledge of
// the numeric enumeration value of the parameter.
class MavCmdDoSetParameter : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 180;
  MavCmdDoSetParameter() { command = kCommandId; }
  // Parameter number
  float Number = 0;
  // Parameter value
  float Value = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set a relay to a condition.
class MavCmdDoSetRelay : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 181;
  MavCmdDoSetRelay() { command = kCommandId; }
  // Relay instance number.
  float Instance = 0;
  // Setting. (1=on, 0=off, others possible depending on system hardware)
  float Setting = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Cycle a relay on and off for a desired number of cycles with a desired
// period.
class MavCmdDoRepeatRelay : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 182;
  MavCmdDoRepeatRelay() { command = kCommandId; }
  // Relay instance number.
  float Instance = 0;
  // Cycle count.
  float Count = 0;
  // Cycle time.
  float Time = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set a servo to a desired PWM value.
class MavCmdDoSetServo : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 183;
  MavCmdDoSetServo() { command = kCommandId; }
  // Servo instance number.
  float Instance = 0;
  // Pulse Width Modulation.
  float Pwm = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Cycle a between its nominal setting and a desired PWM for a desired number of
// cycles with a desired period.
class MavCmdDoRepeatServo : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 184;
  MavCmdDoRepeatServo() { command = kCommandId; }
  // Servo instance number.
  float Instance = 0;
  // Pulse Width Modulation.
  float Pwm = 0;
  // Cycle count.
  float Count = 0;
  // Cycle time.
  float Time = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Terminate flight immediately. Flight termination immediately and irreversably
// terminates the current flight, returning the vehicle to ground. The vehicle
// will ignore RC or other input until it has been power-cycled. Termination may
// trigger safety measures, including: disabling motors and deployment of
// parachute on multicopters, and setting flight surfaces to initiate a landing
// pattern on fixed-wing). On multicopters without a parachute it may trigger a
// crash landing. Support for this command can be tested using the protocol bit:
// MAV_PROTOCOL_CAPABILITY_FLIGHT_TERMINATION. Support for this command can also
// be tested by sending the command with param1=0 (< 0.5); the ACK should be
// either MAV_RESULT_FAILED or MAV_RESULT_UNSUPPORTED.
class MavCmdDoFlighttermination : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 185;
  MavCmdDoFlighttermination() { command = kCommandId; }
  // Flight termination activated if > 0.5. Otherwise not activated and ACK with
  // MAV_RESULT_FAILED.
  float Terminate = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Change altitude set point.
class MavCmdDoChangeAltitude : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 186;
  MavCmdDoChangeAltitude() { command = kCommandId; }
  // Altitude.
  float Altitude = 0;
  // Frame of new altitude.
  float Frame = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Sets actuators (e.g. servos) to a desired value. The actuator numbers are
// mapped to specific outputs (e.g. on any MAIN or AUX PWM or UAVCAN) using a
// flight-stack specific mechanism (i.e. a parameter).
class MavCmdDoSetActuator : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 187;
  MavCmdDoSetActuator() { command = kCommandId; }
  // Actuator 1 value, scaled from [-1 to 1]. NaN to ignore.
  float ActuatorP1 = 0;
  // Actuator 2 value, scaled from [-1 to 1]. NaN to ignore.
  float ActuatorP2 = 0;
  // Actuator 3 value, scaled from [-1 to 1]. NaN to ignore.
  float ActuatorP3 = 0;
  // Actuator 4 value, scaled from [-1 to 1]. NaN to ignore.
  float ActuatorP4 = 0;
  // Actuator 5 value, scaled from [-1 to 1]. NaN to ignore.
  float ActuatorP5 = 0;
  // Actuator 6 value, scaled from [-1 to 1]. NaN to ignore.
  float ActuatorP6 = 0;
  // Index of actuator set (i.e if set to 1, Actuator 1 becomes Actuator 7)
  float Index = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mission command to perform a landing. This is used as a marker in a mission
// to tell the autopilot where a sequence of mission items that represents a
// landing starts. It may also be sent via a COMMAND_LONG to trigger a landing,
// in which case the nearest (geographically) landing sequence in the mission
// will be used. The Latitude/Longitude is optional, and may be set to 0 if not
// needed. If specified then it will be used to help find the closest landing
// sequence.
class MavCmdDoLandStart : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 189;
  MavCmdDoLandStart() { command = kCommandId; }
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mission command to perform a landing from a rally point.
class MavCmdDoRallyLand : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 190;
  MavCmdDoRallyLand() { command = kCommandId; }
  // Break altitude
  float Altitude = 0;
  // Landing speed
  float Speed = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mission command to safely abort an autonomous landing.
class MavCmdDoGoAround : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 191;
  MavCmdDoGoAround() { command = kCommandId; }
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Reposition the vehicle to a specific WGS84 global position.
class MavCmdDoReposition : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 192;
  MavCmdDoReposition() { command = kCommandId; }
  // Ground speed, less than 0 (-1) for default
  float Speed = 0;
  // Bitmask of option flags.
  float Bitmask = 0;
  // Loiter radius for planes. Positive values only, direction is controlled by
  // Yaw value. A value of zero or NaN is ignored.
  float Radius = 0;
  // Yaw heading. NaN to use the current system yaw heading mode (e.g. yaw
  // towards next waypoint, yaw to home, etc.). For planes indicates loiter
  // direction (0: clockwise, 1: counter clockwise)
  float Yaw = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// If in a GPS controlled position mode, hold the current position or continue.
class MavCmdDoPauseContinue : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 193;
  MavCmdDoPauseContinue() { command = kCommandId; }
  // 0: Pause current mission or reposition command, hold current position. 1:
  // Continue mission. A VTOL capable vehicle should enter hover mode
  // (multicopter and VTOL planes). A plane should loiter with the default
  // loiter radius.
  float Continue = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set moving direction to forward or reverse.
class MavCmdDoSetReverse : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 194;
  MavCmdDoSetReverse() { command = kCommandId; }
  // Direction (0=Forward, 1=Reverse)
  float Reverse = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Sets the region of interest (ROI) to a location. This can then be used by the
// vehicle's control system to control the vehicle attitude and the attitude of
// various sensors such as cameras. This command can be sent to a gimbal manager
// but not to a gimbal device. A gimbal is not to react to this message.
class MavCmdDoSetRoiLocation : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 195;
  MavCmdDoSetRoiLocation() { command = kCommandId; }
  // Component ID of gimbal device to address (or 1-6 for non-MAVLink gimbal), 0
  // for all gimbal device components. Send command multiple times for more than
  // one gimbal (but not all gimbals).
  float GimbalDeviceId = 0;
  // Latitude of ROI location
  float Latitude = 0;
  // Longitude of ROI location
  float Longitude = 0;
  // Altitude of ROI location
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Sets the region of interest (ROI) to be toward next waypoint, with optional
// pitch/roll/yaw offset. This can then be used by the vehicle's control system
// to control the vehicle attitude and the attitude of various sensors such as
// cameras. This command can be sent to a gimbal manager but not to a gimbal
// device. A gimbal device is not to react to this message.
class MavCmdDoSetRoiWpnextOffset : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 196;
  MavCmdDoSetRoiWpnextOffset() { command = kCommandId; }
  // Component ID of gimbal device to address (or 1-6 for non-MAVLink gimbal), 0
  // for all gimbal device components. Send command multiple times for more than
  // one gimbal (but not all gimbals).
  float GimbalDeviceId = 0;
  // Pitch offset from next waypoint, positive pitching up
  float PitchOffset = 0;
  // Roll offset from next waypoint, positive rolling to the right
  float RollOffset = 0;
  // Yaw offset from next waypoint, positive yawing to the right
  float YawOffset = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Cancels any previous ROI command returning the vehicle/sensors to default
// flight characteristics. This can then be used by the vehicle's control system
// to control the vehicle attitude and the attitude of various sensors such as
// cameras. This command can be sent to a gimbal manager but not to a gimbal
// device. A gimbal device is not to react to this message. After this command
// the gimbal manager should go back to manual input if available, and otherwise
// assume a neutral position.
class MavCmdDoSetRoiNone : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 197;
  MavCmdDoSetRoiNone() { command = kCommandId; }
  // Component ID of gimbal device to address (or 1-6 for non-MAVLink gimbal), 0
  // for all gimbal device components. Send command multiple times for more than
  // one gimbal (but not all gimbals).
  float GimbalDeviceId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mount tracks system with specified system ID. Determination of target vehicle
// position may be done with GLOBAL_POSITION_INT or any other means. This
// command can be sent to a gimbal manager but not to a gimbal device. A gimbal
// device is not to react to this message.
class MavCmdDoSetRoiSysid : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 198;
  MavCmdDoSetRoiSysid() { command = kCommandId; }
  // System ID
  float SystemId = 0;
  // Component ID of gimbal device to address (or 1-6 for non-MAVLink gimbal), 0
  // for all gimbal device components. Send command multiple times for more than
  // one gimbal (but not all gimbals).
  float GimbalDeviceId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Control onboard camera system.
class MavCmdDoControlVideo : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 200;
  MavCmdDoControlVideo() { command = kCommandId; }
  // Camera ID (-1 for all)
  float Id = 0;
  // Transmission: 0: disabled, 1: enabled compressed, 2: enabled raw
  float Transmission = 0;
  // Transmission mode: 0: video stream, >0: single images every n seconds
  float Interval = 0;
  // Recording: 0: disabled, 1: enabled compressed, 2: enabled raw
  float Recording = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Sets the region of interest (ROI) for a sensor set or the vehicle itself.
// This can then be used by the vehicle's control system to control the vehicle
// attitude and the attitude of various sensors such as cameras.
class MavCmdDoSetRoi : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 201;
  MavCmdDoSetRoi() { command = kCommandId; }
  // Region of interest mode.
  float RoiMode = 0;
  // Waypoint index/ target ID (depends on param 1).
  float WpIndex = 0;
  // Region of interest index. (allows a vehicle to manage multiple ROI's)
  float RoiIndex = 0;
  // MAV_ROI_WPNEXT: pitch offset from next waypoint, MAV_ROI_LOCATION: latitude
  float MavRoiWpnext = 0;
  // MAV_ROI_WPNEXT: roll offset from next waypoint, MAV_ROI_LOCATION: longitude
  float MavRoiWpnext2 = 0;
  // MAV_ROI_WPNEXT: yaw offset from next waypoint, MAV_ROI_LOCATION: altitude
  float MavRoiWpnext3 = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Configure digital camera. This is a fallback message for systems that have
// not yet implemented PARAM_EXT_XXX messages and camera definition files (see
// https://mavlink.io/en/services/camera_def.html
// ).
class MavCmdDoDigicamConfigure : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 202;
  MavCmdDoDigicamConfigure() { command = kCommandId; }
  // Modes: P, TV, AV, M, Etc.
  float Mode = 0;
  // Shutter speed: Divisor number for one second.
  float ShutterSpeed = 0;
  // Aperture: F stop number.
  float Aperture = 0;
  // ISO number e.g. 80, 100, 200, Etc.
  float Iso = 0;
  // Exposure type enumerator.
  float Exposure = 0;
  // Command Identity.
  float CommandIdentity = 0;
  // Main engine cut-off time before camera trigger. (0 means no cut-off)
  float EngineCutpoff = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Control digital camera. This is a fallback message for systems that have not
// yet implemented PARAM_EXT_XXX messages and camera definition files (see
// https://mavlink.io/en/services/camera_def.html
// ).
class MavCmdDoDigicamControl : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 203;
  MavCmdDoDigicamControl() { command = kCommandId; }
  // Session control e.g. show/hide lens
  float SessionControl = 0;
  // Zoom's absolute position
  float ZoomAbsolute = 0;
  // Zooming step value to offset zoom from the current position
  float ZoomRelative = 0;
  // Focus Locking, Unlocking or Re-locking
  float Focus = 0;
  // Shooting Command
  float ShootCommand = 0;
  // Command Identity
  float CommandIdentity = 0;
  // Test shot identifier. If set to 1, image will only be captured, but not
  // counted towards internal frame count.
  float ShotId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mission command to configure a camera or antenna mount
class MavCmdDoMountConfigure : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 204;
  MavCmdDoMountConfigure() { command = kCommandId; }
  // Mount operation mode
  float Mode = 0;
  // stabilize roll? (1 = yes, 0 = no)
  float StabilizeRoll = 0;
  // stabilize pitch? (1 = yes, 0 = no)
  float StabilizePitch = 0;
  // stabilize yaw? (1 = yes, 0 = no)
  float StabilizeYaw = 0;
  // roll input (0 = angle body frame, 1 = angular rate, 2 = angle absolute
  // frame)
  float RollInputMode = 0;
  // pitch input (0 = angle body frame, 1 = angular rate, 2 = angle absolute
  // frame)
  float PitchInputMode = 0;
  // yaw input (0 = angle body frame, 1 = angular rate, 2 = angle absolute
  // frame)
  float YawInputMode = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mission command to control a camera or antenna mount
class MavCmdDoMountControl : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 205;
  MavCmdDoMountControl() { command = kCommandId; }
  // pitch depending on mount mode (degrees or degrees/second depending on pitch
  // input).
  float Pitch = 0;
  // roll depending on mount mode (degrees or degrees/second depending on roll
  // input).
  float Roll = 0;
  // yaw depending on mount mode (degrees or degrees/second depending on yaw
  // input).
  float Yaw = 0;
  // altitude depending on mount mode.
  float Altitude = 0;
  // latitude, set if appropriate mount mode.
  float Latitude = 0;
  // longitude, set if appropriate mount mode.
  float Longitude = 0;
  // Mount mode.
  float Mode = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mission command to set camera trigger distance for this flight. The camera is
// triggered each time this distance is exceeded. This command can also be used
// to set the shutter integration time for the camera.
class MavCmdDoSetCamTriggDist : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 206;
  MavCmdDoSetCamTriggDist() { command = kCommandId; }
  // Camera trigger distance. 0 to stop triggering.
  float Distance = 0;
  // Camera shutter integration time. -1 or 0 to ignore
  float Shutter = 0;
  // Trigger camera once immediately. (0 = no trigger, 1 = trigger)
  float Trigger = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mission command to enable the geofence
class MavCmdDoFenceEnable : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 207;
  MavCmdDoFenceEnable() { command = kCommandId; }
  // enable? (0=disable, 1=enable, 2=disable_floor_only)
  float Enable = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mission item/command to release a parachute or enable/disable auto release.
class MavCmdDoParachute : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 208;
  MavCmdDoParachute() { command = kCommandId; }
  // Action
  float Action = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Command to perform motor test.
class MavCmdDoMotorTest : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 209;
  MavCmdDoMotorTest() { command = kCommandId; }
  // Motor instance number (from 1 to max number of motors on the vehicle).
  float Instance = 0;
  // Throttle type (whether the Throttle Value in param3 is a percentage, PWM
  // value, etc.)
  float ThrottleType = 0;
  // Throttle value.
  float Throttle = 0;
  // Timeout between tests that are run in sequence.
  float Timeout = 0;
  // Motor count. Number of motors to test in sequence: 0/1=one motor, 2= two
  // motors, etc. The Timeout (param4) is used between tests.
  float MotorCount = 0;
  // Motor test order.
  float TestOrder = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Change to/from inverted flight.
class MavCmdDoInvertedFlight : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 210;
  MavCmdDoInvertedFlight() { command = kCommandId; }
  // Inverted flight. (0=normal, 1=inverted)
  float Inverted = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mission command to operate a gripper.
class MavCmdDoGripper : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 211;
  MavCmdDoGripper() { command = kCommandId; }
  // Gripper instance number.
  float Instance = 0;
  // Gripper action to perform.
  float Action = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Enable/disable autotune.
class MavCmdDoAutotuneEnable : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 212;
  MavCmdDoAutotuneEnable() { command = kCommandId; }
  // Enable (1: enable, 0:disable).
  float Enable = 0;
  // Specify which axis are autotuned. 0 indicates autopilot default settings.
  float Axis = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Sets a desired vehicle turn angle and speed change.
class MavCmdNavSetYawSpeed : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 213;
  MavCmdNavSetYawSpeed() { command = kCommandId; }
  // Yaw angle to adjust steering by.
  float Yaw = 0;
  // Speed.
  float Speed = 0;
  // Final angle. (0=absolute, 1=relative)
  float Angle = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mission command to set camera trigger interval for this flight. If triggering
// is enabled, the camera is triggered each time this interval expires. This
// command can also be used to set the shutter integration time for the camera.
class MavCmdDoSetCamTriggInterval : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 214;
  MavCmdDoSetCamTriggInterval() { command = kCommandId; }
  // Camera trigger cycle time. -1 or 0 to ignore.
  float TriggerCycle = 0;
  // Camera shutter integration time. Should be less than trigger cycle time. -1
  // or 0 to ignore.
  float ShutterIntegration = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mission command to control a camera or antenna mount, using a quaternion as
// reference.
class MavCmdDoMountControlQuat : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 220;
  MavCmdDoMountControlQuat() { command = kCommandId; }
  // quaternion param q1, w (1 in null-rotation)
  float Qp1 = 0;
  // quaternion param q2, x (0 in null-rotation)
  float Qp2 = 0;
  // quaternion param q3, y (0 in null-rotation)
  float Qp3 = 0;
  // quaternion param q4, z (0 in null-rotation)
  float Qp4 = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// set id of master controller
class MavCmdDoGuidedMaster : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 221;
  MavCmdDoGuidedMaster() { command = kCommandId; }
  // System ID
  float SystemId = 0;
  // Component ID
  float ComponentId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set limits for external control
class MavCmdDoGuidedLimits : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 222;
  MavCmdDoGuidedLimits() { command = kCommandId; }
  // Timeout - maximum time that external controller will be allowed to control
  // vehicle. 0 means no timeout.
  float Timeout = 0;
  // Altitude (MSL) min - if vehicle moves below this alt, the command will be
  // aborted and the mission will continue. 0 means no lower altitude limit.
  float MinAltitude = 0;
  // Altitude (MSL) max - if vehicle moves above this alt, the command will be
  // aborted and the mission will continue. 0 means no upper altitude limit.
  float MaxAltitude = 0;
  // Horizontal move limit - if vehicle moves more than this distance from its
  // location at the moment the command was executed, the command will be
  // aborted and the mission will continue. 0 means no horizontal move limit.
  float HorizpMoveLimit = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Control vehicle engine. This is interpreted by the vehicles engine controller
// to change the target engine state. It is intended for vehicles with internal
// combustion engines
class MavCmdDoEngineControl : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 223;
  MavCmdDoEngineControl() { command = kCommandId; }
  // 0: Stop engine, 1:Start Engine
  float StartEngine = 0;
  // 0: Warm start, 1:Cold start. Controls use of choke where applicable
  float ColdStart = 0;
  // Height delay. This is for commanding engine start only after the vehicle
  // has gained the specified height. Used in VTOL vehicles during takeoff to
  // start engine after the aircraft is off the ground. Zero for no delay.
  float HeightDelay = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set the mission item with sequence number seq as current item. This means
// that the MAV will continue to this mission item on the shortest path (not
// following the mission items in-between).
class MavCmdDoSetMissionCurrent : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 224;
  MavCmdDoSetMissionCurrent() { command = kCommandId; }
  // Mission sequence value to set
  float Number = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// NOP - This command is only used to mark the upper limit of the DO commands in
// the enumeration
class MavCmdDoLast : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 240;
  MavCmdDoLast() { command = kCommandId; }

 protected:
  virtual void pack();
  virtual void unpack();
};
// Trigger calibration. This command will be only accepted if in pre-flight
// mode. Except for Temperature Calibration, only one sensor should be set in a
// single message and all others should be zero.
class MavCmdPreflightCalibration : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 241;
  MavCmdPreflightCalibration() { command = kCommandId; }
  // 1: gyro calibration, 3: gyro temperature calibration
  float GyroTemperature = 0;
  // 1: magnetometer calibration
  float Magnetometer = 0;
  // 1: ground pressure calibration
  float GroundPressure = 0;
  // 1: radio RC calibration, 2: RC trim calibration
  float RemoteControl = 0;
  // 1: accelerometer calibration, 2: board level calibration, 3: accelerometer
  // temperature calibration, 4: simple accelerometer calibration
  float Accelerometer = 0;
  // 1: APM: compass/motor interference calibration (PX4: airspeed calibration,
  // deprecated), 2: airspeed calibration
  float CompmotOrAirspeed = 0;
  // 1: ESC calibration, 3: barometer temperature calibration
  float EscOrBaro = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set sensor offsets. This command will be only accepted if in pre-flight mode.
class MavCmdPreflightSetSensorOffsets : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 242;
  MavCmdPreflightSetSensorOffsets() { command = kCommandId; }
  // Sensor to adjust the offsets for: 0: gyros, 1: accelerometer, 2:
  // magnetometer, 3: barometer, 4: optical flow, 5: second magnetometer, 6:
  // third magnetometer
  float SensorType = 0;
  // X axis offset (or generic dimension 1), in the sensor's raw units
  float XOffset = 0;
  // Y axis offset (or generic dimension 2), in the sensor's raw units
  float YOffset = 0;
  // Z axis offset (or generic dimension 3), in the sensor's raw units
  float ZOffset = 0;
  // Generic dimension 4, in the sensor's raw units
  float P4thDimension = 0;
  // Generic dimension 5, in the sensor's raw units
  float P5thDimension = 0;
  // Generic dimension 6, in the sensor's raw units
  float P6thDimension = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Trigger UAVCAN configuration (actuator ID assignment and direction mapping).
// Note that this maps to the legacy UAVCAN v0 function UAVCAN_ENUMERATE, which
// is intended to be executed just once during initial vehicle configuration (it
// is not a normal pre-flight command and has been poorly named).
class MavCmdPreflightUavcan : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 243;
  MavCmdPreflightUavcan() { command = kCommandId; }
  // 1: Trigger actuator ID assignment and direction mapping. 0: Cancel command.
  float ActuatorId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request storage of different parameter values and logs. This command will be
// only accepted if in pre-flight mode.
class MavCmdPreflightStorage : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 245;
  MavCmdPreflightStorage() { command = kCommandId; }
  // Action to perform on the persistent parameter storage
  float ParameterStorage = 0;
  // Action to perform on the persistent mission storage
  float MissionStorage = 0;
  // Onboard logging: 0: Ignore, 1: Start default rate logging, -1: Stop
  // logging, > 1: logging rate (e.g. set to 1000 for 1000 Hz logging)
  float LoggingRate = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request the reboot or shutdown of system components.
class MavCmdPreflightRebootShutdown : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 246;
  MavCmdPreflightRebootShutdown() { command = kCommandId; }
  // 0: Do nothing for autopilot, 1: Reboot autopilot, 2: Shutdown autopilot, 3:
  // Reboot autopilot and keep it in the bootloader until upgraded.
  float Autopilot = 0;
  // 0: Do nothing for onboard computer, 1: Reboot onboard computer, 2: Shutdown
  // onboard computer, 3: Reboot onboard computer and keep it in the bootloader
  // until upgraded.
  float Companion = 0;
  // 0: Do nothing for component, 1: Reboot component, 2: Shutdown component, 3:
  // Reboot component and keep it in the bootloader until upgraded
  float ComponentAction = 0;
  // MAVLink Component ID targeted in param3 (0 for all components).
  float ComponentId = 0;
  // WIP: ID (e.g. camera ID -1 for all IDs)
  float Wip = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Override current mission with command to pause mission, pause mission and
// move to position, continue/resume mission. When param 1 indicates that the
// mission is paused (MAV_GOTO_DO_HOLD), param 2 defines whether it holds in
// place or moves to another position.
class MavCmdOverrideGoto : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 252;
  MavCmdOverrideGoto() { command = kCommandId; }
  // MAV_GOTO_DO_HOLD: pause mission and either hold or move to specified
  // position (depending on param2), MAV_GOTO_DO_CONTINUE: resume mission.
  float Continue = 0;
  // MAV_GOTO_HOLD_AT_CURRENT_POSITION: hold at current position,
  // MAV_GOTO_HOLD_AT_SPECIFIED_POSITION: hold at specified position.
  float Position = 0;
  // Coordinate frame of hold point.
  float Frame = 0;
  // Desired yaw angle.
  float Yaw = 0;
  // Latitude/X position.
  float Latitudepx = 0;
  // Longitude/Y position.
  float Longitudepy = 0;
  // Altitude/Z position.
  float Altitudepz = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Mission command to set a Camera Auto Mount Pivoting Oblique Survey (Replaces
// CAM_TRIGG_DIST for this purpose). The camera is triggered each time this
// distance is exceeded, then the mount moves to the next position. Params 4~6
// set-up the angle limits and number of positions for oblique survey, where
// mount-enabled vehicles automatically roll the camera between shots to emulate
// an oblique camera setup (providing an increased HFOV). This command can also
// be used to set the shutter integration time for the camera.
class MavCmdObliqueSurvey : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 260;
  MavCmdObliqueSurvey() { command = kCommandId; }
  // Camera trigger distance. 0 to stop triggering.
  float Distance = 0;
  // Camera shutter integration time. 0 to ignore
  float Shutter = 0;
  // The minimum interval in which the camera is capable of taking subsequent
  // pictures repeatedly. 0 to ignore.
  float MinInterval = 0;
  // Total number of roll positions at which the camera will capture photos
  // (images captures spread evenly across the limits defined by param5).
  float Positions = 0;
  // Angle limits that the camera can be rolled to left and right of center.
  float RollAngle = 0;
  // Fixed pitch angle that the camera will hold in oblique mode if the mount is
  // actuated in the pitch axis.
  float PitchAngle = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// start running a mission
class MavCmdMissionStart : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 300;
  MavCmdMissionStart() { command = kCommandId; }
  // first_item: the first mission item to run
  float FirstItem = 0;
  // last_item: the last mission item to run (after this item is run, the
  // mission ends)
  float LastItem = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Actuator testing command. This is similar to MAV_CMD_DO_MOTOR_TEST but
// operates on the level of output functions, i.e. it is possible to test Motor1
// independent from which output it is configured on. Autopilots typically
// refuse this command while armed.
class MavCmdActuatorTest : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 310;
  MavCmdActuatorTest() { command = kCommandId; }
  // Output value: 1 means maximum positive output, 0 to center servos or
  // minimum motor thrust (expected to spin), -1 for maximum negative (if not
  // supported by the motors, i.e. motor is not reversible, smaller than 0 maps
  // to NaN). And NaN maps to disarmed (stop the motors).
  float Value = 0;
  // Timeout after which the test command expires and the output is restored to
  // the previous value. A timeout has to be set for safety reasons. A timeout
  // of 0 means to restore the previous value immediately.
  float Timeout = 0;
  // Actuator Output function
  float OutputFunction = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Actuator configuration command.
class MavCmdConfigureActuator : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 311;
  MavCmdConfigureActuator() { command = kCommandId; }
  // Actuator configuration action
  float Configuration = 0;
  // Actuator Output function
  float OutputFunction = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Arms / Disarms a component
class MavCmdComponentArmDisarm : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 400;
  MavCmdComponentArmDisarm() { command = kCommandId; }
  // 0: disarm, 1: arm
  float Arm = 0;
  // 0: arm-disarm unless prevented by safety checks (i.e. when landed), 21196:
  // force arming/disarming (e.g. allow arming to override preflight checks and
  // disarming in flight)
  float Force = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Instructs a target system to run pre-arm checks. This allows preflight checks
// to be run on demand, which may be useful on systems that normally run them at
// low rate, or which do not trigger checks when the armable state might have
// changed. This command should return MAV_RESULT_ACCEPTED if it will run the
// checks. The results of the checks are usually then reported in SYS_STATUS
// messages (this is system-specific). The command should return
// MAV_RESULT_TEMPORARILY_REJECTED if the system is already armed.
class MavCmdRunPrearmChecks : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 401;
  MavCmdRunPrearmChecks() { command = kCommandId; }

 protected:
  virtual void pack();
  virtual void unpack();
};
// Turns illuminators ON/OFF. An illuminator is a light source that is used for
// lighting up dark areas external to the sytstem: e.g. a torch or searchlight
// (as opposed to a light source for illuminating the system itself, e.g. an
// indicator light).
class MavCmdIlluminatorOnOff : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 405;
  MavCmdIlluminatorOnOff() { command = kCommandId; }
  // 0: Illuminators OFF, 1: Illuminators ON
  float Enable = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request the home position from the vehicle. The vehicle will ACK the command
// and then emit the HOME_POSITION message.
class MavCmdGetHomePosition : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 410;
  MavCmdGetHomePosition() { command = kCommandId; }

 protected:
  virtual void pack();
  virtual void unpack();
};
// Inject artificial failure for testing purposes. Note that autopilots should
// implement an additional protection before accepting this command such as a
// specific param setting.
class MavCmdInjectFailure : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 420;
  MavCmdInjectFailure() { command = kCommandId; }
  // The unit which is affected by the failure.
  float FailureUnit = 0;
  // The type how the failure manifests itself.
  float FailureType = 0;
  // Instance affected by failure (0 to signal all).
  float Instance = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Starts receiver pairing.
class MavCmdStartRxPair : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 500;
  MavCmdStartRxPair() { command = kCommandId; }
  // 0:Spektrum.
  float Spektrum = 0;
  // RC type.
  float RcType = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request the interval between messages for a particular MAVLink message ID.
// The receiver should ACK the command and then emit its response in a
// MESSAGE_INTERVAL message.
class MavCmdGetMessageInterval : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 510;
  MavCmdGetMessageInterval() { command = kCommandId; }
  // The MAVLink message ID
  float MessageId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set the interval between messages for a particular MAVLink message ID. This
// interface replaces REQUEST_DATA_STREAM.
class MavCmdSetMessageInterval : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 511;
  MavCmdSetMessageInterval() { command = kCommandId; }
  // The MAVLink message ID
  float MessageId = 0;
  // The interval between two messages. Set to -1 to disable and 0 to request
  // default rate.
  float Interval = 0;
  // Target address of message stream (if message has target address fields). 0:
  // Flight-stack default (recommended), 1: address of requestor, 2: broadcast.
  float ResponseTarget = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request the target system(s) emit a single instance of a specified message
// (i.e. a "one-shot" version of MAV_CMD_SET_MESSAGE_INTERVAL).
class MavCmdRequestMessage : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 512;
  MavCmdRequestMessage() { command = kCommandId; }
  // The MAVLink message ID of the requested message.
  float MessageId = 0;
  // Use for index ID, if required. Otherwise, the use of this parameter (if
  // any) must be defined in the requested message. By default assumed not used
  // (0).
  float ReqParamP1 = 0;
  // The use of this parameter (if any), must be defined in the requested
  // message. By default assumed not used (0).
  float ReqParamP2 = 0;
  // The use of this parameter (if any), must be defined in the requested
  // message. By default assumed not used (0).
  float ReqParamP3 = 0;
  // The use of this parameter (if any), must be defined in the requested
  // message. By default assumed not used (0).
  float ReqParamP4 = 0;
  // The use of this parameter (if any), must be defined in the requested
  // message. By default assumed not used (0).
  float ReqParamP5 = 0;
  // Target address for requested message (if message has target address
  // fields). 0: Flight-stack default, 1: address of requestor, 2: broadcast.
  float ResponseTarget = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request MAVLink protocol version compatibility. All receivers should ACK the
// command and then emit their capabilities in an PROTOCOL_VERSION message
class MavCmdRequestProtocolVersion : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 519;
  MavCmdRequestProtocolVersion() { command = kCommandId; }
  // 1: Request supported protocol versions by all nodes on the network
  float Protocol = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request autopilot capabilities. The receiver should ACK the command and then
// emit its capabilities in an AUTOPILOT_VERSION message
class MavCmdRequestAutopilotCapabilities : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 520;
  MavCmdRequestAutopilotCapabilities() { command = kCommandId; }
  // 1: Request autopilot version
  float Version = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request camera information (CAMERA_INFORMATION).
class MavCmdRequestCameraInformation : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 521;
  MavCmdRequestCameraInformation() { command = kCommandId; }
  // 0: No action 1: Request camera capabilities
  float Capabilities = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request camera settings (CAMERA_SETTINGS).
class MavCmdRequestCameraSettings : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 522;
  MavCmdRequestCameraSettings() { command = kCommandId; }
  // 0: No Action 1: Request camera settings
  float Settings = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request storage information (STORAGE_INFORMATION). Use the command's
// target_component to target a specific component's storage.
class MavCmdRequestStorageInformation : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 525;
  MavCmdRequestStorageInformation() { command = kCommandId; }
  // Storage ID (0 for all, 1 for first, 2 for second, etc.)
  float StorageId = 0;
  // 0: No Action 1: Request storage information
  float Information = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Format a storage medium. Once format is complete, a STORAGE_INFORMATION
// message is sent. Use the command's target_component to target a specific
// component's storage.
class MavCmdStorageFormat : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 526;
  MavCmdStorageFormat() { command = kCommandId; }
  // Storage ID (1 for first, 2 for second, etc.)
  float StorageId = 0;
  // Format storage (and reset image log). 0: No action 1: Format storage
  float Format = 0;
  // Reset Image Log (without formatting storage medium). This will reset
  // CAMERA_CAPTURE_STATUS.image_count and CAMERA_IMAGE_CAPTURED.image_index. 0:
  // No action 1: Reset Image Log
  float ResetImageLog = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request camera capture status (CAMERA_CAPTURE_STATUS)
class MavCmdRequestCameraCaptureStatus : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 527;
  MavCmdRequestCameraCaptureStatus() { command = kCommandId; }
  // 0: No Action 1: Request camera capture status
  float CaptureStatus = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request flight information (FLIGHT_INFORMATION)
class MavCmdRequestFlightInformation : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 528;
  MavCmdRequestFlightInformation() { command = kCommandId; }
  // 1: Request flight information
  float FlightInformation = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Reset all camera settings to Factory Default
class MavCmdResetCameraSettings : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 529;
  MavCmdResetCameraSettings() { command = kCommandId; }
  // 0: No Action 1: Reset all settings
  float Reset = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set camera running mode. Use NaN for reserved values. GCS will send a
// MAV_CMD_REQUEST_VIDEO_STREAM_STATUS command after a mode change if the camera
// supports video streaming.
class MavCmdSetCameraMode : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 530;
  MavCmdSetCameraMode() { command = kCommandId; }
  // Camera mode
  float CameraMode = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set camera zoom. Camera must respond with a CAMERA_SETTINGS message (on
// success).
class MavCmdSetCameraZoom : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 531;
  MavCmdSetCameraZoom() { command = kCommandId; }
  // Zoom type
  float ZoomType = 0;
  // Zoom value. The range of valid values depend on the zoom type.
  float ZoomValue = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set camera focus. Camera must respond with a CAMERA_SETTINGS message (on
// success).
class MavCmdSetCameraFocus : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 532;
  MavCmdSetCameraFocus() { command = kCommandId; }
  // Focus type
  float FocusType = 0;
  // Focus value
  float FocusValue = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Set that a particular storage is the preferred location for saving photos,
// videos, and/or other media (e.g. to set that an SD card is used for storing
// videos). There can only be one preferred save location for each particular
// media type: setting a media usage flag will clear/reset that same flag if set
// on any other storage. If no flag is set the system should use its default
// storage. A target system can choose to always use default storage, in which
// case it should ACK the command with MAV_RESULT_UNSUPPORTED. A target system
// can choose to not allow a particular storage to be set as preferred storage,
// in which case it should ACK the command with MAV_RESULT_DENIED.
class MavCmdSetStorageUsage : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 533;
  MavCmdSetStorageUsage() { command = kCommandId; }
  // Storage ID (1 for first, 2 for second, etc.)
  float StorageId = 0;
  // Usage flags
  float Usage = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Tagged jump target. Can be jumped to with MAV_CMD_DO_JUMP_TAG.
class MavCmdJumpTag : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 600;
  MavCmdJumpTag() { command = kCommandId; }
  // Tag.
  float Tag = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Jump to the matching tag in the mission list. Repeat this action for the
// specified number of times. A mission should contain a single matching tag for
// each jump. If this is not the case then a jump to a missing tag should
// complete the mission, and a jump where there are multiple matching tags
// should always select the one with the lowest mission sequence number.
class MavCmdDoJumpTag : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 601;
  MavCmdDoJumpTag() { command = kCommandId; }
  // Target tag to jump to.
  float Tag = 0;
  // Repeat count.
  float Repeat = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// High level setpoint to be sent to a gimbal manager to set a gimbal attitude.
// It is possible to set combinations of the values below. E.g. an angle as well
// as a desired angular rate can be used to get to this angle at a certain
// angular rate, or an angular rate only will result in continuous turning. NaN
// is to be used to signal unset. Note: a gimbal is never to react to this
// command but only the gimbal manager.
class MavCmdDoGimbalManagerPitchyaw : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 1000;
  MavCmdDoGimbalManagerPitchyaw() { command = kCommandId; }
  // Pitch angle (positive to pitch up, relative to vehicle for FOLLOW mode,
  // relative to world horizon for LOCK mode).
  float PitchAngle = 0;
  // Yaw angle (positive to yaw to the right, relative to vehicle for FOLLOW
  // mode, absolute to North for LOCK mode).
  float YawAngle = 0;
  // Pitch rate (positive to pitch up).
  float PitchRate = 0;
  // Yaw rate (positive to yaw to the right).
  float YawRate = 0;
  // Gimbal manager flags to use.
  float GimbalManagerFlags = 0;
  // Component ID of gimbal device to address (or 1-6 for non-MAVLink gimbal), 0
  // for all gimbal device components. Send command multiple times for more than
  // one gimbal (but not all gimbals).
  float GimbalDeviceId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Gimbal configuration to set which sysid/compid is in primary and secondary
// control.
class MavCmdDoGimbalManagerConfigure : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 1001;
  MavCmdDoGimbalManagerConfigure() { command = kCommandId; }
  // Sysid for primary control (0: no one in control, -1: leave unchanged, -2:
  // set itself in control (for missions where the own sysid is still unknown),
  // -3: remove control if currently in control).
  float SysidPrimaryControl = 0;
  // Compid for primary control (0: no one in control, -1: leave unchanged, -2:
  // set itself in control (for missions where the own sysid is still unknown),
  // -3: remove control if currently in control).
  float CompidPrimaryControl = 0;
  // Sysid for secondary control (0: no one in control, -1: leave unchanged, -2:
  // set itself in control (for missions where the own sysid is still unknown),
  // -3: remove control if currently in control).
  float SysidSecondaryControl = 0;
  // Compid for secondary control (0: no one in control, -1: leave unchanged,
  // -2: set itself in control (for missions where the own sysid is still
  // unknown), -3: remove control if currently in control).
  float CompidSecondaryControl = 0;
  // Component ID of gimbal device to address (or 1-6 for non-MAVLink gimbal), 0
  // for all gimbal device components. Send command multiple times for more than
  // one gimbal (but not all gimbals).
  float GimbalDeviceId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Start image capture sequence. Sends CAMERA_IMAGE_CAPTURED after each capture.
// Use NaN for reserved values.
class MavCmdImageStartCapture : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2000;
  MavCmdImageStartCapture() { command = kCommandId; }
  // Desired elapsed time between two consecutive pictures (in seconds). Minimum
  // values depend on hardware (typically greater than 2 seconds).
  float Interval = 0;
  // Total number of images to capture. 0 to capture forever/until
  // MAV_CMD_IMAGE_STOP_CAPTURE.
  float TotalImages = 0;
  // Capture sequence number starting from 1. This is only valid for
  // single-capture (param3 == 1), otherwise set to 0. Increment the capture ID
  // for each capture command to prevent double captures when a command is
  // re-transmitted.
  float SequenceNumber = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Stop image capture sequence Use NaN for reserved values.
class MavCmdImageStopCapture : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2001;
  MavCmdImageStopCapture() { command = kCommandId; }

 protected:
  virtual void pack();
  virtual void unpack();
};
// Re-request a CAMERA_IMAGE_CAPTURED message.
class MavCmdRequestCameraImageCapture : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2002;
  MavCmdRequestCameraImageCapture() { command = kCommandId; }
  // Sequence number for missing CAMERA_IMAGE_CAPTURED message
  float Number = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Enable or disable on-board camera triggering system.
class MavCmdDoTriggerControl : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2003;
  MavCmdDoTriggerControl() { command = kCommandId; }
  // Trigger enable/disable (0 for disable, 1 for start), -1 to ignore
  float Enable = 0;
  // 1 to reset the trigger sequence, -1 or 0 to ignore
  float Reset = 0;
  // 1 to pause triggering, but without switching the camera off or retracting
  // it. -1 to ignore
  float Pause = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// If the camera supports point visual tracking
// (CAMERA_CAP_FLAGS_HAS_TRACKING_POINT is set), this command allows to initiate
// the tracking.
class MavCmdCameraTrackPoint : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2004;
  MavCmdCameraTrackPoint() { command = kCommandId; }
  // Point to track x value (normalized 0..1, 0 is left, 1 is right).
  float PointX = 0;
  // Point to track y value (normalized 0..1, 0 is top, 1 is bottom).
  float PointY = 0;
  // Point radius (normalized 0..1, 0 is image left, 1 is image right).
  float Radius = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// If the camera supports rectangle visual tracking
// (CAMERA_CAP_FLAGS_HAS_TRACKING_RECTANGLE is set), this command allows to
// initiate the tracking.
class MavCmdCameraTrackRectangle : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2005;
  MavCmdCameraTrackRectangle() { command = kCommandId; }
  // Top left corner of rectangle x value (normalized 0..1, 0 is left, 1 is
  // right).
  float TopLeftCorner = 0;
  // Top left corner of rectangle y value (normalized 0..1, 0 is top, 1 is
  // bottom).
  float TopLeftCorner2 = 0;
  // Bottom right corner of rectangle x value (normalized 0..1, 0 is left, 1 is
  // right).
  float BottomRightCorner = 0;
  // Bottom right corner of rectangle y value (normalized 0..1, 0 is top, 1 is
  // bottom).
  float BottomRightCorner2 = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Stops ongoing tracking.
class MavCmdCameraStopTracking : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2010;
  MavCmdCameraStopTracking() { command = kCommandId; }

 protected:
  virtual void pack();
  virtual void unpack();
};
// Starts video capture (recording).
class MavCmdVideoStartCapture : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2500;
  MavCmdVideoStartCapture() { command = kCommandId; }
  // Video Stream ID (0 for all streams)
  float StreamId = 0;
  // Frequency CAMERA_CAPTURE_STATUS messages should be sent while recording (0
  // for no messages, otherwise frequency)
  float StatusFrequency = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Stop the current video capture (recording).
class MavCmdVideoStopCapture : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2501;
  MavCmdVideoStopCapture() { command = kCommandId; }
  // Video Stream ID (0 for all streams)
  float StreamId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Start video streaming
class MavCmdVideoStartStreaming : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2502;
  MavCmdVideoStartStreaming() { command = kCommandId; }
  // Video Stream ID (0 for all streams, 1 for first, 2 for second, etc.)
  float StreamId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Stop the given video stream
class MavCmdVideoStopStreaming : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2503;
  MavCmdVideoStopStreaming() { command = kCommandId; }
  // Video Stream ID (0 for all streams, 1 for first, 2 for second, etc.)
  float StreamId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request video stream information (VIDEO_STREAM_INFORMATION)
class MavCmdRequestVideoStreamInformation : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2504;
  MavCmdRequestVideoStreamInformation() { command = kCommandId; }
  // Video Stream ID (0 for all streams, 1 for first, 2 for second, etc.)
  float StreamId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request video stream status (VIDEO_STREAM_STATUS)
class MavCmdRequestVideoStreamStatus : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2505;
  MavCmdRequestVideoStreamStatus() { command = kCommandId; }
  // Video Stream ID (0 for all streams, 1 for first, 2 for second, etc.)
  float StreamId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request to start streaming logging data over MAVLink (see also LOGGING_DATA
// message)
class MavCmdLoggingStart : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2510;
  MavCmdLoggingStart() { command = kCommandId; }
  // Format: 0: ULog
  float Format = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request to stop streaming log data over MAVLink
class MavCmdLoggingStop : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2511;
  MavCmdLoggingStop() { command = kCommandId; }

 protected:
  virtual void pack();
  virtual void unpack();
};
class MavCmdAirframeConfiguration : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2520;
  MavCmdAirframeConfiguration() { command = kCommandId; }
  // Landing gear ID (default: 0, -1 for all)
  float LandingGearId = 0;
  // Landing gear position (Down: 0, Up: 1, NaN for no change)
  float LandingGearPosition = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request to start/stop transmitting over the high latency telemetry
class MavCmdControlHighLatency : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2600;
  MavCmdControlHighLatency() { command = kCommandId; }
  // Control transmission over high latency telemetry (0: stop, 1: start)
  float Enable = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Create a panorama at the current position
class MavCmdPanoramaCreate : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 2800;
  MavCmdPanoramaCreate() { command = kCommandId; }
  // Viewing angle horizontal of the panorama (+- 0.5 the total angle)
  float HorizontalAngle = 0;
  // Viewing angle vertical of panorama.
  float VerticalAngle = 0;
  // Speed of the horizontal rotation.
  float HorizontalSpeed = 0;
  // Speed of the vertical rotation.
  float VerticalSpeed = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request VTOL transition
class MavCmdDoVtolTransition : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 3000;
  MavCmdDoVtolTransition() { command = kCommandId; }
  // The target VTOL state. For normal transitions, only MAV_VTOL_STATE_MC and
  // MAV_VTOL_STATE_FW can be used.
  float State = 0;
  // Force immediate transition to the specified MAV_VTOL_STATE. 1: Force
  // immediate, 0: normal transition. Can be used, for example, to trigger an
  // emergency "Quadchute". Caution: Can be dangerous/damage vehicle, depending
  // on autopilot implementation of this command.
  float Immediate = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request authorization to arm the vehicle to a external entity, the arm
// authorizer is responsible to request all data that is needs from the vehicle
// before authorize or deny the request. If approved the progress of command_ack
// message should be set with period of time that this authorization is valid in
// seconds or in case it was denied it should be set with one of the reasons in
// ARM_AUTH_DENIED_REASON.
class MavCmdArmAuthorizationRequest : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 3001;
  MavCmdArmAuthorizationRequest() { command = kCommandId; }
  // Vehicle system id, this way ground station can request arm authorization on
  // behalf of any vehicle
  float SystemId = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// This command sets the submode to standard guided when vehicle is in guided
// mode. The vehicle holds position and altitude and the user can input the
// desired velocities along all three axes.
class MavCmdSetGuidedSubmodeStandard : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 4000;
  MavCmdSetGuidedSubmodeStandard() { command = kCommandId; }

 protected:
  virtual void pack();
  virtual void unpack();
};
// This command sets submode circle when vehicle is in guided mode. Vehicle
// flies along a circle facing the center of the circle. The user can input the
// velocity along the circle and change the radius. If no input is given the
// vehicle will hold position.
class MavCmdSetGuidedSubmodeCircle : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 4001;
  MavCmdSetGuidedSubmodeCircle() { command = kCommandId; }
  // Radius of desired circle in CIRCLE_MODE
  float Radius = 0;
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // Target latitude of center of circle in CIRCLE_MODE
  float Latitude = 0;
  // Target longitude of center of circle in CIRCLE_MODE
  float Longitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Delay mission state machine until gate has been reached.
class MavCmdConditionGate : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 4501;
  MavCmdConditionGate() { command = kCommandId; }
  // Geometry: 0: orthogonal to path between previous and next waypoint.
  float Geometry = 0;
  // Altitude: 0: ignore altitude
  float Usealtitude = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Fence return point (there can only be one such point in a geofence
// definition). If rally points are supported they should be used instead.
class MavCmdNavFenceReturnPoint : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 5000;
  MavCmdNavFenceReturnPoint() { command = kCommandId; }
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Fence vertex for an inclusion polygon (the polygon must not be
// self-intersecting). The vehicle must stay within this area. Minimum of 3
// vertices required.
class MavCmdNavFencePolygonVertexInclusion : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 5001;
  MavCmdNavFencePolygonVertexInclusion() { command = kCommandId; }
  // Polygon vertex count
  float VertexCount = 0;
  // Vehicle must be inside ALL inclusion zones in a single group, vehicle must
  // be inside at least one group, must be the same for all points in each
  // polygon
  float InclusionGroup = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Fence vertex for an exclusion polygon (the polygon must not be
// self-intersecting). The vehicle must stay outside this area. Minimum of 3
// vertices required.
class MavCmdNavFencePolygonVertexExclusion : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 5002;
  MavCmdNavFencePolygonVertexExclusion() { command = kCommandId; }
  // Polygon vertex count
  float VertexCount = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Circular fence area. The vehicle must stay inside this area.
class MavCmdNavFenceCircleInclusion : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 5003;
  MavCmdNavFenceCircleInclusion() { command = kCommandId; }
  // Radius.
  float Radius = 0;
  // Vehicle must be inside ALL inclusion zones in a single group, vehicle must
  // be inside at least one group
  float InclusionGroup = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Circular fence area. The vehicle must stay outside this area.
class MavCmdNavFenceCircleExclusion : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 5004;
  MavCmdNavFenceCircleExclusion() { command = kCommandId; }
  // Radius.
  float Radius = 0;
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Rally point. You can have multiple rally points defined.
class MavCmdNavRallyPoint : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 5100;
  MavCmdNavRallyPoint() { command = kCommandId; }
  // Latitude
  float Latitude = 0;
  // Longitude
  float Longitude = 0;
  // Altitude
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Commands the vehicle to respond with a sequence of messages UAVCAN_NODE_INFO,
// one message per every UAVCAN node that is online. Note that some of the
// response messages can be lost, which the receiver can detect easily by
// checking whether every received UAVCAN_NODE_STATUS has a matching message
// UAVCAN_NODE_INFO received earlier; if not, this command should be sent again
// in order to request re-transmission of the node information messages.
class MavCmdUavcanGetNodeInfo : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 5200;
  MavCmdUavcanGetNodeInfo() { command = kCommandId; }

 protected:
  virtual void pack();
  virtual void unpack();
};
// Trigger the start of an ADSB-out IDENT. This should only be used when
// requested to do so by an Air Traffic Controller in controlled airspace. This
// starts the IDENT which is then typically held for 18 seconds by the hardware
// per the Mode A, C, and S transponder spec.
class MavCmdDoAdsbOutIdent : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 10001;
  MavCmdDoAdsbOutIdent() { command = kCommandId; }

 protected:
  virtual void pack();
  virtual void unpack();
};
// Deploy payload on a Lat / Lon / Alt position. This includes the navigation to
// reach the required release position and velocity.
class MavCmdPayloadPrepareDeploy : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 30001;
  MavCmdPayloadPrepareDeploy() { command = kCommandId; }
  // Operation mode. 0: prepare single payload deploy (overwriting previous
  // requests), but do not execute it. 1: execute payload deploy immediately
  // (rejecting further deploy commands during execution, but allowing abort).
  // 2: add payload deploy to existing deployment list.
  float OperationMode = 0;
  // Desired approach vector in compass heading. A negative value indicates the
  // system can define the approach vector at will.
  float ApproachVector = 0;
  // Desired ground speed at release time. This can be overridden by the
  // airframe in case it needs to meet minimum airspeed. A negative value
  // indicates the system can define the ground speed at will.
  float GroundSpeed = 0;
  // Minimum altitude clearance to the release position. A negative value
  // indicates the system can define the clearance at will.
  float AltitudeClearance = 0;
  // Latitude. Note, if used in MISSION_ITEM (deprecated) the units are degrees
  // (unscaled)
  float Latitude = 0;
  // Longitude. Note, if used in MISSION_ITEM (deprecated) the units are degrees
  // (unscaled)
  float Longitude = 0;
  // Altitude (MSL)
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Control the payload deployment.
class MavCmdPayloadControlDeploy : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 30002;
  MavCmdPayloadControlDeploy() { command = kCommandId; }
  // Operation mode. 0: Abort deployment, continue normal mission. 1: switch to
  // payload deployment mode. 100: delete first payload deployment request. 101:
  // delete all payload deployment requests.
  float OperationMode = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Magnetometer calibration based on provided known yaw. This allows for fast
// calibration using WMM field tables in the vehicle, given only the known yaw
// of the vehicle. If Latitude and longitude are both zero then use the current
// vehicle location.
class MavCmdFixedMagCalYaw : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 42006;
  MavCmdFixedMagCalYaw() { command = kCommandId; }
  // Yaw of vehicle in earth frame.
  float Yaw = 0;
  // CompassMask, 0 for all.
  float Compassmask = 0;
  // Latitude.
  float Latitude = 0;
  // Longitude.
  float Longitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Command to operate winch.
class MavCmdDoWinch : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 42600;
  MavCmdDoWinch() { command = kCommandId; }
  // Winch instance number.
  float Instance = 0;
  // Action to perform.
  float Action = 0;
  // Length of line to release (negative to wind).
  float Length = 0;
  // Release rate (negative to wind).
  float Rate = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined waypoint item. Ground Station will show the Vehicle as flying
// through this item.
class MavCmdWaypointUser1 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31000;
  MavCmdWaypointUser1() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // Latitude unscaled
  float Latitude = 0;
  // Longitude unscaled
  float Longitude = 0;
  // Altitude (MSL)
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined waypoint item. Ground Station will show the Vehicle as flying
// through this item.
class MavCmdWaypointUser2 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31001;
  MavCmdWaypointUser2() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // Latitude unscaled
  float Latitude = 0;
  // Longitude unscaled
  float Longitude = 0;
  // Altitude (MSL)
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined waypoint item. Ground Station will show the Vehicle as flying
// through this item.
class MavCmdWaypointUser3 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31002;
  MavCmdWaypointUser3() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // Latitude unscaled
  float Latitude = 0;
  // Longitude unscaled
  float Longitude = 0;
  // Altitude (MSL)
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined waypoint item. Ground Station will show the Vehicle as flying
// through this item.
class MavCmdWaypointUser4 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31003;
  MavCmdWaypointUser4() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // Latitude unscaled
  float Latitude = 0;
  // Longitude unscaled
  float Longitude = 0;
  // Altitude (MSL)
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined waypoint item. Ground Station will show the Vehicle as flying
// through this item.
class MavCmdWaypointUser5 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31004;
  MavCmdWaypointUser5() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // Latitude unscaled
  float Latitude = 0;
  // Longitude unscaled
  float Longitude = 0;
  // Altitude (MSL)
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined spatial item. Ground Station will not show the Vehicle as flying
// through this item. Example: ROI item.
class MavCmdSpatialUser1 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31005;
  MavCmdSpatialUser1() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // Latitude unscaled
  float Latitude = 0;
  // Longitude unscaled
  float Longitude = 0;
  // Altitude (MSL)
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined spatial item. Ground Station will not show the Vehicle as flying
// through this item. Example: ROI item.
class MavCmdSpatialUser2 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31006;
  MavCmdSpatialUser2() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // Latitude unscaled
  float Latitude = 0;
  // Longitude unscaled
  float Longitude = 0;
  // Altitude (MSL)
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined spatial item. Ground Station will not show the Vehicle as flying
// through this item. Example: ROI item.
class MavCmdSpatialUser3 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31007;
  MavCmdSpatialUser3() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // Latitude unscaled
  float Latitude = 0;
  // Longitude unscaled
  float Longitude = 0;
  // Altitude (MSL)
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined spatial item. Ground Station will not show the Vehicle as flying
// through this item. Example: ROI item.
class MavCmdSpatialUser4 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31008;
  MavCmdSpatialUser4() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // Latitude unscaled
  float Latitude = 0;
  // Longitude unscaled
  float Longitude = 0;
  // Altitude (MSL)
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined spatial item. Ground Station will not show the Vehicle as flying
// through this item. Example: ROI item.
class MavCmdSpatialUser5 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31009;
  MavCmdSpatialUser5() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // Latitude unscaled
  float Latitude = 0;
  // Longitude unscaled
  float Longitude = 0;
  // Altitude (MSL)
  float Altitude = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined command. Ground Station will not show the Vehicle as flying
// through this item. Example: MAV_CMD_DO_SET_PARAMETER item.
class MavCmdUser1 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31010;
  MavCmdUser1() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // User defined
  float UserDefined5 = 0;
  // User defined
  float UserDefined6 = 0;
  // User defined
  float UserDefined7 = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined command. Ground Station will not show the Vehicle as flying
// through this item. Example: MAV_CMD_DO_SET_PARAMETER item.
class MavCmdUser2 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31011;
  MavCmdUser2() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // User defined
  float UserDefined5 = 0;
  // User defined
  float UserDefined6 = 0;
  // User defined
  float UserDefined7 = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined command. Ground Station will not show the Vehicle as flying
// through this item. Example: MAV_CMD_DO_SET_PARAMETER item.
class MavCmdUser3 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31012;
  MavCmdUser3() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // User defined
  float UserDefined5 = 0;
  // User defined
  float UserDefined6 = 0;
  // User defined
  float UserDefined7 = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined command. Ground Station will not show the Vehicle as flying
// through this item. Example: MAV_CMD_DO_SET_PARAMETER item.
class MavCmdUser4 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31013;
  MavCmdUser4() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // User defined
  float UserDefined5 = 0;
  // User defined
  float UserDefined6 = 0;
  // User defined
  float UserDefined7 = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// User defined command. Ground Station will not show the Vehicle as flying
// through this item. Example: MAV_CMD_DO_SET_PARAMETER item.
class MavCmdUser5 : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 31014;
  MavCmdUser5() { command = kCommandId; }
  // User defined
  float UserDefined = 0;
  // User defined
  float UserDefined2 = 0;
  // User defined
  float UserDefined3 = 0;
  // User defined
  float UserDefined4 = 0;
  // User defined
  float UserDefined5 = 0;
  // User defined
  float UserDefined6 = 0;
  // User defined
  float UserDefined7 = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
// Request forwarding of CAN packets from the given CAN bus to this component.
// CAN Frames are sent using CAN_FRAME and CANFD_FRAME messages
class MavCmdCanForward : public MavLinkCommand {
 public:
  const static uint16_t kCommandId = 32000;
  MavCmdCanForward() { command = kCommandId; }
  // Bus number (0 to disable forwarding, 1 for first bus, 2 for 2nd bus, 3 for
  // 3rd bus).
  float Bus = 0;

 protected:
  virtual void pack();
  virtual void unpack();
};
}  // namespace mavlinkcom

#endif
