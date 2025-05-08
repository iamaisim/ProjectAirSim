// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_MAVLINK_API_HPP_
#define MULTIROTOR_API_INCLUDE_MAVLINK_API_HPP_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "MavLinkConnection.hpp"
#include "MavLinkMessages.hpp"
#include "MavLinkNode.hpp"
#include "MavLinkVehicle.hpp"
#include "common/pid_controller.hpp"
#include "common/smoothing_filter.hpp"
#include "common/timer.hpp"
#include "core_sim/actor/robot.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/sensors/airspeed.hpp"
#include "core_sim/sensors/barometer.hpp"
#include "core_sim/sensors/distance_sensor.hpp"
#include "core_sim/sensors/gps.hpp"
#include "core_sim/sensors/imu.hpp"
#include "core_sim/sensors/magnetometer.hpp"
#include "vtolfw_api_base.hpp"

namespace microsoft {
namespace projectairsim {

// MavLinkApi
// TODO: Should we use pimpl or some other pattern to hide the implementation?
class MavLinkApi : public VTOLFWApiBase {
 public:
  MavLinkApi() {}
  MavLinkApi(const Robot& robot, TransformTree* ptransformtree);

  ~MavLinkApi();

  //---------------------------------------------------------------------------
  // IController overrides

  void BeginUpdate() override;
  void EndUpdate() override;
  void Reset() override;
  void SetKinematics(const Kinematics* kinematics) override;
  void Update() override;
  std::vector<float> GetControlSignals(const std::string& actuator_id) override;
  const IController::GimbalState& GetGimbalSignal(
      const std::string& gimbal_id) override;

  //---------------------------------------------------------------------------
  // IMultirotorApi overrides

  bool EnableApiControl() override;
  bool DisableApiControl() override;
  bool IsApiControlEnabled() override;
  bool Arm(int64_t command_start_time_nanos) override;
  bool Disarm() override;
  ReadyState GetReadyState() const override;
  bool CanArm() const override;
  Kinematics GetKinematicsEstimated() const override;
  LandedState GetLandedState() const override;

  bool Takeoff(float timeout_sec, int64_t command_start_time_nanos) override;
  bool Land(float timeout_sec, int64_t command_start_time_nanos) override;
  bool MoveToPosition(float x, float y, float z, float velocity,
                      float timeout_sec, DrivetrainType drivetrain,
                      bool yaw_is_rate, float yaw, float lookahead,
                      float adaptive_lookahead,
                      int64_t command_start_time_nanos) override;
  bool GoHome(float timeout_sec, float velocity,
              int64_t command_start_time_nanos) override;
  bool Hover(int64_t command_start_time_nanos) override;
  bool RequestControl(int64_t /*command_start_time_nanos*/) override;
  bool SetMissionMode(int64_t command_start_time_nanos) override;

  // Switched to using service method request-response for all control commands,
  // but leaving the below pub-sub version commented out for reference in case
  // it's needed in the future.
  //   void OnSetpointNEDvelocityYawrate(const Topic& topic,
  //                                     const Message& message)
  //                                     override;

  //---------------------------------------------------------------------------
  // IVTOLFWApi overrides

  bool SetVTOLMode(VTOLMode vtolmode) override;

 protected:
  std::string GetControllerName() const override { return "PX4Api"; }

  //---------------------------------------------------------------------------
  // Low level commands

  void CommandMotorPWMs(float front_right_pwm, float rear_left_pwm,
                        float front_left_pwm, float rear_right_pwm) override;
  void CommandRollPitchYawrateThrottle(float roll, float pitch, float yaw_rate,
                                       float throttle) override;
  void CommandRollPitchYawZ(float roll, float pitch, float yaw,
                            float z) override;
  void CommandRollPitchYawThrottle(float roll, float pitch, float yaw,
                                   float throttle) override;
  void CommandRollPitchYawrateZ(float roll, float pitch, float yaw_rate,
                                float z) override;
  void CommandAngleRatesZ(float roll_rate, float pitch_rate, float yaw_rate,
                          float z) override;
  void CommandAngleRatesThrottle(float roll_rate, float pitch_rate,
                                 float yaw_rate, float throttle) override;
  void CommandHeading(float heading, float speed, float vz) override;
  void CommandVelocity(float vx, float vy, float vz, bool yaw_is_rate,
                       float yaw) override;
  void CommandVelocityZ(float vx, float vy, float z, bool yaw_is_rate,
                        float yaw) override;
  void CommandVelocityBody(float vx, float vy, float vz, bool yaw_is_rate,
                           float yaw) override;
  void CommandVelocityZBody(float vx, float vy, float z, bool yaw_is_rate,
                            float yaw) override;
  void CommandPosition(float x, float y, float z, bool yaw_is_rate,
                       float yaw) override;

  // controller configs
  void SetControllerGains(uint8_t controllerType, const std::vector<float>& kp,
                          const std::vector<float>& ki,
                          const std::vector<float>& kd) override;

  /************************* State APIs *********************************/
  GeoPoint GetGpsLocationEstimated() const override;
  const MultirotorApiParams& GetMultirotorApiParams() const override;

  /************************* basic config APIs
   * *********************************/
  float GetCommandPeriod()
      const override;  // time between two command required for drone in seconds
  float GetTakeoffZ()
      const override;  // the height above ground for the drone after successful
                       // takeoff (Z above ground is negative due to NED
                       // coordinate system).
  // noise in difference of two position coordinates. This is not GPS or
  // position accuracy which can be very low such as 1m. the difference between
  // two position cancels out transitional errors. Typically this would be 0.1m
  // or lower.
  float GetDistanceAccuracy() const override;

 protected:
  // optional overrides
  Vector3 GetPosition() const override;
  Vector3 GetVelocity() const override;
  Quaternion GetOrientation() const override;

  void BeforeTask() override;
  void AfterTask() override;

 private:
  struct MavLinkConnectionInfo {
    /* Default values are requires so uninitialized instance doesn't have random
     * values */

    bool use_serial = true;  // false means use UDP or TCP instead

    // Used to connect via HITL: needed only if use_serial = true
    std::string serial_port = "*";
    int baud_rate = 115200;

    // Used to connect to drone over UDP: needed only if use_serial = false and
    // use_tcp == false
    std::string udp_address = "127.0.0.1";
    int udp_port = 14560;

    // Used to accept connections from drone over TCP: needed only if use_tcp =
    // true
    bool lock_step = false;
    bool use_tcp = false;
    int tcp_port = 4560;

    // The PX4 SITL app requires receiving drone commands over a different
    // mavlink channel called the "ground control station" (or GCS) channel.
    std::string control_ip_address = "127.0.0.1";
    int control_port_local = 14540;
    int control_port_remote = 14580;

    // No actual documentation I can find for default port.
    // However, when the   "MNT_MODE_IN","MNT_MODE_OUT" parameters are passed in
    // these are chosen by PX4 and are printed to the terminal as part of the
    // output.
    int gimbal_port_local = 13280;
    int gimbal_port_remote = 13030;

    // The log viewer can be on a different machine, so you can configure it's
    // ip address and port here.
    // int logviewer_ip_port = 14388;
    // int logviewer_ip_sport =
    //     14389;  // for logging all messages we send to the vehicle.
    // std::string logviewer_ip_address = "";

    // The QGroundControl app can be on a different machine, and AirSim can act
    // as a proxy to forward the mavlink stream over to that machine if you
    // configure it's ip address and port here.
    int qgc_ip_port = 0;
    std::string qgc_ip_address = "";

    uint64_t timeout_lock_step_update_ms =
        100;  // Timeout to exit lock-step when sending sensor update to PX4
              // is delayed too long (milliseconds)
    uint64_t timeout_lock_step_actuator_ms =
        500;  // Timeout to exit lock-step when an actuator update from PX4
              // is delayed too long (milliseconds)
    uint64_t timeout_lock_step_hil_actuator_control_ms =
        100;  // Lock-step timeout when waiting for
              // HIL actuator control message from PX4 (milliseconds)

    // mavlink vehicle identifiers
    uint8_t sim_sysid = 142;
    int sim_compid = 42;
    uint8_t offboard_sysid = 134;
    int offboard_compid = 1;
    uint8_t vehicle_sysid = 135;
    int vehicle_compid = 1;

    // if you want to select a specific local network adapter so you can reach
    // certain remote machines (e.g. wifi versus ethernet) then you will want to
    // change the LocalHostIp accordingly.  This default only works when log
    // viewer and QGC are also on the same machine.  Whatever network you choose
    // it has to be the same one for external
    std::string local_host_ip = "127.0.0.1";

    std::string model = "Generic";

    std::map<std::string, float> params;
  };

 private:  // methods
  void LoadSettings(const Robot& robot);
  void GetSensors(const Robot& robot);
  void InitializeGimbalStatus(const Robot& robot);

  // connection setup and teardown
  void InitializeConnections();
  void OpenAllConnections();
  void CloseAllConnections();
  void Connect();
  void ConnectThread();
  void Disconnect();
  void ResetState();
  void SetNormalMode();
  void SetHILMode();

  void AddStatusMessage(const std::string& message);

  static std::string FindPX4();
  void CreateMavConnection(const MavLinkConnectionInfo& connection_info);
  void CreateMavSerialConnection(const std::string& port_name, int baud_rate);
  void CreateMavEthernetConnection(
      const MavLinkConnectionInfo& connection_info);
  void ConnectVehicle();

  bool ConnectToQGC();
  // bool ConnectToLogViewer();
  void CreateProxy(std::string name, std::string ip, int port,
                   std::string local_host_ip,
                   std::shared_ptr<mavlinkcom::MavLinkNode>& node,
                   std::shared_ptr<mavlinkcom::MavLinkConnection>& connection);

  // message handling
  void ProcessMavMessages(const mavlinkcom::MavLinkMessage& msg);
  void ProcessQgcMessages(const mavlinkcom::MavLinkMessage& msg);

  // Gimbal related
  void HandleGimbalMessages(const mavlinkcom::MavLinkMessage& msg);
  void SetupGimbalConnection(const std::string& remoteIpAddr);
  void SendGimbalDeviceInformation();
  void SendGimbalState();

  // simulation time handling
  void AdvanceSimTime(void);
  TimeMicro GetSimTimeMicros(void);
  void HandleLockStep(void);

  // send sensor data
  void SendSensorData();
  void SendHILSensor(const Vector3* acceleration, const Vector3* gyro,
                     const Vector3* mag, float* abs_pressure,
                     float* diff_pressure, float* pressure_alt);
  void SendDistanceSensor(float min_distance, float max_distance,
                          float current_distance, float sensor_type,
                          float sensor_id, Quaternion orientation);
  void SendHILGps(const GeoPoint& geo_point, const Vector3& velocity,
                  float velocity_xy, float cog, float eph, float epv,
                  int fix_type, unsigned int satellites_visible);
  void SendSystemTime(void);

  // Helpers
  void CheckValidVehicle();
  bool IsValidConnection() const;
  void UpdateState() const;
  void SetArmed(bool armed);
  void WaitForHomeLocation(float timeout_sec, int64_t command_start_time_nanos);
  void WaitForStableGroundPosition(float timeout_sec,
                                   int64_t command_start_time_nanos);
  bool StartOffboardMode();
  void EndOffboardMode();
  void EnsureSafeMode();
  void NormalizeRotorControls();
  bool SendTestMessage(std::shared_ptr<mavlinkcom::MavLinkNode> node);
  void SendParams();
  void MonitorGroundAltitude();
  int TimeoutToMilliseconds(float timeout_sec);

 private:  // variables
  std::unordered_map<std::string, int> actuator_id_to_output_idx_map_;
  std::unordered_map<std::string, int> gimbal_id_to_component_id_;
  std::unordered_map<int, GimbalState> gimbal_component_id_to_state_;
  // How many can we support with a single connection?
  static constexpr int kControlOutputsCount = 16;

  // const SensorCollection* sensors_;
  AirspeedSensor* airspeed_sensor_ = nullptr;
  Imu* imu_sensor_ = nullptr;
  Magnetometer* magnetometer_sensor_ = nullptr;
  Barometer* barometer_sensor_ = nullptr;
  DistanceSensor* distance_sensor_ = nullptr;
  Gps* gps_sensor_ = nullptr;

  mutable std::mutex hil_controls_mutex_;
  MavLinkConnectionInfo connection_info_;
  float control_outputs_[kControlOutputsCount];
  bool is_simulation_mode_;

  static const int pixhawkVendorId =
      9900;  ///< Vendor ID (of 3D Robotics, Inc.) for Pixhawk boards and PX4
             ///< Flow
  static const int pixhawkFMUV5ProductId =
      50;  ///< Product ID for Pixhawk FMU V5 board
  static const int pixhawkFMUV4ProductId =
      18;  ///< Product ID for Pixhawk FMU V2 board (V2 is from the original
           ///< AirSim v1 comment, but is this actually FMU V4?)
  static const int pixhawkFMUV2ProductId =
      17;  ///< Product ID for Pixhawk FMU V2 board
  static const int pixhawkFMUV2OldBootloaderProductId =
      22;  ///< Product ID for Bootloader on older Pixhawk FMU V2 boards
  static const int pixhawkFMUV1ProductId =
      16;  ///< Product ID for PX4 FMU V1 board
  static const int messageReceivedTimeout = 10;            ///< Seconds
  static const int gimbalStatusUpdateTimePeriod = 100000;  // In Micros

  // std::shared_ptr<mavlinkcom::MavLinkNode> logviewer_proxy_,
  //    logviewer_out_proxy_,
  std::shared_ptr<mavlinkcom::MavLinkNode> qgc_proxy_;

  size_t status_messages_MaxSize = 5000;

  std::shared_ptr<mavlinkcom::MavLinkNode> hil_node_;
  std::shared_ptr<mavlinkcom::MavLinkNode> gimbal_node_;
  std::shared_ptr<mavlinkcom::MavLinkConnection> connection_;
  // std::shared_ptr<mavlinkcom::MavLinkVideoServer> video_server_;
  std::shared_ptr<MultirotorApiBase> mav_vehicle_control_;

  mavlinkcom::MavLinkAttPosMocap MocapPoseStampedMessage;
  mavlinkcom::MavLinkHeartbeat HeartbeatMessage;
  mavlinkcom::MavLinkSetMode SetModeMessage;
  mavlinkcom::MavLinkStatustext StatusTextMessage;
  mavlinkcom::MavLinkHilControls HilControlsMessage;
  mavlinkcom::MavLinkHilActuatorControls HilActuatorControlsMessage;
  mavlinkcom::MavLinkGpsRawInt MavLinkGpsRawInt;
  mavlinkcom::MavLinkCommandLong CommandLongMessage;
  mavlinkcom::MavLinkLocalPositionNed MavLinkLocalPositionNed;

  mavlinkcom::MavLinkGimbalDeviceInformation GimbalDeviceInformation;
  mavlinkcom::MavLinkGimbalDeviceSetAttitude GimbalDeviceSetAttitude;
  mavlinkcom::MavLinkGimbalDeviceAttitudeStatus GimbalDeviceAttitudeStatus;

  mavlinkcom::MavLinkHilSensor last_sensor_message_;
  mavlinkcom::MavLinkDistanceSensor last_distance_message_;
  mavlinkcom::MavLinkHilGps last_gps_message_;

  std::mutex mocap_pose_mutex_, heartbeat_mutex_, set_mode_mutex_,
      status_text_mutex_, last_message_mutex_, gimbal_mutex_;

  // variables required for VehicleApiBase implementation
  bool got_first_heartbeat_ = false, is_hil_mode_set_ = false,
       is_armed_ = false;
  mavlinkcom::MAV_TYPE vehicle_type_;
  bool send_params_ = false;
  std::queue<std::string> status_messages_;
  int hil_state_freq_;
  bool actuators_message_supported_ = false;
  uint64_t hil_sensor_clock_ = 0;
  bool was_reset_ = false;
  bool has_home_ = false;
  bool is_ready_ = false;
  bool has_gps_lock_ = false;
  bool enable_gimbal_ = false;      // Is a gimbal actuator mounted for PX4
                                    // (determined by the PX4 parameters)
  bool lock_step_active_ = false;   // If true, we're performing the simulation
                                    // in lock-step with the controller
  bool lock_step_enabled_ = false;  // If true, perform simulation in lock-step
                                    // with the controller if possible
  bool received_actuator_controls_ = false;
  std::mutex mutex_received_actuator_controls_;
  std::condition_variable cv_received_actuator_controls_;
  std::string is_ready_message_;
  Pose mocap_pose_;
  std::thread connect_thread_;
  bool connecting_ = false;  // If true, we're trying to establish a connection
                             // to the controller
  bool connected_ = false;   // If true, we've established a connection to the
                             // controller and started vehicle setup
  bool connected_vehicle_ =
      false;  // If true, we've completed the connection and vehicle setup and
              // the vehicle is ready for use
  SmoothingFilter<float> ground_filter_;
  double ground_variance_ = 1;
  const double GroundTolerance = 0.1;

  uint64_t last_actuator_time_ =
      0;  // Timestamp when we received last control/actuator message from PX4
  uint64_t last_gimbal_time_ = 0;
  uint64_t last_gps_time_ = 0;
  uint64_t last_hil_sensor_time_ = 0;
  uint64_t last_sys_time_ = 0;
  uint64_t last_update_time_ = 0;
  TimeMicro sim_time_us_ = 0;

  // additional variables required for MultirotorApiBase implementation
  // this is optional for methods that might not use vehicle commands
  std::shared_ptr<mavlinkcom::MavLinkVehicle> mav_vehicle_;
  float target_height_;
  bool is_api_control_enabled_;
  PidController thrust_controller_;
  Timer hil_message_timer_;
  Timer gcs_message_timer_;

  // every time we return status update, we need to check if we have new data
  // this is why below two variables are marked as mutable
  mutable int state_version_;
  mutable mavlinkcom::VehicleState current_state_;

  // Debug
  int heartbeat_messages_received_count_ = 0;
  bool first_message_sent_ = false;
  bool first_gps_message_sent_ = false;
  bool got_first_actuator_control_ = false;
  bool setup_gimbal_ =
      false;  // bool to check if the gimbal connection is already setup
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_MAVLINK_API_HPP_