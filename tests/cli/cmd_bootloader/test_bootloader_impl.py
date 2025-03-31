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

"""Testing file 'cli/cmd_bootloader/bootloader_impl.py'."""

import json
import logging
import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

import can
import can.interfaces.pcan
import click
import numpy as np
from can.interfaces.pcan import PcanError

try:
    from cli.cmd_bootloader.bootloader import Bootloader
    from cli.cmd_bootloader.bootloader_can import BootloaderInterfaceCan
    from cli.cmd_bootloader.bootloader_impl import (
        check_bootloader,
        load_app,
        reset_bootloader,
        run_app,
    )
    from cli.helpers.fcan import CanBusConfig
    from cli.helpers.misc import FOXBMS_BIN_FILE

except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_bootloader.bootloader import Bootloader
    from cli.cmd_bootloader.bootloader_can import BootloaderInterfaceCan
    from cli.cmd_bootloader.bootloader_impl import (
        check_bootloader,
        load_app,
        reset_bootloader,
        run_app,
    )
    from cli.helpers.fcan import CanBusConfig
    from cli.helpers.misc import FOXBMS_BIN_FILE

PROGRAM = {
    "len_of_program_in_bytes": 16,
    "len_of_program_in_8_bytes": 2,
    "app_size": 262144,
    "bin_file": FOXBMS_BIN_FILE,
    "vector_table_crc": 10755722470602090755,
    "vector_table": [
        16861741595983085566,
        16861496787142180862,
        16933534594249061028,
        16510180501024141744,
    ],
    "hash_app": "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
    "hash_csv": "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389",
}
ARRAY_CSV = np.array(
    [
        [
            "",
            "data_64_bits_hex",
            "crc_64_bits_hex",
            "data_64_bits",
            "crc_64_bits",
        ],
        [
            "0",
            "0x10178797a00021f",
            "0x1b1fccbffe003729",
            "72471532147835423",
            "1954505888274331433",
        ],
        [
            "1",
            "0x3733626331303166",
            "0xd7da68b1dbd05c5a",
            "3977631074064806246",
            "15553859376183794778",
        ],
    ]
)

RETURN_HASH = "1691115346d2814fcf79829becdc0fa096dac126695b7901907fec2e1b11c389"


# pylint: disable=unused-argument
@patch(
    "cli.cmd_bootloader.bootloader_binary_file.get_sha256_file_hash_str",
    return_value=RETURN_HASH,
)
@patch.object(np, "genfromtxt", return_value=ARRAY_CSV)
@patch.object(Path, "read_text", return_value=None)
@patch.object(logging, "info", return_value=None)
@patch.object(logging, "warning", return_value=None)
@patch.object(logging, "error", return_value=None)
class TestBootloaderImpl(unittest.TestCase):
    """Class to test the module bootloader_impl."""

    def setUp(self):
        pass

    def tearDown(self):
        pass

    @patch.object(json, "loads")
    @patch.object(click, "echo")
    @patch.object(Bootloader, "check_target")
    def test_check_bootloader(
        self,
        mock_check_target,
        mock_echo,
        mock_load,
        *_,
    ):
        """Function to test function check_bootloader()."""
        mock_load.return_value = PROGRAM
        mock_echo.return_value = None

        # Case 1: the input is neither Bootloader nor CanBusConfig
        self.assertEqual(99, check_bootloader(None))

        # Case 2: the input is Bootloader, the check_target returns 0
        mock_check_target.return_value = 0
        with can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        ) as can_bus:
            interface = BootloaderInterfaceCan(can_bus=can_bus)
            bd = Bootloader(interface=interface)
            self.assertEqual(0, check_bootloader(bd))

        # Case 3: the input is Bootloader, the check_target returns 1
        mock_check_target.return_value = 1
        with can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        ) as can_bus:
            interface = BootloaderInterfaceCan(can_bus=can_bus)
            bd = Bootloader(interface=interface)
            self.assertEqual(1, check_bootloader(bd))

        # Case 4: the input is Bootloader, the check_target returns 2
        mock_check_target.return_value = 2
        with can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        ) as can_bus:
            interface = BootloaderInterfaceCan(can_bus=can_bus)
            bd = Bootloader(interface=interface)
            self.assertEqual(2, check_bootloader(bd))

        # Case 5: the input is Bootloader, the check_target returns 3
        mock_check_target.return_value = 3
        with can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        ) as can_bus:
            interface = BootloaderInterfaceCan(can_bus=can_bus)
            bd = Bootloader(interface=interface)
            self.assertEqual(3, check_bootloader(bd))

        # Case 6: the input is Bootloader, the check_target returns 4
        mock_check_target.return_value = 4
        with can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        ) as can_bus:
            interface = BootloaderInterfaceCan(can_bus=can_bus)
            bd = Bootloader(interface=interface)
            self.assertEqual(4, check_bootloader(bd))

        # Case 7: the input is CanBusConfig, the check_target returns 0
        mock_check_target.return_value = 0
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        self.assertEqual(0, check_bootloader(can_bus_config))

        # Case 8: the input is CanBusConfig, the check_target returns 1
        mock_check_target.return_value = 1
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        self.assertEqual(1, check_bootloader(can_bus_config))

        # Case 9: the input is CanBusConfig, the check_target returns 2
        mock_check_target.return_value = 2
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        self.assertEqual(2, check_bootloader(can_bus_config))

        # Case 10: the input is CanBusConfig, the check_target returns 3
        mock_check_target.return_value = 3
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        self.assertEqual(3, check_bootloader(can_bus_config))

        # Case 11: the input is CanBusConfig, the check_target returns 4
        mock_check_target.return_value = 4
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        self.assertEqual(4, check_bootloader(can_bus_config))

        # Case 12: no binary file
        mock_load.reset_mock()
        mock_load.side_effect = FileNotFoundError
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        self.assertEqual(5, check_bootloader(can_bus_config))

        # Case 13: can not initialize can
        mock_load.reset_mock()
        mock_load.side_effect = PcanError
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        self.assertEqual(6, check_bootloader(can_bus_config))

        # Case 14: random other error
        mock_load.reset_mock()
        mock_load.side_effect = Exception
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        self.assertEqual(7, check_bootloader(can_bus_config))

    @patch.object(json, "loads", return_value=PROGRAM)
    @patch.object(click, "echo")
    @patch.object(Bootloader, "run_app")
    @patch("cli.cmd_bootloader.bootloader_impl.check_bootloader")
    def test_run_app(
        self,
        mock_check_bootloader,
        mock_run_app,
        mock_echo,
        mock_load,
        *_,
    ):
        """Function to test function run_app()."""
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)
        mock_echo.return_value = None

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

        # Case 4: no binary file
        mock_load.reset_mock()
        mock_load.side_effect = FileNotFoundError
        mock_check_bootloader.return_value = 0
        mock_run_app.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(3, run_app(can_bus_config))

        # Case 5: can not initialize can
        mock_load.reset_mock()
        mock_load.side_effect = PcanError
        mock_check_bootloader.return_value = 0
        mock_run_app.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(4, run_app(can_bus_config))

        # Case 6: random other error
        mock_load.reset_mock()
        mock_load.side_effect = Exception
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(5, run_app(can_bus_config))

    @patch.object(json, "loads", return_value=PROGRAM)
    @patch.object(click, "echo")
    @patch.object(Bootloader, "check_target")
    @patch.object(Bootloader, "reset_bootloader")
    def test_reset_bootloader(  # pylint: disable=too-many-arguments
        self,
        mock_reset_bootloader,
        mock_check_target,
        mock_echo,
        mock_load,
        *_,
    ):
        """Function to test function run_app()."""
        mock_echo.return_value = None
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)

        # Case 1: bootloader is online, everything goes well
        mock_check_target.return_value = 0
        mock_reset_bootloader.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(0, reset_bootloader(can_bus_config, timeout=0.01))

        # Case 2: the information of bootloader has not been fully received
        mock_check_target.return_value = 1
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(5, reset_bootloader(can_bus_config, timeout=0.01))

        # Case 3: foxBMS application is running
        mock_check_target.return_value = 2
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(2, reset_bootloader(can_bus_config, timeout=0.01))

        # Case 4: not registered return value from check_target()
        mock_check_target.return_value = 4
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(3, reset_bootloader(can_bus_config, timeout=0.01))

        # Case 5: load app not successfully
        mock_check_target.return_value = 0
        mock_reset_bootloader.return_value = False
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(4, reset_bootloader(can_bus_config, timeout=0.01))

        # Case 6: timeout
        mock_check_target.return_value = 3

        # Case 7: no binary file
        mock_load.reset_mock()
        mock_load.side_effect = FileNotFoundError
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(6, reset_bootloader(can_bus_config, timeout=0.01))

        # Case 8: can not initialize can
        mock_load.reset_mock()
        mock_load.side_effect = PcanError
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(7, reset_bootloader(can_bus_config, timeout=0.01))

        # Case 9: random other error
        mock_load.reset_mock()
        mock_load.side_effect = Exception
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(8, reset_bootloader(can_bus_config, timeout=0.01))

    @patch.object(json, "loads", return_value=PROGRAM)
    @patch.object(click, "progressbar")
    @patch.object(click, "echo")
    @patch.object(Bootloader, "check_target")
    @patch.object(Bootloader, "send_app_binary")
    def test_load_app(  # pylint: disable=too-many-positional-arguments,too-many-arguments
        self,
        mock_send_app_binary,
        mock_check_target,
        mock_echo,
        mock_progressbar,
        mock_load,
        *_,
    ):
        """Function to test function run_app()."""

        mock_progressbar.return_value.__enter__.return_value = MagicMock()

        mock_echo.return_value = None
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)

        # Case 1: bootloader is online, everything goes well
        mock_check_target.return_value = 0
        mock_send_app_binary.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(0, load_app(can_bus_config, timeout=0.01))

        # Case 2: bootloader is online, everything goes well, the input binary data is none
        mock_check_target.return_value = 0
        mock_send_app_binary.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(0, load_app(can_bus_config, timeout=0.01))

        # Case 3: bootloader is online, everything goes well, using FOXBMS
        mock_check_target.return_value = 0
        mock_send_app_binary.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(0, load_app(can_bus_config, timeout=0.01))

        # Case 4: the information of bootloader has not been fully received
        mock_check_target.return_value = 1
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(5, load_app(can_bus_config, timeout=0.01))

        # Case 5: foxBMS application is running
        mock_check_target.return_value = 2
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(2, load_app(can_bus_config, timeout=0.01))

        # Case 6: not registered return value from check_target()
        mock_check_target.return_value = 4
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(3, load_app(can_bus_config, timeout=0.01))

        # Case 7: reset_bootloader not successfully
        mock_check_target.return_value = 0
        mock_send_app_binary.return_value = False
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(4, load_app(can_bus_config, timeout=0.01))

        # Case 8: timeout
        mock_check_target.return_value = 3
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(5, load_app(can_bus_config, timeout=0.01))

        # Case 9: no binary file
        mock_load.reset_mock()
        mock_load.side_effect = FileNotFoundError
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(6, load_app(can_bus_config, timeout=0.01))

        # Case 10: can not initialize can
        mock_load.reset_mock()
        mock_load.side_effect = PcanError
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(7, load_app(can_bus_config, timeout=0.01))

        # Case 11: random other error
        mock_load.reset_mock()
        mock_load.side_effect = Exception
        with patch("can.interfaces.pcan.PcanBus"):
            self.assertEqual(8, load_app(can_bus_config, timeout=0.01))


if __name__ == "__main__":
    unittest.main()
