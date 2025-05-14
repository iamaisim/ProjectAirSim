// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

#ifndef MULTIROTOR_API_INCLUDE_SIMPLE_FLIGHT_API_HPP_
#define MULTIROTOR_API_INCLUDE_SIMPLE_FLIGHT_API_HPP_

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
#include "vtolfw_api_base.hpp"

namespace microsoft {
namespace projectairsim {

// todo "firmware" / "firmware wrapper api" or "api" type (wrt px4 / mavlink)
enum class MultirotorApiType { kSimpleFlight = 0 };

// SimpleFlightApi
// TODO: Should we use pimpl or some other pattern to hide the implementation?
class SimpleFlightApi : public VTOLFWApiBase {
 public:
  SimpleFlightApi() {}
  SimpleFlightApi(const Robot& robot, TransformTree* ptransformtree);

  virtual ~SimpleFlightApi() {}

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
  ReadyState GetReadyState() const override;
  Kinematics GetKinematicsEstimated() const override;
  LandedState GetLandedState() const override;

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
  //---------------------------------------------------------------------------
  // MultirotorApiBase overrides
  void CreateTopics(void) override;
  std::string GetControllerName() const override { return "SimpleFlightApi"; }

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

  bool MoveByVelocityBodyFrame(float vx, float vy, float vz, float duration,
                               DrivetrainType drivetrain, bool yaw_is_rate,
                               float yaw, int64_t command_start_time_nanos) override;
  bool MoveByVelocityBodyFrameZ(float vx, float vy, float z, float duration,
                                DrivetrainType drivetrain, bool yaw_is_rate,
                                float yaw, int64_t command_start_time_nanos) override;
  // controller configs
  void SetControllerGains(uint8_t controllerType, const std::vector<float>& kp,
                          const std::vector<float>& ki,
                          const std::vector<float>& kd) override;

  /************************* State APIs *********************************/
  GeoPoint GetGpsLocationEstimated() const override;
  const MultirotorApiParams& GetMultirotorApiParams() const override;

  /************************* Basic Config APIs **************************/
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
  Vector3 GetAngles() const override;
  Vector3 GetPosition() const override;
  Vector3 GetVelocity() const override;
  Quaternion GetOrientation() const override;

 protected:
  void OnTopicRCInput(const Topic& topic, const Message& message);

 private:
  // The kind of target vehicle
  enum class VehicleKind {
    Multirotor,      // Multirotor helicopter (aka., multicopter)
    VTOLTailsitter,  // VTOL tail-sitting fixed-wing vehicle with multicopter
                     // and fixed-wing modes
    VTOLTiltrotor,   // VTOL tilt-rotor fixed-wing vehicle with multicopter
                     // and fixed-wing modes
  };                 // enum class VehicleKind

 private:
  void LoadSettings(const Robot& robot);

 private:
  std::unordered_map<std::string, float> params_map_;
  std::unordered_map<std::string, int> actuator_id_to_output_idx_map_;
  std::unique_ptr<AirSimSimpleFlightBoard> board_;
  std::unique_ptr<AirSimSimpleFlightCommLink> comm_link_;
  std::unique_ptr<AirSimSimpleFlightEstimator> estimator_;
  std::unique_ptr<AirSimSimpleFlightEstimatorFW> estimator_fw_;
  std::unique_ptr<simple_flight::IFirmware> firmware_;
  uint64_t millis_rc_input_last_update_ =
      0;                // Timestamp of when RC input was last received
  int num_motors_ = 4;  // Number of propulsion motors on the vehicle
  std::unique_ptr<simple_flight::Params>
      params_;  // todo params should become simple_flight_api_settings
  Topic rc_input_topic_;  // RC input topic
  MultirotorApiParams safety_params_;
  std::mutex update_lock_;
  VehicleKind vehicle_kind_ =
      VehicleKind::Multirotor;  // The type of vehicle being controlled
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_SIMPLE_FLIGHT_API_HPP_
