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

"""Testing file 'cli/cmd_log/log_impl.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import MagicMock, patch

from can import CanInitializationError, CanOperationError

try:
    from cli.cmd_log.log_impl import log, log_can_message, receive_can_message
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_log.log_impl import log, log_can_message, receive_can_message


class TestLogImpl(unittest.TestCase):
    """Test of the 'log' implementation."""

    def setUp(self) -> None:  # noqa: D102
        self.max_gets = 5
        self.max_sets = 4

    def tearDown(self) -> None:  # noqa: D102
        self.max_gets = 5
        self.max_sets = 4

    @patch("cli.cmd_log.log_impl.asdict")
    @patch("cli.cmd_log.log_impl.Bus")
    def test_receive_can_message_initialization_error(
        self, m_bus: MagicMock, _: MagicMock
    ) -> None:
        """Test an initialization error of the CAN bus."""
        m_bus.side_effect = CanInitializationError
        err = io.StringIO()
        with redirect_stderr(err):
            receive_can_message(MagicMock(), m_bus, MagicMock())
        self.assertEqual("Could not initialize CAN bus.\n", err.getvalue())

    @patch("cli.cmd_log.log_impl.asdict")
    @patch("cli.cmd_log.log_impl.Bus")
    def test_receive_can_message_stop_logging_0(
        self, m_bus: MagicMock, _: MagicMock
    ) -> None:
        """Test that the CAN bus is stopped on a stop event and the case that
        'recv' method of the CAN bus is empty
        """
        data = MagicMock()
        network_ok = MagicMock()
        # no message -> recv is False
        # stop immediately
        m_bus.return_value.__enter__.return_value.recv.return_value = None
        network_ok.set.return_value = None
        network_ok.is_set.return_value = False  # raise StopLogging
        buf = io.StringIO()
        with redirect_stdout(buf):
            receive_can_message(data, m_bus, network_ok)
        self.assertEqual("Stop receiving messages.\n", buf.getvalue())

    @patch("cli.cmd_log.log_impl.asdict")
    @patch("cli.cmd_log.log_impl.Bus")
    def test_receive_can_message_stop_logging_1(
        self, m_bus: MagicMock, _: MagicMock
    ) -> None:
        """Test that the CAN bus is stopped on a stop event and the case that
        'recv' method of the CAN bus receives a few messages.
        """

        def is_set() -> bool:
            self.max_sets -= 1
            return not self.max_sets <= 0

        data = MagicMock()
        network_ok = MagicMock()
        # message -> recv is True
        # stop after a few runs
        m_bus.return_value.__enter__.return_value.recv.return_value = True
        network_ok.set.return_value = None
        network_ok.is_set = is_set  # raise StopLogging after n events

        buf = io.StringIO()
        with redirect_stdout(buf):
            receive_can_message(data, m_bus, network_ok)
        self.assertEqual("Stop receiving messages.\n", buf.getvalue())

    @patch("cli.cmd_log.log_impl.asdict")
    @patch("cli.cmd_log.log_impl.Bus")
    def test_receive_can_message_too_many_errors(
        self, m_bus: MagicMock, _: MagicMock
    ) -> None:
        """Too many errors shall create also a stop"""
        data = MagicMock()
        network_ok = MagicMock()
        # recv too often unsuccessful --> raise CanOperationError
        m_bus.return_value.__enter__.return_value.recv.side_effect = CanOperationError

        out, err = io.StringIO(), io.StringIO()
        with redirect_stdout(out), redirect_stderr(err):
            receive_can_message(data, m_bus, network_ok)
        self.assertEqual("Stop receiving messages.\n", out.getvalue())
        self.assertEqual(
            "Too many errors occurred while receiving messages.\n", err.getvalue()
        )

    def test_log_can_message(self) -> None:
        """Test the 'log_can_message' function"""
        data = MagicMock()
        network_ok = MagicMock()
        network_ok.is_set.return_value = False
        buf = io.StringIO()
        with redirect_stdout(buf):
            log_can_message(data, network_ok, logger=MagicMock())
        self.assertEqual("Start Logging\n", buf.getvalue())

        def dummy_get(**_: object) -> bool | MagicMock:
            self.max_gets -= 1

            if self.max_gets <= 0:
                return False
            return MagicMock()

        def dummy_is_set() -> bool:
            self.max_sets -= 1
            return not self.max_sets <= 0

        data.get = dummy_get
        network_ok.reset_mock()
        network_ok.is_set = dummy_is_set
        buf = io.StringIO()
        with redirect_stdout(buf):
            log_can_message(data, network_ok, logger=MagicMock())
        self.assertEqual("Start Logging\n", buf.getvalue())

        # same test, but not messages at all, i.e., skip the if branch
        self.max_gets = 0
        self.max_sets = 5
        buf = io.StringIO()
        with redirect_stdout(buf):
            log_can_message(data, network_ok, logger=MagicMock())
        self.assertEqual("Start Logging\n", buf.getvalue())

        # same test, but less messages than is_set, i.e., skip the if branch
        self.max_gets = 3
        self.max_sets = 5
        buf = io.StringIO()
        with redirect_stdout(buf):
            log_can_message(data, network_ok, logger=MagicMock())
        self.assertEqual("Start Logging\n", buf.getvalue())

    @patch("cli.cmd_log.log_impl.Queue")
    @patch("cli.cmd_log.log_impl.Process")
    def test_log_failure_0(
        self, mock_process: MagicMock, mock_queue: MagicMock
    ) -> None:
        """Creating the receive process failed"""
        # test RECEIVE_PROCESS_NOT_STARTED
        mock_process = mock_process.return_value
        mock_process.start.side_effect = RuntimeError
        mock_queue.return_value = MagicMock()
        buf = io.StringIO()
        with redirect_stderr(buf):
            ret = log(MagicMock(), Path("mock"))
        self.assertEqual(
            "Could not start receive process.\nExiting...\n", buf.getvalue()
        )
        self.assertEqual(ret, 1)
        mock_queue.return_value.close.assert_called_once()
        mock_queue.return_value.join_thread.assert_called_once()

    @patch("cli.cmd_log.log_impl.Queue")
    @patch("cli.cmd_log.log_impl.Event")
    @patch("cli.cmd_log.log_impl.Process")
    def test_log_failure_1(
        self,
        mock_process: MagicMock,
        mock_event: MagicMock,
        mock_queue: MagicMock,
    ) -> None:
        """Initializing the CAN bus failed"""
        # test CAN_BUS_INITIALIZATION_FAILED
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        mock_instance_event = mock_event.return_value
        mock_instance_event.wait.return_value = False
        mock_queue.return_value = MagicMock()
        buf = io.StringIO()
        with redirect_stderr(buf), TemporaryDirectory() as tmpdir:
            ret = log(MagicMock(), Path(tmpdir))
        self.assertEqual(
            "Could not initialize CAN bus. Timeout\nShutdown...\n", buf.getvalue()
        )
        self.assertEqual(ret, 2)
        mock_queue.return_value.close.assert_called_once()
        mock_queue.return_value.join_thread.assert_called_once()

    @patch("cli.cmd_log.log_impl.Queue")
    @patch("cli.cmd_log.log_impl.SizedRotatingLogger")
    @patch("cli.cmd_log.log_impl.Event")
    @patch("cli.cmd_log.log_impl.Process")
    def test_log_failure_2(
        self,
        mock_process: MagicMock,
        mock_event: MagicMock,
        mock_srl: MagicMock,
        mock_queue: MagicMock,
    ) -> None:
        """Instantiating the logger object failed"""
        # test LOGGER_NOT_STARTED
        mock_instance_process = mock_process.return_value
        mock_queue.return_value = MagicMock()
        mock_instance_process.start.return_value = 0
        mock_instance_process.join.return_value = 0
        mock_instance_process.is_alive.return_value = 0
        mock_instance_event = mock_event.return_value
        mock_instance_event.wait.return_value = True
        mock_srl.side_effect = ValueError
        buf = io.StringIO()
        with redirect_stderr(buf), TemporaryDirectory() as tmpdir:
            ret = log(MagicMock(), Path(tmpdir))
        self.assertEqual(
            "Could not create logger object.\nShutdown...\n", buf.getvalue()
        )
        self.assertEqual(ret, 3)
        mock_instance_process = mock_process.return_value
        mock_instance_process.start.return_value = 0
        mock_instance_process.join.return_value = 0
        mock_instance_process.terminate.return_value = 0
        mock_instance_process.is_alive.return_value = True
        mock_instance_event = mock_event.return_value
        mock_instance_event.wait.return_value = True
        mock_srl.side_effect = ValueError
        buf = io.StringIO()
        with redirect_stderr(buf), TemporaryDirectory() as tmpdir:
            ret = log(MagicMock(), Path(tmpdir))
        self.assertEqual(
            "Could not create logger object.\n"
            "Could could not cancel the receive process gracefully.\n"
            "Terminating...\n"
            "Shutdown...\n",
            buf.getvalue(),
        )
        self.assertEqual(ret, 3)
        self.assertEqual(mock_queue.return_value.close.call_count, 2)
        self.assertEqual(mock_queue.return_value.join_thread.call_count, 2)

    @patch("cli.cmd_log.log_impl.Queue")
    @patch("cli.cmd_log.log_impl.sleep")
    @patch("cli.cmd_log.log_impl.log_can_message")
    @patch("cli.cmd_log.log_impl.SizedRotatingLogger")
    @patch("cli.cmd_log.log_impl.Event")
    @patch("cli.cmd_log.log_impl.Process")
    # pylint: disable-next=too-many-arguments,too-many-positional-arguments
    def test_log_success_0(  # noqa: PLR0913
        self,
        mock_process: MagicMock,
        mock_event: MagicMock,
        mock_srl: MagicMock,
        mock_log_can_message: MagicMock,
        mock_sleep: MagicMock,
        mock_queue: MagicMock,
    ) -> None:
        """Test successfully logging"""
        # receive process got gracefully canceled
        mock_instance_process = mock_process.return_value
        mock_queue.return_value = MagicMock()
        mock_instance_process.start.return_value = 0
        mock_instance_process.join.return_value = 0
        mock_instance_process.is_alive.return_value = 0
        mock_instance_event = mock_event.return_value
        mock_instance_event.clear.return_value = 0  # don't care
        mock_instance_event.wait.return_value = True
        mock_srl.return_value = 0
        mock_log_can_message.side_effect = KeyboardInterrupt
        mock_sleep.return_value = 0
        buf = io.StringIO()
        with redirect_stdout(buf), TemporaryDirectory() as tmpdir:
            ret = log(MagicMock(), Path(tmpdir))
        self.assertEqual("Use Ctrl+C to stop logging.\nShutdown...\n", buf.getvalue())
        self.assertEqual(ret, 0)
        mock_queue.return_value.close.assert_called_once()
        mock_queue.return_value.join_thread.assert_called_once()

    @patch("cli.cmd_log.log_impl.Queue")
    @patch("cli.cmd_log.log_impl.sleep")
    @patch("cli.cmd_log.log_impl.log_can_message")
    @patch("cli.cmd_log.log_impl.SizedRotatingLogger")
    @patch("cli.cmd_log.log_impl.Event")
    @patch("cli.cmd_log.log_impl.Process")
    # pylint: disable-next=too-many-arguments,too-many-positional-arguments
    def test_log_success_1(  # noqa: PLR0913
        self,
        mock_process: MagicMock,
        mock_event: MagicMock,
        mock_srl: MagicMock,
        mock_log_can_message: MagicMock,
        mock_sleep: MagicMock,
        mock_queue: MagicMock,
    ) -> None:
        """Test another branch when successfully logging"""
        # receive process got gracefully canceled
        mock_instance_process = mock_process.return_value
        mock_queue.return_value = MagicMock()
        mock_instance_process.start.return_value = 0
        mock_instance_process.join.return_value = 0
        mock_instance_process.is_alive.return_value = True
        mock_instance_event = mock_event.return_value
        mock_instance_event.clear.return_value = 0  # don't care
        mock_instance_event.wait.return_value = True
        mock_srl.return_value = 0
        mock_log_can_message.side_effect = KeyboardInterrupt
        mock_sleep.return_value = 0
        out = io.StringIO()
        err = io.StringIO()
        with redirect_stdout(out), redirect_stderr(err), TemporaryDirectory() as tmpdir:
            ret = log(MagicMock(), Path(tmpdir))
        self.assertEqual("Use Ctrl+C to stop logging.\nShutdown...\n", out.getvalue())
        self.assertEqual(
            "Could could not cancel the receive process gracefully.\nTerminating...\n",
            err.getvalue(),
        )
        self.assertEqual(ret, 0)
        mock_queue.return_value.close.assert_called_once()
        mock_queue.return_value.join_thread.assert_called_once()

    @patch("cli.cmd_log.log_impl.Queue")
    @patch("cli.cmd_log.log_impl.Process")
    def test_log_failure_0_ignores_queue_cleanup_errors(
        self, mock_process: MagicMock, mock_queue: MagicMock
    ) -> None:
        """Queue cleanup errors should be ignored on early exit."""
        mock_process.return_value.start.side_effect = RuntimeError
        queue = MagicMock()
        queue.close.side_effect = AttributeError()
        queue.join_thread.side_effect = RuntimeError()
        mock_queue.return_value = queue

        with redirect_stderr(io.StringIO()):
            ret = log(MagicMock(), Path("mock"))

        self.assertEqual(ret, 1)
        queue.close.assert_called_once()
        queue.join_thread.assert_called_once()


if __name__ == "__main__":
    unittest.main()
