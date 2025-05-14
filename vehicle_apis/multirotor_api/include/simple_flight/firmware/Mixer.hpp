// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_MIXER_HPP_
#define MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_MIXER_HPP_

#include <algorithm>
#include <vector>

#include "Params.hpp"
#include "interfaces/CommonStructs.hpp"

namespace simple_flight {

  class Mixer {
   public:
    Mixer(const Params* params) : params_(params) {}

    virtual void GetMotorOutput(const AxisNr& controls,
                                std::vector<float>& motor_outputs) {
      const MixerMatrix* mixer_matrix = nullptr;
      auto num_motors = params_->motor.motor_count;

      switch (num_motors) {
        default:
          throw std::runtime_error(
              "Simpleflight mixer motor_count is not handled");
          break;

        case 4:
          mixer_matrix = &mixermatrixQuadX;
          break;

        case 6:
          mixer_matrix = &mixermatrixHexX;
          break;
      }

      if (mixer_matrix != nullptr)
        GetMotorOutputCore(controls, motor_outputs, num_motors, *mixer_matrix);
    }

 protected:
  // Mixer data base class
  class MixerMatrix {
   public:
    virtual const float* GetOutputMixer(int ioutput) const = 0;
    virtual size_t CInput(void) const = 0;
    virtual size_t COutput(void) const = 0;
    virtual size_t CRotor(void) const = 0;
  };  // class MixerMatrix

  // Mixer data for all actuators
  template <size_t cinput, size_t coutput, size_t coutput_rotors>
  class TMixerMatrix : public MixerMatrix {
   public:
    TMixerMatrix(std::initializer_list<float[cinput]> il) {
      int ioutput = 0;
      size_t coutput_copy = std::min(coutput, il.size());

      for (auto it = il.begin(), itEnd = il.begin() + coutput_copy; it < itEnd;
           ++it, ++ioutput)
        memcpy(mat_[ioutput], *it, cinput * sizeof(float));
    }

    virtual const float* GetOutputMixer(int ioutput) const override {
      return (mat_[ioutput]);
    }
    virtual size_t CInput(void) const override { return (cinput); }
    virtual size_t COutput(void) const override { return (coutput); }
    virtual size_t CRotor(void) const override { return (coutput_rotors); }

   protected:
    TMixerMatrix(void) : mat_{0} {}

   protected:
    float mat_[coutput][cinput] = {0};  // Mixer matrix
  };                                    // template class TMixerMatrix

  // Mixer that blends two other mixers
  template <size_t cinput, size_t coutput, size_t coutput_rotors>
  class TMixerMatrixBlended
      : public TMixerMatrix<cinput, coutput, coutput_rotors> {
   public:
    TMixerMatrixBlended(const MixerMatrix& mixermatrix) : prop2_(0.0f) {
      int ioutput = 0;

      for (float *proutput = &this->mat_[0][0],
                 *proutputMax = proutput + coutput * cinput;
           proutput < proutputMax; proutput += cinput, ++ioutput) {
        auto pr = mixermatrix.GetOutputMixer(ioutput);

        memcpy(proutput, pr, cinput * sizeof(float));
      }
    }

    void Set(const MixerMatrix& mixermatrix1, const MixerMatrix& mixermatrix2,
               float prop2) {
      int ioutput = 0;
      float prop1 = 1.0f - prop2;

      for (float *proutput = &this->mat_[0][0],
                 *proutputMax = proutput + coutput * cinput;
           proutput < proutputMax; proutput += cinput, ++ioutput) {
        auto prMixer1 = mixermatrix1.GetOutputMixer(ioutput);
        auto prMixer2 = mixermatrix2.GetOutputMixer(ioutput);

        for (float *pr = proutput, *prMax = proutput + cinput; pr < prMax; ++pr)
          *pr = *prMixer1++ * prop1 + *prMixer2++ * prop2;
      }

      prop2_ = prop2;
    }

    float GetBlendValue(void) { return (prop2_); }

   private:
    float prop2_;                 // Proportion of mixer 2 in mixer matrix
  };                              // template class TMixerMatrixBlended


   protected:
    void GetMotorOutputCore(const AxisNr& controls,
                            std::vector<float>& motor_outputs, int cmotor,
                            const MixerMatrix& mixer_matrix) const {
      size_t ccontrol = controls.AxisCount();
      size_t cinput_active = mixer_matrix.CInput();
      size_t coutput_active = mixer_matrix.COutput();
      int crotor = mixer_matrix.CRotor();

      // Limit inputs and outputs to minimum of number of matrix channels
      // and number of controls and motors
      if (ccontrol < cinput_active) cinput_active = ccontrol;
      if (cmotor < coutput_active) coutput_active = cmotor;

      // Calculate outputs by applying mixer matrix to inputs
      for (int ioutput = 0; ioutput < coutput_active; ++ioutput) {
        float output = 0.0f;
        const float* rgr_coefficient = mixer_matrix.GetOutputMixer(ioutput);

        for (int iinput = 0; iinput < cinput_active; ++iinput)
          output += controls[iinput] * rgr_coefficient[iinput];
        motor_outputs[ioutput] = output;
      }

      // Zero outputs not set by mixer matrix
      for (int ioutput = coutput_active; ioutput < cmotor;
           ++ioutput)
        motor_outputs[ioutput] = 0.0f;

      // If any rotor output undershoots the minimum motor output value,
      // offset all rotor outputs uniformly so that the smallest output
      // is at the zero.
      {
        float output_min = *std::min_element(motor_outputs.begin(),
                                             motor_outputs.begin() + crotor);
        if (output_min < params_->motor.min_motor_output) {
          float undershoot = params_->motor.min_motor_output - output_min;
          for (int irotor = 0; irotor < crotor; ++irotor)
            motor_outputs[irotor] += undershoot;
        }
      }

      // If any rotor output exceeds the maximum motor output value,
      // scale all rotor outputs uniformly so that the largest output
      // is at the maximum.
      {
        float output_max = *std::max_element(
            motor_outputs.begin(), motor_outputs.begin() + crotor);
        float scale = output_max / params_->motor.max_motor_output;

        if (scale > params_->motor.max_motor_output) {
          for (int irotor = 0; irotor < crotor; ++irotor)
            motor_outputs[irotor] /= scale;
        }
      }

      // Clip all non-rotor outputs to minimum and maximum allowed
      for (int imotor = cmotor; imotor < cmotor; ++imotor) {
        motor_outputs[imotor] = std::max(
            params_->motor.min_control_output,
            std::min(motor_outputs[imotor], params_->motor.max_control_output));
      }
    }

 protected:
  // only thing that this matrix does is change the sign
  const TMixerMatrix<4, 4, 4> mixermatrixQuadX {
      // QuadX config
      // Roll, Pitch, Yaw, Throttle
      {-1.0f,  1.0f,  1.0f, 1.0f},  // FRONT_R
      { 1.0f, -1.0f,  1.0f, 1.0f},  // REAR_L
      { 1.0f,  1.0f, -1.0f, 1.0f},  // FRONT_L
      {-1.0f, -1.0f, -1.0f, 1.0f},  // REAR_R
  };
  // Hexarotor X Setup
  //     x-axis
  //   (0)    (1)
  //     \  /
  //      \/
  // (5)-------(2) y-axis
  //      /\
  //     /  \
  //   (4)  (3)
  const TMixerMatrix<4, 6, 6> mixermatrixHexX {
      // HexX config
      // Roll, Pitch, Yaw, Throttle
      { 0.5f,  1.0f, -1.0f, 1.0f},  // 0
      {-0.5f,  1.0f,  1.0f, 1.0f},  // 1
      {-1.0f,  0.0f, -1.0f, 1.0f},  // 2
      {-0.5f, -1.0f,  1.0f, 1.0f},  // 3
      { 0.5f, -1.0f, -1.0f, 1.0f},  // 4
      { 1.0f,  0.0f,  1.0f, 1.0f},  // 5
  };

 protected:
  const Params* params_;  // Mixer parameters
};                        // class class Mixer

// Output mixer for VTOL controllers
class MixerVTOL : public Mixer {
 public:
  MixerVTOL(const Params* params) : Mixer(params) {}

    virtual void GetMotorOutput(
        const AxisNr& controls,
        std::vector<float>& motor_outputs) override {
      int num_motors = params_->motor.motor_count;

      switch (params_->controller_type) {
        default:
          Mixer::GetMotorOutput(controls, motor_outputs);
          break;

        case Params::ControllerType::kVFWTCascade:
          GetMotorOutputCore(controls, motor_outputs, num_motors,
                             mixermatrixVTOLFWTailsitter);
          break;

        case Params::ControllerType::kVTRCascade:
          if (controls.AxisCount() < 4)
              GetMotorOutputCore(controls, motor_outputs, num_motors,
                  mixermatrixVTOLFWTiltrotorMC);
          else {
            auto tilt = std::clamp(controls[4], 0.0f, 1.0f);
            auto dblend =
                fabs(mixermatrixVTOLFWTiltrotor.GetBlendValue() - tilt);

            if ((fabs(mixermatrixVTOLFWTiltrotor.GetBlendValue() - tilt) >
                 0.01) ||
                ((mixermatrixVTOLFWTiltrotor.GetBlendValue() != tilt) &&
                 (microsoft::projectairsim::MathUtils::IsApproximatelyZero(
                      tilt) ||
                  microsoft::projectairsim::MathUtils::IsApproximatelyEqual(
                      tilt, 1.0f))))
              mixermatrixVTOLFWTiltrotor.Set(mixermatrixVTOLFWTiltrotorMC,
                                             mixermatrixVTOLFWTiltrotorFW,
                                             tilt);
            GetMotorOutputCore(controls, motor_outputs, num_motors,
                               mixermatrixVTOLFWTiltrotor);
          }
          break;
      }
    }

   private:
    // VTOL Fixed-Wing Tailsitter Setup (looking down along +Z-axis)
    //      x-axis
    //    (2)    (0)
    //     |      |
    //    ---------   y-axis
    //     (5)  (6) elevons
    //     |      |
    //    (1)    (3)
    //
    // Slot 4 is unused to match PX4's actuator assignment
    const TMixerMatrix<4, 7, 4> mixermatrixVTOLFWTailsitter {
        // VTOL Fix-wing Tailsitter config in fixed-wing mode where
        // x-axis is down and z-axis is backward so the elevons affect
        // y-axis pitch and z-axis yaw (not x-axis roll as you might think at
        // first glance.)  Rotor rotation is when viewed from the front of rotor
        // with the motor behind it.
        // Roll, Pitch, Yaw, Throttle
        {-1.0f,  1.0f,  1.0f, 1.0f},  // 0 rotor front right (rotates CCW)
        { 1.0f, -1.0f,  1.0f, 1.0f},  // 1 rotor rear left (rotates CCW)
        { 1.0f,  1.0f, -1.0f, 1.0f},  // 2 rotor front left (rotates CW)
        {-1.0f, -1.0f, -1.0f, 1.0f},  // 3 rotor rear right (rotates CW)
        { 0.0f,  0.0f,  0.0f, 0.0f},  // 4 unused actuator slot
        { 0.0f, -0.5f,  0.5f, 0.0f},  // 5 elevon left
        { 0.0f, -0.5f, -0.5f, 0.0f},  // 6 elevon right
    };                                // class MixerVTOL

    // VTOL Tiltrotor Setup (looking down along +Z-axis)
    //          x-axis
    //     (2)         (0)  rotors
    //    -----------------   y-axis
    //        (5) | (6)     ailerons
    //     (1)    |    (3)  rotors
    //            |
    //          --|-- (10) elevator
    //           (11) rudder
    // This mapping roughly matches PX4's Generic Quadplane VTOL Tiltrotor
    const TMixerMatrix<5, 12, 4> mixermatrixVTOLFWTiltrotorMC {
        // VTOL tiltrotor config in multicopter mode
        // Rotor rotation is when viewed from the front of rotor with the motor
        // behind it.
        //
        // Roll, Pitch, Yaw, Throttle, Tilt
        {-0.5f,  0.5f,  0.5f, 1.0f, 0.0f},   // 0 rotor front right (rotates CCW)
        { 0.5f, -0.5f,  0.5f, 1.0f, 0.0f},   // 1 rotor rear left (rotates CCW)
        { 0.5f,  0.5f, -0.5f, 1.0f, 0.0f},   // 2 rotor front left (rotates CW)
        {-0.5f, -0.5f, -0.5f, 1.0f, 0.0f},   // 3 rotor rear right (rotates CW)
        { 0.0f,  0.0f,  0.2f, 0.0f, 1.0f},   // 4 tilt front right
        { 0.0f,  0.0f,  0.2f, 0.0f, 1.0f},   // 5 tilt rear left
        { 0.0f,  0.0f, -0.2f, 0.0f, 1.0f},   // 6 tilt front left
        { 0.0f,  0.0f, -0.2f, 0.0f, 1.0f},   // 7 tilt rear right
        { 0.0f,  0.0f,  0.0f, 0.0f, 0.0f},   // 8 aileron left
        { 0.0f,  0.0f,  0.0f, 0.0f, 0.0f},   // 9 aileron right
        { 0.0f,  1.0f,  0.0f, 0.0f, 0.0f},   // 10 elevator
        { 0.0f,  0.0f,  0.0f, 0.0f, 0.0f},   // 11 rudder
    };
    const TMixerMatrix<5, 12, 4> mixermatrixVTOLFWTiltrotorFW {
        // VTOL tiltrotor config in fixed-wing mode
        // Rotor rotation is when viewed from the front of rotor with the motor
        // behind it.
        //
        // Roll, Pitch, Yaw, Throttle, Tilt
        { 0.0f,  0.0f, -0.5f, 1.0f, 0.0f},   // 0 rotor front right (rotates CCW)
        { 0.0f,  0.0f,  0.5f, 1.0f, 0.0f},   // 1 rotor rear left (rotates CCW)
        { 0.0f,  0.0f,  0.5f, 1.0f, 0.0f},   // 2 rotor front left (rotates CW)
        { 0.0f,  0.0f, -0.5f, 1.0f, 0.0f},   // 3 rotor rear right (rotates CW)
        { 0.0f,  0.0f,  0.0f, 0.0f, 1.0f},   // 4 tilt front right
        { 0.0f,  0.0f,  0.0f, 0.0f, 1.0f},   // 5 tilt rear left
        { 0.0f,  0.0f,  0.0f, 0.0f, 1.0f},   // 6 tilt front left
        { 0.0f,  0.0f,  0.0f, 0.0f, 1.0f},   // 7 tilt rear right
        {-1.0f,  0.0f,  0.0f, 0.0f, 0.0f},   // 8 aileron left
        { 1.0f,  0.0f,  0.0f, 0.0f, 0.0f},   // 9 aileron right
        { 0.0f,  1.0f,  0.0f, 0.0f, 0.0f},   // 10 elevator
        { 0.0f,  0.0f, -1.0f, 0.0f, 0.0f},   // 11 rudder
    };

    // Cached mixer for VTOL fixed-wing VTOL at a particular tilt value
    TMixerMatrixBlended<5, 12, 4> mixermatrixVTOLFWTiltrotor = mixermatrixVTOLFWTiltrotorMC;
};  // class MixerVTOL
}  // namespace simple_flight

#endif  // MULTIROTOR_API_SIMPLE_FLIGHT_FIRMWARE_MIXER_HPP_