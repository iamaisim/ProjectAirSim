inkling "2.0"
using Number
using Math

function Reward(gs: GameState) {
    return gs._gym_reward
}

function Terminal(gs: GameState) {
    return gs._gym_terminal
}

type GameState {
    actor_x: Number.Float32,
    actor_y: Number.Float32,
    actor_z: Number.Float32,
    actor_yaw: Number.Float32,

    true_lp_x: Number.Float32,
    true_lp_y: Number.Float32,
    true_lp_z: Number.Float32,

    true_act_x: Number.Float32,
    true_act_y: Number.Float32,
    true_act_z: Number.Float32,
    true_act_roll: Number.Float32,
    true_act_pitch: Number.Float32,
    true_act_yaw: Number.Float32,

    # This requires running simulator with either --esn or --esnca
    # esn: number[100],

    _gym_reward: number,
    _gym_terminal: number,
    _gym_count: number,
}

type ObsState {
    actor_x: Number.Float32,
    actor_y: Number.Float32,
    actor_z: Number.Float32,
    actor_yaw: Number.Float32,

    # This requires running simulator with either --esn or --esnca
    # esn: number[100],
}

type Action {
    Vx: Number.Float32<-5.0..5.0>,
    Vy: Number.Float32<-5.0..5.0>,
    Vz: Number.Float32<-5.0..5.0>,
}

type ProjectAirSimDroneLandingConfig{
    episode_length: Number.Int8,
}

simulator dronelanding_simulator(action: Action, config: ProjectAirSimDroneLandingConfig): GameState {
}

const DIST_SCALE=1E1
const TERMINAL_DISIT_RATIO = 1.2

function f_dist (s: GameState) : number {
    return Math.Sqrt((s.true_lp_x - s.true_act_x) **2 +
    (s.true_lp_y - s.true_act_y) ** 2 +
    (s.true_lp_z - s.true_act_z) ** 2)
}

function f_reward(s:GameState) : number {

    var d = f_dist(s) / DIST_SCALE * Math.Log(0.01)
    var k = 4.0

    return k * Math.E ** d - (k-1) * Math.E ** (d * k / (k-1))
}

function f_terminal(s:GameState) : number {
    var d = f_dist(s)
    return s._gym_terminal # or d > TERMINAL_DISIT_RATIO * DIST_SCALE
}

graph (input: ObsState): Action {

    concept land(input): Action {
        curriculum {
            training {
                EpisodeIterationLimit: 280,
                NoProgressIterationLimit: 1500000,
            }
            reward f_reward
            terminal f_terminal
            source dronelanding_simulator
            lesson landing{
                scenario {
                    episode_length: -1,
                }
            }
        }
    }
    output land
}

