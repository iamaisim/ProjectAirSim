"""Fly a JSBSim Skywalker X8 swarm through Forest in a V formation.

Start a Forest/MountainVillage Project AirSim package before running this file.
The script loads the scene through RPC and owns its paused steppable clock.
"""
from __future__ import annotations
import argparse, json, math, tempfile, time
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
CONFIG_DIR = SCRIPT_DIR / "sim_config"
X8_CONFIG = CONFIG_DIR / "robot_x8_jsbsim.jsonc"
ROUTE_UE_CM = ((24680.,-31500.,9080.), (4980.,-32090.,5720.), (-23630.,3880.,8410.))
LIDAR_TARGET_UE_Z_CM = 2000.0
LIDAR_LOWERING_M = (ROUTE_UE_CM[0][2]-LIDAR_TARGET_UE_Z_CM)/100.0
DISPLAY_WIDTH, DISPLAY_HEIGHT = 640, 360
DT, DT_NS, AIRSPEED = 0.008, 8_000_000, 42.0
CAMERA_DRONE_INDEX = 4  # X8_5, rear-right member of the V

def to_ned(p):
    return p[0]/100, p[1]/100, -p[2]/100

def route_heading_deg():
    n0,e0,_=to_ned(ROUTE_UE_CM[0]); n1,e1,_=to_ned(ROUTE_UE_CM[1])
    return math.degrees(math.atan2(e1-e0,n1-n0)) % 360.0

def formation_for_segment(count, spacing, start, end):
    """Return V offsets rotated into the direction from start to end."""
    n0,e0,_=to_ned(start); n1,e1,_=to_ned(end)
    dn,de=n1-n0,e1-e0; length=math.hypot(dn,de)
    forward_n,forward_e=dn/length,de/length
    right_n,right_e=-forward_e,forward_n
    offsets=[(0.,0.)]; rank=1
    while len(offsets)<count:
        back_n=-forward_n*rank*spacing; back_e=-forward_e*rank*spacing
        lateral=rank*spacing*0.65
        for side in (-1.,1.):
            if len(offsets)<count: offsets.append((back_n+side*right_n*lateral,back_e+side*right_e*lateral))
        rank+=1
    return offsets

def formation(count, spacing):
    return formation_for_segment(count,spacing,ROUTE_UE_CM[0],ROUTE_UE_CM[1])

def scene(home_alt, count, spacing):
    n,e,d = to_ned(ROUTE_UE_CM[0])
    return {
      'id':'ForestX8JSBSimRoute',
      'actors':[{'type':'robot','name':f'X8_{i+1}','origin':{'xyz':f'{n+dn:.6f} {e+de:.6f} {d:.6f}','rpy-deg':f'0 0 {route_heading_deg():.6f}'},'start-landed':False,'robot-config':('robot_x8_camera.jsonc' if i==CAMERA_DRONE_INDEX else 'robot_x8_follower.jsonc')} for i,(dn,de) in enumerate(formation(count,spacing))],
      'clock':{'type':'steppable','step-ns':4_000_000,'real-time-update-rate':4_000_000,'pause-on-start':True},
      'home-geo-point':{'latitude':29.4296741,'longitude':-95.16384722,'altitude':home_alt},
      'segmentation':{'initialize-ids':True,'ignore-existing':False,'use-owner-name':True},
      'scene-type':'UnrealNative'}

def member_waypoints(home_alt, count, spacing, member_index):
    """Return this member's route relative to its own initial position."""
    start_n,start_e,_=to_ned(ROUTE_UE_CM[0])
    start_dn,start_de=formation_for_segment(
        count,spacing,ROUTE_UE_CM[0],ROUTE_UE_CM[1]
    )[member_index]
    member_start_n=start_n+start_dn
    member_start_e=start_e+start_de
    turn_offsets=formation_for_segment(
        count,spacing,ROUTE_UE_CM[1],ROUTE_UE_CM[2]
    )
    return [
        (n+turn_offsets[member_index][0]-member_start_n,
         e+turn_offsets[member_index][1]-member_start_e,
         (home_alt-d)*3.280839895)
        for n,e,d in map(to_ned,ROUTE_UE_CM[1:])
    ]

def seed(drone, speed):
    props={'velocities/u-fps':speed*1.687809857,'velocities/v-fps':0.,'velocities/w-fps':0.,'velocities/p-rad_sec':0.,'velocities/q-rad_sec':0.,'velocities/r-rad_sec':0.,'fcs/elevator-cmd-norm':0.,'fcs/aileron-cmd-norm':0.,'fcs/throttle-cmd-norm':.65}
    for key,value in props.items():
        if not drone.set_jsbsim_property(key,value): raise RuntimeError(f'JSBSim rejected {key}')

def finite(ap):
    names=('position/lat-geod-deg','position/long-gc-deg','position/h-sl-ft','velocities/vtrue-kts','attitude/phi-rad','attitude/theta-rad')
    state={n:ap.get(n) for n in names}
    if not all(math.isfinite(v) for v in state.values()): raise RuntimeError(f'Invalid X8 state: {state}')

def run(a):
    if not X8_CONFIG.is_file():
        raise FileNotFoundError(f"Missing X8 robot config: {X8_CONFIG}")
    from projectairsim import ProjectAirSimClient
    from projectairsim.image_utils import ImageDisplay
    from projectairsim.lidar_utils import LidarDisplay
    from projectairsim.utils import projectairsim_log
    from x8_autopilot import WaypointFollower,X8Autopilot

    class StableX8Autopilot(X8Autopilot):
        """Less aggressive course hold for smooth cinematic formation flight."""

        HEADING_DEADBAND_DEG=1.5

        def __init__(self, drone, control_period_sec):
            super().__init__(drone,control_period_sec)
            self.heading.kp=0.006
            self.heading.ki=0.00005
            self.roll.kp=0.16
            self.roll.kd=-0.070

        def hold(self, *, heading_deg, altitude_ft, airspeed_kts):
            current_heading=math.degrees(self.get('attitude/heading-true-rad'))
            heading_error=(heading_deg-current_heading+180.)%360.-180.
            if abs(heading_error)<self.HEADING_DEADBAND_DEG:
                heading_deg=current_heading
                self.heading.integral=0.
                self.heading.previous_error=0.
            super().hold(
                heading_deg=heading_deg,
                altitude_ft=altitude_ft,
                airspeed_kts=airspeed_kts,
            )
    with tempfile.TemporaryDirectory(prefix='forest_x8_route_') as tmp:
        cfg=Path(tmp); scene_data=scene(a.home_altitude,a.count,a.spacing)
        base=X8_CONFIG.read_text(encoding='utf-8')
        leader=base.replace('"fov-degrees": 75','"fov-degrees": 100')
        follower=base.replace('"id": "Chase",\n      "type": "camera",\n      "enabled": true','"id": "Chase",\n      "type": "camera",\n      "enabled": false')
        import commentjson
        camera_config=commentjson.loads(leader); follower_config=commentjson.loads(follower)
        camera_config['sensors'].append({
            'id':'lidar1',
            'type':'lidar',
            'enabled':True,
            'parent-link':'Frame',
            'number-of-channels':32,
            'range':250,
            'points-per-second':360000,
            'horizontal-rotation-frequency':10,
            'horizontal-fov-start-deg':0.0,
            'horizontal-fov-end-deg':360.0,
            'vertical-fov-upper-deg':15.0,
            'vertical-fov-lower-deg':-25.0,
            'disable-self-hits':True,
            'draw-debug-points':False,
            'origin':{
                'xyz':f'0 0 {LIDAR_LOWERING_M:.1f}',
                'rpy-deg':'0 0 0',
            },
            'report-point-cloud':True,
            'report-azimuth-elevation-range':False,
            'report-no-return-points':False,
            'report-frequency':10,
        })
        for index,actor in enumerate(scene_data['actors']): actor['robot-config']=camera_config if index==CAMERA_DRONE_INDEX else follower_config
        (cfg/'robot_x8_camera.jsonc').write_text(leader,encoding='utf-8')
        (cfg/'robot_x8_follower.jsonc').write_text(follower,encoding='utf-8')
        client=ProjectAirSimClient(address=a.address); connected=False
        image_display=ImageDisplay(
            num_subwin=2,
            subwin_width=DISPLAY_WIDTH,
            subwin_height=DISPLAY_HEIGHT,
        )
        if a.show_rgb:
            image_display.add_image(
                'X8 RGB',
                subwin_idx=0,
                resize_x=DISPLAY_WIDTH,
                resize_y=DISPLAY_HEIGHT,
            )
        lidar_display=None
        if a.show_lidar:
            lidar_subwin=image_display.get_subwin_info(1)
            lidar_display=LidarDisplay(
                win_name='X8 CPU LiDAR',
                x=lidar_subwin['x'],
                y=lidar_subwin['y']+30,
                color_mode=LidarDisplay.COLOR_INTENSITY,
                color_intensity_range=[0.0,1.0],
                width=DISPLAY_WIDTH,
                height=DISPLAY_HEIGHT,
                view=LidarDisplay.VIEW_TOPDOWN,
                view_bounds=[-250,-250,-60,250,250,60],
                coordinate_axes_size=2.0,
            )
        try:
            client.connect(); connected=True
            scene_id=client.request({'method':'/Sim/LoadScene','params':{'scene_config':json.dumps(scene_data)},'version':1.0})
            root=f'/Sim/{scene_id}'; time.sleep(3)
            actors=client.request({'method':root+'/ListActors','params':{},'version':1.0})
            expected=[f'X8_{i+1}' for i in range(a.count)]
            missing=[actor for actor in expected if actor not in actors]
            if missing: raise RuntimeError(f'Swarm actors missing from Unreal: {missing}; loaded={actors}')
            # Unreal automatically selects the first actor with a valid
            # streaming camera. X8_1..X8_4 have Chase disabled, so the
            # viewport already uses X8_5's aerial Chase transform here.
            projectairsim_log().info(
                f'Unreal viewport uses X8_{CAMERA_DRONE_INDEX+1} aerial Chase camera'
            )

            sensor_root=f'{root}/robots/X8_{CAMERA_DRONE_INDEX+1}/sensors'
            if a.show_rgb:
                client.subscribe(
                    f'{sensor_root}/Chase/scene_camera',
                    lambda _,rgb:image_display.receive(rgb,'X8 RGB'),
                )

            lidar_msg_counter=0
            def on_lidar(_,lidar):
                nonlocal lidar_msg_counter
                lidar_msg_counter+=1
                if lidar_msg_counter%10==1:
                    point_cloud=lidar.get('point_cloud',[]) if isinstance(lidar,dict) else []
                    projectairsim_log().info(
                        f'X8 CPU LiDAR msg={lidar_msg_counter} points={len(point_cloud)//3}'
                    )
                lidar_display.receive(lidar)
            if a.show_lidar:
                client.subscribe(f'{sensor_root}/lidar1/lidar',on_lidar)
            if a.show_rgb:
                image_display.start()
            if a.show_lidar:
                lidar_display.start()
            enabled_displays=[]
            if a.show_rgb: enabled_displays.append('RGB')
            if a.show_lidar: enabled_displays.append('CPU LiDAR')
            projectairsim_log().info(
                f'X8_{CAMERA_DRONE_INDEX+1} displays: '
                + (', '.join(enabled_displays) if enabled_displays else 'disabled')
            )

            class RpcDrone:
                def __init__(self, robot_name): self.path=f'{root}/robots/{robot_name}'
                def get_jsbsim_property(self, key):
                    return client.request({'method':self.path+'/GetJSBSimProperty','params':{'_property_name':key},'version':1.0})
                def set_jsbsim_property(self, key, value):
                    return client.request({'method':self.path+'/SetJSBSimProperty','params':{'_property_name':key,'_value':value},'version':1.0})
            autopilots=[]
            for member_index in range(a.count):
                drone=RpcDrone(f'X8_{member_index+1}'); seed(drone,a.airspeed)
                ap=StableX8Autopilot(drone,DT); ap.set_controls(elevator=0.,aileron=0.,throttle=1.)
                autopilots.append(ap)
            navigators=[
                WaypointFollower(
                    autopilots[i],
                    member_waypoints(a.home_altitude,a.count,a.spacing,i),
                    acceptance_radius_m=20.0,
                )
                for i in range(a.count)
            ]
            completed=[False]*a.count
            projectairsim_log().info(f'Loaded X8 swarm: {actors}')
            wall_start=time.perf_counter()
            for i in range(int(a.max_sim_time/DT)):
                for ap in autopilots: finite(ap)
                for member_index,nav in enumerate(navigators):
                    if not completed[member_index]:
                        completed[member_index]=nav.update(a.airspeed)
                client.request({'method':root+'/ContinueForSimTime','params':{'delta_time':DT_NS,'wait_until_complete':True},'version':1.0})
                target_wall_time=wall_start+(i+1)*DT/a.playback_rate
                remaining=target_wall_time-time.perf_counter()
                if remaining>0.: time.sleep(remaining)
                if i%250==0: projectairsim_log().info(f'Forest X8 swarm t={i*DT:.1f}s members={len(autopilots)} target={navigators[0].index+2}/3')
                if all(completed): projectairsim_log().info('Forest X8 swarm route completed'); break
        finally:
            if connected: client.disconnect()
            if a.show_rgb: image_display.stop()
            if lidar_display is not None: lidar_display.stop()

def args():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--count',type=int,default=5); p.add_argument('--spacing',type=float,default=5.)
    p.add_argument('--airspeed',type=float,default=AIRSPEED); p.add_argument('--max-sim-time',type=float,default=180.)
    p.add_argument('--home-altitude',type=float,default=7.5)
    p.add_argument('--address',default='127.0.0.1')
    p.add_argument('--playback-rate',type=float,default=0.5)
    p.add_argument('--no-rgb',action='store_false',dest='show_rgb',default=True,
                   help='Do not open the RGB camera window')
    p.add_argument('--no-lidar',action='store_false',dest='show_lidar',default=True,
                   help='Do not open the CPU LiDAR window')
    a=p.parse_args()
    if a.count<=CAMERA_DRONE_INDEX: p.error('--count must be at least 5 because the camera is on X8_5')
    if a.spacing<=0 or a.airspeed<=0 or a.playback_rate<=0: p.error('--spacing, --airspeed and --playback-rate must be positive')
    return a

if __name__=='__main__': run(args())
