// Copyright (C) IAMAI CONSULTING CORP
//
// MIT License. All rights reserved.

#include "jsbsim_physics.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "jsbsim_ground_callback.hpp"
#include "core_sim/actuators/actuator.hpp"
#include "core_sim/actuators/lift_drag_control_surface.hpp"
#include "core_sim/actuators/rotor.hpp"
#include "core_sim/earth_utils.hpp"
#include "core_sim/math_utils.hpp"
#include "core_sim/physics_common_types.hpp"
#include "core_sim/physics_common_utils.hpp"
#include "core_sim/transforms/transform_utils.hpp"
#include "core_sim/geodetic_converter.hpp"
#include "initialization/FGInitialCondition.h"
#include "initialization/FGTrim.h"
#include "FGFDMExec.h" // JSBSim
#include "models/FGInertial.h"
#include "models/FGAuxiliary.h"
#include "models/FGPropagate.h"
#include "models/FGAccelerations.h"



namespace microsoft {
namespace projectairsim {

namespace {

constexpr double kCommandActiveThreshold = 0.5;

Robot::TerrainElevationCallback BuildTerrainCallback(
    Robot sim_robot) {
  const double fallback_terrain_elevation_asl_m = static_cast<double>(
      sim_robot.GetEnvironment().home_geo_point.geo_point.altitude);
  constexpr double kMaxReasonableNedDistanceMeters = 100000.0;

  return [sim_robot, fallback_terrain_elevation_asl_m,
          kMaxReasonableNedDistanceMeters](double x_m,
                                           double y_m) mutable {
    double query_x_m = x_m;
    double query_y_m = y_m;

    const bool invalid_or_extreme_query =
        !std::isfinite(query_x_m) || !std::isfinite(query_y_m) ||
        std::abs(query_x_m) > kMaxReasonableNedDistanceMeters ||
        std::abs(query_y_m) > kMaxReasonableNedDistanceMeters;

    if (invalid_or_extreme_query) {
      const auto& current_position = sim_robot.GetKinematics().pose.position;
      query_x_m = static_cast<double>(current_position.x());
      query_y_m = static_cast<double>(current_position.y());
    }

    const auto cached_terrain_elevation_asl_m =
        sim_robot.GetCachedTerrainElevationASL();
    if (std::isfinite(cached_terrain_elevation_asl_m)) {
      return cached_terrain_elevation_asl_m;
    }

    auto terrain_elevation_cb = sim_robot.GetTerrainElevationCallback();
    if (terrain_elevation_cb != nullptr) {
      const auto terrain_cb_val = terrain_elevation_cb(query_x_m, query_y_m);
      if (std::isfinite(terrain_cb_val)) {
        return terrain_cb_val;
      }
    }

    return fallback_terrain_elevation_asl_m;
  };
}

}  // namespace

// -----------------------------------------------------------------------------
// class JSBSimPhysicsBody

JSBSimPhysicsBody::JSBSimPhysicsBody(const Robot& robot)
    : sim_robot_(robot) {
  SetName(robot.GetID());
  SetPhysicsType(PhysicsType::kJSBSimPhysics);
  InitializeJSBSimPhysicsBody(robot);
}

void JSBSimPhysicsBody::InitializeJSBSimPhysicsBody(
    const Robot& robot) {
      const auto& links = sim_robot_.GetLinks();
      const auto& root_link = links.at(0);  // TODO allow config to choose root link

  //external_wrench_points_.clear();
  SetMass(root_link.GetInertial().GetMass());  // TODO get mass from JSBSim
  friction_ = root_link.GetCollision().GetFriction();
  restitution_ = root_link.GetCollision().GetRestitution();
  is_grounded_ = sim_robot_.GetStartLanded();
  
  model_ = sim_robot_.GetJSBSimModel();
  jsbsim_dt_sec_ = sim_robot_.GetJSBSimDtSec();
  model_->Setdt(jsbsim_dt_sec_);

  // get difference betweeen JSBSim's initial geopoint position and home geopoint position
  const auto& world_geopoint = sim_robot_.GetEnvironment().home_geo_point.geo_point;
  const auto& model_geopoint = sim_robot_.GetEnvironment().env_info.geo_point;
  home_geopoint_ =
      Vector3(model_geopoint.latitude, model_geopoint.longitude,
              model_geopoint.altitude);
  //Get distance between home geopoint and JSBSim's initial geopoint
  GeodeticConverter converter(world_geopoint.latitude, world_geopoint.longitude,
                              world_geopoint.altitude); 
  converter.geodetic2Ned(home_geopoint_[0], home_geopoint_[1], home_geopoint_[2], &home_geopoint_ned_[0], &home_geopoint_ned_[1], &home_geopoint_ned_[2]);
}

void JSBSimPhysicsBody::EnsureJSBSimInitialized() {
  if (!jsbsim_initialized_) {
    InitializeJSBSim();
  }
}

void JSBSimPhysicsBody::InitializeJSBSim(){
  //updates jsbsim initial conditions with current state
  auto& init = *model_->GetIC();
  //SGPath init_file("reset00.xml");
  //init.Load(init_file);
  init.SetGeodLatitudeDegIC(sim_robot_.GetEnvironment().env_info.geo_point.latitude);
  init.SetLongitudeDegIC(sim_robot_.GetEnvironment().env_info.geo_point.longitude);
  auto alt_ft =
      sim_robot_.GetEnvironment().env_info.geo_point.altitude * MathUtils::meters_to_feets; // TODO: method to convert from m/s to fps
  init.SetAltitudeASLFtIC(alt_ft);

  const auto& home_geo_point =
      sim_robot_.GetEnvironment().home_geo_point.geo_point;
  const auto ground_mode = sim_robot_.GetJSBSimGroundSettings().mode;
  double terrain_elevation_asl_m = home_geo_point.altitude;

  auto terrain_elevation_cb = BuildTerrainCallback(sim_robot_);
  const auto& position = sim_robot_.GetKinematics().pose.position;
  const auto terrain_cb_val = terrain_elevation_cb(position.x(), position.y());
  if (std::isfinite(terrain_cb_val)) {
    terrain_elevation_asl_m = terrain_cb_val;
  }

  // Set terrain elevation at initial position
  init.SetTerrainElevationFtIC(terrain_elevation_asl_m *
                               MathUtils::meters_to_feets);

  auto orientation = TransformUtils::ToRPY(sim_robot_.GetKinematics().pose.orientation);
  init.SetPsiDegIC(MathUtils::rad2Deg(orientation.z()));
  init.SetThetaDegIC(MathUtils::rad2Deg(orientation.y()));
  init.SetPhiDegIC(MathUtils::rad2Deg(orientation.x()));
  auto velocity = sim_robot_.GetKinematics().twist.linear;
  init.SetVNorthFpsIC(velocity.x() * MathUtils::meters_to_feets); // TODO: method to convert from m/s to fps
  init.SetVEastFpsIC(velocity.y() * MathUtils::meters_to_feets);
  init.SetVDownFpsIC(velocity.z() * MathUtils::meters_to_feets);
  auto angular_velocity = sim_robot_.GetKinematics().twist.angular;
  init.SetPRadpsIC(angular_velocity.x());
  init.SetQRadpsIC(angular_velocity.y());
  init.SetRRadpsIC(angular_velocity.z());
  auto wind_velocity = sim_robot_.GetEnvironment().wind_velocity;
  init.SetWindNEDFpsIC(wind_velocity.x() * MathUtils::meters_to_feets, wind_velocity.y() * MathUtils::meters_to_feets,
                       wind_velocity.z() * MathUtils::meters_to_feets);
                       
  // run initial conditions
  if(model_->RunIC() == false){
    throw std::runtime_error("JSBSim failed to initialize");
  }

  // Now FGInertial has set the ellipse - safe to install custom ground callback
  if (model_->GetInertial() != nullptr) {
    if (ground_mode == JSBSimGroundMode::kTerrain) {
      model_->GetInertial()->SetGroundCallback(new JSBSimTerrainGroundCallback(
          terrain_elevation_cb, home_geo_point));
    } else {
      model_->GetInertial()->SetGroundCallback(new JSBSimConstantGroundCallback(
          terrain_elevation_asl_m));
    }
  }

  jsbsim_initialized_ = true;
}

void JSBSimPhysicsBody::SetJSBSimAltitudeGrounded(float delta_altitude){
  auto* propagate = model_->GetPropagate();
  auto terrain_elevation_val = propagate->GetTerrainElevation() + delta_altitude * MathUtils::meters_to_feets;
  propagate->SetTerrainElevation(terrain_elevation_val);
  auto altitude = propagate->GetAltitudeASLmeters();
  auto new_altitude = altitude + delta_altitude;
  propagate->SetAltitudeASLmeters(new_altitude);
}

void JSBSimPhysicsBody::SyncJSBSimStateToKinematics(
    const Kinematics& kinematics) {
  if (!jsbsim_initialized_ || model_ == nullptr ||
      model_->GetPropagate() == nullptr) {
    return;
  }

  auto* propagate = model_->GetPropagate();

  const double model_north_m =
      static_cast<double>(kinematics.pose.position.x()) -
      home_geopoint_ned_[0];
  const double model_east_m =
      static_cast<double>(kinematics.pose.position.y()) -
      home_geopoint_ned_[1];
  const double model_down_m =
      static_cast<double>(kinematics.pose.position.z()) -
      home_geopoint_ned_[2];

  GeodeticConverter converter(home_geopoint_[0], home_geopoint_[1],
                              home_geopoint_[2]);
  double latitude_deg = 0.0;
  double longitude_deg = 0.0;
  float altitude_m = 0.0f;
  converter.ned2Geodetic(model_north_m, model_east_m,
                         static_cast<float>(model_down_m), &latitude_deg,
                         &longitude_deg, &altitude_m);

  if (!std::isfinite(latitude_deg) || !std::isfinite(longitude_deg) ||
      !std::isfinite(altitude_m)) {
    return;
  }

  auto location = propagate->GetLocation();
  location.SetPositionGeodetic(
      MathUtils::deg2Rad(longitude_deg), MathUtils::deg2Rad(latitude_deg),
      static_cast<double>(altitude_m) * MathUtils::meters_to_feets);

  // Refresh JSBSim's local-frame matrices at the new location before building
  // the matching inertial attitude for SetVState().
  propagate->SetLocation(location);

  auto vehicle_state = propagate->GetVState();
  vehicle_state.vLocation = location;

  const auto rpy = TransformUtils::ToRPY(kinematics.pose.orientation);
  const JSBSim::FGQuaternion local_orientation(rpy.x(), rpy.y(), rpy.z());
  vehicle_state.qAttitudeECI =
      propagate->GetTi2l().GetQuaternion() * local_orientation;

  const Vector3 linear_velocity_body =
      PhysicsUtils::TransformVectorToBodyFrame(
          kinematics.twist.linear, kinematics.pose.orientation);
  vehicle_state.vUVW =
      JSBSim::FGColumnVector3(linear_velocity_body.x() *
                                  MathUtils::meters_to_feets,
                              linear_velocity_body.y() *
                                  MathUtils::meters_to_feets,
                              linear_velocity_body.z() *
                                  MathUtils::meters_to_feets);
  vehicle_state.vPQR =
      JSBSim::FGColumnVector3(kinematics.twist.angular.x(),
                              kinematics.twist.angular.y(),
                              kinematics.twist.angular.z());

  propagate->SetVState(vehicle_state);
  propagate->InitializeDerivatives();
}

void JSBSimPhysicsBody::CalculateExternalWrench() {
  // No need to do anything here, all wrenches are calculated in the step by JSBSim
}

void JSBSimPhysicsBody::ReadRobotData() {
  kinematics_ = sim_robot_.GetKinematics();
  collision_info_ = sim_robot_.GetCollisionInfo();
  const auto& cur_env = sim_robot_.GetEnvironment();
  const auto& cur_env_info = cur_env.env_info;
  env_gravity_ = cur_env_info.gravity;
  env_air_density_ = cur_env_info.air_density;
  env_wind_velocity_ = cur_env.wind_velocity;
}

void JSBSimPhysicsBody::WriteRobotData(const Kinematics& kinematics,
                                     TimeNano external_time_stamp) {
  sim_robot_.UpdateKinematics(kinematics, external_time_stamp);
}

bool JSBSimPhysicsBody::IsStillGrounded() {

  if (is_grounded_ &&
      kinematics_.twist.linear.z() >= 0) {
    is_grounded_ = true;
  } else {
    is_grounded_ = false;
  }

  return is_grounded_;
}

bool JSBSimPhysicsBody::IsLandingCollision() {
  if (collision_info_.has_collided == false) {
    return false;
  }

  // Check if collision normal is primarily vertically upward (assumes NED)
  const float normal_norm = collision_info_.normal.norm();
  if (!std::isfinite(normal_norm) ||
      MathUtils::IsApproximatelyZero(normal_norm)) {
    return false;
  }

  const Vector3 normal_world = collision_info_.normal / normal_norm;
  const bool is_ground_normal = MathUtils::IsApproximatelyEqual(
      normal_world.z(), -1.0f, kGroundCollisionAxisTol);

  if (is_ground_normal) {
    is_grounded_ = true;
    return true;
  } else {
    return is_grounded_;
  }
}

bool JSBSimPhysicsBody::NeedsCollisionResponse(const Kinematics& next_kin) {
  const bool is_moving_into_collision =
      (collision_info_.normal.dot(next_kin.twist.linear) < 0.0f);

  if (collision_info_.has_collided && is_moving_into_collision) {
    return true;
  } else {
    return false;
  }
}

// -----------------------------------------------------------------------------
// class JSBSimPhysicsModel

void JSBSimPhysicsModel::SetWrenchesOnPhysicsBody(
    std::shared_ptr<BasePhysicsBody> body) {
}

void JSBSimPhysicsModel::StepPhysicsBody(TimeNano dt_nanos,
                                         std::shared_ptr<BasePhysicsBody> body) {
  // Dynamic cast to a JSBSimPhysicsBody
  std::shared_ptr<JSBSimPhysicsBody> fp_body =
      std::dynamic_pointer_cast<JSBSimPhysicsBody>(body);

  if (fp_body != nullptr) {
    auto dt_sec = SimClock::Get()->NanosToSec(dt_nanos);
    Kinematics next_kin;
    bool sync_jsbsim_state = false;

    // Step 1 - Update physics body's data from robot's latest data
    fp_body->ReadRobotData();
    fp_body->EnsureJSBSimInitialized();

    // Step 2 - Calculate kinematics
    next_kin = CalcNextKinematicsNoCollision(dt_sec, fp_body);
    

    // Step 3 - Calculate kinematics with collision response if needed
    if (fp_body->NeedsCollisionResponse(next_kin)) {
      // JSBSim already resolves ground contact through its ground callback and
      // landing-gear model. Do not advance it a second time for the same
      // Project AirSim tick when the external host also reports ground contact.
      if (!fp_body->IsLandingCollision()) {
        next_kin = CalcNextKinematicsWithCollision(dt_sec, fp_body);
        sync_jsbsim_state = true;
      }
    }

    if (sync_jsbsim_state) {
      fp_body->SyncJSBSimStateToKinematics(next_kin);
    }

    // Step 4 - Save new kinematics to body's referenced sim robot
    fp_body->WriteRobotData(next_kin);
  }
}

Kinematics JSBSimPhysicsModel::CalcNextKinematicsGrounded(
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

Vector3 JSBSimPhysicsBody::GetModelCoordinates(){ //TODO: use GeoPoint instead of Vector3
//Get JSBSim geopoint
  auto* propagate = model_->GetPropagate();
  auto lat_deg = propagate->GetLocation().GetLatitudeDeg();
  auto lon_deg = propagate->GetLocation().GetLongitudeDeg();
  auto alt_mt = propagate->GetAltitudeASLmeters();
  return Vector3(lat_deg, lon_deg, alt_mt);
}

Vector3 JSBSimPhysicsBody::GetModelPosition() {
  auto* propagate = model_->GetPropagate();

  GeodeticConverter converter(home_geopoint_[0], home_geopoint_[1],
                              home_geopoint_[2]);

  double n = 0.0;
  double e = 0.0;
  double d = 0.0;
  converter.geodetic2Ned(propagate->GetLocation().GetGeodLatitudeDeg(),
                         propagate->GetLocation().GetLongitudeDeg(),
                         static_cast<float>(propagate->GetAltitudeASLmeters()),
                         &n, &e, &d);

  return Vector3(n, e, d);
}

Quaternion JSBSimPhysicsBody::GetModelOrientation() {
  auto* propagate = model_->GetPropagate();
  double roll = propagate->GetEuler(1);
  double pitch = propagate->GetEuler(2);
  double yaw = propagate->GetEuler(3);
  return TransformUtils::ToQuaternion(roll, pitch, yaw);
}

Vector3 JSBSimPhysicsBody::GetModelLinearVelocity() {
  auto* propagate = model_->GetPropagate();
  double n = propagate->GetVel(1) * MathUtils::feets_to_meters; //TODO: replace for a FeetsToMeters method
  double e = propagate->GetVel(2) * MathUtils::feets_to_meters;
  double d = propagate->GetVel(3) * MathUtils::feets_to_meters;
  return Vector3(n, e, d);
}

//Body Frame
Vector3 JSBSimPhysicsBody::GetModelAngularVelocity() {
  auto* propagate = model_->GetPropagate();
  double p = propagate->GetPQR(1);
  double q = propagate->GetPQR(2);
  double r = propagate->GetPQR(3);
  return Vector3(p, q, r);
}

// Returns inertial linear acceleration in world frame (NED).
// GetBodyAccel() is applied force divided by mass, expressed in body axes, and
// excludes gravity. Rotate it to NED and add gravity to preserve the Kinematics
// contract; the IMU later subtracts gravity and rotates back to body axes.
// GetUVWdot() cannot be used here because it also contains rotating-frame
// terms, which would inject omega-cross-velocity errors into the IMU.
Vector3 JSBSimPhysicsBody::GetModelLinearAcceleration() {
  auto* accel = model_->GetAccelerations();

  const Vector3 specific_force_body(
      accel->GetBodyAccel(1) * MathUtils::feets_to_meters,
      accel->GetBodyAccel(2) * MathUtils::feets_to_meters,
      accel->GetBodyAccel(3) * MathUtils::feets_to_meters);

  const Quaternion orientation = GetModelOrientation();
  return PhysicsUtils::TransformVectorToWorldFrame(specific_force_body,
                                                    orientation) +
         env_gravity_;
}

//Body Frame
Vector3 JSBSimPhysicsBody::GetModelAngularAcceleration() {
  auto* accel = model_->GetAccelerations();
  double pdot = accel->GetPQRdot(1);
  double qdot = accel->GetPQRdot(2);
  double rdot = accel->GetPQRdot(3);
  return Vector3(pdot, qdot, rdot);
}

Kinematics JSBSimPhysicsBody::GetKinematicsFromModel() {
  Kinematics kinematics;
  kinematics.pose.position = GetModelPosition();
  //add initial x,y position to the model's relative position
  kinematics.pose.position[0] += home_geopoint_ned_[0];
  kinematics.pose.position[1] += home_geopoint_ned_[1];
  kinematics.pose.position[2] += home_geopoint_ned_[2];
  kinematics.pose.orientation = GetModelOrientation();
  kinematics.twist.linear = GetModelLinearVelocity();
  kinematics.twist.angular = GetModelAngularVelocity();
  kinematics.accels.linear = GetModelLinearAcceleration();
  kinematics.accels.angular = GetModelAngularAcceleration();
  return kinematics;
}

Kinematics JSBSimPhysicsModel::CalcNextKinematicsNoCollision(
    TimeSec dt_sec, std::shared_ptr<JSBSimPhysicsBody>& fp_body) {
  // Advance JSBSim by exactly one Project AirSim physics interval. Treat the
  // configured JSBSim dt as the maximum integration substep. The previous
  // residual-time accumulator published an unchanged state whenever the PAS
  // tick was shorter than jsbsim_dt_sec_, followed by a full-step jump.
  constexpr double kTimeEpsilonSec = 1e-12;
  double remaining_time_sec = dt_sec;
  while (remaining_time_sec > kTimeEpsilonSec) {
    const double integration_step_sec =
        std::min(remaining_time_sec, fp_body->jsbsim_dt_sec_);
    fp_body->model_->Setdt(integration_step_sec);
    fp_body->model_->Run();
    remaining_time_sec -= integration_step_sec;
  }
  fp_body->model_->Setdt(fp_body->jsbsim_dt_sec_);

  return fp_body->GetKinematicsFromModel();
}

// TO-DO: this is a copy-paste from fast physics. Extract as a common collision reaction 
Kinematics JSBSimPhysicsModel::CalcNextKinematicsWithCollision(
    TimeSec dt_sec, std::shared_ptr<JSBSimPhysicsBody>& fp_body) {
  const auto& collision_info = fp_body->collision_info_;
  const auto& cur_kin = fp_body->kinematics_;
  const auto& restitution = fp_body->restitution_;
  const auto& friction = fp_body->friction_;
  const auto& mass_inv = fp_body->mass_inv_;
  const auto& inertia_inv = fp_body->inertia_inv_;
  constexpr float kMinImpulseDenom = 1e-5f;
  constexpr float kMinTangentNorm = 1e-4f;
  constexpr float kMaxPenetrationDepth = 1.0f;
  constexpr float kMaxDeltaVCollision = 20.0f;

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
  // TODO(edufford) This formula doesn't produce realistic bounces, improve it
  const float restitution_impulse_denom =
      mass_inv + (inertia_inv * r_contact.cross(normal_body))
                     .cross(r_contact)
                     .dot(normal_body);

  if (!std::isfinite(restitution_impulse_denom) ||
      std::abs(restitution_impulse_denom) < kMinImpulseDenom) {
    kin_with_collision = cur_kin;
    kin_with_collision.accels.linear = Vector3::Zero();
    kin_with_collision.accels.angular = Vector3::Zero();
    kin_with_collision.twist.linear = Vector3::Zero();
    kin_with_collision.twist.angular = Vector3::Zero();
    const auto penetration_depth_clamped =
        std::clamp(collision_info.penetration_depth, 0.0f, kMaxPenetrationDepth);
    kin_with_collision.pose.position =
        collision_info.position +
        (collision_info.normal * penetration_depth_clamped);
    return kin_with_collision;
  }

  float restitution_impulse = -contact_vel_body.dot(normal_body) *
                              (1.0f + restitution) /
                              restitution_impulse_denom;

  const float mass = mass_inv > kMinImpulseDenom ? (1.0f / mass_inv) : 0.0f;
  const float max_impulse = mass * kMaxDeltaVCollision;
  if (std::isfinite(max_impulse) && max_impulse > 0.0f) {
    restitution_impulse = std::clamp(restitution_impulse, -max_impulse, max_impulse);
  }

  // Step 2 - Add restitution impulse to next twist velocities

  kin_with_collision.twist.linear =
      ave_vel_lin + collision_info.normal * restitution_impulse * mass_inv;

  // TODO(edufford) Shouldn't this formula should account for inertia?
  kin_with_collision.twist.angular =
      ave_vel_ang + r_contact.cross(normal_body) * restitution_impulse;

  // Step 3 - Calculate friction impulse (tangent to contact)

  const Vector3 contact_tang_body =
      contact_vel_body - normal_body * normal_body.dot(contact_vel_body);

  if (!std::isfinite(contact_tang_body.norm()) ||
      contact_tang_body.norm() < kMinTangentNorm) {
    kin_with_collision.accels.linear = Vector3::Zero();
    kin_with_collision.accels.angular = Vector3::Zero();
    const auto penetration_depth_clamped =
        std::clamp(collision_info.penetration_depth, 0.0f, kMaxPenetrationDepth);
    kin_with_collision.pose.position =
        collision_info.position +
        (collision_info.normal * penetration_depth_clamped) +
        (kin_with_collision.twist.linear * dt_sec);
    kin_with_collision.pose.orientation = cur_kin.pose.orientation;
    return kin_with_collision;
  }

  const Vector3 contact_tang_unit_body = contact_tang_body.normalized();

  const float friction_mag_denom =
      mass_inv + (inertia_inv * r_contact.cross(contact_tang_unit_body))
                     .cross(r_contact)
                     .dot(contact_tang_unit_body);

  float friction_mag =
      -contact_tang_body.norm() * friction / friction_mag_denom;
  if (std::isfinite(max_impulse) && max_impulse > 0.0f) {
    friction_mag = std::clamp(friction_mag, -max_impulse, max_impulse);
  }

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
      (collision_info.normal *
       std::clamp(collision_info.penetration_depth, 0.0f,
            kMaxPenetrationDepth)) +
      (kin_with_collision.twist.linear * dt_sec);

  kin_with_collision.pose.orientation = cur_kin.pose.orientation;

  return kin_with_collision;
}

}  // namespace projectairsim
}  // namespace microsoft
