// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

using Newtonsoft.Json;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using UnityProjectAirSim.Config;
using UnityProjectAirSim.Sensors;
using UnityProjectAirSim.Weather;
using UnityEditor;
using UnityEngine;

namespace UnityProjectAirSim.Robot
{
    public class UnityRobot : MonoBehaviour
    {
        string RobotName;
        int SimRobotIndex = -1;
        InteropKinematics RobotKinematics;
        Int64 KinematicsUpdatedTimeStamp = 0;
        Dictionary<string, InteropActuatedTransform> RobotActuatedTransforms =
            new Dictionary<string, InteropActuatedTransform>();
        InteropCollisionInfo CollisionInfo;
        Mutex UpdateMutex = new Mutex();

        delegate bool SetKinematicsDelegate(InteropKinematics kin, Int64 timeStamp);
        SetKinematicsDelegate SetKinematicsDelegateInstance;

        delegate bool SetActuatedTransformDelegate(
            InteropActuatedTransform actuatedTransform, Int64 timeStamp);
        SetActuatedTransformDelegate SetActuatedTransformDelegateInstance;

        private List<GameObject> _linkChildObjects = new List<GameObject>();
        private Dictionary<string, GameObject> _linkChildObjectsDict =
            new Dictionary<string, GameObject>();
        private List<UnityRobotJoint> _joints = new List<UnityRobotJoint>();
        private List<(string, UnitySensor)> _sensors =
            new List<(string, UnitySensor)>();

        private void Awake() { }

        // Start is called before the first frame update
        void Start() { }

        // Update is called once per rendered frame
        void Update() { }

        // FixedUpdate is called once per fixed time step for Unity physics
        // set in the project settings > Time > Fixed Timestep (0.02 sec default)
        void FixedUpdate()
        {
            UpdateMutex.WaitOne();

            {
                // Set UnityRobot's pose to the latest kinematics pose
                transform.position =
                    UnityTransform.NEDToUnityEUN(RobotKinematics.pose.position);
                transform.rotation =
                    UnityTransform.NEDToUnityEUN(RobotKinematics.pose.orientation);
            }

            {
                // Copy KinematicsUpdatedTimeStamp to sensors to match this updated pose
                foreach ((var sensorId, var sensor) in _sensors)
                {
                    if (!sensor) continue;  // non-Unity sensor (e.g. IMU)
                    sensor.PoseUpdatedTimeStamp = KinematicsUpdatedTimeStamp;
                }
            }

            {
                // Apply actuated transforms to their target links
                foreach (KeyValuePair<string, InteropActuatedTransform> actuatedTransform in
                             RobotActuatedTransforms)
                {
                    if (_linkChildObjectsDict.ContainsKey(actuatedTransform.Key))
                    {
                        var unityRobotLink = _linkChildObjectsDict[actuatedTransform.Key].GetComponent<UnityRobotLink>();
                        unityRobotLink.SetActuatedTransform(actuatedTransform.Value);
                    }
                    else
                    {
                        // TODO Log warning
                    }
                }
                // Clear the actuated transforms that have been applied
                RobotActuatedTransforms.Clear();
            }

            UpdateMutex.ReleaseMutex();
        }

        public void Initialize(RobotConfig robotConfig, string robotName,
                               PoseConfig poseConfig)
        {
            RobotName = robotName;
            SimRobotIndex = PInvokeWrapper.GetActorIndex(RobotName);

            RobotKinematics.pose = new InteropPose(
                poseConfig.Position, poseConfig.Rotation.ToQuaternion());
            UnityUtils.InitializePose(gameObject, poseConfig);
            InitializeLinks(robotConfig.Links);
            InitializeJoints(robotConfig.Joints);
            InitializeSensors(robotConfig.Sensors);

            // Connect callback delegates to the instance method of this UnityRobot
            // instance
            SetKinematicsDelegateInstance = this.SetUnityRobotKinematics;
            PInvokeWrapper.SetRobotKinematicsCallback(
                SimRobotIndex,
                Marshal.GetFunctionPointerForDelegate(SetKinematicsDelegateInstance));

            SetActuatedTransformDelegateInstance = this.SetUnityRobotActuatedTransform;
            PInvokeWrapper.SetActuatedTransformCallback(
                SimRobotIndex, Marshal.GetFunctionPointerForDelegate(
                                   SetActuatedTransformDelegateInstance));
        }

        public bool SetUnityRobotKinematics(InteropKinematics kin, Int64 timeStamp)
        {
            UpdateMutex.WaitOne();
            RobotKinematics = kin;
            KinematicsUpdatedTimeStamp = timeStamp;
            UpdateMutex.ReleaseMutex();
            return true;
        }

        public bool SetUnityRobotActuatedTransform(
            InteropActuatedTransform actuatedTransform, Int64 timeStamp)
        {
            UpdateMutex.WaitOne();
            string linkId = Marshal.PtrToStringAnsi(actuatedTransform.actuated_link_id);
            RobotActuatedTransforms[linkId] = actuatedTransform;
            UpdateMutex.ReleaseMutex();
            return true;
        }

        private void InitializeLinks(LinkConfig[] linkConfigs)
        {
            InitializeRootLink(linkConfigs[0]);

            for (int linkIndex = 1; linkIndex < linkConfigs.Length; ++linkIndex)
            {
                var linkObject = UnityUtils.CreateAsChildObject<UnityRobotLink>(
                    linkConfigs[linkIndex].Name, gameObject);
                _linkChildObjects.Add(linkObject);
                _linkChildObjectsDict.Add(linkConfigs[linkIndex].Name, linkObject);

                var linkComponent = linkObject.GetComponent<UnityRobotLink>();
                linkComponent.Initialize(linkConfigs[linkIndex]);
            }
        }

        private void InitializeRootLink(LinkConfig rootLinkConfig)
        {
            var linkComponent = gameObject.AddComponent<UnityRobotLink>();
            linkComponent.Initialize(rootLinkConfig, IsRoot: true);
        }

        private void InitializeJoints(JointConfig[] jointConfigs)
        {
            foreach (var jointConfig in jointConfigs)
            {
                _joints.Add(new UnityRobotJoint(jointConfig));
            }
        }

        private void InitializeSensors(SensorConfig[] sensorConfigs)
        {
            foreach (var sensorConfig in sensorConfigs)
            {
                UnitySensorFactory.CreateSensor(SimRobotIndex, sensorConfig, gameObject,
                                                out var id, out var sensor);
                _sensors.Add((id, sensor));
            }
        }

        void OnCollisionEnter(Collision collision)
        {
            CollisionInfo.has_collided = true;
            CollisionInfo.segmentation_id = -1;
            CollisionInfo.object_name = collision.collider.name;
            UnityTransform.UnityEUNToNED(collision.contacts[0].normal,
                                         ref CollisionInfo.normal);
            UnityTransform.UnityEUNToNED(collision.contacts[0].point,
                                         ref CollisionInfo.impact_point);
            CollisionInfo.position = RobotKinematics.pose.position;
            CollisionInfo.penetration_depth = -collision.contacts[0].separation;
            CollisionInfo.object_name = collision.collider.name;
            CollisionInfo.time_stamp = KinematicsUpdatedTimeStamp;

            PInvokeWrapper.InvokeCollisionDetection(SimRobotIndex, CollisionInfo);
            Debug.Log("OnCollisionEnter: " + collision.collider.name);
        }

        void OnCollisionStay(Collision collision)
        {
            // CollisionInfo.has_collided = true;
            // CollisionInfo.segmentation_id = -1;
            // CollisionInfo.object_name = collision.collider.name;
            // UnityTransform.UnityEUNToNED(collision.contacts[0].normal, ref
            // CollisionInfo.normal);
            // UnityTransform.UnityEUNToNED(collision.contacts[0].point, ref
            // CollisionInfo.impact_point); CollisionInfo.position =
            // RobotKinematics.pose.position; CollisionInfo.penetration_depth =
            // collision.contacts[0].separation; CollisionInfo.object_name =
            // collision.collider.name; CollisionInfo.time_stamp =
            // KinematicsUpdatedTimeStamp;

            // TODO Do something with this new CollisionInfo?
        }

        void OnCollisionExit()
        {
            CollisionInfo.has_collided = false;
            PInvokeWrapper.SetRobotHasCollided(SimRobotIndex, false);
            Debug.Log("OnCollisionExit");
        }
    }
}