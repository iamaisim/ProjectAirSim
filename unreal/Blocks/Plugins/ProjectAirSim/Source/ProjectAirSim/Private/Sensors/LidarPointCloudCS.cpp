// Copyright (C) Microsoft Corporation.  All rights reserved.

#include "LidarPointCloudCS.h"

IMPLEMENT_GLOBAL_SHADER(FLidarPointCloudCS,
                        "/CustomShaders/LidarPointCloudCS.usf",
                        "MainComputeShader", SF_Compute);