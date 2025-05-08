from argparse import ArgumentError
import multiprocessing as mp

import numpy as np
import open3d as o3d
from matplotlib import cm
from projectairsim.image_utils import SEGMENTATION_PALLETE

# from projectairsim.utils import projectairsim_log


class LidarDisplay:
    """Class to display LIDAR point cloud data in a window. This runs its visualization
    display update loop in a separate process independent of the client's async.io main
    loop since it's independent of the robot controls and for visualization only.
    """

    class ExitRequest:
        """Class for requesting the display loop exit"""

        def __init__(self):
            pass

    class SetViewBoundsRequest:
        """Class for requesting the display loop recalculate the view bounds"""

        def __init__(self):
            pass

    class ViewChangeRequest:
        """Class for making a view change request to the display loop"""

        def __init__(self, lookat_xyz, view_front, view_up, zoom):
            self.lookat_xyz = lookat_xyz
            self.view_front = view_front
            self.view_up = view_up
            self.zoom = zoom

    # Preset view identifiers
    VIEW_CUSTOM = (
        0  # Display LIDAR returns looking along custom forward and up directions
    )
    VIEW_PERSPECTIVE = 1  # Display LIDAR returns looking almost horizontally forward
    VIEW_FORWARD = 2  # Display LIDAR returns from looking forward
    VIEW_TOPDOWN = 3  # Display LIDAR returns from the top looking down

    # Color identifiers
    COLOR_INTENSITY = 0
    COLOR_SEGMENTATION = 1

    RGB_BLACK = [0, 0, 0]  # Black color

    PLASMA_PALLETE = np.array(cm.get_cmap("plasma").colors)

    # Class attibute to store the predefined color palette. For more details, refer
    # to the Segmentation documentation page.
    _color_pallete_norm = (np.asarray(SEGMENTATION_PALLETE) / 255).tolist()
    _num_color_pallette_norm = len(_color_pallete_norm)

    def __init__(
        self,
        win_name="LIDAR",
        color_mode=COLOR_SEGMENTATION,
        color_intensity_range=[0.0, 1.0],
        width=400,
        height=225,
        x=50,
        y=50,
        lookat_xyz=None,
        zoom=0.2,
        view=VIEW_PERSPECTIVE,
        view_front=None,  # When view == VIEW_CUSTOM, vector to the viewer relative to the LIDAR
        view_up=None,  # When view == VIEW_CUSTOM, vector towards up in display view relative to the LIDAR
        view_bounds=None,  # Used to set the view bounds shown by the Open3D window; if None, bounds are automatically set by the first point cloud (NED -x, -y, -z, +x, +y, +z)
        coordinate_axes_size=5.0,  # Size of coordinate axes indicator (set to 0.0 or less to disable)
    ):
        self.running = False
        self.runner = None

        self.win_name = win_name
        self.color_mode = color_mode
        self.width = width
        self.height = height
        self.x = x
        self.y = y
        self.coordinate_axes_size = coordinate_axes_size
        self.zoom = zoom
        self.view_bounds = view_bounds
        self.window_created = False
        self.reset_bounding_box = False

        self.control_q = mp.Queue()
        self.points_q = mp.Queue()
        self.buffer_size = 5

        if (view == self.VIEW_CUSTOM) and (
            not lookat_xyz or not view_front or not view_up
        ):
            raise ArgumentError(
                "LidarDisplay: View is VIEW_CUSTOM but one or more of lookat_xyz,"
                " view_front, or view_up arguments are missing"
            )

        if view == self.VIEW_CUSTOM:
            self.set_view_custom(lookat_xyz, view_front, view_up)
        else:
            self.set_view_preset(view, lookat_xyz)

        if (
            len(color_intensity_range) == 2
            and color_intensity_range[0] <= color_intensity_range[1]
        ):
            self.plasma_range = np.linspace(
                color_intensity_range[0],
                color_intensity_range[1],
                self.PLASMA_PALLETE.shape[0],
            )
        else:
            raise ArgumentError("LidarDisplay: Invalid color_intensity_range")

    def set_view_preset(self, view: int, lookat_xyz=None):
        if view == self.VIEW_TOPDOWN:
            # Top-down view along -Z-axis
            self.view_front = [0.0, 0.0, -1.0]
            self.view_up = [1.0, 0.0, 0.0]
            self.lookat_xyz = [0.0, 0.0, -5.0] if not lookat_xyz else lookat_xyz
            self.view = view
        elif view == self.VIEW_FORWARD:
            # Forward view along +X-axis
            self.view_front = [-1.0, 0.0, 0.0]
            self.view_up = [0.0, 0.0, -1.0]
            self.lookat_xyz = [0.0, 0.0, 0.0] if not lookat_xyz else lookat_xyz
            self.view = view
        elif view == self.VIEW_PERSPECTIVE:
            # Perspective view along +X-axis looking slightly down and below XY plane
            self.view_front = [-0.95, 0.0, -0.5]
            self.view_up = [0.5, 0.0, -0.95]
            self.lookat_xyz = [0.0, 0.0, 5.0] if not lookat_xyz else lookat_xyz
            self.view = view
        elif view == self.VIEW_CUSTOM:
            raise ArgumentError(
                "VIEW_CUSTOM is not valid here--call set_view_custom() instead"
            )
        else:
            raise ArgumentError(f"Unrecognized preset view ID {view}")

        if self.running:
            self.control_q.put(
                self.ViewChangeRequest(
                    self.lookat_xyz, self.view_front, self.view_up, self.zoom
                )
            )

    def set_view_custom(self, lookat_xyz, view_front, view_up):
        # View front/up coords can be captured by mousing the LidarDisplay window and
        # pressing Ctrl-C, then Ctrl-V to paste the JSON of the captured view settings
        self.view = self.VIEW_CUSTOM
        self.lookat_xyz = lookat_xyz
        self.view_front = view_front
        self.view_up = view_up

        if self.running:
            self.control_q.put(
                self.ViewChangeRequest(
                    self.lookat_xyz, self.view_front, self.view_up, self.zoom
                )
            )

    def set_zoom(self, zoom):
        self.zoom = zoom
        if self.running:
            self.control_q.put(
                self.ViewChangeRequest(
                    self.lookat_xyz, self.view_front, self.view_up, self.zoom
                )
            )

    def start(self):
        if not self.running:
            self.running = True

            # Open3D runs very slowly using multi-threading on Linux (~70 ms loop time),
            # so run it using multi-processing instead (<1 ms loop time).
            self.runner = mp.Process(target=self.display_loop)
            self.runner.start()

    def stop(self):
        self.running = False
        if self.runner and self.runner.is_alive():
            self.control_q.put(self.ExitRequest())
            self.runner.join(5)  # Give runner a chance to exit by itself
            if self.runner.is_alive():
                # Didn't exit, force the issue
                self.runner.terminate()
                self.runner.join()
            self.runner = None

        # Empty queues so their feeder threads can exit
        while not self.points_q.empty():
            self.points_q.get()
        while not self.control_q.empty():
            self.control_q.get()

    def set_view_bounds(self, view_bounds):
        self.view_bounds = view_bounds
        if self.running:
            self.control_q.put(self.SetViewBoundsRequest(view_bounds))

    def add_geometries(self):
        self.o3d_vis.add_geometry(self.point_cloud)

        # Add coordinate axes at origin to show sensor's position
        if self.coordinate_axes_size >= 0.0:
            mesh_frame = o3d.geometry.TriangleMesh.create_coordinate_frame(
                size=self.coordinate_axes_size, origin=[0, 0, 0]
            )
            self.o3d_vis.add_geometry(mesh_frame)

    def set_geometries_and_view(self):
        self.o3d_vis.clear_geometries()

        if not self.view_bounds:
            self.add_geometries()
            self.set_view()
        else:
            # Set visualizer bounds with a temporary geometry set to our desired bounds
            points_sav = self.point_cloud.points
            colors_sav = self.point_cloud.colors

            points = np.array(self.view_bounds, dtype=np.dtype("f4"))
            points = np.reshape(points, (int(points.shape[0] / 3), 3))
            self.point_cloud.points = o3d.utility.Vector3dVector(points)
            self.point_cloud.colors = o3d.utility.Vector3dVector(
                np.c_[
                    np.array([255, 255]),
                    np.array([255, 255]),
                    np.array([255, 255]),
                ]
            )
            self.add_geometries()
            self.set_view()

            # Now replace the temporary geometry with the real geometry
            self.point_cloud.points = points_sav
            self.point_cloud.colors = colors_sav
            points_sav = None
            colors_sav = None
            self.o3d_vis.update_geometry(self.point_cloud)

    def dump_excess_data(self):
        while not self.points_q.empty() and self.points_q.qsize() > self.buffer_size:
            self.points_q.get()

    def init_window(self):
        """Creates and sets up window for displaying point cloud.
        Note: self.point_cloud.points should already be populated with some point data
              before calling this or else the window view won't display all of the area
              that the points cover properly.
        """
        self.o3d_vis.create_window(
            window_name=self.win_name,
            width=self.width,
            height=self.height,
            left=self.x,
            top=self.y,
        )
        self.o3d_vis.get_render_option().point_size = 3.0
        self.o3d_vis.get_render_option().background_color = [
            0.0,
            0.0,
            0.0,
        ]  # black

        self.set_geometries_and_view()

    def display_loop(self):
        self.view_change_pending = False
        recalculate_view_bounds = True

        try:
            self.o3d_vis = o3d.visualization.Visualizer()
            self.point_cloud = o3d.geometry.PointCloud()

            while self.running:
                # Process any control requests
                while not self.control_q.empty():
                    request = self.control_q.get()
                    # print(f"LidarDisplay.display_loop(): Got request {request}")
                    if type(request) == self.ExitRequest:
                        self.running = False
                        break  # Exit the loop and close the window

                    elif type(request) == self.SetViewBoundsRequest:
                        self.view_bounds = request.view_bounds
                        recalculate_view_bounds = True

                    elif type(request) == self.ViewChangeRequest:
                        # Set a new view
                        self.lookat_xyz = request.lookat_xyz
                        self.view_front = request.view_front
                        self.view_up = request.view_up
                        self.zoom = request.zoom
                        self.view_change_pending = True

                # Process any new points
                points_data = None
                while not self.points_q.empty():
                    points_data = self.points_q.get()
                if points_data:
                    points = np.array(points_data["point_cloud"], dtype=np.dtype("f4"))

                    # Set colors based on the color pallette chosen
                    point_colors = []
                    if self.color_mode == self.COLOR_INTENSITY:
                        points_intensity = (
                            (np.array(points_data["intensity_cloud"]) * 255.0) + 1
                        ) / 256.0
                        point_colors = np.c_[
                            np.interp(
                                points_intensity,
                                self.plasma_range,
                                self.PLASMA_PALLETE[:, 0],
                            ),
                            np.interp(
                                points_intensity,
                                self.plasma_range,
                                self.PLASMA_PALLETE[:, 1],
                            ),
                            np.interp(
                                points_intensity,
                                self.plasma_range,
                                self.PLASMA_PALLETE[:, 2],
                            ),
                        ]
                    else:
                        points_segmentation = np.array(
                            points_data["segmentation_cloud"]
                        )
                        point_colors = [
                            self.RGB_BLACK
                            if (i < 0) or (i >= self._num_color_pallette_norm)
                            else self._color_pallete_norm[i]
                            for i in points_segmentation
                        ]

                    points = np.reshape(points, (int(points.shape[0] / 3), 3))
                    # projectairsim_log().info(f"Received {points.shape[0]} points")

                    self.point_cloud.points = o3d.utility.Vector3dVector(points)
                    self.point_cloud.colors = o3d.utility.Vector3dVector(point_colors)

                    if not self.window_created:
                        self.init_window()  # create window from thread that updates it
                        self.window_created = True
                    elif (
                        not recalculate_view_bounds or len(self.point_cloud.points) < 2
                    ):
                        self.o3d_vis.update_geometry(self.point_cloud)
                    else:
                        # Force visualizer to recalculate the bounding box
                        recalculate_view_bounds = False
                        self.set_geometries_and_view()

                # Set view if view parameters were changed
                if self.window_created and self.view_change_pending:
                    self.set_view()

                # Do Open3D processing
                self.o3d_vis.poll_events()
                self.o3d_vis.update_renderer()

        except KeyboardInterrupt:
            pass  # Just exit normally

        finally:
            self.window_created = False

    def receive(self, lidar_data):
        """Callback for receiving new LIDAR data. Keep this processing as short as
        possible to avoid blocking the callback for receiving data from NNG.
        """
        if self.running and lidar_data is not None:
            self.dump_excess_data()
            self.points_q.put(lidar_data)

    def set_view(self):
        # Set view direction
        self.o3d_vis.get_view_control().set_front(self.view_front)
        self.o3d_vis.get_view_control().set_up(self.view_up)

        # Adjust center of view down slightly and zoom in
        self.o3d_vis.get_view_control().set_lookat(self.lookat_xyz)
        self.o3d_vis.get_view_control().set_zoom(self.zoom)

        self.view_change_pending = False
