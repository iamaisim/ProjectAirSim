// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace UnityProjectAirSim
{
    public static class UnityTransform
    {
        private const float EulerSingularity = 89.9F * Mathf.PI / 180.0F;

        public static Vector3 NEDToUnityEUN(InteropVector3 ned)
        {
            var eun = new Vector3();
            NEDToUnityEUN(ned, ref eun);
            return eun;
        }

        public static InteropVector3 UnityEUNToNED(Vector3 nue)
        {
            var ned = new InteropVector3();
            UnityEUNToNED(nue, ref ned);
            return ned;
        }

        public static Quaternion NEDToUnityEUN(InteropQuaternion ned)
        {
            var eun = new Quaternion();
            NEDToUnityEUN(ned, ref eun);
            return eun;
        }

        public static InteropQuaternion UnityEUNToNED(Quaternion nue)
        {
            var ned = new InteropQuaternion();
            UnityEUNToNED(nue, ref ned);
            return ned;
        }

        public static InteropPose UnityEUNToNED(Transform nue)
        {
            var ned = new InteropPose();
            ned.position = UnityEUNToNED(nue.position);
            ned.orientation = UnityEUNToNED(nue.rotation);
            return ned;
        }

        // By reference equivalents.
        public static void NEDToUnityEUN(InteropVector3 src, ref Vector3 dst)
        {
            dst.Set(src.y, -src.z, src.x);
        }

        public static void UnityEUNToNED(Vector3 src, ref InteropVector3 dst)
        {
            dst.Set(src.z, src.x, -src.y);
        }

        public static void NEDToUnityEUN(InteropQuaternion src, ref Quaternion dst)
        {
            dst.Set(-src.y, src.z, -src.x, src.w);
        }

        public static void UnityEUNToNED(Quaternion src, ref InteropQuaternion dst)
        {
            dst.Set(src.z, -src.x, -src.y, src.w);
        }

        // Expects rotation in radians
        public static InteropQuaternion ToQuaternion(this InteropVector3 rpy)
        {
            return InteropRPYToQuaternion(rpy.x, rpy.y, rpy.z);
        }

        // Expects rotation in degrees
        public static InteropQuaternion ToQuaternionFromDeg(this InteropVector3 rpyDeg)
        {
            return InteropRPYToQuaternion(Mathf.Deg2Rad * rpyDeg.x,
                                          Mathf.Deg2Rad * rpyDeg.y,
                                          Mathf.Deg2Rad * rpyDeg.z);
        }

        private static InteropQuaternion InteropRPYToQuaternion(float rollInRad,
                                                                float pitchInRad,
                                                                float yawInRad)
        {
            // z-y-x rotation convention (Tait-Bryan angles)
            // http://www.sedris.org/wg8home/Documents/WG80485.pdf

            if (Mathf.Abs(pitchInRad) > EulerSingularity)
            {
                // TODO Trigger a warning for clipped pitch angle
                pitchInRad = Mathf.Clamp(pitchInRad, -EulerSingularity, EulerSingularity);
            }

            float t0 = Mathf.Cos(yawInRad * 0.5F);
            float t1 = Mathf.Sin(yawInRad * 0.5F);
            float t2 = Mathf.Cos(rollInRad * 0.5F);
            float t3 = Mathf.Sin(rollInRad * 0.5F);
            float t4 = Mathf.Cos(pitchInRad * 0.5F);
            float t5 = Mathf.Sin(pitchInRad * 0.5F);

            var w = t0 * t2 * t4 + t1 * t3 * t5;
            var x = t0 * t3 * t4 - t1 * t2 * t5;
            var y = t0 * t2 * t5 + t1 * t3 * t4;
            var z = t1 * t2 * t4 - t0 * t3 * t5;

            return new InteropQuaternion(x, y, z, w);
        }
    }
}