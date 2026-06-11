"""
Shared helpers for RGB image validation in pytest (non-test module; not collected).

Uses OpenCV + NumPy only (already project dependencies).
"""

from __future__ import annotations

import os
from typing import Optional, Tuple

import cv2
import numpy as np


def grayscale_normalized_cross_correlation(
    img_bgr: np.ndarray, ref_bgr: np.ndarray
) -> float:
    """Return Pearson correlation of flattened grayscale patches (1.0 = identical up to affine scaling).

    Both images are resized to the reference dimensions before comparison.
    """
    if ref_bgr is None or ref_bgr.size == 0 or img_bgr is None or img_bgr.size == 0:
        return 0.0
    h, w = ref_bgr.shape[:2]
    cur = cv2.resize(img_bgr, (w, h), interpolation=cv2.INTER_AREA)
    g1 = cv2.cvtColor(cur, cv2.COLOR_BGR2GRAY).astype(np.float64)
    g2 = cv2.cvtColor(ref_bgr, cv2.COLOR_BGR2GRAY).astype(np.float64)
    g1 -= g1.mean()
    g2 -= g2.mean()
    n = float(np.linalg.norm(g1.ravel()) * np.linalg.norm(g2.ravel()))
    if n < 1e-9:
        return 0.0
    return float(np.dot(g1.ravel(), g2.ravel()) / n)


def assert_rgb_scene_image_valid(
    img_msg: dict,
    *,
    reference_image_path: str = "",
    min_similarity_to_reference: float = 0.85,
    min_gray_std: float = 2.0,
) -> Tuple[Optional[float], np.ndarray]:
    """Decode a scene camera message and assert it carries plausible image data.

    Always checks non-empty buffer, shape, and non-flat intensity (std on grayscale).

    If ``reference_image_path`` is set (non-empty and file exists), also asserts
    normalized grayscale correlation to that reference is >= ``min_similarity_to_reference``.

    Returns:
        (similarity_or_none, bgr_image) for optional extra assertions by the caller.
    """
    assert img_msg is not None
    assert "data" in img_msg and "width" in img_msg and "height" in img_msg
    w, h = int(img_msg["width"]), int(img_msg["height"])
    assert w > 0 and h > 0
    raw = img_msg["data"]
    if isinstance(raw, list):
        nparr = np.array(raw, dtype=np.uint8)
    else:
        nparr = np.frombuffer(raw, dtype=np.uint8)
    expected = w * h * 3
    assert nparr.size >= expected, f"buffer too small: {nparr.size} < {expected}"
    bgr = nparr[:expected].reshape((h, w, 3))
    gray = cv2.cvtColor(bgr, cv2.COLOR_BGR2GRAY)
    assert float(gray.std()) >= min_gray_std, "image looks flat or empty (low std)"

    sim: Optional[float] = None
    path = (reference_image_path or "").strip()
    if path and os.path.isfile(path):
        ref = cv2.imread(path, cv2.IMREAD_COLOR)
        assert ref is not None and ref.size > 0, f"failed to load reference: {path}"
        sim = grayscale_normalized_cross_correlation(bgr, ref)
        assert sim >= min_similarity_to_reference, (
            f"scene image similarity {sim:.4f} below minimum "
            f"{min_similarity_to_reference:.4f} vs reference {path}"
        )
        print("similarity to reference:", sim)
    return sim, bgr
