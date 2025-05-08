inkling "2.0"
using Goal
using Math
type ObsState {
    pose: number[4],
    displacement_to_goal: number[3],
    distance_to_obstacles: number[3],
}
type Action {
    Vx: number<-5.0 .. 5.0>,
    Vy: number<-5.0 .. 5.0>,
    Vz: number<-5.0 .. 5.0>,
}
type ProjectAirSimConfig {
    episode_length: number,
    step_limit: number,
    goals: {
        v_vertex: {
            x: number,
            y: number,
            z: number
        },
        v_apex_left: {
            x: number,
            y: number,
            z: number
        },
        v_apex_right: {
            x: number,
            y: number,
            z: number
        },
    },
    num_obstacles: number,
    obstacles_config: {
        PowerTower1: {
            type: "PowerTower",
            pose: {
                translation: {
                    x: number,
                    y: number,
                    z: number
                },
                rotation: {
                    w: number,
                    x: number,
                    y: number,
                    z: number
                },
            },
            scale: {
                x: number,
                y: number,
                z: number
            },
            enable_physics: number,
        },
        PowerTower2: {
            type: "PowerTower",
            pose: {
                translation: {
                    x: number,
                    y: number,
                    z: number
                },
                rotation: {
                    w: number,
                    x: number,
                    y: number,
                    z: number
                },
            },
            scale: {
                x: number,
                y: number,
                z: number
            },
            enable_physics: number,
        },
        Building1: {
            type: "Building",
            pose: {
                translation: {
                    x: number,
                    y: number,
                    z: number
                },
                rotation: {
                    w: number,
                    x: number,
                    y: number,
                    z: number
                },
            },
            scale: {
                x: number,
                y: number,
                z: number
            },
            enable_physics: number,
        },
    }
}
simulator detectavoid_simulator(action: Action, config: ProjectAirSimConfig): ObsState {
    package "ProjectAirSim-DetectAvoid-reconfigurable-v8-nogpu"
}
const DIST_SCALE = 1E1
const TERMINAL_DISIT_RATIO = 1.2
graph (input: ObsState): Action {
    concept detectandavoid(input): Action {
        curriculum {
            source detectavoid_simulator
            training {
                EpisodeIterationLimit: 400,
                NoProgressIterationLimit: 2000000,
            }
            goal (state: ObsState) {
                reach move_to_target:
                    state.displacement_to_goal
                    in Goal.Sphere([0.0, 0.0, 0.0], 4)
                avoid `obstacle 1 collision`:
                    state.distance_to_obstacles[0]
                    in Goal.Range(0, 5)
                avoid `obstacle 2 collision`:
                    state.distance_to_obstacles[1]
                    in Goal.Range(0, 5)
                avoid `obstacle 3 collision`:
                    state.distance_to_obstacles[2]
                    in Goal.Range(0, 5)

            }
            lesson detectavoid1 {
                scenario {
                    episode_length: -1,
                    num_obstacles: 3,
                    step_limit: 400,
                    goals: {
                        v_vertex: {
                            x: 75.3,
                            y: 399.4,
                            z: -1.1
                        },
                        v_apex_left: {
                            x: 136.5,
                            y: 370.7,
                            z: -1.1
                        },
                        v_apex_right: {
                            x: 133.8,
                            y: 422.4,
                            z: -1.1
                        },
                    },
                    obstacles_config: {
                        PowerTower1: {
                            type: "PowerTower",
                            pose: {
                                translation: {
                                    x: 64,
                                    y: 372,
                                    z: -1
                                },
                                rotation: {
                                    w: 0,
                                    x: 0,
                                    y: 0,
                                    z: 0
                                },
                            },
                            scale: {
                                x: 1,
                                y: 1,
                                z: 1
                            },
                            enable_physics: 0,
                        },
                        PowerTower2: {
                            type: "PowerTower",
                            pose: {
                                translation: {
                                    x: 65.2,
                                    y: 420,
                                    z: -1
                                },
                                rotation: {
                                    w: 0,
                                    x: 0,
                                    y: 0,
                                    z: 0
                                },
                            },
                            scale: {
                                x: 1,
                                y: 1,
                                z: 1
                            },
                            enable_physics: 0,
                        },
                        Building1: {
                            type: "Building",
                            pose: {
                                translation: {
                                    x: 105.0,
                                    y: 400.0,
                                    z: -1.0
                                },
                                rotation: {
                                    w: 0,
                                    x: 0,
                                    y: 0,
                                    z: 0
                                },
                            },
                            scale: {
                                x: 1.0,
                                y: 1.0,
                                z: 1.0
                            },
                            enable_physics: 0
                        },
                    }
                }
            }
        }
    }
    output detectandavoid
}
