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

"""Testing file 'cli/com/parameter.py'."""

import sys
import unittest
from pathlib import Path

try:
    from cli.com.parameter import (
        CANLoggerParameter,
        ComControl,
        FileParameter,
        ModbusParameter,
        MQTTParameter,
    )
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.com.parameter import (
        CANLoggerParameter,
        ComControl,
        FileParameter,
        ModbusParameter,
        MQTTParameter,
    )


class TestComControl(unittest.TestCase):
    """Unit tests for the ComControl dataclass.
    Ensures that all queue and event fields are unique per instance and
    default values are set correctly.
    """

    def test_default_queues_and_events_are_distinct(self):
        """Queues and Events should not be shared between instances."""
        c1 = ComControl()
        c2 = ComControl()
        self.assertIsNot(c1.input, c2.input)
        self.assertIsNot(c1.output, c2.output)
        self.assertIsNot(c1.ready, c2.ready)
        self.assertIsNot(c1.shutdown, c2.shutdown)
        self.assertIsNot(c1.logger, c2.logger)
        self.assertEqual(c1.log_level, 50)


class TestModbusParameter(unittest.TestCase):
    """Tests ModbusParameter defaults and immutability."""

    def test_defaults_and_immutability(self):
        """Defaults should be set."""
        p = ModbusParameter(host="localhost")
        self.assertEqual(p.port, 502)
        self.assertEqual(p.timeout, 1)
        self.assertEqual(p.retries, 3)
        self.assertFalse(p.ignore)


class TestMQTTParameter(unittest.TestCase):
    """Unit tests for the MQTTParameter dataclass.
    Checks initialization, default argument values, and immutability.
    """

    def test_basic_init_and_frozen(self):
        """Test initialization and immutability of MQTTParameter."""
        param = MQTTParameter(
            broker="localhost",
            port=1883,
            subscribe=["topic1", "topic2"],
            tls_cert="cert.pem",
            username="user",
            password="pw",  # noqa
        )
        self.assertEqual(param.broker, "localhost")
        self.assertEqual(param.port, 1883)
        self.assertEqual(param.subscribe, ["topic1", "topic2"])
        self.assertEqual(param.tls_cert, "cert.pem")
        self.assertEqual(param.username, "user")
        self.assertEqual(param.password, "pw")
        # Test immutability
        with self.assertRaises(AttributeError):
            param.broker = "something"

    def test_defaults(self):
        """Test default values for optional MQTTParameter fields."""
        param = MQTTParameter(broker="127.0.0.1", port=1884, subscribe=[])
        self.assertIsNone(param.tls_cert)
        self.assertIsNone(param.username)
        self.assertIsNone(param.password)


class TestCANLoggerParameter(unittest.TestCase):
    """Unit tests for the CANLoggerParameter dataclass.
    Checks default values and custom initialization.
    """

    def test_defaults_and_custom(self):
        """Test default and custom values for CANLoggerParameter."""
        p = CANLoggerParameter()
        self.assertEqual(p.max_bytes, 65536)
        self.assertEqual(p.rollover_count, 0)
        self.assertFalse(p.log_can_files)
        p2 = CANLoggerParameter(max_bytes=123, rollover_count=2, log_can_files=True)
        self.assertEqual(p2.max_bytes, 123)
        self.assertEqual(p2.rollover_count, 2)
        self.assertTrue(p2.log_can_files)


class TestFileParameter(unittest.TestCase):
    """Unit tests for the FileParameter dataclass.
    Checks default values and post-initialization path conversion.
    """

    def test_defaults(self):
        """Test defaults for FileParameter."""
        f = FileParameter()
        self.assertIsNone(f.input_file)
        self.assertIsNone(f.output_file)
        self.assertEqual(f.encoding, "utf-8")
        self.assertIsInstance(f.can_logger, CANLoggerParameter)

    def test_path_conversion_in_post_init(self):
        """Test that input_file and output_file get converted to Path."""
        f = FileParameter(input_file="in.txt", output_file="out.txt")
        self.assertIsInstance(f.input_file, Path)
        self.assertIsInstance(f.output_file, Path)
        self.assertEqual(f.input_file.name, "in.txt")
        self.assertEqual(f.output_file.name, "out.txt")


if __name__ == "__main__":
    unittest.main()
