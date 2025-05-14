#!/usr/bin/env python
"""
Serve ONNX model on GPU.
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
"""

import json
from io import BytesIO
from typing import List

import numpy as np
import onnxruntime as ort
import uvicorn
from fastapi import APIRouter, FastAPI, File, UploadFile
from PIL import Image

from projectairsim.autonomy.serving.config import config
from projectairsim.autonomy.serving.models import (
    AerialobstacleDetectorMetadata,
    LandingpadDetectorMetadata,
    PerceptionModelOutput,
)

ort_session_options = ort.SessionOptions()
ort_session_options.enable_profiling = False
ort_session_options.graph_optimization_level = (
    ort.GraphOptimizationLevel.ORT_ENABLE_EXTENDED
)
ort_session_options.optimized_model_filepath = f"{config.model_path}.optimized.onnx"
ort_session = ort.InferenceSession(
    config.model_path,
    providers=config.execution_providers,
    sess_options=ort_session_options,
)

model_metadata = [
    {
        "name": LandingpadDetectorMetadata.name,
        "description": LandingpadDetectorMetadata.description,
        "externalDocs": {
            "description": LandingpadDetectorMetadata.external_docs["description"],
            "url": LandingpadDetectorMetadata.external_docs["url"],
        },
    },
    {
        "name": AerialobstacleDetectorMetadata.name,
        "description": AerialobstacleDetectorMetadata.description,
        "externalDocs": {
            "description": AerialobstacleDetectorMetadata.external_docs["description"],
            "url": AerialobstacleDetectorMetadata.external_docs["url"],
        },
    },
]

router = APIRouter()

# Use FastAPI to serve the ONNX model on GPU
app = FastAPI(
    title="ProjectAirSim:: Autonomy Blocks",
    description="Autonomy Blocks:: Perception:: Model Registry",
    version="0.1.20221101",
    openapi_tags=model_metadata,
)

api = APIRouter()


def preprocess(img_data: bytes) -> np.ndarray:
    """
    Preprocess the image input.
    """
    is_from_sim = False
    try:
        data = json.loads(
            img_data.decode("utf-8")
        )  # data from uploaded file cannot be decoded
        img_np = np.array(data.get("data"), dtype=np.uint8).reshape(
            ((data.get("height"), data.get("width"), 3))
        )  # Format into np array of correct size
        img_rgb = img_np[:, :, ::-1]  # BGR to RGB
        img = Image.fromarray(img_rgb)
        is_from_sim = True
    except UnicodeDecodeError as e:
        print("Image data from file")

    if not is_from_sim:
        img = Image.open(BytesIO(img_data))

    img = img.resize((224, 224))
    img = np.array(img).astype(np.float32)
    img = img.transpose((2, 0, 1))  # HWC to CHW
    img = img / 255.0
    img = img.reshape(1, 3, 224, 224)
    return img


def postprocess(prediction: np.ndarray) -> List[float]:
    """
    Postprocess the prediction output.
    """
    prediction = np.squeeze(prediction)
    return prediction.tolist()


@api.post(
    "/landingpad/",
    response_model=PerceptionModelOutput,
    tags=[LandingpadDetectorMetadata.name],
)
async def predict(input_image: UploadFile = File(...)):
    """
    input_image: pre-processed image of shape (1, 3, 224, 224), type float32
    output: BoundingBox prediction of shape (1, 4), type List[float32]
    """
    image_frame = preprocess(await input_image.read())
    session_input = ort_session.get_inputs()
    prediction = ort_session.run(None, {session_input[0].name: image_frame})
    output = postprocess(prediction)
    return {"prediction": output}


@api.post(
    "/aerialobstacle/",
    response_model=PerceptionModelOutput,
    tags=[AerialobstacleDetectorMetadata.name],
)
async def predict(input_image: bytes = File(...)):
    """
    input_image: pre-processed image of shape (1, 3, 224, 224), type float32
    output: BoundingBox prediction of shape (1, 16), type List[float32]
    """
    image_frame = preprocess(input_image)
    session_input = ort_session.get_inputs()
    prediction = ort_session.run(None, {session_input[0].name: image_frame})
    output = postprocess(prediction)
    return {"prediction": output}


app.include_router(api, prefix="/perception")

if __name__ == "__main__":
    uvicorn.run(app, log_level="info")
