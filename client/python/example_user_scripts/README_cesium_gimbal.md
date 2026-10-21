# PX4 gimbal drone in a Cesium world

Flies a PX4 quadrotor with a gimbaled payload camera (GimbalCam) over real-world
Google 3D tiles streamed through Cesium for Unreal. The default spawn is Seletar
Airport's runway, Singapore.

## Prerequisites

- Unreal Engine 5.7 at `~/UnrealEngine-5.7`, with the Blocks editor built
  (Cesium for Unreal installed in `unreal/Blocks/Plugins/CesiumForUnreal`)
- PX4-Autopilot SITL built at `~/Documents/PX4-Autopilot`
- Conda env `projectairsim` (Python client, pymavlink, PyYAML)
- For the spotter: UE's template Mannequin, copied in once with
  `./install_spotter_assets.sh` (then restart the simulator)
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

The sim starts paused until the Cesium ground under the drone has loaded
(about 10 s once tiles are cached; longer on a location's first visit), then
sets the drone on it.

| Command | What it does |
|---|---|
| `./run_cesium_sim.sh` | Scripted demo at Seletar: take off, climb to 40 m, sweep the gimbal, land |
| `./run_cesium_sim.sh --qgc` | Load the world and wait; fly and steer the gimbal from QGroundControl or the GimbalCam window |
| `./run_cesium_sim.sh --qgc --lat 1.3521 --lon 103.8198 --heading 90` | Same, spawning somewhere else |
| `./run_cesium_sim.sh --qgc --spotter` | Also open the spotter window |
| `./run_cesium_sim.sh --px4-params my.params --pi-ip 192.168.199.73 --qgc` | Your PX4 parameters, plus MAVLink and video for the Raspberry Pi |

## Launcher options

These are handled by `run_cesium_sim.sh`; everything else goes to
`px4_gimbal_cesium.py`.

| Option | Default | Meaning |
|---|---|---|
| `--vehicle quad\|vtol` | quad | Airframe. VTOL is not available with PX4 yet |
| `--px4-params FILE` | | PX4 parameters applied at boot. Accepts `NAME VALUE` lines (like ArduPilot `.parm` files, `#` comments allowed) or a QGC `.params` export. Repeatable; later files win |
| `--pi-ip IP` | | Also send MAVLink to a companion computer at `IP`, and announce the video at `rtsp://<this laptop's LAN IP>:8554/gimbal` |
| `--pi-ports P1,P2` | 15540,15541 | Companion-computer MAVLink UDP ports (as `--serial1/2` in the old ArduPilot scripts) |

The sim also sets `NAV_RCL_ACT`, `NAV_DLL_ACT` and `COM_OBL_RC_ACT` after PX4
connects (no RC in simulation), so those three override your file.

## Script options

| Option | Default | Meaning |
|---|---|---|
| `--lat`, `--lon` | Seletar runway | Spawn position (degrees). Pick open ground: trees and roofs are solid |
| `--heading` | 33 at Seletar, else 0 | Spawn heading, degrees clockwise from north |
| `--ground-alt` | looked up online | Ground elevation at the spawn (m MSL). Pass it to skip the Open-Meteo lookup |
| `--spawn-height` | 15 | The drone starts this high above `--ground-alt` and settles onto the real surface |
| `--geoid` | 8.1 | Geoid height (m); about 8 across Singapore |
| `--objects FILE` | `config_spawned_objects.yml` | Objects to place in the world; `''` for none |
| `--camera-props FILE` | `camera_properties.json` | GimbalCam resolution, sensor and zoom lens |
| `--spotter` | off | Open the spotter window |
| `--qgc` | off | Manual mode instead of the scripted demo |
| `--rtsp-url URL` | `rtsp://127.0.0.1:8554/gimbal` | Where the GimbalCam video is published |
| `--rtsp-advertise-url URL` | same as `--rtsp-url` | Video address announced to QGC and the Pi (set by `--pi-ip`) |
| `--no-rtsp` | off | Don't publish video |
| `--tile-wait SEC` | 90 | Longest time to wait for the ground under the drone to load |
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
| `goto LAT LON [HEADING]` | Move the drone to a new place without restarting Unreal (`--qgc` mode) |

## Changing location

Start somewhere else with `--lat/--lon/--heading`, or move while running by
typing `goto LAT LON [HEADING]` in the terminal (`--qgc` mode). `goto` keeps
Unreal running, reloads the scene at the new spot, restarts PX4 for it (PX4 SITL
can only connect to one scene), waits for the ground there and respawns the
objects and spotter. It takes about 25 s once tiles are cached. Reconnect
QGroundControl afterwards.

Objects and spotter points are absolute lat/lon, so edit
`config_spawned_objects.yml` and `config_spotter_points.yml` for the new area.

## Camera properties and optical zoom

`camera_properties.json` uses the same format as the old ArduPilot package,
plus a `zoom` block:

```jsonc
"resolution": "1080p",                       // 360p, 480p, 720p, 1080p, 2k, 4k or WxH
"sensor": { "sensor_width_mm": 6.19, "sensor_height_mm": 3.48 },
"lens":   { "focal_length_mm": 5.2, "focus_distance_m": 50.0 },
"zoom":   { "focal_length_max_mm": 52.0,     // or "optical_zoom_max": 10
            "zoom_speed_percent_per_s": 30.0 }
```

The field of view is `2·atan(sensor_width / 2·focal_length)`, so zooming from
5.2 to 52 mm narrows a ZR10 from 61.5° to 6.8°, like the real 10x lens. Also
applied: `focus_distance_m` (depth of field), `ue_overrides.chromatic_aberration`
and the `ue_noise_*` image noise. Lens distortion, aperture and platform
vibration aren't simulated yet; the script warns if they're enabled.

## Camera and gimbal

**GimbalCam window** (click it first so it has keyboard focus):

| Input | Action |
|---|---|
| W / S | Tilt up / down |
| A / D | Pan left / right |
| Mouse wheel, + / - | Zoom in / out along the lens's focal-length range |
| C | Centre (level, straight ahead) |
| X | Look straight down |

**Video stream:** `rtsp://127.0.0.1:8554/gimbal` (H.264 at the camera's resolution, 30 fps). Any
player or model can read it, for example
`cv2.VideoCapture("rtsp://127.0.0.1:8554/gimbal")`.

**QGroundControl:** add a manual link under Application Settings → Comm Links:
type UDP, port 14550, server `127.0.0.1:18570`, then Connect. The script
appears to QGC as a MAVLink camera, so QGC picks up the video and its zoom and
shutter controls work. Snapshots are saved to `~/Pictures/projectairsim/`.

QGC and the GimbalCam window both steer the gimbal through PX4. Whichever was
used last is in control.

## Spotter

`--spotter` (or `python spotter_control.py` while a scene is running) opens a
window to place a walking human in the world:

- Pick a point from `config_spotter_points.yml` and press **Spawn here**,
  **Teleport** or **Walk to**. The spotter follows the ground and plays walk
  and idle animations.
- Click the window, then hold **W/S** to walk forward/back and **A/D** to turn.
- **Speed** sets walking speed; **Save position as point** appends the current
  spot to the YAML file.
- The status line shows its position and distance to the drone.

## Raspberry Pi (companion computer)

With `--pi-ip`, PX4 sends MAVLink to the Pi on UDP 15540 and 15541 (like the
old ArduPilot `--serial1/2 udpclient:` links). The Pi can:

- read video from `rtsp://<laptop LAN IP>:8554/gimbal` (printed at startup)
- steer the gimbal through PX4's gimbal manager (`MAV_CMD_DO_GIMBAL_MANAGER_CONFIGURE`,
  then `GIMBAL_MANAGER_SET_ATTITUDE` to PX4 1/1)
- zoom with `MAV_CMD_SET_CAMERA_ZOOM` to the camera component (1/100)

Allow UDP 15540-15541 and TCP 8554 through any firewall between the two.

## Ports

| Port | Used by |
|---|---|
| UDP 14550 ← 18570 | PX4 ↔ QGroundControl |
| UDP 14030 ← 14280 | PX4 ↔ this script (gimbal control and MAVLink camera) |
| UDP 15540, 15541 ← 14590, 14591 | PX4 ↔ Raspberry Pi (with `--pi-ip`) |
| TCP 8554 | RTSP video |
| TCP 8989 / 8990 | Simulator API |

## Logs and disk space

- Sim, PX4 and RTSP server logs: `/tmp/projectairsim-run/`, overwritten each run
- PX4 flight logs: `~/Documents/PX4-Autopilot/build/px4_sitl_default/rootfs/log/`,
  about 20 MB per run and never cleaned up. Delete old date folders when needed
- Cesium tile cache: `unreal/Blocks/cesium-request-cache.sqlite`, capped at 4096 tiles

## Troubleshooting

- **"Drone fell through the Cesium ground":** the ground didn't load in time.
  Run again (tiles are cached now) or raise `--tile-wait`.
- **Spotter "no skeletal object … or no animation":** run
  `./install_spotter_assets.sh` and restart the simulator.
- **"Port … is in use":** a previous simulator or PX4 is still running. Close it,
  or run `pkill -x px4; pkill -x UnrealEditor`.
- **PX4 "Arming denied" at startup:** normal for a few seconds while its estimator
  settles; the demo retries.
