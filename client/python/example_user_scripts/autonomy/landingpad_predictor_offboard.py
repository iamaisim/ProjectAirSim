"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.
ProjectAirSim:: Autonomy building-blocks:: Perception: LandingPad predictor
"""

import glob
import logging
import os
import pathlib
from argparse import ArgumentParser

import cv2
import numpy as np
from projectairsim.autonomy.perception import PosePredictor

logger = logging.getLogger("ProjectAirSim::Autonomy::Perception:landingpad_predictor")
handler = logging.StreamHandler()
formatter = logging.Formatter("[%(asctime)s] [%(name)s]  [%(levelname)s]  %(message)s")
handler.setFormatter(formatter)
logger.addHandler(handler)
logger.setLevel(logging.INFO)

parser = ArgumentParser("ProjectAirSim::Autonomy::Perception:landingpad_predictor")
parser.add_argument(
    "--images-dir", required=True, help="Path to directory containing input images"
)
parser.add_argument(
    "--pretrained-model", required=True, help="Path to pre-trained model."
)
parser.add_argument(
    "--interactive",
    action="store_true",
    default=False,
    help="Show predictions on every input image and wait for key press",
)
parser.add_argument(
    "--perception-arch",
    default="compass",
    help="Perception model architecture of the pre-trained model. Default=compass",
)
parser.add_argument(
    "--gpu-id",
    default=0,
    help="GPU device ID for running the perception model. Default=0",
)

args = parser.parse_args()

##### Perception module config
perception_arch = args.perception_arch
perception_pretrained_model = args.pretrained_model
perception_gpu = args.gpu_id
perception_module = PosePredictor(
    perception_arch,
    perception_pretrained_model,
    perception_gpu,
)
#####


def predict(input_image):
    # Convert cv2's BGR repr to RGB
    input_image = cv2.cvtColor(input_image, cv2.COLOR_BGR2RGB)
    # [x, y, w, h] unscaled
    output = perception_module.predict(input_image)
    # De-normalize outputs
    h, w = input_image.shape[0], input_image.shape[1]
    output = list(np.array(output) * np.array([w, h, w, h]))
    return output


if __name__ == "__main__":
    image_dir: str = args.images_dir
    logger.info(
        f"ProjectAirSim::Autonomy::Perception: LandingPad Predictor: Using images dir:{image_dir} "
    )
    logger.info(f"Found {glob.glob('{args.images_dir}/*.png')} images.")
    for img_name in glob.glob(f"{args.images_dir}/*.png"):
        logger.info(f"Loading:{img_name}")
        image_bgr = cv2.imread(img_name)
        prediction = predict(image_bgr)
        x_c, y_c, w, h = list(prediction)
        logger.info(f"Prediction:x_c:{x_c} y_c:{y_c} w:{w} h:{h}")
        # <Use the prediction>
        # Example: Draw a bounding box on the image based on the prediction
        cv2.rectangle(
            img=image_bgr,
            pt1=(int(x_c - w / 2), int(y_c - h / 2)),
            pt2=(int(x_c + w / 2), int(y_c + h / 2)),
            color=(0, 0, 255),  # Red
            thickness=2,
        )
        if args.interactive:
            # Display the predicted bounding box
            cv2.imshow("Prediction", image_bgr)
            cv2.waitKey(0)  # Waits for a keyboard keypress event
        else:
            # Save the prediction on image to disk
            output_dir = "outputs"
            pathlib.Path(output_dir).mkdir(exist_ok=True)
            output_fname = os.path.join(output_dir, os.path.basename(img_name))
            if not cv2.imwrite(output_fname, image_bgr):
                raise Exception(f"cv2 could not write output image to {output_fname}")
