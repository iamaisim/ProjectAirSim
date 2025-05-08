// Copyright (C) Microsoft Corporation. All rights reserved.

using System;
using System.Collections.Generic;

using UnityEngine;

using UnityProjectAirSim;
using UnityProjectAirSim.Config;

namespace UnityProjectAirSim.Sensors
{
    public class UnityLidar : UnitySensor
    {
        // --------------- LaserDirection ---------------

        public struct LaserDirection
        {
            public float VerticalAngleDeg;
            public float HorizontalAngleDeg;
            public int Channel;

            public LaserDirection(float inVertAngleDeg, float inHorAngleDeg, int inChannel)
            {
                this.VerticalAngleDeg = inVertAngleDeg;
                this.HorizontalAngleDeg = inHorAngleDeg;
                this.Channel = inChannel;
            }
        }

        // --------------- IScanPattern ---------------

        public abstract class IScanPattern
        {
            public abstract void BeginScan(float simTimeDeltaSec);
            public abstract void EndScan();
            public abstract bool GetLasersToShoot(ref List<LaserDirection> lasersToShoot);
            public abstract void Setup(LidarConfig lidarConfig);
        }

        // --------------- ScanPatternBase ---------------

        public abstract class ScanPatternBase : IScanPattern
        {
            protected float _dSecSimTime = 0.0f;
            protected LidarConfig _lidarConfig;

            public override void BeginScan(float simTimeDeltaSec)
            {
                _dSecSimTime = simTimeDeltaSec;
            }

            public override void EndScan() { }

            public override void Setup(LidarConfig lidarConfig)
            {
                _lidarConfig = lidarConfig;
            }
        }

        // --------------- CylindricalScanPattern ---------------

        public class CylindricalScanPattern : ScanPatternBase
        {
            protected float _angleDistanceOfTickDeg = 0.0f;
            protected float _currentHorizontalAngleDeg = 0.0f;
            protected List<float> _laserAnglesDeg = new List<float>();

            public override void EndScan()
            {
                base.EndScan();

                // Update sensor's current horizontal angle to the end of this tick's sweep
                _currentHorizontalAngleDeg =
                   (_currentHorizontalAngleDeg + _angleDistanceOfTickDeg) % 360.0f;
            }

            public override bool GetLasersToShoot(ref List<LaserDirection> lasersToShoot)
            {
                UInt32 numChannel = (UInt32)_lidarConfig.NumberOfChannels;

                var numPointsPerLaser = (UInt32)(Math.Round(
                    _lidarConfig.PointsPerSecond * _dSecSimTime /
                    (float)(numChannel)));

                if (numPointsPerLaser <= 0)
                {
                    // TODO Log warning
                    return false;
                }

                _angleDistanceOfTickDeg =
                      _lidarConfig.HorizontalRotationFrequency * 360.0f * _dSecSimTime;

                float AngleDistanceOfLaserMeasureDeg = _angleDistanceOfTickDeg / numPointsPerLaser;

                for (int channelIdx = 0; channelIdx < numChannel; ++channelIdx)
                {
                    for (int pointIdxSingleLaser = 0; pointIdxSingleLaser < numPointsPerLaser; ++pointIdxSingleLaser)
                    {
                        float HorizontalAngleDeg =
                          (_currentHorizontalAngleDeg + AngleDistanceOfLaserMeasureDeg * pointIdxSingleLaser) % 360.0f;

                        // Check if the laser is outside the requested horizontal fov
                        if (IsAngleInRange(HorizontalAngleDeg,
                                           _lidarConfig.HorizontalFovStartDeg,
                                           _lidarConfig.HorizontalFovEndDeg) == false)
                        {
                            continue;
                        }

                        LaserDirection Beam = new LaserDirection();
                        Beam.VerticalAngleDeg = _laserAnglesDeg[channelIdx];
                        Beam.HorizontalAngleDeg = HorizontalAngleDeg;
                        Beam.Channel = channelIdx;
                        lasersToShoot.Add(Beam);
                    }
                }

                return true;
            }

            public override void Setup(LidarConfig lidarConfig)
            {
                base.Setup(lidarConfig);

                int numberOfLasers = lidarConfig.NumberOfChannels;

                // Calculate verticle angle distance between each laser
                float deltaAngleDeg = 0.0f;
                if (numberOfLasers > 1)
                {
                    deltaAngleDeg = (lidarConfig.VerticalFovUpperDeg - lidarConfig.VerticalFovLowerDeg) /
                        (float)(numberOfLasers - 1);
                }

                // Store vertical angles for each laser
                _laserAnglesDeg.Clear();
                for (int i = 0; i < numberOfLasers; ++i)
                {
                    float verticalAngleDeg = lidarConfig.VerticalFovUpperDeg - (float)(i) * deltaAngleDeg;
                    _laserAnglesDeg.Add(verticalAngleDeg);
                }
            }
        }

        // --------------- UnityLidar ---------------

        private int _simRobotIndex = -1;
        private int _simSensorIndex = -1;
        private LidarConfig _lidarSettings;
        private Int64 _curSimTime = 0;
        private Int64 _lastSimTime = 0;
        private Int64 _lastSimTimeReport = 0;
        private Int64 _simTimeDeltaReportTarget = 0;
        private List<float> _pointCloud = new List<float>();
        private List<float> _pointCloudPending = new List<float>();
        private List<int> _segmentationCloud = new List<int>();
        private List<int> _segmentationCloudPending = new List<int>();
        private List<float> _intensityCloud = new List<float>();
        private List<float> _intensityCloudPending = new List<float>();
        private List<int> _laserIndexCloud = new List<int>();
        private List<int> _laserIndexCloudPending = new List<int>();
        List<RaycastHit> _groundTruthHits = new List<RaycastHit>();
        private IScanPattern _scanPattern;

        // Start is called before the first frame update
        void Start()
        { }

        // Update is called once per frame
        void Update() { }

        // Do tick in physics FixedUpdate() since line traces are based on physics colliders
        void FixedUpdate()
        {
            _curSimTime = PInvokeWrapper.GetSimTimeNanos();
            var simTimeDeltaSec = (float)((_curSimTime - _lastSimTime) / 1.0E9);
            var simTimeDeltaLastReport = _curSimTime - _lastSimTimeReport;

            // Accumulate new sensor returns
            Simulate(simTimeDeltaSec);

            // Move sensor data to pending report buffers
            _pointCloudPending.AddRange(_pointCloud);
            _pointCloud.Clear();

            _segmentationCloudPending.AddRange(_segmentationCloud);
            _segmentationCloud.Clear();

            _intensityCloudPending.AddRange(_intensityCloud);
            _intensityCloud.Clear();

            _laserIndexCloudPending.AddRange(_laserIndexCloud);
            _laserIndexCloud.Clear();

            // Generate report message according to reporting frequency setting
            if ((_lastSimTimeReport == 0) || (simTimeDeltaLastReport >= _simTimeDeltaReportTarget))
            {
                // Publish lidar data to sim
                int numPoints = _pointCloudPending.Count / 3;

                InteropLidarMessage lidarMessage = new InteropLidarMessage(
                    _curSimTime, numPoints, _pointCloudPending.ToArray(),
                    _segmentationCloudPending.ToArray(),
                    _intensityCloudPending.ToArray(), _laserIndexCloudPending.ToArray(),
                    UnityTransform.UnityEUNToNED(transform));

                PInvokeWrapper.PublishLidarData(_simRobotIndex, _simSensorIndex,
                                                lidarMessage);
                // Debug.Log((float)(_curSimTime / 1E9));

                // Clear reported sensor returns
                _pointCloudPending.Clear();
                _segmentationCloudPending.Clear();
                _intensityCloudPending.Clear();
                _laserIndexCloudPending.Clear();

                // Save report time, adjusting for the difference between actual and target
                // interval to achieve the target reporting interval on average
                {
                    var dSimTime = simTimeDeltaLastReport - _simTimeDeltaReportTarget;

                    if (dSimTime > 0)
                    {
                        if (dSimTime > (2 * _simTimeDeltaReportTarget))
                        {
                            // Limit the time adjustment so it doesn't grow out of control
                            dSimTime = 2 * _simTimeDeltaReportTarget;
                        }

                        _lastSimTimeReport = _curSimTime - dSimTime;
                    }
                }
            }

            _lastSimTime = _curSimTime;
        }

        public void Initialize(int robotIndex, LidarConfig lidarSettings)
        {
            _simRobotIndex = robotIndex;
            _simSensorIndex =
                PInvokeWrapper.GetSensorIndex(robotIndex, lidarSettings.Id);
            _lidarSettings = lidarSettings;

            // TODO Set based on lidar-kind config
            _scanPattern = new CylindricalScanPattern();

            // Normalize
            _lidarSettings.HorizontalFovStartDeg = (360.0f + _lidarSettings.HorizontalFovStartDeg) % 360.0f;
            _lidarSettings.HorizontalFovEndDeg = (360.0f + _lidarSettings.HorizontalFovEndDeg) % 360.0f;

            // Calculate reporting interval from frequency
            _simTimeDeltaReportTarget = (_lidarSettings.ReportFrequency < 0.01f) ? 0 : (Int64)((1.0f / _lidarSettings.ReportFrequency) * 1E9);

            _scanPattern.Setup(_lidarSettings);
        }

        private void Simulate(float simTimeDeltaSec)
        {
            List<LaserDirection> lasersToShoot = new List<LaserDirection>();

            _pointCloud.Clear();
            _segmentationCloud.Clear();
            _groundTruthHits.Clear();

            // 1. Decide lasers to shoot for this tick

            _scanPattern.BeginScan(simTimeDeltaSec);
            if (!_scanPattern.GetLasersToShoot(ref lasersToShoot)) return;

            // 2. Shoot the lasers

            // TODO Use ParallelFor job scheduling
            foreach (var laserDir in lasersToShoot)
            {
                RaycastHit hitInfo;
                bool didHit =
                  ShootSingleLaser(laserDir, transform.position, transform.rotation, out hitInfo);

                if (!didHit) continue;

                // Get segmentation ID of the hit component
                var hitObjName = hitInfo.collider.gameObject.name;
                int segmentationId = CameraFiltersScript.GetSegmentationId(hitObjName);

                // Calculate relative location in world frame from sensor to hit point
                Vector3 pointWorldFrame = hitInfo.point - transform.position;

                // Transform to lidar frame by doing inverse rotation of lidar's
                // rotation from world frame
                Vector3 pointSensorFrame = Quaternion.Inverse(transform.rotation) * pointWorldFrame;

                InteropVector3 pointNed = UnityTransform.UnityEUNToNED(pointSensorFrame);

                _pointCloud.Add(pointNed.x);
                _pointCloud.Add(pointNed.y);
                _pointCloud.Add(pointNed.z);

                _segmentationCloud.Add(segmentationId);
                _intensityCloud.Add(0);  // TODO replace dummy
                _laserIndexCloud.Add(laserDir.Channel);

                if (_lidarSettings.DrawDebugPoints)
                {
                    _groundTruthHits.Add(hitInfo);
                }
            }

            // 3. Optional - draw debug hit points on Unity scene
            if (_lidarSettings.DrawDebugPoints)
            {
                foreach (var hit in _groundTruthHits)
                {
                    Vector3 whisker = Vector3.Normalize(hit.point - transform.position);
                    Debug.DrawLine(hit.point - whisker * 0.2f, hit.point, Color.green, 0.02f, false);
                }
            }

            // 4. Do end of scan
            _scanPattern.EndScan();
        }

        // Assumes that angles are in 0-360 deg range
        public static bool IsAngleInRange(float angle, float startAngle, float endAngle)
        {
            if (startAngle < endAngle)
            {
                return (startAngle <= angle && angle <= endAngle);
            }
            else
            {
                return (startAngle <= angle || angle <= endAngle);
            }
        }

        private bool ShootSingleLaser(LaserDirection laserDir, Vector3 lidarBodyLoc, Quaternion lidarBodyRot, out RaycastHit hitInfo)
        {
            Quaternion laserRotInSensorFrame = Quaternion.Euler(-laserDir.VerticalAngleDeg, laserDir.HorizontalAngleDeg, 0);
            Quaternion laserRotInWorldFrame = lidarBodyRot * laserRotInSensorFrame;
            Vector3 rayDirectionVector = laserRotInWorldFrame * Vector3.forward;
            Vector3 endTrace = lidarBodyLoc + _lidarSettings.Range * rayDirectionVector;

            // Debug.DrawLine(lidarBodyLoc, endTrace, Color.white, 0.02f, false);

            return Physics.Linecast(lidarBodyLoc, endTrace, out hitInfo);
        }

        // void OnDrawGizmos()
        // {
        //     if (_lidarSettings.DrawDebugPoints)
        //     {
        //         foreach (var hit in _groundTruthHits)
        //         {
        //             Gizmos.color = Color.green;
        //             Gizmos.DrawSphere(hit.point, 0.1f);
        //         }
        //     }
        // }

        void OnDestroy()
        {
            // TODO PInvoke.EndSensorUpdate(); (Lidar.EndUpdate())
        }
    }
}
