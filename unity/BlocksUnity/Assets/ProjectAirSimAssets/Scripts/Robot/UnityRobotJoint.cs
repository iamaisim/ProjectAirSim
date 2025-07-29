// Copyright (C) Microsoft Corporation. 
// Copyright (C) 2025 IAMAI CONSULTING CORP

// MIT License. All rights reserved.

using UnityProjectAirSim.Config;

// Really just a stripped down version of JointConfig, just to make it more
// explicit the parameters we do use.
public class UnityRobotJoint
{
    private readonly string _parentLink;
    private readonly string _childLink;

    public UnityRobotJoint(JointConfig jointConfig)
    {
        _parentLink = jointConfig.ParentLink;
        _childLink = jointConfig.ChildLink;
    }
}