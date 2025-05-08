// Copyright (C) Microsoft Corporation. All rights reserved.

#include "fast_physics.hpp"

#include <iostream>
#include <memory>
#include <vector>

#include "core_sim/actuators/actuator.hpp"
#include "core_sim/actuators/lift_drag_control_surface.hpp"
#include "core_sim/actuators/rotor.hpp"
#include "core_sim/actuators/wheel.hpp"
#include "core_sim/earth_utils.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/physics_common_utils.hpp"
#include "core_sim/transforms/transform_utils.hpp"

namespace microsoft {
namespace projectairsim {

// -----------------------------------------------------------------------------
// class FastPhysicsBody

FastPhysicsBody::FastPhysicsBody(const Robot& robot) : sim_robot_(robot) {
  SetName(robot.GetID());
  SetPhysicsType(PhysicsType::kFastPhysics);
  InitializeFastPhysicsBody();
}

void FastPhysicsBody::InitializeFastPhysicsBody() {
  // Get robot physics-related data
  auto& actuators = sim_robot_.GetActuators();
  const auto& links = sim_robot_.GetLinks();
  const auto& root_link = links.at(0);  // TODO allow config to choose root link

  // Process robot's actuators to store pointers to their wrench points and
  // lift-drag control angles
  external_wrench_entries_.clear();
  lift_drag_control_angles_.clear();
  for (auto& actuator_ref : actuators) {
    Actuator& actuator = actuator_ref.get();

    if (actuator.GetType() == ActuatorType::kRotor) {
      auto& rotor_ref = static_cast<Rotor&>(actuator);

      external_wrench_entries_.emplace_back(
          &rotor_ref.GetWrenchPoint(),
          &static_cast<TransformTree::RefFrame&>(rotor_ref));
    } else if (actuator.GetType() == ActuatorType::kWheel) {
      auto& wheel_ref = static_cast<Wheel&>(actuator);

      external_wrench_entries_.emplace_back(
          &wheel_ref.GetWrenchPoint(),
          &static_cast<TransformTree::RefFrame&>(wheel_ref));

    } else if (actuator.GetType() == ActuatorType::kLiftDragControlSurface) {
      // Populate lift_drag_control_angles_ map as:
      //   key = control surface actuator's parent link (wing it affects)
      //   value = pointer to control surface actuator's control angle
      const auto& control_surface_ref =
          static_cast<const LiftDragControlSurface&>(actuator);

      lift_drag_control_angles_.emplace(control_surface_ref.GetParentLink(),
                                        &control_surface_ref.GetControlAngle());
    }
  }

  SetMass(root_link.GetInertial().GetMass());  // TODO accumulate link masses?

  auto inertia = CalculateInertia(links);
  SetInertia(inertia);

  drag_faces_ = InitializeDragFaces(links);
  lift_drag_links_ = InitializeLiftDragLinks(links);

  friction_ = root_link.GetCollision().GetFriction();
  restitution_ = root_link.GetCollision().GetRestitution();
  is_grounded_ = sim_robot_.GetStartLanded();

  // if there are wheel actutors then get the rover's length
  auto wheels = sim_robot_.GetWheels();
  if (!wheels.empty()) {
    // get first wheel
    auto& wheel_ref = static_cast<Wheel&>(*wheels[0]);
    // get second wheel
    auto& wheel_ref2 = static_cast<Wheel&>(*wheels[1]);
    // get the distance between the two wheels
    rover_length_ =
        wheel_ref.GetWheelSettings().origin_setting.translation_.x() -
        wheel_ref2.GetWheelSettings().origin_setting.translation_.x();
    if (rover_length_ == 0) {
      // get distance using the third wheel
      auto& wheel_ref3 = static_cast<Wheel&>(*wheels[2]);
      rover_length_ =
          wheel_ref.GetWheelSettings().origin_setting.translation_.x() -
          wheel_ref3.GetWheelSettings().origin_setting.translation_.x();
    }
  }
}

// TODO Rename this to "SetActuationOutputs"?
void FastPhysicsBody::CalculateExternalWrench() {
  // 1. Process external wrenches from actuators (like rotors)

  // Aggregate external wrenches into the total external_wrench_ on body
  Wrench aggregate_wrench = Wrench::Zero();
  for (auto& wrench_entry : external_wrench_entries_) {
    auto wrench_pt = *wrench_entry.wrench_point;

    // Make wrench relative to physics body
    {
      Pose pose;
      auto transform_tree = wrench_entry.refframe->GetTransformTree();

      pose.position = wrench_pt.position;
      transform_tree->Convert(pose, *wrench_entry.refframe, *this, &pose);

      wrench_pt.position = pose.position;
      wrench_pt.wrench.torque =
          pose.orientation._transformVector(wrench_pt.wrench.torque);
      wrench_pt.wrench.force =
          pose.orientation._transformVector(wrench_pt.wrench.force);
    }

    // Accumulate wrench itself
    aggregate_wrench += wrench_pt.wrench;

    // Add additional torque from force applied at radius to CG, tau = r X F
    aggregate_wrench.torque += wrench_pt.position.cross(wrench_pt.wrench.force);
  }

  // Transform total external wrench force to world frame (forces/linear
  // kinematics are in world frame, moments/angular kinematics are in body
  // frame).
  aggregate_wrench.force = PhysicsUtils::TransformVectorToWorldFrame(
      aggregate_wrench.force, kinematics_.pose.orientation);

  external_wrench_ = aggregate_wrench;

  // Add external force wrench in world frame
  external_wrench_ += Wrench(ext_force_, Vector3::Zero());

  // 2. Process lift-drag control surface actuator control angles

  // No aggregation or processing is needed for lift_drag_control_angles_ since
  // they are just direct references to the lift-drag control surface actuators'
  // control angle output variables.
}

// Assume root link (index = 0) is the CG and use parallel axis theorem
// (I = Icm + m * D^2) to aggregate other link inertias to the root's CG.
// https://en.wikipedia.org/wiki/Parallel_axis_theorem
Matrix3x3 FastPhysicsBody::CalculateInertia(const std::vector<Link>& links) {
  // Start with root link as base inertia
  Matrix3x3 inertia = links[0].GetInertial().GetInertia();

  // Aggregate each other link's inertia using parallel axis theorem
  for (int i = 1; i < links.size(); ++i) {
    auto& link = links[i];
    const auto& link_inertia = link.GetInertial().GetInertia();
    float link_mass = link.GetInertial().GetMass();

    // Calculate the position of the link relative to the body
    Pose pose;
    auto& refframe = static_cast<const TransformTree::RefFrame&>(link);

    refframe.GetTransformTree()->Convert(pose, refframe, *this, &pose);
    auto& link_pos = pose.position;

    float inertia_dx =
        link_inertia(0, 0) +
        link_mass * (link_pos.y() * link_pos.y() + link_pos.z() * link_pos.z());

    float inertia_dy =
        link_inertia(1, 1) +
        link_mass * (link_pos.x() * link_pos.x() + link_pos.z() * link_pos.z());

    float inertia_dz =
        link_inertia(2, 2) +
        link_mass * (link_pos.x() * link_pos.x() + link_pos.y() * link_pos.y());

    inertia(0, 0) += inertia_dx;
    inertia(1, 1) += inertia_dy;
    inertia(2, 2) += inertia_dz;
  }

  return inertia;
}

// FastPhysicsBody::InitializeDragFaces() assumes that the body is like a drone
// with a central cuboid body (frame) with params set in the root link (index =
// 0) that is connected to actuators (propellers) with params set in the other
// links that are treated as cylinders with normal in the z-axis. The drag is
// the combination of the body cuboid and each actuator cylinder for each of the
// top/bottom (z axis), left/right (x axis), and front/back faces (y axis).
// These assumption can't handle actuators that are rotated at other tilts
// relative to the body z axis. It also currently assumes NED coordinates.
std::vector<DragFace> FastPhysicsBody::InitializeDragFaces(
    const std::vector<Link>& links) {
  // Aggregate cross sectional areas for all links
  Vector3 cross_section_areas = Vector3::Zero();
  for (const auto& link : links) {
    cross_section_areas += link.GetInertial().GetCrossSectionAreas();
  }

  // Drag = 1/2 * Cd * A * air_density * V^2, drag_factor = (1/2 * Cd)
  float drag_factor = links[0].GetInertial().GetDragCoefficient() / 2;
  const Vector3& body_box = links[0].GetInertial().GetBodyBox();

  // Add six drag faces representing 6 sides of the cuboid.
  // Each DragFace is constructed as (position, normal, area, drag_factor)

  // TODO This assumes NED coordinates with negative Z direction
  std::vector<DragFace> drag_faces;
  // Top face
  drag_faces.emplace_back(Vector3(0, 0, -body_box.z() / 2), Vector3(0, 0, -1),
                          cross_section_areas.z(), drag_factor);

  // Bottom face
  drag_faces.emplace_back(Vector3(0, 0, body_box.z() / 2), Vector3(0, 0, 1),
                          cross_section_areas.z(), drag_factor);

  // Left face
  drag_faces.emplace_back(Vector3(0, -body_box.y() / 2, 0), Vector3(0, -1, 0),
                          cross_section_areas.y(), drag_factor);

  // Right face
  drag_faces.emplace_back(Vector3(0, body_box.y() / 2, 0), Vector3(0, 1, 0),
                          cross_section_areas.y(), drag_factor);

  // Back face
  drag_faces.emplace_back(Vector3(-body_box.x() / 2, 0, 0), Vector3(-1, 0, 0),
                          cross_section_areas.x(), drag_factor);

  // Front face
  drag_faces.emplace_back(Vector3(body_box.x() / 2, 0, 0), Vector3(1, 0, 0),
                          cross_section_areas.x(), drag_factor);

  return drag_faces;
}

std::vector<std::reference_wrapper<const Link>>
FastPhysicsBody::InitializeLiftDragLinks(const std::vector<Link>& links) {
  std::vector<std::reference_wrapper<const Link>> lift_drag_links;
  for (const Link& link : links) {
    if (link.GetInertial().GetLiftDrag().enabled) {
      lift_drag_links.push_back(link);
    }
  }
  return lift_drag_links;
}

void FastPhysicsBody::ReadRobotData() {
  kinematics_ = sim_robot_.GetKinematics();
  collision_info_ = sim_robot_.GetCollisionInfo();
  ext_force_ = sim_robot_.GetExternalForce();
  const auto& cur_env = sim_robot_.GetEnvironment();
  const auto& cur_env_info = cur_env.env_info;
  env_gravity_ = cur_env_info.gravity;
  env_air_density_ = cur_env_info.air_density;
  env_wind_velocity_ = cur_env.wind_velocity;
}

void FastPhysicsBody::WriteRobotData(const Kinematics& kinematics,
                                     TimeNano external_time_stamp) {
  sim_robot_.UpdateKinematics(kinematics, external_time_stamp);
}

bool FastPhysicsBody::IsStillGrounded() {
  // Handle special case of already grounded by sticking to the ground until
  // we see wrench force greater than gravity
  const float normalized_force = external_wrench_.force.squaredNorm();
  const Vector3 weight = mass_ * env_gravity_;
  const float normalized_weight = weight.squaredNorm();

  if (is_grounded_ && normalized_force < normalized_weight &&
      kinematics_.twist.linear.z() >= 0) {
    is_grounded_ = true;
  } else {
    is_grounded_ = false;
  }

  return is_grounded_;
}

bool FastPhysicsBody::IsLandingCollision() {
  if (!collision_info_.has_collided) return false;

  // Check if collision normal is primarily vertically upward (assumes NED)
  const Vector3 normal_body = PhysicsUtils::TransformVectorToBodyFrame(
      collision_info_.normal, kinematics_.pose.orientation);

  const bool is_ground_normal = MathUtils::IsApproximatelyEqual(
      normal_body.z(), -1.0f, kGroundCollisionAxisTol);

  // Check if velocity is primarily vertically downward (assumes NED)
  const float vel_x_abs = fabs(kinematics_.twist.linear.x());
  const float vel_y_abs = fabs(kinematics_.twist.linear.y());
  const float vel_z = kinematics_.twist.linear.z();

  const bool is_moving_downward = (vel_z > vel_x_abs) && (vel_z > vel_y_abs);

  if (is_ground_normal && is_moving_downward) {
    is_grounded_ = true;
    return true;
  } else {
    return false;
  }
}

bool FastPhysicsBody::NeedsCollisionResponse(const Kinematics& next_kin) {
  const bool is_moving_into_collision =
      (collision_info_.normal.dot(next_kin.twist.linear) < 0.0f);

  if (collision_info_.has_collided && is_moving_into_collision) {
    return true;
  } else {
    return false;
  }
}

FastPhysicsBody::operator TransformTree::RefFrame&(void) {
  return (const_cast<TransformTree::RefFrame&>(
      operator const TransformTree::RefFrame&()));
}

FastPhysicsBody::operator const TransformTree::RefFrame&(void) const {
  // If the physics body isn't loaded (units tests), return the global frame
  return (sim_robot_.IsLoaded()
              ? static_cast<const TransformTree::RefFrame&>(sim_robot_)
              : TransformTree::kRefFrameGlobal);
}

// -----------------------------------------------------------------------------
// class FastPhysicsModel

void FastPhysicsModel::SetWrenchesOnPhysicsBody(
    std::shared_ptr<BasePhysicsBody> body) {
  // Dynamic cast to a FastPhysicsBody
  std::shared_ptr<FastPhysicsBody> fp_body =
      std::dynamic_pointer_cast<FastPhysicsBody>(body);
  if (fp_body != nullptr) {
    fp_body->CalculateExternalWrench();
  }
}

void FastPhysicsModel::StepPhysicsBody(TimeNano dt_nanos,
                                       std::shared_ptr<BasePhysicsBody> body) {
  // Dynamic cast to a FastPhysicsBody
  std::shared_ptr<FastPhysicsBody> fp_body =
      std::dynamic_pointer_cast<FastPhysicsBody>(body);

  if (fp_body != nullptr) {
    auto dt_sec = SimClock::Get()->NanosToSec(dt_nanos);
    Kinematics next_kin;

    // Step 1 - Update physics body's data from robot's latest data
    fp_body->ReadRobotData();

    // Step 2 - Calculate kinematics without collisions
    if (fp_body->IsStillGrounded()) {
      if (fp_body->sim_robot_.HasWheels())
        next_kin = CalcNextKinematicsWithWheels(
            dt_sec, fp_body, fp_body->kinematics_.pose.position);
      else
        next_kin =
            CalcNextKinematicsGrounded(fp_body->kinematics_.pose.position,
                                       fp_body->kinematics_.pose.orientation);
    } else {
      next_kin = CalcNextKinematicsNoCollision(dt_sec, fp_body);
    }

    // Step 3 - Calculate kinematics with collision response if needed
    if (fp_body->NeedsCollisionResponse(next_kin)) {
      if (fp_body->IsLandingCollision()) {
        // Calculate the landed position to stick at by offsetting back
        // along the collision normal with an additional kCollisionOffset margin
        // to prevent getting stuck in a collided state.
        const Vector3 landed_position =
            fp_body->collision_info_.position +
            fp_body->collision_info_.normal *
                (fp_body->collision_info_.penetration_depth +
                 FastPhysicsBody::kCollisionOffset);

        if (fp_body->sim_robot_.HasWheels()) {
          next_kin =
              CalcNextKinematicsWithWheels(dt_sec, fp_body, landed_position);
        } else
          next_kin = CalcNextKinematicsGrounded(
              landed_position, fp_body->kinematics_.pose.orientation);
      } else {
        next_kin = CalcNextKinematicsWithCollision(dt_sec, fp_body);
      }
    }

    // Step 4 - Save new kinematics to body's referenced sim robot
    fp_body->WriteRobotData(next_kin);
  }
}

Kinematics FastPhysicsModel::CalcNextKinematicsGrounded(
    const Vector3& position, const Quaternion& orientation) {
  Kinematics kin_grounded;
  // Zero out accels/velocities
  kin_grounded.accels.linear = Vector3::Zero();
  kin_grounded.accels.angular = Vector3::Zero();
  kin_grounded.twist.linear = Vector3::Zero();
  kin_grounded.twist.angular = Vector3::Zero();

  // Pass-through position
  kin_grounded.pose.position = position;

  // Zero out roll/pitch, pass-through yaw
  auto rpy = TransformUtils::ToRPY(orientation);
  kin_grounded.pose.orientation = TransformUtils::ToQuaternion(0., 0., rpy[2]);

  return kin_grounded;
}

Kinematics FastPhysicsModel::CalcNextKinematicsWithWheels(
    TimeSec dt_sec, std::shared_ptr<FastPhysicsBody>& fp_body,
    const Vector3& landed_position) {
  auto wheels = fp_body->sim_robot_.GetWheels();
  float rover_speed = 0;
  float rover_steering_speed = 0;
  float rover_steering = 0;
  // get first wheel connected to the engine and get the rotating speed as the
  // rover's speed
  for (auto wheel : wheels) {
    if (wheel->IsEngineConnected()) {
      auto rover_rotating_speed = wheel->GetRotatingSpeed();
      rover_speed = wheel->GetRadius() * rover_rotating_speed;
      break;
    }
  }

  // get first wheel connected to the steering and get the steering and steering
  // speed as the ones for the rover
  for (auto wheel : wheels) {
    if (wheel->IsSteeringConnected()) {
      rover_steering_speed = wheel->GetSteeringSpeed();
      rover_steering = wheel->GetSteering();
      break;
    }
  }
  Kinematics kin = fp_body->GetKinematics();
  Quaternion orientation = kin.pose.orientation;
  // get orientation yaw
  auto rpy = TransformUtils::ToRPY(orientation);
  float radians_yaw = rpy[2];
  auto dradians_yaw =
      atan2(rover_speed * sin(rover_steering) * dt_sec, fp_body->rover_length_);

  // set kin.pose.orientation with (0,0,yaw)
  kin.pose.orientation =
      TransformUtils::ToQuaternion(0., 0., radians_yaw + dradians_yaw);

  // update x position and y position
  auto dm_per_sec_x = rover_speed * cos(radians_yaw);
  auto dm_per_sec_y = rover_speed * sin(radians_yaw);
  kin.pose.position.x() += dm_per_sec_x * dt_sec;
  kin.pose.position.y() += dm_per_sec_y * dt_sec;
  kin.pose.position.z() = landed_position.z();

  // Update velocities
  kin.twist.angular.z() = dradians_yaw / dt_sec;

  return kin;
}

Kinematics FastPhysicsModel::CalcNextKinematicsNoCollision(
    TimeSec dt_sec, std::shared_ptr<FastPhysicsBody>& fp_body) {
  const auto& cur_kin = fp_body->kinematics_;
  const auto& external_wrench = fp_body->external_wrench_;
  const auto& drag_faces = fp_body->drag_faces_;
  const auto& lift_drag_links = fp_body->lift_drag_links_;
  const auto& lift_drag_control_angles = fp_body->lift_drag_control_angles_;
  const auto& mass_inv = fp_body->mass_inv_;
  const auto& inertia = fp_body->inertia_;
  const auto& inertia_inv = fp_body->inertia_inv_;
  const auto& env_air_density = fp_body->env_air_density_;
  const auto& env_gravity = fp_body->env_gravity_;
  const auto& env_wind_velocity = fp_body->env_wind_velocity_;

  Kinematics kin_no_collision;  // main output of this function

  // Step 1 - Calculate aerodynamic wrenches on body

  //   Step 1a - Calculate air flow velocity at each link
  //   Currently, wind is not supported so assume all air flow velocity comes
  //   from the body motion velocity.

  const Vector3 ave_vel_lin =
      cur_kin.twist.linear + cur_kin.accels.linear * (0.5f * dt_sec);

  const Vector3 ave_vel_ang =
      cur_kin.twist.angular + cur_kin.accels.angular * (0.5f * dt_sec);

  //   TODO Calculate air flow velocity at each link considering wind
  //   from the environment and possibly slipstream from rotor actuators
  //   (generalize as an array of external airflows or separate aerodynamic
  //   links from inertial links?).

  //   Step 1b - Calculate drag wrench from body's drag face cross-sectional
  //   areas and cur velocity.

  const Wrench drag_wrench = CalcDragFaceWrench(
      ave_vel_lin, ave_vel_ang, drag_faces, cur_kin.pose.orientation,
      env_air_density, env_wind_velocity);

  //   Step 1c - Calculate lift-drag wrenches

  const Wrench lift_drag_wrench = CalcLiftDragWrench(
      lift_drag_links, lift_drag_control_angles, cur_kin.pose.orientation,
      ave_vel_lin, env_air_density, env_wind_velocity);

  // Step 2 - Calculate total wrench on body

  const Wrench total_wrench = external_wrench + drag_wrench + lift_drag_wrench;

  // Step 3 - Calculate new linear and angular accels

  kin_no_collision.accels.linear = total_wrench.force * mass_inv + env_gravity;

  // Euler's rotation equation:
  // https://en.wikipedia.org/wiki/Euler%27s_equations_(rigid_body_dynamics)
  // Use torque to find out the angular accel angular momentum L = I * omega
  const Vector3 angular_momentum = inertia * ave_vel_ang;
  const Vector3 angular_momentum_rate =
      total_wrench.torque - ave_vel_ang.cross(angular_momentum);

  kin_no_collision.accels.angular = inertia_inv * angular_momentum_rate;

  // Step 4 - Calculate new linear and angular twist velocities by
  //          integrating accels

  // Verlet integration:
  // http://www.physics.udel.edu/~bnikolic/teaching/phys660/numerical_ode/node5.html
  kin_no_collision.twist.linear =
      cur_kin.twist.linear +
      (cur_kin.accels.linear + kin_no_collision.accels.linear) * 0.5f * dt_sec;

  kin_no_collision.twist.angular =
      cur_kin.twist.angular +
      (cur_kin.accels.angular + kin_no_collision.accels.angular) * 0.5f *
          dt_sec;

  // Clip by speed of light to prevent infinity/NaN:
  const auto c_light = EarthUtils::kSpeedOfLight;
  if (kin_no_collision.twist.linear.squaredNorm() > c_light * c_light) {
    kin_no_collision.twist.linear /=
        (kin_no_collision.twist.linear.norm() / c_light);

    kin_no_collision.accels.linear = Vector3::Zero();
  }

  if (kin_no_collision.twist.angular.squaredNorm() > c_light * c_light) {
    kin_no_collision.twist.angular /=
        (kin_no_collision.twist.angular.norm() / c_light);

    kin_no_collision.accels.angular = Vector3::Zero();
  }

  // Step 5 - Calculate new pose/orientation by integrating twist velocities
  // TODO Should this calc use current kinematics
  // kin_no_collision.twist.linear instead of ave_vel_lin from prev
  // kinematics?
  kin_no_collision.pose.position = cur_kin.pose.position + ave_vel_lin * dt_sec;

  // Use angular velocty in body frame to calculate angular displacement
  // over last dt seconds
  const auto angle_per_unit = ave_vel_ang.norm();
  if (MathUtils::IsDefinitelyGreaterThan(angle_per_unit, 0.0f)) {
    // Convert change in angle to unit quaternion
    const AngleAxis angle_dt_aa =
        AngleAxis(angle_per_unit * dt_sec, ave_vel_ang / angle_per_unit);
    const Quaternion angle_dt_q = Quaternion(angle_dt_aa);

    // Apply change in angle to previous orientation
    kin_no_collision.pose.orientation = cur_kin.pose.orientation * angle_dt_q;

    if (PhysicsUtils::HasNan(kin_no_collision.pose.position) ||
        PhysicsUtils::HasNan(kin_no_collision.pose.orientation) ||
        PhysicsUtils::HasNan(kin_no_collision.twist.linear) ||
        PhysicsUtils::HasNan(kin_no_collision.twist.angular) ||
        PhysicsUtils::HasNan(kin_no_collision.accels.linear) ||
        PhysicsUtils::HasNan(kin_no_collision.accels.angular)) {
      throw Error("NaN in kinematics.");
    }

    // Re-normalize quaternion to avoid accumulating error
    kin_no_collision.pose.orientation.normalize();
  } else {
    // No change in orientation because angular velocity is zero
    kin_no_collision.pose.orientation = cur_kin.pose.orientation;
  }

  return kin_no_collision;
}

Kinematics FastPhysicsModel::CalcNextKinematicsWithCollision(
    TimeSec dt_sec, std::shared_ptr<FastPhysicsBody>& fp_body) {
  const auto& collision_info = fp_body->collision_info_;
  const auto& cur_kin = fp_body->kinematics_;
  const auto& restitution = fp_body->restitution_;
  const auto& friction = fp_body->friction_;
  const auto& mass_inv = fp_body->mass_inv_;
  const auto& inertia_inv = fp_body->inertia_inv_;

  Kinematics kin_with_collision;  // main output of this function

  const Vector3 normal_body = PhysicsUtils::TransformVectorToBodyFrame(
      collision_info.normal, cur_kin.pose.orientation);

  const Vector3 ave_vel_lin =
      cur_kin.twist.linear + cur_kin.accels.linear * dt_sec;

  const Vector3 ave_vel_ang =
      cur_kin.twist.angular + cur_kin.accels.angular * dt_sec;

  // Step 1 - Calculate restitution impulse (normal to impact)

  // Velocity at contact point
  const Vector3 ave_vel_lin_body = PhysicsUtils::TransformVectorToBodyFrame(
      ave_vel_lin, cur_kin.pose.orientation);

  // Contact point vector
  const Vector3 r_contact =
      collision_info.impact_point - collision_info.position;

  const Vector3 contact_vel_body =
      ave_vel_lin_body + ave_vel_ang.cross(r_contact);

  // GafferOnGames - Collision response with columb friction
  // http://gafferongames.com/virtual-go/collision-response-and-coulomb-friction/
  // Assuming collision is with static fixed body,
  // impulse magnitude = j = -(1 + R)V.N / (1/m + (I'(r X N) X r).N)
  // Physics Part 3, Collision Response, Chris Hecker, eq 4(a)
  // http://chrishecker.com/images/e/e7/Gdmphys3.pdf
  // V(t+1) = V(t) + j*N / m
  // TODO This formula doesn't produce realistic bounces, improve it
  const float restitution_impulse_denom =
      mass_inv + (inertia_inv * r_contact.cross(normal_body))
                     .cross(r_contact)
                     .dot(normal_body);

  const float restitution_impulse = -contact_vel_body.dot(normal_body) *
                                    (1.0f + restitution) /
                                    restitution_impulse_denom;

  // Step 2 - Add restitution impulse to next twist velocities

  kin_with_collision.twist.linear =
      ave_vel_lin + collision_info.normal * restitution_impulse * mass_inv;

  // TODO Shouldn't this formula should account for inertia?
  kin_with_collision.twist.angular =
      ave_vel_ang + r_contact.cross(normal_body) * restitution_impulse;

  // Step 3 - Calculate friction impulse (tangent to contact)

  const Vector3 contact_tang_body =
      contact_vel_body - normal_body * normal_body.dot(contact_vel_body);

  const Vector3 contact_tang_unit_body = contact_tang_body.normalized();

  const float friction_mag_denom =
      mass_inv + (inertia_inv * r_contact.cross(contact_tang_unit_body))
                     .cross(r_contact)
                     .dot(contact_tang_unit_body);

  const float friction_mag =
      -contact_tang_body.norm() * friction / friction_mag_denom;

  // Step 4 - Add friction impulse to next twist velocities

  const Vector3 contact_tang_unit = PhysicsUtils::TransformVectorToWorldFrame(
      contact_tang_unit_body, cur_kin.pose.orientation);

  kin_with_collision.twist.linear += contact_tang_unit * friction_mag;

  kin_with_collision.twist.angular +=
      r_contact.cross(contact_tang_unit_body) * friction_mag * mass_inv;

  // Step 5 - Zero out next accels during collision response to prevent
  //          cancelling out impulse response

  kin_with_collision.accels.linear = Vector3::Zero();
  kin_with_collision.accels.angular = Vector3::Zero();

  // Step 6 - Calculate next position/orientation from collision position

  kin_with_collision.pose.position =
      collision_info.position +
      (collision_info.normal * collision_info.penetration_depth) +
      (kin_with_collision.twist.linear * dt_sec);

  kin_with_collision.pose.orientation = cur_kin.pose.orientation;

  return kin_with_collision;
}

Wrench FastPhysicsModel::CalcDragFaceWrench(
    const Vector3& ave_vel_lin, const Vector3& ave_vel_ang,
    const std::vector<DragFace>& drag_faces, const Quaternion& orientation,
    float air_density, const Vector3& wind_velocity) {
  Wrench drag_wrench = Wrench::Zero();

  auto net_ave_vel_lin_body = ave_vel_lin - wind_velocity;
  const Vector3 ave_vel_lin_body = PhysicsUtils::TransformVectorToBodyFrame(
      net_ave_vel_lin_body, orientation);

  // Aggregate drag wrench over all drag faces in the body frame
  for (const DragFace& drag_face : drag_faces) {
    // Use just the component of body linear velocity normal to the drag face.
    // Trying to apply drag from rotational velocity of the faces properly would
    // require more complex physics.
    const float vel_normal = drag_face.normal.dot(ave_vel_lin_body);

    // If vel_normal is negative then we cull the face. If vel_normal is too low
    // then drag is not generated.
    if (vel_normal > kDragMinVelocity) {
      // Drag vector magnitude is proportional to v^2, with direction opposite
      // of velocity. Total drag = b*v + c*v*v but we ignore the first term as
      // b << c (pg 44, Classical Mechanics, John Taylor).
      const Vector3 drag_force = ave_vel_lin_body.stableNormalized() *
                                 (-drag_face.drag_factor * drag_face.area *
                                  air_density * vel_normal * vel_normal);

      drag_wrench.force += drag_force;
      // No drag torque is applied because we only calculate force opposing the
      // motion of the CG. To generate drag torque, more complex physics to
      // calculate the center of pressure location would be needed to apply the
      // drag force at a moment arm away from the CG.
    }
  }

  // Transform total drag force back to world frame (forces/linear kinematics
  // are in world frame, moments/angular kinematics are in body frame).
  drag_wrench.force =
      PhysicsUtils::TransformVectorToWorldFrame(drag_wrench.force, orientation);

  return drag_wrench;
}

Wrench FastPhysicsModel::CalcLiftDragWrench(
    const std::vector<std::reference_wrapper<const Link>>& lift_drag_links,
    const std::unordered_map<std::string, const float*>&
        lift_drag_control_angles,
    const Quaternion& body_orientation, const Vector3& body_velocity,
    float air_density, const Vector3& wind_velocity) {
  // This method outputs total lift_drag_wrench on body in world frame
  Wrench lift_drag_wrench = Wrench::Zero();

  // Calculate lift-drag on the body from each link with lift-drag params
  for (std::reference_wrapper<const Link> lift_drag_link_ref :
       lift_drag_links) {
    const Link& lift_drag_link = lift_drag_link_ref.get();
    const LiftDrag& lift_drag_params =
        lift_drag_link.GetInertial().GetLiftDrag();

    // Get control_angle for this link
    const auto control_angle_itr =
        lift_drag_control_angles.find(lift_drag_link.GetID());

    const float control_angle =
        (control_angle_itr != lift_drag_control_angles.end())
            ? *(control_angle_itr->second)
            : 0.0f;

    // Step 1 - Get linear vel of link's center of pressure (CP) in world frame

    // For now, assume CP velocity is same as the body velocity.
    // TODO Account for link's angular velocity to adjust CP velocity.
    // const Vector3& cp_translation = lift_drag_params.center_pressure_xyz;
    const Vector3& cp_vel_linear_world = body_velocity - wind_velocity;

    // Skip this link if its velocity is too low to generate lift/drag
    if (cp_vel_linear_world.stableNorm() <= kDragMinVelocity) continue;

    // Step 2 - Calculate direction vectors in world frame

    // Get the link's frame orientation relative to world frame
    Pose pose;
    const TransformTree::RefFrame& refframe = lift_drag_link.GetInertial();
    refframe.GetTransformTree()->Convert(pose, refframe,
                                         TransformTree::kRefFrameGlobal, &pose);
    const auto& link_orientation_world = pose.orientation;

    // Link's forward direction in world frame
    const Vector3 forward_world = PhysicsUtils::TransformVectorToWorldFrame(
        lift_drag_params.forward_xyz, link_orientation_world);

    // Skip this link if its velocity is not in forward direction
    if (forward_world.dot(cp_vel_linear_world) <= 0.0f) continue;

    // Link's upward direction in world frame
    const Vector3 upward_world = PhysicsUtils::TransformVectorToWorldFrame(
        lift_drag_params.upward_xyz, link_orientation_world);

    // Direction normal to lift-drag plane
    const Vector3 lift_drag_plane_normal =
        forward_world.cross(upward_world).stableNormalized();

    // Step 3 - Adjust for the link's sideways motion/sweep angle by reducing
    // velocity to only its component in the lift-drag plane

    const Vector3 vel_normal_to_lift_drag_plane =
        cp_vel_linear_world.dot(lift_drag_plane_normal) *
        lift_drag_plane_normal;

    const Vector3 vel_in_lift_drag_plane =
        cp_vel_linear_world - vel_normal_to_lift_drag_plane;

    // Step 4 - Calculate Angle of Attack alpha (angle between velocity
    // direction in the lift-drag plane and the link's forward direction):

    const Vector3 lift_direction =
        lift_drag_plane_normal.cross(vel_in_lift_drag_plane).stableNormalized();

    const Vector3 drag_direction = (-vel_in_lift_drag_plane).stableNormalized();

    const Vector3 moment_direction = lift_drag_plane_normal;

    // cos(alpha) = (a . b) / ||a|| * ||b|| = (a . b) for unit vectors a & b
    const float cos_alpha =
        std::clamp(lift_direction.dot(upward_world), -1.0f, 1.0f);

    float alpha = (lift_direction.dot(forward_world) >= 0.0f)
                      ? lift_drag_params.alpha_0 + std::acos(cos_alpha)
                      : lift_drag_params.alpha_0 - std::acos(cos_alpha);

    // Normalize alpha to between +/-90 deg
    while (std::abs(alpha) > 0.5 * M_PI) {
      alpha = alpha > 0 ? alpha - M_PI : alpha + M_PI;
    }

    // Step 5 - Calculate dynamic pressure:
    //   q = 1/2 * rho * (vel in lift-drag plane)^2

    const float spd_in_lift_drag_plane = vel_in_lift_drag_plane.stableNorm();

    const float q =
        0.5f * air_density * spd_in_lift_drag_plane * spd_in_lift_drag_plane;

    // Step 6 - Calculate coeff of lift cl and lift force

    double cl;
    const float alpha_stall = lift_drag_params.alpha_stall;
    const float cla = lift_drag_params.c_lift_alpha;
    const float cla_stall = lift_drag_params.c_lift_alpha_stall;

    if (alpha > alpha_stall) {
      cl = cla * alpha_stall + cla_stall * (alpha - alpha_stall);
      // Guard to prevent neg lift with pos angle of attack beyond stall point
      cl = std::max(cl, 0.0);
    } else if (alpha < -alpha_stall) {
      cl = -cla * alpha_stall + cla_stall * (alpha + alpha_stall);
      // Guard to prevent pos lift with neg angle of attack beyond stall point
      cl = std::min(cl, 0.0);
    } else {
      cl = cla * alpha;
    }

    // Add lift effect from control surface angle
    cl += lift_drag_params.control_surface_cl_per_rad * control_angle;

    // Calculate lift force
    const Vector3 lift_force = cl * q * lift_drag_params.area * lift_direction;

    // Step 7 - Calculate coeff of drag cd and drag force

    double cd;
    const float cda = lift_drag_params.c_drag_alpha;
    const float cda_stall = lift_drag_params.c_drag_alpha_stall;

    if (alpha > alpha_stall) {
      cd = cda * alpha_stall + cda_stall * (alpha - alpha_stall);
      // Guard to prevent neg drag with pos angle of attack beyond stall point
      cd = std::max(cd, 0.0);
    } else if (alpha < -alpha_stall) {
      cd = -cda * alpha_stall + cda_stall * (alpha + alpha_stall);
      // Guard to prevent pos drag with neg angle of attack beyond stall point
      cd = std::min(cd, 0.0);
    } else {
      cd = cda * alpha;
    }

    // Drag magnitude is always positive in drag direction (opposite of link's
    // motion direction)
    cd = std::abs(cd);

    // Add additional positive drag effect from control surface angle
    cd += std::abs(lift_drag_params.control_surface_cd_per_rad * control_angle);

    // Calculate drag force
    const Vector3 drag_force = cd * q * lift_drag_params.area * drag_direction;

    // Step 8 - Calculate coeff of moment cm and moment

    double cm;
    const float cma = lift_drag_params.c_moment_alpha;
    const float cma_stall = lift_drag_params.c_moment_alpha_stall;

    if (alpha > alpha_stall) {
      cm = cma * alpha_stall + cma_stall * (alpha - alpha_stall);
      // Guard to prevent neg moment with pos angle of attack beyond stall point
      cm = std::max(cm, 0.0);
    } else if (alpha < -alpha_stall) {
      cm = -cma * alpha_stall + cma_stall * (alpha + alpha_stall);
      // Guard to prevent pos moment with neg angle of attack beyond stall point
      cm = std::min(cm, 0.0);
    } else {
      cm = cma * alpha;
    }

    // Add moment effect from control surface angle
    cm += lift_drag_params.control_surface_cm_per_rad * control_angle;

    // Calculate moment in world frame and convert to body frame
    const Vector3 moment_world =
        cm * q * lift_drag_params.area * moment_direction;

    const Vector3 moment_body = PhysicsUtils::TransformVectorToBodyFrame(
        moment_world, body_orientation);

    // Step 9 - Calculate total force and aggregate this link's lift-drag wrench
    // into total lift_drag_wrench

    const Vector3 total_force_world = lift_force + drag_force;

    // Add additional torque from force applied at radius to CG, tau = r X F
    const Vector3 total_force_body = PhysicsUtils::TransformVectorToBodyFrame(
        total_force_world, body_orientation);

    const Vector3 cg_to_cp_translation =
        lift_drag_link.GetInertial().GetOrigin().translation_ +
        lift_drag_params.center_pressure_xyz;

    const Vector3 torque_from_force_body =
        cg_to_cp_translation.cross(total_force_body);

    // Check force and moment for NaN
    if (PhysicsUtils::HasNan(total_force_world) ||
        PhysicsUtils::HasNan(torque_from_force_body) ||
        PhysicsUtils::HasNan(moment_body)) {
      throw Error("NaN in lift-drag wrench.");
    }

    // Aggregate this link's lift-drag force (in world frame) and torque (in
    // body frame) into total lift_drag_wrench
    lift_drag_wrench.force += total_force_world;

    lift_drag_wrench.torque += torque_from_force_body;
    lift_drag_wrench.torque += moment_body;
  }

  return lift_drag_wrench;
}

}  // namespace projectairsim
}  // namespace microsoft
