// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License.

using System;

using UnityEngine;

namespace UnityProjectAirSim.Sensors
{
    public class UnitySensor : MonoBehaviour
    {
        public Int64 PoseUpdatedTimeStamp
        {
            get;
            set;
        }
        = 0;
    }
}