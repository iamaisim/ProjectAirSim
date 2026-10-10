"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.

Model lifecycle helpers shared by the stock Simulink examples.
"""

import sys

from projectairsim.utils import projectairsim_log


def _cancel_pending_call(future):
    if future is not None and not future.done():
        if not future.cancel() and not future.done():
            raise RuntimeError("Pending MATLAB command could not be cancelled.")


def wait_for_simulink_call(future, timeout):
    """Wait for a lifecycle call without treating Engine cancellation as success."""
    try:
        result = future.result(timeout=timeout)
        # Engine.result() can consume Ctrl+C and return None after cancelling.
        if future.cancelled() or not future.done():
            raise KeyboardInterrupt("MATLAB command interrupted.")
        return result
    except BaseException:
        try:
            _cancel_pending_call(future)
        except BaseException as error:
            projectairsim_log().error("MATLAB command cancellation failed: %s", error)
        raise


def cleanup_simulink_model(
    matlab_engine, model_name, client, drone=None, start_future=None
):
    """Release flight authority, stop the model, and disconnect the client.

    A missing model name means startup was not requested. The shared MATLAB
    desktop remains open. Cleanup failures do not replace an active exception.
    """
    active_exception = sys.exc_info()[0] is not None
    actions = [("model start cancellation", lambda: _cancel_pending_call(start_future))]
    if drone is not None:
        actions.extend([
            ("command cancellation", drone.cancel_last_task),
            ("disarming", drone.disarm),
            ("API control release", drone.disable_api_control),
        ])
    if matlab_engine is not None and model_name is not None:
        def stop_model():
            future = matlab_engine.set_param(
                model_name,
                "SimulationCommand",
                "stop",
                nargout=0,
                background=True,
            )
            wait_for_simulink_call(future, timeout=15)

        actions.append(("model stop", stop_model))
    if client is not None:
        actions.append(("client disconnect", client.disconnect))

    first_error = None
    for name, action in actions:
        try:
            action()
        except BaseException as error:
            projectairsim_log().error("Cleanup failed during %s: %s", name, error)
            if first_error is None:
                first_error = error
    if first_error is not None and not active_exception:
        raise first_error
