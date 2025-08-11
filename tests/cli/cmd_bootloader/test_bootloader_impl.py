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

# pylint: disable=too-many-lines

import io
import json
import logging
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, create_autospec, patch

import can
import can.interfaces.pcan
import click
import numpy as np
from can.exceptions import CanInitializationError
from can.interfaces.pcan.pcan import PcanCanInitializationError

try:
    from cli.cmd_bootloader.bootloader import Bootloader, BootloaderStatus
    from cli.cmd_bootloader.bootloader_can import BootloaderInterfaceCan
    from cli.cmd_bootloader.bootloader_impl import (
        _check_bootloader,
        _check_bootloader_status,
        _instantiate_bootloader,
        check_bootloader,
        load_app,
        reset_bootloader,
        run_app,
    )
    from cli.helpers.fcan import CanBusConfig
    from cli.helpers.misc import FOXBMS_BIN_FILE

except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_bootloader.bootloader import Bootloader, BootloaderStatus
    from cli.cmd_bootloader.bootloader_can import BootloaderInterfaceCan
    from cli.cmd_bootloader.bootloader_impl import (
        _check_bootloader,
        _check_bootloader_status,
        _instantiate_bootloader,
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
    @patch.object(Bootloader, "check_target")
    # pylint: disable=too-many-statements
    def test_check_bootloader(self, mock_check_target, mock_load, *_):
        """Function to test function check_bootloader()."""
        mock_load.return_value = PROGRAM

        # Case 1: the input is neither Bootloader nor CanBusConfig
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = check_bootloader(None)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(99, ret)
        self.assertEqual(err, "Invalid bootloader configuration.\n")
        self.assertEqual(out, "")

        # Case 2: the input is Bootloader, the check_target returns 0
        mock_check_target.return_value = (
            0,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            with can.interface.Bus("test", interface="virtual") as can_bus:
                interface = BootloaderInterfaceCan(can_bus=can_bus)
                bd = Bootloader(interface=interface)
                ret = check_bootloader(bd)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(0, ret)
        self.assertEqual(err, "")
        self.assertEqual(
            out, "Checking if the bootloader is online...\nBootloader is running.\n"
        )

        # Case 3: the input is Bootloader, the check_target returns 1
        mock_check_target.return_value = (
            1,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", None),
        )

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            with can.interface.Bus("test", interface="virtual") as can_bus:
                interface = BootloaderInterfaceCan(can_bus=can_bus)
                bd = Bootloader(interface=interface)
                ret = check_bootloader(bd)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(1, ret)
        self.assertEqual(
            err, "Bootloader is running, but some information is missing.\n"
        )
        self.assertEqual(out, "Checking if the bootloader is online...\n")

        # Case 4: the input is Bootloader, the check_target returns 2
        mock_check_target.return_value = (2, BootloaderStatus(None, None, None))

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            with can.interface.Bus("test", interface="virtual") as can_bus:
                interface = BootloaderInterfaceCan(can_bus=can_bus)
                bd = Bootloader(interface=interface)
                ret = check_bootloader(bd)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(2, ret)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            "Checking if the bootloader is online...\n"
            "foxBMS 2 application is running.\n",
        )

        # Case 5: the input is Bootloader, the check_target returns 3
        mock_check_target.return_value = (3, BootloaderStatus(None, None, None))

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            with can.interface.Bus("test", interface="virtual") as can_bus:
                interface = BootloaderInterfaceCan(can_bus=can_bus)
                bd = Bootloader(interface=interface)
                ret = check_bootloader(bd)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(3, ret)
        self.assertEqual(err, "Bootloader is not reachable.\n")
        self.assertEqual(out, "Checking if the bootloader is online...\n")

        # Case 6: the input is Bootloader, the check_target returns 4
        mock_check_target.return_value = (4, (None, None, None))

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            with can.interface.Bus("test", interface="virtual") as can_bus:
                interface = BootloaderInterfaceCan(can_bus=can_bus)
                bd = Bootloader(interface=interface)
                ret = check_bootloader(bd)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(4, ret)
        self.assertEqual(err, "Unknown return value, something went wrong.\n")
        self.assertEqual(out, "Checking if the bootloader is online...\n")

        # Case 7: the input is CanBusConfig, the check_target returns 0
        mock_check_target.return_value = (
            0,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            can_bus_config = CanBusConfig(
                interface="virtual", channel=None, bitrate=None
            )
            ret = check_bootloader(can_bus_config)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(0, ret)
        self.assertEqual(err, "")
        self.assertEqual(
            out, "Checking if the bootloader is online...\nBootloader is running.\n"
        )

        # Case 8: the input is CanBusConfig, the check_target returns 1
        mock_check_target.return_value = (
            1,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", None),
        )

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            can_bus_config = CanBusConfig(
                interface="virtual", channel=None, bitrate=None
            )
            ret = check_bootloader(can_bus_config)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(1, ret)
        self.assertEqual(
            err, "Bootloader is running, but some information is missing.\n"
        )
        self.assertEqual(out, "Checking if the bootloader is online...\n")

        # Case 9: the input is CanBusConfig, the check_target returns 2
        mock_check_target.return_value = (2, BootloaderStatus(None, None, None))

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            can_bus_config = CanBusConfig(
                interface="virtual", channel=None, bitrate=None
            )
            ret = check_bootloader(can_bus_config)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(2, ret)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            "Checking if the bootloader is online...\n"
            "foxBMS 2 application is running.\n",
        )

        # Case 10: the input is CanBusConfig, the check_target returns 3
        mock_check_target.return_value = (3, BootloaderStatus(None, None, None))
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            can_bus_config = CanBusConfig(
                interface="virtual", channel=None, bitrate=None
            )
            ret = check_bootloader(can_bus_config)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(3, ret)
        self.assertEqual(err, "Bootloader is not reachable.\n")
        self.assertEqual(out, "Checking if the bootloader is online...\n")

        # Case 11: the input is CanBusConfig, the check_target returns 4
        mock_check_target.return_value = (4, BootloaderStatus(None, None, None))

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            can_bus_config = CanBusConfig(
                interface="virtual", channel=None, bitrate=None
            )
            ret = check_bootloader(can_bus_config)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(4, ret)
        self.assertEqual(err, "Unknown return value, something went wrong.\n")
        self.assertEqual(out, "Checking if the bootloader is online...\n")

        # Case 12: can not initialize can
        mock_load.reset_mock()
        mock_load.side_effect = PcanCanInitializationError("Initialization error")

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            can_bus_config = CanBusConfig(
                interface="virtual", channel=None, bitrate=None
            )
            ret = check_bootloader(can_bus_config)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(5, ret)
        self.assertEqual(
            err, "Could not initialize CAN bus 'virtual:None':Initialization error\n"
        )
        self.assertEqual(out, "")

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
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = run_app(bus_cfg=can_bus_config)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(1, ret)
        self.assertEqual(err, "")
        self.assertEqual(out, "")

        # Case 2: run app fails
        mock_check_bootloader.return_value = 0
        mock_run_app.return_value = False
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = run_app(bus_cfg=can_bus_config)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(2, ret)
        self.assertEqual(err, "Starting the application not successfully.\n")
        self.assertEqual(out, "Starting the application...\n")

        # Case 3: run app successfully
        mock_check_bootloader.return_value = 0
        mock_run_app.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = run_app(bus_cfg=can_bus_config)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(0, ret)
        self.assertEqual(err, "")
        self.assertEqual(out, "Starting the application...\nApplication is running.\n")

        # Case 4: can not initialize can
        mock_load.reset_mock()
        mock_load.side_effect = PcanCanInitializationError
        mock_check_bootloader.return_value = 0
        mock_run_app.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = run_app(bus_cfg=can_bus_config)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(5, ret)
        self.assertEqual(err, "Could not initialize CAN bus 'virtual:None':\n")
        self.assertEqual(out, "")

    @patch.object(json, "loads", return_value=PROGRAM)
    @patch.object(Bootloader, "check_target")
    @patch.object(Bootloader, "reset_bootloader")
    def test_reset_bootloader(  # pylint: disable=too-many-arguments
        self,
        mock_reset_bootloader,
        mock_check_target,
        mock_load,
        *_,
    ):
        """Function to test function run_app()."""
        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)

        # Case 1: bootloader is online, everything goes well
        mock_check_target.return_value = (
            0,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )
        mock_reset_bootloader.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = reset_bootloader(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(0, ret)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            "Bootloader is online.\n"
            "Resetting bootloader...\n"
            "Successfully resetted bootloader.\n",
        )

        # Case 2: the information of bootloader has not been fully received
        mock_check_target.return_value = (
            1,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", None),
        )
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = reset_bootloader(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(5, ret)
        self.assertEqual(err, "Timeout, abort.\n")
        self.assertEqual(out, "")

        # Case 3: foxBMS application is running
        mock_check_target.return_value = (2, BootloaderStatus(None, None, None))
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = reset_bootloader(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(2, ret)
        self.assertEqual(err, "")
        self.assertEqual(out, "The foxBMS 2 application is running, aborting.\n")

        # Case 4: not registered return value from check_target()
        mock_check_target.return_value = (4, BootloaderStatus(None, None, None))
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = reset_bootloader(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(3, ret)
        self.assertEqual(err, "Unknown check value, aborting.\n")
        self.assertEqual(out, "")

        # Case 5: load app not successfully
        mock_check_target.return_value = (
            0,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )
        mock_reset_bootloader.return_value = False
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = reset_bootloader(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(4, ret)
        self.assertEqual(err, "Resetting bootloader was not successful.\n")
        self.assertEqual(out, "Bootloader is online.\nResetting bootloader...\n")

        # Case 6: timeout
        mock_check_target.return_value = (3, BootloaderStatus(None, None, None))

        # Case 7: can not initialize can
        mock_load.reset_mock()
        mock_load.side_effect = PcanCanInitializationError("foo")
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = reset_bootloader(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(5, ret)
        self.assertEqual(err, "Could not initialize CAN bus 'virtual:None':foo\n")
        self.assertEqual(out, "")

    @patch.object(json, "loads", return_value=PROGRAM)
    @patch.object(click, "progressbar")
    @patch.object(Bootloader, "check_target")
    @patch.object(Bootloader, "send_app_binary")
    # pylint: disable-next=too-many-positional-arguments,too-many-arguments,too-many-statements
    def test_load_app(
        self,
        mock_send_app_binary,
        mock_check_target,
        mock_progressbar,
        mock_load,
        *_,
    ):
        """Function to test function run_app()."""

        mock_progressbar.return_value.__enter__.return_value = MagicMock()

        can_bus_config = CanBusConfig(interface="virtual", channel=None, bitrate=None)

        # Case 1: bootloader is online, everything goes well
        mock_check_target.return_value = (
            0,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )
        mock_send_app_binary.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = load_app(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(0, ret)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            "Bootloader is online.\n"
            "Uploading application to target...\n"
            "Successfully uploaded the application binary to the target, "
            "starting the foxBMS application!\n",
        )

        # Case 2: bootloader is online, everything goes well, the input binary data is none
        mock_check_target.return_value = (
            0,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )
        mock_send_app_binary.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = load_app(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(0, ret)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            "Bootloader is online.\n"
            "Uploading application to target...\n"
            "Successfully uploaded the application binary to the target, "
            "starting the foxBMS application!\n",
        )

        # Case 3: bootloader is online, everything goes well, using foxBMS
        mock_check_target.return_value = (
            0,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )
        mock_send_app_binary.return_value = True
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = load_app(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(0, ret)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            "Bootloader is online.\n"
            "Uploading application to target...\n"
            "Successfully uploaded the application binary to the target, "
            "starting the foxBMS application!\n",
        )

        # Case 4: the information of bootloader has not been fully received
        mock_check_target.return_value = (
            1,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", None),
        )
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = load_app(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(5, ret)
        self.assertEqual(err, "Timeout, abort.\n")
        self.assertEqual(out, "")

        # Case 5: foxBMS application is running
        mock_check_target.return_value = (2, BootloaderStatus(None, None, None))
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = load_app(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(2, ret)
        self.assertEqual(err, "")
        self.assertEqual(out, "The foxBMS 2 application is running, aborting.\n")

        # Case 6: not registered return value from check_target()
        mock_check_target.return_value = (4, BootloaderStatus(None, None, None))
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = load_app(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(3, ret)
        self.assertEqual(err, "Unknown check value, aborting.\n")
        self.assertEqual(out, "")

        # Case 7: reset_bootloader not successfully
        mock_check_target.return_value = (
            0,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )
        mock_send_app_binary.return_value = False
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = load_app(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(4, ret)
        self.assertEqual(
            err,
            "Sending the application binary to the bootloader was not successful.\n",
        )
        self.assertEqual(
            out, "Bootloader is online.\nUploading application to target...\n"
        )

        # Case 8: timeout
        mock_check_target.return_value = (
            1,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", None),
        )
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = load_app(bus_cfg=can_bus_config, timeout=0.01)
        self.assertEqual(5, ret)
        self.assertEqual(
            err,
            "Sending the application binary to the bootloader was not successful.\n",
        )
        self.assertEqual(
            out, "Bootloader is online.\nUploading application to target...\n"
        )

        # Case 9: can not initialize can
        mock_load.reset_mock()
        mock_load.side_effect = PcanCanInitializationError
        with patch("can.interfaces.pcan.PcanBus"):
            _err, _out = io.StringIO(), io.StringIO()
            with redirect_stderr(_err), redirect_stdout(_out):
                ret = load_app(bus_cfg=can_bus_config, timeout=0.01)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(5, ret)
        self.assertEqual(err, "Could not initialize CAN bus 'virtual:None':\n")
        self.assertEqual(out, "")


@patch("cli.cmd_bootloader.bootloader_impl.BootloaderInterfaceCan")
@patch("cli.cmd_bootloader.bootloader_impl.Bootloader")
class TestBootloaderImpl_InstantiateBootloader(unittest.TestCase):  # pylint:disable=invalid-name
    """Test '_instantiate_bootloader' function"""

    def test(self, _: MagicMock, bic: MagicMock):
        """Call function under test"""
        bus = MagicMock()
        ret = _instantiate_bootloader(bus)  # type:ignore
        self.assertIn("Bootloader()", str(ret))
        bic.assert_called_once_with(can_bus=bus)


@patch("cli.cmd_bootloader.bootloader_impl.Bootloader")
@patch("cli.cmd_bootloader.bootloader_impl.time")
class TestBootloaderImpl_CheckBootloaderStatus(unittest.TestCase):  # pylint:disable=invalid-name
    """Test '_check_bootloader_status' function"""

    def test_timeout(self, t: MagicMock, *_: MagicMock):
        """Timeout when checking the bootloader status"""
        t.time.side_effect = [0.0, 21.0]

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = _check_bootloader_status(None)  # type:ignore
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual((5, BootloaderStatus(None, None, None)), ret)
        self.assertEqual(err, "Timeout, abort.\n")
        self.assertEqual(out, "")

    def test_bootloader_online(self, t: MagicMock, bl: MagicMock):
        """Bootloader is online"""
        t.time.side_effect = [0.0, 1.0]
        bl.check_target.return_value = (
            0,
            BootloaderStatus("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = _check_bootloader_status(bl)

        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(
            (0, BootloaderStatus("CanFsmStateNoCommunication", "BootFsmStateWait", 0)),
            ret,
        )
        self.assertEqual(err, "")
        self.assertEqual(out, "Bootloader is online.\n")

    def test_bootloader_wait_for_power_on(self, t: MagicMock, bl: MagicMock):
        """Bootloader is not attached, wait for it to be powered on."""
        t.time.side_effect = [0.0, 1.0, 2.0, 21.0]
        bl.check_target.return_value = (
            3,
            BootloaderStatus(None, None, None),
        )
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = _check_bootloader_status(bl)

        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual((5, BootloaderStatus(None, None, None)), ret)
        self.assertEqual(err, "Timeout, abort.\n")
        self.assertEqual(out, "Waiting for the bootloader to be powered on...\n")

    def test_bootloader_app_running(self, t: MagicMock, bl: MagicMock):
        """Bootloader is online"""
        t.time.side_effect = [0.0, 1.0]
        bl.check_target.return_value = (2, BootloaderStatus(None, None, None))
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = _check_bootloader_status(bl)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual((2, BootloaderStatus(None, None, None)), ret)
        self.assertEqual(err, "")
        self.assertEqual(out, "The foxBMS 2 application is running, aborting.\n")

    def test_bootloader_unknown_state(self, t: MagicMock, bl: MagicMock):
        """Unknown bootloader state"""
        t.time.side_effect = [0.0, 1.0]
        bl.check_target.return_value = (99, BootloaderStatus(None, None, None))
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = _check_bootloader_status(bl)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual((3, BootloaderStatus(None, None, None)), ret)
        self.assertEqual(err, "Unknown check value, aborting.\n")
        self.assertEqual(out, "")

    def test_bootloader_multi_check(self, t: MagicMock, bl: MagicMock):
        """Need a few tries"""
        t.time.side_effect = [0.0, 1.0, 2.0, 3.0, 4.0, 21]
        bl.check_target.return_value = (
            1,
            BootloaderStatus("CanFsmStateNoCommunication", "BootFsmStateWait", None),
        )

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = _check_bootloader_status(bl)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(
            (
                5,
                BootloaderStatus(
                    "CanFsmStateNoCommunication", "BootFsmStateWait", None
                ),
            ),
            ret,
        )
        self.assertEqual(err, "Timeout, abort.\n")
        self.assertEqual(out, "")


@patch("cli.cmd_bootloader.bootloader_impl.Bootloader")
class TestBootloaderImpl_CheckBootloader(unittest.TestCase):  # pylint:disable=invalid-name
    """Testing '_check_bootloader' function."""

    def test__check_bootloader_ok(self, mock_bl: MagicMock):
        """Bootloader is running"""
        mock_bl.check_target.return_value = (
            0,
            BootloaderStatus("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = _check_bootloader(mock_bl)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(0, ret)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            "Checking if the bootloader is online...\nBootloader is running.\n",
        )

    def test__check_bootloader_bl_runs_but_undefined_error(self, mock_bl: MagicMock):
        """Bootloader is running, but something undefined is wrong."""
        mock_bl.check_target.return_value = (
            1,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", None),
        )

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = _check_bootloader(mock_bl)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(1, ret)
        self.assertEqual(
            err, "Bootloader is running, but some information is missing.\n"
        )
        self.assertEqual(out, "Checking if the bootloader is online...\n")

    def test__check_bootloader_app_running(self, mock_bl: MagicMock):
        """Application is running"""
        mock_bl.check_target.return_value = (2, BootloaderStatus(None, None, None))

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = _check_bootloader(mock_bl)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(2, ret)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            "Checking if the bootloader is online...\n"
            "foxBMS 2 application is running.\n",
        )

    def test__check_bootloader_not_reachable(self, mock_bl: MagicMock):
        """Bootloader is not reachable"""
        mock_bl.check_target.return_value = (3, BootloaderStatus(None, None, None))

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = _check_bootloader(mock_bl)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(3, ret)
        self.assertEqual(err, "Bootloader is not reachable.\n")
        self.assertEqual(out, "Checking if the bootloader is online...\n")

    def test__check_bootloader_undefined_error(self, mock_bl: MagicMock):
        """Check unknown return value behavior"""
        mock_bl.check_target.return_value = (12, BootloaderStatus(None, None, None))

        _err, _out = io.StringIO(), io.StringIO()

        with redirect_stderr(_err), redirect_stdout(_out):
            ret = _check_bootloader(mock_bl)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(12, ret)
        self.assertEqual(err, "Unknown return value, something went wrong.\n")
        self.assertEqual(out, "Checking if the bootloader is online...\n")


@patch("cli.cmd_bootloader.bootloader_impl._check_bootloader")
class TestBootloaderImplCheckBootloader(unittest.TestCase):
    """Testing 'check_bootloader' function."""

    def test_check_bootloader_bootloader_object(self, mock_cb: MagicMock):
        """Check case a Bootloader object is passed as argument."""
        mock_cb.return_value = 1
        mock_bl = create_autospec(Bootloader)
        ret = check_bootloader(mock_bl)
        self.assertEqual(1, ret)

    @patch("cli.cmd_bootloader.bootloader_impl.asdict", return_value={})
    @patch("cli.cmd_bootloader.bootloader_impl.can.Bus")
    @patch("cli.cmd_bootloader.bootloader_impl._instantiate_bootloader")
    def test_check_bootloader_can_bus_config_object(
        self,
        _instantiate_bootloader: MagicMock,
        bus: MagicMock,
        _: MagicMock,
        _check_bootloader: MagicMock,
    ):
        """Check case a CanBusConfig object is passed as argument."""
        _check_bootloader.return_value = 1
        bus.return_value.__enter__.return_value = MagicMock()
        _instantiate_bootloader.return_value = MagicMock()
        tmp = create_autospec(CanBusConfig)
        ret = check_bootloader(tmp)
        self.assertEqual(1, ret)

    def test_check_bootloader_invalid_object(self, *_: tuple[MagicMock]):
        """Invalid bootloader configuration."""
        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = check_bootloader(None)  # type: ignore
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(99, ret)
        self.assertEqual(err, "Invalid bootloader configuration.\n")
        self.assertEqual(out, "")


@patch("cli.cmd_bootloader.bootloader_impl.asdict", return_value={})
@patch("cli.cmd_bootloader.bootloader_impl.can.Bus")
@patch("cli.cmd_bootloader.bootloader_impl._instantiate_bootloader")
@patch("cli.cmd_bootloader.bootloader_impl.check_bootloader")
class TestBootloaderImplRunApp(unittest.TestCase):
    """Testing 'run_app' function."""

    def test__check_bootloader_check_fails(
        self,
        m_check_bootloader: MagicMock,
        _instantiate_bootloader: MagicMock,
        bus: MagicMock,
        _: MagicMock,
    ):
        """Run of the app fails."""
        bus.return_value.__enter__.return_value = MagicMock()
        mock_bl = MagicMock()
        mock_bl.run_app.return_value = False
        _instantiate_bootloader.return_value = mock_bl

        m_check_bootloader.return_value = 1

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = run_app(bus_cfg=MagicMock())
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(1, ret)
        self.assertEqual(err, "")
        self.assertEqual(out, "")
        bus.assert_called_once_with()
        _instantiate_bootloader.assert_called_once()
        m_check_bootloader.assert_called_once()
        mock_bl.run_app.assert_not_called()  # pylint: disable=no-member

    def test__check_bootloader_app_successfully_started(
        self,
        m_check_bootloader: MagicMock,
        _instantiate_bootloader: MagicMock,
        bus: MagicMock,
        _: MagicMock,
    ):
        """Start of the application is successful."""
        bus.return_value.__enter__.return_value = MagicMock()
        mock_bl = MagicMock()
        mock_bl.run_app.return_value = True
        _instantiate_bootloader.return_value = mock_bl

        m_check_bootloader.return_value = 0

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = run_app(bus_cfg=MagicMock())
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(0, ret)
        self.assertEqual(err, "")
        self.assertEqual(out, "Starting the application...\nApplication is running.\n")
        bus.assert_called_once_with()
        _instantiate_bootloader.assert_called_once()
        m_check_bootloader.assert_called_once()
        mock_bl.run_app.assert_called_once()  # pylint: disable=no-member

    def test__check_bootloader_app_not_started(
        self,
        m_check_bootloader: MagicMock,
        _instantiate_bootloader: MagicMock,
        bus: MagicMock,
        _: MagicMock,
    ):
        """Start of the application is successful."""
        bus.return_value.__enter__.return_value = MagicMock()
        mock_bl = MagicMock()
        mock_bl.run_app.return_value = False
        _instantiate_bootloader.return_value = mock_bl

        m_check_bootloader.return_value = 0

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = run_app(bus_cfg=MagicMock())
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(2, ret)
        self.assertEqual(err, "Starting the application not successfully.\n")
        self.assertEqual(out, "Starting the application...\n")
        bus.assert_called_once_with()
        _instantiate_bootloader.assert_called_once()
        m_check_bootloader.assert_called_once()
        mock_bl.run_app.assert_called_once()  # pylint: disable=no-member


@patch("cli.cmd_bootloader.bootloader_impl.asdict", return_value={})
@patch("cli.cmd_bootloader.bootloader_impl.can.Bus")
@patch("cli.cmd_bootloader.bootloader_impl._instantiate_bootloader")
@patch("cli.cmd_bootloader.bootloader_impl._check_bootloader_status")
class TestBootloaderImplResetBootloader(unittest.TestCase):
    """Testing 'reset_bootloader' function."""

    def test_reset_bootloader_check_unsuccessful(
        self,
        _check_bootloader_status: MagicMock,
        _instantiate_bootloader: MagicMock,
        bus: MagicMock,
        _: MagicMock,
    ):
        """Status check of the bootloader fails"""
        bus.return_value.__enter__.return_value = MagicMock()
        _instantiate_bootloader.return_value = MagicMock()
        _check_bootloader_status.return_value = (-1, BootloaderStatus(None, None, None))

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = reset_bootloader(bus_cfg=MagicMock(), timeout=MagicMock())
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(-1, ret)
        self.assertEqual(err, "")
        self.assertEqual(out, "")
        bus.assert_called_once_with()
        _instantiate_bootloader.assert_called_once()
        _check_bootloader_status.assert_called_once()

    def test_reset_bootloader_reset_successful(
        self,
        _check_bootloader_status: MagicMock,
        _instantiate_bootloader: MagicMock,
        bus: MagicMock,
        _: MagicMock,
    ):
        """Rest of the bootloader is successful."""
        bus.return_value.__enter__.return_value = MagicMock()
        mock_bl = MagicMock()
        mock_bl.reset_bootloader.return_value = True
        _instantiate_bootloader.return_value = mock_bl

        _check_bootloader_status.return_value = (
            0,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = reset_bootloader(bus_cfg=MagicMock(), timeout=-1)
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(0, ret)
        self.assertEqual(err, "")
        self.assertEqual(
            out, "Resetting bootloader...\nSuccessfully resetted bootloader.\n"
        )
        bus.assert_called_once_with()
        _instantiate_bootloader.assert_called_once()
        _check_bootloader_status.assert_called_once()
        mock_bl.reset_bootloader.assert_called_once()  # pylint: disable=no-member

    def test_reset_bootloader_reset_unsuccessful(
        self,
        _check_bootloader_status: MagicMock,
        _instantiate_bootloader: MagicMock,
        bus: MagicMock,
        _: MagicMock,
    ):
        """Rest of the bootloader is successful."""
        bus.return_value.__enter__.return_value = MagicMock()
        mock_bl = MagicMock()
        mock_bl.reset_bootloader.return_value = False
        _instantiate_bootloader.return_value = mock_bl
        _check_bootloader_status.return_value = (
            0,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = reset_bootloader(bus_cfg=MagicMock(), timeout=MagicMock())
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(4, ret)
        self.assertEqual(err, "Resetting bootloader was not successful.\n")
        self.assertEqual(out, "Resetting bootloader...\n")
        bus.assert_called_once_with()
        _instantiate_bootloader.assert_called_once()
        _check_bootloader_status.assert_called_once()
        mock_bl.reset_bootloader.assert_called_once()  # pylint: disable=no-member


@patch("cli.cmd_bootloader.bootloader_impl.asdict", return_value={})
@patch("cli.cmd_bootloader.bootloader_impl.can.Bus")
@patch("cli.cmd_bootloader.bootloader_impl._instantiate_bootloader")
@patch("cli.cmd_bootloader.bootloader_impl._check_bootloader_status")
class TestBootloaderImplLoadApp(unittest.TestCase):
    """Testing 'load_app' function."""

    def test_bus_initialization_error_base_init_exception(
        self, _1: MagicMock, _2: MagicMock, mock_can_bus: MagicMock, _3: MagicMock
    ):
        """Initialization of the provided CAN bus is not successful."""
        mock_can_bus.return_value.__enter__.side_effect = CanInitializationError("foo")

        mock_can_bus.interface = "foo"
        mock_can_bus.channel = "bar"

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = load_app(bus_cfg=MagicMock(), timeout=MagicMock())
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(ret, 5)
        self.assertRegex(
            err,
            r"Could not initialize CAN bus '.*mock.interface.*:.*mock.channel'.*':foo",
        )
        self.assertEqual(out, "")

    def test_bus_initialization_error_base_init_exception_library_missing(
        self, _1: MagicMock, _2: MagicMock, mock_can_bus: MagicMock, _3: MagicMock
    ):
        """Initialization of the provided CAN bus is not successful."""
        mock_can_bus.return_value.__enter__.side_effect = NameError("foo")

        mock_can_bus.interface = "foo"
        mock_can_bus.channel = "bar"

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = load_app(bus_cfg=MagicMock(), timeout=MagicMock())
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(ret, 5)
        self.assertRegex(
            err,
            r"Could not initialize CAN bus '.*mock.interface.*:.*mock.channel'.*':foo"
            r"\nIs the Kvaser canlib is installed\?\n",
        )
        self.assertEqual(out, "")

    def test_bus_initialization_error_vendor_specific_init_exception(
        self, _1: MagicMock, _2: MagicMock, mock_can_bus: MagicMock, _3: MagicMock
    ):
        """Initialization of the provided CAN bus is not successful."""
        mock_can_bus.return_value.__enter__.side_effect = PcanCanInitializationError(
            "foo"
        )
        mock_can_bus.interface = "foo"
        mock_can_bus.channel = "bar"

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = load_app(bus_cfg=MagicMock(), timeout=MagicMock())
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(ret, 5)
        self.assertRegex(
            err,
            r"Could not initialize CAN bus '.*mock.interface.*:.*mock.channel'.*':foo",
        )
        self.assertEqual(out, "")

    def test_invalid_bootloader_status(
        self,
        _check_bootloader_status: MagicMock,
        _instantiate_bootloader: MagicMock,
        mock_can_bus: MagicMock,
        _: MagicMock,
    ):
        """Catch invalid bootloader status."""
        _instantiate_bootloader.return_value = MagicMock()
        _check_bootloader_status.return_value = (-1, BootloaderStatus(None, None, None))
        mock_can_bus.return_value.__enter__.return_value = MagicMock()

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            ret = load_app(bus_cfg=MagicMock(), timeout=MagicMock())
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(-1, ret)
        self.assertEqual(err, "")
        self.assertEqual(out, "")
        mock_can_bus.assert_called_once_with()
        _instantiate_bootloader.assert_called_once()
        _check_bootloader_status.assert_called_once()

    def test_upload_working(
        self,
        _check_bootloader_status: MagicMock,
        _instantiate_bootloader: MagicMock,
        mock_can_bus: MagicMock,
        _: MagicMock,
    ):
        """Upload of the binary to the target works."""
        _check_bootloader_status.return_value = (
            0,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )

        mock_bl = MagicMock()
        mock_bl.send_app_binary.return_value = True
        _instantiate_bootloader.return_value = mock_bl
        mock_can_bus.return_value.__enter__.return_value = MagicMock()

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            result = load_app(bus_cfg=MagicMock(), timeout=MagicMock())
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(result, 0)
        self.assertEqual(err, "")
        self.assertEqual(
            out,
            "Uploading application to target...\n"
            "Successfully uploaded the application binary to the target, "
            "starting the foxBMS application!\n",
        )
        mock_can_bus.assert_called_once_with()
        _instantiate_bootloader.assert_called_once()
        _check_bootloader_status.assert_called_once()
        mock_bl.send_app_binary.assert_called_once()

    def test_upload_not_working(
        self,
        _check_bootloader_status: MagicMock,
        _instantiate_bootloader: MagicMock,
        mock_can_bus: MagicMock,
        _: MagicMock,
    ):
        """Upload of the binary to the target does not work."""
        _check_bootloader_status.return_value = (
            0,
            ("CanFsmStateNoCommunication", "BootFsmStateWait", 0),
        )
        mock_bl = MagicMock()
        mock_bl.send_app_binary.return_value = False
        _instantiate_bootloader.return_value = mock_bl
        mock_can_bus.return_value.__enter__.return_value = MagicMock()

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            result = load_app(bus_cfg=MagicMock(), timeout=MagicMock())
        err, out = _err.getvalue(), _out.getvalue()

        self.assertEqual(result, 4)
        self.assertEqual(
            err,
            "Sending the application binary to the bootloader was not successful.\n",
        )
        self.assertEqual(out, "Uploading application to target...\n")
        mock_can_bus.assert_called_once_with()
        _instantiate_bootloader.assert_called_once()
        _check_bootloader_status.assert_called_once()
        mock_bl.send_app_binary.assert_called_once()


if __name__ == "__main__":
    unittest.main()
