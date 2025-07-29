// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

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