"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
Pytest end-end test script for base ProjectAirSim client functionality
"""

import pytest

from projectairsim import ProjectAirSimClient


@pytest.fixture(scope="class")
def robo():
    class ProjectAirSim:
        client = ProjectAirSimClient()

    robo_obj = ProjectAirSim()
    yield robo_obj

    print("\nTeardown client...")
    robo_obj.client.disconnect()


class TestClientBase:
    def test_connect(self, robo):
        assert robo.client.state is False
        robo.client.connect()
        assert robo.client.state is True
        assert len(robo.client.socket_topics.dialers) > 0

    def test_disconnect(self, robo):
        robo.client.disconnect()
        assert len(robo.client.subs) == 0
        assert robo.client.state is False
        assert len(robo.client.socket_topics.dialers) == 0
        robo.client.connect() # needed if the client will be teared down.
