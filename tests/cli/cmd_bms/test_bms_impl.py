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

import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from multiprocessing import Queue
from pathlib import Path
from queue import Empty
from unittest.mock import MagicMock, call, patch

from can import CanInitializationError, CanOperationError

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
        read_can_message,
        receive_send_can_message,
        reinitialize_fram,
        reset_software,
        set_rtc_time,
        shutdown,
    )
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
        read_can_message,
        receive_send_can_message,
        reinitialize_fram,
        reset_software,
        set_rtc_time,
        shutdown,
    )


class TestBmsImpl(unittest.TestCase):  # pylint: disable=too-many-public-methods
    """Tests the bms implementation script"""

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

    @patch("cli.cmd_bms.bms_impl.Process")
    def test_initialization_failure_0(self, mock_process: MagicMock):
        """Creating the receive and send process failed."""
        mock_process = mock_process.return_value
        mock_process.start.side_effect = RuntimeError
        buf = io.StringIO()
        with redirect_stderr(buf):
            ret = initialization(
                MagicMock(), MagicMock(), MagicMock(), MagicMock(), 0, 0
            )
        self.assertEqual(
            "Could not start receive and send process.\nExiting...\n", buf.getvalue()
        )
        self.assertEqual(ret, False)

    @patch("cli.cmd_bms.bms_impl.Process")
    def test_initialization_failure_1(self, mock_process: MagicMock):
        """Initializing the CAN bus failed."""
        network_ok = MagicMock()
        network_ok.wait.return_value = False
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        buf = io.StringIO()
        with redirect_stderr(buf):
            ret = initialization(
                MagicMock(), MagicMock(), MagicMock(), network_ok, 0, 0
            )
        self.assertEqual(
            "Could not initialize CAN bus. Timeout\nShutdown...\n", buf.getvalue()
        )
        self.assertEqual(ret, False)

    @patch("cli.cmd_bms.bms_impl.Process")
    def test_initialization_success(self, mock_process: MagicMock):
        """Test processes successfully started."""
        network_ok = MagicMock()
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        network_ok.wait.return_value = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            ret_init = initialization(
                MagicMock(), MagicMock(), MagicMock(), network_ok, 0, 0
            )
        self.assertEqual(  # pylint: disable-next=line-too-long
            "Started the receive and send process and initialized the CAN bus.\nStarted the read process.\n",
            buf.getvalue(),
        )
        self.assertIsInstance(ret_init, tuple)

    @patch("cli.cmd_bms.bms_impl.sleep")
    def test_shutdown_failure(self, mock_sleep: MagicMock):
        """Test processes not gracefully cancelled."""
        network_ok = MagicMock()
        p_recv = MagicMock()
        p_read = MagicMock()
        network_ok.clear.return_value = True
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
    def test_shutdown_success(self, mock_sleep: MagicMock):
        """Test processes gracefully cancelled."""
        network_ok = MagicMock()
        p_recv = MagicMock()
        p_read = MagicMock()
        network_ok.clear.return_value = True
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

    @patch("cli.cmd_bms.bms_impl.asdict")
    @patch("cli.cmd_bms.bms_impl.Bus")
    def test_rec_send_failure_0(self, mock_bus: MagicMock, mock_asdict: MagicMock):  # pylint: disable=unused-argument
        """Tests initialization error of the CAN bus."""
        mock_bus.side_effect = CanInitializationError
        buf = io.StringIO()

        with redirect_stderr(buf):
            receive_send_can_message(MagicMock(), MagicMock(), mock_bus, MagicMock())
        self.assertEqual("Could not initialize CAN bus.\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_impl.asdict")
    @patch("cli.cmd_bms.bms_impl.Bus")
    def test_rec_send_empty(self, mock_bus: MagicMock, mock_asdict: MagicMock):  # pylint: disable=unused-argument
        """Test that the CAN bus is stopped on a stop event and the case that
        'recv' method of the CAN bus and the send-queue is empty."""
        rec_q = MagicMock()
        send_q = MagicMock()
        network_ok = MagicMock()
        mock_bus.return_value.__enter__.return_value.recv.return_value = None
        send_q.get.side_effect = Empty
        network_ok.set.return_value = None
        network_ok.is_set.return_value = False
        buf = io.StringIO()
        with redirect_stdout(buf):
            receive_send_can_message(rec_q, send_q, mock_bus, network_ok)
        self.assertEqual("Stop receiving and sending messages.\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_impl.asdict")
    @patch("cli.cmd_bms.bms_impl.Bus")
    def test_rec_send_stop_0(self, mock_bus: MagicMock, mock_asdict: MagicMock):  # pylint: disable=unused-argument
        """Test that the CAN bus is stopped on a stop event and the case that
        'recv' method of the CAN bus is empty."""
        rec_q = MagicMock()
        send_q = MagicMock()
        network_ok = MagicMock()
        mock_bus.return_value.__enter__.return_value.recv.return_value = None
        network_ok.set.return_value = None
        network_ok.is_set.return_value = False
        buf = io.StringIO()
        with redirect_stdout(buf):
            receive_send_can_message(rec_q, send_q, mock_bus, network_ok)
        self.assertEqual("Stop receiving and sending messages.\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_impl.asdict")
    @patch("cli.cmd_bms.bms_impl.Bus")
    def test_rec_send_stop_1(self, mock_bus: MagicMock, mock_asdict: MagicMock):  # pylint: disable=unused-argument
        """Test that the CAN bus is stopped on a stop event and the case that
        'recv' method of the CAN bus receives a few messages."""
        rec_q = MagicMock()
        send_q = MagicMock()
        network_ok = MagicMock()
        mock_bus.return_value.__enter__.return_value.recv.return_value = True
        network_ok.set.return_value = None
        network_ok.is_set.side_effect = [True, True, True, False]

        buf = io.StringIO()
        with redirect_stdout(buf):
            receive_send_can_message(rec_q, send_q, mock_bus, network_ok)
        self.assertEqual("Stop receiving and sending messages.\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_impl.asdict")
    @patch("cli.cmd_bms.bms_impl.Bus")
    def test_rec_send_errors(self, mock_bus: MagicMock, mock_asdict: MagicMock):  # pylint: disable=unused-argument
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

    def test_read_can_message_0(self):
        """Tests the read_can_message method in the case that network_ok is not set."""
        rec_q = Queue()
        network_ok = MagicMock()
        network_ok.is_set.return_value = False
        buf = io.StringIO()
        with redirect_stdout(buf):
            read_can_message(rec_q, network_ok, 0, 0)
        self.assertEqual("", buf.getvalue())
        network_ok.is_set.assert_has_calls([call(), call()])

    def test_read_can_message_1(self):
        """Tests the read_can_message method
        in the case that network_ok is set a few times
        but there are no messages."""
        rec_q = Queue()
        network_ok = MagicMock()
        network_ok.is_set = MagicMock()
        network_ok.is_set.side_effect = [True, True, False, False]
        buf = io.StringIO()
        with redirect_stdout(buf):
            read_can_message(rec_q, network_ok, 0, 0)
        self.assertEqual("", buf.getvalue())
        network_ok.is_set.assert_has_calls([call(), call(), call(), call()])

    def test_read_can_message_2(self):
        """Tests the read_can_message method
        in the case that network_ok is set a few times
        but there are less messages and they are not logged."""
        rec_q = MagicMock()
        network_ok = MagicMock()
        msg_id = MagicMock()
        msg = MagicMock()
        msg_id.value = 0
        msg.arbitration_id = 1
        msg.timestamp = 1
        rec_q.get = MagicMock()
        network_ok.is_set = MagicMock()
        rec_q.get.side_effect = [msg, msg, Empty]
        rec_q.empty.side_effect = [True]
        network_ok.is_set.side_effect = [True, True, True, False]
        buf = io.StringIO()
        with redirect_stdout(buf):
            read_can_message(rec_q, network_ok, msg_id, 1)
        self.assertEqual("", buf.getvalue())
        network_ok.is_set.assert_has_calls([call(), call(), call(), call()])
        rec_q.get.assert_has_calls(
            [call(block=False), call(block=False), call(block=False)]
        )

    def test_read_can_message_3(self):
        """Tests the read_can_message method
        in the case that network_ok is set a few times,
        there are more messages and they are not logged."""
        rec_q = MagicMock()
        network_ok = MagicMock()
        msg_id = MagicMock()
        msg = MagicMock()
        msg_id.value = 0
        msg.arbitration_id = 1
        msg.timestamp = 1
        rec_q.get = MagicMock()
        network_ok.is_set = MagicMock()
        rec_q.get.side_effect = [msg, msg, msg]
        rec_q.empty.side_effect = [False, True]
        network_ok.is_set.side_effect = [True, True, False]
        buf = io.StringIO()
        with redirect_stdout(buf):
            read_can_message(rec_q, network_ok, msg_id, 1)
        self.assertEqual("", buf.getvalue())
        network_ok.is_set.assert_has_calls([call(), call(), call()])
        rec_q.get.assert_has_calls(
            [call(block=False), call(block=False), call(block=False)]
        )

    @patch("cantools.database.can.database.Database.get_message_by_frame_id")
    def test_read_can_message_4(self, mock_get_message: MagicMock):
        """Tests the read_can_message method
        in the case that network_ok is set a few times, a few messages are logged
        but msg_num is never 0."""
        mock_get_message.return_value.decode.return_value = "Message"
        rec_q = MagicMock()
        network_ok = MagicMock()
        msg_id = MagicMock()
        msg_num = MagicMock()
        msg = MagicMock()
        msg.arbitration_id = 0
        msg.timestamp = 1
        msg.data = ""
        rec_q.get.side_effect = [msg, msg, msg]
        rec_q.empty.side_effect = [False, True]
        network_ok.is_set.side_effect = [True, True, False]
        msg_id.value = 0
        msg_num.value = 4
        buf = io.StringIO()
        with redirect_stdout(buf):
            read_can_message(rec_q, network_ok, msg_id, msg_num)
        self.assertEqual(  # pylint: disable-next=line-too-long
            "Message ID 0: Message\nMessage ID 0: Message\nMessage ID 0: Message\n",
            buf.getvalue(),
        )
        network_ok.is_set.assert_has_calls([call(), call(), call()])
        rec_q.get.assert_has_calls(
            [call(block=False), call(block=False), call(block=False)]
        )

    @patch("cantools.database.can.database.Database.get_message_by_frame_id")
    def test_read_can_message_5(self, mock_get_message: MagicMock):
        """Tests the read_can_message method
        in the case that network_ok is set a few times
        and msg_num is 0 after logging the first message."""
        mock_get_message.return_value.decode.return_value = "Message"
        rec_q = MagicMock()
        network_ok = MagicMock()
        msg_id = MagicMock()
        msg_num = MagicMock()
        msg = MagicMock()
        msg.arbitration_id = 0
        msg.timestamp = 1
        msg.data = ""
        rec_q.get.side_effect = [msg, msg, msg]
        rec_q.empty.side_effect = [False, True]
        network_ok.is_set.side_effect = [True, True, False]
        msg_id.value = 0
        msg_num.value = 1
        buf = io.StringIO()
        with redirect_stdout(buf):
            read_can_message(rec_q, network_ok, msg_id, msg_num)
        self.assertEqual(  # pylint: disable-next=line-too-long
            "Message ID 0: Message\nAll messages with ID 0 have been printed.\n",
            buf.getvalue(),
        )
        network_ok.is_set.assert_has_calls([call(), call(), call()])
        rec_q.get.assert_has_calls(
            [call(block=False), call(block=False), call(block=False)]
        )

    @patch("cantools.database.can.database.Database.get_message_by_frame_id")
    def test_read_can_message_6(self, mock_get_message: MagicMock):
        """Tests the read_can_message method
        in the case that network_ok is set a few times
        and msg_num is 0 after logging the second message."""
        mock_get_message.return_value.decode.return_value = "Message"
        rec_q = MagicMock()
        network_ok = MagicMock()
        msg_id = MagicMock()
        msg_num = MagicMock()
        msg = MagicMock()
        msg.arbitration_id = 0
        msg.timestamp = 1
        msg.data = ""
        rec_q.get.side_effect = [msg, msg, msg]
        rec_q.empty.side_effect = [False, True]
        network_ok.is_set.side_effect = [True, True, False]
        msg_id.value = 0
        msg_num.value = 2
        buf = io.StringIO()
        with redirect_stdout(buf):
            read_can_message(rec_q, network_ok, msg_id, msg_num)
        self.assertEqual(  # pylint: disable-next=line-too-long
            "Message ID 0: Message\nMessage ID 0: Message\nAll messages with ID 0 have been printed.\n",
            buf.getvalue(),
        )
        network_ok.is_set.assert_has_calls([call(), call(), call()])
        rec_q.get.assert_has_calls(
            [call(block=False), call(block=False), call(block=False)]
        )

    @patch("cantools.database.can.database.Database.get_message_by_frame_id")
    def test_read_can_message_7(self, mock_get_message: MagicMock):
        """Tests the read_can_message method
        in the case that network_ok is set a few times
        and msg_num is 0 after logging the third message."""
        mock_get_message.return_value.decode.return_value = "Message"
        rec_q = MagicMock()
        network_ok = MagicMock()
        msg_id = MagicMock()
        msg_num = MagicMock()
        msg = MagicMock()
        msg.arbitration_id = 0
        msg.timestamp = 1
        msg.data = ""
        rec_q.get.side_effect = [msg, msg, msg]
        rec_q.empty.side_effect = [False, True]
        network_ok.is_set.side_effect = [True, True, False]
        msg_id.value = 0
        msg_num.value = 3
        buf = io.StringIO()
        with redirect_stdout(buf):
            read_can_message(rec_q, network_ok, msg_id, msg_num)
        self.assertEqual(  # pylint: disable-next=line-too-long
            "Message ID 0: Message\nMessage ID 0: Message\nMessage ID 0: Message\nAll messages with ID 0 have been printed.\n",
            buf.getvalue(),
        )
        network_ok.is_set.assert_has_calls([call(), call(), call()])
        rec_q.get.assert_has_calls(
            [call(block=False), call(block=False), call(block=False)]
        )


if __name__ == "__main__":
    unittest.main()
