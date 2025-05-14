// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_FIRMWARE_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_FIRMWARE_HPP_

#ifdef LVMON_REPORTING
#include <LVMon/lvmon.h>
#endif  // LVMON_REPORTING

#include <vector>

#include "Mixer.hpp"
#include "OffboardApi.hpp"
#include "Params.hpp"
#include "RemoteControl.hpp"
#include "interfaces/CommonStructs.hpp"
#include "interfaces/IBoard.hpp"
#include "interfaces/ICommLink.hpp"
#include "interfaces/IFirmware.hpp"
#include "interfaces/IStateEstimator.hpp"
#include "multirotor/CascadeController.hpp"
#include "vtolfixedwing/VFTCascadeController.hpp"
#include "vtoltiltrotor/VTRCascadeController.hpp"

namespace simple_flight {

class Firmware : public IFirmware {
 public:
  Firmware(
      Params* params, IBoard* board, ICommLink* comm_link,
      IStateEstimator* state_estimator,
      microsoft::projectairsim::AirSimSimpleFlightEstimatorFW* state_estimator_fw)
      : params_(params),
        board_(board),
        comm_link_(comm_link),
        state_estimator_(state_estimator),
        offboard_api_(params, board, board, state_estimator, state_estimator_fw,
                      comm_link),
        mixer_(params) {
    switch (params->controller_type) {
      case Params::ControllerType::kCascade:
        controller_ = std::unique_ptr<CascadeController>(
            new CascadeController(params, board, comm_link));
        break;
      case Params::ControllerType::kVFWTCascade:
        controller_ =
            std::unique_ptr<VFTCascadeController>(new VFTCascadeController(
                params, board, comm_link, state_estimator_fw));
        break;
      case Params::ControllerType::kVTRCascade:
        controller_ =
            std::unique_ptr<VTRCascadeController>(new VTRCascadeController(
                params, board, comm_link, state_estimator_fw));
        break;
      default:
        throw std::invalid_argument(
            "Cannot recognize controller specified by params->controller_type");
    }

    controller_->Initialize(&offboard_api_, state_estimator_);
  }

  virtual void Reset() override {
    IFirmware::Reset();

    board_->Reset();
    comm_link_->Reset();
    controller_->Reset();
    offboard_api_.Reset();

    motor_outputs_.assign(params_->motor.motor_count, 0);
  }

  virtual void Update() override {
    IFirmware::Update();
    board_->Update();
    offboard_api_.Update();
    controller_->Update();

    const AxisNr& output_controls = controller_->GetOutput();

    // convert controller output in to motor outputs
    mixer_.GetMotorOutput(output_controls, motor_outputs_);

    // finally write the motor outputs
    for (uint16_t motor_index = 0; motor_index < params_->motor.motor_count;
         ++motor_index)
      board_->WriteOutput(motor_index, motor_outputs_.at(motor_index));

    comm_link_->Update();

#ifdef LVMON_REPORTING
    {
      static const char* c_mpimotorsz[] = {
          "firmware/motor/0", "firmware/motor/1", "firmware/motor/2",
          "firmware/motor/3", "firmware/motor/4", "firmware/motor/5",
      };

      int cmotor = params_->motor.motor_count;
      const char** psz = c_mpimotorsz;

      if (cmotor > _countof(c_mpimotorsz)) cmotor = _countof(c_mpimotorsz);

      for (int imotor = 0; imotor < cmotor; ++imotor)
        LVMon::Set(*psz++, motor_outputs_.at(imotor));
    }
#endif  // LVMON_REPORTING
  }

  virtual IOffboardApi& OffboardApi() override { return offboard_api_; }

 private:
  // objects we use
  Params* params_;
  IBoard* board_;
  ICommLink* comm_link_;
  IStateEstimator* state_estimator_;

  simple_flight::OffboardApi offboard_api_;
  MixerVTOL mixer_;
  std::unique_ptr<IController> controller_;

  std::vector<float> motor_outputs_;
};

}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_FIRMWARE_HPP_