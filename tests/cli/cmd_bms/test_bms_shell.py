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

"""Testing file 'cli/cmd_bms/bms_shell.py'."""

# cspell:ignore getrtc,mcuwaferinfo,mcuid,mculotnumber

import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from multiprocessing import Manager, managers
from pathlib import Path
from unittest.mock import MagicMock, patch

from cantools.database.can.database import Database

try:
    from cli.cmd_bms.bms_shell import BMSShell, run_shell
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_bms.bms_shell import BMSShell, run_shell


class TestRunShell(unittest.TestCase):
    """Class to test the run_shell method"""

    @patch("cli.cmd_bms.bms_shell.Database.get_message_by_name")
    @patch("cli.cmd_bms.bms_shell.database.load_file")
    @patch("cmd.Cmd.cmdloop")
    def test_database(
        self,
        mock_cmdloop: MagicMock,
        mock_load_file: MagicMock,
        mock_get_message: MagicMock,
    ):
        """Tests the method when the given file is a Database"""
        app_dbc = MagicMock()
        mock_load_file.return_value = Database()
        mock_get_message.return_value = MagicMock()
        mock_cmdloop.side_effect = KeyboardInterrupt
        out = io.StringIO()
        err = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = run_shell(MagicMock(), app_dbc)
        self.assertEqual(ret, 0)
        self.assertEqual("Exiting...\nThe shell has been stopped. \n", out.getvalue())
        self.assertEqual(
            "Error detected. Please wait for the Process to terminate.\n",
            err.getvalue(),
        )

    @patch("cli.cmd_bms.bms_shell.database.load_file")
    def test_not_database(self, mock_load_file: MagicMock):
        """Tests the method when the given file is not a Database"""
        app_dbc = MagicMock()
        mock_load_file.return_value = Path()
        out = io.StringIO()
        err = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            ret = run_shell(MagicMock(), app_dbc)
        self.assertEqual(ret, 1)
        self.assertEqual("", out.getvalue())
        self.assertEqual("DBC file is not of type 'Database'.\n", err.getvalue())


@patch("cli.cmd_bms.bms_shell.initialization")
class TestDoInit(unittest.TestCase):
    """Class to test the BMSShell.do_init method"""

    def setUp(self):
        manager = Manager()
        self.shell = BMSShell()
        self.shell.msg_arr = manager.list([[], [], []])
        self.shell.bus_cfg = MagicMock()
        self.shell.app_dbc = None

    def test_initialized_true(self, _: MagicMock):
        """Shell has already been initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            ret = self.shell.do_init(None)
        self.assertEqual("The CAN bus has already been initialized.\n", buf.getvalue())
        self.assertEqual(ret, False)

    def test_fail(self, mock_initialization: MagicMock):
        """Initializing the CAN bus failed."""
        self.shell.initialized = False
        mock_initialization.return_value = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            ret = self.shell.do_init(None)
        self.assertEqual("", buf.getvalue())
        self.assertTrue(ret)

    def test_success(self, mock_initialization: MagicMock):
        """Initializing the CAN bus was successful."""
        self.shell.initialized = False
        mock_initialization.return_value = (MagicMock(), MagicMock())
        buf = io.StringIO()
        with redirect_stdout(buf):
            ret = self.shell.do_init(None)
        self.assertEqual("", buf.getvalue())
        self.assertTrue(self.shell.initialized)
        self.assertFalse(ret)


class TestAddMsg(unittest.TestCase):
    """Test add_msg method"""

    def setUp(self):
        self.manager = Manager()
        self.shell = BMSShell()
        self.shell.bus_cfg = MagicMock()

    def test_add_msg_str(self):
        """Test add_msg function with msg_id as string"""
        self.shell.msg_arr = self.manager.list([[], [], []])
        self.shell.app_dbc = MagicMock()
        self.shell.app_dbc.get_message_by_name.return_value.frame_id = 10
        self.shell.add_msg(msg_id="f_Debug")
        self.assertEqual(self.shell.msg_arr[0], [10])
        self.assertEqual(self.shell.msg_arr[1], [1])
        self.assertEqual(self.shell.msg_arr[2], [1])

    def test_add_msg_int(self):
        """Test add_msg function with msg_id as integer"""
        self.shell.msg_arr = self.manager.list([[], [], []])
        self.shell.add_msg(msg_id=10)
        self.assertEqual(self.shell.msg_arr[0], [10])
        self.assertEqual(self.shell.msg_arr[1], [1])
        self.assertEqual(self.shell.msg_arr[2], [1])

    def test_add_msg(self):
        """Test add_msg function when msg_arr already contains entries"""
        self.shell.msg_arr = self.manager.list([[5], [5], [0]])
        self.shell.add_msg(msg_id=10)
        self.assertEqual(self.shell.msg_arr[0], [5, 10])
        self.assertEqual(self.shell.msg_arr[1], [5, 1])
        self.assertEqual(self.shell.msg_arr[2], [0, 1])

    def test_add_msg_amount(self):
        """Test add_msg function with amount given"""
        self.shell.msg_arr = self.manager.list([[], [], []])
        self.shell.add_msg(msg_id=10, amount=5)
        self.assertEqual(self.shell.msg_arr[0], [10])
        self.assertEqual(self.shell.msg_arr[1], [5])
        self.assertEqual(self.shell.msg_arr[2], [1])

    def test_add_msg_output(self):
        """Test add_msg function with output given"""
        self.shell.msg_arr = self.manager.list([[], [], []])
        self.shell.add_msg(msg_id=10, output=0)
        self.assertEqual(self.shell.msg_arr[0], [10])
        self.assertEqual(self.shell.msg_arr[1], [1])
        self.assertEqual(self.shell.msg_arr[2], [0])

    def test_add_msg_invalid(self):
        """Test add_msg function with invalid msg_id"""
        self.shell.msg_arr = self.manager.list([[], [], []])
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.add_msg(msg_id=None)
        self.assertEqual("Invalid message ID\n", buf.getvalue())
        self.assertEqual(self.shell.msg_arr[0], [])
        self.assertEqual(self.shell.msg_arr[1], [])
        self.assertEqual(self.shell.msg_arr[2], [])


class TestBmsShell(unittest.TestCase):
    """Class to test the BMSShell class"""

    def test_preloop(self):
        """Tests the preloop function"""
        shell = BMSShell()
        shell.preloop()
        self.assertIsInstance(shell.msg_arr, managers.ListProxy)
        self.assertEqual(shell.msg_arr[0], [])
        self.assertEqual(shell.msg_arr[1], [])
        self.assertEqual(shell.msg_arr[2], [])

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


class TestCommands(unittest.TestCase):  # pylint: disable=too-many-public-methods
    """Class to test all commands that send a message"""

    def setUp(self):
        self.manager = Manager()
        self.shell = BMSShell()
        self.shell.msg_arr = self.manager.list([[], [], []])
        self.shell.message = None

    def test_do_fram_0(self):
        """Tests the do_fram method, when the CAN bus is not initialized."""
        self.shell.initialized = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.do_fram(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.reinitialize_fram")
    def test_do_fram_1(self, _: MagicMock):
        """Tests the do_fram method, when the CAN bus is initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.shell.do_fram(None)
        self.assertEqual("FRAM has been reinitialized.\n", buf.getvalue())

    def test_do_rtc_0(self):
        """Tests the do_rtc method, when the CAN bus is not initialized."""
        self.shell.initialized = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.do_rtc(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.set_rtc_time")
    @patch("cli.cmd_bms.bms_shell.BMSShell.add_msg")
    def test_do_rtc_1(self, mock_add_msg: MagicMock, _: MagicMock):
        """Tests the do_rtc method, when the CAN bus is initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.shell.do_rtc(None)
        self.assertEqual("RTC time has been set:\n", buf.getvalue())
        mock_add_msg.assert_called_once_with("f_DebugResponse")

    def test_do_softwarereset_0(self):
        """Tests the do_softwarereset method, when the CAN bus is not initialized."""
        self.shell.initialized = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.do_softwarereset(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.reset_software")
    def test_do_softwarereset_1(self, _: MagicMock):
        """Tests the do_softwarereset method, when the CAN bus is initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.shell.do_softwarereset(None)
        self.assertEqual("Software reset has been triggered.\n", buf.getvalue())

    def test_do_boottimestamp_0(self):
        """Tests the do_boottimestamp method, when the CAN bus is not initialized."""
        self.shell.initialized = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.do_boottimestamp(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_boot_timestamp")
    @patch("cli.cmd_bms.bms_shell.BMSShell.add_msg")
    def test_do_boottimestamp_1(self, mock_add_msg: MagicMock, _: MagicMock):
        """Tests the do_boottimestamp method, when the CAN bus is initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.shell.do_boottimestamp(None)
        self.assertEqual("Boot Timestamp has been requested.\n", buf.getvalue())
        mock_add_msg.assert_called_once_with("f_DebugResponse")

    def test_do_getrtc_0(self):
        """Tests the do_getrtc method, when the CAN bus is not initialized."""
        self.shell.initialized = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.do_getrtc(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_rtc_time")
    @patch("cli.cmd_bms.bms_shell.BMSShell.add_msg")
    def test_do_getrtc_1(self, mock_add_msg: MagicMock, _: MagicMock):
        """Tests the do_getrtc method, when the CAN bus is initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.shell.do_getrtc(None)
        self.assertEqual("RTC time has been requested.\n", buf.getvalue())
        mock_add_msg.assert_called_once_with("f_DebugResponse")

    def test_do_uptime_0(self):
        """Tests the do_uptime method, when the CAN bus is not initialized."""
        self.shell.initialized = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.do_uptime(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_uptime")
    @patch("cli.cmd_bms.bms_shell.BMSShell.add_msg")
    def test_do_uptime_1(self, mock_add_msg: MagicMock, _: MagicMock):
        """Tests the do_uptime method, when the CAN bus is initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.shell.do_uptime(None)
        self.assertEqual("Uptime has been requested.\n", buf.getvalue())
        mock_add_msg.assert_called_once_with("f_DebugResponse")

    def test_do_buildconfig_0(self):
        """Tests the do_buildconfig method, when the CAN bus is not initialized."""
        self.shell.initialized = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.do_buildconfig(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_build_configuration")
    @patch("cli.cmd_bms.bms_shell.BMSShell.add_msg")
    def test_do_buildconfig_1(self, mock_add_msg: MagicMock, _: MagicMock):
        """Tests the do_buildconfig method, when the CAN bus is initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.shell.do_buildconfig(None)
        self.assertEqual("Build Configuration has been requested.\n", buf.getvalue())
        mock_add_msg.assert_called_once_with("f_DebugBuildConfiguration", 19)

    def test_do_commithash_0(self):
        """Tests the do_commithash method, when the CAN bus is not initialized."""
        self.shell.initialized = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.do_commithash(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_commit_hash")
    @patch("cli.cmd_bms.bms_shell.BMSShell.add_msg")
    def test_do_commithash_1(self, mock_add_msg: MagicMock, _: MagicMock):
        """Tests the do_commithash method, when the CAN bus is initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.shell.do_commithash(None)
        self.assertEqual("Commit Hash has been requested.\n", buf.getvalue())
        mock_add_msg.assert_called_once_with("f_DebugResponse", 2)

    def test_do_mcuwaferinfo_0(self):
        """Tests the do_mcuwaferinfo method, when the CAN bus is not initialized."""
        self.shell.initialized = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.do_mcuwaferinfo(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_mcu_wafer_info")
    @patch("cli.cmd_bms.bms_shell.BMSShell.add_msg")
    def test_do_mcuwaferinfo_1(self, mock_add_msg: MagicMock, _: MagicMock):
        """Tests the do_mcuwaferinfo method, when the CAN bus is initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.shell.do_mcuwaferinfo(None)
        self.assertEqual("MCU Wafer information has been requested.\n", buf.getvalue())
        mock_add_msg.assert_called_once_with("f_DebugResponse")

    def test_do_mculotnumber_0(self):
        """Tests the do_mculotnumber method, when the CAN bus is not initialized."""
        self.shell.initialized = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.do_mculotnumber(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_mcu_lot_number")
    @patch("cli.cmd_bms.bms_shell.BMSShell.add_msg")
    def test_do_mculotnumber_1(self, mock_add_msg: MagicMock, _: MagicMock):
        """Tests the do_mculotnumber method, when the CAN bus is initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.shell.do_mculotnumber(None)
        self.assertEqual("MCU lot number has been requested.\n", buf.getvalue())
        mock_add_msg.assert_called_once_with("f_DebugResponse")

    def test_do_mcuid_0(self):
        """Tests the do_mcuid method, when the CAN bus is not initialized."""
        self.shell.initialized = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.do_mcuid(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_mcu_id")
    @patch("cli.cmd_bms.bms_shell.BMSShell.add_msg")
    def test_do_mcuid_1(self, mock_add_msg: MagicMock, _: MagicMock):
        """Tests the do_mcuid method, when the CAN bus is initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.shell.do_mcuid(None)
        self.assertEqual("MCU ID has been requested.\n", buf.getvalue())
        mock_add_msg.assert_called_once_with("f_DebugResponse")

    def test_do_softwareversion_0(self):
        """Tests the do_softwareversion method, when the CAN bus is not initialized."""
        self.shell.initialized = False
        buf = io.StringIO()
        with redirect_stderr(buf):
            self.shell.do_softwareversion(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())

    @patch("cli.cmd_bms.bms_shell.get_software_version")
    @patch("cli.cmd_bms.bms_shell.BMSShell.add_msg")
    def test_do_softwareversion_1(self, mock_add_msg: MagicMock, _: MagicMock):
        """Tests the do_softwareversion method, when the CAN bus is initialized."""
        self.shell.initialized = True
        buf = io.StringIO()
        with redirect_stdout(buf):
            self.shell.do_softwareversion(None)
        self.assertEqual("Software version has been requested.\n", buf.getvalue())
        mock_add_msg.assert_called_once_with("f_DebugResponse")


class TestExit(unittest.TestCase):
    """Class to test the do_exit method"""

    def setUp(self):
        self.manager = Manager()

    def test_no_init(self):
        """Tests the do_exit method, when the CAN bus is not initialized."""
        buf = io.StringIO()
        shell = BMSShell()
        with redirect_stdout(buf):
            result = shell.do_exit(None)
        self.assertEqual("Exiting...\n", buf.getvalue())
        self.assertTrue(result)

    @patch("cli.cmd_bms.bms_shell.shutdown")
    @patch("cli.cmd_bms.bms_shell.Process")
    def test_with_init(self, mock_process: MagicMock, mock_shutdown: MagicMock):  # pylint: disable=unused-argument
        """Tests the do_exit method, when the CAN bus is initialized."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.msg_arr = self.manager.list([[], [], []])
        shell.initialized = True
        shell.p_recv = MagicMock()
        shell.p_read = MagicMock()
        shell.network_ok = MagicMock()
        with redirect_stdout(buf):
            result = shell.do_exit(None)
        self.assertEqual("", buf.getvalue())
        self.assertEqual(shell.initialized, False)
        self.assertTrue(result)
        self.assertEqual(shell.msg_arr[0], [])
        self.assertEqual(shell.msg_arr[1], [])
        self.assertEqual(shell.msg_arr[2], [])


@patch("cli.cmd_bms.bms_shell.BMSShell.add_msg")
class TestDoLog(unittest.TestCase):
    """Class to test the do_log method"""

    def setUp(self):
        self.manager = Manager()

    def test_not_init(self, _: MagicMock):
        """CAN bus is not initialized"""
        buf = io.StringIO()
        shell = BMSShell()
        with redirect_stderr(buf):
            result = shell.do_log(None)
        self.assertEqual("CAN bus has to be initialized: INIT\n", buf.getvalue())
        self.assertFalse(result)

    def test_wrong_input_integer(self, mock_add_msg: MagicMock):
        """Input is not an integer."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.msg_arr = self.manager.list([[], [], []])
        shell.initialized = True
        with redirect_stderr(buf):
            result = shell.do_log("integer")
        self.assertEqual(
            "Message ID has to be given as an integer\n"
            "or as a hexadecimal number in the format '300h' or '0x300'.\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        mock_add_msg.assert_not_called()

    def test_no_input(self, mock_add_msg: MagicMock):
        """No input has been given."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True
        shell.msg_arr = self.manager.list([[], [], []])
        with redirect_stderr(buf):
            result = shell.do_log("")
        self.assertEqual(
            "ID of the message to be logged has to be specified.\n", buf.getvalue()
        )
        self.assertFalse(result)
        mock_add_msg.assert_not_called()

    def test_wrong_input(self, mock_add_msg: MagicMock):
        """Input is neither an integer nor a string."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True
        shell.msg_arr = self.manager.list([[], [], []])
        with redirect_stderr(buf):
            result = shell.do_log(None)
        self.assertEqual(
            "Message ID has to be given as an integer\n"
            "or as a hexadecimal number in the format '300h' or '0x300'.\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        mock_add_msg.assert_not_called()

    def test_correct_input(self, mock_add_msg: MagicMock):
        """Input is an integer as a string."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True
        shell.msg_arr = self.manager.list([[1], [], []])
        with redirect_stdout(buf):
            result = shell.do_log("1")
        self.assertEqual(
            "Waiting for message with ID 0x1.\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        mock_add_msg.assert_called_once_with(1, 1, 1)

    def test_correct_input_integer(self, mock_add_msg: MagicMock):
        """Input is an integer."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True
        shell.msg_arr = self.manager.list([[1], [], []])
        with redirect_stdout(buf):
            result = shell.do_log(1)
        self.assertEqual(
            "Waiting for message with ID 0x1.\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        mock_add_msg.assert_called_once_with(1)

    def test_correct_input_hex_h(self, mock_add_msg: MagicMock):
        """Input is a hexadecimal number indicated by 'h'."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True
        shell.msg_arr = self.manager.list([[768], [], []])
        with redirect_stdout(buf):
            result = shell.do_log("300h")
        self.assertEqual(
            "Waiting for message with ID 0x300.\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        mock_add_msg.assert_called_once_with(768, 1, 1)

    def test_correct_input_hex_x(self, mock_add_msg: MagicMock):
        """Input is a hexadecimal number indicated by '0x'."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True
        shell.msg_arr = self.manager.list([[768], [], []])
        with redirect_stdout(buf):
            result = shell.do_log("0x300")
        self.assertEqual(
            "Waiting for message with ID 0x300.\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        mock_add_msg.assert_called_once_with(768, 1, 1)

    def test_correct_input_string_number(self, mock_add_msg: MagicMock):
        """Input contains an integer as a string and a number."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True
        shell.msg_arr = self.manager.list([[1], [4], [1]])
        with redirect_stdout(buf):
            result = shell.do_log("1 4")
        self.assertEqual(
            "Waiting for message with ID 0x1.\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        mock_add_msg.assert_called_once_with(1, 4, 1)

    def test_correct_input_string_file(self, mock_add_msg: MagicMock):
        """Input contains an integer as a string and 'FILE'"""
        buf = io.StringIO()
        shell = BMSShell()
        shell.initialized = True
        shell.msg_arr = self.manager.list([[1], [1], [0]])
        with redirect_stdout(buf):
            result = shell.do_log("1 FILE")
        self.assertEqual(
            "Waiting for message with ID 0x1.\n",
            buf.getvalue(),
        )
        self.assertFalse(result)
        mock_add_msg.assert_called_once_with(1, 1, 0)

    def test_correct_string_wrong_file(self, mock_add_msg: MagicMock):
        """Input contains an integer as a string
        and an invalid argument for logging to a file.
        """
        out = io.StringIO()
        err = io.StringIO()
        shell = BMSShell()
        shell.initialized = True
        shell.msg_arr = self.manager.list([[1], [1], [1]])
        with redirect_stdout(out), redirect_stderr(err):
            result = shell.do_log("1 INVALID")
        self.assertEqual(
            "Waiting for message with ID 0x1.\n",
            out.getvalue(),
        )
        self.assertEqual(
            "For logging to a file enter the argument 'FILE'.\n", err.getvalue()
        )
        self.assertFalse(result)
        mock_add_msg.assert_called_once_with(1, 1, 1)

    def test_stop(self, mock_add_msg: MagicMock):
        """Input is 'stop'."""
        buf = io.StringIO()
        shell = BMSShell()
        shell.msg_arr = self.manager.list([[1], [1], [1]])
        shell.initialized = True
        with redirect_stdout(buf):
            result = shell.do_log("stop")
        self.assertEqual("This doesn't work yet.\n", buf.getvalue())
        self.assertFalse(result)
        mock_add_msg.assert_not_called()


if __name__ == "__main__":
    unittest.main()
