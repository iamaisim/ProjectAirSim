import os
from dataclasses import dataclass
from typing import List


@dataclass
class Config:
    title: str
    description: str
    model_path: str
    version: str
    execution_providers: List[str]


main = {
    "title": "ProjectAirSim:: Autonomy Blocks",
    "description": "Autonomy Blocks:: Perception:: Model Registry",
    # Path to the directory where the model is stored.
    "model_path": os.getenv(
        "MODEL_PATH",
        r"pretrained-3daod-model.onnx",
    ),
    "version": "0.1.20221101",
    "execution_provider": [
        "CUDAExecutionProvider"
    ],  # 'CPUExecutionProvider' is not supported,
}

config = Config(
    main["title"],
    main["description"],
    main["model_path"],
    main["version"],
    main["execution_provider"],
)
