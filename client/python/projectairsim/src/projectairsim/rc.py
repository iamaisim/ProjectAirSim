"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI  Consulting.  

MIT License.

Utility classes for using a input controller such as an Xbox game
controller as a remote control for an Project AirSim flight controller
such as Simple Flight.
"""
import json
import re
import threading

import commentjson
from inputs import get_gamepad

import projectairsim
from projectairsim.utils import projectairsim_log


# ----------------------------------------------------------------------------
class RCConfig:
    """
    RC configuration class.  This class specifies the mapping between the
    controller and the RC input to the flight controller and OOB input to
    the RC class object.
    """

    # ---------------------------------------------------------------------------
    # RCConfig Types
    # ---------------------------------------------------------------------------
    class ChannelEntry:
        """
        This class contains the configuration info for a single RC channel
        to the flight controller.
        """

        # -------------------------------------------------------------------------
        # ChannelEntry Types
        # -------------------------------------------------------------------------
        class MinMax:
            """
            This class stores a minimum and maximum value pair.
            """

            def __init__(self, min, max):
                """
                Constructor.

                Arguments:
                  min - Minimum value
                  max - Maximum value
                """
                self.min = min
                self.max = max

        # -------------------------------------------------------------------------
        # ChannelEntry Static Methods
        # -------------------------------------------------------------------------
        @staticmethod
        def create(json_data):
            """
            Create an instance of ChannelEntry giving the initialization in data
            JSON format.

            Arguments:
              json_data - Dictionary loaded from JSON
            """
            kwargs = {}
            kwargs["input_channel"] = json_data["input_channel"]
            for key in ("input", "input_dead", "output"):
                for attrib in ("min", "max"):
                    kwargs[key + "_" + attrib] = json_data[key + "_range"][attrib]
            return RCConfig.ChannelEntry(**kwargs)

        def __init__(
            self,
            input_channel,
            input_min: float = -1.0,
            input_max: float = 1.0,
            input_dead_min: float = 0.0,
            input_dead_max: float = 0.0,
            output_min: float = -1.0,
            output_max: float = 1.0,
        ):
            """
            Constructor.

            input_channel is the name of the channel from the source BaseController
            (e.g., 'xLeft' which is the horizontal axis of the left joystick of the
            Xbox game controller.)  The names are defined by the BaseController
            subclass.

            Source channel values are mapped to intermediate values according to
            this table:

                Source Value Range                Intermediate Value Range
                ------------------                ------------------------
                < input_min                       -1.0
                [input_min..input_dead_min)       [-1.0..0.0)
                [input_dead_min..input_dead_max)  0.0
                [input_dead_min..input_max]       [0.0..+1.0]
                > input_max                       1.0

            The intermediate values are then mapped from the range [-1.0..+1.0] to
            the output range [output_min..output_max] and either forwarded as RC
            input to the flight controller or processed as an OOB channel value.

            Arguments:
              input_channel - Name of the source controller (BaseController) channel
              input_min - Minimum expected source channel value
              input_max - Maximum expected source channel value
              input_dead_min - Minimum of source value range mapped to 0.0
              input_dead_max - Maximum of source value range mapped to 0.0
              output_min - Output value when source value is at input_min
              output_max - OUtput value when source value is at input_max
            """
            self.input_channel = input_channel
            self.input_range = self.MinMax(input_min, input_max)
            self.input_dead_range = self.MinMax(input_dead_min, input_dead_max)
            self.output_range = self.MinMax(output_min, output_max)

    # --------------------------------------------------------------------------
    class CustomEncoder(json.JSONEncoder):
        """
        Helper class for serializing a ChannelEntry object to JSON.  This class
        adds encoding of ChannelEntry and ChannelEntry.MinMax objects.
        """

        def default(self, o):
            """
            Return a serializable object for o.

            Arguments:
              o - Object for which to return a serializable object
            """
            if isinstance(o, RCConfig.ChannelEntry) or isinstance(
                o, RCConfig.ChannelEntry.MinMax
            ):
                return o.__dict__
            else:
                return super().default(o)

    # -------------------------------------------------------------------------
    # RCConfig Methods
    # -------------------------------------------------------------------------
    def __init__(self):
        """
        Constructor.
        """
        self.channel_map = {}
        self.channel_map_oob = {}

    def load(self, config_file_path: str):
        """
        Load configuration data from the specified Commented JSON file.

        Argumeents:
          config_file_path - Path to the Commented-JSON file to load
        """
        with open(config_file_path, "r") as file_in:
            json_data = file_in.read()
        data = commentjson.loads(json_data)

        # Load RC input channel map
        self.channel_map = {}
        for pair in data["channel_map"].items():
            if pair[0].isdigit():
                i = int(pair[0])
                if i not in self.channel_map:
                    self.channel_map[i] = self.ChannelEntry.create(pair[1])

        # Load OOB channel map
        for pair in data["channel_map_oob"].items():
            channel_entry_key = pair[0]
            if channel_entry_key not in self.channel_map_oob:
                self.channel_map_oob[channel_entry_key] = self.ChannelEntry.create(
                    pair[1]
                )

    def save(self, config_file_path: str):
        """
        Save the configuration data to the specified Commented-JSON file.  The
        file is silently overwritten if it exists.

        Arguments:
          config_file_path - Path to the Commented-JSON file to write
        """
        data = {
            "channel_map": self.channel_map,
            "channel_map_oob": self.channel_map_oob,
        }
        json_data = commentjson.dumps(data, indent=2, cls=self.CustomEncoder)
        with open(config_file_path, "w") as file_out:
            file_out.write(json_data)


# ----------------------------------------------------------------------------
class BaseInputController:
    """
    An object of this class reads a user input controller such as an Xbox game
    controller and presents the state of the controller as a dictionary of
    channels, one per input control.  This class is a base class--users must
    instantiate a subclass that actually does something.

    The key name of each channel is custom to each subclass--see the subclass
    documentation for the names.

    The value of each channel is the native numeric representation such as
    [-32768..32767] for an Xbox joystick axis or [0,1] for a button.  This
    representation will be mapped to the appropriate RC input channel value by
    the BaseRC object according to the RCConfig configuration data.
    """

    def __init(self):
        """
        Constructor.
        """
        self.channels = {}  # Dictionary of controller channel values

    def read(self):
        """
        Read the state of the input controller and return the current state.  This
        method usually does not return until controller has received new input.

        Returns:
          (Return) - Dictionary of controller channel values
        """
        raise NotImplemented("BaseController.read() is not implemented")


# ----------------------------------------------------------------------------
class BaseRC:
    """
    This class handles mapping the controller channels to the flight
    controller RC input channels and sending the result to the controller.

    The RCConfig class specifies which Simple Flight RC input channel comes
    from which controller channel and how to convert the controller channel
    values to those appropriate for the Simple Flight RC input channels.
    The RCConfig channel mapping can map more or fewer channels than the
    standard Simple Flight channels.  All mapped channels are passed on to
    Simple Flight which determines how to handle the missing or extra channels.

    Some subclasses also support OOB channels which control additional
    functionality implemented by the subclass.  OOB channels are processed
    like the RC input channels but the output values are used internally by
    this class and are not sent to Simple Flight directly.
    """

    def __init__(
        self,
        client: projectairsim.ProjectAirSimClient,
        robot_topic_name: str,
        secs_send_interval: float = 0.1,
    ):
        """
        Constructor.

        The robot_path string is the prefix of the Project AirSim topic name up to and
        including the name of the robot without the terminating forward-slash ('/').
        E.g., "/Sim/SceneBasicDrone/robots/Drone1".

        Arguments:
          client - Project AirSim client object
          robot_topic_name - RC input channels are published to this Project AirSim topic name
          secs_send_interval - How long to pause between sending RC input updates
              to the flight controller (seconds)
        """
        self.client = client  # Project AirSim client
        self.rc_config = RCConfig()  # RC configuration
        self.rc_input_topic_name = robot_topic_name  # Name of Project AirSim topic where we publish RC input updates
        self.secs_send_interval = secs_send_interval  # Interval between sending RC input to the flight controller (seconds)
        self._channels = []  # Current RC input channel values
        self._event_exit = threading.Event()  # Event to exit the sending thread
        self._lock = threading.Lock()  # Access guard to self._channels

        # Create and start the sending thread
        self.sender_thread = threading.Thread(
            target=self._sender_thread, name="RC Sender"
        )
        self.start()

    def __del__(self):
        """
        Destructor.
        """
        self.stop()

    @staticmethod
    def get_robot_topic_path(
        client: projectairsim.ProjectAirSimClient, robot_name: str
    ):
        """
        Given an Project AirSim topic name, find the first topic containing the
        specified robot_name and return the first part of the topic name
        up to but not including the forward-slash ('/') terminating the robot
        name.

        Arguments:
          client - Project AirSim client object
          robot_name - Name of the robot
        """
        robot_topic_name = None
        reRobot = re.compile("(.*/robots/([^/]*))")
        for topic_name in client.topics:
            re_match = reRobot.match(topic_name)
            if re_match and (re_match.group(2) == robot_name):
                robot_topic_name = re_match.group(1)

        return robot_topic_name

    def set(self, channels):
        """
        Set the controller input channel values.

        Arguments:
          channels - Dictionary of channel values from controller
        """
        output_channels = []

        # Process the normal channels
        if not output_channels:
            for ichannel in range(len(self.rc_config.channel_map)):
                channel_entry = self.rc_config.channel_map[ichannel]

                if channel_entry.input_channel not in channels:
                    projectairsim_log().error(
                        f"set(): input channel {channel_entry.input_channel} not in channels"
                    )
                    channel_value = 0.0
                else:
                    channel_value = channels[channel_entry.input_channel]

                    # Map controller's value to intermediate range [-1 to +1]
                    if channel_value < channel_entry.input_dead_range.min:
                        channel_value = -self._calc_prop(
                            channel_value,
                            channel_entry.input_dead_range.min,
                            channel_entry.input_range.min,
                        )
                    elif channel_value > channel_entry.input_dead_range.max:
                        channel_value = self._calc_prop(
                            channel_value,
                            channel_entry.input_dead_range.max,
                            channel_entry.input_range.max,
                        )
                    else:
                        channel_value = 0.0

                    # Map intermediate value to output range
                    channel_value = (channel_value + 1.0) / 2.0 * (
                        channel_entry.output_range.max - channel_entry.output_range.min
                    ) + channel_entry.output_range.min

                # Add channel's value
                output_channels.append(channel_value)

        # Process OOB channels--these override the normal channels
        self._process_oob_channels(channels, output_channels)

        # Set new RC input values
        with self._lock:
            self._channels = output_channels

    def start(self):
        """
        Start sending RC inputs to the flight controller
        """
        if not self._event_exit.is_set():
            self._event_exit.clear()
            self.sender_thread.start()

    def stop(self):
        """
        Stop sending RC inputs to the flight controller
        """
        self._event_exit.set()

    def _calc_prop(self, value, value_first, value_last):
        """
        Returns the proportional position of value in the range
        [value_first, value_last]. The return value is limited to [0.0, 1.0].

        Note that negative values and ranges still return positive values.  For
        instance, a value of -50 for a range of [-10, -110] returns 0.4 since -50
        is 4/10ths of the way going from -10 to -110.  Flipping the range to
        [-110, -10], returns 0.6 because -50 is 6/10ths of the way from -110 to
        -10.

        Arguments:
            value - The value to consider
            value_first - First value of the range (inclusive)
            value_last - Last value of the range (inclusive)

        Returns:
            (Return) Value's proportional position in the range from value_first
                (0.0) to value_last (1.0)
        """
        value = (value - value_first) / (value_last - value_first)

        if value > 1.0:
            return 1.0
        elif value < 0.0:
            return 0.0

        return value

    def _process_oob_channels(self, controller_channels, output_channels):
        """
        Process out-of-band channels.  This method can modify the array of output
        channels that is sent to the flight controller.

        Arguments:
          controller_channels - Channel values from the input controller
          output_channels - Original outputs to flight controller

        Returns:
          output_channels - Outputs to send to flight controller
        """
        pass

    def _sender_thread(self):
        """
        RC input sender thread function.  This function periodically sends the
        RC input data to the flight controller
        """
        projectairsim_log().debug("Sender thread starting.")
        while not self._event_exit.wait(0.1):
            with self._lock:
                self._send_rc_input(self._channels)

        projectairsim_log().debug("Sender thread exiting.")

    def _send_rc_input(self, channels_array):
        """
        Send RC input channel values to flight controller

        Arguments:
          channels_array - Array of floats, one per RC channel
        """
        message = {"channels": channels_array}
        self.client.publish(self.rc_input_topic_name, message)


# ----------------------------------------------------------------------------
class SimpleFlightRC(BaseRC):
    """
    This class extends BaseRC for the Simple Flight flight controller.

    This class supports OOB channels to control additional functionality
    implemented by this class.

    Simple Flight requires periodic input in order to detect whether the remote
    control is still connected.  This class automatically sends the RC input
    channels periodically regardless of how frequent update() is called in order
    to support input controllers that don't return data unless input has changed.
    The start() and stop() methods will pause and resume sending RC input.
    """

    # Simple Flight by default assigns the following functions to these RC
    # channels.
    CHANNEL_XROLL = 0  # X-roll angle level/rate [-1.0 - 1.0]
    CHANNEL_ZYAW = 1  # Z-yaw angle level/rate [-1.0 - 1.0]
    CHANNEL_THROTTLE = 2  # Throttle channel [0.0 - 1.0]
    CHANNEL_YPITCH = 3  # Y-pitch angle/rate [-1.0 - 1.0]
    CHANNEL_ANGLE_LEVEL_OR_RATE = 4  # Axis channels controls axis level (0) or rate (1)
    CHANNEL_API_CONTROL_ENABLE = 5  # API control disable(0) or enable (1)

    # This class supports the following out-of-band channel indices.  OOB
    # channels control additional functionality implemented by this class and
    # are not sent to Simple Flight directly.
    #
    # To arm or disarm the vehicle, the corresponding channel must be asserted
    # for the minimum duration set by Simple Flight (200 ms by default) and the
    # throttle must be at zero or at least below the arm/disarm threshold
    # (0.1 by default.)  Asserting both arm and disarm channels simultaneously
    # causes undefined behavior.
    CHANNEL_OOB_ARM = "arm_vehicle"  # Arm vehicle; set to 0.5 or greater to assert
    CHANNEL_OOB_DISARM = (
        "disarm_vehicle"  # Disarm vehicle; set to 0.5 or greater to assert
    )

    # Special RC channel values sent to Simple Flight to arm the vehicle
    _channels_arm = [
        -1.0,  # X-roll
        1.0,  # Z-yaw
        0.0,  # Throttle
        1.0,  # Y-pitch
    ]

    # Special RC channel values sent to Simple Flight to disamrm the vehicle
    _channels_disarm = [
        1.0,  # X-roll
        -1.0,  # Z-yaw
        0.0,  # Throttle
        1.0,  # Y-pitch
    ]

    def __init__(self, client: projectairsim.ProjectAirSimClient, robot_name: str):
        """
        Constructor.

        The robot_name string is the name of the robot specified in the robot
        config file.

        Arguments:
          client - Project AirSim client object
          robot_name - Name of the robot using the Simple Flight controller
        """
        # Get the Simple Flight RC input topic starting with the robot name
        robot_topic_path = projectairsim.rc.SimpleFlightRC.get_robot_topic_path(
            client, robot_name
        )
        if robot_topic_path is None:
            raise ValueError(f'Robot "{robot_name}" wasn\'t found in the scene')
        projectairsim_log().info(
            f'Attaching as RC controller for robot "{robot_topic_path}"'
        )

        super().__init__(client, robot_topic_path + "/simple_flight/rc_input")

    def _process_oob_channels(self, controller_channels, output_channels):
        """
        Process out-of-band channels.  If this method returns a channel array,
        that array is sent to the flight controller instead of the standard RC
        input array.

        Arguments:
          controller_channels - Channel values from the input controller
          output_channels - Original outputs to flight controller

        Returns:
          output_channels - Outputs to send to flight controller
        """
        oob_channels = None

        if self.CHANNEL_OOB_DISARM in self.rc_config.channel_map_oob:
            channel_entry = self.rc_config.channel_map_oob[self.CHANNEL_OOB_DISARM]
            if controller_channels[channel_entry.input_channel] > 0.5:
                oob_channels = self._channels_disarm

        if (oob_channels is None) and (
            self.CHANNEL_OOB_ARM in self.rc_config.channel_map_oob
        ):
            channel_entry = self.rc_config.channel_map_oob[self.CHANNEL_OOB_ARM]
            if controller_channels[channel_entry.input_channel] > 0.5:
                oob_channels = self._channels_arm

        if oob_channels is not None:
            # Copy the OOB channels over the output channels
            for i in range(0, len(oob_channels)):
                # Don't override the throttle setting; the user must hold
                # it at zero in order to arm or disarm
                if i != self.CHANNEL_THROTTLE:
                    output_channels[i] = oob_channels[i]


# ----------------------------------------------------------------------------
class VirtualToggleSwitch:
    """
    This class implements a virtual toggle switch by converting push button
    actions into a toggle-like behavior.  This useful for input controller
    classes to create virtual toggle switches if the input controller doesn't
    have actual toggle switches.

    On a "push", the switch value toggles between 0.0 (off) or 1.0 (on).
    """

    def __init__(self, input_threshold: float = 0.5):
        """
        Constructor.

        Arguments:
          input_threshold - Value above which an input is considered a "push"
        """
        self.value = 0  # Current value
        self._input_is_on = (
            False  # Last input to update() was above self._input_threshold
        )
        self._input_threshold = (
            input_threshold  # Input value above which the toggle is considered "pushed"
        )

    def update(self, input):
        """
        Handle a new input value and update the switch value.

        Arguments:
          input - New input value

        Returns:
          (Return) - New switch value
        """
        if self._input_is_on:
            if input < self._input_threshold:
                self._input_is_on = False
        elif input >= self._input_threshold:
            self._input_is_on = True
            self.value = 1 if self.value < 0.5 else 0

        return self.value


# ----------------------------------------------------------------------------
class XboxInputController(BaseInputController):
    """
    Input controller for Xbox game controller.
    """

    def __init__(self):
        """
        Constructor.
        """
        self.channels = {
            "xLeft": 0,  # Left joystick, horizontal direction [-32768..32767]
            "xRight": 0,  # Right joystick, horizontal direction [-32768..32767]
            "yLeft": 0,  # Left joystick, vertical direction [-32768..32767]
            "yRight": 0,  # Right joystick, vertical direction [-32768..32767]
            "xHat": 0,  # Directional pad, horizontal direction [-1..1]
            "yHat": 0,  # Directional pad, vertical direction [-1..1]
            "zLeft": 0,  # Left trigger [-32768..32767]
            "zRight": 0,  # Right trigger [-32768..32767]
            "btnA": 0,  # "A" button [0, 1]
            "btnB": 0,  # "B" button [0, 1]
            "btnX": 0,  # "X" button [0, 1]
            "btnY": 0,  # "Y" button [0, 1]
            "btnBack": 0,  # "Back" button [0, 1]
            "btnStart": 0,  # "Start" button [0, 1]
            "btnShoulderLeft": 0,  # Left shoulder button [0, 1]
            "btnShoulderRight": 0,  # Right shoulder button [0, 1]
            "btnThumbLeft": 0,  # Left joystick thumb switch [0, 1]
            "btnThumbRight": 0,  # Right joystick thumb switch [0, 1]
        }

    def read(self):
        """
        Read the state of the input controller and return the current state.  This
        method usually does not return until controller has received new input.

        Returns:
          (Return) - Dictionary of controller channel values
        """
        is_updated = False

        while not is_updated:
            events = get_gamepad()
            for event in events:
                is_recognized = True
                if event.ev_type == "Absolute":
                    if event.code == "ABS_X":
                        self.channels["xLeft"] = event.state
                    elif event.code == "ABS_Y":
                        self.channels["yLeft"] = event.state
                    elif event.code == "ABS_RX":
                        self.channels["xRight"] = event.state
                    elif event.code == "ABS_RY":
                        self.channels["yRight"] = event.state
                    elif event.code == "ABS_HAT0X":
                        self.channels["xHat"] = event.state
                    elif event.code == "ABS_HAT0Y":
                        self.channels["yHat"] = event.state
                    elif event.code == "ABS_Z":
                        self.channels["zLeft"] = event.state
                    elif event.code == "ABS_RZ":
                        self.channels["zRight"] = event.state
                    else:
                        is_recognized = False
                elif event.ev_type == "Key":
                    if event.code == "BTN_EAST":
                        self.channels["btnB"] = event.state
                    elif event.code == "BTN_NORTH":
                        self.channels["btnY"] = event.state
                    elif event.code == "BTN_SOUTH":
                        self.channels["btnA"] = event.state
                    elif event.code == "BTN_WEST":
                        self.channels["btnX"] = event.state
                    elif (
                        event.code == "BTN_START"
                    ):  # Not a mistake, BTN_START is returned for the button marked "BACK" on the Xbox controller
                        self.channels["btnBack"] = event.state
                    elif (
                        event.code == "BTN_SELECT"
                    ):  # BTN_SELECT is returned for the button marked "START" on the Xbox controller
                        self.channels["btnStart"] = event.state
                    elif event.code == "BTN_THUMBL":
                        self.channels["btnThumbLeft"] = event.state
                    elif event.code == "BTN_THUMBR":
                        self.channels["btnThumbRight"] = event.state
                    elif event.code == "BTN_TL":
                        self.channels["btnShoulderLeft"] = event.state
                    elif event.code == "BTN_TR":
                        self.channels["btnShoulderRight"] = event.state
                    else:
                        is_recognized = False
                else:
                    is_recognized = False

                if is_recognized:
                    is_updated = True

        return self.channels


# ----------------------------------------------------------------------------
class XboxInputControllerSF(XboxInputController):
    """
    This class extends XboxInputController to add two virtual toggle switches
    for the Simple Flight flight controller: one to the set either angle-level
    or angle-rate control, another to enable or disable API control.
    """

    def __init__(self):
        """
        Constructor.
        """
        super().__init__()
        self._vts_level_rate = VirtualToggleSwitch()
        self._vts_enable_api_control = VirtualToggleSwitch()

    def read(self):
        """
        Read the state of the input controller and return the current state.  This
        method usually does not return until controller has received new input.

        Returns:
          (Return) - Dictionary of controller channel values
        """
        channels = super().read()

        # Update virtual toggle switches
        channels["switchLevelRate"] = self._vts_level_rate.update(channels["btnY"])
        channels["switchEnableAPIControl"] = self._vts_enable_api_control.update(
            channels["btnX"]
        )

        return channels
