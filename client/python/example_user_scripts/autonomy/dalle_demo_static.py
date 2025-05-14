"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
ProjectAirSim:: Autonomy building-blocks:: DALL-E Demo standalone
"""
import commentjson
import os
import jsonschema
import cv2
import numpy as np
from projectairsim.autonomy.perception import PosePredictor
from dalle_api import DalleImageGenerator

from projectairsim import Drone, ProjectAirSimClient, World


def read_cjson(json_file: str):
    with open(json_file) as f:
        data = commentjson.load(f)
    return data


def validate_config(config, schema):
    try:
        jsonschema.validate(instance=config, schema=schema)
    except jsonschema.exceptions.ValidationError as err:
        raise err


# Load and validate App config
app_config_fname = "dalle_app_config.jsonc"
app_config_schema_fname = "perception_app_config_schema.jsonc"
config_path = os.path.join("configs", app_config_fname)
config_schema_path = os.path.join("configs", "schema", app_config_schema_fname)
input_images_path = os.path.join("images", "inputs")
config = read_cjson(config_path)
schema = read_cjson(config_schema_path)
validate_config(config, schema)

# Load the Perception module
perception_arch = config.get("perception-arch")
perception_pretrained_model = config.get("perception-pretrained-model")
perception_gpu = config.get("perception-gpu")
perception_module = PosePredictor(
    perception_arch,
    perception_pretrained_model,
    perception_gpu,
)


def predict(image_packed):

    img = np.array(image_packed)
    w, h = 600, 600
    img_np = np.reshape(img, [h, w, 3])
    # Convert BGR to RGB
    img_np = img_np[:, :, ::-1].copy()
    # [x, y, w, h] unscaled
    output = perception_module.predict(img_np)
    # Scale outputs;
    # Currently GT uses CVRect; Where (x, y) is (col, row)
    if config.get("bb-normalized"):
        output = list(np.multiply(output, [w, h, w, h]))
    print("bbcoords: ", output)
    return output


def main():
    image_filenames = os.listdir(input_images_path)
    # Read all the images from image folder in a for loop
    i = 0
    for img in image_filenames:
        rgb = cv2.imread(os.path.join(input_images_path, img))
        i += 1
        # Detect the landing pad in the image
        prediction = predict(rgb)
        # Create a mask for the landing pad
        mask = np.zeros((600, 600, 4), dtype=np.uint8)
        x_c, y_c, w, h = list(prediction)
        # Make the image  fully transparent using the alpha channel except for the landing pad bounding box
        # The alpha channel is the 4th channel in the
        # RGBA image. The alpha channel is used to control the transparency of an image.
        # 0 is fully transparent and 255 is fully opaque.
        mask[:, :, 3] = 0  # set alpha channel to 0
        # Set the alpha channel to 255 for the landing pad bounding box
        # The bounding box is the area of the image that is not transparent
        mask[
            int(y_c - h / 2) : int(y_c + h / 2), int(x_c - w / 2) : int(x_c + w / 2), 3
        ] = 255

        # cv2.rectangle(
        #     mask,
        #     (int(x_c - w / 2), int(y_c - h / 2)),
        #     (int(x_c + w / 2), int(y_c + h / 2)),
        #     255,
        #     -1,  # -1 to fill the rectangle
        # )

        # Create images directory if it doesn't exist
        if not os.path.exists("images"):
            os.makedirs("images")
        # Save the rgb image and mask to disk
        cv2.imwrite((os.path.join("images", f"rgb_{i}.png")), rgb)
        # Write the RGBA mask to disk
        cv2.imwrite((os.path.join("images", f"mask_{i}.png")), mask)
        # Create output directory if it doesn't exist
        if not os.path.exists("output"):
            os.makedirs("output")
        # loop through the lines from prompt input json file
        j = 0
        for line in open(os.path.join("prompts.txt")):
            # Use the DallE Edit API
            j += 1
            DalleImageGenerator.edit(
                prompt=line,
                image_path=f"images/rgb_{i}.png",
                mask_path=f"images/mask_{i}.png",
                num=10,
                size="512x512",
                output_dir=f"./output/image_{i}/prompt_{j}",
            )


def display_image_with_bb(
    image, win_name: str, bb_coords: list, resize_x: int = None, resize_y: int = None
):
    """Display the image using OpenCV HighGUI"""

    if image is None:
        return

    # Unpack image data
    img = np.array(image["data"], dtype="uint8")
    img_cv = np.reshape(img, (image["height"], image["width"], 3))
    x_c, y_c, w, h = list(bb_coords)
    cv2.rectangle(
        img_cv,
        (int(x_c - w / 2), int(y_c - h / 2)),
        (int(x_c + w / 2), int(y_c + h / 2)),
        (0, 0, 233),
        2,
    )

    # Resize image if requested
    if resize_x is not None and resize_y is not None:
        img_cv = cv2.resize(
            img_cv, (resize_x, resize_y), interpolation=cv2.INTER_LINEAR
        )

    # Display image
    cv2.imshow(win_name, img_cv)


def display(image, image_name, bb_coords):
    global window_created
    # Create window if not already visible
    if not window_created:
        cv2.namedWindow(
            image_name,
            flags=cv2.WINDOW_GUI_NORMAL + cv2.WINDOW_AUTOSIZE,
        )
        window_created = True

    display_image_with_bb(
        image,
        image_name,
        bb_coords,
    )

    key = cv2.waitKey(1)  # expensive, can take minimum 5~15 ms
    if key == 27:  # Esc key
        cv2.destroyAllWindows()


if __name__ == "__main__":
    main()  # Runner function

