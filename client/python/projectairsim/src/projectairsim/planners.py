"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Implements the  motion planners for generating a flight path
"""
import math
from pqdict import pqdict as PQDict
from projectairsim.utils import projectairsim_log, point_distance


class AStarPlanner:
    def __init__(
        self, occupancy_map: list, map_center: list, map_size: list, resolution: int, ground_z_ned: float = 0
    ) -> None:
        """Implementation of the A* planner for generating a flight path
        The planner works in NEU coordinates. Inputs and outputs are in NED

        Args:
            occupancy_map (list): Voxel grid in RLE format
            map_center (list): center of the voxel grid in NED
            map_size (list): list (length, width, height) of the voxel grid in meters
            resolution (int): edge len of each voxel in meters
        """
        self.occupancy_map = occupancy_map
        self.map_center = list(map_center)
        self.map_center[2] *= -1  # NED to NEU
        self.map_length, self.map_width, self.map_height = map_size
        self.resolution = resolution
        self.ground_z = -ground_z_ned

    def generate_plan(self, start_pos: list, goal_pos: list):
        """For given start and goal positions (in NED), returns a path in NED coordinates

        Args:
            start_pos (list): (x, y, z) in NED
            goal_pos (list): (x, y, z) in NED

        Returns:
            list: list of (x, y, z) coordinates in NED
        """
        # NED to NEU
        start_pos_neu = (start_pos[0], start_pos[1], -start_pos[2])
        goal_pos_neu = (goal_pos[0], goal_pos[1], -goal_pos[2])
        open_pq = PQDict()
        open_pq.additem(start_pos_neu, 0)
        g_matrix = self.occupancy_map.copy()  # occupancy map
        g_matrix[:] = [math.inf for x in g_matrix]
        start_pos_idx = self.get_grid_idx(start_pos_neu)
        g_matrix[start_pos_idx] = 0
        parent_nodes = {}
        self.astar(
            open_pq,
            parent_nodes,
            g_matrix,
            goal_pos_neu,
        )
        path = self.get_path(parent_nodes, goal_pos_neu)

        return path

    def get_grid_idx(self, coordinate: list, is_NED=False):
        """For a given coordinate in NEU, returns the index of the corresponding voxel in the occupancy map

        Args:
            coordinate (list): (x, y, z) in NEU
        Returns:
            int: index of the corresponding voxel in the occupancy map
        """
        if is_NED:  # NED to NEU
            coordinate = (coordinate[0], coordinate[1], -coordinate[2])

        x = coordinate[0] - self.map_center[0]
        y = coordinate[1] - self.map_center[1]
        z = coordinate[2] - self.map_center[2]
        num_cells_x = self.map_length // self.resolution
        num_cells_y = self.map_width // self.resolution
        num_cells_z = self.map_height // self.resolution

        x_idx = round(x / self.resolution + (num_cells_x / 2))
        y_idx = round(y / self.resolution + (num_cells_y / 2))

        z_idx = round(z / self.resolution + (num_cells_z / 2))
        grid_idx = x_idx + num_cells_x * (z_idx + num_cells_z * y_idx)

        return int(grid_idx)

    def check_coordinate_validity(self, coordinate, is_NED=False):
        """For a given coordinate in NEU, returns True if the coordinate is valid (not occupied) in the occupancy map

        Args:
            coordinate (list): coordinate in NEU

        Returns:
            bool: If the coordinate is valid (not occupied) in the occupancy map
        """

        if is_NED:  # NED to NEU
            coordinate = (coordinate[0], coordinate[1], -coordinate[2])

        # Pose cannot be underground ( z < 0 in NED)
        if coordinate[2] < self.resolution + self.ground_z:  # Needs to be greater than min res in z
            return False

        # projectairsim_log().info(f"In range")
        idx = self.get_grid_idx(coordinate)

        try:
            return not self.occupancy_map[idx]
        except:
            projectairsim_log().info(f"Index {idx} for {coordinate} is out of bounds")
            return False

    def astar(
        self,
        open_pq: PQDict,
        parent_nodes: dict,
        g_matrix: list,
        goal: list,
    ):
        """Implementation of the A* algorithm"""
        max_nodes = 0
        while True:
            try:
                current_pt = open_pq.pop()

            except KeyError:
                projectairsim_log().info("No paths found")
                break

            goal_idx = self.get_grid_idx(goal)

            if current_pt == goal:
                projectairsim_log().info("Path found successfully")
                break
            elif self.get_grid_idx(current_pt) == goal_idx:
                projectairsim_log().info("Path found successfully")
                # add goal entry to dict
                open_pq.additem(
                    goal,
                    g_matrix[goal_idx],
                )
                parent_nodes[goal] = current_pt
                break

            max_nodes = self.neighbourhood_search(
                open_pq,
                current_pt,
                parent_nodes,
                g_matrix,
                goal,
                max_nodes,
            )

    def neighbourhood_search(
        self,
        open_pq: PQDict,
        current_pt,
        parent_nodes,
        g_matrix,
        goal,
        max_nodes,
    ):
        """Looks for valid neighbors of the current point and updates the open priority queue"""
        path_cost = 1
        x_c, y_c, z_c = current_pt
        current_pt_idx = self.get_grid_idx(current_pt)
        x_cells = self.map_length // self.resolution
        y_cells = self.map_width // self.resolution
        z_cells = self.map_height // self.resolution

        x_max, y_max, z_max = (
            (x_cells // 2) * self.resolution + self.map_center[0],
            (y_cells // 2) * self.resolution + self.map_center[1],
            (z_cells // 2) * self.resolution + self.map_center[2],
        )
        x_min, y_min, z_min = (
            (-x_cells // 2) * self.resolution + self.map_center[0],
            (-y_cells // 2) * self.resolution + self.map_center[1],
            (-z_cells // 2) * self.resolution + self.map_center[2],
        )
        if len(open_pq) > max_nodes:
            max_nodes = len(open_pq)
        for i in range(-1, 2):
            for j in range(-1, 2):
                for k in range(-1, 2):
                    x_n, y_n, z_n = (
                        x_c + i * self.resolution,
                        y_c + j* self.resolution,
                        z_c + k * self.resolution,
                    )
                    next_pt = (x_n, y_n, z_n)

                    # Check if point is within boundary

                    if (
                        (x_n >= x_min)
                        and (y_n >= y_min)
                        and (z_n >= z_min)
                        and (x_n < x_max)
                        and (y_n < y_max)
                        and (z_n >= self.ground_z)
                        and (z_n < z_max)
                    ):
                        next_pt_idx = self.get_grid_idx(next_pt)

                        if self.check_coordinate_validity(
                            next_pt
                        ):  # check occupancy map
                            if g_matrix[next_pt_idx] > g_matrix[
                                current_pt_idx
                            ] + self.cost_f(next_pt, current_pt):
                                g_matrix[next_pt_idx] = g_matrix[
                                    current_pt_idx
                                ] + self.cost_f(next_pt, current_pt)
                                parent_nodes[next_pt] = current_pt

                                try:
                                    open_pq.updateitem(
                                        next_pt,
                                        g_matrix[next_pt_idx]
                                        + path_cost * self.heuristic(next_pt, goal),
                                    )

                                except KeyError:
                                    open_pq.additem(
                                        next_pt,
                                        g_matrix[next_pt_idx]
                                        + path_cost * self.heuristic(next_pt, goal),
                                    )
        return max_nodes

    def cost_f(self, point1, point2):
        return point_distance(point1, point2)

    def heuristic(self, point, goal):
        return point_distance(point, goal)

    def get_path(self, parent_nodes, goal_pos):
        try:
            parent = parent_nodes[goal_pos]
            path = []
            goal_pos = list(goal_pos)
            goal_pos[2] *= -1
            path.append(goal_pos)

            while parent in parent_nodes:
                pos = parent_nodes[parent]
                pos = list(pos)
                # NEU to NED
                pos[2] *= -1
                path.append(pos)
                parent = parent_nodes[parent]

            path = path[::-1]
        except KeyError:
            projectairsim_log().info("No path found")
            path = []
        return path
