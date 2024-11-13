// Copyright (C) Microsoft Corporation. All rights reserved.

#include "jsbsim_physics.hpp"

#include <memory>
#include <vector>

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



namespace microsoft {
namespace projectairsim {

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
  model_->SetPropertyValue("simulation/dt", 0.008333333333); //TODO: use configuration instead of harcoded value

  InitializeJSBSim();

  // get difference betweeen JSBSim's initial geopoint position and home geopoint position
  const auto& world_geopoint = sim_robot_.GetEnvironment().home_geo_point.geo_point;
  home_geopoint_ = GetModelCoordinates();
  //Get distance between home geopoint and JSBSim's initial geopoint
  GeodeticConverter converter(world_geopoint.latitude, world_geopoint.longitude,
                              world_geopoint.altitude); 
  converter.geodetic2Ned(home_geopoint_[0], home_geopoint_[1], home_geopoint_[2], &home_geopoint_ned_[0], &home_geopoint_ned_[1], &home_geopoint_ned_[2]);
  // unreal terrain is flat. Use initial altitude
  home_geopoint_ned_[2] = -(home_geopoint_[2]-world_geopoint.altitude);
}

void JSBSimPhysicsBody::InitializeJSBSim(){
  //updates jsbsim initial conditions with current state
  auto& init = *model_->GetIC();
  //SGPath init_file("reset00.xml");
  //init.Load(init_file);
  init.SetLatitudeDegIC(sim_robot_.GetEnvironment().env_info.geo_point.latitude);
  init.SetLongitudeDegIC(sim_robot_.GetEnvironment().env_info.geo_point.longitude);
  auto alt_ft =
      sim_robot_.GetEnvironment().env_info.geo_point.altitude * MathUtils::meters_to_feets; // TODO: method to convert from m/s to fps
  init.SetAltitudeASLFtIC(alt_ft);
  // Set terrain elevation at initial position
  init.SetTerrainElevationFtIC(alt_ft); 
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

  /*JSBSim::FGTrim trim_(model_.get(), JSBSim::tGround);
  if (!trim_.DoTrim()) 
	{
		trim_.Report();
		trim_.TrimStats();

    throw std::runtime_error("TRIM FAILED");
	}*/
}

void JSBSimPhysicsBody::SetJSBSimAltitudeGrounded(float delta_altitude){
  //model_->SetPropertyValue("position/h-sl-meters", -position[2]);
  auto terrain_elevation_val = model_->GetPropertyValue("position/terrain-elevation-asl-ft") + delta_altitude * MathUtils::meters_to_feets;
  model_->SetPropertyValue("position/terrain-elevation-asl-ft", terrain_elevation_val);
  auto altitude = model_->GetPropertyValue("position/h-sl-meters");
  auto new_altitude = altitude + delta_altitude * MathUtils::meters_to_feets;
  model_->SetPropertyValue("position/h-sl-meters", new_altitude);
}

// TODO Rename this to "SetActuationOutputs"?
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
  const Vector3 normal_body = PhysicsUtils::TransformVectorToBodyFrame(
      collision_info_.normal, kinematics_.pose.orientation);

  const bool is_ground_normal = MathUtils::IsApproximatelyEqual(
      normal_body.z(), -1.0f, kGroundCollisionAxisTol);

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
    // To simulate collision response start uncommenting next lines and delete the two previous ones
    
    // Step 1 - Update physics body's data from robot's latest data
    fp_body->ReadRobotData();
    


    //Step 2 - Calculate kinematics with collision response if needed
    if (fp_body->NeedsCollisionResponse(fp_body->kinematics_)) {
     if (fp_body->IsLandingCollision()) {
       // Calculate the landed position to stick at by offsetting back
       // along the collision normal with an additional kCollisionOffset margin
       // to prevent getting stuck in a collided state.
       // Calculate the landed position to stick at by offsetting back
       // along the collision normal with an additional kCollisionOffset margin
       // to prevent getting stuck in a collided state.
       const Vector3 delta_position =
           fp_body->collision_info_.normal *
               (fp_body->collision_info_.penetration_depth +
                JSBSimPhysicsBody::kCollisionOffset);
      auto landed_position = fp_body->collision_info_.position + delta_position;
       //next_kin = CalcNextKinematicsGrounded(
       //    landed_position, fp_body->kinematics_.pose.orientation);
       // TO-DO: set ground forces to model
       next_kin = CalcNextKinematicsNoCollision(dt_sec, fp_body);
       // set jsbsim to new position
       fp_body->SetJSBSimAltitudeGrounded(-delta_position[2]);
       next_kin.pose.position = landed_position;
       fp_body->WriteRobotData(next_kin);
     } else {
       next_kin = CalcNextKinematicsWithCollision(dt_sec, fp_body);
       fp_body->WriteRobotData(next_kin);
       // Update environment to get updated geopoint
       fp_body->sim_robot_.UpdateEnvironment();
       fp_body->InitializeJSBSim();
     }
    } else {
     next_kin = CalcNextKinematicsNoCollision(dt_sec, fp_body);    
     fp_body->WriteRobotData(next_kin);
    }    
  }
}

//Kinematics JSBSimPhysicsModel::CalcNextKinematicsGrounded(
//    const Vector3& position, const Quaternion& orientation) {
//  Kinematics kin_grounded;
//  // Zero out accels/velocities
//  kin_grounded.accels.linear = Vector3::Zero();
//  kin_grounded.accels.angular = Vector3::Zero();
//  kin_grounded.twist.linear = Vector3::Zero();
//  kin_grounded.twist.angular = Vector3::Zero();
//
//  // Pass-through position
//  kin_grounded.pose.position = position;
//
//  // Zero out roll/pitch, pass-through yaw
//  auto rpy = TransformUtils::ToRPY(orientation);
//  kin_grounded.pose.orientation = TransformUtils::ToQuaternion(0., 0., rpy[2]);
//
//  return kin_grounded;
//}


Vector3 JSBSimPhysicsBody::GetModelCoordinates(){ //TODO: use GeoPoint instead of Vector3
//Get JSBSim geopoint
  auto lat_deg = model_->GetPropertyValue("position/lat-gc-deg");
  auto lon_deg = model_->GetPropertyValue("position/long-gc-deg");
  auto alt_mt = model_->GetPropertyValue("position/h-sl-meters");
  return Vector3(lat_deg, lon_deg, alt_mt);
}

Vector3 JSBSimPhysicsBody::GetModelPosition() {
  //get latitude displacement
  auto delta_lat = model_->GetPropertyValue("position/lat-gc-deg") -
          home_geopoint_[0];
  //get latitude in meters
  double n = std::copysign(
      model_->GetPropertyValue("position/distance-from-start-lat-mt"),
      delta_lat);
  // get longitud displacement
  auto delta_lon = model_->GetPropertyValue("position/long-gc-deg") -
          home_geopoint_[1];
  //get longitud in meters
  double e = std::copysign(
      model_->GetPropertyValue("position/distance-from-start-lon-mt"),
      delta_lon);
  double d =
      -(model_->GetPropertyValue("position/h-sl-meters") - home_geopoint_[2]);
  return Vector3(n, e, d);
}

Quaternion JSBSimPhysicsBody::GetModelOrientation() {
  double roll = model_->GetPropertyValue("attitude/roll-rad");
  double pitch = model_->GetPropertyValue("attitude/pitch-rad");
  double yaw = MathUtils::deg2Rad(model_->GetPropertyValue("attitude/psi-deg"));
  return TransformUtils::ToQuaternion(roll, pitch, yaw);
}

Vector3 JSBSimPhysicsBody::GetModelLinearVelocity() {
  double n = model_->GetPropertyValue("velocities/v-north-fps") * MathUtils::feets_to_meters; //TODO: replace for a FeetsToMeters method
  double e = model_->GetPropertyValue("velocities/v-east-fps") * MathUtils::feets_to_meters;
  double d = model_->GetPropertyValue("velocities/v-down-fps") * MathUtils::feets_to_meters;
  return Vector3(n, e, d);
}

//Body Frame
Vector3 JSBSimPhysicsBody::GetModelAngularVelocity() {
  double p = model_->GetPropertyValue("velocities/p-rad_sec");
  double q = model_->GetPropertyValue("velocities/q-rad_sec");
  double r = model_->GetPropertyValue("velocities/r-rad_sec");
  return Vector3(p, q, r);
}

Vector3 JSBSimPhysicsBody::GetModelLinearAcceleration() {
  double ui = model_->GetPropertyValue("accelerations/uidot-ft_sec2") * MathUtils::feets_to_meters;
  double vi = model_->GetPropertyValue("accelerations/vidot-ft_sec2") * MathUtils::feets_to_meters;
  double wi = model_->GetPropertyValue("accelerations/widot-ft_sec2") * MathUtils::feets_to_meters;
  return Vector3(ui, vi, wi);
}

//Body Frame
Vector3 JSBSimPhysicsBody::GetModelAngularAcceleration() {
  double pdot = model_->GetPropertyValue("accelerations/pdot-rad_sec2");
  double qdot = model_->GetPropertyValue("accelerations/qdot-rad_sec2");
  double rdot = model_->GetPropertyValue("accelerations/rdot-rad_sec2");
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
  

  residual_time_ += dt_sec;
  // get jsbsim dt
  double jsbsim_dt = fp_body->model_->GetPropertyValue("simulation/dt");

  // Run JSBSim for the residual time
  while (residual_time_ >= jsbsim_dt) {
    // Run JSBSim for one time step
    fp_body->model_->Run();
    residual_time_ -= jsbsim_dt;
  }

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

  const float restitution_impulse = -contact_vel_body.dot(normal_body) *
                                    (1.0f + restitution) /
                                    restitution_impulse_denom;

  // Step 2 - Add restitution impulse to next twist velocities

  kin_with_collision.twist.linear =
      ave_vel_lin + collision_info.normal * restitution_impulse * mass_inv;

  // TODO(edufford) Shouldn't this formula should account for inertia?
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

}  // namespace projectairsim
}  // namespace microsoft