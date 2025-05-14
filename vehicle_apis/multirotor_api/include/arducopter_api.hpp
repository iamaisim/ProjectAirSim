// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_INCLUDE_ARDUCOPTER_API_HPP_
#define MULTIROTOR_API_INCLUDE_ARDUCOPTER_API_HPP_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "UdpSocket.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/sensors/airspeed.hpp"
#include "core_sim/sensors/barometer.hpp"
#include "core_sim/sensors/distance_sensor.hpp"
#include "core_sim/sensors/gps.hpp"
#include "core_sim/sensors/imu.hpp"
// #include "core_sim/sensors/lidar.hpp"
#include "core_sim/sensors/magnetometer.hpp"
#include "vtolfw_api_base.hpp"

namespace microsoft {
namespace projectairsim {

// ArduCopterApi
// TODO: Should we use pimpl or some other pattern to hide the implementation?
class ArduCopterApi : public VTOLFWApiBase {
 public:
  ArduCopterApi() {}
  ArduCopterApi(const Robot& robot, TransformTree* ptransformtree);

  ~ArduCopterApi();

  //---------------------------------------------------------------------------
  // IController overrides

  void BeginUpdate() override;
  void EndUpdate() override;
  void Reset() override;
  void SetKinematics(const Kinematics* kinematics) override;
  void Update() override;
  std::vector<float> GetControlSignals(const std::string& actuator_id) override;
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
  std::string GetControllerName() const override { return "ArduCopterApi"; }

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
  void CommandVelocityBody(float vx, float vy, float vz,
                                   bool yaw_is_rate, float yaw) override;

  void CommandVelocityZBody(float vx, float vy, float z,
                                    bool yaw_is_rate, float yaw) override;
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

 private:  // types
  static const int k_ardu_copter_rotor_control_count_ = 11;
  struct RotorControlMessage {
    uint16_t pwm[k_ardu_copter_rotor_control_count_];
  };
  struct ArduPilotConnectionInfo {
    /* Default values are requires so uninitialized instance doesn't have random
     * values */

    // bool use_serial = true;  // false means use UDP or TCP instead

    // Used to connect via HITL: needed only if use_serial = true
    // std::string serial_port = "*";
    // int baud_rate = 115200;

    // udp
    std::string ardupilot_ip = "127.0.0.1";
    int ardupilot_udp_port = 9003;

    // bool lock_step = false;
    std::string local_host_ip = "127.0.0.1";
    int local_host_udp_port = 9002;
  };
  std::unique_ptr<mavlinkcom::UdpSocket> udp_socket_;

 private:  // methods
  void LoadSettings(const Robot& robot);
  void GetSensors(const Robot& robot);
  void ReciveRotorsControls();

  // connection setup and teardown
  void InitializeConnections();
  void OpenAllConnections();
  void CloseAllConnections();
  void Connect();
  void ConnectThread();
  void Disconnect();
  void ResetState();

  void AddStatusMessage(const std::string& message) const;

  void CreateUdpConnection(const ArduPilotConnectionInfo& connection_info);

  // simulation time handling
  void AdvanceSimTime(void);
  TimeMicro GetSimTimeMicros(void);
  // void HandleLockStep(void);

  // send sensor data
  void SendSensorData();

  // Helpers
  void NormalizeRotorControls();

 private:  // variables
  uint16_t port_;
  std::string ip_;
  std::unordered_map<std::string, int> actuator_id_to_output_idx_map_;
  // How many can we support with a single connection?
  static constexpr int k_control_outputs_count_ = 16;

  // const SensorCollection* sensors_;
  AirspeedSensor* airspeed_sensor_ = nullptr;
  Imu* imu_sensor_ = nullptr;
  Magnetometer* magnetometer_sensor_ = nullptr;
  Barometer* barometer_sensor_ = nullptr;
  DistanceSensor* distance_sensor_ = nullptr;
  Gps* gps_sensor_ = nullptr;
  ArduPilotConnectionInfo connection_info_;
  // Lidar* lidar_sensor_ = nullptr;
  float control_outputs_[k_control_outputs_count_];
  bool is_simulation_mode_;
  // bool use_lidar_ = false;
  bool use_distance_sensor_ = false;

  // variables required for VehicleApiBase implementation
  // bool is_hil_mode_set_ = false;
  std::queue<std::string> status_messages_;
  bool is_ready_ = false;
  bool has_gps_lock_ = false;
  // bool lock_step_active_ = false;
  // bool lock_step_enabled_ = false;
  // bool received_actuator_controls_ = false;
  // std::mutex mutex_received_actuator_controls_;
  // std::condition_variable cv_received_actuator_controls_;
  std::string is_ready_message_;
  std::thread connect_thread_;
  bool connecting_ = false;  // If true, we're trying to establish a connection
                             // to the controller
  bool connected_ = false;   // If true, we've established a connection to the
                             // controller and started vehicle setup
  // bool connected_vehicle_ =
  //   false;  If true, we've completed the connection and vehicle setup and
  //           the vehicle is ready for use
  uint64_t last_gps_time_ = 0;
  // uint64_t last_hil_sensor_time_ = 0;
  // uint64_t last_update_time_ = 0;
  TimeMicro sim_time_us_ = 0;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_ARDUCOPTER_API_HPP_