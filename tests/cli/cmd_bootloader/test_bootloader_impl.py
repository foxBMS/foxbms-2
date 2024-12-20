#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Testing file 'cli/cmd_bootloader/bootloader_impl.py'."""

import io
import sys
import unittest
from pathlib import Path
from unittest.mock import patch

import can
import can.interfaces.pcan

# Redirect message or not
MSG_REDIRECT = True

try:
    from cli.cmd_bootloader.bootloader import Bootloader, CanBusConfig
    from cli.cmd_bootloader.bootloader_can import BootloaderInterfaceCan
    from cli.cmd_bootloader.bootloader_impl import (
        check_bootloader,
        load_app,
        reset_bootloader,
        run_app,
    )

except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))
    from cli.cmd_bootloader.bootloader import Bootloader, CanBusConfig
    from cli.cmd_bootloader.bootloader_can import BootloaderInterfaceCan
    from cli.cmd_bootloader.bootloader_impl import (
        check_bootloader,
        load_app,
        reset_bootloader,
        run_app,
    )


class TestBootloaderImpl(unittest.TestCase):
    """Class to test the module bootloader_impl."""

    def setUp(self):
        # Redirect the sys.stdout to the StringIO object
        if MSG_REDIRECT:
            sys.stdout = io.StringIO()

    def tearDown(self):
        # Reset sys.stdout to its original value
        sys.stdout = sys.__stdout__

    @patch.object(Bootloader, "check_target")
    def test_check_bootloader(self, mock_check_target):
        """Function to test function check_bootloader()."""

        # Case 1: the input is neither Bootloader nor CanBusConfig
        self.assertEqual(99, check_bootloader(None))

        # Case 2: the input is Bootloader, the check_target returns True
        mock_check_target.return_value = True
        with can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        ) as can_bus:
            interface = BootloaderInterfaceCan(can_bus=can_bus)
            bd = Bootloader(interface=interface, path_app_binary=None)
            self.assertEqual(0, check_bootloader(bd))

        # Case 3: the input is Bootloader, the check_target returns False
        mock_check_target.return_value = False
        with can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        ) as can_bus:
            interface = BootloaderInterfaceCan(can_bus=can_bus)
            bd = Bootloader(interface=interface, path_app_binary=None)
            self.assertEqual(2, check_bootloader(bd))

        # Case 4: the input is CanBusConfig, the check_target returns True
        mock_check_target.return_value = True
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        self.assertEqual(0, check_bootloader(can_bus_config))

        # Case 5: the input is CanBusConfig, the check_target returns False
        mock_check_target.return_value = False
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        self.assertEqual(1, check_bootloader(can_bus_config))

    @patch.object(Bootloader, "run_app")
    @patch("cli.cmd_bootloader.bootloader_impl.check_bootloader")
    def test_run_app(self, mock_check_bootloader, mock_run_app):
        """Function to test function run_app()."""
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        # Case 1: cannot get the status of bootloader
        mock_check_bootloader.return_value = 1
        mock_run_app.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(1, run_app(can_bus_config))
        # Case 2: run app fails
        mock_check_bootloader.return_value = 0
        mock_run_app.return_value = False
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(2, run_app(can_bus_config))
        # Case 3: run app successfully
        mock_check_bootloader.return_value = 0
        mock_run_app.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(0, run_app(can_bus_config))

    @patch.object(Bootloader, "reset_bootloader")
    @patch("cli.cmd_bootloader.bootloader_impl.check_bootloader")
    def test_reset_bootloader(self, mock_check_bootloader, mock_reset_bootloader):
        """Function to test function run_app()."""
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        # Case 1: cannot get the status of bootloader
        mock_check_bootloader.return_value = 1
        mock_reset_bootloader.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(1, reset_bootloader(can_bus_config))
        # Case 2: run app fails
        mock_check_bootloader.return_value = 0
        mock_reset_bootloader.return_value = False
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(2, reset_bootloader(can_bus_config))
        # Case 3: run app successfully
        mock_check_bootloader.return_value = 0
        mock_reset_bootloader.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(0, reset_bootloader(can_bus_config))

    @patch("cli.cmd_bootloader.bootloader_impl.FOXBMS_BIN_FILE", None)
    @patch.object(Bootloader, "send_app_binary")
    @patch("cli.cmd_bootloader.bootloader_impl.check_bootloader")
    def test_load_app(self, mock_check_bootloader, mock_send_app_binary):
        """Function to test function run_app()."""
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        # Case 1: cannot get the status of bootloader
        mock_check_bootloader.return_value = 1
        mock_send_app_binary.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(1, load_app(can_bus_config, None))
        # Case 2: run app fails
        mock_check_bootloader.return_value = 0
        mock_send_app_binary.return_value = False
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(2, load_app(can_bus_config, None))
        # Case 3: run app successfully
        mock_check_bootloader.return_value = 0
        mock_send_app_binary.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(0, load_app(can_bus_config, None))


if __name__ == "__main__":
    unittest.main()
