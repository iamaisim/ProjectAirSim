"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
ProjectAirSim:: Autonomy building-blocks::Perception: COMPASS
"""
import torch
from torch import nn

from projectairsim.autonomy.models.backbone.select_backbone import select_resnet


class Compass(nn.Module):
    def __init__(self):
        """COMPASS v1

        Args:
            args : Argparse
        """
        super(Compass, self).__init__()

        self.preEncoder, _, _, _, _ = select_resnet("resnet18")

        self.pred = nn.Sequential(
            nn.Linear(256, 128), nn.ReLU(inplace=True), nn.Linear(128, 4)
        )

    def init_weights(self, pretrained_model):
        ckpt = torch.load(pretrained_model)["state_dict"]
        ckpt2 = {}
        for key in ckpt:
            if key.startswith("backbone_rgb"):
                ckpt2[key.replace("backbone_rgb.", "")] = ckpt[key]

        self.preEncoder.load_state_dict(ckpt2)
        print("Loaded pre-trained model.")

    def forward(self, x):
        x = x.unsqueeze(2)
        x = self.preEncoder(x)
        x = x.mean(dim=(2, 3, 4))
        # x = self.fc1(x)
        x = self.pred(x)
        return x
