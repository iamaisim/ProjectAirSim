// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.
// UnrealLidarSensor implementation that uses Ray Tracing in Unreal.
// The implementation uses a model similar to CARLA Lidar implementation
// (https://carla.readthedocs.io/en/latest/ref_sensors/#lidar-raycast-sensor)

#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "UnrealSensor.h"
#include "core_sim/clock.hpp"
#include "core_sim/sensors/lidar.hpp"
#include "core_sim/transforms/transform_utils.hpp"

// comment so that generated.h is always the last include file with clang-format
#include "UnrealLidar.generated.h"

UCLASS() class UUnrealLidar : public UUnrealSensor {
  GENERATED_BODY()

 public:
  explicit UUnrealLidar(const FObjectInitializer& ObjectInitializer);

  void Initialize(const microsoft::projectairsim::Lidar& Lidar);

  void TickComponent(float DeltaTime, ELevelTick TickType,
                     FActorComponentTickFunction* ThisTickFunction) override;

  void SetupLidarFromSettings(
      const microsoft::projectairsim::LidarSettings& LidarSettings);

 protected:
  void BeginPlay() override;
  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

 private:
  struct LaserDirection {
    float VerticalAngleDeg = 0;
    float HorizontalAngleDeg = 0;
    int Channel = 0;
    float OffsetDistanceVertical =
        0;  // distance by which the beam is offset from the component position
  };

  // Lidar scan pattern interface.  Objects with this interface generate
  // laser beam directions from which the scene is sampled to create the
  // lidar sensor returns.
  class IScanPattern {
   public:
    virtual ~IScanPattern() {}

    virtual void BeginScan(float SimTimeDeltaSec) = 0;
    virtual void EndScan(void) = 0;
    virtual bool GetLasersToShoot(
        TArray<LaserDirection>* PLasersToShootRet) = 0;
    virtual void Setup(
        const microsoft::projectairsim::LidarSettings& LidarSettings) = 0;
  };  // class IScanPattern

  // Scan pattern base class.
  class ScanPatternBase : public IScanPattern {
   public:
    ScanPatternBase(void) : IScanPattern() {}

    void BeginScan(float SimTimeDeltaSec) override {
      DSecSimTime_ = SimTimeDeltaSec;
    }
    void EndScan(void) override {}
    void Setup(
        const microsoft::projectairsim::LidarSettings& LidarSettings) override;

   protected:
    float DSecSimTime_ = 0.0f;  // Time passed since last scan update
                                // (seconds)
    microsoft::projectairsim::LidarSettings
        LidarSettings_;  // Lidar sensor settings
  };                     // class ScanPatternBase

  // Cylindrical scan pattern.  Each laser beam traces a horizontal flat cone
  // with respect to the sensor.  The number of channels specify the number
  // of lasers which are spaced vertically to equally divide the vertical FOV.
  class CylindricalScanPattern : public ScanPatternBase {
   public:
    CylindricalScanPattern(void) : ScanPatternBase() {}

    void EndScan(void) override;
    bool GetLasersToShoot(TArray<LaserDirection>* PLasersToShootRet) override;
    void Setup(
        const microsoft::projectairsim::LidarSettings& LidarSettings) override;

   protected:
    float AngleDistanceOfTickDeg_ = 0.0f;
    float CurrentHorizontalAngleDeg_ =
        0.0f;  // Current horizontal angle of the scan pass
    std::vector<float> LaserAnglesDeg_;  // Vertical angles for each laser
  };                                     // class CylindricalScanPattern

  // Scan pattern based on hypotrochoid curve: a roulette where a circle
  // rolls along the interior of a larger fixed circle with the curve traced
  // by a point attached to the rolling circle.
  //
  // The polar coordinates of the points sampled on the roulette are used to
  // generate the horizontal and vertical angles of the sensor's beam(s).
  // The polar angle is the horizontal angle of the beam from the Lidar
  // sensor.  The polar radius is mapped to the vertical FOV:  A radius of 0
  // corresponds to the lower vertical FOV angle while a radius of 1
  // corresponds to the upper vertical FOV angle.
  //
  // This implementation is more general by disconnecting the rotation of the
  // rolling circle from its position on the fixed circle.  The tracing point
  // is fixed at radius 0.5 from the rolling circle center which is at radius
  // 0.5 from the fixed circle center so that the roulette curve's polar
  // radius has a range of [0, 1].
  class RosetteScanPattern : public ScanPatternBase {
   public:
    RosetteScanPattern(void);

    void EndScan(void) override;
    bool GetLasersToShoot(TArray<LaserDirection>* PLasersToShootRet) override;
    void Setup(
        const microsoft::projectairsim::LidarSettings& LidarSettings) override;

   protected:
    float degree_major_ =
        0.0f;  // Current angle parameter for major circle (degrees)
    float degree_minor_ =
        0.0f;  // Current angle parameter for minor circle (degrees)
    float degree_vertical_min_ =
        -90.0f;  // Minimum vertical scan position (degrees, inclusive)
    float degree_vertical_range_ = 80.0f;  // Vertical scan range (degrees)
    float degree_per_sec_major_ =
        360.0f * 10.0f *
        3.0f;  // Major circle scan cycle rate (degrees per second)
    float degree_per_sec_minor_ =
        360.0f * 10.0f * 19.0f /
        3.0f;  // Minor circle scan cycle rate (degrees per second)
    float ddegree_scan_major_ = 0.0f;  // Change in major circle angle parameter
                                       // for the current scan update
    float ddegree_scan_minor_ = 0.0f;  // Change in minor circle angle parameter
                                       // for the current scan update
    float scaling_radial_ = 1.0f;      // Scaling of minor circle in radial
                                       // direction
    float distance_between_lasers_ = 0.1f;  // offset of individual lasers
  };                                        // class RosetteScanPattern

 private:
  void InitializePose(const microsoft::projectairsim::Transform& Pose);

  FHitResult ShootSingleLaser(const LaserDirection& LaserDir,
                              const FVector& LidarBodyLoc,
                              const FRotator& LidarBodyRot);

  void Simulate(const float DeltaTime);

 private:
  static bool IsAngleInRange(float Angle, float StartAngle, float EndAngle);

 private:
  static constexpr int NoSegmentationID = -1;  // Value for no segmentation ID

 private:
  bool bScanOrientationIsDefault =
      true;                           // If true, don't apply ScanOrientationRot
  std::vector<float> IntensityCloud;  // Return intensity data
  std::vector<float>
      IntensityCloudPending;  // Return intensity data waiting to be reported
  std::vector<int> LaserIndexCloud;
  std::vector<int>
      LaserIndexCloudPending;      // Return index data waiting to be reported
  TimeNano LastSimTime = 0;        // Simulation time of last sensor update
  TimeNano LastSimTimeReport = 0;  // Simulation time of last sensor report
  microsoft::projectairsim::Lidar Lidar;  // Corresponding sim sensor object
  AActor* OwnerActor;                     // Owning engine actor
  std::vector<float> PointCloud;  // Sensor returns from current simulation pass
  std::vector<bool> ReturnCloud; // Marks if points were returned for each index
  std::vector<float>
      PointCloudPending;  // Sensor returns waiting to be reported
  std::vector<float> AzimuthElevationRangeCloud;  // Sensor returns from current
                                                  // simulation pass
  std::vector<float>
      AzimuthElevationRangeCloudPending;  // Sensor returns waiting to be
                                          // reported
  std::unique_ptr<IScanPattern> PScanPattern =
      nullptr;  // Current object implementing the laser scan pattern
  FRotator ScanOrientationRot;  // Additional rotation of laser direction per
                                // scan orientation axis
  std::vector<int>
      SegmentationCloud;  // Segmentation ID's corresponding to point cloud from
                          // current simulation pass
  std::vector<int>
      SegmentationCloudPending;  // Segmentation ID's corresponding to point
                                 // cloud waiting to be reported
  microsoft::projectairsim::LidarSettings Settings;  // Sensor settings
  TimeNano SimTimeDeltaReportTarget =
      0;  // Desired sensor report interval (0 = as soon as possible)

  bool ReportPointCloud = true;
  bool ReportAzimuthElevationRange = false;
};
