# Copyright (C) IAMAI CONSULTING CORP
#
# MIT License. All rights reserved.

import tkinter as tk
import threading
import time

from projectairsim import ProjectAirSimClient, Drone, World
from projectairsim.utils import projectairsim_log


# -----------------------------------------------
# Drone Position Viewer
# -----------------------------------------------
class DronePositionViewer:
    """Display a live 2D position trace and altitude for one drone."""

    def __init__(self, drone, close_event=None):
        self.drone = drone
        self.close_event = close_event
        # Position is updated by a topic callback and consumed by Tk's UI thread.
        self.pose_lock = threading.Lock()
        self.latest_position = None

        # Window setup
        self.root = tk.Tk()
        self.root.title("Project AirSim Drone Position Viewer")

        # Left: XY plane canvas
        self.canvas_size = 400
        self.canvas = tk.Canvas(self.root, width=self.canvas_size, height=self.canvas_size, bg="white")
        self.canvas.pack(side="left", padx=10, pady=10)

        # Right side panel for altitude
        self.alt_frame = tk.Frame(self.root, width=80, height=self.canvas_size)
        self.alt_frame.pack(side="right", padx=20, fill="y")

        # Canvas for vertical altitude bar
        self.alt_canvas_height = 400
        self.alt_canvas_width = 90
        self.alt_canvas = tk.Canvas(
            self.alt_frame,
            width=self.alt_canvas_width,
            height=self.alt_canvas_height,
            bg="lightgray"
        )
        self.alt_canvas.pack()

        # Altitude marker (moving horizontally across the bar)
        self.alt_marker = self.alt_canvas.create_line(
            0, self.alt_canvas_height/2,
            self.alt_canvas_width, self.alt_canvas_height/2,
            fill="red", width=3
        )

        # Text showing current altitude next to marker
        self.alt_text = self.alt_canvas.create_text(
            self.alt_canvas_width/2,
            self.alt_canvas_height/2 - 15,
            text="-- m",
            font=("Arial", 14)
        )

        # Create the red dot
        self.dot_size = 8
        self.dot = self.canvas.create_oval(
            self.canvas_size/2 - self.dot_size,
            self.canvas_size/2 - self.dot_size,
            self.canvas_size/2 + self.dot_size,
            self.canvas_size/2 + self.dot_size,
            fill="red"
        )

        # Scaling
        self.scale = 10

        # Project AirSim uses NED coordinates, so altitude is -position.z.
        self.MIN_DISPLAY_ALTITUDE = 0.0

        # Store the path
        self.path_points = []
        self.path_line = None      # canvas line object

        # Thread stop flag
        self.stop_event = threading.Event()

        # Use topic stream for pose updates so UI polling does not compete with
        # command requests on the same service socket.
        self.pose_topic = self.drone.robot_info["actual_pose"]
        self.drone.client.subscribe(self.pose_topic, self.on_pose_update)

        # Run UI polling on Tk's main thread. Topic callbacks only update the
        # latest pose and never touch Tk widgets directly.
        self.refresh_after_id = self.root.after(100, self.refresh_view)

        self.root.protocol("WM_DELETE_WINDOW", self.on_close)
        self.root.mainloop()

    def refresh_view(self):
        """Refresh the Tk widgets with the latest published position."""
        self.refresh_after_id = None
        if self.stop_event.is_set():
            return

        if self.close_event is not None and self.close_event.is_set():
            self.on_close()
            return

        with self.pose_lock:
            position = self.latest_position

        if position is not None:
            x = position["x"]
            y = position["y"]
            altitude = -position["z"]

            # Clamp only the visualization altitude; the simulation remains
            # responsible for the actual physics and collision response.
            altitude = max(altitude, self.MIN_DISPLAY_ALTITUDE)

            self.update_view(x, y, altitude)

        self.refresh_after_id = self.root.after(100, self.refresh_view)

    def on_pose_update(self, _, msg):
        # The actual_pose topic publishes {"position": ..., "orientation": ...}.
        position = msg.get("position") if isinstance(msg, dict) else None
        if isinstance(position, dict):
            with self.pose_lock:
                self.latest_position = position.copy()

    def update_view(self, x, y, altitude):
        # Convert world → screen coordinates
        screen_x = self.canvas_size/2 + x * self.scale
        screen_y = self.canvas_size/2 - y * self.scale

        # ---------- NEW: store and draw dashed path ----------
        self.path_points.append((screen_x, screen_y))

        # Draw dashed path
        if len(self.path_points) > 1:
            flat = [coord for point in self.path_points for coord in point]
            if self.path_line is None:
                self.path_line = self.canvas.create_line(
                    *flat, fill="black", dash=(4, 4), width=2
                )
            else:
                self.canvas.coords(self.path_line, *flat)
        # -----------------------------------------------------

        # Move the red dot
        self.canvas.coords(
            self.dot,
            screen_x - self.dot_size,
            screen_y - self.dot_size,
            screen_x + self.dot_size,
            screen_y + self.dot_size
        )

                # ---------- ALTITUDE BAR UPDATE (SMOOTH + CLEAN LABELS) ----------
        ALT_PIXELS = 100        # pixels per meter
        TICK_STEP = 0.05        # fine ticks every 0.05 m (smooth scrolling)
        VIEW_RANGE = 5.0        # ±5m window

        marker_y = self.alt_canvas_height / 2

        # Remove previous tick marks
        self.alt_canvas.delete("tick")

        # Tick range
        tick_min = altitude - VIEW_RANGE
        tick_max = altitude + VIEW_RANGE

        # Start aligned with tick grid
        tick = round(tick_min / TICK_STEP) * TICK_STEP

        while tick <= tick_max:
            dz = tick - altitude
            y = marker_y - dz * ALT_PIXELS

            if 0 <= y <= self.alt_canvas_height:

                # Check: integer or half-integer?
                is_half = abs((tick * 2) - round(tick * 2)) < 0.001

                if is_half:
                    # long tick + label
                    length = 25
                    font = ("Arial", 10)
                    label = f"{tick:.1f}"   # show one decimal (0.0, 0.5, 1.0, ...)
                else:
                    # small tick only
                    length = 10
                    font = None
                    label = None

                # Draw tick mark
                self.alt_canvas.create_line(
                    0, y, length, y, fill="black", tags="tick"
                )

                # Draw label for half & whole meters
                if label:
                    self.alt_canvas.create_text(
                        length + 5, y,
                        text=label,
                        anchor="w",
                        font=font,
                        tags="tick"
                    )

            tick += TICK_STEP

        # Marker line
        self.alt_canvas.coords(
            self.alt_marker,
            0, marker_y,
            self.alt_canvas_width, marker_y
        )

        # Marker altitude label
        self.alt_canvas.coords(self.alt_text, self.alt_canvas_width/2, marker_y - 15)
        self.alt_canvas.itemconfigure(self.alt_text, text=f"{altitude:.2f} m")


    def on_close(self):
        self.stop_event.set()
        self.root.destroy()
