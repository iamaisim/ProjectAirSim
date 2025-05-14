// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License.

#ifndef PHYSICS_TEST_TEST_DATA_PHYSICS_TEST_CONFIG_HPP_
#define PHYSICS_TEST_TEST_DATA_PHYSICS_TEST_CONFIG_HPP_

// JSON Data as a string. Needed since the new load methods take in the string
// literals as a parameter and not the file name to avoid parsing on server
// side. This config is just copied directly from scene_basic_drone.json and
// robot_quadrotor_fastphysics.json as the base config for physics unit tests.
constexpr const char* physics_test_config = R"(
{
  "id": "SceneBasicDrone",
  "actors": [
    {
      "type": "robot",
      "name": "Drone1",
      "origin": {
        "xyz": "0.0 0.0 -4.0",
        "rpy-deg": "0 0 0"
      },
      "robot-config": {
        "physics-type": "fast-physics",
        "links": [
          {
            "name": "Frame",
            "inertial": {
              "mass": 1.0,
              "inertia": {
                "type": "geometry",
                "geometry": {
                  "box": {
                    "size": "0.180 0.110 0.040"
                  }
                }
              },
              "aerodynamics": {
                "drag-coefficient": 0.325,
                "type": "geometry",
                "geometry": {
                  "box": {
                    "size": "0.180 0.110 0.040"
                  }
                }
              }
            },
            "collision": {
              "restitution": 0.1,
              "friction": 0.5
            },
            "visual": {
              "geometry": {
                "type": "unreal_mesh",
                "name": "/Drone/Quadrotor1"
              }
            }
          },
          {
            "name": "Prop_FL",
            "inertial": {
              "origin": {
                "xyz": "0.253 -0.253 -0.01",
                "rpy-deg": "0 0 0"
              },
              "mass": 0.055,
              "inertia": {
                "type": "point-mass"
              },
              "aerodynamics": {
                "drag-coefficient": 0.325,
                "type": "geometry",
                "geometry": {
                  "cylinder": {
                    "radius": 0.1143,
                    "length": 0.01
                  }
                }
              }
            },
            "visual": {
              "origin": {
                "xyz": "0.253 -0.253 -0.01",
                "rpy-deg": "0 0 0"
              },
              "geometry": {
                "type": "unreal_mesh",
                "name": "/Drone/PropellerRed"
              }
            }
          },
          {
            "name": "Prop_FR",
            "inertial": {
              "origin": {
                "xyz": "0.253 0.253 -0.01",
                "rpy-deg": "0 0 0"
              },
              "mass": 0.055,
              "inertia": {
                "type": "point-mass"
              },
              "aerodynamics": {
                "drag-coefficient": 0.325,
                "type": "geometry",
                "geometry": {
                  "cylinder": {
                    "radius": 0.1143,
                    "length": 0.01
                  }
                }
              }
            },
            "visual": {
              "origin": {
                "xyz": "0.253 0.253 -0.01",
                "rpy-deg": "0 0 0"
              },
              "geometry": {
                "type": "unreal_mesh",
                "name": "/Drone/PropellerRed"
              }
            }
          },
          {
            "name": "Prop_RL",
            "inertial": {
              "origin": {
                "xyz": "-0.253 -0.253 -0.01",
                "rpy-deg": "0 0 0"
              },
              "mass": 0.055,
              "inertia": {
                "type": "point-mass"
              },
              "aerodynamics": {
                "drag-coefficient": 0.325,
                "type": "geometry",
                "geometry": {
                  "cylinder": {
                    "radius": 0.1143,
                    "length": 0.01
                  }
                }
              }
            },
            "visual": {
              "origin": {
                "xyz": "-0.253 -0.253 -0.01",
                "rpy-deg": "0 0 0"
              },
              "geometry": {
                "type": "unreal_mesh",
                "name": "/Drone/PropellerWhite"
              }
            }
          },
          {
            "name": "Prop_RR",
            "inertial": {
              "origin": {
                "xyz": "-0.253 0.253 -0.01",
                "rpy-deg": "0 0 0"
              },
              "mass": 0.055,
              "inertia": {
                "type": "point-mass"
              },
              "aerodynamics": {
                "drag-coefficient": 0.325,
                "type": "geometry",
                "geometry": {
                  "cylinder": {
                    "radius": 0.1143,
                    "length": 0.01
                  }
                }
              }
            },
            "visual": {
              "origin": {
                "xyz": "-0.253 0.253 -0.01",
                "rpy-deg": "0 0 0"
              },
              "geometry": {
                "type": "unreal_mesh",
                "name": "/Drone/PropellerWhite"
              }
            }
          }
        ],
        "joints": [
          {
            "id": "Frame_Prop_FL",
            "type": "fixed",
            "parent-link": "Frame",
            "child-link": "Prop_FL",
            "axis": "0 0 1"
          },
          {
            "id": "Frame_Prop_FR",
            "type": "fixed",
            "parent-link": "Frame",
            "child-link": "Prop_FR",
            "axis": "0 0 1"
          },
          {
            "id": "Frame_Prop_RL",
            "type": "fixed",
            "parent-link": "Frame",
            "child-link": "Prop_RL",
            "axis": "0 0 1"
          },
          {
            "id": "Frame_Prop_RR",
            "type": "fixed",
            "parent-link": "Frame",
            "child-link": "Prop_RR",
            "axis": "0 0 1"
          }
        ],
        "controller": {
          "id": "Simple_Flight_Controller",
          "type": "simple-flight-api",
          "simple-flight-api-settings": {
            "actuator-order": [
              {
                "id": "Prop_FR_actuator"
              },
              {
                "id": "Prop_RL_actuator"
              },
              {
                "id": "Prop_FL_actuator"
              },
              {
                "id": "Prop_RR_actuator"
              }
            ]
          }
        },
        "actuators": [
          {
            "name": "Prop_FL_actuator",
            "type": "rotor",
            "enabled": true,
            "parent-link": "Frame",
            "child-link": "Prop_FL",
            "origin": {
              "xyz": "0.253 -0.253 -0.01",
              "rpy-deg": "0 0 0"
            },
            "rotor-settings": {
              "turning-direction": "clock-wise",
              "normal-vector": "0.0 0.0 -1.0",
              "coeff-of-thrust": 0.109919,
              "coeff-of-torque": 0.040164,
              "max-rpm": 6396.667,
              "propeller-diameter": 0.2286,
              "smoothing-tc": 0.005
            }
          },
          {
            "name": "Prop_FR_actuator",
            "type": "rotor",
            "enabled": true,
            "parent-link": "Frame",
            "child-link": "Prop_FR",
            "origin": {
              "xyz": "0.253 0.253 -0.01",
              "rpy-deg": "0 0 0"
            },
            "rotor-settings": {
              "turning-direction": "counter-clock-wise",
              "normal-vector": "0.0 0.0 -1.0",
              "coeff-of-thrust": 0.109919,
              "coeff-of-torque": 0.040164,
              "max-rpm": 6396.667,
              "propeller-diameter": 0.2286,
              "smoothing-tc": 0.005
            }
          },
          {
            "name": "Prop_RL_actuator",
            "type": "rotor",
            "enabled": true,
            "parent-link": "Frame",
            "child-link": "Prop_RL",
            "origin": {
              "xyz": "-0.253 -0.253 -0.01",
              "rpy-deg": "0 0 0"
            },
            "rotor-settings": {
              "turning-direction": "counter-clock-wise",
              "normal-vector": "0.0 0.0 -1.0",
              "coeff-of-thrust": 0.109919,
              "coeff-of-torque": 0.040164,
              "max-rpm": 6396.667,
              "propeller-diameter": 0.2286,
              "smoothing-tc": 0.005
            }
          },
          {
            "name": "Prop_RR_actuator",
            "type": "rotor",
            "enabled": true,
            "parent-link": "Frame",
            "child-link": "Prop_RR",
            "origin": {
              "xyz": "-0.253 0.253 -0.01",
              "rpy-deg": "0 0 0"
            },
            "rotor-settings": {
              "turning-direction": "clock-wise",
              "normal-vector": "0.0 0.0 -1.0",
              "coeff-of-thrust": 0.109919,
              "coeff-of-torque": 0.040164,
              "max-rpm": 6396.667,
              "propeller-diameter": 0.2286,
              "smoothing-tc": 0.005
            }
          }
        ],
        "sensors": [
          {
            "id": "DownCamera",
            "type": "camera",
            "enabled": true,
            "parent-link": "Frame",
            "capture-interval": 0.001,
            "capture-settings": [
              {
                "image-type": 0,
                "width": 400,
                "height": 225,
                "fov-degrees": 90,
                "capture-enabled": true,
                "pixels-as-float": false,
                "compress": false,
                "target-gamma": 2.5
              },
              {
                "image-type": 1,
                "width": 400,
                "height": 225,
                "fov-degrees": 90,
                "capture-enabled": false,
                "pixels-as-float": false,
                "compress": false
              },
              {
                "image-type": 2,
                "width": 400,
                "height": 225,
                "fov-degrees": 90,
                "capture-enabled": true,
                "pixels-as-float": false,
                "compress": false
              },
              {
                "image-type": 3,
                "width": 400,
                "height": 225,
                "fov-degrees": 90,
                "capture-enabled": false,
                "pixels-as-float": false,
                "compress": false
              }
            ],
            "noise-settings": [
              {
                "enabled": false,
                "image-type": 1,
                "rand-contrib": 0.2,
                "rand-speed": 100000.0,
                "rand-size": 500.0,
                "rand-density": 2,
                "horz-wave-contrib": 0.03,
                "horz-wave-strength": 0.08,
                "horz-wave-vert-size": 1.0,
                "horz-wave-screen-size": 1.0,
                "horz-noise-lines-contrib": 1.0,
                "horz-noise-lines-density-y": 0.01,
                "horz-noise-lines-density-xy": 0.5,
                "horz-distortion-contrib": 1.0,
                "horz-distortion-strength": 0.002
              }
            ],
            "gimbal": {
              "stabilization": 1,
              "rpy-deg": "0 0 0"
            },
            "origin": {
              "xyz": "0 0.0 0.0",
              "rpy-deg": "0 -90 0"
            }
          },
          {
            "id": "IMU1",
            "type": "imu",
            "enabled": true,
            "parent-link": "Frame",
            "accelerometer": {
              "velocity-random-walk": 0.0123,
              "tau": 800,
              "bias-stability": 2e-5,
              "turn-on-bias": "0 0 0"
            },
            "gyroscope": {
              "angle-random-walk": 0.0123,
              "tau": 500,
              "bias-stability": 1e-6,
              "turn-on-bias": "0 0 0"
            }
          },
          {
            "id": "GPS",
            "type": "gps",
            "enabled": false,
            "parent-link": "Frame"
          }
        ]
      }
    }
  ],
  "clock": {
    "type": "steppable",
    "step-ns": 3000000,
    "real-time-update-rate": 3000000,
    "pause-on-start": false
  },
  "home-geo-point": {
    "latitude": 47.641468,
    "longitude": -122.140165,
    "altitude": 122.0
  },
  "segmentation": {
    "initialize-ids": true,
    "ignore-existing": false,
    "use-owner-name": true
  }
}
)";

#endif  // PHYSICS_TEST_TEST_DATA_PHYSICS_TEST_CONFIG_HPP_
