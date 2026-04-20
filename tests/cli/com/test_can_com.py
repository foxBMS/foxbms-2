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

# cspell:ignore arxml

"""Testing file 'cli/com/can_com.py'."""

import sys
import time
import unittest
from pathlib import Path
from threading import Thread
from unittest.mock import MagicMock, Mock, patch

from can import CanInitializationError, CanOperationError, Message
from cantools.database.errors import UnsupportedDatabaseFormatError

try:
    import cli.com.can_com as mod
    from cli.com.can_com import CANProcess
    from cli.com.parameter import ComControl
    from cli.helpers.fcan import CanBusConfig
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    import cli.com.can_com as mod
    from cli.com.can_com import CANProcess
    from cli.com.parameter import ComControl
    from cli.helpers.fcan import CanBusConfig


class TestCANInit(unittest.TestCase):
    """Tests for CAN.__init__ in isolation."""

    @patch("cli.com.can_com.CANProcess")
    def test_initializes_and_registers_process(self, mock_can_process: Mock) -> None:
        """Ensures CAN registers a CANProcess instance in its process registry."""
        mock_can_process.__name__ = "CANProcess"
        can = mod.CAN(name="TestCAN", parameter=CanBusConfig(interface="pcan"))

        mock_can_process.assert_called_once()
        # pylint: disable=W0212
        self.assertEqual(can.name, "TestCAN")
        self.assertIs(can._processes[mock_can_process.__name__], mock_can_process())


class TestCANRead(unittest.TestCase):
    """Tests for CAN.read in isolation."""

    def setUp(self) -> None:
        """Prepare a CAN instance with stubbed base and real ComControl."""
        self.p_proc = patch("cli.com.can_com.CANProcess")
        mock_can_process = self.p_proc.start()
        mock_can_process.__name__ = "CANProcess"

    def tearDown(self) -> None:
        """Stop patches."""
        self.p_proc.stop()

    def test_read_behaviors(self) -> None:
        """Returns None if empty and process is dead; returns message when present."""
        can = mod.CAN(name="Test", parameter=CanBusConfig(interface="pcan"))

        can.is_alive = MagicMock(return_value=True)
        self.assertIsNone(can.read(block=True, timeout=0.1))

        msg = "MSG"
        can.control.output.put(msg)
        can.is_alive = MagicMock(return_value=True)
        # patch empty() because the method does not reliable return the
        # correct status of the queue
        with patch.object(
            can.control.output, "empty", return_value=False
        ) as mock_empty:
            self.assertEqual(can.read(block=True, timeout=0.1), msg)
            can.is_alive = MagicMock(return_value=False)
            mock_empty.return_value = True
            with patch("cli.com.can_com.logger") as mock_debug:
                can.read(block=True, timeout=0.1)
                mock_debug.debug.assert_called_with(
                    "Input queue can not be read, because related process is not running."
                )


class TestCANWrite(unittest.TestCase):
    """Tests for CAN.write in isolation."""

    def setUp(self) -> None:
        """Prepare a CAN instance with stubbed base and real ComControl."""
        self.p_proc = patch("cli.com.can_com.CANProcess")
        mock_can_process = self.p_proc.start()
        mock_can_process.__name__ = "CANProcess"

    def tearDown(self) -> None:
        """Stop patches."""
        self.p_proc.stop()

    def test_write_raises_when_not_alive_and_puts_when_alive(self) -> None:
        """Raises RuntimeError if not alive; enqueues message when alive."""
        can = mod.CAN(name="Test", parameter=CanBusConfig(interface="pcan"))

        can.is_alive = MagicMock(return_value=False)
        with self.assertRaises(RuntimeError):
            can.write({"id": 1, "data": [1]})

        can.is_alive = MagicMock(return_value=True)
        can.write({"id": 2, "data": [2]})
        # time.sleep is used to give empty() the chance to update its status
        time.sleep(0.1)
        self.assertFalse(can.control.input.empty())
        self.assertEqual(can.control.input.get(), {"id": 2, "data": [2]})


@patch("cli.com.can_com.recho")
@patch("cli.com.can_com.ThreadSafeBus")
@patch("cli.com.can_com.asdict")
class TestCANProcessConnect(unittest.TestCase):
    """Tests for CANProcess._connect in isolation."""

    def setUp(self) -> None:
        """Prepare the Bus parameter"""
        self.param = CanBusConfig(interface="pcan")

    def test_connect_success(self, mock_asdict: Mock, mock_bus: Mock, _: Mock) -> None:
        """Returns a bus on success"""
        control = ComControl()
        mock_asdict.return_value = {"channel": "can0", "bustype": "socketcan"}
        p = CANProcess("P", control, self.param)
        # pylint: disable=W0212
        bus = p._connect()
        mock_bus.assert_called_once_with(channel="can0", bustype="socketcan")
        self.assertIs(bus, mock_bus())

    def test_connect_failure(
        self, mock_asdict: Mock, mock_bus: Mock, mock_recho: Mock
    ) -> None:
        """Echoes and shuts down on initialization error."""
        control = ComControl()
        mock_asdict.return_value = {"channel": "can0", "bustype": "socketcan"}
        mock_bus.side_effect = CanInitializationError("init failed")
        p = CANProcess("P", control, self.param)
        with patch.object(p, "shutdown") as mock_shutdown:
            # pylint: disable=W0212
            bus2 = p._connect()
            mock_recho.assert_called_with("Could not initialize CAN bus.")
            mock_shutdown.assert_called_once()
            self.assertIsNone(bus2)


@patch("cli.com.can_com.Thread")
@patch("cli.com.can_com.add_queue_handler")
class TestCANProcessRun(unittest.TestCase):
    """Tests for CANProcess.run in isolation."""

    def test_run_starts_write_thread_and_receive_loop(
        self, mock_add_q_handler: Mock, mock_thread: Mock
    ) -> None:
        """Sets up logging, connects, signals ready, spawns writer thread, and calls receiver."""
        control = ComControl()
        proc = CANProcess("Runner", control, CanBusConfig(interface="kvaser"))

        fake_bus = Mock()
        with patch.object(proc, "_connect", return_value=fake_bus):
            with patch.object(mod, "logger") as mock_logger:
                with patch.object(proc, "_write_can_messages") as mock_write:
                    t_instance = MagicMock()
                    mock_thread.return_value = t_instance

                    proc.run()

                    mock_add_q_handler.assert_called_once_with(control.logger)
                    mock_logger.setLevel.assert_called_once_with(control.log_level)
                    self.assertTrue(control.ready.is_set())
                    mock_thread.assert_called_once()
                    t_instance.start.assert_called_once()
                    mock_write.assert_called_once()

    def test_run_but_bus_is_none(
        self, mock_add_q_handler: Mock, mock_thread: Mock
    ) -> None:
        """Test the run method but bus is none."""
        control = ComControl()
        proc = CANProcess("Runner", control, CanBusConfig(interface="kvaser"))

        with patch.object(proc, "_connect", return_value=None):
            with patch.object(mod, "logger") as mock_logger:
                proc.run()
                mock_add_q_handler.assert_called_once_with(control.logger)
                mock_logger.setLevel.assert_called_once_with(control.log_level)
                mock_thread.assert_not_called()


class TestLoadDatabase(unittest.TestCase):
    """Tests for CANProcess.load_database in isolation."""

    def setUp(self) -> None:
        """Create a CANProcess with a dummy control and parameter namespace."""
        self.parameter = CanBusConfig(interface="pcan", dbc="path/to/test.dbc")
        self.proc = CANProcess(
            name="test", control=ComControl(), parameter=self.parameter
        )
        # Mock shutdown to observe calls without side effects
        self.proc.shutdown = Mock()

    @patch("cli.com.can_com.recho")
    @patch("cli.com.can_com.database.load_file")
    def test_loads_database_successfully(
        self, mock_load_file: Mock, mock_recho: Mock
    ) -> None:
        """Should return a database object when load_file succeeds."""
        fake_db = Mock()
        mock_load_file.return_value = fake_db
        # pylint: disable=W0212
        result = self.proc._load_database()
        self.assertIs(result, fake_db)
        self.proc.shutdown.assert_not_called()
        mock_recho.assert_not_called()
        mock_load_file.assert_called_once_with(self.parameter.dbc, encoding="utf-8")

    @patch("cli.com.can_com.recho")
    @patch("cli.com.can_com.database.load_file")
    def test_unsupported_format_calls_shutdown_and_returns_none(
        self, mock_load_file: Mock, mock_recho: Mock
    ):
        """Should handle UnsupportedDatabaseFormatError by logging and shutting down."""
        mock_load_file.side_effect = UnsupportedDatabaseFormatError(
            e_arxml=None, e_dbc="bad_format", e_kcd=None, e_sym=None, e_cdd=None
        )
        # pylint: disable=W0212
        result = self.proc._load_database()
        self.assertIsNone(result)
        self.proc.shutdown.assert_called_once()
        mock_recho.assert_called_once_with("DBC format is not valid!")

    @patch("cli.com.can_com.recho")
    @patch("cli.com.can_com.database.load_file")
    def test_file_not_found_calls_shutdown_and_returns_none(
        self, mock_load_file: Mock, mock_recho: Mock
    ):
        """Should handle FileNotFoundError by logging and shutting down."""
        mock_load_file.side_effect = FileNotFoundError("not found")
        # pylint: disable=W0212
        result = self.proc._load_database()
        self.assertIsNone(result)
        self.proc.shutdown.assert_called_once()
        mock_recho.assert_called_once_with("DBC file is not found!")


class TestGetCanMsg(unittest.TestCase):
    """Tests for CANProcess._get_can_msg in isolation."""

    def test_returns_same_message_object(self) -> None:
        """Ensure that passing a Message instance returns it unchanged."""
        input_msg = Message(arbitration_id=0x123, data=b"\x01\x02")
        # pylint: disable=W0212
        result = CANProcess._get_can_msg(input_msg, start_time=0.0, dbc=None)
        self.assertIs(result, input_msg)

    def test_constructs_message_from_raw_dict_data(self) -> None:
        """Ensure that dict with raw bytes is converted into a Message."""
        msg_dict = {"id": 0x321, "data": b"\xaa\xbb\xcc"}
        # pylint: disable=W0212
        result = CANProcess._get_can_msg(msg_dict, start_time=0.0, dbc=None)
        self.assertIsInstance(result, Message)
        self.assertEqual(result.arbitration_id, 0x321)
        self.assertEqual(result.data, b"\xaa\xbb\xcc")
        self.assertFalse(result.is_extended_id)

    def test_returns_none_when_dict_data_requires_dbc_but_none_provided(self) -> None:
        """If dict data is a field dict and no DBC is given, it should return None."""
        msg_dict = {"id": 0x111, "data": {"field1": 1, "field2": 2}}
        # pylint: disable=W0212
        result = CANProcess._get_can_msg(msg_dict, start_time=0.0, dbc=None)
        self.assertIsNone(result)

    def test_encodes_dict_data_with_dbc(self) -> None:
        """Dict data should be encoded using the provided DBC."""
        msg_dict = {"id": 0x555, "data": {"field1": 1, "field2": 2}}
        fake_dbc = Mock()
        fake_dbc.get_message_by_frame_id().encode.return_value = [
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8,
        ]
        # pylint: disable=W0212
        result = CANProcess._get_can_msg(msg_dict, start_time=0.0, dbc=fake_dbc)
        self.assertIsInstance(result, Message)
        self.assertEqual(result.arbitration_id, 0x555)
        self.assertEqual(result.data, b"\x01\x02\x03\x04\x05\x06\x07\x08")
        self.assertFalse(result.is_extended_id)


class TestCANProcessWriteCanMessages(unittest.TestCase):
    """Tests for CANProcess._write_can_messages in isolation."""

    def test_sends_message_instances_and_dict_messages(self) -> None:
        """Sends direct Message instances and builds Message from dicts (list and dict data)."""
        control = ComControl()
        control.shutdown.is_set = Mock(return_value=False)
        proc = CANProcess(
            "Writer", control, CanBusConfig(interface="pcan", dbc="file.dbc")
        )
        # CANProcess._get_can_msg = Mock(side_effect=["test", "test", "test", KeyError])
        # pylint: disable=W0212
        proc._bus = MagicMock()
        proc._bus.__enter__.return_value = Mock()
        with patch.object(proc, "_load_database") as mock_load_file:
            control.input.put(
                Message(arbitration_id=123, is_extended_id=False, data=[1, 2, 3])
            )
            control.input.put({"id": 123, "data": [1, 2, 3]})
            control.input.put({"id": 0x7E0, "data": {"SignalA": 1}})
            t = Thread(target=proc._write_can_messages, daemon=True)
            t.start()
            t.join(timeout=0.5)
            # pylint: disable=E1101
            self.assertEqual(proc._bus.__enter__.return_value.send.call_count, 3)
            mock_load_file.assert_called_once()

    def test_no_dbc_or_no_bus(self) -> None:
        """Tests the method with no available dbc or bus"""
        # case with bus none
        control = ComControl()
        proc = CANProcess(
            "Writer", control, CanBusConfig(interface="pcan", dbc="file.dbc")
        )
        # pylint: disable=W0212
        with patch("cli.com.can_com.sleep") as mock_sleep:
            t = Thread(target=proc._write_can_messages, daemon=True)
            t.start()
            t.join(timeout=0.1)
            mock_sleep.assert_not_called()
        # case with dbc file is none
        proc = CANProcess("Writer", control, CanBusConfig(interface="pcan"))
        proc._bus = MagicMock()
        with patch.object(proc, "_load_database") as mock_load_file:
            t = Thread(target=proc._write_can_messages, daemon=True)
            t.start()
            t.join(timeout=0.1)
            mock_load_file.assert_not_called()

    @patch("cli.com.can_com.logger")
    def test_message_invalid_type(self, mock_logger: Mock) -> None:
        """Testcase with a CAN message with invalid type."""
        control = ComControl()
        control.shutdown.is_set = Mock(side_effect=[False, True])
        proc = CANProcess(
            "Writer", control, CanBusConfig(interface="pcan", dbc="file.dbc")
        )
        # pylint: disable=W0212
        proc._bus = MagicMock()
        proc._bus.__enter__.return_value = Mock()
        with patch.object(proc, "_load_database"):
            control.input.put([1, 2, 3])
            while control.input.empty():
                time.sleep(0.1)
            t = Thread(target=proc._write_can_messages, daemon=True)
            t.start()
            t.join(timeout=0.1)
            mock_logger.debug.assert_called_once_with(
                "Message '%s' is ignored. DBC file not valid or it "
                "is not a dict or Message object.",
                [1, 2, 3],
            )

    @patch("cli.com.can_com.logger")
    def test_message_key_error(self, mock_logger: Mock) -> None:
        """Test message with missing keys"""
        control = ComControl()
        control.shutdown.is_set = Mock(side_effect=[False, True])
        proc = CANProcess(
            "Writer", control, CanBusConfig(interface="pcan", dbc="file.dbc")
        )
        # pylint: disable=W0212
        proc._bus = MagicMock()
        proc._bus.__enter__.return_value = Mock()
        with patch.object(proc, "_load_database"):
            with patch.object(CANProcess, "_get_can_msg", side_effect=KeyError):
                control.input.put({"data": [1, 2, 3]})
                # Ensure empty returns correct status of the queue
                while control.input.empty():
                    time.sleep(0.1)
                t = Thread(target=proc._write_can_messages, daemon=True)
                t.start()
                t.join(timeout=0.1)
                mock_logger.error.assert_called_once_with(
                    "Provided json dictionary does not contain at "
                    "least one mandatory key id or data in the in "
                    "the defined CAN message."
                )


class TestCANProcessReceiveCanMessages(unittest.TestCase):
    """Tests for CANProcess._receive_can_messages in isolation."""

    @patch("cli.com.can_com.recho")
    def test_too_many_can_errors_triggers_shutdown(self, mock_recho: Mock) -> None:
        """Accumulates operation errors and triggers shutdown when threshold is reached."""
        control = ComControl()
        proc = CANProcess("Receiver", control, CanBusConfig(interface="pcan"))
        # pylint: disable=W0212
        proc._bus = MagicMock()
        proc._bus.__enter__.return_value = Mock()
        op_error = CanOperationError("err")
        with (
            patch.object(
                proc, "_receive_can_message", side_effect=[op_error, op_error, op_error]
            ),
            patch("cli.com.can_com.MAX_CAN_OPERATION_ERRORS_PER_HOUR", 3),
        ):
            t = Thread(target=proc._receive_can_messages, daemon=True)
            t.start()
            t.join(timeout=2)

        self.assertTrue(control.shutdown.is_set())
        mock_recho.assert_called_with(
            "Too many CAN errors occurred while receiving messages."
        )

    @patch("cli.com.can_com.logger")
    def test_bus_none(self, mock_logger: Mock) -> None:
        """Test receive_can_messages method with no bus"""
        control = ComControl()
        proc = CANProcess("Receiver", control, CanBusConfig(interface="pcan"))
        # pylint: disable=W0212
        proc._bus = None
        t = Thread(target=proc._receive_can_messages, daemon=True)
        t.start()
        t.join(timeout=2)
        mock_logger.info.assert_not_called()


class TestCANProcessReceiveCanMessage(unittest.TestCase):
    """Tests for CANProcess._receive_can_message in isolation."""

    def test_puts_received_messages_on_output_queue(self) -> None:
        """Receives a message from bus and forwards it to output until shutdown."""
        bus = Mock()
        bus.recv = Mock(return_value="CAN_MSG")
        control = ComControl()
        proc = CANProcess("RecvOne", control, CanBusConfig(interface="pcan"))
        # pylint: disable=W0212
        t = Thread(
            name="test", target=proc._receive_can_message, daemon=True, args=[bus]
        )
        t.start()
        control.shutdown.set()
        t.join(timeout=0.1)
        # time.sleep is used to give empty() the chance to update its status
        time.sleep(0.1)
        self.assertFalse(control.output.empty())
        self.assertEqual(control.output.get(), "CAN_MSG")

    def test_recv_timeout(self) -> None:
        """Receives a message from bus and forwards it to output until shutdown."""
        bus = Mock()
        bus.recv = Mock(return_value=None)
        control = ComControl()
        proc = CANProcess("RecvOne", control, CanBusConfig(interface="pcan"))
        # pylint: disable=W0212
        with patch.object(control.output, "put") as mock_put:
            t = Thread(
                name="test", target=proc._receive_can_message, daemon=True, args=[bus]
            )
            t.start()
            control.shutdown.set()
            t.join(timeout=0.1)
            # time.sleep is used to give empty() the chance to update its status
            time.sleep(0.1)
            mock_put.assert_not_called()


if __name__ == "__main__":
    unittest.main()
