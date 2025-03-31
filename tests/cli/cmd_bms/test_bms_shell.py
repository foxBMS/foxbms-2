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

"""Testing file 'cli/cmd_bms/bms_shell.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_bms.bms_shell import BMSShell, run_shell
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_bms.bms_shell import BMSShell, run_shell


class TestBmsShell(unittest.TestCase):  # pylint: disable=too-many-public-methods
    """Class to test the BMSShell class"""

    @patch("cmd.Cmd.cmdloop")
    def test_run_shell(self, mock_cmdloop: MagicMock):
        """Tests the method run_shell"""
        mock_cmdloop.side_effect = KeyboardInterrupt
        out = io.StringIO()
        err = io.StringIO()

        with redirect_stderr(err), redirect_stdout(out):
            run_shell(MagicMock())
        self.assertEqual("Exiting...\nThe shell has been stopped. \n", out.getvalue())
        self.assertEqual(
            "Error detected. Please wait for the Process to terminate.\n",
            err.getvalue(),
        )

    def test_init_0(self):
        """Shell has already been initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            ret = shell.do_init(None)
        self.assertEqual("The CAN bus has already been initialized.\n", buf.getvalue())
        self.assertEqual(ret, False)

    @patch("cli.cmd_bms.bms_shell.initialization")
    def test_init_1(self, mock_initialization: MagicMock):
        """Initializing the CAN bus failed."""
        shell = BMSShell()
        shell.bus_cfg = MagicMock()
        mock_initialization.return_value = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            ret = shell.do_init(None)
        self.assertEqual("", buf.getvalue())
        self.assertTrue(ret)

    @patch("cli.cmd_bms.bms_shell.initialization")
    def test_init_2(self, mock_initialization: MagicMock):
        """Test initialization of the CAN bus was successful."""
        shell = BMSShell()
        shell.bus_cfg = MagicMock()
        mock_initialization.return_value = (MagicMock(), MagicMock())
        buf = io.StringIO()
        with redirect_stdout(buf):
            ret = shell.do_init(None)
        self.assertEqual("", buf.getvalue())
        self.assertTrue(shell.initialized)
        self.assertFalse(ret)

    def test_do_fram_0(self):
        """Tests the do_fram method, when the CAN bus is not initialized."""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.do_fram(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.reinitialize_fram")
    def test_do_fram_1(self, mock_fram):  # pylint: disable=unused-argument
        """Tests the do_fram method, when the CAN bus is initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()

        with redirect_stdout(buf):
            shell.do_fram(None)
        self.assertEqual("FRAM has been reinitialized.\n", buf.getvalue())

    def test_do_rtc_0(self):
        """Tests the do_rtc method, when the CAN bus is not initialized."""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.do_rtc(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.set_rtc_time")
    def test_do_rtc_1(self, mock_rtc):  # pylint: disable=unused-argument
        """Tests the do_rtc method, when the CAN bus is initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()

        with redirect_stdout(buf):
            shell.do_rtc(None)
        self.assertEqual("RTC time has been set:\n", buf.getvalue())
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    @patch("cli.cmd_bms.bms_shell.set_rtc_time")
    def test_do_rtc_2(self, mock_rtc):  # pylint: disable=unused-argument
        """Tests the do_rtc method, when the CAN bus is initialized
        and when waiting for a message."""
        shell = BMSShell()
        shell.initialized = True
        shell.msg_id.value = 1
        out = io.StringIO()
        err = io.StringIO()

        with redirect_stdout(out), redirect_stderr(err):
            shell.do_rtc(None)
        self.assertEqual("RTC time has been set:\n", out.getvalue())
        self.assertEqual(
            "Waiting for message with ID 1 has been stopped.\n", err.getvalue()
        )
        self.assertEqual(shell.msg_id.value, 769)
        self.assertEqual(shell.msg_num.value, 1)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_softwarereset_0(self):
        """Tests the do_softwarereset method, when the CAN bus is not initialized."""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.do_softwarereset(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.reset_software")
    def test_do_softwarereset_1(self, mock_software_reset):  # pylint: disable=unused-argument
        """Tests the do_softwarereset method, when the CAN bus is initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()

        with redirect_stdout(buf):
            shell.do_softwarereset(None)
        self.assertEqual("Software reset has been triggered.\n", buf.getvalue())
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_boottimestamp_0(self):
        """Tests the do_boottimestamp method, when the CAN bus is not initialized."""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.do_boottimestamp(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_boot_timestamp")
    def test_do_boottimestamp_1(self, mock_boot_timestamp):  # pylint: disable=unused-argument
        """Tests the do_boottimestamp method, when the CAN bus is initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()

        with redirect_stdout(buf):
            shell.do_boottimestamp(None)
        self.assertEqual("Boot Timestamp has been requested.\n", buf.getvalue())
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    @patch("cli.cmd_bms.bms_shell.get_boot_timestamp")
    def test_do_boottimestamp_2(self, mock_boot_timestamp):  # pylint: disable=unused-argument
        """Tests the do_boottimestamp method, when the CAN bus is initialized
        and when waiting for a message."""
        shell = BMSShell()
        shell.msg_id.value = 1
        shell.initialized = True
        out = io.StringIO()
        err = io.StringIO()

        with redirect_stdout(out), redirect_stderr(err):
            shell.do_boottimestamp(None)
        self.assertEqual("Boot Timestamp has been requested.\n", out.getvalue())
        self.assertEqual(
            "Waiting for message with ID 1 has been stopped.\n", err.getvalue()
        )
        self.assertEqual(shell.msg_id.value, 769)
        self.assertEqual(shell.msg_num.value, 1)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_getrtc_0(self):
        """Tests the do_getrtc method, when the CAN bus is not initialized."""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.do_getrtc(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_rtc_time")
    def test_do_getrtc_1(self, mock_get_rtc):  # pylint: disable=unused-argument
        """Tests the do_getrtc method, when the CAN bus is initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()

        with redirect_stdout(buf):
            shell.do_getrtc(None)
        self.assertEqual("RTC time has been requested.\n", buf.getvalue())
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    @patch("cli.cmd_bms.bms_shell.get_rtc_time")
    def test_do_getrtc_2(self, mock_get_rtc):  # pylint: disable=unused-argument
        """Tests the do_getrtc method, when the CAN bus is initialized
        and when waiting for a message."""
        shell = BMSShell()
        shell.msg_id.value = 1
        shell.initialized = True
        out = io.StringIO()
        err = io.StringIO()

        with redirect_stdout(out), redirect_stderr(err):
            shell.do_getrtc(None)
        self.assertEqual("RTC time has been requested.\n", out.getvalue())
        self.assertEqual(
            "Waiting for message with ID 1 has been stopped.\n", err.getvalue()
        )
        self.assertEqual(shell.msg_id.value, 769)
        self.assertEqual(shell.msg_num.value, 1)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_uptime_0(self):
        """Tests the do_uptime method, when the CAN bus is not initialized."""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.do_uptime(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_uptime")
    def test_do_uptime_1(self, mock_get_uptime):  # pylint: disable=unused-argument
        """Tests the do_uptime method, when the CAN bus is initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()

        with redirect_stdout(buf):
            shell.do_uptime(None)
        self.assertEqual("Uptime has been requested.\n", buf.getvalue())
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    @patch("cli.cmd_bms.bms_shell.get_uptime")
    def test_do_uptime_2(self, mock_get_uptime):  # pylint: disable=unused-argument
        """Tests the do_uptime method, when the CAN bus is initialized
        and when waiting for a message."""
        shell = BMSShell()
        shell.msg_id.value = 1
        shell.initialized = True
        out = io.StringIO()
        err = io.StringIO()

        with redirect_stdout(out), redirect_stderr(err):
            shell.do_uptime(None)
        self.assertEqual("Uptime has been requested.\n", out.getvalue())
        self.assertEqual(
            "Waiting for message with ID 1 has been stopped.\n", err.getvalue()
        )
        self.assertEqual(shell.msg_id.value, 769)
        self.assertEqual(shell.msg_num.value, 1)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_buildconfig_0(self):
        """Tests the do_buildconfig method, when the CAN bus is not initialized."""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.do_buildconfig(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_build_configuration")
    def test_do_buildconfig_1(self, mock_build_config):  # pylint: disable=unused-argument
        """Tests the do_buildconfig method, when the CAN bus is initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()

        with redirect_stdout(buf):
            shell.do_buildconfig(None)
        self.assertEqual("Build Configuration has been requested.\n", buf.getvalue())
        self.assertEqual(shell.msg_id.value, 771)
        self.assertEqual(shell.msg_num.value, 19)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    @patch("cli.cmd_bms.bms_shell.get_build_configuration")
    def test_do_buildconfig_2(self, mock_build_config):  # pylint: disable=unused-argument
        """Tests the do_buildconfig method, when the CAN bus is initialized
        and when waiting for a message."""
        shell = BMSShell()
        shell.msg_id.value = 1
        shell.initialized = True
        out = io.StringIO()
        err = io.StringIO()

        with redirect_stdout(out), redirect_stderr(err):
            shell.do_buildconfig(None)
        self.assertEqual("Build Configuration has been requested.\n", out.getvalue())
        self.assertEqual(
            "Waiting for message with ID 1 has been stopped.\n", err.getvalue()
        )
        self.assertEqual(shell.msg_id.value, 771)
        self.assertEqual(shell.msg_num.value, 19)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_commithash_0(self):
        """Tests the do_commithash method, when the CAN bus is not initialized."""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.do_commithash(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_commit_hash")
    def test_do_commithash_1(self, mock_commit_hash):  # pylint: disable=unused-argument
        """Tests the do_commithash method, when the CAN bus is initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()

        with redirect_stdout(buf):
            shell.do_commithash(None)
        self.assertEqual("Commit Hash has been requested.\n", buf.getvalue())
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    @patch("cli.cmd_bms.bms_shell.get_commit_hash")
    def test_do_commithash_2(self, mock_commit_hash):  # pylint: disable=unused-argument
        """Tests the do_commithash method, when the CAN bus is initialized
        and when waiting for a message."""
        shell = BMSShell()
        shell.msg_id.value = 1
        shell.initialized = True
        out = io.StringIO()
        err = io.StringIO()

        with redirect_stdout(out), redirect_stderr(err):
            shell.do_commithash(None)
        self.assertEqual("Commit Hash has been requested.\n", out.getvalue())
        self.assertEqual(
            "Waiting for message with ID 1 has been stopped.\n", err.getvalue()
        )
        self.assertEqual(shell.msg_id.value, 769)
        self.assertEqual(shell.msg_num.value, 2)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_mcuwaferinfo_0(self):
        """Tests the do_mcuwaferinfo method, when the CAN bus is not initialized."""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.do_mcuwaferinfo(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_mcu_wafer_info")
    def test_do_mcuwaferinfo_1(self, mock_wafer_info):  # pylint: disable=unused-argument
        """Tests the do_mcuwaferinfo method, when the CAN bus is initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()

        with redirect_stdout(buf):
            shell.do_mcuwaferinfo(None)
        self.assertEqual("MCU Wafer information has been requested.\n", buf.getvalue())
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    @patch("cli.cmd_bms.bms_shell.get_mcu_wafer_info")
    def test_do_mcuwaferinfo_2(self, mock_wafer_info):  # pylint: disable=unused-argument
        """Tests the do_mcuwaferinfo method, when the CAN bus is initialized
        and when waiting for a message."""
        shell = BMSShell()
        shell.msg_id.value = 1
        shell.initialized = True
        out = io.StringIO()
        err = io.StringIO()

        with redirect_stdout(out), redirect_stderr(err):
            shell.do_mcuwaferinfo(None)
        self.assertEqual("MCU Wafer information has been requested.\n", out.getvalue())
        self.assertEqual(
            "Waiting for message with ID 1 has been stopped.\n", err.getvalue()
        )
        self.assertEqual(shell.msg_id.value, 769)
        self.assertEqual(shell.msg_num.value, 1)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_mculotnumber_0(self):
        """Tests the do_mculotnumber method, when the CAN bus is not initialized."""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.do_mculotnumber(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_mcu_lot_number")
    def test_do_mculotnumber_1(self, mock_lot_number):  # pylint: disable=unused-argument
        """Tests the do_mculotnumber method, when the CAN bus is initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()

        with redirect_stdout(buf):
            shell.do_mculotnumber(None)
        self.assertEqual("MCU lot number has been requested.\n", buf.getvalue())
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    @patch("cli.cmd_bms.bms_shell.get_mcu_lot_number")
    def test_do_mculotnumber_2(self, mock_lot_number):  # pylint: disable=unused-argument
        """Tests the do_mculotnumber method, when the CAN bus is initialized
        and when waiting for a message."""
        shell = BMSShell()
        shell.msg_id.value = 1
        shell.initialized = True
        out = io.StringIO()
        err = io.StringIO()

        with redirect_stdout(out), redirect_stderr(err):
            shell.do_mculotnumber(None)
        self.assertEqual("MCU lot number has been requested.\n", out.getvalue())
        self.assertEqual(
            "Waiting for message with ID 1 has been stopped.\n", err.getvalue()
        )
        self.assertEqual(shell.msg_id.value, 769)
        self.assertEqual(shell.msg_num.value, 1)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_mcuid_0(self):
        """Tests the do_mcuid method, when the CAN bus is not initialized."""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.do_mcuid(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_mcu_id")
    def test_do_mcuid_1(self, mock_id):  # pylint: disable=unused-argument
        """Tests the do_mcuid method, when the CAN bus is initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()

        with redirect_stdout(buf):
            shell.do_mcuid(None)
        self.assertEqual("MCU ID has been requested.\n", buf.getvalue())
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    @patch("cli.cmd_bms.bms_shell.get_mcu_id")
    def test_do_mcuid_2(self, mock_id):  # pylint: disable=unused-argument
        """Tests the do_mcuid method, when the CAN bus is initialized
        and when waiting for a message."""
        shell = BMSShell()
        shell.msg_id.value = 1
        shell.initialized = True
        out = io.StringIO()
        err = io.StringIO()

        with redirect_stdout(out), redirect_stderr(err):
            shell.do_mcuid(None)
        self.assertEqual("MCU ID has been requested.\n", out.getvalue())
        self.assertEqual(
            "Waiting for message with ID 1 has been stopped.\n", err.getvalue()
        )
        self.assertEqual(shell.msg_id.value, 769)
        self.assertEqual(shell.msg_num.value, 1)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_softwareversion_0(self):
        """Tests the do_softwareversion method, when the CAN bus is not initialized."""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.do_softwareversion(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_software_version")
    def test_do_softwareversion_1(self, mock_software_version):  # pylint: disable=unused-argument
        """Tests the do_softwareversion method, when the CAN bus is initialized."""
        shell = BMSShell()
        shell.initialized = True
        buf = io.StringIO()

        with redirect_stdout(buf):
            shell.do_softwareversion(None)
        self.assertEqual("Software version has been requested.\n", buf.getvalue())
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    @patch("cli.cmd_bms.bms_shell.get_software_version")
    def test_do_softwareversion_2(self, mock_software_version):  # pylint: disable=unused-argument
        """Tests the do_softwareversion method, when the CAN bus is initialized
        and when waiting for a message."""
        shell = BMSShell()
        shell.msg_id.value = 1
        shell.initialized = True
        out = io.StringIO()
        err = io.StringIO()

        with redirect_stdout(out), redirect_stderr(err):
            shell.do_softwareversion(None)
        self.assertEqual("Software version has been requested.\n", out.getvalue())
        self.assertEqual(
            "Waiting for message with ID 1 has been stopped.\n", err.getvalue()
        )
        self.assertEqual(shell.msg_id.value, 769)
        self.assertEqual(shell.msg_num.value, 1)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_exit_0(self):
        """Tests the do_exit method, when the CAN bus is not initialized."""
        buf = io.StringIO()
        shell = BMSShell()

        with redirect_stdout(buf):
            result = shell.do_exit(None)
        self.assertEqual("Exiting...\n", buf.getvalue())
        self.assertTrue(result)

    @patch("cli.cmd_bms.bms_shell.shutdown")
    @patch("cli.cmd_bms.bms_shell.Process")
    def test_do_exit_1(self, mock_process: MagicMock, mock_shutdown: MagicMock):  # pylint: disable=unused-argument
        """Tests the do_exit method, when the CAN bus is initialized."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True
        shell.p_recv = MagicMock()
        shell.p_read = MagicMock()
        shell.network_ok = MagicMock()

        with redirect_stdout(buf):
            result = shell.do_exit(None)
        self.assertEqual("", buf.getvalue())
        self.assertEqual(shell.initialized, False)
        self.assertTrue(result)

    def test_do_log_0(self):
        """Tests the do_log method, when the CAN bus is not initialized."""
        buf = io.StringIO()
        shell = BMSShell()

        with redirect_stderr(buf):
            result = shell.do_log(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())
        self.assertFalse(result)

    def test_do_log_1(self):
        """Tests the do_log method, when the input is not an integer."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True

        with redirect_stderr(buf):
            result = shell.do_log("integer")
        self.assertEqual(
            "Message ID has to be given as an integer\n"
            "or as a hexadecimal number in the format '300h' or '0x300'.\n",
            buf.getvalue(),
        )
        self.assertFalse(result)

    def test_do_log_2(self):
        """Tests the do_log method, when the input is an integer as a string."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True

        with redirect_stdout(buf):
            result = shell.do_log("1")
        self.assertEqual(
            "Waiting for message with ID 1.\nTo stop logging enter LOG STOP\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        self.assertEqual(shell.msg_id.value, 1)
        self.assertEqual(shell.msg_num.value, 1)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_log_3(self):
        """Tests the do_log method, when the input is an integer."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True

        with redirect_stdout(buf):
            result = shell.do_log(1)
        self.assertEqual(
            "Waiting for message with ID 1.\nTo stop logging enter LOG STOP\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        self.assertEqual(shell.msg_id.value, 1)
        self.assertEqual(shell.msg_num.value, 1)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_log_4(self):
        """Tests the do_log method, when the input is a hexadecimal number
        indicated by 'h'."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True

        with redirect_stdout(buf):
            result = shell.do_log("300h")
        self.assertEqual(
            "Waiting for message with ID 768.\nTo stop logging enter LOG STOP\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        self.assertEqual(shell.msg_id.value, 768)
        self.assertEqual(shell.msg_num.value, 1)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_log_5(self):
        """Tests the do_log method, when the input is a hexadecimal number
        indicated by '0x'."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True

        with redirect_stdout(buf):
            result = shell.do_log("0x300")
        self.assertEqual(
            "Waiting for message with ID 768.\nTo stop logging enter LOG STOP\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        self.assertEqual(shell.msg_id.value, 768)
        self.assertEqual(shell.msg_num.value, 1)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_log_6(self):
        """Tests the do_log method, when the input contains an integer as a string
        and a number."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True

        with redirect_stdout(buf):
            result = shell.do_log("1 4")
        self.assertEqual(
            "Waiting for message with ID 1.\nTo stop logging enter LOG STOP\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        self.assertEqual(shell.msg_id.value, 1)
        self.assertEqual(shell.msg_num.value, 4)
        shell.msg_id.value = 0
        shell.msg_num.value = 0

    def test_do_log_7(self):
        """Tests the do_log method, when no input has been given."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True

        with redirect_stderr(buf):
            result = shell.do_log("")
        self.assertEqual(
            "ID of the message to be logged has to be specified.\n", buf.getvalue()
        )
        self.assertFalse(result)
        self.assertEqual(shell.msg_id.value, 0)
        self.assertEqual(shell.msg_num.value, 0)

    def test_do_log_8(self):
        """Tests the do_log method,
        when the input is neither an integer nor a string."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True

        with redirect_stderr(buf):
            result = shell.do_log(None)
        self.assertEqual(
            "Message ID has to be given as an integer\n"
            "or as a hexadecimal number in the format '300h' or '0x300'.\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        self.assertEqual(shell.msg_id.value, 0)
        self.assertEqual(shell.msg_num.value, 0)

    def test_do_log_stop(self):
        """Tests the do_log method, when the input is 'stop'."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.msg_id.value = 1
        shell.msg_num.value = 2
        shell.initialized = True
        with redirect_stdout(buf):
            result = shell.do_log("stop")
        self.assertEqual(
            "Waiting for message with ID 1 has been stopped.\n", buf.getvalue()
        )
        self.assertFalse(result)
        self.assertEqual(shell.msg_id.value, 0)
        self.assertEqual(shell.msg_num.value, 0)

    def test_precmd(self):
        """Tests the precmd method"""
        shell = BMSShell()

        result = shell.precmd("EXIT")
        self.assertEqual(result, "exit")

    def test_default(self):
        """Tests the default method"""
        shell = BMSShell()
        buf = io.StringIO()

        with redirect_stderr(buf):
            shell.onecmd("LINE")

        self.assertEqual("Invalid command: LINE\n", buf.getvalue())


if __name__ == "__main__":
    unittest.main()
