// Copyright (C) Microsoft Corporation. All rights reserved.

#ifndef CORE_SIM__SIM_SRC_CONSTANT_HPP_
#define CORE_SIM__SIM_SRC_CONSTANT_HPP_

namespace microsoft {
namespace projectairsim {

class Constant {
 public:
  class Component {
   public:
    static constexpr const char* client_authorization =
        "microsoft::projectairsim::client_authorization";
    static constexpr const char* topic_manager =
        "microsoft::projectairsim::topic_manager";
    static constexpr const char* topic = "microsoft::projectairsim::topic";
    static constexpr const char* simulator =
        "microsoft::projectairsim::simulator";
    static constexpr const char* scene = "microsoft::projectairsim::scene";
    static constexpr const char* robot =
        "microsoft::projectairsim::actor::robot";
    static constexpr const char* env_actor =
        "microsoft::projectairsim::actor::env_actor";
    static constexpr const char* env_object =
        "microsoft::projectairsim::actor::env_object";
    static constexpr const char* trajectory =
        "microsoft::projectairsim::trajectory";
    static constexpr const char* link = "microsoft::projectairsim::link";
    static constexpr const char* inertial =
        "microsoft::projectairsim::inertial";
    static constexpr const char* collision =
        "microsoft::projectairsim::collision";
    static constexpr const char* visual = "microsoft::projectairsim::visual";
    static constexpr const char* material =
        "microsoft::projectairsim::material";
    static constexpr const char* file_mesh = "microsoft::projectairsim::mesh";
    static constexpr const char* unreal_mesh =
        "microsoft::projectairsim::unreal_mesh";
    static constexpr const char* skeletal_mesh =
        "microsoft::projectairsim::skeletal_mesh";
    static constexpr const char* joint = "microsoft::projectairsim::joint";
    static constexpr const char* camera =
        "microsoft::projectairsim::sensors::camera";
    static constexpr const char* imu = "microsoft::projectairsim::sensors::imu";
    static constexpr const char* airspeed =
        "microsoft::projectairsim::sensors::airspeed";
    static constexpr const char* barometer =
        "microsoft::projectairsim::sensors::barometer";
    static constexpr const char* gimbal =
        "microsoft::projectairsim::actuators::gimbal";
    static constexpr const char* lidar =
        "microsoft::projectairsim::sensors::lidar";
    static constexpr const char* distance_sensor =
        "microsoft::projectairsim::sensors::distance_sensor";
    static constexpr const char* radar =
        "microsoft::projectairsim::sensors::radar";
    static constexpr const char* gps = "microsoft::projectairsim::sensors::gps";
    static constexpr const char* battery =
        "microsoft::projectairsim::sensors::battery";
    static constexpr const char* rotor =
        "microsoft::projectairsim::actuators::rotor";
    static constexpr const char* wheel =
        "microsoft::projectairsim::actuators::wheel";
    static constexpr const char* simple_flight_api =
        "microsoft::projectairsim::multirotor_api::simple_flight_api";
    static constexpr const char* simple_drive_api =
        "microsoft::projectairsim::multiwheel_api::simple_drive_api";
    static constexpr const char* px4_api =
        "microsoft::projectairsim::multirotor_api::px4_api";
    static constexpr const char* service = "microsoft::projectairsim::service";
    static constexpr const char* service_manager =
        "microsoft::projectairsim::service_manager";
    static constexpr const char* service_runner =
        "microsoft::projectairsim::service_runner";
  };

  class Config {
   public:
    static constexpr const char* id = "id";
    static constexpr const char* topics = "topics";
    static constexpr const char* default_scene = "default-scene";
    static constexpr const char* actors = "actors";
    static constexpr const char* env_actors = "environment-actors";
    static constexpr const char* env_objects = "environment-objects";
    static constexpr const char* clock = "clock";
    static constexpr const char* steppable = "steppable";
    static constexpr const char* real_time = "real-time";
    static constexpr const char* type = "type";
    static constexpr const char* robot = "robot";
    static constexpr const char* env_actor = "env_actor";
    static constexpr const char* env_object = "env_object";
    static constexpr const char* env_car = "env_car";
    static constexpr const char* env_human = "env_human";
    static constexpr const char* env_particle_effect = "env_particle_effect";
    static constexpr const char* origin = "origin";
    static constexpr const char* translation = "xyz";
    static constexpr const char* geo_point = "geo-point";
    static constexpr const char* rotation_rad = "rpy";
    static constexpr const char* rotation_deg = "rpy-deg";
    static constexpr const char* robot_config = "robot-config";
    static constexpr const char* env_actor_config = "env-actor-config";
    static constexpr const char* env_object_config = "env-object-config";
    static constexpr const char* script = "script";
    static constexpr const char* loop = "loop";
    static constexpr const char* auto_start = "auto_start";
    static constexpr const char* trajectory = "trajectory";
    static constexpr const char* time_sec = "time_sec";
    static constexpr const char* pose_x = "pose_x";
    static constexpr const char* pose_y = "pose_y";
    static constexpr const char* pose_z = "pose_z";
    static constexpr const char* pose_roll = "pose_roll";
    static constexpr const char* pose_pitch = "pose_pitch";
    static constexpr const char* pose_yaw = "pose_yaw";
    static constexpr const char* velocity_linear_x = "velocity_linear_x";
    static constexpr const char* velocity_linear_y = "velocity_linear_y";
    static constexpr const char* velocity_linear_z = "velocity_linear_z";
    static constexpr const char* links = "links";
    static constexpr const char* inertial = "inertial";
    static constexpr const char* collision = "collision";
    static constexpr const char* visual = "visual";
    static constexpr const char* mass = "mass";
    static constexpr const char* clamp_input = "clamp_input";
    static constexpr const char* clamp_output = "clamp_output";
    static constexpr const char* color = "color";
    static constexpr const char* colored_texture = "colored-texture";
    static constexpr const char* texture = "texture";
    static constexpr const char* file_name = "file_name";
    static constexpr const char* input_map = "input-map";
    static constexpr const char* input_max = "input-max";
    static constexpr const char* input_min = "input-min";
    static constexpr const char* name = "name";
    static constexpr const char* revolutions_per_sec = "revolutions-per-sec";
    static constexpr const char* initial_angle = "initial-angle";
    static constexpr const char* output_max = "output-max";
    static constexpr const char* output_min = "output-min";
    static constexpr const char* scale = "scale";
    static constexpr const char* step = "step-ns";
    static constexpr const char* pause_on_start = "pause-on-start";
    static constexpr const char* real_time_update_rate =
        "real-time-update-rate";
    static constexpr const char* geometry = "geometry";
    static constexpr const char* file_mesh = "file_mesh";
    static constexpr const char* unreal_mesh = "unreal_mesh";
    static constexpr const char* skeletal_mesh = "skeletal_mesh";
    static constexpr const char* material = "material";
    // static constexpr const char* physics_enabled = "physics-enabled";
    // static constexpr const char* gravity_enabled = "gravity-enabled";
    static constexpr const char* inertia_tensor_scale = "inertia-tensor-scale";
    static constexpr const char* stablization_threshold_multiplier =
        "stablization-threshold-multiplier";
    static constexpr const char* velocity_solver_iteration_count =
        "velocity-solver-iteration-count";
    static constexpr const char* position_solver_iteration_count =
        "position-solver-iteration-count";
    static constexpr const char* joints = "joints";
    static constexpr const char* fixed = "fixed";
    static constexpr const char* revolute = "revolute";
    static constexpr const char* continuous = "continuous";
    static constexpr const char* parent_link = "parent-link";
    static constexpr const char* child_link = "child-link";
    static constexpr const char* limit = "limit";
    static constexpr const char* axis = "axis";
    static constexpr const char* parent_dominates = "parent-dominates";
    static constexpr const char* spring_constant = "spring-constant";
    static constexpr const char* damping_constant = "damping-constant";
    static constexpr const char* max_force = "max-force";
    static constexpr const char* ip = "ip";
    static constexpr const char* port = "port";
    static constexpr const char* services = "services";
    static constexpr const char* frequency = "frequency";
    static constexpr const char* message_type = "message-type";
    static constexpr const char* published = "published";
    static constexpr const char* subscribed = "subscribed";
    static constexpr const char* int8 = "int8";
    static constexpr const char* joint_state = "joint-state";
    static constexpr const char* flight_control_setpoint =
        "flight-control-setpoint";
    static constexpr const char* sensors = "sensors";
    static constexpr const char* actuators = "actuators";
    static constexpr const char* enabled = "enabled";
    static constexpr const char* vr_mode = "vr-mode";
    //! Camera sensor related config parameters
    static constexpr const char* camera = "camera";
    static constexpr const char* capture_interval = "capture-interval";
    static constexpr const char* capture_settings = "capture-settings";
    static constexpr const char* image_type = "image-type";
    static constexpr const char* capture_enabled = "capture-enabled";
    static constexpr const char* streaming_enabled = "streaming-enabled";
    static constexpr const char* show_debug_plots = "show-debug-plots";
    static constexpr const char* width = "width";
    static constexpr const char* height = "height";
    static constexpr const char* fov_degrees = "fov-degrees";
    static constexpr const char* pixels_as_float = "pixels-as-float";
    static constexpr const char* compress = "compress";
    static constexpr const char* auto_exposure_method = "auto-exposure-method";
    static constexpr const char* auto_exposure_speed = "auto-exposure-speed";
    static constexpr const char* auto_exposure_bias = "auto-exposure-bias";
    static constexpr const char* auto_exposure_max_brightness =
        "auto-exposure-max-brightness";
    static constexpr const char* auto_exposure_min_brightness =
        "auto-exposure-min-brightness";
    static constexpr const char* auto_exposure_low_percent =
        "auto-exposure-low-percent";
    static constexpr const char* auto_exposure_high_percent =
        "auto-exposure-high-percent";
    static constexpr const char* auto_exposure_histogram_log_min =
        "auto-exposure-histogram-log-min";
    static constexpr const char* auto_exposure_histogram_log_max =
        "auto-exposure-histogram-log-max";
    static constexpr const char* motion_blur_amount = "motion-blur-amount";
    static constexpr const char* target_gamma = "target-gamma";
    static constexpr const char* max_depth_meters = "max-depth-meters";
    static constexpr const char* chromatic_aberration_intensity =
        "chromatic-aberration-intensity";
    static constexpr const char* depth_of_field_focal_region =
        "depth-of-field-focal-region";
    static constexpr const char* depth_of_field_transition_region =
        "depth-of-field-transition-region";
    static constexpr const char* noise_settings = "noise-settings";
    static constexpr const char* rand_contrib = "rand-contrib";
    static constexpr const char* rand_speed = "rand-speed";
    static constexpr const char* rand_size = "rand-size";
    static constexpr const char* rand_density = "rand-density";
    static constexpr const char* horz_wave_contrib = "horz-wave-contrib";
    static constexpr const char* horz_wave_strength = "horz-wave-strength";
    static constexpr const char* horz_wave_vert_size = "horz-wave-vert-size";
    static constexpr const char* horz_wave_screen_size =
        "horz-wave-screen-size";
    static constexpr const char* horz_noise_lines_contrib =
        "horz-noise-lines-contrib";
    static constexpr const char* horz_noise_lines_density_y =
        "horz-noise-lines-density-y";
    static constexpr const char* horz_noise_lines_density_xy =
        "horz-noise-lines-density-xy";
    static constexpr const char* horz_distortion_contrib =
        "horz-distortion-contrib";
    static constexpr const char* horz_distortion_strength =
        "horz-distortion-strength";
    static constexpr const char* gimbal = "gimbal";
    static constexpr const char* gimbal_id = "gimbal-id";
    static constexpr const char* lock_roll = "lock-roll";
    static constexpr const char* lock_pitch = "lock-pitch";
    static constexpr const char* lock_yaw = "lock-yaw";
    static constexpr const char* focal_length = "focal-length";
    static constexpr const char* aperture = "aperture";
    static constexpr const char* annotation_settings = "annotation-settings";
    static constexpr const char* distance_filter_enabled =
        "distance-filter-enabled";
    static constexpr const char* distance_filter_range =
        "distance-filter-range";
    static constexpr const char* object_ids = "object-ids";
    static constexpr const char* bbox2D_settings = "bbox2D-settings";
    static constexpr const char* bbox3D_settings = "bbox3D-settings";
    static constexpr const char* alignment = "alignment";
    static constexpr const char* post_process_model_settings =
        "post-process-model-settings";
    static constexpr const char* model_filepath = "model-filepath";
    static constexpr const char* execution_provider = "execution-provider";

    //! IMU sensor related config parameters
    static constexpr const char* imu = "imu";
    static constexpr const char* accelerometer = "accelerometer";
    static constexpr const char* gravity = "gravity";
    static constexpr const char* velocity_random_walk = "velocity-random-walk";
    static constexpr const char* tau = "tau";
    static constexpr const char* bias_stability = "bias-stability";
    static constexpr const char* turn_on_bias = "turn-on-bias";
    static constexpr const char* gyroscope = "gyroscope";
    static constexpr const char* angle_random_walk = "angle-random-walk";
    //! Airspeed sensor related config parameters
    static constexpr const char* airspeed = "airspeed";
    //! Barometer sensor related config parameters
    static constexpr const char* barometer = "barometer";
    static constexpr const char* battery = "battery";
    static constexpr const char* qnh = "qnh";
    static constexpr const char* pressure_factor_sigma =
        "pressure_factor_sigma";
    static constexpr const char* pressure_factor_tau = "pressure-factor-tau";
    static constexpr const char* uncorrelated_noise_sigma =
        "uncorrelated_noise_sigma";
    static constexpr const char* update_latency = "update-latency";
    static constexpr const char* update_frequency = "update-frequency";
    static constexpr const char* startup_delay = "startup-delay";
    // Battery sensor config parameters
    static constexpr const char* total_battery_capacity =
        "total-battery-capacity";
    static constexpr const char* battery_mode = "battery-mode";
    static constexpr const char* battery_capacity_at_start =
        "battery-capacity-on-start";
    static constexpr const char* battery_drain_rate_at_start =
        "battery-drain-rate-on-start";
    static constexpr const char* rotor_power_coefficient =
        "rotor-power-coefficient";
    static constexpr const char* battery_simple_discharge_mode =
        "simple-discharge-mode";
    static constexpr const char* battery_rotor_power_discharge =
        "rotor-power-discharge-mode";
    //! Magnetometer sensor related config parameters
    static constexpr const char* magnetometer = "magnetometer";
    static constexpr const char* noise_sigma = "noise-sigma";
    static constexpr const char* scale_factor = "scale-factor";
    static constexpr const char* noise_bias = "noise-bias";
    static constexpr const char* dynamic_reference_source =
        "dynamic-reference-source";
    static constexpr const char* ref_source = "ref-source";
    //! GPS sensor related config parameters
    static constexpr const char* gps = "gps";
    static constexpr const char* eph_time_constant = "eph-time-const";
    static constexpr const char* epv_time_constant = "epv-time-constant";
    static constexpr const char* eph_initial = "eph-initial";
    static constexpr const char* epv_initial = "epv-initial";
    static constexpr const char* eph_final = "eph-final";
    static constexpr const char* epv_final = "epv-final";
    static constexpr const char* eph_min_3d = "eph-min-3d";
    static constexpr const char* eph_min_2d = "eph-min-2d";
    //! Lidar related config parameters
    static constexpr const char* lidar = "lidar";
    static constexpr const char* lidar_type = "lidar-type";
    static constexpr const char* scan_rpy = "scan-rpy";
    static constexpr const char* number_of_channels = "number-of-channels";
    static constexpr const char* range = "range";
    static constexpr const char* points_per_second = "points-per-second";
    static constexpr const char* report_frequency = "report-frequency";
    static constexpr const char* report_no_return_points =
        "report-no-return-points";
    static constexpr const char* horizontal_rotation_frequency =
        "horizontal-rotation-frequency";
    static constexpr const char* horizontal_fov_start_deg =
        "horizontal-fov-start-deg";
    static constexpr const char* horizontal_fov_end_deg =
        "horizontal-fov-end-deg";
    static constexpr const char* vertical_rotation_frequency =
        "vertical-rotation-frequency";
    static constexpr const char* vertical_fov_upper_deg =
        "vertical-fov-upper-deg";
    static constexpr const char* vertical_fov_lower_deg =
        "vertical-fov-lower-deg";
    static constexpr const char* disable_self_hits = "disable-self-hits";
    static constexpr const char* draw_debug_points = "draw-debug-points";
    static constexpr const char* no_return_point_value =
        "no-return-point-value";
    static constexpr const char* distance_between_lasers =
        "distance-between-lasers";
    static constexpr const char* angle_between_lasers_pitch_max =
        "angle-between-lasers-pitch-max";
    static constexpr const char* angle_between_lasers_pitch_min =
        "angle-between-lasers-pitch-min";
    static constexpr const char* angle_between_lasers_yaw_max =
        "angle-between-lasers-yaw-max";
    static constexpr const char* angle_between_lasers_yaw_min =
        "angle-between-lasers-yaw-min";
    static constexpr const char* generic_cylindrical = "generic_cylindrical";
    static constexpr const char* generic_rosette = "generic_rosette";
    static constexpr const char* gpu_cylindrical = "gpu_cylindrical";
    static constexpr const char* livox_avia = "livox_avia";
    static constexpr const char* livox_mid70 = "livox_mid70";
    static constexpr const char* report_point_cloud = "report-point-cloud";
    static constexpr const char* report_azimuth_elevation_range =
        "report-azimuth-elevation-range";
    // Distance sensor related config parameters
    static constexpr const char* distance_sensor = "distance-sensor";
    static constexpr const char* min_distance = "min-distance";
    static constexpr const char* max_distance = "max-distance";
    // Segmentation ID config parameters
    static constexpr const char* segmentation = "segmentation";
    static constexpr const char* initialize_ids = "initialize-ids";
    static constexpr const char* ignore_existing = "ignore-existing";
    static constexpr const char* use_owner_name = "use-owner-name";
    // Radar sensor config parameters
    static constexpr const char* radar = "radar";
    static constexpr const char* fov = "fov";
    static constexpr const char* azimuth_max = "azimuth-max";
    static constexpr const char* azimuth_min = "azimuth-min";
    static constexpr const char* elevation_max = "elevation-max";
    static constexpr const char* elevation_min = "elevation-min";
    static constexpr const char* azimuth_resolution = "azimuth-resolution";
    static constexpr const char* elevation_resolution = "elevation-resolution";
    static constexpr const char* range_max = "range-max";
    static constexpr const char* range_min = "range-min";
    static constexpr const char* range_resolution = "range-resolution";
    static constexpr const char* velocity_max = "velocity-max";
    static constexpr const char* velocity_min = "velocity-min";
    static constexpr const char* velocity_resolution = "velocity-resolution";
    static constexpr const char* detection_interval = "detection-interval";
    static constexpr const char* track_interval = "track-interval";
    static constexpr const char* rcs_adjust_factor = "rcs-adjust-factor";
    static constexpr const char* masks = "masks";
    static constexpr const char* rcs_sqm_max = "rcs-sqm-max";
    static constexpr const char* rcs_sqm_min = "rcs-sqm-min";
    //! Actuator related config parameters
    static constexpr const char* rotor = "rotor";
    static constexpr const char* wheel = "wheel";
    static constexpr const char* physics = "physics";
    static constexpr const char* physics_type = "physics-type";
    static constexpr const char* non_physics = "non-physics";
    static constexpr const char* fast_physics = "fast-physics";
    static constexpr const char* matlab_physics = "matlab-physics";
    static constexpr const char* physics_connection = "physics-connection";
    static constexpr const char* control_connection = "control-connection";
    static constexpr const char* start_landed = "start-landed";
    static constexpr const char* unreal_physics = "unreal-physics";
    static constexpr const char* restitution = "restitution";
    static constexpr const char* friction = "friction";
    static constexpr const char* body_box_xyz = "body-box-xyz";
    static constexpr const char* linear_drag_coefficient =
        "linear-drag-coefficient";
    static constexpr const char* angular_drag_coefficient =
        "angular-drag-coefficient";
    static constexpr const char* home_geo_point = "home-geo-point";
    static constexpr const char* wind = "wind";
    static constexpr const char* velocity = "velocity";
    static constexpr const char* scene_type = "scene-type";
    static constexpr const char* enable_sim_topic_callback =
        "enable-sim-topic-callback";
    static constexpr const char* tiles_dir = "tiles-dir";
    static constexpr const char* horizon_tiles_dir = "horizon-tiles-dir";
    static constexpr const char* tiles_altitude_offset =
        "tiles-altitude-offset";
    static constexpr const char* tiles_lod_max = "tiles-lod-max";
    static constexpr const char* tiles_lod_min = "tiles-lod-min";
    static constexpr const char* latitude = "latitude";
    static constexpr const char* longitude = "longitude";
    static constexpr const char* altitude = "altitude";
    static constexpr const char* normal_vector = "normal-vector";
    static constexpr const char* turning_direction = "turning-direction";
    static constexpr const char* clock_wise = "clock-wise";
    static constexpr const char* counter_clock_wise = "counter-clock-wise";
    static constexpr const char* coeff_of_thrust = "coeff-of-thrust";
    static constexpr const char* coeff_of_torque = "coeff-of-torque";
    static constexpr const char* wheel_type = "wheel-type";
    static constexpr const char* coeff_of_wheel_torque =
        "coeff-of-wheel_torque";
    static constexpr const char* coeff_of_friction = "coeff-of-friction";
    static constexpr const char* air_density = "air-density";
    static constexpr const char* max_rpm = "max-rpm";
    static constexpr const char* propeller_diameter = "propeller-diameter";
    static constexpr const char* propeller_height = "propeller-height";
    static constexpr const char* smoothing_tc = "smoothing-tc";
    static constexpr const char* rotor_settings = "rotor-settings";
    static constexpr const char* wheel_settings = "wheel-settings";
    static constexpr const char* steering_connected = "steering-connected";
    static constexpr const char* first_order_filter_tc =
        "first-order-filter-tc";
    static constexpr const char* controller = "controller";
    static constexpr const char* simple_flight_api = "simple-flight-api";
    static constexpr const char* simple_flight_api_settings =
        "simple-flight-api-settings";
    static constexpr const char* simple_drive_api = "simple-drive-api";
    static constexpr const char* simple_drive_api_settings =
        "simple-drive-api-settings";
    static constexpr const char* px4_api = "px4-api";
    static constexpr const char* px4_api_settings = "px4-api-settings";
    static constexpr const char* ardupilot_api = "ardupilot-api";
    static constexpr const char* ardupilot_api_settings =
        "ardupilot-api-settings";
    static constexpr const char* actuator_order = "actuator-order";
    static constexpr const char* actuator_id = "actuator-id";
    static constexpr const char* inertia = "inertia";
    static constexpr const char* matrix = "matrix";
    static constexpr const char* ixx = "ixx";
    static constexpr const char* iyy = "iyy";
    static constexpr const char* izz = "izz";
    static constexpr const char* point_mass = "point-mass";
    static constexpr const char* box = "box";
    static constexpr const char* size = "size";
    static constexpr const char* cylinder = "cylinder";
    static constexpr const char* radius = "radius";
    static constexpr const char* length = "length";
    static constexpr const char* aerodynamics = "aerodynamics";
    static constexpr const char* drag_coefficient = "drag-coefficient";
    static constexpr const char* cross_section_areas_xyz =
        "cross-section-areas-xyz";
    // Lift-drag actuator config parameters
    static constexpr const char* lift_drag = "lift-drag";
    static constexpr const char* alpha_0 = "alpha-0";
    static constexpr const char* alpha_stall = "alpha-stall";
    static constexpr const char* c_lift_alpha = "c-lift-alpha";
    static constexpr const char* c_lift_alpha_stall = "c-lift-alpha-stall";
    static constexpr const char* c_drag_alpha = "c-drag-alpha";
    static constexpr const char* c_drag_alpha_stall = "c-drag-alpha-stall";
    static constexpr const char* c_moment_alpha = "c-moment-alpha";
    static constexpr const char* c_moment_alpha_stall = "c-moment-alpha-stall";
    static constexpr const char* area = "area";
    static constexpr const char* control_surface_cl_per_rad =
        "control-surface-cl-per-rad";
    static constexpr const char* control_surface_cd_per_rad =
        "control-surface-cd-per-rad";
    static constexpr const char* control_surface_cm_per_rad =
        "control-surface-cm-per-rad";
    static constexpr const char* center_pressure_xyz = "center-pressure-xyz";
    static constexpr const char* forward_xyz = "forward-xyz";
    static constexpr const char* upward_xyz = "upward-xyz";
    static constexpr const char* lift_drag_control_surface =
        "lift-drag-control-surface";
    static constexpr const char* lift_drag_control_surface_settings =
        "lift-drag-control-surface-settings";
    static constexpr const char* rotation_rate = "rotation-rate";
    static constexpr const char* distributed_sim = "distributed-sim";
    // Tilt actuator config parameters
    static constexpr const char* tilt = "tilt";
    static constexpr const char* angle_min = "angle-min";
    static constexpr const char* angle_max = "angle-max";
    static constexpr const char* target = "target";
    static constexpr const char* tilt_settings = "tilt-settings";
  };
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM__SIM_SRC_CONSTANT_HPP_
