# Project AirSim Python Client

`projectairsim` is the Python client library for [Project AirSim](https://github.com/iamaisim/ProjectAirSim), a simulation platform for drones, robots, and autonomous systems.

## Install

```bash
pip install projectairsim
```

Install optional LiDAR support with:

```bash
pip install "projectairsim[lidar]"
```

## Usage

```python
from projectairsim import ProjectAirSimClient

client = ProjectAirSimClient()
```

The client connects to a running Project AirSim simulation server. See the [client setup guide](https://github.com/iamaisim/ProjectAirSim/blob/main/docs/client_setup.md) for configuration and examples.

## License

Project AirSim is licensed under the MIT License. See the [repository license](https://github.com/iamaisim/ProjectAirSim/blob/main/LICENSE) for details.
