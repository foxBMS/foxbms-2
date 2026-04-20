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

"""Testing file 'cli/commands/c_com.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import Mock, patch

from click.testing import CliRunner

try:
    from cli.cli import main
    from cli.commands.c_com import can, client, device, modbus, mqtt
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cli import main
    from cli.commands.c_com import can, client, device, modbus, mqtt


class TestCANCommand(unittest.TestCase):
    """Unit tests for the CAN Click command."""

    @patch("cli.commands.c_com.can_main")
    def test_can_command_calls_main_and_exits(self, mock_main: Mock) -> None:
        """Test that the CAN command calls main with correct arguments and exits."""
        runner = CliRunner()
        config_path = Path("test_config.yaml")
        input_path = Path("test_input.txt")
        output_path = Path("test_output.txt")

        # Create the config file for the exists=True option
        with runner.isolated_filesystem():
            # pylint: disable=W1514
            config_path.write_text("testconfig")
            input_path.write_text("test\n")
            result = runner.invoke(
                can,
                [
                    "--config",
                    str(config_path),
                    "--input",
                    str(input_path),
                    "--output",
                    str(output_path),
                    "-v",
                ],
            )
            self.assertEqual(result.exit_code, 0)
            mock_main.assert_called_once_with(config_path, input_path, output_path)


class TestDeviceCommand(unittest.TestCase):
    """Tests the 'device' click command."""

    @patch("cli.commands.c_com.StartTcpServer")
    @patch("cli.commands.c_com.secho")
    def test_starts_server_with_args(self, _: Mock, mock_start_server: Mock) -> None:
        """Verify server starts with provided address and port and exits with code 0."""
        runner = CliRunner()
        result = runner.invoke(device, ["--address", "127.0.0.1", "--port", "1502"])
        self.assertEqual(result.exit_code, 0)
        _, kwargs = mock_start_server.call_args
        self.assertIn("address", kwargs)
        self.assertEqual(kwargs["address"], ("127.0.0.1", 1502))


class TestClientCommand(unittest.TestCase):
    """Tests the 'client' click command."""

    @patch("cli.commands.c_com.modbus_main")
    def test_invokes_main(self, mock_main: Mock) -> None:
        """Verify client calls modbus_impl.main with given paths."""
        runner = CliRunner()
        config_path = Path("test_config.yaml")
        input_path = Path("test_input.jsonl")
        output_path = Path("test_output.jsonl")
        with runner.isolated_filesystem():
            # pylint: disable=W1514
            config_path.write_text("test yaml\n")
            input_path.write_text("test\n")
            result = runner.invoke(
                client,
                [
                    "--config",
                    str(config_path),
                    "--input",
                    str(input_path),
                    "--output",
                    str(output_path),
                ],
            )
        self.assertEqual(result.exit_code, 0)
        mock_main.assert_called_once()


class TestModbusGroup(unittest.TestCase):
    """Tests the 'modbus' click group."""

    def test_group_help(self) -> None:
        """Verify the group provides help text without errors."""
        runner = CliRunner()
        result = runner.invoke(modbus, ["--help"])
        self.assertEqual(result.exit_code, 0)
        self.assertIn("Usage", result.output)


class TestMQTTCommand(unittest.TestCase):
    """Unit tests for the MQTT Click command."""

    @patch("cli.commands.c_com.mqtt_main")
    def test_mqtt_command_calls_main_and_exits(self, mock_main: Mock) -> None:
        """Test that the MQTT command calls main with correct arguments and exits."""
        runner = CliRunner()
        config_path = Path("test_config.yaml")
        input_path = Path("test_input.txt")
        output_path = Path("test_output.txt")

        # Create the config file for the exists=True option
        with runner.isolated_filesystem():
            # pylint: disable=W1514
            config_path.write_text("broker: test\nport: 1883\nsubscribe: [topic]\n")
            input_path.write_text("test\n")
            result = runner.invoke(
                mqtt,
                [
                    "--config",
                    str(config_path),
                    "--input",
                    str(input_path),
                    "--output",
                    str(output_path),
                    "-v",
                ],
            )
            self.assertEqual(result.exit_code, 0)
            mock_main.assert_called_once_with(config_path, input_path, output_path)


class TestFoxCliMainCommandCom(unittest.TestCase):
    """Test of the 'com-test' command."""

    def test_cli_com_test_help(self):
        """Test 'fox.py com-test --help' command."""
        runner = CliRunner()
        result = runner.invoke(main, ["com-test", "--help"])
        self.assertEqual(result.exit_code, 0)

    def test_cli_com_test_can(self):
        """Test 'fox.py com-test can --help' command."""
        runner = CliRunner()
        result = runner.invoke(main, ["com-test", "can", "--help"])
        self.assertEqual(result.exit_code, 0)

    def test_cli_com_test_mqtt(self):
        """Test 'fox.py com-test can --help' command."""
        runner = CliRunner()
        result = runner.invoke(main, ["com-test", "mqtt", "--help"])
        self.assertEqual(result.exit_code, 0)


if __name__ == "__main__":
    unittest.main()
