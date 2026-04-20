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

"""Testing file 'cli/cmd_com/com_impl.py'."""

import json
import sys
import unittest
from json.decoder import JSONDecodeError
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import MagicMock, Mock, patch

try:
    from cli.cmd_com.com_impl import can_main, modbus_main, mqtt_main
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_com.com_impl import can_main, modbus_main, mqtt_main


class TestCANMain(unittest.TestCase):  # pylint: disable=R0902
    """Unit tests for cli.cmd_com.com_impl.can_main"""

    def setUp(self):
        """Set up common patches and defaults."""
        # Patch all external dependencies used inside the module
        self.p_read_config = patch("cli.cmd_com.com_impl.read_config")
        self.p_can = patch("cli.cmd_com.com_impl.CAN")
        self.p_file = patch("cli.cmd_com.com_impl.File")
        self.p_recho = patch("cli.cmd_com.com_impl.recho")
        self.p_secho = patch("cli.cmd_com.com_impl.secho")
        self.p_logger = patch("cli.cmd_com.com_impl.logger")
        self.p_sys_exit = patch("cli.cmd_com.com_impl.sys.exit")
        self.p_loads = patch("cli.cmd_com.com_impl.loads")
        self.p_can_bus_config = patch("cli.cmd_com.com_impl.CanBusConfig")
        self.p_can_logger_parameter = patch("cli.cmd_com.com_impl.CANLoggerParameter")
        self.p_file_parameter = patch("cli.cmd_com.com_impl.FileParameter")

        self.mock_read_config = self.p_read_config.start()
        self.mock_can = self.p_can.start()
        self.mock_file = self.p_file.start()
        self.mock_recho = self.p_recho.start()
        self.mock_secho = self.p_secho.start()
        self.mock_logger = self.p_logger.start()
        self.mock_sys_exit = self.p_sys_exit.start()
        self.mock_loads = self.p_loads.start()
        self.mock_can_bus_config = self.p_can_bus_config.start()
        self.mock_can_logger_parameter = self.p_can_logger_parameter.start()
        self.mock_file_parameter = self.p_file_parameter.start()

        # Reasonable default config stub
        self.mock_read_config.return_value = {
            "connection": {"iface": "can0"},
            "logger": {"level": "INFO"},
        }

        # Returned instances for CAN and File
        self.can_inst = MagicMock(name="CANInstance")
        self.file_inst = MagicMock(name="FileInstance")
        self.mock_can.return_value = self.can_inst
        self.mock_file.return_value = self.file_inst

        # Default: loads returns a simple dict
        self.mock_loads.return_value = {"foo": "bar"}

        # Silence logger in tests
        self.mock_logger.debug.return_value = None
        self.mock_logger.info.return_value = None

    def tearDown(self):
        """Stop all patches."""
        for p in [
            self.p_read_config,
            self.p_can,
            self.p_file,
            self.p_recho,
            self.p_secho,
            self.p_logger,
            self.p_sys_exit,
            self.p_loads,
            self.p_can_bus_config,
            self.p_can_logger_parameter,
            self.p_file_parameter,
        ]:
            p.stop()

    def test_happy_path_with_input_and_output(self):
        """Runs the main loop once with input and output and exits cleanly via RuntimeError."""
        with TemporaryDirectory() as tmp:
            config_path = Path(tmp) / "config.yaml"
            input_path = Path(tmp) / "in.jsonl"
            output_path = Path(tmp) / "logs" / "out.log"

            # Mock behavior: one loop iteration then break via RuntimeError
            self.file_inst.read.side_effect = ['{"foo": "bar"}', RuntimeError("stop")]
            self.can_inst.read.side_effect = [{"id": 1, "data": [1, 2, 3]}]

            # Execute
            can_main(config=config_path, input_file=input_path, output_file=output_path)

            # Startup invoked
            self.can_inst.start.assert_called_once()
            self.file_inst.start.assert_called_once()

            # Informational message when output_file is set
            self.mock_secho.assert_any_call("Logging is stopped with Ctrl+C.")

            # Input handled: parsed and written to CAN
            self.mock_loads.assert_called_once_with('{"foo": "bar"}')
            self.can_inst.write.assert_called_once_with({"foo": "bar"})

            # Output handled: read from CAN and written to file
            self.can_inst.read.assert_called_once_with(block=True, timeout=1)
            self.file_inst.write_can_message.assert_called_once_with(
                {"id": 1, "data": [1, 2, 3]}
            )

            # Shutdown sequence
            self.mock_secho.assert_any_call("End of communication")
            self.can_inst.shutdown.assert_called_once_with(block=True, timeout=1)
            self.file_inst.shutdown.assert_called_once_with(block=True, timeout=1)

            # sys.exit should not be called on happy path
            self.mock_sys_exit.assert_not_called()

    def test_child_process_error_triggers_exit(self):
        """If starting a process fails, error is echoed and program exits with code 1."""
        with TemporaryDirectory() as tmp:
            config_path = Path(tmp) / "config.yaml"

            # Raise on CAN start (first start call)
            self.can_inst.start.side_effect = ChildProcessError("boom")

            # Make sys.exit raise SystemExit to stop control flow in test
            self.mock_sys_exit.side_effect = SystemExit(1)

            with self.assertRaises(SystemExit):
                can_main(config=config_path, input_file=None, output_file=None)

            self.mock_recho.assert_called_once_with("boom")
            self.mock_sys_exit.assert_called_once_with(1)

            # File start should not be called after CAN start fails
            self.file_inst.start.assert_not_called()

            # No shutdown after immediate exit
            self.can_inst.shutdown.assert_not_called()
            self.file_inst.shutdown.assert_not_called()

    def test_json_decode_error_is_reported(self):
        """Invalid JSON lines are reported and the loop continues until RuntimeError."""
        with TemporaryDirectory() as tmp:
            config_path = Path(tmp) / "config.yaml"
            input_path = Path(tmp) / "in.jsonl"

            # read returns invalid line, then exit loop
            self.file_inst.read.side_effect = ["invalid"]
            self.mock_loads.side_effect = JSONDecodeError("msg", "doc", 0)

            can_main(config=config_path, input_file=input_path, output_file=None)

            # Error echoed for invalid JSON
            self.mock_recho.assert_any_call(
                "Some lines in the input file are not in the json format."
            )

            # Write should not be called due to JSON error
            self.can_inst.write.assert_not_called()

            # Clean shutdown reached
            self.mock_secho.assert_any_call("End of communication")
            self.can_inst.shutdown.assert_called_once_with(block=True, timeout=1)
            self.file_inst.shutdown.assert_called_once_with(block=True, timeout=1)

    def test_no_output_file_and_empty_input(self):
        """Tests the case with no available input and output file."""
        with TemporaryDirectory() as tmp:
            config_path = Path(tmp) / "config.yaml"
            input_path = Path(tmp) / "input.jsonl"
            self.file_inst.read.side_effect = [None, KeyboardInterrupt]
            can_main(config=config_path, input_file=input_path, output_file=None)
            # Startup invoked
            self.can_inst.read.assert_not_called()
            self.mock_secho.assert_any_call("End of communication")
            self.can_inst.shutdown.assert_called_once_with(block=True, timeout=1)
            self.file_inst.shutdown.assert_called_once_with(block=True, timeout=1)

    def test_empty_msg(self):
        """Tests receiving an empty message."""
        with TemporaryDirectory() as tmp:
            config_path = Path(tmp) / "config.yaml"
            output_path = Path(tmp) / "out.jsonl"
            self.can_inst.read.side_effect = [None, KeyboardInterrupt]
            can_main(config=config_path, input_file=None, output_file=output_path)
            # Startup invoked
            self.file_inst.write_can_message.assert_not_called()


@patch("cli.cmd_com.com_impl.secho")
@patch("cli.cmd_com.com_impl.File")
@patch("cli.cmd_com.com_impl.Modbus")
@patch("cli.cmd_com.com_impl.read_config")
class TestModbusMain(unittest.TestCase):
    """Unit tests for the 'cli.cmd_com.com_impl.modbus_main' function."""

    def test_happy_path(
        self,
        mock_read_config: Mock,
        mock_modbus_cls: Mock,
        mock_file_cls: Mock,
        _: Mock,
    ) -> None:
        """main() should:

        - Read config
        - Start Modbus and File handlers

        - Forward a single command and write the result
        - Shut down cleanly

        """
        mock_read_config.return_value = {
            "host": "localhost",
            "port": 502,
            "timeout": 1,
            "retries": 3,
            "ignore": False,
        }
        mock_modbus = MagicMock()
        mock_modbus_cls.return_value = mock_modbus
        result_dict = {
            "date": "x",
            "code": "read_coils",
            "address": 10,
            "length": 4,
            "values": [0, 1, 0, 1],
        }
        mock_modbus.read.return_value = result_dict

        mock_file = MagicMock()
        mock_file.read.side_effect = [
            json.dumps(
                {
                    "date": "x",
                    "code": "read_coils",
                    "address": 10,
                    "length": 4,
                    "values": None,
                }
            ),
            None,
        ]
        mock_file_cls.return_value = mock_file

        modbus_main(
            config=Path("cfg.yaml"),
            input_file=Path("in.jsonl"),
            output_file=Path("out.jsonl"),
        )

        mock_modbus.start.assert_called_once()
        mock_file.start.assert_called_once()
        mock_modbus.write.assert_called_once()
        mock_modbus.read.assert_called_once()
        mock_file.write.assert_called_once_with(str(result_dict))
        mock_modbus.shutdown.assert_called_once()
        mock_file.shutdown.assert_called_once()

    def test_json_decode_error(
        self, mock_read_config: Mock, mock_modbus_cls: Mock, mock_file_cls, _: Mock
    ) -> None:
        """Malformed JSON in the input should cause a user-facing message and a clean shutdown."""
        mock_read_config.return_value = {
            "host": "localhost",
            "port": 502,
            "timeout": 1,
            "retries": 3,
            "ignore": False,
        }
        mock_modbus_cls.return_value = MagicMock()

        mock_file = MagicMock()
        mock_file.read.side_effect = ["{malformed-json", None]
        mock_file_cls.return_value = mock_file

        with patch("cli.cmd_com.com_impl.recho") as mock_recho:
            modbus_main(
                config=Path("cfg.yaml"),
                input_file=Path("in.jsonl"),
                output_file=Path("out.jsonl"),
            )
            mock_recho.assert_called()
            mock_file.shutdown.assert_called_once()
            mock_modbus_cls.return_value.shutdown.assert_called_once()

    def test_empty_queue(
        self, mock_read_config: Mock, mock_modbus_cls: Mock, mock_file_cls, _: Mock
    ) -> None:
        """When the output queue is empty and read() raises Empty, main() should report and stop."""
        mock_read_config.return_value = {
            "host": "localhost",
            "port": 502,
            "timeout": 1,
            "retries": 3,
            "ignore": False,
        }
        mock_modbus = MagicMock()
        mock_modbus.read.side_effect = __import__("queue").Empty()
        mock_modbus_cls.return_value = mock_modbus

        mock_file = MagicMock()
        mock_file.read.side_effect = [
            json.dumps(
                {
                    "date": "x",
                    "code": "read_coils",
                    "address": 1,
                    "length": 1,
                    "values": None,
                }
            ),
        ]
        mock_file_cls.return_value = mock_file

        with patch("cli.cmd_com.com_impl.recho") as mock_recho:
            modbus_main(
                config=Path("cfg.yaml"),
                input_file=Path("in.jsonl"),
                output_file=Path("out.jsonl"),
            )
            mock_recho.assert_called()
            mock_file.shutdown.assert_called_once()
            mock_modbus.shutdown.assert_called_once()

    def test_child_process_error(
        self,
        mock_read_config: Mock,
        mock_modbus_cls: Mock,
        mock_file_cls: Mock,
        _: Mock,
    ) -> None:
        """A ChildProcessError during start should be reported and cause exit code 1."""
        mock_read_config.return_value = {"host": "localhost"}
        mock_modbus = MagicMock()
        mock_file = MagicMock()
        mock_modbus.start.side_effect = ChildProcessError("start failed")
        mock_modbus_cls.return_value = mock_modbus
        mock_file_cls.return_value = mock_file

        with (
            patch("cli.cmd_com.com_impl.recho") as mock_recho,
            self.assertRaises(SystemExit) as cm,
        ):
            modbus_main(
                config=Path("cfg.yaml"),
                input_file=Path("in.jsonl"),
                output_file=Path("out.jsonl"),
            )
            mock_recho.assert_called_with("start failed")
            self.assertEqual(cm.exception.code, 1)

    def test_keyboard_interrupt(
        self, mock_read_config: Mock, mock_modbus_cls: Mock, mock_file_cls, _: Mock
    ) -> None:
        """KeyboardInterrupt should break the loop and trigger clean shutdown."""
        mock_read_config.return_value = {"host": "localhost"}
        mock_modbus = MagicMock()
        mock_modbus_cls.return_value = mock_modbus

        mock_file = MagicMock()
        mock_file.read.side_effect = KeyboardInterrupt()
        mock_file_cls.return_value = mock_file

        modbus_main(
            config=Path("cfg.yaml"),
            input_file=Path("in.jsonl"),
            output_file=Path("out.jsonl"),
        )

        mock_file.shutdown.assert_called_once()
        mock_modbus.shutdown.assert_called_once()


@patch("cli.cmd_com.com_impl.read_config")
@patch("cli.cmd_com.com_impl.MQTT")
@patch("cli.cmd_com.com_impl.File")
class TestMQTTMain(unittest.TestCase):
    """Unit tests for the 'cli.cmd_com.com_impl.mqtt_main' function."""

    def setUp(self) -> None:
        """TODO"""
        self.config_path = Path("config.json")
        self.input_path = Path("input.txt")
        self.output_path = Path("output.txt")
        self.mock_client = MagicMock()
        self.mock_file_handler = MagicMock()

    def test_main_normal_flow(
        self, mock_file: Mock, mock_mqtt: Mock, mock_read_config: Mock
    ) -> None:
        """Test the main flow with input and output files present, and normal message processing."""
        mock_read_config.return_value = {
            "broker": "test",
            "port": 0,
            "subscribe": "test",
        }
        # Mock read() to return a valid JSON string once, then None to exit the loop.
        self.mock_file_handler.read.side_effect = [
            '{"topic": "test", "data": "payload"}',
            None,
        ]
        self.mock_client.read.side_effect = [
            {"topic": "test", "data": "payload"},
            KeyboardInterrupt,
        ]

        mock_mqtt.return_value = self.mock_client
        mock_file.return_value = self.mock_file_handler

        with patch("cli.cmd_com.com_impl.secho"):
            mqtt_main(self.config_path, self.input_path, self.output_path)

        self.mock_client.start.assert_called_once()
        self.mock_file_handler.start.assert_called_once()
        self.mock_client.write.assert_called_with({"topic": "test", "data": "payload"})
        self.mock_file_handler.write.assert_called_with(
            str({"topic": "test", "data": "payload"})
        )
        self.mock_client.shutdown.assert_called_once_with(block=True, timeout=1)
        self.mock_file_handler.shutdown.assert_called_once_with(block=True, timeout=1)

    def test_main_normal_flow_without_output_file(
        self, mock_file: Mock, mock_mqtt: Mock, mock_read_config: Mock
    ) -> None:
        """Test the main flow with input but no output file present."""
        mock_read_config.return_value = {
            "broker": "test",
            "port": 0,
            "subscribe": "test",
        }
        # Mock read() to return a valid JSON string once, then None to exit the loop.
        self.mock_file_handler.read.side_effect = [
            '{"topic": "test", "data": "payload"}',
            KeyboardInterrupt,
        ]

        mock_mqtt.return_value = self.mock_client
        mock_file.return_value = self.mock_file_handler

        with patch("cli.cmd_com.com_impl.secho"):
            mqtt_main(self.config_path, self.input_path, None)

        self.mock_client.start.assert_called_once()
        self.mock_file_handler.start.assert_called_once()
        self.mock_client.write.assert_called_with({"topic": "test", "data": "payload"})
        self.mock_client.read.assert_not_called()
        self.mock_client.shutdown.assert_called_once_with(block=True, timeout=1)
        self.mock_file_handler.shutdown.assert_called_once_with(block=True, timeout=1)

    def test_main_normal_flow_without_input_file(
        self, mock_file: Mock, mock_mqtt: Mock, mock_read_config: Mock
    ) -> None:
        """Test the main flow with output but no input file present."""
        mock_read_config.return_value = {
            "broker": "test",
            "port": 0,
            "subscribe": "test",
        }
        # Mock read() to return a valid JSON string once, then None to exit the loop.
        self.mock_client.read.side_effect = [
            None,
            None,
            {"topic": "test", "data": "payload"},
            KeyboardInterrupt,
        ]

        mock_mqtt.return_value = self.mock_client
        mock_file.return_value = self.mock_file_handler

        with patch("cli.cmd_com.com_impl.secho"):
            mqtt_main(self.config_path, None, self.output_path)

        self.mock_client.start.assert_called_once()
        self.mock_file_handler.start.assert_called_once()
        self.mock_client.write.assert_not_called()
        self.mock_client.shutdown.assert_called_once_with(block=True, timeout=1)
        self.mock_file_handler.shutdown.assert_called_once_with(block=True, timeout=1)

    def test_main_childprocesserror(
        self, mock_file: Mock, mock_mqtt: Mock, mock_read_config: Mock
    ) -> None:
        """Test that ChildProcessError during start leads to sys.exit."""
        mock_mqtt.return_value = self.mock_client
        mock_file.return_value = self.mock_file_handler

        self.mock_client.start.side_effect = ChildProcessError("Process error")
        mock_read_config.return_value = {
            "broker": "test",
            "port": 0,
            "subscribe": "test",
        }

        with self.assertRaises(SystemExit), patch("cli.cmd_com.com_impl.recho"):
            with patch("cli.cmd_com.com_impl.secho"):
                mqtt_main(self.config_path, self.input_path, self.output_path)

    @patch("cli.cmd_com.com_impl.recho")
    def test_main_jsondecodeerror(
        self, mock_recho: Mock, mock_file: Mock, mock_mqtt: Mock, mock_read_config: Mock
    ) -> None:
        """Test that JSONDecodeError in user_input triggers error output."""
        mock_file.return_value = self.mock_file_handler
        mock_mqtt.return_value = self.mock_client

        self.mock_file_handler.read.side_effect = ["invalid json", None]
        self.mock_client.read.side_effect = [None, None]
        mock_read_config.return_value = {
            "broker": "test",
            "port": 0,
            "subscribe": "test",
        }

        with (
            patch(
                "cli.cmd_com.com_impl.loads",
                side_effect=JSONDecodeError("msg", "doc", 0),
            ),
            patch("cli.cmd_com.com_impl.secho"),
        ):
            mqtt_main(self.config_path, self.input_path, self.output_path)
        mock_recho.assert_called_with(
            "Some lines in the input file are not in the json format."
        )


if __name__ == "__main__":
    unittest.main()
