#!/usr/bin/env python
"""
Warmup script for ONNX model on GPU.
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
"""

import numpy as np
import onnxruntime as ort

from projectairsim.autonomy.serving.config import config


def setup_onnx():
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
    return ort_session


def warmup_onnx(ort_session: ort.InferenceSession):
    session_input = ort_session.get_inputs()
    test_input = np.random.rand(*session_input[0].shape).astype(np.float32)
    for _ in range(10):
        _ = ort_session.run(None, {session_input[0].name: test_input})


if __name__ == "__main__":
    print("Warming up ONNX RunTime on GPU...")
    ort_session = setup_onnx()
    warmup_onnx(ort_session)
    print("Finished.")
