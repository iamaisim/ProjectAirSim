# Camera Images Post processing using Neural Network models
We are adding support to execute different Neural Networks models natively on the Sim itself to postprocess the scene images from Camera.
Right now, this is aimed at running a pretrained a model to extract information like object recognition, semantic segmentation etc...

## Pre-Requisites
Currently, we support execution on the following inferencing runtimes.
1. CUDA (See CUDA requirements for details.)
2. Plain CPU

You can enable or disable CUDA support from the config option. See sample config for an example. TensorRT is not supported right but will be added in the future versions. 

### Model Format
Only ONNX models are currently supported.

### Neural Network Input and Output structures
Currently, we only publish uint8 as our output with Pub/Sub model. So output data is expected to be uint8/int. 
1. The input layer:
    - Float 
    - Dimensions of the same size as the scene camera - by default this is 400x225x3. 
2. The output layer:
    - uint8/int
    - Output will be packaged a simple vector and published. 

### CUDA Requirements
To enable CUDA, you need to have the following installed on your machine locally. 
1. CUDA 11.0 or greater
2. CUDNN associated with the CUDA version of your choice. 

### TensorRT Requirments
1. CUDA 11.0 or greater
2. CUDNN associated with CUDA above. (Required as a fallback for when TensorRT does not support some of the operations.)
2. TensorRT library associated with the CUDA version of your choice.
3. TensorRT needs the neural network model to contain shape inference information in the onnx file. See here https://onnxruntime.ai/docs/execution-providers/TensorRT-ExecutionProvider.html#shape-inference-for-tensorrt-subgraphs 

# Image post processing settings

## Sample config: DownCamera Sensor configured with post processing model
Scroll down to see post-process-model-settings since they are at the bottom.

      {
        "id": "DownCamera",
        "type": "camera",
        "enabled": true,
        "parent-link": "Frame",
        "capture-interval": 0.001,
        "capture-settings": [
          {
            "image-type": 0,
            "width": 400,
            "height": 225,
            "fov-degrees": 90,
            "capture-enabled": true,
            "streaming-enabled": false,
            "pixels-as-float": false,
            "compress": false,
            "target-gamma": 2.5
          },
          {
            "image-type": 1,
            "width": 400,
            "height": 225,
            "fov-degrees": 90,
            "capture-enabled": false,
            "streaming-enabled": false,
            "pixels-as-float": false,
            "compress": false
          },
          {
            "image-type": 2,
            "width": 400,
            "height": 225,
            "fov-degrees": 90,
            "capture-enabled": true,
            "streaming-enabled": false,
            "pixels-as-float": false,
            "compress": false
          },
          {
            "image-type": 3,
            "width": 400,
            "height": 225,
            "fov-degrees": 90,
            "capture-enabled": true,
            "streaming-enabled": false,
            "pixels-as-float": false,
            "compress": false
          }
        ],
        "noise-settings": [
          {
            "enabled": false,
            "image-type": 1,
            "rand-contrib": 0.2,
            "rand-speed": 100000.0,
            "rand-size": 500.0,
            "rand-density": 2,
            "horz-wave-contrib": 0.03,
            "horz-wave-strength": 0.08,
            "horz-wave-vert-size": 1.0,
            "horz-wave-screen-size": 1.0,
            "horz-noise-lines-contrib": 1.0,
            "horz-noise-lines-density-y": 0.01,
            "horz-noise-lines-density-xy": 0.5,
            "horz-distortion-contrib": 1.0,
            "horz-distortion-strength": 0.002
          }
        ],
        "origin": {
          "xyz": "1.1 2.2 -3.3",
          "rpy-deg": "0.0 -85.94 0.0"
        },
        "post-process-model-settings": {
          "enabled": true,
          "model-filepath": "C:/repos/models/fcn-resnet50-12-int8.onnx",
          "execution-provider": "cpu"
        }
      },


# Post processing model settings

Only the post processing settings are documented here. For other general camera settings, see other camera pages. 

| Parameter | Value | Description |
| --------- | ----- | ----------- |
| `enabled` | bool | Whether post processing is enabled. |
| `execution-provider` | string | Default is "cpu". Use "cuda" or "tensorrt" to execute using CUDA/TensorRT execution provider. This might speed up the execution process. See pre-reqs for enabling CUDA or TensorRT. |
| `model-filepath` | string | Absolute path to the model file to execute on the sim. Currently needs to be an ONNX  model. |


