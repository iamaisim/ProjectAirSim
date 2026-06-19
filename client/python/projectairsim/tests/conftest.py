"""
Copyright (C) 2025 IAMAI CONSULTING CORP -- MIT License.
Pytest configuration for projectairsim integration tests.
"""


def pytest_addoption(parser):
    parser.addoption(
        "--plot",
        action="store_true",
        default=False,
        help="Show real-time 3D matplotlib viewer during flight",
    )
