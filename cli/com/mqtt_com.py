#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# We kindly request you to use one or more of the following phrases to refer to
# foxBMS in your hardware, software, documentation or advertising materials:
#
# - "This product uses parts of foxBMS®"
# - "This product includes parts of foxBMS®"
# - "This product is derived from foxBMS®"

"""Implements the a basic MQTT communication functionalities"""

import json
import signal
import socket
import ssl
from pathlib import Path
from queue import Empty
from time import sleep
from typing import Any

from paho.mqtt import client as mqtt_client
from paho.mqtt.client import Client, MQTTMessage
from paho.mqtt.enums import CallbackAPIVersion

from ..helpers.logger import add_queue_handler, logger
from .com_interface import ComInterface, ProcessInterface
from .parameter import ComControl, MQTTParameter


class MQTT(ComInterface):
    """Communication interface for sending and receiving messages via MQTT protocol.
    Initializes and manages a background process for MQTT communication.
    """

    def __init__(self, name: str, parameter: MQTTParameter) -> None:
        """Initializes the MQTT communication interface.

        Args:
            name (str): The name of the interface.
            parameter (MQTTParameter): Configuration parameters for MQTT.
        """
        super().__init__(name=name)
        self._processes[MQTTProcess.__name__] = MQTTProcess(
            name, self.control, parameter
        )

    def read(self, block: bool = False, timeout: float | None = None) -> dict | None:
        """Reads an incoming MQTT message from the output queue.

        Args:
            block (bool): Whether to block if the queue is empty.
            timeout (float | None): Timeout for blocking (in seconds).

        Returns:
            dict | None: Dictionary with 'topic' and 'data' if available, otherwise None.
        """
        # If the related process is not alive and the queue is empty, reading would block.
        if self.control.output.empty():
            if not self.is_alive():
                debug_text = "Input queue can not be read, because related process is not running."
                logger.debug(debug_text)
            return None
        return self.control.output.get(block=block, timeout=timeout)

    def write(self, msg: dict) -> None:
        """Publishes a message to a specific MQTT topic via the input queue.

        Args:
            msg (dict): The MQTT message as dict

        Raises:
            RuntimeError: If the related MQTT process is not running.
        """
        if not self.is_alive():
            error_text = "Output queue can not be written, because related process is not running."
            raise RuntimeError(error_text)
        self.control.input.put(msg)


class MQTTProcess(ProcessInterface):
    """Background process for handling MQTT client connection, publishing, and subscription.
    Manages the MQTT client lifecycle and message routing.
    """

    def __init__(
        self, name: str, control: ComControl, parameter: MQTTParameter
    ) -> None:
        """Initializes the MQTT process.

        Args:
            name (str): Process name.
            control (ComControl): Communication control object.
            parameter (MQTTParameter): MQTT configuration parameters.
        """
        super().__init__(name=name, daemon=True)
        self.name = name
        self.control = control
        self.parameter = parameter
        self._client: Client | None = None

    def run(self) -> None:
        """Starts the MQTT process, connects to the broker, and begins the event loop."""
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        add_queue_handler(self.control.logger)
        logger.setLevel(self.control.log_level)
        # The client object must be created here (not in __init__)
        # to avoid "cannot pickle '_thread.lock' object" exceptions.
        self._client = self._connect()
        logger.debug("MQTT client connected.")
        if isinstance(self._client, Client):
            logger.debug("MQTT client loop has started.")
            self._client.loop_start()
            self.control.ready.set()
            self._read_from_input()

    def _connect(self) -> Client | None:
        """Connects to the MQTT broker and subscribes to topics.

        Returns:
            Client | None: The connected MQTT Client instance, or None on failure.

        Handles:
            socket.gaierror: Invalid broker address.
            socket.timeout: Connection timeout.
            ssl.SSLError: SSL certificate/password errors.
        """
        try:
            return self._connect_sub()
        except socket.gaierror:
            logger.error(
                "Broker address '%s' seems to be wrong.", self.parameter.broker
            )
        except TimeoutError:
            logger.error(
                "Connection to broker '%s' with port '%s' can not be established, because of a "
                "timeout. Port correct ?",
                self.parameter.broker,
                self.parameter.port,
            )
        except ssl.SSLError:
            logger.error("SSL Error because of wrong certificate or password ?")
        self.shutdown()
        return None

    def _connect_sub(self) -> Client:
        """Creates and configures the MQTT client, connects to the broker, and subscribes to topics.

        Returns:
            Client: The initialized and connected MQTT Client.
        """
        client = mqtt_client.Client(
            client_id="fox_MQTT",
            callback_api_version=CallbackAPIVersion.VERSION2,
        )
        client = self._set_username_and_pwd(client)
        client = self._set_tls(client)
        client.connect(self.parameter.broker, self.parameter.port)
        for topic in self.parameter.subscribe:
            client.subscribe(topic)
        client.on_message = self._write_to_output
        return client

    def _set_username_and_pwd(self, client: Client) -> Client:
        """Sets username and password for MQTT client authentication if provided.

        Args:
            client (Client): The MQTT client instance.

        Returns:
            Client: The MQTT client with authentication set.

        Raises:
            ssl.SSLError: If password is missing when username is provided.
        """
        if self.parameter.username:
            if not self.parameter.password:
                logger.error("Password is missing!")
                raise ssl.SSLError
            client.username_pw_set(
                username=self.parameter.username, password=self.parameter.password
            )
        return client

    def _set_tls(self, client: Client) -> Client:
        """Configures TLS/SSL for the MQTT client if a certificate is provided.

        Args:
            client (Client): The MQTT client instance.

        Returns:
            Client: The MQTT client with TLS configured.

        Raises:
            ssl.SSLError: If the certificate file does not exist.
        """
        if self.parameter.tls_cert:
            tls_cert = Path(self.parameter.tls_cert)
            if Path(tls_cert).is_file():
                client.tls_set(
                    ca_certs=self.parameter.tls_cert,
                    cert_reqs=ssl.CERT_REQUIRED,
                    tls_version=ssl.PROTOCOL_TLS_CLIENT,
                )
            else:
                logger.error("TLS certificate not found!")
                raise ssl.SSLError
        return client

    def _write_to_output(
        self,
        _client: Client,
        _user_data: Any,  # noqa: ANN401
        msg: MQTTMessage,
    ) -> None:
        """Callback function for handling incoming MQTT messages.

        Puts messages into the output queue.

        Args:
            client: The MQTT client instance.
            user_data: User data (unused).
            msg: The received MQTT message.
        """
        msg_dict = {"topic": msg.topic, "data": msg.payload.decode()}
        self.control.output.put(msg_dict)

    def _read_from_input(self) -> None:
        """Reads outgoing messages from the input queue and publishes them via MQTT.

        Handles:
            JSONDecodeError: If a message cannot be parsed.
            KeyError: If a message does not contain required keys.
        """
        try:
            self._read_from_input_sub()
        except json.decoder.JSONDecodeError as e:
            logger.error(
                "The message '%s' in the '%s' queue "
                "can not be converted to a dictionary.",
                e.doc,
                self.name,
            )
        except KeyError:
            logger.error(
                "One message from the input read by '%s' does "
                "not contain the keyword topic or data.",
                self.name,
            )
        self.shutdown()

    def _read_from_input_sub(self) -> None:
        """Continuously reads messages from the input queue and publishes them to MQTT topics.

        Raises:
            TypeError: If the MQTT client is not initialized.
        """
        if not isinstance(self._client, Client):
            error_text = "Client has not yet started."
            raise TypeError(error_text)
        while not self.control.shutdown.is_set():
            try:
                msg = self.control.input.get(block=False)
                topic = msg["topic"]
                msg_data = msg["data"]
                self._client.publish(topic, msg_data)
            except Empty:
                sleep(0.01)
                continue
