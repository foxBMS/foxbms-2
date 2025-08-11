#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Testing file 'cli/cmd_bms/bms_impl.py'."""

import datetime
import io
import shutil
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from multiprocessing import Manager, Queue
from pathlib import Path
from queue import Empty
from unittest.mock import MagicMock, call, patch

from can import CanInitializationError, CanOperationError
from can.io import SizedRotatingLogger

try:
    from cli.cmd_bms.bms_impl import (
        get_boot_timestamp,
        get_build_configuration,
        get_commit_hash,
        get_mcu_id,
        get_mcu_lot_number,
        get_mcu_wafer_info,
        get_rtc_time,
        get_software_version,
        get_uptime,
        initialization,
        initialize_logger,
        log_can_message,
        read_can_message,
        receive_send_can_message,
        reinitialize_fram,
        reset_software,
        set_rtc_time,
        shutdown,
    )
    from cli.helpers.misc import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_bms.bms_impl import (
        get_boot_timestamp,
        get_build_configuration,
        get_commit_hash,
        get_mcu_id,
        get_mcu_lot_number,
        get_mcu_wafer_info,
        get_rtc_time,
        get_software_version,
        get_uptime,
        initialization,
        initialize_logger,
        log_can_message,
        read_can_message,
        receive_send_can_message,
        reinitialize_fram,
        reset_software,
        set_rtc_time,
        shutdown,
    )
    from cli.helpers.misc import PROJECT_BUILD_ROOT


class TestCreateMessage(unittest.TestCase):
    """Tests all methods that create a message"""

    def test_reinitialize_fram(self):
        """Test the reinitialize_fram  method"""
        queue = Queue()
        reinitialize_fram(queue)
        msg = queue.get()
        self.assertEqual(768, msg.arbitration_id)
        self.assertEqual(0.0, msg.timestamp)
        self.assertEqual(bytearray(b"\x03\xff\xff\xff\xff\xff\xff\xff"), msg.data)

    def test_set_rtc_time(self):
        """Test the set_rtc_time method."""
        queue = Queue()
        set_rtc_time(queue)
        msg = queue.get()
        self.assertEqual(768, msg.arbitration_id)
        self.assertEqual(0.0, msg.timestamp)

    def test_get_rtc_time(self):
        """Tests the get_rtc_time method."""
        queue = Queue()
        get_rtc_time(queue)
        msg = queue.get()
        self.assertEqual(768, msg.arbitration_id)
        self.assertEqual(0.0, msg.timestamp)
        self.assertEqual(bytearray(b"\x04\xfd\xff\xff\xff\xff\xff\xff"), msg.data)

    def test_get_boot_timestamp(self):
        """Tests the get_boot_timestamp method"""
        queue = Queue()
        get_boot_timestamp(queue)
        msg = queue.get()
        self.assertEqual(768, msg.arbitration_id)
        self.assertEqual(0.0, msg.timestamp)
        self.assertEqual(bytearray(b"\x04\xfe\xff\xff\xff\xff\xff\xff"), msg.data)

    def test_reset_software(self):
        """Tests the reset_software method."""
        queue = Queue()
        reset_software(queue)
        msg = queue.get()
        self.assertEqual(768, msg.arbitration_id)
        self.assertEqual(0.0, msg.timestamp)
        self.assertEqual(bytearray(b"\x02\xff\xff\xff\xff\xff\xff\xff"), msg.data)

    def test_get_uptime(self):
        """Tests the get_uptime method."""
        queue = Queue()
        get_uptime(queue)
        msg = queue.get()
        self.assertEqual(768, msg.arbitration_id)
        self.assertEqual(0.0, msg.timestamp)
        self.assertEqual(bytearray(b"\x05\xff\xff\xff\xff\xff\xff\xff"), msg.data)

    def test_get_build_configuration(self):
        """Tests the get_build_configuration method."""
        queue = Queue()
        get_build_configuration(queue)
        msg = queue.get()
        self.assertEqual(768, msg.arbitration_id)
        self.assertEqual(0.0, msg.timestamp)
        self.assertEqual(bytearray(b"\x00\xe0\xff\xff\xff\xff\xff\xff"), msg.data)

    def test_get_commit_hash(self):
        """Tests the get_commit_hash method."""
        queue = Queue()
        get_commit_hash(queue)
        msg = queue.get()
        self.assertEqual(768, msg.arbitration_id)
        self.assertEqual(0.0, msg.timestamp)
        self.assertEqual(bytearray(b"\x00\xd0\xff\xff\xff\xff\xff\xff"), msg.data)

    def test_get_mcu_wafer_info(self):
        """Tests the get_mcu_wafer_info method."""
        queue = Queue()
        get_mcu_wafer_info(queue)
        msg = queue.get()
        self.assertEqual(768, msg.arbitration_id)
        self.assertEqual(0.0, msg.timestamp)
        self.assertEqual(bytearray(b"\x00\xc8\xff\xff\xff\xff\xff\xff"), msg.data)

    def test_get_mcu_lot_number(self):
        """Tests the get_mcu_lot_number method."""
        queue = Queue()
        get_mcu_lot_number(queue)
        msg = queue.get()
        self.assertEqual(768, msg.arbitration_id)
        self.assertEqual(0.0, msg.timestamp)
        self.assertEqual(bytearray(b"\x00\xc4\xff\xff\xff\xff\xff\xff"), msg.data)

    def test_get_mcu_id(self):
        """Tests the get_mcu_id method."""
        queue = Queue()
        get_mcu_id(queue)
        msg = queue.get()
        self.assertEqual(768, msg.arbitration_id)
        self.assertEqual(0.0, msg.timestamp)
        self.assertEqual(bytearray(b"\x00\xc2\xff\xff\xff\xff\xff\xff"), msg.data)

    def test_get_software_version(self):
        """Tests the get_software_version method."""
        queue = Queue()
        get_software_version(queue)
        msg = queue.get()
        self.assertEqual(768, msg.arbitration_id)
        self.assertEqual(0.0, msg.timestamp)
        self.assertEqual(bytearray(b"\x00\xc1\xff\xff\xff\xff\xff\xff"), msg.data)


class TestInitializeLogger(unittest.TestCase):
    """Test initialize_logger method"""

    def setUp(self):
        self.start_time = datetime.datetime.now()

    def tearDown(self):
        logs_dir = PROJECT_BUILD_ROOT / "logs"
        if logs_dir.is_dir():
            time_logs_dir = logs_dir.stat().st_mtime
            mod_time_logs_dir = datetime.datetime.fromtimestamp(time_logs_dir)
            if mod_time_logs_dir >= self.start_time:
                shutil.rmtree(logs_dir)

    @patch("cli.cmd_bms.bms_impl.SizedRotatingLogger")
    def test_failure(self, mock_logger: MagicMock):
        """Creating SizedRotatingLogger object failed."""
        mock_logger.side_effect = ValueError
        with self.assertRaises(ValueError):
            initialize_logger()
        logs_dir = PROJECT_BUILD_ROOT / Path("logs")
        self.assertTrue(logs_dir.is_dir())

    def test_success(self):
        """SizedRotatingLogger object successfully created."""
        ret = initialize_logger()
        ret.stop()
        logs_dir = PROJECT_BUILD_ROOT / Path("logs")
        self.assertIsInstance(ret, SizedRotatingLogger)
        self.assertTrue(logs_dir.is_dir())
        self.assertTrue((logs_dir / Path("foxBMS_CAN_bms_log.txt")).exists())


class TestInitialize(unittest.TestCase):
    """Test initialization method"""

    @patch("cli.cmd_bms.bms_impl.Process")
    def test_receive_send_process_failure(self, mock_process: MagicMock):
        """Creating the receive and send process failed."""
        mock_process = mock_process.return_value
        mock_process.start.side_effect = RuntimeError
        buf = io.StringIO()
        with redirect_stderr(buf):
            ret = initialization(
                "", MagicMock(), MagicMock(), MagicMock(), MagicMock(), MagicMock()
            )
        self.assertEqual(
            "Could not start receive and send process.\nExiting...\n", buf.getvalue()
        )
        self.assertEqual(ret, False)

    @patch("cli.cmd_bms.bms_impl.Process")
    def test_can_bus_failure(self, mock_process: MagicMock):
        """Initializing the CAN bus failed."""
        network_ok = MagicMock()
        network_ok.wait.return_value = False
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        err = io.StringIO()
        with redirect_stderr(err):
            ret = initialization(
                "", MagicMock(), MagicMock(), MagicMock(), network_ok, MagicMock()
            )
        self.assertEqual(
            "Could not initialize CAN bus. Timeout\nShutdown...\n", err.getvalue()
        )
        self.assertEqual(ret, False)

    @patch("cli.cmd_bms.bms_impl.Process")
    @patch("cli.cmd_bms.bms_impl.shutdown")
    def test_read_process_failure(
        self,
        mock_shutdown: MagicMock,  # pylint: disable=unused-argument
        mock_process: MagicMock,
    ):
        """Initializing the read process failed."""
        network_ok = MagicMock()
        network_ok.wait.return_value = True
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.side_effect = [0, RuntimeError]
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = initialization(
                "", MagicMock(), MagicMock(), MagicMock(), network_ok, MagicMock()
            )
        self.assertEqual("Could not start read process.\n", err.getvalue())
        self.assertEqual(
            "Started the receive and send process and initialized the CAN bus.\n",
            out.getvalue(),
        )
        self.assertEqual(ret, False)

    @patch("cli.cmd_bms.bms_impl.Process")
    @patch("cli.cmd_bms.bms_impl.shutdown")
    def test_sizedrotatinglogger_failure(
        self,
        mock_shutdown: MagicMock,  # pylint: disable=unused-argument
        mock_process: MagicMock,
    ):
        """Initializing the SizedRotatingLogger failed."""
        network_ok = MagicMock()
        network_ok.wait.side_effect = [True, False]
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = initialization(
                "", MagicMock(), MagicMock(), MagicMock(), network_ok, MagicMock()
            )
        self.assertEqual(
            "Could not create logger object.\n",
            err.getvalue(),
        )
        self.assertEqual(
            "Started the receive and send process and initialized the CAN bus.\n",
            out.getvalue(),
        )
        self.assertEqual(ret, False)

    @patch("cli.cmd_bms.bms_impl.Process")
    def test_success(
        self,
        mock_process: MagicMock,
    ):
        """Test processes successfully started."""
        network_ok = MagicMock()
        network_ok.wait.return_value = True
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        buf = io.StringIO()
        with redirect_stdout(buf):
            ret_init = initialization(
                "", MagicMock(), MagicMock(), MagicMock(), network_ok, MagicMock()
            )
        self.assertEqual(  # pylint: disable-next=line-too-long
            "Started the receive and send process and initialized the CAN bus.\nStarted the read process.\n",
            buf.getvalue(),
        )
        self.assertIsInstance(ret_init, tuple)


class TestShutdown(unittest.TestCase):
    """Test shutdown method"""

    @patch("cli.cmd_bms.bms_impl.sleep")
    def test_failure(self, mock_sleep: MagicMock):
        """Test processes not gracefully cancelled."""
        network_ok = MagicMock()
        network_ok.clear.return_value = True
        p_recv = MagicMock()
        p_read = MagicMock()
        p_recv.join.return_value = 0
        p_recv.is_alive.return_value = True
        p_read.join.return_value = 0
        p_read.is_alive.return_value = True
        mock_sleep.return_value = 0
        out = io.StringIO()
        err = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            shutdown(p_recv, p_read, network_ok)
        self.assertEqual(  # pylint: disable-next=line-too-long
            "Could not cancel the receive and send process gracefully.\nTerminating...\nCould not cancel the read process gracefully.\nTerminating...\n",
            err.getvalue(),
        )
        self.assertEqual("Shutdown...\n", out.getvalue())

    @patch("cli.cmd_bms.bms_impl.sleep")
    def test_success(self, mock_sleep: MagicMock):
        """Test processes gracefully cancelled."""
        network_ok = MagicMock()
        network_ok.clear.return_value = True
        p_recv = MagicMock()
        p_read = MagicMock()
        p_recv.join.return_value = 0
        p_recv.is_alive.return_value = False
        p_read.join.return_value = 0
        p_read.is_alive.return_value = False
        mock_sleep.return_value = 0
        out = io.StringIO()
        err = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            shutdown(p_recv, p_read, network_ok)
        self.assertEqual("", err.getvalue())
        self.assertEqual("Shutdown...\n", out.getvalue())


class TestLogCanMessage(unittest.TestCase):
    """Test log_can_message method"""

    def setUp(self):
        self.manager = Manager()

    def test_array_empty(self):
        """Tests the log_can_message function when msg_arr is empty"""
        msg_arr = self.manager.list([[], [], []])
        msg = MagicMock()
        msg.arbitration_id = 1
        log_can_message(msg, msg_arr, MagicMock(), "")
        self.assertEqual(msg_arr[0], [])
        self.assertEqual(msg_arr[1], [])
        self.assertEqual(msg_arr[2], [])

    @patch("cantools.database.can.database.Database.get_message_by_frame_id")
    def test_amount_one(self, mock_get_message):
        """Tests the log_can_message function when msg has to be logged once."""
        mock_get_message.return_value.decode.return_value = {"Message": "content"}
        msg_arr = self.manager.list([[1], [1], [1]])
        msg = MagicMock()
        msg.arbitration_id = 1
        buf = io.StringIO()
        with redirect_stdout(buf):
            log_can_message(msg, msg_arr, MagicMock(), "prompt")
        self.assertEqual(
            """Message ID 0x1: {'Message': 'content'}
All messages with ID 0x1 have been logged.\nprompt""",
            buf.getvalue(),
        )
        self.assertEqual(msg_arr[0], [])
        self.assertEqual(msg_arr[1], [])
        self.assertEqual(msg_arr[2], [])

    @patch("cantools.database.can.database.Database.get_message_by_frame_id")
    def test_amount_five(self, mock_get_message):
        """Tests the log_can_message function when msg has to be logged several times."""
        mock_get_message.return_value.decode.return_value = "Message"
        msg_arr = self.manager.list([[1], [5], [1]])
        msg = MagicMock()
        msg.arbitration_id = 1
        buf = io.StringIO()
        with redirect_stdout(buf):
            log_can_message(msg, msg_arr, MagicMock(), "prompt")
        self.assertEqual("Message ID 0x1: Message\n", buf.getvalue())
        self.assertEqual(msg_arr[0], [1])
        self.assertEqual(msg_arr[1], [4])
        self.assertEqual(msg_arr[2], [1])

    @patch("cantools.database.can.database.Database.get_message_by_frame_id")
    def test_amount_zero(self, mock_get_message):
        """Tests the log_can_message function when the msg ID is in the array
        but it is not supposed to be logged."""
        mock_get_message.return_value.decode.return_value = "Message"
        msg_arr = self.manager.list([[1], [0], [0]])
        msg = MagicMock()
        msg.arbitration_id = 1
        buf = io.StringIO()
        with redirect_stdout(buf):
            log_can_message(msg, msg_arr, MagicMock(), "prompt")
        self.assertEqual("", buf.getvalue())
        self.assertEqual(msg_arr[0], [])
        self.assertEqual(msg_arr[1], [])
        self.assertEqual(msg_arr[2], [])

    def test_log_to_file(self):
        """Tests the log_can_message function when msg has to be logged to a file."""
        msg_arr = self.manager.list([[1], [2], [0]])
        msg = MagicMock()
        msg.arbitration_id = 1
        logger = MagicMock()
        log_can_message(msg, msg_arr, logger, "prompt")
        self.assertEqual(msg_arr[0], [1])
        self.assertEqual(msg_arr[1], [1])
        self.assertEqual(msg_arr[2], [0])

    @patch("cantools.database.can.database.Database.get_message_by_frame_id")
    def test_short_hash_high(self, mock_get_message: MagicMock):
        """Tests function when message contains 'shortHashHigh7' key"""
        mock_get_message.return_value.decode.return_value = {
            "shortHashHigh7": 1630615892
        }
        msg_arr = self.manager.list([[1], [1], [1]])
        msg = MagicMock()
        msg.arbitration_id = 1
        buf = io.StringIO()
        with redirect_stdout(buf):
            log_can_message(msg, msg_arr, MagicMock(), "prompt")
        self.assertEqual(
            """Message ID 0x1: {'shortHashHigh7': 'a19T'}
All messages with ID 0x1 have been logged.\nprompt""",
            buf.getvalue(),
        )
        self.assertEqual(msg_arr[0], [])
        self.assertEqual(msg_arr[1], [])
        self.assertEqual(msg_arr[2], [])

    @patch("cantools.database.can.database.Database.get_message_by_frame_id")
    def test_short_hash_low(self, mock_get_message: MagicMock):
        """Tests function when message contains 'shortHashLow7' key"""
        mock_get_message.return_value.decode.return_value = {
            "shortHashLow7": 1630615892
        }
        msg_arr = self.manager.list([[1], [1], [1]])
        msg = MagicMock()
        msg.arbitration_id = 1
        buf = io.StringIO()
        with redirect_stdout(buf):
            log_can_message(msg, msg_arr, MagicMock(), "prompt")
        self.assertEqual(
            """Message ID 0x1: {'shortHashLow7': 'a19T'}
All messages with ID 0x1 have been logged.\nprompt""",
            buf.getvalue(),
        )
        self.assertEqual(msg_arr[0], [])
        self.assertEqual(msg_arr[1], [])
        self.assertEqual(msg_arr[2], [])


class TestReceiveSendCanMessage(unittest.TestCase):
    """Test receive_send_can_message method"""

    @patch("cli.cmd_bms.bms_impl.asdict")
    @patch("cli.cmd_bms.bms_impl.Bus")
    def test_bus_init_failure(self, mock_bus: MagicMock, mock_asdict: MagicMock):  # pylint: disable=unused-argument
        """Tests initialization error of the CAN bus."""
        mock_bus.side_effect = CanInitializationError
        buf = io.StringIO()
        with redirect_stderr(buf):
            receive_send_can_message(MagicMock(), MagicMock(), mock_bus, MagicMock())
        self.assertEqual("Could not initialize CAN bus.\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_impl.asdict")
    @patch("cli.cmd_bms.bms_impl.Bus")
    def test_stop_bus_queue_empty(self, mock_bus: MagicMock, mock_asdict: MagicMock):  # pylint: disable=unused-argument
        """Test that the CAN bus is stopped on a stop event and the case that
        'recv' method of the CAN bus and the send-queue is empty."""
        rec_q = MagicMock()
        send_q = MagicMock()
        send_q.get.side_effect = Empty
        network_ok = MagicMock()
        network_ok.set.return_value = None
        network_ok.is_set.return_value = False
        mock_bus.return_value.__enter__.return_value.recv.return_value = None
        buf = io.StringIO()
        with redirect_stdout(buf):
            receive_send_can_message(rec_q, send_q, mock_bus, network_ok)
        self.assertEqual("Stop receiving and sending messages.\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_impl.asdict")
    @patch("cli.cmd_bms.bms_impl.Bus")
    def test_stop_bus_emtpy(self, mock_bus: MagicMock, mock_asdict: MagicMock):  # pylint: disable=unused-argument
        """Test that the CAN bus is stopped on a stop event and the case that
        'recv' method of the CAN bus is empty."""
        rec_q = MagicMock()
        send_q = MagicMock()
        network_ok = MagicMock()
        network_ok.set.return_value = None
        network_ok.is_set.return_value = False
        mock_bus.return_value.__enter__.return_value.recv.return_value = None
        buf = io.StringIO()
        with redirect_stdout(buf):
            receive_send_can_message(rec_q, send_q, mock_bus, network_ok)
        self.assertEqual("Stop receiving and sending messages.\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_impl.asdict")
    @patch("cli.cmd_bms.bms_impl.Bus")
    def test_stop_bus_receives_msgs(self, mock_bus: MagicMock, mock_asdict: MagicMock):  # pylint: disable=unused-argument
        """Test that the CAN bus is stopped on a stop event and the case that
        'recv' method of the CAN bus receives a few messages."""
        rec_q = MagicMock()
        send_q = MagicMock()
        network_ok = MagicMock()
        network_ok.set.return_value = None
        network_ok.is_set.side_effect = [True, True, True, False]
        mock_bus.return_value.__enter__.return_value.recv.return_value = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            receive_send_can_message(rec_q, send_q, mock_bus, network_ok)
        self.assertEqual("Stop receiving and sending messages.\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_impl.asdict")
    @patch("cli.cmd_bms.bms_impl.Bus")
    def test_stop_errors(self, mock_bus: MagicMock, mock_asdict: MagicMock):  # pylint: disable=unused-argument
        """Test that the CAN bus is stopped after too many errors."""
        rec_q = MagicMock()
        send_q = MagicMock()
        network_ok = MagicMock()
        mock_bus.return_value.__enter__.return_value.recv.side_effect = (
            CanOperationError
        )
        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err):
            receive_send_can_message(rec_q, send_q, mock_bus, network_ok)
        self.assertEqual("Stop receiving and sending messages.\n", out.getvalue())
        self.assertEqual(
            "Too many errors occurred while receiving and sending messages.\n",
            err.getvalue(),
        )


class TestReadCanMessage(unittest.TestCase):
    """Test read_can_message method"""

    @patch("cli.cmd_bms.bms_impl.initialize_logger")
    def test_logger_init_failure(self, mock_logger: MagicMock):
        """Tests the read_can_message method in the case that initializing the Logger failed."""
        mock_logger.side_effect = ValueError
        rec_q = Queue()
        network_ok = MagicMock()
        network_ok.clear = MagicMock()
        read_can_message("", rec_q, network_ok, MagicMock())
        network_ok.clear.assert_called_once()

    @patch("cli.cmd_bms.bms_impl.initialize_logger")
    def test_network_not_set(self, mock_logger: MagicMock):
        """Tests the read_can_message method in the case that network_ok is not set."""
        rec_q = Queue()
        network_ok = MagicMock()
        network_ok.is_set.return_value = False
        mock_logger_instance = mock_logger.return_value
        buf = io.StringIO()
        with redirect_stdout(buf):
            read_can_message("", rec_q, network_ok, MagicMock())
        self.assertEqual("", buf.getvalue())
        network_ok.is_set.assert_has_calls([call(), call()])
        mock_logger_instance.stop.assert_called_once()

    @patch("cli.cmd_bms.bms_impl.initialize_logger")
    def test_network_set_no_msgs(self, mock_logger: MagicMock):
        """Tests the read_can_message method
        in the case that network_ok is set once
        but there are no messages."""
        rec_q = Queue()
        network_ok = MagicMock()
        network_ok.is_set = MagicMock()
        network_ok.is_set.side_effect = [True, False, False]
        mock_logger_instance = mock_logger.return_value
        buf = io.StringIO()
        with redirect_stdout(buf):
            read_can_message("", rec_q, network_ok, MagicMock())
        self.assertEqual("", buf.getvalue())
        network_ok.is_set.assert_has_calls([call(), call(), call()])
        mock_logger_instance.stop.assert_called_once()

    @patch("cli.cmd_bms.bms_impl.log_can_message")
    @patch("cli.cmd_bms.bms_impl.initialize_logger")
    def test_network_set_not_enough_msgs(
        self, mock_logger: MagicMock, mock_log_message: MagicMock
    ):
        """Tests the read_can_message method
        in the case that network_ok is set a few times
        but there are less messages."""
        msg = MagicMock()
        msg.timestamp = 1
        rec_q = MagicMock()
        rec_q.get = MagicMock()
        rec_q.get.side_effect = [msg, msg, Empty]
        rec_q.empty.side_effect = [True]
        network_ok = MagicMock()
        network_ok.is_set = MagicMock()
        network_ok.is_set.side_effect = [True, True, True, False]
        msg_arr = MagicMock()
        logger = MagicMock()
        mock_logger.return_value = logger
        buf = io.StringIO()
        with redirect_stdout(buf):
            read_can_message("", rec_q, network_ok, msg_arr)
        self.assertEqual("", buf.getvalue())
        network_ok.is_set.assert_has_calls([call(), call(), call(), call()])
        rec_q.get.assert_has_calls(
            [call(block=False), call(block=False), call(block=False)]
        )
        mock_log_message.assert_has_calls(
            [
                call(msg=msg, msg_arr=msg_arr, logger=logger, prompt=""),
                call(msg=msg, msg_arr=msg_arr, logger=logger, prompt=""),
            ]
        )
        logger.stop.assert_called_once()

    @patch("cli.cmd_bms.bms_impl.log_can_message")
    @patch("cli.cmd_bms.bms_impl.initialize_logger")
    def test_network_set_more_msgs(
        self, mock_logger: MagicMock, mock_log_message: MagicMock
    ):
        """Tests the read_can_message method
        in the case that network_ok is set a few times,
        there are more messages and they are not logged."""
        msg = MagicMock()
        msg.timestamp = 1
        rec_q = MagicMock()
        rec_q.get = MagicMock()
        rec_q.get.side_effect = [msg, msg, msg]
        rec_q.empty.side_effect = [False, True]
        network_ok = MagicMock()
        network_ok.is_set = MagicMock()
        network_ok.is_set.side_effect = [True, True, False]
        msg_arr = MagicMock()
        logger = MagicMock()
        mock_logger.return_value = logger
        buf = io.StringIO()
        with redirect_stdout(buf):
            read_can_message("", rec_q, network_ok, msg_arr)
        self.assertEqual("", buf.getvalue())
        logger.stop.assert_called_once()
        network_ok.is_set.assert_has_calls([call(), call(), call()])
        rec_q.get.assert_has_calls(
            [call(block=False), call(block=False), call(block=False)]
        )
        mock_log_message.assert_has_calls(
            [
                call(msg=msg, msg_arr=msg_arr, logger=logger, prompt=""),
                call(msg=msg, msg_arr=msg_arr, logger=logger, prompt=""),
                call(msg=msg, msg_arr=msg_arr, logger=logger, prompt=""),
            ]
        )


if __name__ == "__main__":
    unittest.main()
