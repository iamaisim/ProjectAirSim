// Copyright (C) Microsoft Corporation. All rights reserved.

using System;
using System.Runtime.InteropServices;

namespace UnityProjectAirSim
{
    public static class PInvokeWrapper
    {
        private const string DLL_NAME = "sim_unity_wrapper";

        [DllImport(DLL_NAME)]
        public static extern bool LoadServer(int topicsPort, int servicesPort);

        [DllImport(DLL_NAME)]
        public static extern bool StartServer();

        [DllImport(DLL_NAME)]
        public static extern void StopServer();

        [DllImport(DLL_NAME)]
        public static extern void SetCallbackLoadExternalScene(
            IntPtr loadUnityScenePtr);

        [DllImport(DLL_NAME)]
        public static extern void SetCallbackStartExternalScene(
            IntPtr startUnityScenePtr);

        [DllImport(DLL_NAME)]
        public static extern void SetCallbackStopExternalScene(
            IntPtr stopUnityScenePtr);

        [DllImport(DLL_NAME)]
        public static extern void SetCallbackUnloadExternalScene(
            IntPtr unloadUnityScenePtr);

        [DllImport(DLL_NAME)]
        public static extern IntPtr GetSceneConfigJSON();

        [DllImport(DLL_NAME)]
        public static extern Int32 GetActorIndex(string actorName);

        [DllImport(DLL_NAME)]
        public static extern Int32 GetSensorIndex(Int32 actorIndex, string sensorId);

        [DllImport(DLL_NAME)]
        public static extern void InvokeCollisionDetection(
            Int32 actorIndex, InteropCollisionInfo collisionInfo);

        [DllImport(DLL_NAME)]
        public static extern void SetRobotKinematicsCallback(
            Int32 actorIndex, IntPtr setUnityRobotKinematicsPtr);

        [DllImport(DLL_NAME)]
        public static extern void SetActuatedTransformCallback(
            Int32 actorIndex, IntPtr setUnityRobotActuatedTransformPtr);

        [DllImport(DLL_NAME)]
        public static extern void SetRobotHasCollided(Int32 actorIndex,
                                                      bool hasCollided);

        [DllImport(DLL_NAME)]
        public static extern void PublishImages(Int32 actorIndex, Int32 sensorIndex,
                                                InteropImageMessage[] imageMessage);

        [DllImport(DLL_NAME)]
        public static extern void AddCameraRequestToCaptureQueue(Int32 actorIndex,
                                                                 Int32 sensorIndex,
                                                                 Int64 capturedTime);

        [DllImport(DLL_NAME)]
        public static extern bool IsCameraCaptureQueueFull(Int32 actorIndex,
                                                           Int32 sensorIndex);

        [DllImport(DLL_NAME)]
        public static extern Int64 GetSimTimeNanos();

        [DllImport(DLL_NAME)]
        public static extern void PublishLidarData(Int32 actorIndex, Int32 sensorIndex,
                                                   InteropLidarMessage lidarMessage);

        [DllImport(DLL_NAME)]
        public static extern IntPtr GetTileKeysToRender(out int size);

        [DllImport(DLL_NAME)]
        public static extern void EcefToNed(double x, double y, double z,
            out double north, out double east, out double down);

        [DllImport(DLL_NAME)]
        public static extern void GetHomeGeoPointInECEF(
            out double x, out double y, out double z);

        [DllImport(DLL_NAME)]
        public static extern IntPtr GetEcefToNeuRotationMatrix();

        [DllImport(DLL_NAME)]
        public static extern bool CameraIsPoseUpdatePending(Int32 actorIndex, Int32 sensorIndex);

        [DllImport(DLL_NAME)]
        public static extern IntPtr CameraGetLookAtObject(Int32 actorIndex, Int32 sensorIndex);

        [DllImport(DLL_NAME)]
        public static extern bool CameraResetPose(Int32 actorIndex, Int32 sensorIndex, bool waitForPoseUpdate);

        [DllImport(DLL_NAME)]
        public static extern void CameraMarkPoseUpdateAsCompleted(Int32 actorIndex, Int32 sensorIndex);

        [DllImport(DLL_NAME)]
        public static extern InteropPose CameraGetDesiredPose(Int32 actorIndex, Int32 sensorIndex);

        #region WorldApiMethods
        [DllImport(DLL_NAME)]
        public static extern void SetSpawnObjectCallback(IntPtr spawnObjectPtr);

        [DllImport(DLL_NAME)]
        public static extern void SetSpawnObjectAtGeoCallback(IntPtr spawnObjectPtr);

        [DllImport(DLL_NAME)]
        public static extern void SetDestroyObjectCallback(IntPtr destroyObjectPtr);

        [DllImport(DLL_NAME)]
        public static extern void SetEnableWeatherVisualEffectsCallback(IntPtr enableWeatherVisualEffectsPtr);

        [DllImport(DLL_NAME)]
        public static extern void SetSetWeatherVisualEffectsParamCallback(IntPtr setWeatherVisualEffectsParamPtr);

        [DllImport(DLL_NAME)]
        public static extern void SetResetWeatherEffectsCallback(IntPtr resetWeatherEffectsPtr);

        [DllImport(DLL_NAME)]
        public static extern void SetSetSunPositionFromDateTimeCallback(IntPtr setSunPositionFromDateTimePtr);

        [DllImport(DLL_NAME)]
        public static extern void SetSetCloudShadowStrengthCallback(IntPtr setCloudShadowStrengthPtr);

        [DllImport(DLL_NAME)]
        public static extern void SetSetSegmentationIDByNameCallback(IntPtr setSegmentationIDByNamePtr);
        #endregion WorldApiMethods
    }
}