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

"""Testing file 'cli/com/mqtt_com.py'."""

import json
import socket
import ssl
import sys
import unittest
from pathlib import Path
from queue import Empty
from time import sleep
from typing import Any
from unittest.mock import MagicMock, Mock, call, patch

try:
    from cli.com.mqtt_com import MQTT, MQTTProcess
    from cli.com.parameter import ComControl, MQTTParameter
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.com.mqtt_com import MQTT, MQTTProcess
    from cli.com.parameter import ComControl, MQTTParameter


class TestMQTTInit(unittest.TestCase):
    """Test initialization of MQTT class and process registration."""

    @patch("cli.com.mqtt_com.MQTTProcess")
    def test_init_sets_process(self, mock_process: Mock) -> None:
        """Verify that the MQTTProcess is correctly registered in the _processes attribute."""
        mock_process.__name__ = "DummyProcess"
        parameter: MQTTParameter = MQTTParameter(
            broker="test", port="111", subscribe=["test"]
        )
        mqtt: MQTT = MQTT("testname", parameter)
        self.assertIn("DummyProcess", mqtt._processes)  # pylint: disable=protected-access
        self.assertTrue(hasattr(mqtt, "_processes"))


class TestMQTTRead(unittest.TestCase):
    """Test the read method of MQTT."""

    @patch("cli.com.mqtt_com.MQTTProcess")
    def setUp(self, mock_process: Mock) -> None:  # pylint: disable=arguments-differ
        """Set up a dummy MQTT instance for read tests."""
        mock_process.__name__ = "test"
        self.parameter: MQTTParameter = MQTTParameter(
            broker="test", port="111", subscribe=["test"]
        )
        self.mqtt: MQTT = MQTT("testname", self.parameter)
        self.mqtt.is_alive = MagicMock(return_value=True)

    def test_read_returns_dict(self) -> None:
        """Test that read returns a dictionary from the output queue."""
        self.mqtt.control.output.empty = Mock(return_value=False)
        self.mqtt.control.output.get = MagicMock(
            return_value={"topic": "t", "data": "d"}
        )
        result = self.mqtt.read()
        self.assertEqual(result, {"topic": "t", "data": "d"})

    def test_read_none_if_not_alive_and_empty(self) -> None:
        """Test that read returns None if process is not alive and queue is empty."""
        self.mqtt.control.output.empty = MagicMock(return_value=True)
        self.mqtt.is_alive = MagicMock(return_value=False)
        with patch("cli.com.mqtt_com.logger") as mock_logger:
            return_value = self.mqtt.read()
            mock_logger.debug.assert_called_once_with(
                "Input queue can not be read, because related process is not running."
            )
        self.assertIsNone(return_value)

    def test_read_none_if_alive_and_empty(self) -> None:
        """Test that read returns None if process is not alive and queue is empty."""
        self.mqtt.control.output.empty = MagicMock(return_value=True)
        self.mqtt.is_alive = MagicMock(return_value=True)
        self.assertIsNone(self.mqtt.read())


class TestMQTTWrite(unittest.TestCase):
    """Test the write method of MQTT."""

    @patch("cli.com.mqtt_com.MQTTProcess")
    def setUp(self, mock_process: Mock) -> None:  # pylint: disable=W0221
        """Set up a dummy MQTT instance for write tests."""
        mock_process.__name__ = "test"
        self.parameter: MQTTParameter = MQTTParameter(
            broker="test", port="111", subscribe=["test"]
        )
        self.mqtt: MQTT = MQTT("testname", self.parameter)
        self.mqtt.is_alive = MagicMock(return_value=True)

    def test_write_puts_message(self) -> None:
        """Test that write puts the correct message into the input queue."""
        self.mqtt.control.input.put = MagicMock()
        self.mqtt.write({"topic": "topic1", "data": "payload1"})
        self.mqtt.control.input.put.assert_called_once_with(
            {"topic": "topic1", "data": "payload1"}
        )

    def test_write_raises_if_not_alive(self) -> None:
        """Test that write raises RuntimeError if the process is not alive."""
        self.mqtt.is_alive = MagicMock(return_value=False)
        with self.assertRaises(RuntimeError) as context:
            self.mqtt.write({"t": "d"})
        self.assertIn("Output queue can not be written", str(context.exception))


class TestMQTTStart(unittest.TestCase):
    """Test the start call of MQTT."""

    def test_start_call(self) -> None:
        """Verify that calling the start method will create the process without an
        exception.
        """
        control: ComControl = ComControl()
        parameter: MQTTParameter = MQTTParameter(
            broker="test", port="111", subscribe=["test"]
        )
        proc: MQTTProcess = MQTTProcess("name", control, parameter)
        proc.start()
        proc.terminate()
        # Wait until process is dead then call close()
        while proc.is_alive():
            sleep(1)
        proc.close()


class TestMQTTProcessInit(unittest.TestCase):
    """Test initialization of MQTTProcess."""

    def test_init_sets_attributes(self) -> None:
        """Verify that attributes are set correctly during initialization."""
        control: ComControl = ComControl()
        parameter: MQTTParameter = MQTTParameter(
            broker="test", port="111", subscribe=["test"]
        )
        proc: MQTTProcess = MQTTProcess("name", control, parameter)
        self.assertEqual(proc.name, "name")
        self.assertEqual(proc.control, control)
        self.assertEqual(proc.parameter, parameter)
        self.assertIsNone(proc._client)  # pylint: disable=protected-access


@patch("cli.com.mqtt_com.isinstance")
@patch("cli.com.mqtt_com.add_queue_handler")
@patch("cli.com.mqtt_com.logger")
class TestMQTTProcessRun(unittest.TestCase):
    """Test the run method of MQTTProcess."""

    def test_run_starts_everything(
        self, mock_logger: Mock, mock_add_queue_handler: Mock, mock_isinstance: Mock
    ) -> None:
        """Test that run sets up, starts client loop, and reads from input when client is valid."""
        mock_isinstance.return_value = True
        control: ComControl = ComControl()
        parameter: MQTTParameter = MQTTParameter(
            broker="test", port="111", subscribe=["test"]
        )
        proc: MQTTProcess = MQTTProcess("test", control, parameter)
        dummy_client: Any = MagicMock()
        proc._connect = MagicMock(return_value=dummy_client)  # pylint: disable=protected-access
        proc._read_from_input = MagicMock()  # pylint: disable=protected-access
        control.ready.set = MagicMock()
        proc.run()
        mock_add_queue_handler.assert_called_once_with(control.logger)
        mock_logger.setLevel.assert_called_once_with(control.log_level)
        proc._connect.assert_called_once()  # pylint: disable=protected-access
        dummy_client.loop_start.assert_called_once()
        control.ready.set.assert_called_once()
        proc._read_from_input.assert_called_once()  # pylint: disable=protected-access

    def test_run_starts_but_no_client(
        self, mock_logger: Mock, mock_add_queue_handler: Mock, mock_isinstance: Mock
    ) -> None:
        """Test that run does not start client loop if client is invalid."""
        mock_isinstance.return_value = False
        control: ComControl = ComControl()
        parameter: MQTTParameter = MQTTParameter(
            broker="test", port="111", subscribe=["test"]
        )
        proc: MQTTProcess = MQTTProcess("test", control, parameter)
        dummy_client = MagicMock()
        # pylint: disable=protected-access
        proc._connect = MagicMock(return_value=dummy_client)
        proc._read_from_input = MagicMock()
        control.ready.set = MagicMock()
        proc.run()
        mock_add_queue_handler.assert_called_once_with(control.logger)
        mock_logger.setLevel.assert_called_once_with(control.log_level)
        proc._connect.assert_called_once()  # pylint: disable=protected-access
        dummy_client.loop_start.assert_not_called()
        control.ready.set.assert_not_called()
        proc._read_from_input.assert_not_called()  # pylint: disable=protected-access


@patch("cli.com.mqtt_com.logger")
class TestMQTTProcessConnect(unittest.TestCase):
    """Test the _connect method of MQTTProcess."""

    def setUp(self) -> None:
        """Set up a dummy MQTTProcess instance for connection tests."""
        control = ComControl()
        parameter = MQTTParameter(broker="test", port="111", subscribe=["test"])
        self.proc = MQTTProcess("test", control, parameter)

    def test_connect_success(self, _: Mock) -> None:
        """Test successful connection returns client."""
        client: Any = MagicMock()
        self.proc._connect_sub = MagicMock(  # pylint: disable=protected-access
            return_value=client
        )
        result: Any = self.proc._connect()  # pylint: disable=protected-access
        self.assertEqual(result, client)

    def test_connect_gaierror(self, mock_logger: Mock) -> None:
        """Test that a socket.gaierror triggers error log and shutdown."""
        self.proc._connect_sub = MagicMock(  # pylint: disable=protected-access
            side_effect=socket.gaierror
        )
        self.proc.shutdown = MagicMock()
        result: Any = self.proc._connect()  # pylint: disable=protected-access
        self.assertIsNone(result)
        self.proc.shutdown.assert_called_once()
        mock_logger.error.assert_called_once()

    def test_connect_timeout(self, mock_logger: Mock) -> None:
        """Test that a socket.timeout triggers error log and shutdown."""
        self.proc._connect_sub = MagicMock(  # pylint: disable=protected-access
            side_effect=socket.timeout
        )
        self.proc.shutdown = MagicMock()
        result: Any = self.proc._connect()  # pylint: disable=protected-access
        self.assertIsNone(result)
        self.proc.shutdown.assert_called_once()
        mock_logger.error.assert_called_once()

    def test_connect_sslerror(self, mock_logger: Mock) -> None:
        """Test that a ssl.SSLError triggers error log and shutdown."""
        self.proc._connect_sub = MagicMock(  # pylint: disable=protected-access
            side_effect=ssl.SSLError
        )
        self.proc.shutdown = MagicMock()
        result: Any = self.proc._connect()  # pylint: disable=protected-access
        self.assertIsNone(result)
        self.proc.shutdown.assert_called_once()
        mock_logger.error.assert_called_once()


class TestMQTTProcessPrivateConnect(unittest.TestCase):
    """Test the __connect method of MQTTProcess."""

    @patch("cli.com.mqtt_com.mqtt_client.Client")
    def test_private_connect_calls_setup_and_connects(self, mock_client: Mock) -> None:
        """Test that __connect configures client and subscribes to topics."""
        dummy_client = Mock(name="DummyClient")
        mock_client.return_value = dummy_client
        control: ComControl = ComControl()
        parameter: MQTTParameter = MQTTParameter(
            broker="test", port="111", subscribe=["topic/1", "topic/2"]
        )
        proc: MQTTProcess = MQTTProcess("test", control, parameter)
        proc._set_username_and_pwd = MagicMock(side_effect=lambda c: c)  # pylint: disable=protected-access
        proc._set_tls = MagicMock(side_effect=lambda c: c)  # pylint: disable=protected-access
        proc.parameter = parameter
        result: Any = proc._connect_sub()  # pylint: disable=protected-access
        self.assertEqual(result, dummy_client)
        self.assertEqual(
            mock_client.return_value.subscribe.call_args_list,
            [call("topic/1"), call("topic/2")],
        )
        self.assertIsNotNone(mock_client.return_value.on_message)


@patch("cli.com.mqtt_com.logger")
class TestMQTTProcessSetUsernamePwd(unittest.TestCase):
    """Test the _set_username_and_pwd method of MQTTProcess."""

    def setUp(self) -> None:
        """Set up dummy client and control for authentication tests."""
        self.control: ComControl = ComControl()
        self.dummy_client = Mock()

    def test_set_username_and_pwd_sets_when_both_given(self, _: Mock) -> None:
        """Test that username and password are set if both are provided."""
        parameter: MQTTParameter = MQTTParameter(
            broker="test",
            port="111",
            subscribe=["test"],
            username="user",
            password="pwd",  # noqa
        )
        proc: MQTTProcess = MQTTProcess("test", self.control, parameter)
        # pylint: disable-next=protected-access
        result = proc._set_username_and_pwd(self.dummy_client)
        self.dummy_client.username_pw_set.assert_called_once_with(
            username="user",
            password="pwd",  # noqa
        )
        self.assertEqual(result, self.dummy_client)

    def test_set_username_and_pwd_raises_when_missing_pwd(
        self, mock_logger: Mock
    ) -> None:
        """Test that missing password raises ssl.SSLError."""
        parameter: MQTTParameter = MQTTParameter(
            broker="test",
            port="111",
            subscribe=["test"],
            username="user",
            password=None,
        )
        proc: MQTTProcess = MQTTProcess("test", self.control, parameter)
        with self.assertRaises(ssl.SSLError):
            proc._set_username_and_pwd(self.dummy_client)  # pylint: disable=protected-access
        mock_logger.error.assert_called_once()

    def test_set_username_and_pwd_no_username(self, _: Mock) -> None:
        """Test that if not username is passed the same client is returned"""
        parameter: MQTTParameter = MQTTParameter(
            broker="test",
            port="111",
            subscribe=["test"],
            username=None,
            password=None,
        )
        proc: MQTTProcess = MQTTProcess("test", self.control, parameter)
        result = proc._set_username_and_pwd(self.dummy_client)  # pylint: disable=protected-access
        self.assertEqual(result, self.dummy_client)


@patch("cli.com.mqtt_com.Path")
@patch("cli.com.mqtt_com.logger")
class TestMQTTProcessSetTLS(unittest.TestCase):
    """Test the _set_tls method of MQTTProcess."""

    def setUp(self) -> None:
        """Set up dummy client and parameters for TLS tests."""
        self.control: ComControl = ComControl()
        self.dummy_client = Mock()
        self.parameter: MQTTParameter = MQTTParameter(
            broker="test", port="111", subscribe=["test"], tls_cert="cert.pem"
        )

    def test_set_tls_sets_if_cert_exists(self, _: Mock, mock_path: Mock) -> None:
        """Test that TLS is set if certificate file exists."""
        proc: MQTTProcess = MQTTProcess("test", self.control, self.parameter)
        mock_path.return_value.is_file.return_value = True
        result: Any = proc._set_tls(self.dummy_client)  # pylint: disable=protected-access
        self.dummy_client.tls_set.assert_called_once()
        self.assertEqual(result, self.dummy_client)

    def test_set_tls_sets_no_cert(self, *_: list[Mock]) -> None:
        """Test that there is no TLS certificate passed."""
        parameter: MQTTParameter = MQTTParameter(
            broker="test", port="111", subscribe=["test"]
        )
        proc: MQTTProcess = MQTTProcess("test", self.control, parameter)
        proc._set_tls(self.dummy_client)  # pylint: disable=protected-access
        self.dummy_client.tls_set.assert_not_called()

    def test_set_tls_raises_if_cert_missing(
        self, mock_logger: Mock, mock_path: Mock
    ) -> None:
        """Test that missing certificate file raises ssl.SSLError."""
        proc: MQTTProcess = MQTTProcess("test", self.control, self.parameter)
        mock_path.return_value.is_file.return_value = False
        with self.assertRaises(ssl.SSLError):
            proc._set_tls(self.dummy_client)  # pylint: disable=protected-access
        mock_logger.error.assert_called_once()


class TestMQTTProcessWriteToOutput(unittest.TestCase):
    """Test the _write_to_output method of MQTTProcess."""

    def setUp(self) -> None:
        """Set up dummy client and control for output tests."""
        self.control: ComControl = ComControl()
        self.dummy_client = Mock()
        self.parameter: MQTTParameter = MQTTParameter(
            broker="test", port="111", subscribe=["test"]
        )

    def test_write_to_output_puts_correct_dict(self) -> None:
        """Test that received message is correctly put into output queue."""
        proc: MQTTProcess = MQTTProcess("test", self.control, self.parameter)
        msg = MagicMock()
        msg.topic = "a/b"
        msg.payload.decode.return_value = "payload"
        proc.control.output = MagicMock()
        proc._write_to_output(None, None, msg)  # pylint: disable=protected-access
        proc.control.output.put.assert_called_once_with(
            {"topic": "a/b", "data": "payload"}
        )


class TestMQTTProcessReadFromInput(unittest.TestCase):
    """Test the _read_from_input and __read_from_input methods of MQTTProcess."""

    def setUp(self) -> None:
        """Set up dummy client and control for input tests."""
        self.control: ComControl = ComControl()
        self.dummy_client = Mock()
        self.parameter: MQTTParameter = MQTTParameter(
            broker="test", port="111", subscribe=["test"]
        )

    @patch("cli.com.mqtt_com.logger")
    def test_read_from_input_json_error(self, mock_logger: Mock) -> None:
        """Test that a JSONDecodeError is handled and logged."""
        proc: MQTTProcess = MQTTProcess("test", self.control, self.parameter)
        proc._read_from_input_sub = MagicMock(  # pylint: disable=protected-access
            side_effect=json.decoder.JSONDecodeError(msg="Test", doc="Test", pos=0)
        )
        proc.shutdown = MagicMock()
        proc.name = "test"
        proc._read_from_input()  # pylint: disable=protected-access
        proc.shutdown.assert_called_once()
        mock_logger.error.assert_called_once()

    @patch("cli.com.mqtt_com.logger")
    def test_read_from_input_key_error(self, mock_logger: Mock) -> None:
        """Test that a KeyError is handled and logged."""
        proc: MQTTProcess = MQTTProcess("test", self.control, self.parameter)
        proc._read_from_input_sub = MagicMock(side_effect=KeyError())  # pylint: disable=protected-access
        proc.shutdown = MagicMock()
        proc.name = "test"
        proc._read_from_input()  # pylint: disable=protected-access
        proc.shutdown.assert_called_once()
        mock_logger.error.assert_called_once()

    @patch("cli.com.mqtt_com.isinstance")
    def test_private_read_from_input_publishes_messages(
        self, mock_isinstance: Mock
    ) -> None:
        """Test that messages from input queue are published to MQTT topics."""
        mock_isinstance.return_value = True
        self.control.shutdown.is_set = MagicMock(side_effect=[False, True])
        self.control.input.get = MagicMock(side_effect=[{"topic": "t", "data": "d"}])
        proc: MQTTProcess = MQTTProcess("test", self.control, self.parameter)
        # pylint: disable=protected-access
        proc._client = self.dummy_client
        proc._client.publish = MagicMock()
        proc._read_from_input()
        proc._client.publish.assert_called_once_with("t", "d")
        # pylint: enable=protected-access

    @patch("cli.com.mqtt_com.isinstance")
    def test_private_read_from_input_empty_messages(
        self, mock_isinstance: Mock
    ) -> None:
        """Test that messages from input queue with empty message."""
        mock_isinstance.return_value = True
        self.control.shutdown.is_set = MagicMock(side_effect=[False, True])
        self.control.input.get = MagicMock(side_effect=Empty)
        proc: MQTTProcess = MQTTProcess("test", self.control, self.parameter)
        # pylint: disable=protected-access
        proc._client = self.dummy_client
        proc._client.publish = MagicMock()  # pylint: disable=protected-access
        with patch("cli.com.mqtt_com.sleep") as s:
            proc._read_from_input()  # pylint: disable=protected-access
            s.assert_called_once()

    def test_private_read_from_input_raises_typeerror(self) -> None:
        """Test that __read_from_input raises TypeError if client is not initialized."""
        proc: MQTTProcess = MQTTProcess("test", self.control, self.parameter)
        # pylint: disable=protected-access
        proc._client = None
        with self.assertRaises(TypeError):
            proc._read_from_input_sub()  # pylint: disable=protected-access


if __name__ == "__main__":
    unittest.main()
