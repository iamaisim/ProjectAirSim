from unittest.mock import patch

import numpy as np

from projectairsim.image_utils import ImageDisplay
from projectairsim.utils import unpack_image


def _depth_message(values):
    depth = np.asarray(values, dtype=np.float16)
    return {
        "encoding": "16FC1",
        "height": depth.shape[0],
        "width": depth.shape[1],
        "data": depth.tobytes(),
        "annotations": [],
    }


def test_unpack_image_preserves_metric_float16_depth():
    image_msg = _depth_message([[0.0, 25.0], [100.0, np.inf]])

    unpacked = unpack_image(image_msg)

    assert unpacked.dtype == np.float16
    np.testing.assert_array_equal(
        unpacked, np.array([[0.0, 25.0], [100.0, np.inf]], dtype=np.float16)
    )


def test_display_image_converts_float16_depth_for_opencv():
    image_msg = _depth_message([[0.0, 25.0], [100.0, np.inf]])

    with patch("projectairsim.image_utils.cv2.imshow") as imshow:
        ImageDisplay().display_image(image_msg, "Depth")

    displayed = imshow.call_args.args[1]
    assert displayed.dtype == np.float32
    np.testing.assert_allclose(displayed, [[0.0, 0.25], [1.0, 1.0]])
