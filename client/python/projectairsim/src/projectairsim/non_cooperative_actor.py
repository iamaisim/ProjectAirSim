from projectairsim.utils import projectairsim_log, point_distance
from projectairsim import EnvActor


class NonCooperativeActor(EnvActor):
    def __init__(
        self,
        client,
        world,
        name,
        spawn_point,
        speed,
        distance_ahead_of_target=0.0,
    ) -> None:
        """Class for managing a non-cooperative actor

        Arguments:
            client (Client): the sim client
            world (World): the sim world
            actor (EnvActor): the env actor that will act as the non-cooperative actor
            spawn_point (tuple): the drone starting point, in NED
            speed (float): the desired movement speed of the non-cooperative actor
            distance_ahead_of_target (float): the distance in front of the drone that the non-cooperative actor is expected to pass
        """
        super().__init__(client, world, name)
        self.world = world
        self.spawn_point = spawn_point
        self.speed = speed
        self.distance_ahead_of_target = distance_ahead_of_target

    def setup_drone_intercept(
        self, intercept_point, drone, drone_path, drone_speeds, intercept_leg_id
    ):
        """Sets up the non-cooperative actor to intercept a drone

        Arguments:
            intercept_point (tuple): the point at which to intercept the drone
            drone (Drone): the drone
            drone_path (List[List[float]]): the path that the drone is following
            drone_speeds (List[float]): list of speeds for each path leg
            intercept_leg_id (int): the leg of the path containing the intercept point
        """
        self.drone = drone
        self.drone_path = drone_path
        self.drone_speeds = drone_speeds
        self.intercept_leg_id = intercept_leg_id
        self.intercept_point = intercept_point

    def set_trigger_drone_distance(self, dist=20.0):
        """Sets the non-cooperative actor to start moving when the drone gets close to the intercept point

        Arguments:
            dist (float): the detection distance
        """
        if not self.drone:
            projectairsim_log().error("Drone is not set up")
            return
        if dist < self.distance_ahead_of_target:
            projectairsim_log().error(
                "Trigger distance less than intercept distance ahead of the drone"
            )
            return
        self.trigger_hit = False
        self.init_dist = dist
        self.topic = self.drone.robot_info["actual_pose"]
        self.client.subscribe(self.topic, self.handle_drone_pose)

    def begin_intercept(self, drone_pos):
        projectairsim_log().info("Launching non-cooperative actor")
        self.world.generate_intercept_trajectory(
            "intercept_trajectory",
            self.spawn_point,
            self.intercept_point,
            self.speed,
            [drone_pos, self.intercept_point],
            [self.drone_speeds[self.intercept_leg_id]],
            0,
            self.distance_ahead_of_target,
        )
        self.set_trajectory(
            "intercept_trajectory", time_offset=self.world.get_sim_time() / 1000000000
        )

    def handle_drone_pose(self, topic, pose):
        """Listener method"""
        drone_point = (
            pose["position"]["x"],
            pose["position"]["y"],
            pose["position"]["z"],
        )
        d = point_distance(drone_point, self.intercept_point)
        if d < self.init_dist and not self.trigger_hit:
            self.trigger_hit = True
            self.begin_intercept(drone_point)
