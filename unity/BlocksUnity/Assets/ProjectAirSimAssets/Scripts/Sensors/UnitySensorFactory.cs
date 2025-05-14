// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

using Newtonsoft.Json;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityProjectAirSim.Config;
using UnityEngine;

namespace UnityProjectAirSim.Sensors
{
    public static class UnitySensorFactory
    {
        public static bool CreateSensor(int robotIndex, SensorConfig sensorConfig,
                                        GameObject robotGameObj, out string id,
                                        out UnitySensor sensor)
        {
            id = sensorConfig.Id;
            sensor = null;

            // TODO should use enum. Can we reuse airsim enum?
            switch (sensorConfig.Type)
            {
                case "camera":
                    // TODO attach to parent link instead of robotGameObj
                    // Create a camera gimbal game object and attach to robot
                    var camGimbalObject =
                        UnityUtils.CreateAsChildObject<UnityCameraGimbal>(
                            sensorConfig.Id + "Gimbal", robotGameObj);
                    // Set camera gimbal to same pose as robot
                    camGimbalObject.transform.localPosition = Vector3.zero;
                    camGimbalObject.transform.localRotation = Quaternion.identity;

                    // Create camera game object with a Camera component attached
                    var camObject = UnityUtils.CreateAsChildObject<Camera>(
                        sensorConfig.Id, camGimbalObject);

                    // Add our UnityCamera component
                    var cameraComp =
                        camObject.AddComponent<UnityCamera>();

                    // TODO this should work instead of hack below
                    // cameraComp.Initialize(sensorConfig as CameraConfig);

                    var cameraConfig = JsonConvert.DeserializeObject<CameraConfig>(
                        sensorConfig.JsonString);

                    // Initialize camera gimbal component
                    var camGimbalComp = camGimbalObject.GetComponent<UnityCameraGimbal>();
                    camGimbalComp.Initialize(cameraConfig);

                    // Initialize camera object and component
                    UnityUtils.InitializePose(camObject, cameraConfig.Origin);
                    cameraComp.Initialize(robotIndex, cameraConfig);

                    sensor = cameraComp;
                    return true;

                case "lidar":
                    var lidarObject = UnityUtils.CreateAsChildObject<UnityLidar>(
                        sensorConfig.Id, robotGameObj);

                    var lidarComp =
                        lidarObject.GetComponent<UnityLidar>();

                    var lidarConfig = JsonConvert.DeserializeObject<LidarConfig>(
                        sensorConfig.JsonString);

                    UnityUtils.InitializePose(lidarObject, lidarConfig.Origin);
                    lidarComp.Initialize(robotIndex, lidarConfig);

                    sensor = lidarComp;
                    return true;

                default:
                    Console.WriteLine($"Sensor {sensorConfig.Type} not supported");
                    return false;
            }
        }
    }
}