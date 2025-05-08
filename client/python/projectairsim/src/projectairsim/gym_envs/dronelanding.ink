inkling "2.0"

using Math
using Number

const CloseEnough = 1.23
const Scale = 10

function Reward(gs: GameState) {
    return gs._gym_reward
}

function Terminal(gs: GameState) {
    return gs._gym_terminal
}

type GameState {
    predicted_actor_x: Number.Float32,
    predicted_actor_y: Number.Float32,
    predicted_actor_z: Number.Float32,
    predicted_actor_yaw: Number.Float32,

    true_actor_x: Number.Float32,
    true_actor_y: Number.Float32,
    true_actor_z: Number.Float32,
    true_actor_roll: Number.Float32,
    true_actor_pitch: Number.Float32,
    true_actor_yaw: Number.Float32,

    landing_pad_x: Number.Float32,
    landing_pad_y: Number.Float32,
    landing_pad_z: Number.Float32,
    landing_pad_roll: Number.Float32,
    landing_pad_pitch: Number.Float32,
    landing_pad_yaw: Number.Float32,

    diff_x: Number.Float32,
    diff_y: Number.Float32,
    diff_z: Number.Float32,
    diff_yaw: Number.Float32,

    _gym_reward: number,
    _gym_terminal: number,
    _gym_count: number,
}

type Action {
    Vx: Number.Float32,
    Vy: Number.Float32,
    Vz: Number.Float32,
}

type ObservableState {
    diff_x: Number.Float32,
    diff_y: Number.Float32,
    diff_z: Number.Float32,
    diff_yaw: Number.Float32,
}


type ProjectAirSimDroneLandingConfig{
    episode_length: Number.Int8,
}

simulator dronelanding_simulator(action: Action, config: ProjectAirSimDroneLandingConfig): GameState {
}

function dist_func (gs: GameState) : {
    return Math.Sqrt((gs.landing_pad_x - gs.true_actor_x) ** 2 +
		     (gs.landing_pad_y - gs.true_actor_y) ** 2 +
                     (gs.landing_pad_z - gs.true_actor_z) ** 2)
}

function reward_func (gs: GameState) : {
    var d = dist_func(gs)
    return (CloseEnough / d) * Scale
}

graph (input: ObservableState): Action {

    concept land(input): Action {
        curriculum {
            reward reward_func
            terminal Terminal
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
