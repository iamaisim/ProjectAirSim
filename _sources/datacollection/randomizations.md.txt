# Object-Level Randomizations

The `Data Generation` module allows developers to introduce randomizations at the object level. That is, the assets being spawned in a scene can have a variety of their properties randomized. This allows for more variety in the collected data and leads to a more rounded dataset at the end. These randomizations can also be used to induce edge-case scenarios to the dataset; again for obtaining a better dataset.

The following properties can be randomized through the module:
* [Texture](#texture)
* [Scale](#scale)
* [Rotation](#rotation)
* [Translation](#translation)
* [Flip](#flip)

An example for configuring these randomizations at an asset level can be found in [`datacollector_config.jsonc`](./../../client/python/example_user_scripts/datacollection/configs/datacollector_config.jsonc#L26)

## Texture
Allows for the texture of the asset to be randomized. This feature can be configured using the following param:

````json
"texture": [ // List of different textures to choose from.
    "default",
    "shiny_texture",
    "matte_texture"
]
````
## Scale

Allows for the scale of the asset to be randomized. Scale is a unit-less multiple that is applied to the `x`, `y` and, `z` dimensions of the asset. This feature can be configured using the following param:

````json
"scale": {
    "amount": 2, // Number of variations to produce.
    "upper_bound": 100, // Upper bound of the scale factor.
    "lower_bound": 50 // Lower bound of the scale factor.
}
````

## Rotation

Allows for the rotational-orientation of the asset to be randomized. This type of randomization can be applied to either of the rotational axes - `roll`, `pitch` or `yaw`. This feature can be configured using the following param:

````json
"rotation": {
    "amount": 2, // Number of variations to produce.
    "upper_bound": 90.0, // Upper bound of rotation, in degrees.
    "lower_bound": -90.0, // Lower bound of rotation, in degrees.
    "axis": "roll" // Axis for rotation.
}
````

## Translation

Allows for the position of the asset to be randomized. This type of randomization can be applied to either of the cartesian axes - `x`, `y` or `z`. This feature can be configured using the following param:

````json
"rotation": {
    "amount": 2, // Number of variations to produce.
    "upper_bound": 90.0, // Upper bound of rotation, in degrees.
    "lower_bound": -90.0, // Lower bound of rotation, in degrees.
    "axis": "roll" // Axis for rotation.
}
````
## Flip

Allows for randomized flipping of the asset along an axis. This type of randomization can be applied to either of the rotational axes - `roll`, `pitch` or `yaw`. This feature can be configured using the following param:

````json
"flip": {
    "axis": "roll", // Axis for flipping.
    "initial_rpy": [
        20,
        0,
        0
    ] // Starting roll, pitch, yaw of the object.
}
````                                         