# PX4 gimbal drone in a Cesium world

Flies a PX4 quadrotor with a gimbaled payload camera (GimbalCam) over real-world
Google 3D tiles streamed through Cesium for Unreal. The default spawn is Seletar
Airport's runway, Singapore.

## Prerequisites

- Unreal Engine 5.7 at `~/UnrealEngine-5.7`, with the Blocks editor built
  (Cesium for Unreal installed in `unreal/Blocks/Plugins/CesiumForUnreal`)
- PX4-Autopilot SITL built at `~/Documents/PX4-Autopilot`
- Conda env `projectairsim` (Python client, pymavlink, PyYAML)
- A Cesium ion token in `~/.cesium_ion_token`:
  ```bash
  echo 'export CESIUM_ION_TOKEN=<token>' > ~/.cesium_ion_token && chmod 600 ~/.cesium_ion_token
  ```

## Running

```bash
cd ~/Documents/ProjectAirSim/client/python/example_user_scripts
./run_cesium_sim.sh [options]
```

`run_cesium_sim.sh` starts the simulator, a fresh PX4 SITL with the gimbal
enabled, an RTSP server if port 8554 is free, and `px4_gimbal_cesium.py` with
your options. Ctrl+C, or closing the simulator window, stops everything.

The first run at a new location pauses about 45 s while tiles download; later
runs use the local cache.

| Command | What it does |
|---|---|
| `./run_cesium_sim.sh` | Scripted demo at Seletar: take off, climb to 40 m, sweep the gimbal, land |
| `./run_cesium_sim.sh --qgc` | Load the world and wait; fly and steer the gimbal from QGroundControl or the GimbalCam window |
| `./run_cesium_sim.sh --qgc --lat 1.3521 --lon 103.8198 --heading 90` | Same, spawning somewhere else |

## Options

| Option | Default | Meaning |
|---|---|---|
| `--lat`, `--lon` | Seletar runway | Spawn position (degrees). Pick open ground: trees and roofs are solid |
| `--heading` | 33 at Seletar, else 0 | Spawn heading, degrees clockwise from north |
| `--ground-alt` | looked up online | Ground elevation at the spawn (m MSL). Pass it to skip the Open-Meteo lookup |
| `--spawn-height` | 15 | The drone starts this high above `--ground-alt` and settles onto the real surface |
| `--geoid` | 8.1 | Geoid height (m); about 8 across Singapore |
| `--objects FILE` | `config_spawned_objects.yml` | Objects to place in the world; `''` for none |
| `--qgc` | off | Manual mode instead of the scripted demo |
| `--rtsp-url URL` | `rtsp://127.0.0.1:8554/gimbal` | Where the GimbalCam video is published |
| `--no-rtsp` | off | Don't publish video |
| `--tile-wait SEC` | 45 | How long to keep the sim paused while tiles load |
| `--scene FILE` | | Load a scene from `sim_config/` as-is instead of building one from `--lat/--lon` |

Run `python px4_gimbal_cesium.py --help` for the full list.

## Spawned objects

Objects are defined in `config_spawned_objects.yml`. Each has a position and
exactly one of `asset` (a model built into the sim) or `file` (your own
`.glb`/`.gltf`):

```yaml
objects:
  - name: Car1
    asset: SKM_SportsCar      # or: file: models/my_target.glb
    lat: 1.413730
    lon: 103.865200
    heading: 33               # degrees from north
    length: 4.5               # longest side in metres (or scale: [x, y, z])
    height: 0                 # metres above ground
    physics: false
```

Keep objects within a few hundred metres of the drone so the ground under them
has loaded.

While the sim runs, type these in the terminal:

| Command | Effect |
|---|---|
| `list` | Show objects spawned this session |
| `spawn ASSET LAT LON [HEADING] [LENGTH_M]` | Spawn a built-in asset on the ground |
| `delete NAME` / `delete all` | Remove objects |
| `reload` | Delete all and respawn from the YAML file (edit it first) |
| `assets [REGEX]` | List built-in assets |

## Camera and gimbal

**GimbalCam window** (click it first so it has keyboard focus):

| Input | Action |
|---|---|
| W / S | Tilt up / down |
| A / D | Pan left / right |
| Mouse wheel, + / - | Zoom in / out (optical, 1x to 10x) |
| C | Centre (level, straight ahead) |
| X | Look straight down |

**Video stream:** `rtsp://127.0.0.1:8554/gimbal` (H.264, 1280x720, 30 fps). Any
player or model can read it, for example
`cv2.VideoCapture("rtsp://127.0.0.1:8554/gimbal")`.

**QGroundControl:** add a manual link under Application Settings → Comm Links:
type UDP, port 14550, server `127.0.0.1:18570`, then Connect. The script
appears to QGC as a MAVLink camera, so QGC picks up the video and its zoom and
shutter controls work. Snapshots are saved to `~/Pictures/projectairsim/`.

QGC and the GimbalCam window both steer the gimbal through PX4. Whichever was
used last is in control.

## Ports

| Port | Used by |
|---|---|
| UDP 14550 ← 18570 | PX4 ↔ QGroundControl |
| UDP 14030 ← 14280 | PX4 ↔ this script (gimbal control and MAVLink camera) |
| TCP 8554 | RTSP video |
| TCP 8989 / 8990 | Simulator API |

## Logs and disk space

- Sim, PX4 and RTSP server logs: `/tmp/projectairsim-run/`, overwritten each run
- PX4 flight logs: `~/Documents/PX4-Autopilot/build/px4_sitl_default/rootfs/log/`,
  about 20 MB per run and never cleaned up. Delete old date folders when needed
- Cesium tile cache: `unreal/Blocks/cesium-request-cache.sqlite`, capped at 4096 tiles

## Troubleshooting

- **"Drone fell through the Cesium ground":** the tiles weren't loaded in time.
  Run again (they're cached now) or raise `--tile-wait`.
- **"Port … is in use":** a previous simulator or PX4 is still running. Close it,
  or run `pkill -x px4; pkill -x UnrealEditor`.
- **PX4 "Arming denied" at startup:** normal for a few seconds while its estimator
  settles; the demo retries.
