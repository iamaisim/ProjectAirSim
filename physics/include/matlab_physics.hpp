// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef PHYSICS_INCLUDE_MATLAB_PHYSICS_HPP_
#define PHYSICS_INCLUDE_MATLAB_PHYSICS_HPP_

#include "base_physics.hpp"
#include "core_sim/actor/robot.hpp"
#include "nng/nng.h"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class MatlabPhysicsBody

class MatlabPhysicsBody : public BasePhysicsBody {
 public:
  MatlabPhysicsBody() {}
  explicit MatlabPhysicsBody(const Robot& robot);
  ~MatlabPhysicsBody() override {}

  void InitializeMatlabPhysicsBody();

  // Aggregate all externally applied wrenches on body CG into wrench_
  void CalculateExternalWrench() override;

  void ReadRobotData();
  void WriteRobotData(const Kinematics& kinematics);

  // These conversion operators allow this object to be passed directly to
  // TransformTree methods
  operator TransformTree::RefFrame&(void);
  operator const TransformTree::RefFrame&(void) const;

 protected:
  friend class MatlabPhysicsModel;

 protected:
  // Entry for each external wrench
  struct ExternalWrenchEntry {
    const TransformTree::RefFrame*
        refframe;  // External wrench point's reference frame
    const WrenchPoint*
        wrench_point;  // External wrench point, relative to refframe

    ExternalWrenchEntry(const WrenchPoint* wrench_point_in,
                        const TransformTree::RefFrame* refframe_in)
        : refframe(refframe_in), wrench_point(wrench_point_in) {}
  };  // struct ExternalWrenchEntry

 protected:
  Robot sim_robot_;
  bool connected_ = false;
  nng_socket nng_socket_ = NNG_SOCKET_INITIALIZER;
  std::string connection_string_;
  std::vector<ExternalWrenchEntry> external_wrench_entries_;
  Environment::EnvInfo environment_info_;
  CollisionInfo collision_info_;
  std::vector<float> lift_drag_control_angles_;
};

// -----------------------------------------------------------------------------
// class MatlabPhysicsModel

class MatlabPhysicsModel {
 public:
  MatlabPhysicsModel();
  ~MatlabPhysicsModel();

  void Start(std::shared_ptr<BasePhysicsBody> body);

  void SetWrenchesOnPhysicsBody(std::shared_ptr<BasePhysicsBody> body);

  void StepPhysicsBody(TimeNano dt_nanos,
                       std::shared_ptr<BasePhysicsBody> body);

  void Stop(std::shared_ptr<BasePhysicsBody> body);

  void ConnectToMatlab(std::shared_ptr<MatlabPhysicsBody> matlab_body);

 private:
  std::atomic<bool> running_ = false;
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // PHYSICS_INCLUDE_MATLAB_PHYSICS_HPP_
