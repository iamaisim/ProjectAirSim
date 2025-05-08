# Autonomy Building Blocks

The Autonomy module of Project AirSim, aims to provide [pre-trained models](#pre-trained-models), fine-tuning scripts, [RL Gym environments](#rl-gym-environments), [data collection scripts](#2-data-collection) and [sample/demo](#3-sample-applications) applications for you to try out and leverage to build custom models for your autonomy needs and use-cases.
The pre-trained model allow you to develop efficient, scenario/task-specific models with relatively less amount of new data from your scenarios.

## Pre-trained Models

Ready-to-use/ready-to-finetune neural network models for your applications.

### 1. Perception

Camera sensor-based visual processing models for the object detection. Object detection include tasks like detecting landing pads, drones, vehicles, people, trees, builoing, power poles etc.
The current release of the autonomy module contains pre-trained models for the following perception tasks: 1. Landing pad detection

#### 1.1 Landing pad detection

The landing pad detection model detects and localizes a landing pad on the image captured by a downward-facing camera mounted on a drone flying in Project AirSim. The pre-trained model by default is trained to detect the `BasicLandingPad` that is available as part of the Project AirSim Sim package assets.
The visual of the `BasicLandingPad` is shown below:

![BasicLandingPad](../images/autonomy/basiclandingpad.png)

The pre-trained model is capable of detecting the landing pad under varying environmental  conditions like snow, rain and typical variations in lighting. Sample predictions are shown below for reference:

| ![](../images/autonomy/basiclandingpad-perception-model-output-set1.jpg) | ![](../images/autonomy/basiclandingpad-perception-model-output-set2.jpg) | ![](../images/autonomy/basiclandingpad-perception-model-output-set3.jpg)|
|------------------------------------------------------------|------------------------------------------------------------|------------------------------------------------------------|

##### 1.1.1 Running the landing pad detection demo

The landing pad prediction demo script uses a pre-trained model to predict the bounding boxes around the landing pads present in the input set of images. You will need a pre-trained model and a directory containing a sample set of images to run this demo.

Please follow the steps in the [ProjectAirSim Autonomy Setup](setup.md) documentation page to setup your python environment before proceeding to the next steps below.

1. Obtain the latest pre-trained model and save it to your local disk, for example at: `client/python/example_user_scripts/autonomy/pretrained-basiclandingpad-perception-model.pth.tar`

1. Obtain a set of test images with the `BasicLandingPad` to run this demo. You can use the images in [client/python/example_user_scripts/autonomy/input_images](input_images) for the demo.

1. Run the `landingpad_predictor` demo script with the `airsim-venv` python environment activated: `(airsim-venv)$ cd  client/python/example_user_scripts/autonomy && python landingpad_predictor.py --images=input_images --pretrained-model=pretrained-basiclandingpad-perception-model.pth.tar --interactive`

The above demo script will interactively display the predicted bounding box on the input RGB image and wait for your keyboard key press to advance to the next image. You can also run the above script without the `--interactive` argument to have the predictions saved to the `client/python/example_user_scripts/autonomy/outputs/` directory.

##### 1.1.2 Workflow for building your own landing pad detection models:

1. Obtain the latest pre-trained model and save it to your local disk, for example at: `client/python/example_user_scripts/autonomy/pretrained-basiclandingpad-perception-model.pth.tar`

1. Collect new data for your scenario (e.g: your custom landing pad asset) using the [Data Collector](fix-me)

1. Fine-tune/customize the pre-trained model to work for your scenario using the [Finetuning script](fix-me)

We currently offer pre-trained models for the landing pad detection task. Pre-trained models for additional tasks and updated models for existing tasks will be made available thourgh our upcoming releases.

### 2. Data collection

Sample data collection scripts and workflow.

*Coming soon! Stay tuned for the next release*


### 3. Sample Applications

1. Using a perception module for visual Take-off and landing

This application allows a drone equipped with a single, downward-facing RGB color camera to precisely detect the landing pad and provide localization information, which can be used  by a navigation controller for visual precision landing.

![](../images/autonomy/takeoff-landing-app-cam-view.gif)

This application runs the model in-the-loop with the sim. After each movement command, the drone gets an image from its downward-facing RBG camera, runs that image through the model and displays the predicted landing pad location as a bounding box.

The application is configured through [`perception_app_config.jsonc`](../../client/python/example_user_scripts/autonomy/configs/perception_app_config.jsonc). This config file provides a sample implementation of how the motions in the application script, [`takeoff_landing_sync.py`](../../client/python/example_user_scripts/autonomy/takeoff_landing_sync.py) can be defined. To run the sim and model in sync, the motion of the drone is broken up into smaller steps which are defined through the config.

 If you have not done so already, please follow the steps in the [ProjectAirSim Autonomy Setup](setup.md) documentation page to setup your python environment before proceeding to the next steps below.

1. Run the Project AirSim Sim server: `Blocks.sh` / `Blocks.exe`

1. Update the `client/python/example_user_scripts/autonomy/configs/perception_app_config.jsonc` if necessary, `"perception-pretrained-model"`:
1. With the `airsim-venv` python environment activated, launch the Takeoff-Landing demo app:
`python client/python/example_user_scripts/autonomy/takeoff_landing_sync.py`

## RL Gym environments

The Autonomy Gym environments provide configurable Markov Decision Processes with an interface compatible with the [OpenAI Gym](https://gym.openai.com) specifications. This allows the user to use any Reinforcement Learning algorithm to train an Agent to solve tasks like obstacle avoidance, drone landing etc.

Please visit the [gym_envs doc](gym/gym_envs.md) for more details on the Autonomy Gym environments.