// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_JSBSIM_API_HPP_
#define MULTIROTOR_API_INCLUDE_JSBSIM_API_HPP_

#include <memory>
#include <string>
#include <vector>

#include "core_sim/actor/robot.hpp"
#include "core_sim/runtime_components.hpp"
#include "core_sim/service_method.hpp"
#include "core_sim/transforms/transform_tree.hpp"
#include "simple_flight/AirSimSimpleFlightBoard.hpp"
#include "simple_flight/AirSimSimpleFlightCommLink.hpp"
#include "simple_flight/AirSimSimpleFlightEstimator.hpp"
#include "simple_flight/AirSimSimpleFlightEstimatorFW.hpp"
#include "simple_flight/firmware/Firmware.hpp"
#include "landing_gear_api_base.hpp"


namespace microsoft {
namespace projectairsim {

// todo "firmware" / "firmware wrapper api" or "api" type (wrt px4 / mavlink)
//enum class MultirotorApiType { kJSBSim = 0 };

// JSBSimApi
// TODO: Should we use pimpl or some other pattern to hide the implementation?
class JSBSimApi : public LandingGearApiBase   { 
 public:
  JSBSimApi() {}
  JSBSimApi(const Robot& robot, TransformTree* ptransformtree);

  virtual ~JSBSimApi() {}

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
  bool CanArm() const override;

  float GetTakeOffZ() override; 
  bool SetTakeOffZ(float takeoffZ) override;
  bool Takeoff(
    float timeout_sec, TimeNano _service_method_start_time) override;
  bool Land(float timeout_sec, int64_t command_start_time_nanos) override;
  //SetBrakes
  bool SetBrakes(float value);
  bool MoveToZ(float z, float velocity, float timeout_sec, bool yaw_is_rate,
              float yaw, float lookahead, float adaptive_lookahead,
              int64_t command_start_time_nanos) override;
  bool MoveToPosition(float x, float y, float z, float velocity,
                      float timeout_sec, DrivetrainType drivetrain,
                      bool yaw_is_rate, float yaw, float lookahead,
                      float adaptive_lookahead,
                      int64_t command_start_time_nanos) override;
  bool MoveByHeading(float heading, float speed, 
                     float vz, float duration,
                     float heading_margin, float yaw_rate,
                     float timeout_sec,
                     int64_t command_start_time_nanos) override;
  float GetJSBSimProperty(const std::string& property);
  bool SetJSBSimProperty(const std::string& property, float value);

  // Switched to using service method request-response for all control commands,
  // but leaving the below pub-sub version commented out for reference in case
  // it's needed in the future.
  //   void OnSetpointNEDvelocityYawrate(const Topic& topic,
  //                                     const Message& message)
  //                                     override;

  //---------------------------------------------------------------------------
  // IVTOLFWApi overrides

 protected:
  //---------------------------------------------------------------------------
  // MultirotorApiBase overrides
  std::string GetControllerName() const override { return "JSBSimApi"; }

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
  void CommandPosition(float x, float y, float z, bool yaw_is_rate,
                       float yaw) override;
  void CommandVelocityBody(float vx, float vy, float vz,
                           bool yaw_is_rate, float yaw) override;
  void CommandVelocityZBody(float vx, float vy, float z,
                            bool yaw_is_rate, float yaw) override;

  // controller configs
  void SetControllerGains(uint8_t controllerType, const std::vector<float>& kp,
                          const std::vector<float>& ki,
                          const std::vector<float>& kd) override;
  ReadyState GetReadyState() const override;
  GeoPoint GetGpsLocationEstimated() const override;

  /************************* State APIs *********************************/
  Kinematics GetKinematicsEstimated() const override;
  LandedState GetLandedState() const override;
  // GeoPoint GetGpsLocation() const override;
  const MultirotorApiParams& GetMultirotorApiParams() const override;

  /************************* Basic Config APIs **************************/
  float GetCommandPeriod()
      const override;  // time between two command required for drone in seconds
  // noise in difference of two position coordinates. This is not GPS or
  // position accuracy which can be very low such as 1m. the difference between
  // two position cancels out transitional errors. Typically this would be 0.1m
  // or lower.
  float GetDistanceAccuracy() const override;

 protected:
  // optional overrides
  Vector3 GetAngles() const override;
  Vector3 GetPosition() const override;
  Vector3 GetVelocity() const override;
  Quaternion GetOrientation() const override;

 protected:
  void RegisterServiceMethods(void) override;

 private:
  // The kind of target vehicle
  enum class VehicleKind {
    Multirotor,      // Multirotor helicopter (aka., multicopter)
    VTOLTailsitter,  // VTOL tail-sitting fixed-wing vehicle with multicopter
                     // and fixed-wing modes
    VTOLTiltrotor,  // VTOL tilt-rotor fixed-wing vehicle with multicopter
                     // and fixed-wing modes
    FixedWing,      // Fixed-wing aircraft
    Other,
  };                 // enum class VehicleKind

 private:
  void LoadSettings(const Robot& robot);
  static void Break(std::shared_ptr<JSBSim::FGFDMExec> model);

 private:
  std::unordered_map<std::string, int> actuator_id_to_output_idx_map_;
  std::unique_ptr<AirSimSimpleFlightBoard> board_;
  std::unique_ptr<AirSimSimpleFlightCommLink> comm_link_;
  std::unique_ptr<AirSimSimpleFlightEstimator> estimator_;
  std::unique_ptr<AirSimSimpleFlightEstimatorFW> estimator_fw_;
  std::unique_ptr<simple_flight::IFirmware> firmware_;
  uint64_t millis_rc_input_last_update_ = 0;  // Timestamp of when RC input was last received
  std::unique_ptr<simple_flight::Params> params_;  // todo params should become simple_flight_api_settings
  Topic rc_input_topic_;  // RC input topic
  MultirotorApiParams safety_params_;
  std::mutex update_lock_;
  VehicleKind vehicle_kind_ = VehicleKind::Multirotor;  // The type of vehicle being controlled
  float takeoff_z_ = 100.0f;  // The z-coordinate of the takeoff position
  bool is_api_control_enabled_ = false;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_SIMPLE_FLIGHT_API_HPP_