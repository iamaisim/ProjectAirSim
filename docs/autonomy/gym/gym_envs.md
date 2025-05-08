# Autonomy Gym Environments

Project AirSim's Autonomy module offers a set of "Gym environments", which are Project AirSim Simulation environments with built-in mechanisms to act as a training platform for Reinforcement Learning agents. The Autonomy Gym environments represent configurable Markov Decision Processes with an interface compatible with the [OpenAI Gym](https://gym.openai.com) specifications. This allows the user to use any Reinforcement Learning algorithm to train an Agent to solve tasks like obstacle avoidance, drone landing etc.
Following is a description of the RL Gym environments made available as part of this release:

## 1. Detect Avoid
The `ProjectAirSimDetectAvoidEnv-v0` environment allows training an RL Agent (or a [Microsoft Bonsai](https://bons.ai) Brain) to control a Drone/UAV to avoid obstacles and reach desired goal locations in an ProjectAirSim simulation environment.
The training scenario with the default set of config values for the goal locations and start state is shown below:

![](../../images/autonomy/gym_envs/detectavoidenv-default-config.png)

The Agent is tasked with controlling the drone using high-level velocity vector commands to navigate the drone to the goal location while avoiding the obstacles.

### Using the environment for RL training

0. **Setup**: If you have not done so already, please follow the [autonomy module setup instructions](../setup.md) to setup a python environment with the necessary Project AirSim client library modules installed. If you are planning to use [Microsoft Bonsai Platform](https://bons.aihttps://docs.microsoft.com/en-us/bonsai/) for training your RL Agents, we provide connectors to seamlessly connect to your Bonsai workspace.
Activate the python environment where you have installed the `projectairsim` python client package and install the additional `bonsai` module dependencies using the following command where `VERSION` is the Project AirSim version ID such as `0.1.8` (e.g. `projectairsim-0.1.8-py3-none-any.whl`) depending on the wheel package you have.

        > Note: The extra **[autonomy]** (including the square braces) after the wheel file name is necessary.

        `python -m pip install projectairsim-{VERSION}-py3-none-any.whl[autonomy]`


1. **Run Project AirSim Sim server**: `Blocks.sh` / `Blocks.exe`

1. **Create ProjectAirSim Detect-Avoid Gym environment**: You can create an instance of the `ProjectAirSimDetectAvoidEnv-v0` using the following python code:

```python
>>>import projectairsim.autonomy.gym_envs
>>>import gym
>>>detect_avoid_env = gym.make("ProjectAirSimDetectAvoidEnv-v0")
>>># Use the `detect_avoid_env` like any other OpenAI Gym Environment to train your RL Agent!
```

###  State/Observation space and Action space

```python
observation_space: spaces.Dict = spaces.Dict(
            {
                "pose": spaces.Box(-math.inf, math.inf, shape=(4,)),
                "obstacles": spaces.Tuple(
                    (
                        spaces.Box(0, math.inf, obstacle_bbox_shape)
                        for _ in range(num_obstacles)
                    )
                ),
                "distance_to_obstacles": spaces.Box(-math.inf, math.inf, (num_obstacles,)),
		 "displacement_to_goal": spaces.Box(-math.inf, math.inf, (3,)) # Dx, Dy, Dz

            }
        )

action_space = spaces.Box(-1.0, 1.0, shape=(3,))

	reward = (arena_norm_factor – norm_l2(GoalPose, DronePose)) / arena_norm_factor
```

### Customizing the training environment

This environment has the following features which promote generalizability in the learned Agent policies:
  - Randomize Goal location on every episode
  - Randomize Obstacle locations on every episode
  - Enable/Disable episodic obstacle randomization

The list of configurable parameters supported by this environment is provided below with comments on their usage:

``` python
{
    # dt: Delta/step time in seconds between commanding actions, needs to be longer
    # than the minimum command duration of the flight controller (50 Hz, 20 ms)
    "dt": 0.1,
    "step_limit": 200,  # Time budget: step_limit * dt seconds
    "goal_tolerance": 0.5,  # L1 distance from goal pose
    "randomize_obstacle_poses": False,  # Enable/Disable randomization of obstacle poses
    "num_obstacles": 1,  # Number of obstacles in the env
    "arena_config": {  # Arena ground plane scene coordinates in meters
        "top_left": {"x_m": 145, "y_m": 350, "z_m": -1.1,},
        "center": {"x_m": 96, "y_m": 398, "z_m": -1.1},
        "extent_x_m": 98,
        "extent_y_m": 96,
        "extent_z_m": 100,  # Roof height (excluding antenna) of Building1 with scale:0.5
    },
    "goals": {
        "v_vertex": {"x": 75.3, "y": 399.4, "z": 1.1},
        "v_apex_left": {"x": 136.5, "y": 370.7, "z": 1.1},
        "v_apex_right": {"x": 133.8, "y": 422.4, "z": 1.1},
    },
    "obstacles_info": {
        "PowerTower": {
            "size": {  # Size of asset at default scale=1
                "length_m": 9.6,
                "breadth_m": 22,
                "height_m": 38,
            }
        },
        "Building": {"size": {"length_m": 62.0, "breadth_m": 62.0, "height_m": 242}},
    },
    "obstacles_config": {
        "PowerTower1": {
            "type": "PowerTower",  # Key from obstacles_info
            "pose": {  # Default pose for reference
                "translation": Vector3({"x": 64.0, "y": 372.0, "z": -1.0}),
                "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
            },
            "scale": [1, 1, 1],
            "enable_physics": False,
        },
        "PowerTower2": {
            "type": "PowerTower",  # Key from obstacles_info
            "pose": {  # Default pose for reference
                "translation": Vector3({"x": 65.2, "y": 420.0, "z": -1.0}),
                "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
            },
            "scale": [1, 1, 1],
            "enable_physics": False,
        },
        "Building1": {
            "type": "Building",
            "pose": {  # Default pose for reference
                "translation": Vector3({"x": 105.0, "y": 400.0, "z": -1.0}),
                "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
            },
            "scale": [0.5, 0.5, 0.5],
            "enable_physics": False,
        },
    },
    "obstacle_poses": {
        "front-left": {
            "translation": Vector3({"x": 64.0, "y": 372.0, "z": -1.0}),
            "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
        },
        "front-right": {
            "translation": Vector3({"x": 65.2, "y": 420.0, "z": -1.0}),
            "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
        },
        "back": {
            "translation": Vector3({"x": 105.0, "y": 400.0, "z": -1.0}),
            "rotation": Quaternion({"w": 0, "x": 0, "y": 0, "z": 0}),
        },
    },
}
```

### RL Policy performance Assesment and debugging

The `ProjectAirSimDetectAvoidEnv-v0` offers a debugging mode to help with runtime assessment of RL Agents or Bonsai Brains or for debugging purposes to visualize and identify issues during training.

This mode can be enabled by setting `DEBUGGING` to `True` in the `projectairsim_detect_avoid_env.py` script. Once set, a visual rendering is enabled at every `step(a)` in the environment similar to the screenshot shown below:

![](../../images/autonomy/gym_envs/projectairsim_detectavoid_debug_view.png)


### Containerized Autonomy Gym Environment for Training a Bonsai Brain
The containerized Autonomy Gym Environments for Bonsai training contain all the necessary system dependencies and runtime components for the Sim (Project AirSim Server + Client + Autonomy Gym environment + Bonsai Connector) in a single docker container. You can connect this Simulator package to the Bonsai platform for training in two ways:

1. [Bonsai-platform-managed Sim]()
2. [User-managed (or Unmanaged) Sim]()

#### Visualization during training/debugging
If you would like to visualize the performance of the Agent/Brain in the environment, you can run the visualization enabled container package.

For example, to run the `ProjectAirSimDetectAvoidEnv` that is reconfigurable using inkling, you can run the following:

 > NOTE: You will need an access token to pull images from projectairsim.azurecr.io

0. Login to ACR: `az login && az acr login -n "projectairsim"`
1. Pull the container image: `docker pull  docker pull projectairsim.azurecr.io/bonsai/projectairsim-detectavoid-env-reconfigurable:v7-viz-updated-dist-calc`

```bash
sudo docker run -it --rm --gpus all --ipc=host -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY -e XAUTHORITY -e "SIM_WORKSPACE=YOUR-SIM-WORKSPACE-ID" -e "SIM_ACCESS_KEY=YOUR-SIM-ACCESS-KEY" projectairsim.azurecr.io/bonsai/projectairsim-detectavoid-env-reconfigurable:v7-viz-updated-dist-calc
```
Please insert `YOUR-SIM-WORKSPACE-ID` and `SIM-ACCESS-KEY` from your [Bonsai workspace](https://preview.bons.ai) settings.