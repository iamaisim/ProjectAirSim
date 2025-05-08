"""
Copyright (C) Microsoft Corporation. All rights reserved.
ProjectAirSim:: Autonomy building-blocks::Perception: Backbone selector
"""
from projectairsim.autonomy.models.backbone.resnet_2d3d import (
    resnet18_2d3d_full_C1,
    resnet18_2d3d_full_C2,
    resnet34_2d3d_full,
    resnet18_2d3d_full,
    resnet50_2d3d_full,
)


def select_resnet(network):
    param = {"feature_size": 1024}
    if network == "resnet18":
        model_img = resnet18_2d3d_full(track_running_stats=True)
        model_seg = resnet18_2d3d_full_C1(track_running_stats=True)
        model_depth = resnet18_2d3d_full_C1(track_running_stats=True)
        model_flow = resnet18_2d3d_full_C2(track_running_stats=True)

        param["feature_size"] = 256
    elif network == "resnet34":
        model_img = resnet34_2d3d_full(track_running_stats=True)
        param["feature_size"] = 256
    elif network == "resnet50":
        model_img = resnet50_2d3d_full(track_running_stats=True)
    else:
        raise NotImplementedError

    return model_img, model_seg, model_depth, model_flow, param
