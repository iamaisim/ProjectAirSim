# Image Level Augmentations

The `Data Generation` module allows developers to introduce augmentations to the collected images. That is, enriching base dataset with operations like vertical and horizontal flipping. This allows for more variety in the collected data and leads to a more rounded dataset at the end. These augmentations can also be used to induce edge-case scenarios to the dataset; again for obtaining datasets that more closely represents the real-world scenarios.

The augmentation module can be enabled using the following param:
````json
"augmentation-spec": {
        "2DBBox": {
            "enabled": true
        },
}
````

Currently, the following properties can be augmented through the module:
* [Horizontal Flipping](#horizontal-flipping)
* [Vertical Flipping](#vertical-flipping)
* [Random Rotation](#random-rotation)
* [Random Cropping](#random-cropping)
* [Affine Transformation](#affine-transformation)
* [Brightness-Contrast](#brightness-contrast)
* [Hue-Saturation-Value](#hue-saturation-value)
* [Motion Blur](#motion-blur)
* [Gaussian Noise](#gaussian-noise)

An example for configuring these augmentations at an asset level can be found in [`datacollector_config.jsonc`](./../../client/python/example_user_scripts/datacollection/configs/datacollector_config.jsonc#L26)

Refer to [`augmentation APIs`](api.md#augmentation-apis) section from `api.md` for the API reference for the augmentation module.

## Horizontal Flipping
Allows for the flipping of the input horizontally around the y-axis. This feature can be configured using the following param:

````json
"horizontal-flip": { //Flip horizontally around the y-axis.
    "enabled": true,
    "p": 1 //probability of applying the transform. Default: 1.
}
````

## Vertical Flipping
Allows for the flipping of the input horizontally around the y-axis. This feature can be configured using the following param:

````json
"vertical-flip": { //Flip horizontally around the y-axis.
    "enabled": true,
    "p": 1 //probability of applying the transform. Default: 1.
}
````

## Random Cropping
Crops a random part of the input. This feature can be configured using the following param:

````json
"crop": { //Crop a random part of the input
    "enabled": true,
    "crop-height": 224, //Height of cropped image
    "crop-width": 224, //Width of cropped image
    "p": 1 //probability of applying the transform. Default: 1.
},
````

## Random Rotation
Rotate the input by an angle selected randomly from the uniform distribution. This feature can be configured using the following param:

````json
"rotate": { //Rotate the input by an angle selected randomly from the uniform distribution
    "enabled": true,
    "angular-limit": 180, //Rotation angle randomly selected from range(-limit, limit)
    "p": 1 //probability of applying the transform. Default: 1.
}, 
````

## Affine Transformation
Apply an affine transformation to the input image. This feature can be configured using the following param:

````json
"affine-transform": { //Apply affine transformations to image
    "enabled": true,
    "translate-percent": 20, //Translation as a fraction of the image height/width
    "scale": 0.5, //Scaling factor in. Param Range - (0,1)
    "shear": 30, //Shear image along x-axis with angle in range(-angle, angle)
    "p": 1
},
````

## Brightness-Contrast
Randomly change brightness and contrast of the input image. This feature can be configured using the following param:

````json
"brightness-contrast": { //Randomly change brightness and contrast of the input image
    "enabled": true,
    "brightness-limit": 0.5, //factor range for changing brightness. Select value from (0,1)
    "contrast-limit": 0.2, //factor range for changing contrast. Select value from (0,1)
    "p": 1
},
````

## Hue-Saturation-Value
Randomly change hue, saturation and value of the input image. This feature can be configured using the following param:

````json
"hue-saturation-value": { //Randomly change hue, saturation and value of the input image
    "enabled": true,
    "hue-shift-limit": 20, //hue shift randomly selected from range(-limit, limit)
    "saturation-shift-limit": 50, //sat shift randomly selected from range(-limit, limit)
    "value-shift-limit": 30, //value shift randomly selected from range(-limit, limit)
    "p": 1
},
````
## Motion Blur
Apply motion blur to the input image using a random-sized kernel. This feature can be configured using the following param:

````json
"motion-blur": { //Apply motion blur to the input image using a random-sized kernel
    "enabled": true,
    "blur-limit": 21, //maximum kernel size for blurring the input image. Must be a odd number
    "p": 1
},
````
## Gaussian Noise
Apply gaussian noise to the input image. This feature can be configured using the following param:

````json
"gaussian-noise": { //Apply gaussian noise to the input image
    "enabled": true,
    "variance-limit": 200, //variance range for noise 
    "p": 1
}
````
