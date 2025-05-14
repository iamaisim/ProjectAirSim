"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Script implements an A* planner in python to compute an optimized route
between two points in space (NED coordinates)
The main function of the script is `planner()` which takes in a voxel gird occupancy map,
a start and end positions as `Waypoint` objects and the length of the desired trajectory
"""

import numpy as np
from pqdict import pqdict
import copy
from typing import List

from projectairsim.datacollection.types import OccupancyMap, WayPoint
import projectairsim.datacollection.utils as utils
from projectairsim.utils import projectairsim_log


def astar(
    open_pq: pqdict,
    parent_nodes: dict,
    g_matrix: List[float],
    map: OccupancyMap,
    goal: WayPoint,
    step_len: int,
):
    min_dist = step_len
    max_nodes = 0
    while True:
        try:
            current_pose: WayPoint = open_pq.pop()
        except KeyError:
            projectairsim_log().info("No paths found")
            break

        if utils.dist_xyz(current_pose, goal) < min_dist:
            projectairsim_log().info("Path found successfully")
            break

        max_nodes = neighbourhood_search(
            open_pq,
            current_pose,
            parent_nodes,
            g_matrix,
            map,
            goal,
            max_nodes,
            step_len,
        )

    projectairsim_log().info(f"Max nodes = {max_nodes}")

    return current_pose


def neighbourhood_search(
    open_pq: pqdict,
    current_pose: WayPoint,
    parent_nodes: dict,
    g_matrix: List[int],
    map: OccupancyMap,
    goal: WayPoint,
    max_nodes: int,
    step_len: int,
):
    path_cost = 1
    current_pose_idx = map.get_grid_idx(current_pose)

    if len(open_pq) > max_nodes:
        max_nodes = len(open_pq)
    for i in range(-step_len, 2 * step_len, step_len):
        for j in range(-step_len, 2 * step_len, step_len):
            for k in range(-step_len, 2 * step_len, step_len):
                next_pose = copy.deepcopy(current_pose)

                next_pose.pose_x += i
                next_pose.pose_y += j
                next_pose.pose_z += k

                # Check if point is within boundary

                if map.check_validity(next_pose):
                    next_pose_idx = map.get_grid_idx(next_pose)

                    if g_matrix[next_pose_idx] > g_matrix[current_pose_idx] + cost_f(
                        next_pose, current_pose
                    ):
                        g_matrix[next_pose_idx] = g_matrix[current_pose_idx] + cost_f(
                            next_pose, current_pose
                        )
                        parent_nodes[next_pose] = current_pose

                        try:
                            open_pq.updateitem(
                                next_pose,
                                g_matrix[next_pose_idx]
                                + path_cost * heuristic(next_pose, goal),
                            )

                        except KeyError:
                            open_pq.additem(
                                next_pose,
                                g_matrix[next_pose_idx]
                                + path_cost * heuristic(next_pose, goal),
                            )
    return max_nodes


def cost_f(pose1: WayPoint, pose2: WayPoint):
    return utils.dist_xyz(pose1, pose2)


def heuristic(pose: WayPoint, goal: WayPoint):
    return utils.dist_xyz(pose, goal)


def get_path(parent_nodes: dict, goal: WayPoint):
    parent = parent_nodes[goal]
    path = []
    goal.pose_z *= -1  # NEU to NED
    path.append(goal)

    while parent in parent_nodes:
        pos = parent_nodes[parent]
        pos.pose_z *= -1  # NEU to NED
        path.append(pos)
        parent = parent_nodes[parent]

    path = path[::-1]
    return path


def planner(map: OccupancyMap, start: WayPoint, goal: WayPoint, num_poses: int):
    step_len = int(utils.dist_xyz(start, goal) // num_poses)

    # NOTE: The planner works in NEU coordinates
    # NOTE: Pre-process NED coordinates to NEU
    # NOTE: Post process NEU coordinates to NED

    start_neu = start
    goal_neu = goal

    start_neu.pose_z *= -1
    goal_neu.pose_z *= -1

    open_pq = pqdict()
    open_pq.additem(start_neu, 0)
    g_matrix = copy.deepcopy(map.occupancy_map)  # occupancy map
    g_matrix[:] = [np.inf for x in g_matrix]
    start_pos_idx = map.get_grid_idx(start_neu)
    g_matrix[start_pos_idx] = 0
    parent_nodes = {}
    last_pose = astar(open_pq, parent_nodes, g_matrix, map, goal_neu, step_len)
    path = get_path(parent_nodes, last_pose)

    return path
