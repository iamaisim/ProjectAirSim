// Copyright (C) Microsoft Corporation. All rights reserved.

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