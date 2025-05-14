// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#include "LidarPointCloudCS.h"

IMPLEMENT_GLOBAL_SHADER(FLidarPointCloudCS,
                        "/CustomShaders/LidarPointCloudCS.usf",
                        "MainComputeShader", SF_Compute);