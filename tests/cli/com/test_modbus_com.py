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

"""Testing file 'cli/com/modbus_com.py'."""

import sys
import unittest
from json import JSONDecodeError
from pathlib import Path
from queue import Empty
from unittest.mock import MagicMock, Mock, patch

from pymodbus.client import ModbusTcpClient
from pymodbus.exceptions import ModbusException
from pymodbus.pdu import ExceptionResponse

try:
    from cli.com.modbus_com import Modbus, ModbusProcess
    from cli.com.parameter import ComControl, ModbusParameter
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.com.modbus_com import Modbus, ModbusProcess
    from cli.com.parameter import ComControl, ModbusParameter


class TestModbusInit(unittest.TestCase):
    """Tests Modbus.__init__."""

    def test_init_registers_process(self) -> None:
        """Creating Modbus should register a ModbusProcess in its internal process map."""
        m = Modbus(name="test", parameter=ModbusParameter(host="localhost"))
        self.assertIn("ModbusProcess", m._processes)  # type: ignore[attr-defined] pylint: disable=protected-access


class TestModbusRead(unittest.TestCase):
    """Tests Modbus.read()."""

    def setUp(self) -> None:
        self.modbus = Modbus(name="test", parameter=ModbusParameter(host="localhost"))

    def test_returns_item_when_available(self) -> None:
        """When an item is in the output queue, read() should return it."""
        expected = {"ok": True}
        self.modbus.is_alive = MagicMock(return_value=True)
        self.modbus.control.output.empty = Mock(return_value=False)
        self.modbus.control.output.get = MagicMock(return_value=expected)
        result = self.modbus.read()
        self.assertEqual(result, expected)

    def test_returns_none_when_not_alive_and_empty(self) -> None:
        """When the process is not alive and the output queue is empty, read() returns None."""
        self.modbus.is_alive = MagicMock(return_value=False)
        self.assertIsNone(self.modbus.read(block=True, timeout=1))
        self.modbus.is_alive = MagicMock(return_value=False)
        self.modbus.control.output.empty = Mock(return_value=True)
        result = self.modbus.read()
        self.assertEqual(result, None)


class TestModbusWrite(unittest.TestCase):
    """Tests Modbus.write()."""

    def setUp(self) -> None:
        self.modbus = Modbus(name="test", parameter=ModbusParameter(host="localhost"))

    def test_puts_command_when_alive(self) -> None:
        """If the process is alive, the command is enqueued to the input queue."""
        self.modbus.is_alive = MagicMock(return_value=True)
        cmd = {"code": "read_coils", "address": 10, "length": 1, "values": None}

        self.modbus.write(cmd)
        self.modbus.control.output.empty = Mock(return_value=False)
        got = self.modbus.control.input.get(block=True, timeout=1)
        self.assertEqual(got, cmd)

    def test_raises_when_not_alive(self) -> None:
        """If the process is not alive, a RuntimeError is raised."""
        self.modbus.is_alive = MagicMock(return_value=False)
        with self.assertRaises(RuntimeError):
            self.modbus.write({"code": "x"})


class TestModbusProcessInit(unittest.TestCase):
    """Tests ModbusProcess.__init__."""

    def test_init_assigns_fields(self) -> None:
        """ModbusProcess should set name, control, parameter, and client placeholder."""
        control = ComControl()
        parameter = ModbusParameter(host="localhost")
        proc = ModbusProcess(name="proc", control=control, parameter=parameter)
        self.assertEqual(proc.name, "proc")
        self.assertIs(proc.control, control)
        self.assertIs(proc.parameter, parameter)
        self.assertIsNone(proc._client)  # pylint: disable=protected-access


class TestModbusProcessDel(unittest.TestCase):
    """Tests ModbusProcess.__del__()."""

    def test_del_closes_client(self) -> None:
        """__del__ should close the client if present."""
        control = ComControl()
        parameter = ModbusParameter(host="localhost")
        proc = ModbusProcess(name="proc", control=control, parameter=parameter)
        proc._client = MagicMock(spec_set=ModbusTcpClient)  # pylint: disable=protected-access
        with patch("cli.com.modbus_com.logger"):
            proc.__del__()  # pylint: disable=unnecessary-dunder-call
            proc._client.close.assert_called_once()  # pylint: disable=protected-access


@patch("cli.com.modbus_com.add_queue_handler")
@patch("cli.com.modbus_com.signal.signal")
class TestModbusProcessRun(unittest.TestCase):
    """Tests ModbusProcess.run()."""

    def test_run_connects_and_signals_ready(
        self, _: Mock, mock_add_queue_handler: Mock
    ) -> None:
        """run() should connect, set ready event, and start executing commands."""
        control = ComControl()
        parameter = ModbusParameter(host="localhost")
        proc = ModbusProcess(name="proc", control=control, parameter=parameter)

        # Stub connect and execute behavior, pretend connected client
        proc._connect = MagicMock(  # pylint: disable=protected-access
            return_value=MagicMock(spec_set=ModbusTcpClient)
        )
        proc._execute_command = MagicMock()  # pylint: disable=protected-access

        proc.run()

        mock_add_queue_handler.assert_called_once_with(control.logger)
        self.assertTrue(control.ready.is_set())
        proc._execute_command.assert_called_once()  # pylint: disable=protected-access

    def test_run_connects_no_client(
        self, _: Mock, mock_add_queue_handler: Mock
    ) -> None:
        """run() should connect, set ready event, and start executing commands."""
        control = ComControl()
        parameter = ModbusParameter(host="localhost")
        proc = ModbusProcess(name="proc", control=control, parameter=parameter)

        # Stub connect and execute behavior, pretend connected client
        proc._connect = MagicMock(  # pylint: disable=protected-access
            return_value=None
        )
        proc._execute_command = MagicMock()  # pylint: disable=protected-access

        proc.run()

        mock_add_queue_handler.assert_called_once_with(control.logger)
        self.assertFalse(control.ready.is_set())
        proc._execute_command.assert_not_called()  # pylint: disable=protected-access


class TestModbusProcessConnect(unittest.TestCase):
    """Tests ModbusProcess._connect()."""

    def test_connect_success(self) -> None:
        """When the client connects successfully, the client instance is returned."""
        control = ComControl()
        parameter = ModbusParameter(host="localhost", port=1502, timeout=2, retries=5)
        proc = ModbusProcess(name="proc", control=control, parameter=parameter)

        mock_client = MagicMock()
        mock_client.connect.return_value = True
        # patch within the method, otherwise __del__ method will make problems
        with patch(
            "cli.com.modbus_com.ModbusTcpClient", return_value=mock_client
        ) as mock_client_cls:
            client = proc._connect()  # pylint: disable=protected-access
        self.assertIs(client, mock_client)
        mock_client_cls.assert_called_once_with(
            host="localhost", port=1502, timeout=2, retries=5
        )
        mock_client.connect.assert_called_once()

    def test_connect_failure(self) -> None:
        """On failed connection, None is returned and shutdown() is requested."""
        control = ComControl()
        parameter = ModbusParameter(host="localhost")
        proc = ModbusProcess(name="proc", control=control, parameter=parameter)
        proc.shutdown = MagicMock()
        mock_client = MagicMock()
        mock_client.connect.return_value = False
        # patch within the method, otherwise __del__ method will make problems
        with patch("cli.com.modbus_com.ModbusTcpClient", return_value=mock_client):
            client = proc._connect()  # pylint: disable=protected-access
        self.assertIsNone(client)
        proc.shutdown.assert_called_once()


@patch("cli.com.modbus_com.logger")
class TestModbusProcessExecuteCommand(unittest.TestCase):
    """Tests ModbusProcess._execute_command()."""

    def test_execute_command_catches_modbus_exceptions_and_shutdowns(
        self, mock_logger: Mock
    ) -> None:
        """_execute_command should catch Modbus-related exceptions and call shutdown()."""
        control = ComControl()
        parameter = ModbusParameter(host="localhost")
        proc = ModbusProcess(name="proc", control=control, parameter=parameter)
        proc._execute_command_sub = MagicMock(  # pylint: disable=protected-access
            side_effect=__import__("pymodbus").exceptions.ModbusException("boom")
        )
        proc.shutdown = MagicMock()
        proc._execute_command()  # pylint: disable=protected-access
        proc.shutdown.assert_called_once()
        mock_logger.assert_not_called()

    def test_execute_command_catches_json_key_errors(self, mock_logger: Mock) -> None:
        """_execute_command should catch KeyError and call shutdown()."""
        control = ComControl()
        parameter = ModbusParameter(host="localhost")
        proc = ModbusProcess(name="proc", control=control, parameter=parameter)
        proc._execute_command_sub = MagicMock(side_effect=KeyError("missing"))  # pylint: disable=protected-access
        proc.shutdown = MagicMock()
        proc._execute_command()  # pylint: disable=protected-access
        proc.shutdown.assert_called_once()
        self.assertIn(
            "One command from the input read", list(mock_logger.error.call_args[0])[0]
        )

    def test_execute_command_catches_json_errors(self, mock_logger: Mock) -> None:
        """_execute_command should catch JSONDecodeError and call shutdown()."""
        control = ComControl()
        parameter = ModbusParameter(host="localhost")
        proc = ModbusProcess(name="proc", control=control, parameter=parameter)
        proc._execute_command_sub = MagicMock(  # pylint: disable=protected-access
            side_effect=JSONDecodeError(msg="wrong command", doc="", pos=0)
        )
        proc.shutdown = MagicMock()
        proc._execute_command()  # pylint: disable=protected-access
        proc.shutdown.assert_called_once()
        self.assertIn(
            "an not be converted to a dictionary",
            list(mock_logger.error.call_args[0])[0],
        )

    def test_execute_command_catches_modbus_errors(self, mock_logger: Mock) -> None:
        """_execute_command should catch ModbusException and call shutdown()."""
        control = ComControl()
        parameter = ModbusParameter(host="localhost")
        proc = ModbusProcess(name="proc", control=control, parameter=parameter)
        proc._execute_command_sub = MagicMock(  # pylint: disable=protected-access
            side_effect=ModbusException("modbus error")
        )
        proc.shutdown = MagicMock()
        proc._execute_command()  # pylint: disable=protected-access
        proc.shutdown.assert_called_once()
        self.assertIn("Could not send request", list(mock_logger.error.call_args[0])[0])


class DummyReadResult:  # pylint: disable=too-few-public-methods
    """Simple result holder emulating pymodbus read result objects."""

    def __init__(self, bits: list | None = None, registers: list | None = None) -> None:
        self.bits = bits or []
        self.registers = registers or []


class TestModbusProcessExecuteCommandSub(unittest.TestCase):
    """Tests ModbusProcess._execute_command_sub()."""

    def setUp(self) -> None:
        self.control = ComControl()
        self.parameter = ModbusParameter(host="localhost", ignore=True)
        self.proc = ModbusProcess(
            name="proc", control=self.control, parameter=self.parameter
        )

        client = MagicMock(spec_set=ModbusTcpClient)
        client.read_coils.return_value = DummyReadResult(bits=[1, 0, 1, 0])
        client.read_discrete_inputs.return_value = DummyReadResult(bits=[0, 1])
        client.read_input_registers.return_value = DummyReadResult(registers=[10, 20])
        client.read_holding_registers.return_value = DummyReadResult(
            registers=[100, 200]
        )
        client.write_coils.return_value = DummyReadResult(bits=[1, 1])
        client.write_registers.return_value = DummyReadResult(registers=[42])
        self.proc._client = client  # pylint: disable=protected-access

    def test_no_valid_client(self) -> None:
        """Verifys the interrupt if no valid client is passed."""
        parameter = ModbusParameter(host="localhost", ignore=False)
        proc = ModbusProcess(name="proc", control=self.control, parameter=parameter)
        proc._client = MagicMock()  # pylint: disable=protected-access
        with patch("cli.com.modbus_com.logger") as mock_logger:
            proc._execute_command_sub()  # pylint: disable=protected-access
        mock_logger.error.assert_called_once_with("Client has not yet started.")

    def test_read_coils_flow(self) -> None:
        """Verify the read_coils command returning expected result."""
        cmd = {
            "date": "x",
            "code": "read_coils",
            "address": 0,
            "length": 2,
            "values": None,
        }
        # copy is needed otherwise assert will always be true
        expected_result = cmd.copy()
        expected_result["values"] = [1, 0]
        with (
            patch.object(self.control.shutdown, "is_set", side_effect=[False, True]),
            patch.object(self.control.input, "get", return_value=cmd),
            patch.object(self.control.output, "put") as mock_put,
        ):
            self.proc._execute_command_sub()  # pylint: disable=protected-access

        mock_put.assert_called_once_with(expected_result)

    def test_write_coils_flow(self) -> None:
        """Verify the write_coils command updates 'values' from the result."""
        cmd = {
            "date": "x",
            "code": "write_coils",
            "address": 0,
            "length": 2,
            "values": [1, 1],
        }
        # copy is needed otherwise assert will always be true
        expected_result = cmd.copy()
        with (
            patch.object(self.control.shutdown, "is_set", side_effect=[False, True]),
            patch.object(self.control.input, "get", return_value=cmd),
            patch.object(self.control.output, "put") as mock_put,
        ):
            self.proc._execute_command_sub()  # pylint: disable=protected-access
        mock_put.assert_called_once_with(expected_result)

    def test_read_discrete_input_flow(self) -> None:
        """Verify the read_discrete_input command returning expected result."""
        cmd = {
            "date": "x",
            "code": "read_discrete_input",
            "address": 0,
            "length": 2,
            "values": None,
        }
        # copy is needed otherwise assert will always be true
        expected_result = cmd.copy()
        expected_result["values"] = [0, 1]
        with (
            patch.object(self.control.shutdown, "is_set", side_effect=[False, True]),
            patch.object(self.control.input, "get", return_value=cmd),
            patch.object(self.control.output, "put") as mock_put,
        ):
            self.proc._execute_command_sub()  # pylint: disable=protected-access
        mock_put.assert_called_once_with(expected_result)

    def test_read_input_registers_flow(self) -> None:
        """Verify the read_input_register command returning expected result."""
        cmd = {
            "date": "x",
            "code": "read_input_register",
            "address": 0,
            "length": 2,
            "values": None,
        }
        # copy is needed otherwise assert will always be true
        expected_result = cmd.copy()
        expected_result["values"] = [10, 20]
        with (
            patch.object(self.control.shutdown, "is_set", side_effect=[False, True]),
            patch.object(self.control.input, "get", return_value=cmd),
            patch.object(self.control.output, "put") as mock_put,
        ):
            self.proc._execute_command_sub()  # pylint: disable=protected-access
        mock_put.assert_called_once_with(expected_result)

    def test_read_holding_registers_flow(self) -> None:
        """Verify the read_holding_register command returning expected result."""
        cmd = {
            "date": "x",
            "code": "read_holding_register",
            "address": 0,
            "length": 2,
            "values": None,
        }
        # copy is needed otherwise assert will always be true
        expected_result = cmd.copy()
        expected_result["values"] = [100, 200]
        with (
            patch.object(self.control.shutdown, "is_set", side_effect=[False, True]),
            patch.object(self.control.input, "get", return_value=cmd),
            patch.object(self.control.output, "put") as mock_put,
        ):
            self.proc._execute_command_sub()  # pylint: disable=protected-access
        mock_put.assert_called_once_with(expected_result)

    def test_write_holding_registers_flow(self) -> None:
        """Verify the read_holding_register command updates 'values' and puts to output."""
        cmd = {
            "date": "x",
            "code": "write_holding_register",
            "address": 0,
            "length": 2,
            "values": [42],
        }
        # copy is needed otherwise assert will always be true
        expected_result = cmd.copy()
        with (
            patch.object(self.control.shutdown, "is_set", side_effect=[False, True]),
            patch.object(self.control.input, "get", return_value=cmd),
            patch.object(self.control.output, "put") as mock_put,
        ):
            self.proc._execute_command_sub()  # pylint: disable=protected-access
        mock_put.assert_called_once_with(expected_result)

    def test_unknown_code_ignored(self) -> None:
        """With ignore=True, unknown codes should be logged debug and not break the loop."""
        cmd = {
            "date": "x",
            "code": "unknown",
            "address": 0,
            "length": 1,
            "values": None,
        }
        with (
            patch.object(self.control.shutdown, "is_set", side_effect=[False, True]),
            patch.object(self.control.input, "get", return_value=cmd),
            patch.object(self.control.output, "put") as mock_put,
            patch("cli.com.modbus_com.logger") as mock_logger,
        ):
            # Should not raise, but also not put anything into output
            self.proc._execute_command_sub()  # pylint: disable=protected-access
            mock_put.assert_called_once()
            mock_logger.debug.assert_called_once()

    def test_unknown_code_breaks_when_not_ignored(self) -> None:
        """With ignore=False, unknown codes should break the loop immediately."""
        parameter = ModbusParameter(host="localhost", ignore=False)
        proc = ModbusProcess(name="proc", control=self.control, parameter=parameter)
        proc._client = MagicMock(spec_set=ModbusTcpClient)  # pylint: disable=protected-access
        cmd = {
            "date": "x",
            "code": "unknown",
            "address": 0,
            "length": 1,
            "values": None,
        }
        with (
            patch.object(self.control.shutdown, "is_set", side_effect=[False, True]),
            patch.object(self.control.input, "get", return_value=cmd),
            patch.object(self.control.output, "put") as mock_put,
            patch("cli.com.modbus_com.logger") as mock_logger,
        ):
            proc._execute_command_sub()  # pylint: disable=protected-access
            mock_put.assert_not_called()
            mock_logger.error.assert_called_once()

    def test_bad_response_ignored(self) -> None:
        """With ignore=True, bad response should be ignored."""
        parameter = ModbusParameter(host="localhost", ignore=True)
        proc = ModbusProcess(name="proc", control=self.control, parameter=parameter)
        proc._client = MagicMock(spec_set=ModbusTcpClient)  # pylint: disable=protected-access
        proc._client.read_coils.return_value = ExceptionResponse(2)  # pylint: disable=protected-access
        cmd = {
            "date": "x",
            "code": "read_coils",
            "address": 0,
            "length": 2,
            "values": None,
        }
        with (
            patch.object(self.control.shutdown, "is_set", side_effect=[False, True]),
            patch.object(self.control.input, "get", return_value=cmd),
            patch.object(self.control.output, "put") as mock_put,
            patch("cli.com.modbus_com.logger") as mock_logger,
        ):
            proc._execute_command_sub()  # pylint: disable=protected-access
        mock_put.assert_called_once_with(cmd)
        mock_logger.debug.assert_called_once()

    def test_bad_response_not_ignored(self) -> None:
        """With ignore=False, bad response should not be ignored."""
        parameter = ModbusParameter(host="localhost", ignore=False)
        proc = ModbusProcess(name="proc", control=self.control, parameter=parameter)
        proc._client = MagicMock(spec_set=ModbusTcpClient)  # pylint: disable=protected-access
        proc._client.read_coils.return_value = ExceptionResponse(2)  # pylint: disable=protected-access
        cmd = {
            "date": "x",
            "code": "read_coils",
            "address": 0,
            "length": 2,
            "values": None,
        }
        with (
            patch.object(self.control.shutdown, "is_set", side_effect=[False, True]),
            patch.object(self.control.input, "get", return_value=cmd),
            patch.object(self.control.output, "put") as mock_put,
            patch("cli.com.modbus_com.logger") as mock_logger,
        ):
            proc._execute_command_sub()  # pylint: disable=protected-access
        mock_put.assert_not_called()
        mock_logger.error.assert_called_once()

    def test_empty_queue(self) -> None:
        """Test with empty queue exception"""
        # copy is needed otherwise assert will always be true
        with (
            patch.object(self.control.shutdown, "is_set", side_effect=[False, True]),
            patch.object(self.control.input, "get", side_effect=Empty),
            patch.object(self.control.output, "put") as mock_put,
        ):
            self.proc._execute_command_sub()  # pylint: disable=protected-access
            mock_put.assert_not_called()


if __name__ == "__main__":
    unittest.main()
