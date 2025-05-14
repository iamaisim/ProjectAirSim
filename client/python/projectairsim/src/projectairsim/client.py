"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.
Python client for ProjectAirSim.
"""

import asyncio
import base64
import cryptography
import cryptography.hazmat.primitives.serialization
import cryptography.hazmat.primitives.asymmetric.padding
import cryptography.hazmat.primitives.hashes
import inspect
import json
import random
import threading
import time
from datetime import datetime, timezone
from sys import platform, version_info
from typing import Any, Dict

import msgpack
import pynng

import projectairsim.utils as utils
from projectairsim.types import FrameType, ProjectAirSimTopic, SerializationType
from projectairsim.utils import projectairsim_log


class ProjectAirSimClient:
    def __init__(self, address="127.0.0.1", port_topics=8989, port_services=8990):
        self.topics = {}
        self.subs = {}
        self.recv_topic_thread = None
        self.state = False
        self.topic_info_updated = False
        self.serialization_type = SerializationType.MSGPACK_JSON
        self.address = address
        self.port_topics = port_topics
        self.port_services = port_services
        self.lock = threading.Lock()
        self.socket_topics = None
        self.socket_services = None
        self.request_id = None

    def connect(self, connect_retries=10):
        """Connects to the server"""
        projectairsim_log().info(f"Connecting to simulation server at {self.address}")
        projectairsim_log().info(f"Connecting to simulation topics using pair1")

         # Socket for comm via ProjectAirSim topics
        self.socket_topics = pynng.Pair1(
            send_timeout=1000,  # Timeout after 1 second
        )
        # Socket for comm via ProjectAirSim services
        # TODO: Revisit timeout for service methods
        # TODO: Enable resend_time & handle on the server-side to prevent cancellation
        #       of on-going, long-running methods
        self.socket_services = pynng.Req0(
            recv_timeout=300000,  # Timeout after 5 minutes if no REP
            send_timeout=1000,  # Timeout after 1 second if send fails
            resend_time=-1,  # Disable retries
        )
        self.request_id = self.request_id_generator()

        retries = 0

        while retries < connect_retries:
            try:
                if "win" in platform:
                    # todo check linux API for socket.set_int_option()
                    self.socket_topics.dial(
                        f"tcp://{self.address}:{self.port_topics}".encode(), block=True
                    )
                    self.socket_services.dial(
                        f"tcp://{self.address}:{self.port_services}".encode(), block=True
                    )
                if "linux" in platform:
                    self.socket_topics.dial(
                        address=f"tcp://{self.address}:{self.port_topics}", block=True
                    )
                    self.socket_services.dial(
                        address=f"tcp://{self.address}:{self.port_services}", block=True
                    )
                retries = connect_retries + 100
            except:
                projectairsim_log().error("Failed to connect to sim server, retrying.")
                retries += 1
                time.sleep(1)

        if retries == connect_retries:
            projectairsim_log().info("Connection open failed.")
            return False
        
        projectairsim_log().info("Connection opened.")
        self.state = True
        self.recv_topic_thread = threading.Thread(target=self.__recv_topic)
        self.recv_topic_thread.start()
        projectairsim_log().info("Started the pub-sub topic receiving thread.")
        return True
    
    def get_topic_info(self):
        """This is used by World to get the list of topic info on reload scene."""
        projectairsim_log().info(
            "Getting the list of available topic info from the sim server..."
        )
        self.topic_info_updated = False
        self.subscribe("/$topics", self.on_topic_info)
        timeout_sec = 60
        timeout = time.time() + timeout_sec

        while self.topic_info_updated is False:
            time.sleep(0.01)  # wait until subscription message is received
            if time.time() > timeout:
                # TODO raise pynng.exceptions.Timeout exception?
                utils.projectairsim_log().warning(
                    "Timeout waiting to get topic info, disconnecting client."
                )
                self.disconnect()
                break

        if self.topic_info_updated:
            projectairsim_log().info("Successfully received topic info list.")

    def get_subscriptions_summary(self):
        return self.subs.copy()

    def on_topic_info(self, _, topic_info_msg):
        """Callback function used by get_topic_info"""
        self.topics = {}
        for msg in topic_info_msg:
            topic = ProjectAirSimTopic(
                msg["path"], msg["type"], msg["message_type"], msg["frequency"]
            )
            self.topics[topic.path] = topic
        self.topic_info_updated = True

    def subscribe(self, topic, callback, reliability=1.0):
        """Subscribes to a server topic

        Args:
            topic (str): the name of the topic
            callback (callable): function to call when data is received
        """
        if topic in self.subs:
            self.subs[topic]["callbacks"].append(callback)
            self.subs[topic]["reliability"] = reliability
        else:
            self.subs[topic] = {"reliability": reliability, "callbacks": [callback]}
        subscribe_msg = self.__make_subscribe_frame(topic)
        # self.socket_topics.send(subscribe_msg)
        self.try_send(subscribe_msg)

    def update_subscription_options(self, topic, reliability):
        """Updates settings of a server topic

        Args:
            topic (str): the name of the topic
        """
        self.subs[topic]["reliability"] = max(0.0, min(reliability, 1.0))

    def publish(self, topic, message):
        """Publishes a message to a given server topic"""
        frame = self.__make_message_frame(topic, message)
        # self.socket_topics.send(frame)
        self.try_send(frame)

    def set_interactive_feature(self, feature_name, enabled):
        """Sets the state of an interactive feature"""
        feature_req: Dict = {
            "method": "/Sim/SetInteractiveFeature",
            "params": {"feature_id": feature_name, "enable": enabled},
            "version": 1.0,
        }

        feature_res: bool = self.request(feature_req)

        if not feature_res:
            utils.projectairsim_log().error("Failed to set interactive feature.")
        else:
            utils.projectairsim_log().info("Successfully set interactive feature.")

    def unsubscribe(self, topics):
        """Unsubscribes from one or more server topics

        Args:
            topics (str or List): the topics. can be given as a string for one topic
                or a list for multiple
        """
        if not isinstance(topics, list):
            topics = [topics]

        if 0:
            unsubscribe_req: Dict = {
                "method": "/Sim/Unsubscribe",
                "params": {"topic_paths": topics},
                "version": 1.0,
            }
            unsubscribe_res: bool = self.request(unsubscribe_req)

            if not unsubscribe_res:
                utils.projectairsim_log().error("Failed to unsubscribe topics.")

        for topic in topics:
            frame = self.__make_unsubscribe_frame(topic)
            self.try_send(frame)

            self.subs.pop(topic, None)

    def unsubscribe_all(self):
        """Unsubscribes from all server topics"""

        frame = self.__make_unsubscribeall_frame()
        self.try_send(frame)

    def try_send(self, msg, max_retries=5):
        """Attempts to send a message up to max_retries times"""
        num_retries = 0

        while num_retries < max_retries:
            try:
                num_retries += 1
                self.socket_topics.send(msg)
                return
            except pynng.exceptions.NNGException:
                utils.projectairsim_log().warning(
                    f"NNG failed to send message. Retry: #{num_retries}/{max_retries}"
                )

        utils.projectairsim_log().warning("Max retries reached.")

    def request(self, request_data) -> Any:
        """Makes a synchronous request to the Sim server and returns the response

        Args:
            request_data (Dict): Request object as per JSON-RPC spec for named
             parameters with the `"jsonrpc"` field replaced by `"version"`. Example:
             {"method": "/xyz", "params": {"data": b"123abc"}, "version": 1.0, "id": 1}
             *Note*: The `"params"` field should have the `"data"` member
        Raises:
            RuntimeError: When a Timeout exception is raised by NNG while
                processing the request
        Returns:
            Any: Response for the request
        """
        request_packed = self.preprocess_request(request_data)
        self.socket_services.send(request_packed)
        try:
            response = self.socket_services.recv_msg()
            output = self.postprocess_response(response)
            return output
        except pynng.exceptions.Timeout:
            utils.projectairsim_log().error(
                f"Timed out receiving response for request: {request_data}."
            )
            self.disconnect()
            raise RuntimeError(
                f"Fatal Timeout ocurred while processing request for"
                f" method: {request_data.get('method')}"
            )

    async def request_async(self, request_data, callback=None) -> asyncio.Task:
        """Asynchronously send request to the Sim server and return response

        Args:
            request_data (Dict): Request message dict:
              {"method": ..., "params": ..., "version": ...}

            callback (Callable): Callback to call when request is complete

        Returns:
            asyncio.Task: Returns a awaitable Task that wraps the coroutine for
                          processing the result of the requested API call.
        """
        request_packed = self.preprocess_request(request_data)
        # Use a socket context to allow concurrent requests with separate state
        ctx = self.socket_services.new_context()
        await ctx.asend(request_packed)

        response = ctx.arecv_msg()
        post_proc_task = asyncio.create_task(
            self.postprocess_response_async_with_callback(response, callback)
        )
        self.set_task_name(post_proc_task, request_data["method"])
        post_proc_task.add_done_callback(self.check_for_exception)

        return post_proc_task

    def set_task_name(self, task: asyncio.Task, task_name: str) -> None:
        """Sets the name of an asyncio task"""
        if version_info < (3, 8):
            # For Python ver < 3.8, just dynamically add a name string
            task.name = task_name
        else:
            # For Python ver >= 3.8, use task naming methods
            task.set_name(task_name)

    def get_task_name(self, task: asyncio.Task) -> str:
        """Retrieves the name of an asyncio task"""
        return task.name if version_info < (3, 8) else task.get_name()

    def check_for_exception(self, task: asyncio.Task) -> None:
        # Check for any exception raised during the async task's execution and re-raise
        # it. Note: task.exception() should only be executed after task is done, so
        # this function is called using task.add_done_callback(). If called on a
        # cancelled task, it will raise a CancelledError exception. If called on a task
        # that's not done yet, it will raise an InvalidStateError exception.
        try:
            exc = task.exception()
            if isinstance(exc, Exception):
                projectairsim_log().error(
                    f"Exception occurred for task '{self.get_task_name(task)}': {exc}",
                    exc_info=exc,
                )
                self.disconnect()
                raise exc
        except asyncio.CancelledError:
            projectairsim_log().warning(
                "An async task was cancelled before completing."
            )

    @staticmethod
    def request_id_generator() -> int:
        """Static method to generate request IDs"""
        req_id = 0
        while True:
            yield req_id
            req_id += 1

    def preprocess_request(self, request):
        """Internal function for processing requests before they are sent"""
        # Request data validation should go here.
        # Use a unique, auto-generated ID for each request
        request["id"] = next(self.request_id)

        if self.serialization_type == SerializationType.PURE_JSON:
            request_json = json.dumps(request)
            request_packed = request_json.encode()
        else:  # default, SerializationType.MSGPACK_JSON
            request["params"] = {
                "data": msgpack.packb(request["params"], use_bin_type=True)
            }
            # Encode the str-type values in the request dict (keys should stay as str
            # for msgpack's use_bin_type=True)
            request = {
                key: val.encode() if isinstance(val, str) else val
                for key, val in request.items()
            }
            request_packed = msgpack.packb(request, use_bin_type=True)

        return request_packed

    def postprocess_response(self, response):
        """Performs error handling, type checking, casting etc. on a response message"""
        response = response.bytes
        result = None

        if self.serialization_type == SerializationType.PURE_JSON:
            # Decode bytes to str
            response = response.decode()
            # Load the response JSON from str
            response = json.loads(response)
            # Convert to native Dict
            response = dict(response)
            if "result" in response:  # ResponseSuccess
                result = response["result"]
            else:  # ResponseFailure
                result = response.get("error", None)
                raise RuntimeError(
                    f"ERROR code: {result['code']}, message: {result['message']}"
                )
        else:  # default, SerializationType.MSGPACK_JSON
            response = msgpack.unpackb(response, raw=False)
            # Recursively decode all parts of response data
            response = utils.decode(response)
            if "result" in response:  # ResponseSuccess
                result = response["result"]["data"]
            elif "error" in response:  # ResponseFailure
                result = response["error"]["data"]
                raise RuntimeError(
                    f"ERROR code: {result['code']}, message: {result['message']}"
                )

        return result

    async def postprocess_response_async_with_callback(
        self, response, callback: callable
    ) -> None:
        """Helper function that for asynchronously postprocessing a response"""
        if inspect.isawaitable(response):
            response = await response
        result = self.postprocess_response(response)
        # Return or validate result here
        if callback is not None:
            callback(result)

    def disconnect(self):
        """Disconnects from the server"""
        projectairsim_log().info("Disconnecting from simulation server...")
        self.state = False  # stops receive loop thread
        if self.recv_topic_thread is not None and self.recv_topic_thread.is_alive():
            self.recv_topic_thread.join()
        # Unsubscribe after stopping the receive loop that processes the
        # incoming subscription messages.
        self.unsubscribe_all()
        self.socket_topics.close()
        self.socket_services.close()
        projectairsim_log().info("Disconnected.")

    def __get_authorization_token_public_key(self) -> str:
        """Get the public RSA key to encrypt the client authorization token.

        Returns:
            Public RSA key in OpenSSL format, base-64 encoded
        """
        get_authorization_token_public_key_req: Dict = {
            "method": f"/Sim/GetAuthorizationTokenPublicKey",
            "params": {},
            "version": 1.0,
        }
        encryption_key = self.request(get_authorization_token_public_key_req)
        return encryption_key

    def set_authorization_token(self, token: str) -> datetime:
        """Set the client authorization token.

        Returns:
            datatime when the token expires
        """
        encryption_key_openssh = bytes(
            self.__get_authorization_token_public_key(), "utf-8"
        )
        rsa_key_public = (
            cryptography.hazmat.primitives.serialization.load_ssh_public_key(
                encryption_key_openssh
            )
        )
        ciphertext = rsa_key_public.encrypt(
            token,
            cryptography.hazmat.primitives.asymmetric.padding.OAEP(
                mgf=cryptography.hazmat.primitives.asymmetric.padding.MGF1(
                    algorithm=cryptography.hazmat.primitives.hashes.SHA1()
                ),
                algorithm=cryptography.hazmat.primitives.hashes.SHA1(),
                label=None,
            ),
        )
        token_encrypted_base64 = base64.b64encode(ciphertext)

        params: Dict = {"token_encrypted_base64": token_encrypted_base64}
        set_authorization_token_req: Dict = {
            "method": f"/Sim/SetAuthorizationToken",
            "params": params,
            "version": 1.0,
        }
        timestamp_expiration = self.request(set_authorization_token_req)
        if timestamp_expiration == 0xFFFFFFFFFFFFFFFF:
            datetime_authorization_expired = datetime.max
            projectairsim_log().info(f"Client successfully authorized (no expiration)")
        else:
            datetime_authorization_expired = datetime.fromtimestamp(
                timestamp_expiration, timezone.utc
            )
            projectairsim_log().info(
                f"Client successfully authorized"
                f" until {datetime_authorization_expired.astimezone()}"
            )

        return datetime_authorization_expired

    def __make_subscribe_frame(self, topic):
        frame = [FrameType.SUBSCRIBE, topic, ""]
        return msgpack.packb(frame, use_bin_type=True)

    def __make_unsubscribe_frame(self, topic):
        frame = [FrameType.UNSUBSCRIBE, topic, ""]
        return msgpack.packb(frame, use_bin_type=True)

    def __make_unsubscribeall_frame(self):
        frame = [FrameType.UNSUBSCRIBEALL, "", ""]
        return msgpack.packb(frame, use_bin_type=True)

    def __make_message_frame(self, topic, message):
        packed_msg = msgpack.packb(message, use_bin_type=True)
        frame = [FrameType.MESSAGE, topic, packed_msg]
        return msgpack.packb(frame, use_bin_type=True)

    def __recv_topic_frame(self):
        try:
            # Check if new data is ready to be received with non-blocking recv() call
            frame_packed = self.socket_topics.recv_msg(block=False)
        except pynng.exceptions.TryAgain:
            return
        
        # If data was ready to be received, process and return it.
        # (Topic frames are packed as arrays instead of maps with string keys, so
        # need to unpack with raw=True to handle the binary values. Also, from msgpack
        # 1.0 the default buffer size limit was increased to 100 MB so no need to
        # set the limit explicitly anymore.)
        frame = msgpack.unpackb(frame_packed.bytes, raw=True)
        return frame

    def __recv_topic(self):
        while self.state:
            frame = None
            try:
                frame = self.__recv_topic_frame()
            except Exception as e:
                utils.projectairsim_log().warning(e, exc_info=True)
                break

            if frame is not None:
                topic_name = frame[1].decode("utf-8")

                if topic_name not in self.subs:
                    projectairsim_log().warn(
                        f"Received topic '{topic_name}' not found in client"
                        f" subscription list: {self.subs}"
                    )
                    continue

                reliability = self.subs[topic_name]["reliability"]
                rand = random.random()

                if (topic_name in self.subs) and (rand <= reliability):
                    if topic_name not in self.topics:
                        # In case this topic isn't found in the list of topic info,
                        # use a dummy topic with unknown info
                        topic = ProjectAirSimTopic(topic_name, "unknown", "unknown", 0)
                    else:
                        topic = self.topics[topic_name]

                    msg = msgpack.unpackb(frame[2], raw=False)
                    callbacks = self.subs[topic_name]["callbacks"]
                    
                    for callback in callbacks:
                         callback(topic, msg)
