// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// Sensorfactory that is responsible for creating sensors like UUnrealCamera,
// UUnrealImu, etc.

#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "CoreMinimal.h"
#include "GPULidar.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "UnrealCamera.h"
#include "UnrealDistanceSensor.h"
#include "UnrealLidar.h"
#include "UnrealLogger.h"
#include "UnrealRadar.h"
#include "UnrealScene.h"
#include "UnrealSensor.h"
#include "UnrealTransforms.h"
#include "core_sim/sensors/camera.hpp"
#include "core_sim/sensors/sensor.hpp"

class PROJECTAIRSIM_API UnrealSensorFactory {
 public:
  static std::pair<std::string, UUnrealSensor*> CreateSensor(
      const microsoft::projectairsim::Sensor& SimSensor,
      USceneComponent* Parent,
      AUnrealScene* UnrealScene) {
    UnrealLogger::Log(microsoft::projectairsim::LogLevel::kTrace,
                      TEXT("[UnrealSensorFactory] Creating Sensor '%S'."),
                      SimSensor.GetId().c_str());

    auto Id = SimSensor.GetId();
    UUnrealSensor* Sensor = nullptr;

    if (SimSensor.GetType() == microsoft::projectairsim::SensorType::kCamera) {
      UnrealLogger::Log(
          microsoft::projectairsim::LogLevel::kTrace,
          TEXT("[UnrealSensorFactory] Creating CameraSensor '%S'."),
          SimSensor.GetId().c_str());

      auto& sim_camera = (microsoft::projectairsim::Camera&)SimSensor;
      USceneComponent* CameraParent = Parent;

      auto cam_settings = sim_camera.GetCameraSettings();
      // If gimbal settings are used, attach a spring arm.
      if (cam_settings.gimbal_setting.lock_pitch ||
          cam_settings.gimbal_setting.lock_roll ||
          cam_settings.gimbal_setting.lock_yaw) {
        USpringArmComponent* SpringArm =
            NewObject<USpringArmComponent>(Parent, TEXT("SpringArm0"));
        // Spring arm attaches directly to parent link with zero arm length,
        // since it is only used for stabilizing rotations.
        SpringArm->AttachToComponent(
            Parent, FAttachmentTransformRules::SnapToTargetIncludingScale);

        SpringArm->TargetArmLength = 0.f;
        SpringArm->bInheritPitch = !cam_settings.gimbal_setting.lock_pitch;
        SpringArm->bInheritRoll = !cam_settings.gimbal_setting.lock_roll;
        SpringArm->bInheritYaw = !cam_settings.gimbal_setting.lock_yaw;
        // TODO Consider making custom chase cam following logic instead of
        // relying on USpringArmComponent to handle VTOL orientations.

        // Camera lag causes stuttering because
        // it is based on UE DeltaTime which is not synchronized with the robot
        // motion calculated by sim time, so it should be disabled.
        SpringArm->bEnableCameraLag = false;
        SpringArm->bEnableCameraRotationLag = false;
        SpringArm->RegisterComponent();

        CameraParent = SpringArm;
      }

      auto camera = NewObject<UUnrealCamera>(CameraParent, Id.c_str());
      camera->Initialize(sim_camera, UnrealScene);
      camera->AttachToComponent(
          CameraParent, FAttachmentTransformRules::KeepRelativeTransform);

      // Apply camera origin offset from the parent (either the parent link or
      // the spring arm attached at the parent link if gimbal rotation
      // stabilization is configured)
      camera->SetRelativePoseFromNed(cam_settings.origin_setting);

      Sensor = camera;
    } else if (SimSensor.GetType() ==
               microsoft::projectairsim::SensorType::kImu) {
      UnrealLogger::Log(microsoft::projectairsim::LogLevel::kTrace,
                        TEXT("[UnrealSensorFactory] Received request to create "
                             "IMU Sensor '%S'."),
                        SimSensor.GetId().c_str());
    } else if (SimSensor.GetType() ==
               microsoft::projectairsim::SensorType::kAirspeed) {
      UnrealLogger::Log(microsoft::projectairsim::LogLevel::kTrace,
                        TEXT("[UnrealSensorFactory] Received request to create "
                             "Airspeed Sensor '%S'."),
                        SimSensor.GetId().c_str());
    } else if (SimSensor.GetType() ==
               microsoft::projectairsim::SensorType::kBarometer) {
      UnrealLogger::Log(microsoft::projectairsim::LogLevel::kTrace,
                        TEXT("[UnrealSensorFactory] Received request to create "
                             "Barometer Sensor '%S'."),
                        SimSensor.GetId().c_str());
    } else if (SimSensor.GetType() ==
               microsoft::projectairsim::SensorType::kMagnetometer) {
      UnrealLogger::Log(microsoft::projectairsim::LogLevel::kTrace,
                        TEXT("[UnrealSensorFactory] Received request to create "
                             "Magnetometer Sensor '%S'."),
                        SimSensor.GetId().c_str());
    } else if (SimSensor.GetType() ==
               microsoft::projectairsim::SensorType::kLidar) {
      UnrealLogger::Log(
          microsoft::projectairsim::LogLevel::kTrace,
          TEXT("[UnrealSensorFactory] Creating LidarSensor '%S'."),
          SimSensor.GetId().c_str());
      auto& sim_lidar =
          static_cast<const microsoft::projectairsim::Lidar&>(SimSensor);

      if (sim_lidar.GetLidarSettings().lidar_kind ==
          microsoft::projectairsim::LidarKind::kGPUCylindrical) {
        auto lidar = NewObject<UGPULidar>(Parent, Id.c_str());
        lidar->Initialize(sim_lidar);
        lidar->AttachToComponent(
            Parent, FAttachmentTransformRules::KeepRelativeTransform);
        Sensor = lidar;
      } else {
        auto lidar = NewObject<UUnrealLidar>(Parent, Id.c_str());
        lidar->Initialize(sim_lidar);
        lidar->AttachToComponent(
            Parent, FAttachmentTransformRules::KeepRelativeTransform);
        Sensor = lidar;
      }
    } else if (SimSensor.GetType() ==
               microsoft::projectairsim::SensorType::kDistanceSensor) {
      UnrealLogger::Log(
          microsoft::projectairsim::LogLevel::kTrace,
          TEXT("[UnrealSensorFactory] Creating DistanceSensor '%S'."),
          SimSensor.GetId().c_str());
      auto& sim_distance_sensor =
          static_cast<const microsoft::projectairsim::DistanceSensor&>(SimSensor);
      auto distance_sensor =
          NewObject<UUnrealDistanceSensor>(Parent, Id.c_str());
      distance_sensor->Initialize(sim_distance_sensor);
      distance_sensor->AttachToComponent(
          Parent, FAttachmentTransformRules::KeepRelativeTransform);
      Sensor = distance_sensor;
    } else if (SimSensor.GetType() ==
               microsoft::projectairsim::SensorType::kDistanceSensor) {
      UnrealLogger::Log(
          microsoft::projectairsim::LogLevel::kTrace,
          TEXT("[UnrealSensorFactory] Creating DistanceSensor '%S'."),
          SimSensor.GetId().c_str());
      auto& sim_distance_sensor =
          static_cast<const microsoft::projectairsim::DistanceSensor&>(SimSensor);
      auto distance_sensor =
          NewObject<UUnrealDistanceSensor>(Parent, Id.c_str());
      distance_sensor->Initialize(sim_distance_sensor);
      distance_sensor->AttachToComponent(
          Parent, FAttachmentTransformRules::KeepRelativeTransform);
      Sensor = distance_sensor;
    } else if (SimSensor.GetType() ==
               microsoft::projectairsim::SensorType::kRadar) {
      UnrealLogger::Log(
          microsoft::projectairsim::LogLevel::kTrace,
          TEXT("[UnrealSensorFactory] Creating RadarSensor '%S'."),
          SimSensor.GetId().c_str());
      auto& sim_radar =
          static_cast<const microsoft::projectairsim::Radar&>(SimSensor);
      auto radar = NewObject<UUnrealRadar>(Parent, Id.c_str());
      radar->Initialize(sim_radar);
      radar->AttachToComponent(
          Parent, FAttachmentTransformRules::KeepRelativeTransform);
      Sensor = radar;
    } else if (SimSensor.GetType() ==
               microsoft::projectairsim::SensorType::kGps) {
      UE_LOG(SimPlugin, Log,
             TEXT("Received request to create GPS Sensor '%S'."),
             SimSensor.GetId().c_str());
    } else if (SimSensor.GetType() ==
               microsoft::projectairsim::SensorType::kBattery) {
      UE_LOG(SimPlugin, Log,
             TEXT("Received request to create Battery Sensor '%S'."),
             SimSensor.GetId().c_str());
    } else {
      UnrealLogger::Log(
          microsoft::projectairsim::LogLevel::kError,
          TEXT("[UnrealSensorFactory] Unsupported sensor type '%S'."),
          SimSensor.GetId().c_str());
      throw;
    }
    return {Id, Sensor};
  }
};
