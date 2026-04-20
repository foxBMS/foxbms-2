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

"""Testing file 'cli/com/com_interface.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, Mock, patch

try:
    from cli.com.com_interface import ComInterface, ProcessInterface
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.com.com_interface import ComInterface, ProcessInterface


class TestInit(unittest.TestCase):
    """Unit tests for the initialization of the ComInterface class.
    Ensures that all attributes are set correctly during construction.
    """

    @patch("cli.com.com_interface.logger")
    @patch("cli.com.com_interface.get_listener")
    @patch("cli.com.com_interface.ComControl")
    def test_init(self, _: Mock, mock_get_listener: Mock, mock_logger: Mock) -> None:
        """Test that ComInterface initializes all attributes as expected.

        Args:
            mock_get_listener (Mock): Mocked get_listener function.
            mock_logger (Mock): Mocked logger.
        """
        mock_logger.level = 10
        mock_get_listener.return_value = MagicMock()
        ci = ComInterface("test")
        self.assertEqual(ci.name, "test")
        self.assertIsNotNone(ci.control)
        # _processes should be initialized as an empty dict
        self.assertEqual(ci._processes, {})  # pylint: disable=W0212
        self.assertIsNotNone(ci.log_listener)


@patch("cli.com.com_interface.logger")
class TestStart(unittest.TestCase):
    """Unit tests for the start() method of ComInterface.
    Covers process starting logic and error handling.
    """

    @patch("cli.com.com_interface.recho")
    def test_start_no_processes(self, mock_recho: Mock, _: Mock) -> None:
        """Test that a message is echoed when no processes are available.

        Args:
            mock_recho (Mock): Mocked recho function.
        """
        ci = ComInterface("test")
        ci.log_listener = MagicMock()
        # _processes is empty
        ci._processes = {}  # pylint: disable=W0212
        ci.start()
        mock_recho.assert_called_with("No process available")

    def test_start_process_ready(self, _: Mock) -> None:
        """Test starting a process and waiting for it to become ready."""
        ci = ComInterface("test")
        process = MagicMock()
        # Simulate process readiness in two steps
        process.control.ready.is_set.side_effect = [True, False, True]
        process.control.shutdown.is_set.return_value = False
        ci.log_listener = MagicMock()
        ci._processes = {"proc_1": process}  # pylint: disable=W0212
        with patch("cli.com.com_interface.time") as mock_sleep:
            ci.start()
            mock_sleep.sleep.assert_called_once_with(0.1)
        process.start.assert_called_once()
        process.control.ready.clear.assert_called_once()

    def test_start_process_shutdown(self, _: Mock) -> None:
        """Test that ChildProcessError is raised if a process sets shutdown during startup."""
        ci = ComInterface("test")
        process = MagicMock()
        # ready never becomes True, shutdown is set
        process.control.ready.is_set.side_effect = [False, False]
        process.control.shutdown.is_set.return_value = True
        ci.log_listener = MagicMock()
        ci._processes = {"proc": process}  # pylint: disable=W0212
        with self.assertRaises(ChildProcessError):
            ci.start()


@patch("cli.com.com_interface.time")
class TestShutdown(unittest.TestCase):
    """Unit tests for the shutdown() method of ComInterface.
    Tests both blocking and non-blocking shutdowns, including timeouts.
    """

    def test_shutdown_non_blocking(self, _: Mock) -> None:
        """Test that shutdown sets the shutdown event in non-blocking mode."""
        ci = ComInterface("test")
        ci.is_alive = MagicMock()
        ci.is_alive.return_value = False
        ci.control.shutdown = MagicMock()
        ci.shutdown(block=False)
        ci.control.shutdown.set.assert_called_once()

    def test_shutdown_blocking(self, mock_time: MagicMock) -> None:
        """Test that shutdown blocks until all processes have terminated.

        Args:
            mock_time (MagicMock): Mocked time module.
        """
        ci = ComInterface("test")
        ci.is_alive = MagicMock(side_effect=[True, False])
        ci.control.shutdown = MagicMock()
        mock_time.time.side_effect = [0, 1]
        ci.shutdown(block=True, timeout=2)
        # is_alive should be called twice: [True, False]
        self.assertEqual(ci.is_alive.call_count, 2)

    def test_shutdown_blocking_timeout(self, mock_time: Mock) -> None:
        """Test that shutdown returns after timeout if processes are still alive.

        Args:
            mock_time (Mock): Mocked time module.
        """
        ci = ComInterface("test")
        ci.is_alive = MagicMock(return_value=True)
        ci.control.shutdown = MagicMock()
        # Simulate timeout expiration
        mock_time.time.side_effect = [0, 5]
        ci.shutdown(block=True, timeout=2)
        self.assertTrue(ci.control.shutdown.set.called)


class TestIsAlive(unittest.TestCase):
    """Unit tests for the is_alive() method of ComInterface.
    Checks correct reporting of process liveness.
    """

    def test_is_alive_no_processes(self) -> None:
        """Test that is_alive returns False if there are no processes."""
        ci = ComInterface("test")
        ci._processes = {}  # pylint: disable=W0212
        self.assertFalse(ci.is_alive())

    def test_is_alive_all(self) -> None:
        """Test that is_alive returns True only if all processes are alive."""
        ci = ComInterface("test")
        p1 = MagicMock()
        p2 = MagicMock()
        p1.is_alive.return_value = True
        p2.is_alive.return_value = True
        ci._processes = {"p1": p1, "p2": p2}  # pylint: disable=W0212
        self.assertTrue(ci.is_alive())
        p2.is_alive.return_value = False
        self.assertFalse(ci.is_alive())

    def test_is_alive_specific(self) -> None:
        """Test that is_alive returns correct status for a specific process."""
        ci = ComInterface("test")
        p1 = MagicMock()
        p1.is_alive.return_value = True
        ci._processes = {"p1": p1}  # pylint: disable=W0212
        self.assertTrue(ci.is_alive("p1"))
        p1.is_alive.return_value = False
        self.assertFalse(ci.is_alive("p1"))


class TestProcessInterface(unittest.TestCase):
    """Unit test for the shutdown() method of the ProcessInterface base class."""

    def test_shutdown(self) -> None:
        """Test that shutdown sets the shutdown event in the process control."""
        pi = MagicMock(spec=ProcessInterface)
        pi.control = MagicMock()
        ProcessInterface.shutdown(pi)
        pi.control.shutdown.set.assert_called_once()


if __name__ == "__main__":
    unittest.main()
