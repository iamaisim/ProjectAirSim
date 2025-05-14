"""
Model interface defininitions for Autonomy Blocks
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
"""

from dataclasses import dataclass
from typing import Dict, List

from fastapi import File, UploadFile
from pydantic import BaseModel


@dataclass
class ModelMetadata:
    """Model metadata."""

    name: str
    description: str
    # externalDocs is a Dict with 'url' and 'description'.
    external_docs: Dict[str, str]


class PerceptionModelInput(BaseModel):
    """
    Input for RGB-camera-based perception models.
    """

    input_image: UploadFile = File(...)


class PerceptionModelOutput(BaseModel):
    """
    Output for RGB-camera-based perception models.
    BoundingBox prediction of shape (1, 4), type List[float32]
    """

    prediction: List[float]


LandingpadDetectorMetadata = ModelMetadata(
    name="2DLPD:: landingpad_detector",
    description="Autonomy Blocks landing pad detector model. [See Data Card](http://localhost:8501/#data-card) and [Model Card](http://localhost:8501/#model-card).",
    external_docs={
        "url": "https://aka.ms/airsim",
        "description": "Read more about this model:https://aka.ms/airsim",
    },
)

AerialobstacleDetectorMetadata = ModelMetadata(
    name="3DAOD:: aerialobstacle_detector",
    description="Autonomy Blocks aerial object detector model. [See Data Card](http://localhost:8501/#data-card) and [Model Card](http://localhost:8501/#model-card).",
    external_docs={
        "url": "https://aka.ms/airsim",
        "description": "Read more about this model:https://aka.ms/airsim",
    },
)
