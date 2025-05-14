"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
ProjectAirSim:: Autonomy:: Gym Environment: Sample policies

Sample policies to test RL Sim envs locally. These are intended to take
RL Agent states and return RL Agent actions.
"""

import numpy as np
from typing import Dict


def random_policy(state: Dict = None) -> Dict:
    """
    Ignore the state, move randomly.
    """
    action = {
        "Vx": np.random.uniform(-1, 1),
        "Vy": np.random.uniform(-1, 1),
        "Vz": np.random.uniform(-1, 1),
    }
    return action


def coast(state: Dict = None) -> Dict:
    """
    Ignore the state, fly up and up.
    """
    action = {"Vx": 0, "Vy": 0, "Vz": -1}
    return action


def benchmark(state: Dict = None) -> Dict:
    """
    TO DO: Add benchmark control policies such as PID, LQR, LQG, MPC
    These benchmark policies can be use-case specific. If benchmark is not accessible,
    you can also replay pre-recorded datasets of actions corresponding states.
    """
    pass


POLICIES = {"random": random_policy, "coast": coast}
