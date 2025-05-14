// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

using System;
using UnityProjectAirSim;
using UnityProjectAirSim.Config;
using UnityEngine;

namespace UnityProjectAirSim.Sensors
{
    public class UnityCameraGimbal : UnitySensor
    {
        private bool _lockPitch;
        private bool _lockYaw;
        private bool _lockRoll;

        // Start is called before the first frame update
        void Start() { }

        // Update is called once per frame
        void Update()
        {
            float pitch = _lockPitch ? 0.0f : transform.eulerAngles.x;
            float yaw = _lockYaw ? 0.0f : transform.eulerAngles.y;
            float roll = _lockRoll ? 0.0f : transform.eulerAngles.z;
            this.transform.rotation = Quaternion.Euler(pitch, yaw, roll);
        }

        public void Initialize(CameraConfig camSettings)
        {
            if (camSettings.Gimbal != null)
            {
                _lockPitch = camSettings.Gimbal.LockPitch;
                _lockYaw = camSettings.Gimbal.LockYaw;
                _lockRoll = camSettings.Gimbal.LockRoll;
            }
        }
    }
}
