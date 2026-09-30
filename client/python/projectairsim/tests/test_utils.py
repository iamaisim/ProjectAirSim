"""
Copyright (C) 2025 IAMAI CONSULTING CORP
MIT License.
Tests for configuration merge helpers.
"""

from projectairsim.utils import merge_lists


def test_merge_lists_matches_name_and_id_keys():
    base = [
        {"name": "Gimbal", "enabled": False},
        {"id": "DownCamera", "gimbal": {"lock-roll": False}},
        {"image-type": 0, "streaming-enabled": False},
    ]
    overlay = [
        {"name": "Gimbal", "enabled": True},
        {
            "id": "DownCamera",
            "gimbal": {"gimbal-id": "Gimbal", "lock-roll": True},
        },
        {"image-type": 0, "streaming-enabled": True},
    ]

    merged = merge_lists(base, overlay)

    assert len(merged) == 3
    assert merged[0]["enabled"] is True
    assert merged[1]["gimbal"] == {
        "gimbal-id": "Gimbal",
        "lock-roll": True,
    }
    assert merged[2]["streaming-enabled"] is True
