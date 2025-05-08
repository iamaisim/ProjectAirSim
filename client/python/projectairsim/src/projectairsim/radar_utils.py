import math
import multiprocessing as mp
import matplotlib

# Tkinter is available with python 3.x+; Setting "TkAgg" backend below takes precedence
# over other methods of choosing the backend by the user (using a .matplotlibrc file or env var).
# While not a good idea to force the use of TkAgg as the backend, this avoids reported
# crashes with GTK backend which is system default on some linux distros.
# see docs/sensors/display for more info on the MPL backend.
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt
import matplotlib.animation as animation


class RadarDisplay(object):
    def __init__(
        self,
        sensor_name="Radar1",
        det_plot_width=800,
        det_plot_height=225,
        det_plot_x=50,
        det_plot_y=50,
        track_plot_width=800,
        track_plot_height=225,
        track_plot_x=900,
        track_plot_y=50,
        x_axis_width=20.0,
        y_axis_width=20.0,
        z_axis_width=10.0,
        anim_interval_sec=0.02,
    ):
        """2D plot/display for Radar sensor data.

        Data points are plotted using sensor-local frame with axes +X forward,
        +Y right, +Z up (not NED) for a more natural plotting view.
        """
        self.sensor_name = sensor_name
        self.det_plot_dims = {
            "width": det_plot_width,
            "height": det_plot_height,
            "x": det_plot_x,
            "y": det_plot_y,
        }
        self.track_plot_dims = {
            "width": track_plot_width,
            "height": track_plot_height,
            "x": track_plot_x,
            "y": track_plot_y,
        }
        self.x_axis_width = x_axis_width
        self.y_axis_width = y_axis_width
        self.z_axis_width = z_axis_width
        self.anim_interval_sec = anim_interval_sec
        self.dpi = plt.rcParams["figure.dpi"]  # to convert figsize px->inches
        self.font_size = 8

        self.dets_fig = None
        self.dets_ax = []
        self.dets_plot_line_objs = []
        self.tracks_fig = None
        self.tracks_ax = []
        self.tracks_plot_line_objs = []
        self.track_arrows = []
        self.arrow_size = 0.1

        self.running = False
        self.runner = mp.Process(target=self.display_animated_plots)
        self.detections_q = mp.Queue()
        self.tracks_q = mp.Queue()
        self.dets_q_size = 1
        self.tracks_q_size = 1

    def receive_detections(self, detections):
        """Callback for receiving new RADAR detection data. Keep this processing as
        short as possible to avoid blocking the callback for receiving data from NNG.
        """
        if self.running and detections is not None:
            # Use a multi-processing queue to pipe the data across processes
            self.dump_excess_data(self.detections_q, self.dets_q_size)
            self.detections_q.put(detections)

    def receive_tracks(self, tracks):
        """Callback for receiving new RADAR tracks data. Keep this processing as
        short as possible to avoid blocking the callback for receiving data from NNG.
        """
        if self.running and tracks is not None:
            # Use a multi-processing queue to pipe the data across processes
            self.dump_excess_data(self.tracks_q, self.tracks_q_size)
            self.tracks_q.put(tracks)

    def dump_excess_data(self, data_q, q_size):
        while not data_q.empty() and data_q.qsize() > q_size:
            data_q.get()

    def start(self):
        self.running = True
        self.runner.start()

    def stop(self):
        self.running = False
        self.detections_q.close()
        self.detections_q.join_thread()
        self.tracks_q.close()
        self.tracks_q.join_thread()
        if self.runner.is_alive():
            self.runner.terminate()
            self.runner.join()

    def init_ortho_plot(self, name="", size_x=900, size_y=225, pos_x=0, pos_y=0):
        new_fig = plt.figure(figsize=(size_x / self.dpi, size_y / self.dpi))
        axes_list = []
        plot_line_list = []

        backend = matplotlib.get_backend()
        if backend == "TkAgg":
            new_fig.canvas.manager.window.wm_geometry(f"+{pos_x}+{pos_y}")
        new_fig.set_tight_layout(True)

        axes_list.append(new_fig.add_subplot(131))
        axes_list.append(new_fig.add_subplot(132))
        axes_list.append(new_fig.add_subplot(133))

        axes_list[0].set_title(f"{name} - Top")
        axes_list[0].set_xlabel("Y (m)")
        axes_list[0].set_ylabel("X (m)")
        axes_list[0].plot(0, 0, "vr", alpha=1.0)  # sensor at origin
        (plot_line_obj_0,) = axes_list[0].plot([], [], ".k", alpha=0.5)
        plot_line_list.append(plot_line_obj_0)
        # Put sensor at bottom-middle of plot
        axes_list[0].set_xlim(-self.y_axis_width / 2, self.y_axis_width / 2)  # sensor Y
        axes_list[0].set_ylim(-1, self.x_axis_width)  # sensor X

        axes_list[1].set_title(f"{name} - Right")
        axes_list[1].set_xlabel("X (m)")
        axes_list[1].set_ylabel("Z (m)")
        axes_list[1].plot(0, 0, "<r", alpha=1.0)  # sensor at origin
        (plot_line_obj_1,) = axes_list[1].plot([], [], ".k", alpha=0.5)
        plot_line_list.append(plot_line_obj_1)
        # Put sensor at left-middle of plot
        axes_list[1].set_xlim(-1, self.x_axis_width)  # sensor X
        axes_list[1].set_ylim(-self.z_axis_width / 2, self.z_axis_width / 2)  # sensor Z

        axes_list[2].set_title(f"{name} - Back")
        axes_list[2].set_xlabel("Y (m)")
        axes_list[2].set_ylabel("Z (m)")
        axes_list[2].plot(0, 0, "sr", alpha=1.0)  # sensor at origin
        (plot_line_obj_2,) = axes_list[2].plot([], [], ".k", alpha=0.5)
        plot_line_list.append(plot_line_obj_2)
        # Put sensor at middle-middle of plot
        axes_list[2].set_xlim(-self.y_axis_width / 2, self.y_axis_width / 2)  # sensor Y
        axes_list[2].set_ylim(-self.z_axis_width / 2, self.z_axis_width / 2)  # sensor Z

        return new_fig, axes_list, plot_line_list

    def get_next_det_data(self):
        while self.running:
            x_vec = []
            y_vec = []
            z_vec = []
            got_new_data = False

            if not self.detections_q.empty():
                detections = self.detections_q.get()
                got_new_data = True
                for det in detections:
                    azimuth = det["azimuth"]
                    elevation = det["elevation"]
                    range = det["range"]
                    # Use +X forward, +Y right, +Z up for natural plotting
                    x = range * math.cos(elevation) * math.cos(azimuth)
                    y = range * math.cos(elevation) * math.sin(azimuth)
                    z = range * math.sin(elevation)
                    x_vec.append(x)
                    y_vec.append(y)
                    z_vec.append(z)

            yield x_vec, y_vec, z_vec, got_new_data

    def get_next_track_data(self):
        while self.running:
            x_vec = []
            y_vec = []
            z_vec = []
            vel_x_vec = []
            vel_y_vec = []
            vel_z_vec = []
            got_new_data = False

            if not self.tracks_q.empty():
                tracks = self.tracks_q.get()
                got_new_data = True
                for track in tracks:
                    pos = track["position_est"]
                    vel = track["velocity_est"]
                    x_vec.append(pos["x"])
                    y_vec.append(pos["y"])
                    z_vec.append(-pos["z"])  # NED -> NEU
                    vel_x_vec.append(vel["x"])
                    vel_y_vec.append(vel["y"])
                    vel_z_vec.append(-vel["z"])  # NED -> NEU

            yield x_vec, y_vec, z_vec, vel_x_vec, vel_y_vec, vel_z_vec, got_new_data

    def update_det_plots(self, data):
        # Unpack new data from this animation frame yielded from get_next_det_data()
        x_vec, y_vec, z_vec, got_new_data = data

        if got_new_data and len(y_vec) == len(x_vec) and len(z_vec) == len(x_vec):
            # Refresh detection data objects with new data
            self.dets_plot_line_objs[0].set_data(y_vec, x_vec)
            self.dets_plot_line_objs[1].set_data(x_vec, z_vec)
            self.dets_plot_line_objs[2].set_data(y_vec, z_vec)

        # Return list of all updated plot objects to be animated
        return self.dets_plot_line_objs

    def update_track_plots(self, data):
        # Unpack new data from this animation frame yielded from get_next_track_data()
        x_vec, y_vec, z_vec, vel_x_vec, vel_y_vec, vel_z_vec, got_new_data = data

        if got_new_data and len(y_vec) == len(x_vec) and len(z_vec) == len(x_vec):
            # Refresh track data objects with new data
            self.tracks_plot_line_objs[0].set_data(y_vec, x_vec)
            self.tracks_plot_line_objs[1].set_data(x_vec, z_vec)
            self.tracks_plot_line_objs[2].set_data(y_vec, z_vec)

            # Add velocity arrows to track data points
            for arrow in self.track_arrows:
                arrow.remove()
            self.track_arrows.clear()

            for i in range(len(x_vec)):
                a1 = self.tracks_ax[0].arrow(
                    y_vec[i],
                    x_vec[i],
                    vel_y_vec[i],
                    vel_x_vec[i],
                    head_width=self.arrow_size,
                    head_length=self.arrow_size,
                )
                a2 = self.tracks_ax[1].arrow(
                    x_vec[i],
                    z_vec[i],
                    vel_x_vec[i],
                    vel_z_vec[i],
                    head_width=self.arrow_size,
                    head_length=self.arrow_size,
                )
                a3 = self.tracks_ax[2].arrow(
                    y_vec[i],
                    z_vec[i],
                    vel_y_vec[i],
                    vel_z_vec[i],
                    head_width=self.arrow_size,
                    head_length=self.arrow_size,
                )
                self.track_arrows.append(a1)
                self.track_arrows.append(a2)
                self.track_arrows.append(a3)

        # Return concatenated list of all updated plot objects to be animated
        return self.tracks_plot_line_objs + self.track_arrows

    def display_animated_plots(self):
        matplotlib.rcParams["toolbar"] = "None"
        matplotlib.rcParams["font.size"] = 8

        # Set up Radar Detections orthographic plots
        (self.dets_fig, self.dets_ax, self.dets_plot_line_objs,) = self.init_ortho_plot(
            f"{self.sensor_name} Detections",
            self.det_plot_dims["width"],
            self.det_plot_dims["height"],
            self.det_plot_dims["x"],
            self.det_plot_dims["y"],
        )

        # Set up Radar Detections plot animation
        self.ani_dets = animation.FuncAnimation(
            self.dets_fig,
            self.update_det_plots,
            self.get_next_det_data,
            interval=self.anim_interval_sec * 1000,
            blit=True,
        )

        # Set up Radar Tracks orthographic plots
        (
            self.tracks_fig,
            self.tracks_ax,
            self.tracks_plot_line_objs,
        ) = self.init_ortho_plot(
            f"{self.sensor_name} Tracks",
            self.track_plot_dims["width"],
            self.track_plot_dims["height"],
            self.track_plot_dims["x"],
            self.track_plot_dims["y"],
        )

        # Set up Radar Tracks plot animation
        self.ani_tracks = animation.FuncAnimation(
            self.tracks_fig,
            self.update_track_plots,
            self.get_next_track_data,
            interval=self.anim_interval_sec * 1000,
            blit=True,
        )

        # Start plot animations
        plt.show()
