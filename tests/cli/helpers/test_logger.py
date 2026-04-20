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

"""Testing file 'cli/helpers/logger.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import Mock, patch

try:
    import logging  # noqa: TID251
    from logging.handlers import QueueListener  # noqa: TID251
    from queue import Queue

    from cli.helpers import logger
    from cli.helpers.logger import add_queue_handler, get_listener
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    import logging  # noqa: TID251, pylint: disable=C0412
    from logging.handlers import QueueListener  # noqa: TID251
    from queue import Queue  # pylint: disable=C0412

    from cli.helpers import logger
    from cli.helpers.logger import add_queue_handler, get_listener


class TestMisc(unittest.TestCase):
    """Test of 'misc.py'."""

    def test_set_logging_levels(self) -> None:
        """Test logging level setter"""
        logger.set_logging_level(-1)
        logger.set_logging_level(2)
        with self.assertLogs("fox.py", level="DEBUG") as log:
            logger.set_logging_level(10)
        self.assertEqual(["DEBUG:fox.py:Setting logging level to 10"], log.output)

    def test_ignore_third_party_logging(self) -> None:
        """Test ignore specific loggers."""
        logger.ignore_third_party_logging()


class TestAddQueueHandler(unittest.TestCase):
    """Unit tests for the add_queue_handler function."""

    @patch("cli.helpers.logger.logger")
    def test_adds_queue_handler(self, mock_logger: Mock) -> None:
        """Test that add_queue_handler adds a QueueHandler to the patched logger."""
        mock_logger.handlers = []
        mock_logger.addHandler = Mock()
        add_queue_handler(Queue())
        mock_logger.addHandler.assert_called_once()


class TestGetListener(unittest.TestCase):
    """Unit tests for the get_listener function."""

    def test_get_listener_returns_queuelistener(self) -> None:
        """Test that get_listener returns a QueueListener instance."""
        listener = get_listener(Queue())

        self.assertIsInstance(listener, QueueListener)

    def test_listener_check_handler(self) -> None:
        """Test that the listener has the correct handlers."""
        logger.logger.handlers.clear()
        logger.set_logging_level()
        listener = get_listener(Queue())
        self.assertIsInstance(listener.handlers[0], logging.StreamHandler)


if __name__ == "__main__":
    unittest.main()
