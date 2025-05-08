// Copyright (C) Microsoft Corporation.  All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UnrealHelpers.h"
#include "core_sim/math_utils.hpp"
#include "core_sim/transforms/transform.hpp"
#include "core_sim/transforms/transform_utils.hpp"

class UnrealTransform {
 public:
  static microsoft::projectairsim::Vector3 UnrealToNedLinear(
      const FVector& vec_unreal) {
    microsoft::projectairsim::Vector3 vec_ned =
        microsoft::projectairsim::TransformUtils::NeuToNedLinear(
            microsoft::projectairsim::TransformUtils::ToMeters(
                UnrealHelpers::ToVector3(vec_unreal)));
    return vec_ned;
  }

  static FVector NedToUnrealLinear(
      const microsoft::projectairsim::Vector3& vec_ned) {
    FVector vec_unreal = UnrealHelpers::ToFVector(
        microsoft::projectairsim::TransformUtils::NedToNeuLinear(
            microsoft::projectairsim::TransformUtils::ToCentimeters(vec_ned)));
    return vec_unreal;
  }

  static microsoft::projectairsim::Transform ToGlobalNed(
      const FTransform& transform_neu) {
    microsoft::projectairsim::Vector3 position_ned =
        UnrealToNedLinear(transform_neu.GetLocation());

    microsoft::projectairsim::Quaternion rotation_ned =
        microsoft::projectairsim::TransformUtils::NeuToNedQuat(
            UnrealHelpers::ToQuaternion(transform_neu.GetRotation()));

    return microsoft::projectairsim::Transform(position_ned, rotation_ned);
  }

  static FTransform FromGlobalNed(
      const microsoft::projectairsim::Transform& transform_ned) {
    FQuat rotation_neu = UnrealHelpers::ToFQuat(
        microsoft::projectairsim::TransformUtils::NedToNeuQuat(
            transform_ned.rotation_));

    FVector position_neu = NedToUnrealLinear(transform_ned.translation_);

    return FTransform(rotation_neu, position_neu);
  }

  static microsoft::projectairsim::Transform GetPoseNed(
      const USceneComponent* scene_component) {
    if (scene_component == nullptr) return microsoft::projectairsim::Transform();

    microsoft::projectairsim::Transform Pose =
        UnrealTransform::ToGlobalNed(scene_component->GetComponentTransform());

    return Pose;
  }
};
