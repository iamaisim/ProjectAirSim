#!/usr/bin/env python3
"""
Copyright (C) Microsoft Corporation. All rights reserved.
ProjectAirSim:: Autonomy:: Gym Environment: MSFT Bonsai SDK3 Simulator Integration using Python

Usage:
  1. Run this script to start and register the Sim instance
  with your Bonsai Workspace
  2. Connect your registered simulator to a Brain via UI, or
   using the CLI:
   `bonsai simulator unmanaged connect -b <brain-name> -a <train-or-assess> -c <concept-name> --simulator-name ProjectAirSimDetectAvoid
"""

import asyncio
import datetime
from enum import Enum
import os
import pathlib
import time
from distutils.util import strtobool
from typing import Any, Dict

from projectairsim.autonomy.gym_envs.detectavoid_simulator import ProjectAirSimDetectAvoid
from azure.core.exceptions import HttpResponseError
from dotenv import load_dotenv, set_key
from microsoft_bonsai_api.simulator.client import BonsaiClient, BonsaiClientConfig
from microsoft_bonsai_api.simulator.generated.models import (
    SimulatorInterface,
    SimulatorSessionResponse,
    SimulatorState,
)

from policies import random_policy

log_path = "logs"

class EpisodeStatus(Enum):
    START = 0
    STEP=1
    END=2


class ProjectAirSimSimulatorSession:
    def __init__(
        self,
        render: bool = False,
        env_name: str = "ProjectAirSim",
        env_config: Dict = {},
        log_data: bool = False,
        log_file: str = None,
    ):
        """Simulator Interface with the Bonsai Platform

        Parameters
        ----------
        render : bool, optional
            Whether to visualize episodes during training, by default False
        env_name : str, optional
            Name of simulator interface, by default "Cartpole"
        log_data: bool, optional
            Whether to log data, by default False
        log_file : str, optional
            where to log data, by default None
        """
        self.env_config = env_config
        self.sim = ProjectAirSimDetectAvoid(env_config=env_config)
        # self.sim.run_gym()
        self.env_name = env_name
        self.render = render
        self.log_data = log_data
        if not log_file:
            current_time = datetime.datetime.now().strftime("%Y-%m-%d-%H-%M-%S")
            log_file = current_time + "_" + env_name + "_log.csv"
            log_file = os.path.join(log_path, log_file)
            logs_directory = pathlib.Path(log_file).parent.absolute()
            if not pathlib.Path(logs_directory).exists():
                print(
                    "Directory does not exist at {0}, creating now...".format(
                        str(logs_directory)
                    )
                )
                logs_directory.mkdir(parents=True, exist_ok=True)
        self.log_file = os.path.join(log_path, log_file)

        # book keeping for rate status
        self.iteration_count = 0
        self.episode_count = 0
        self._log_interval = 10.0  # seconds
        self._last_status = time.time()

    def get_state(self) -> Dict[str, float]:
        """Extract current states from the simulator

        Returns
        -------
        Dict[str, float]
            Returns float of current values from the simulator
        """
        return self.sim._last_state

    def halted(self) -> bool:
        """Halt current episode. Note, this should only be called if the simulator has reached an unexpected state.

        Returns
        -------
        bool
            Whether to terminate current episode
        """
        return False

    def episode_start(self, config: Dict[str, Any]):
        self.iteration_count = 0
        self.episode_reward = 0

        if "iteration_limit" in config:
            self.sim._iteration_limit = config.get("iteration_limit", 1000)

        # initial observation
        observation = self.sim.gym_episode_start(config)
        state = self.sim.gym_to_state(observation)
        # Add metadata for Brain telemetry and refinement
        state["iteration_count"] = self.iteration_count
        state["episode_count"] = self.episode_count
        state["episode_status"] = EpisodeStatus.START
        self.sim._set_last_state(state, 0, False)
        return state

    async def stepping(self, action):
        return await self.sim.gym_simulate(action)

    async def episode_step(self, action: Dict[str, Any]):

        # Brain action to Sim action
        gym_action = self.sim.action_to_gym(action)
        # Sim Initializers
        rwd_accum = 0
        done = False
        i = 0
        observation = None
        # Simulate available sim run iterations
        for i in range(self.sim._skip_frame):
            observation, reward, done, info = await self.stepping(gym_action)
            self.iteration_count += 1
            rwd_accum += reward

            # print(
            #    "step action: {} state: {} reward: {} done: {}".format(
            #        str(gym_action), str(observation), str(reward), str(done)
            #    )
            # )

            # episode limits
            if self.sim._iteration_limit > 0:
                if self.iteration_count >= self.sim._iteration_limit:
                    done = True
                    print("iteration_limit reached.")
                    break

            # render if not headless
            if not self.sim._headless:
                if "human" in self.sim._env.metadata["render.modes"]:
                    self.sim._env.render()

        # print a periodic status of iterations and episodes
        # self._periodic_status_update(self.episode_count, self.episode_reward)

        # calculate reward
        reward = rwd_accum / (i + 1)
        self.episode_reward += reward

        # convert state and return to the server
        state = self.sim.gym_to_state(observation)
        # add metadata for Brain telemetry & refinement
        state["iteration_count"] =  self.iteration_count
        state["episode_count"] = self.episode_count  # Doesn't change until episode_finish
        state["episode_status"] = EpisodeStatus.STEP
        self.sim._set_last_state(state, reward, done)
        return state, reward, done

    def episode_finish(self, reason: str):
        print("Episode {} reward is {}".format(self.episode_count, self.episode_reward))
        print(
            "finish episode: "
            + str(self.episode_count)
            + " reward: "
            + str(self.episode_reward)
        )
        self.episode_count += 1
        self.sim._last_status = time.time()

    def sim_render(self):
        pass

    def _periodic_status_update(self, episode_count, episode_reward):
        """print a periodic status update showing iterations/sec"""
        if time.time() - self._last_status > self._log_interval:
            print(
                "Episode {} is still running, "
                "reward so far is {}".format(episode_count, episode_reward)
            )
            self._last_status = time.time()

    def log_iterations(self, state, action, episode: int = 0, iteration: int = 1):
        """Log iterations during training to a CSV.

        Parameters
        ----------
        state : Dict
        action : Dict
        episode : int, optional
        iteration : int, optional
        """

        import pandas as pd

        def add_prefixes(d, prefix: str):
            return {f"{prefix}_{k}": v for k, v in d.items()}

        state = add_prefixes(state, "state")
        action = add_prefixes(action, "action")
        config = add_prefixes(self.config, "config")
        data = {**state, **action, **config}
        data["episode"] = episode
        data["iteration"] = iteration
        log_df = pd.DataFrame(data, index=[0])

        if os.path.exists(self.log_file):
            log_df.to_csv(
                path_or_buf=self.log_file, mode="a", header=False, index=False
            )
        else:
            log_df.to_csv(path_or_buf=self.log_file, mode="w", header=True, index=False)


def env_setup():
    """Helper function to setup connection with Project Bonsai

    Returns
    -------
    Tuple
        workspace, and access_key
    """

    load_dotenv(verbose=True)
    workspace = os.getenv("SIM_WORKSPACE")
    access_key = os.getenv("SIM_ACCESS_KEY")

    env_file_exists = os.path.exists(".env")
    if not env_file_exists:
        open(".env", "a").close()

    if not all([env_file_exists, workspace]):
        workspace = input("Please enter your workspace id: ")
        set_key(".env", "SIM_WORKSPACE", workspace)
    if not all([env_file_exists, access_key]):
        access_key = input("Please enter your access key: ")
        set_key(".env", "SIM_ACCESS_KEY", access_key)

    load_dotenv(verbose=True, override=True)
    workspace = os.getenv("SIM_WORKSPACE")
    access_key = os.getenv("SIM_ACCESS_KEY")

    return workspace, access_key


async def test_random_policy(
    num_episodes: int = 50000,
    render: bool = True,
    num_iterations: int = 250,
    log_iterations: bool = False,
):
    """Test a policy using random actions over a fixed number of episodes

    Parameters
    ----------
    num_episodes : int, optional
        number of iterations to run, by default 10
    """

    sim = ProjectAirSimSimulatorSession(
        render=render, log_data=log_iterations, log_file="random_policy.csv"
    )
    for episode in range(num_episodes):
        iteration = 0
        terminal = False
        episode_config = {"episode_length": 250, "iteration_limit": 1000}
        sim.episode_start(config=episode_config)
        sim_state = sim.get_state()
        # it is important to know initial actions for evolution of the dynamics
        action = {"Vx": 0.1, "Vy": 0.1, "Vz": 0.1}
        if log_iterations:
            sim.log_iterations(sim_state, action, episode, iteration)
        while not terminal:
            action = random_policy(sim_state)
            # sim iteration
            await sim.episode_step(action)
            sim_state = sim.get_state()
            iteration += 1
            if log_iterations:
                sim.log_iterations(sim_state, action, episode, iteration)
            print(f"Running iteration #{iteration} for episode #{episode}")
            print(f"Observations: {sim_state}")
            terminal = iteration >= num_iterations
    return sim


async def main(
    render: bool = False, log_iterations: bool = False, config_setup: bool = False
):
    """Main entrypoint for running simulator connections

    Parameters
    ----------
    render : bool, optional
        visualize steps in environment, by default True, by default False
    log_iterations: bool, optional
        log iterations during training to a CSV file
    """

    # workspace environment variables
    if config_setup:
        env_setup()
        load_dotenv(verbose=True, override=True)

    # Grab standardized way to interact with sim API
    sim = ProjectAirSimSimulatorSession(render=render, log_data=log_iterations)

    # Configure client to interact with Bonsai service
    config_client = BonsaiClientConfig()
    client = BonsaiClient(config_client)

    # Create simulator session and init sequence id
    registration_info = SimulatorInterface(
        name=sim.env_name,
        timeout=60,
        simulator_context=config_client.simulator_context,
    )

    def CreateSession(
        registration_info: SimulatorInterface, config_client: BonsaiClientConfig
    ):
        """Creates a new Simulator Session and returns new session, sequenceId"""

        try:
            print(
                "config: {}, {}".format(config_client.server, config_client.workspace)
            )
            registered_session: SimulatorSessionResponse = client.session.create(
                workspace_name=config_client.workspace, body=registration_info
            )
            print("Registered simulator. {}".format(registered_session.session_id))

            return registered_session, 1
        except HttpResponseError as ex:
            print(
                "HttpResponseError in Registering session: StatusCode: {}, Error: {}, Exception: {}".format(
                    ex.status_code, ex.error.message, ex
                )
            )
            raise ex
        except Exception as ex:
            print(
                "UnExpected error: {}, Most likely, it's some network connectivity issue, make sure you are able to reach bonsai platform from your network.".format(
                    ex
                )
            )
            raise ex

    registered_session, sequence_id = CreateSession(registration_info, config_client)
    episode = 0
    iteration = 0  # TODO: iteration vs sim.iteration_count?

    try:
        while True:
            # Advance by the new state depending on the event type
            # TODO: it's risky not doing doing `get_state` without first initializing the sim
            sim_state = SimulatorState(
                sequence_id=sequence_id,
                state=sim.get_state(),
                halted=sim.halted(),
            )
            print(sim.get_state())

            try:
                event = client.session.advance(
                    workspace_name=config_client.workspace,
                    session_id=registered_session.session_id,
                    body=sim_state,
                )
                sequence_id = event.sequence_id
                print(
                    "[{}] Last Event: {}".format(time.strftime("%H:%M:%S"), event.type)
                )
            except HttpResponseError as ex:
                print(
                    "HttpResponseError in Advance: StatusCode: {}, Error: {}, Exception: {}".format(
                        ex.status_code, ex.error.message, ex
                    )
                )
                # This can happen in network connectivity issue, though SDK has retry logic, but even after that request may fail,
                # if your network has some issue, or sim session at platform is going away..
                # So let's re-register sim-session and get a new session and continue iterating. :-)
                registered_session, sequence_id = CreateSession(
                    registration_info, config_client
                )
                continue
            except Exception as err:
                print("Unexpected error in Advance: {}".format(err))
                # Ideally this shouldn't happen, but for very long-running sims It can happen with various reasons, let's re-register sim & Move on.
                # If possible try to notify Bonsai team to see, if this is platform issue and can be fixed.
                registered_session, sequence_id = CreateSession(
                    registration_info, config_client
                )
                continue

            # Event loop
            if event.type == "Idle":
                time.sleep(event.idle.callback_time)
                print("Idling...")
            elif event.type == "EpisodeStart":
                # print(event.episode_start.config)
                sim.episode_start(event.episode_start.config)
                episode += 1
                # Note: episode iteration starts at 1 for matching Telescope
                if sim.log_data:
                    sim.log_iterations(
                        episode=episode,
                        iteration=iteration,
                        state=sim.get_state(),
                        action=event.episode_step.action,
                    )
            elif event.type == "EpisodeStep":
                iteration += 1
                await sim.episode_step(event.episode_step.action)
                if sim.log_data:
                    sim.log_iterations(
                        episode=episode,
                        iteration=iteration,
                        state=sim.get_state(),
                        action=event.episode_step.action,
                    )
            elif event.type == "EpisodeFinish":
                print("Episode Finishing...")
                # Set metadata to indicate episode termination
                state = sim.get_state()
                state["terminal"] = True
                state["episode_status"] = EpisodeStatus.END
                iteration = 0
            elif event.type == "Unregister":
                print(
                    "Simulator Session unregistered by platform because '{}', Registering again!".format(
                        event.unregister.details
                    )
                )
                registered_session, sequence_id = CreateSession(
                    registration_info, config_client
                )
                continue
            else:
                pass
    except KeyboardInterrupt:
        # Gracefully unregister with keyboard interrupt
        client.session.delete(
            workspace_name=config_client.workspace,
            session_id=registered_session.session_id,
        )
        print("Unregistered simulator.")
    except Exception as err:
        # Gracefully unregister for any other exceptions
        client.session.delete(
            workspace_name=config_client.workspace,
            session_id=registered_session.session_id,
        )
        print("Unregistered simulator because: {}".format(err))


if __name__ == "__main__":

    import argparse

    parser = argparse.ArgumentParser(description="Bonsai and Simulator Integration...")
    parser.add_argument(
        "--render",
        type=lambda x: bool(strtobool(x)),
        default=False,
        help="Render training episodes",
    )
    parser.add_argument(
        "--log-iterations",
        type=lambda x: bool(strtobool(x)),
        default=False,
        help="Log iterations during training",
    )
    parser.add_argument(
        "--config-setup",
        type=lambda x: bool(strtobool(x)),
        default=False,
        help="Use a local environment file to setup access keys and workspace ids",
    )
    parser.add_argument(
        "--test-local",
        type=lambda x: bool(strtobool(x)),
        default=False,
        help="Run simulator locally without connecting to platform",
    )

    args = parser.parse_args()

    if args.test_local:
        asyncio.run(
            test_random_policy(render=args.render, log_iterations=args.log_iterations)
        )
    else:
        asyncio.run(
            main(
                config_setup=args.config_setup,
                render=args.render,
                log_iterations=args.log_iterations,
            )
        )
