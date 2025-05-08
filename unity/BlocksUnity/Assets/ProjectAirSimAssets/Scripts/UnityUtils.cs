// Copyright (C) Microsoft Corporation. All rights reserved.

using UnityProjectAirSim.Config;
using UnityEngine;

namespace UnityProjectAirSim
{
    internal static class UnityUtils
    {
        internal static GameObject CreateAsChildObject<ComponentType>(
            string name, GameObject parentObj)
        {
            var childObj = new GameObject(name, typeof(ComponentType));
            childObj.transform.SetParent(parentObj.transform);
            return childObj;
        }

        internal static void InitializePose(GameObject gameObject,
                                            PoseConfig poseConfig)
        {
            if (poseConfig != null)  // TODO: the config classes should be in charge of
                                     // default value handling
            {
                gameObject.transform.localPosition =
                    UnityTransform.NEDToUnityEUN(poseConfig.Position);

                if (poseConfig.RotationDeg != InteropVector3.Zero)
                {
                    gameObject.transform.localRotation =
                        UnityTransform.NEDToUnityEUN(
                            poseConfig.RotationDeg.ToQuaternionFromDeg());
                }
                else
                {
                    gameObject.transform.localRotation =
                        UnityTransform.NEDToUnityEUN(
                            poseConfig.Rotation.ToQuaternion());
                }
            }
        }

        internal static void InitializePose(GameObject gameObject,
                                            InteropPose pose)
        {
            gameObject.transform.localPosition =
                UnityTransform.NEDToUnityEUN(pose.position);

            gameObject.transform.localRotation =
                UnityTransform.NEDToUnityEUN(pose.orientation);
        }
    }
}