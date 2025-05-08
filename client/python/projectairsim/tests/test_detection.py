import pytest
import time
from projectairsim import ProjectAirSimClient, World, Drone

class NonphysicsDrone(Drone):
    def __init__(self, client, world, name):
        super().__init__(client, world, name)
        self.cur_pose = None
        self.cur_rgb_image = None
        self.sensors_info = None

    # Method to handle the callback for the current pose
    def callback_actual_pose(self, topic, pose_msg):
        self.cur_pose = pose_msg

    # Method to handle the callback for the RGB image
    def callback_rgb_image(self, topic, image_msg):
        self.cur_rgb_image = image_msg

    # Method to handle the callback for the sensors info
    def callback_sensors_info(self, topic, sensors_msg):
        self.sensors_info = sensors_msg


@pytest.fixture(scope="module")
def setup_drone():
    # Create the client and connect to the simulator
    client = ProjectAirSimClient()
    client.connect()

    # Create the world and the drone
    world = World(client, "scene_test_nonphysics_annotations_drone.jsonc")
    drone = NonphysicsDrone(client, world, "Drone1")

    # Subscribe to the sensors
    client.subscribe(drone.robot_info["actual_pose"], drone.callback_actual_pose)
    client.subscribe(drone.sensors["Chase"]["scene_camera"], drone.callback_rgb_image)
    client.subscribe(drone.sensors["Chase"]["scene_camera_info"], drone.callback_sensors_info)

    # Allow time for data to be received
    time.sleep(2)

    yield drone

    # Disconnect from the simulator
    client.disconnect()


def assert_annotations_present(drone):
    assert drone.cur_rgb_image is not None, "No RGB image was received"
    assert len(drone.cur_rgb_image.get("annotations", [])) > 0, "No annotations were detected"

def test_annotations_returned(setup_drone):
    drone = setup_drone
    assert_annotations_present(drone)

def test_bbox_center_detection(setup_drone):
    drone = setup_drone
    assert_annotations_present(drone)
    annotations = drone.cur_rgb_image["annotations"]

    # Get the dimensions of the image
    image_width = drone.sensors_info["width"]
    image_height = drone.sensors_info["height"]

    # Define the world positions of the objects
    expected_positions = {
        "OrangeBall": {"x": 91.15, "y": 32.10, "z": -5.70},
        "Cone_5": {"x": 91.4, "y": -35.4, "z": -6.0},
    }

    tolerance = 0.01

    for annotation in annotations:
        object_name = annotation["object_id"]
        bbox_center_x = annotation["bbox2d"]["center"]["x"]
        bbox_center_y = annotation["bbox2d"]["center"]["y"]
        bbox_position_center_x = annotation["bbox3d"]["center"]["x"]
        bbox_position_center_y = annotation["bbox3d"]["center"]["y"]
        bbox_position_center_z = annotation["bbox3d"]["center"]["z"]

        # Validate bounding box center within image bounds
        assert 0 <= bbox_center_x <= image_width, f"The bbox center X ({bbox_center_x}) is out of image bounds"
        assert 0 <= bbox_center_y <= image_height, f"The bbox center Y ({bbox_center_y}) is out of image bounds"

        # Validate 3D bounding box center
        if object_name in expected_positions:
            expected_position = expected_positions[object_name]
            assert abs(bbox_position_center_x - expected_position["x"]) <= tolerance, \
                f"The bbox center X ({bbox_position_center_x}) for {object_name} is not close to expected"
            assert abs(bbox_position_center_y - expected_position["y"]) <= tolerance, \
                f"The bbox center Y ({bbox_position_center_y}) for {object_name} is not close to expected"
            assert abs(bbox_position_center_z - expected_position["z"]) <= tolerance, \
                f"The bbox center Z ({bbox_position_center_z}) for {object_name} is not close to expected"

def test_bounding_box_alignment(setup_drone):
    drone = setup_drone
    assert_annotations_present(drone)
    annotation = drone.cur_rgb_image["annotations"][0]
    bbox_center = annotation["bbox2d"]["center"]

    # Validate bounding box center within image bounds
    assert 0 <= bbox_center["x"] <= drone.cur_rgb_image["width"], "The bounding box center is out of the image bounds"
    assert 0 <= bbox_center["y"] <= drone.cur_rgb_image["height"], "The bounding box center is out of the image bounds"


def test_2d_bbox_consistency(setup_drone):
    # Verify that the 2D bounding box does not change over time
    drone = setup_drone
    assert_annotations_present(drone)
    annotation = drone.cur_rgb_image["annotations"][0]
    bbox_center = annotation["bbox2d"]["center"]

    initial_center_x = bbox_center["x"]
    initial_center_y = bbox_center["y"]
    
    time.sleep(1)  # Simulate a slight delay

    annotation = drone.cur_rgb_image["annotations"][0]
    assert initial_center_x == annotation["bbox2d"]["center"]["x"], "The 2D bounding box has shifted on the X axis"
    assert initial_center_y == annotation["bbox2d"]["center"]["y"], "The 2D bounding box has shifted on the Y axis"

def test_bbox3d_size(setup_drone):
    drone = setup_drone
    assert_annotations_present(drone)
    annotations = drone.cur_rgb_image["annotations"]

    expected_size = {"x": 10, "y": 10, "z": 10}
    tolerance = 0.001

    for annotation in annotations:
        bbox3d_size = annotation["bbox3d"]["size"]
        for axis in expected_size:
            assert abs(bbox3d_size[axis] - expected_size[axis]) <= tolerance, \
                f"The bbox3d size {axis} for {annotation['object_id']} is incorrect"


def test_bbox3d_orientation(setup_drone):
    drone = setup_drone
    assert_annotations_present(drone)
    annotations = drone.cur_rgb_image["annotations"]

    for annotation in annotations:
        quaternion = annotation["bbox3d"]["quaternion"]
        # Verify the orientation (quaternion) is consistent and expected
        assert quaternion == {"w": 1, "x": 0, "y": 0, "z": 0}, \
            f"The bbox3d orientation for {annotation['object_id']} is incorrect"


def test_bbox3d_in_image_space_vertices(setup_drone):
    drone = setup_drone
    assert_annotations_present(drone)
    annotations = drone.cur_rgb_image["annotations"]

    image_width = drone.sensors_info["width"]
    image_height = drone.sensors_info["height"]

    # Expected vertices for each object
    expected_vertices = {
        'Cone_5': [
            {'x': 368, 'y': 257}, {'x': 362, 'y': 188}, {'x': 435, 'y': 257},
            {'x': 431, 'y': 188}, {'x': 393, 'y': 255}, {'x': 389, 'y': 192},
            {'x': 454, 'y': 255}, {'x': 451, 'y': 192}
        ],
        'OrangeBall': [
            {'x': 822, 'y': 259}, {'x': 826, 'y': 190}, {'x': 889, 'y': 259},
            {'x': 895, 'y': 190}, {'x': 805, 'y': 256}, {'x': 808, 'y': 194},
            {'x': 866, 'y': 256}, {'x': 870, 'y': 194}
        ]
    }

    for annotation in annotations:
        bbox3d_vertices = annotation["bbox3d_in_image_space"]

        # Verify that there are exactly 8 vertices
        assert len(bbox3d_vertices) == 8, \
            f"The bbox3d_in_image_space for {annotation['object_id']} does not have 8 vertices"

        # Retrieve the expected vertices for the current object_id
        expected_vertices_for_object = expected_vertices.get(annotation['object_id'])

        # Ensure that we have expected vertices to compare against
        assert expected_vertices_for_object is not None, \
            f"No expected vertices found for object {annotation['object_id']}"

        # Verify that each vertex matches the expected value
        for vertex, expected_vertex in zip(bbox3d_vertices, expected_vertices_for_object):
            assert vertex == expected_vertex, \
                f"Expected vertex {expected_vertex} but got {vertex} for {annotation['object_id']}"

        # Verify each vertex of bbox3d_in_image_space is within the image bounds
        for vertex in bbox3d_vertices:
            assert 0 <= vertex["x"] <= image_width, \
                f"The bbox3d vertex X for {annotation['object_id']} is out of image bounds"
            assert 0 <= vertex["y"] <= image_height, \
                f"The bbox3d vertex Y for {annotation['object_id']} is out of image bounds"

def test_bbox3d_in_projection_space_range(setup_drone):
    # TODO: The values in this test are hardcoded and are not verified. This test should be updated to verify the values.
    drone = setup_drone
    assert_annotations_present(drone)
    annotations = drone.cur_rgb_image["annotations"]

    # Initial values for the expected vertices to use as regression test
    expected_projection_vertices = {
        'Cone_5': [
            {'x': -0.4240451157093048, 'y': 0.2842429280281067, 'z': 0.0010496167233213782},
            {'x': -0.43307656049728394, 'y': 0.4770702123641968, 'z': 0.0010719718411564827},
            {'x': -0.31908345222473145, 'y': 0.2842429280281067, 'z': 0.0010496167233213782},
            {'x': -0.32587939500808716, 'y': 0.4770702123641968, 'z': 0.0010719718411564827},
            {'x': -0.3844926655292511, 'y': 0.2913464605808258, 'z': 0.0009517146972939372},
            {'x': -0.3919031620025635, 'y': 0.46597820520401, 'z': 0.0009700575028546154},
            {'x': -0.2893212139606476, 'y': 0.2913464605808258, 'z': 0.0009517146972939372},
            {'x': -0.2948974370956421, 'y': 0.46597820520401, 'z': 0.0009700575028546154}
        ],
        'OrangeBall': [
            {'x': 0.2850034534931183, 'y': 0.2784576714038849, 'z': 0.0010516734328120947},
            {'x': 0.29108569025993347, 'y': 0.4715474247932434, 'z': 0.0010741171427071095},
            {'x': 0.39017075300216675, 'y': 0.2784576714038849, 'z': 0.0010516734328120947},
            {'x': 0.3984973728656769, 'y': 0.4715474247932434, 'z': 0.0010741171427071095},
            {'x': 0.25837278366088867, 'y': 0.2861143946647644, 'z': 0.000953405280597508},
            {'x': 0.26336154341697693, 'y': 0.46096134185791016, 'z': 0.0009718139190226793},
            {'x': 0.3537133038043976, 'y': 0.2861143946647644, 'z': 0.000953405280597508},
            {'x': 0.3605429232120514, 'y': 0.46096134185791016, 'z': 0.0009718139190226793}
        ]
    }

    projection_bounds = {"x": (-1.0, 1.0), "y": (-1.0, 1.0), "z": (0.0, 1.0)}

    for annotation in annotations:
        bbox3d_projection_vertices = annotation["bbox3d_in_projection_space"]

        # Verify that there are exactly 8 vertices
        assert len(bbox3d_projection_vertices) == 8, \
            f"The bbox3d_in_projection_space for {annotation['object_id']} does not have 8 vertices"

        # Retrieve the expected vertices for the current object_id
        expected_vertices = expected_projection_vertices.get(annotation['object_id'], [])
        assert expected_vertices, f"No expected vertices found for object {annotation['object_id']}"

        # Verify each vertex of bbox3d_in_projection_space is within the expected limits and matches the expected values
        for i, vertex in enumerate(bbox3d_projection_vertices):
            expected_vertex = expected_vertices[i]
            for axis, (min_val, max_val) in projection_bounds.items():
                assert min_val <= vertex[axis] <= max_val, \
                    f"The projection vertex {axis} for {annotation['object_id']} is out of expected bounds: {vertex[axis]}"
                assert abs(vertex[axis] - expected_vertex[axis]) <= 0.001, \
                    f"The projection vertex {axis} for {annotation['object_id']} does not match the expected value: got {vertex[axis]}, expected {expected_vertex[axis]}"

                
def test_3d_bbox_consistency(setup_drone):
    #Verify that the 3D bounding box does not change over time
    drone = setup_drone
    assert_annotations_present(drone)
    annotation = drone.cur_rgb_image["annotations"][0]
    bbox3d_center = annotation["bbox3d"]["center"]

    initial_center = bbox3d_center.copy()

    time.sleep(1)  # Simulate a slight delay

    annotation = drone.cur_rgb_image["annotations"][0]
    bbox3d_center = annotation["bbox3d"]["center"]

    for axis in initial_center:
        assert initial_center[axis] == bbox3d_center[axis], f"The 3D bounding box has shifted on the {axis} axis"

def test_bbox_square_and_center_comparison(setup_drone):
    drone = setup_drone
    assert_annotations_present(drone)
    annotation = drone.cur_rgb_image["annotations"][0]

    # Extract 3D bounding box vertices
    bbox3d_vertices = annotation["bbox3d_in_image_space"]

    # Calculate min and max for x and y
    x_min = min(vertex["x"] for vertex in bbox3d_vertices)
    x_max = max(vertex["x"] for vertex in bbox3d_vertices)
    y_min = min(vertex["y"] for vertex in bbox3d_vertices)
    y_max = max(vertex["y"] for vertex in bbox3d_vertices)

    # Calculate center of the square
    calculated_center_x = (x_min + x_max) / 2
    calculated_center_y = (y_min + y_max) / 2

    # Extract bbox2d center
    bbox2d_center_x = annotation["bbox2d"]["center"]["x"]
    bbox2d_center_y = annotation["bbox2d"]["center"]["y"]

    # Compare the calculated center with the bbox2d center with a low tolerance
    tolerance = 0.001
    assert abs(calculated_center_x - bbox2d_center_x) <= tolerance, \
        f"The calculated center X ({calculated_center_x}) does not match bbox2d center X ({bbox2d_center_x})"
    assert abs(calculated_center_y - bbox2d_center_y) <= tolerance, \
        f"The calculated center Y ({calculated_center_y}) does not match bbox2d center Y ({bbox2d_center_y})"

    # Calculate size of the bounding box
    calculated_size_x = x_max - x_min
    calculated_size_y = y_max - y_min

    # Extract bbox2d size
    bbox2d_size_x = annotation["bbox2d"]["size"]["x"]
    bbox2d_size_y = annotation["bbox2d"]["size"]["y"]

    # Compare the calculated size with the bbox2d size with a low tolerance
    assert abs(calculated_size_x - bbox2d_size_x) <= tolerance, \
        f"The calculated size X ({calculated_size_x}) does not match bbox2d size X ({bbox2d_size_x})"
    assert abs(calculated_size_y - bbox2d_size_y) <= tolerance, \
        f"The calculated size Y ({calculated_size_y}) does not match bbox2d size Y ({bbox2d_size_y})"
