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

"""Testing file 'cli/cmd_gui/frame_sim/sim_gui.py'."""

import os
import shutil
import sys
import tkinter as tk
import unittest
from datetime import UTC, datetime
from pathlib import Path
from unittest.mock import MagicMock, call, patch

try:
    from cli.cmd_gui.frame_sim import sim_gui
    from cli.helpers.misc import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_sim import sim_gui
    from cli.helpers.misc import PROJECT_BUILD_ROOT

RUN_TESTS = os.environ.get("DISPLAY", False) or sys.platform.startswith("win32")
PATH_GUI = PROJECT_BUILD_ROOT / "sim_frame"


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestSimulateBmsFrame(unittest.TestCase):
    """Test of the SimulateBmsFrame class"""

    def setUp(self):
        self.start_time = datetime.now(tz=UTC)
        sim_gui.PROJECT_BUILD_ROOT = PATH_GUI
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        self.frame = sim_gui.SimulateBmsFrame(self.root, text)

    def tearDown(self):
        self.root.update()
        self.root.destroy()
        sim_gui.PROJECT_BUILD_ROOT = PROJECT_BUILD_ROOT
        remove_data(self.start_time)

    def test_write_text_empty(self):
        """Test 'write_text' function when the file is empty"""
        mock_select = MagicMock()
        mock_select.return_value = self.frame
        self.frame.parent.select = mock_select
        self.frame.file_path.touch()
        self.frame.write_text()
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(0, self.frame.text_index)

    def test_write_text(self):
        """Test 'write_text' function when the file is not empty"""
        mock_select = MagicMock()
        mock_select.return_value = self.frame
        self.frame.parent.select = mock_select
        self.frame.file_path.write_text("New content.", encoding="utf-8")
        self.frame.write_text()
        self.assertEqual("New content.\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(12, self.frame.text_index)

    def test_write_text_not_selected(self):
        """Test 'write_text' function when SimulateBmsFrame is not selected"""
        mock_select = MagicMock()
        mock_select.return_value = ""
        self.frame.parent.select = mock_select
        self.frame.file_path.write_text("New content.", encoding="utf-8")
        self.frame.write_text()
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(0, self.frame.text_index)

    def test_write_text_string(self):
        """Test 'write_text' function when a string is passed"""
        mock_select = MagicMock()
        mock_select.return_value = self.frame
        self.frame.parent.select = mock_select
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.frame.write_text(file_input="New content.")
        self.assertEqual("New content.\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(12, self.frame.text_index)

    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.after")
    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.write_text")
    def test_check_threads_alive(
        self, mock_write_text: MagicMock, mock_after: MagicMock
    ):
        """Test 'check_threads' function when the Threads are still alive"""
        self.frame.bms_process = MagicMock()
        self.frame.bms_process.is_alive.return_value = True
        self.frame.unit_process = MagicMock()
        self.frame.unit_process.is_alive.return_value = True
        self.frame.check_threads()

        mock_after.assert_called_once_with(50, self.frame.check_threads)
        self.frame.bms_process.is_alive.assert_called_once()
        self.frame.unit_process.is_alive.assert_not_called()
        mock_write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.write_text")
    def test_check_threads_dead(self, mock_write_text: MagicMock):
        """Test 'check_threads' function when the Threads are not alive"""
        self.frame.bms_process = MagicMock()
        self.frame.bms_process.is_alive.return_value = False
        self.frame.unit_process = MagicMock()
        self.frame.unit_process.is_alive.return_value = False
        self.frame.sim_active = True
        self.frame.check_threads()

        self.frame.bms_process.is_alive.assert_called_once()
        self.frame.unit_process.is_alive.assert_called_once()
        mock_write_text.assert_has_calls([call(), call("Simulation terminated.\n")])
        self.assertFalse(self.frame.sim_active)

    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.write_text")
    def test_check_threads_dead_can_com(self, mock_write_text: MagicMock):
        """Test 'check_threads' function when the Threads are not alive and
        the 'can_com_*' attributes exist
        """
        self.frame.bms_process = MagicMock()
        self.frame.bms_process.is_alive.return_value = False
        self.frame.unit_process = MagicMock()
        self.frame.unit_process.is_alive.return_value = False
        self.frame.sim_active = True
        self.frame.can_com_bms = MagicMock()
        self.frame.can_com_unit = MagicMock
        self.frame.check_threads()
        self.frame.bms_process.is_alive.assert_called_once()
        self.frame.unit_process.is_alive.assert_called_once()
        mock_write_text.assert_has_calls([call(), call("Simulation terminated.\n")])
        self.assertFalse(self.frame.sim_active)
        self.assertFalse(hasattr(self.frame, "can_com_bms"))
        self.assertFalse(hasattr(self.frame, "can_com_unit"))

    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.write_text")
    def test_start_stop_sim_cb_no_config(self, mock_write_text: MagicMock):
        """Test 'start_stop_sim_cb' function when CAN Bus configs are missing"""
        self.frame.start_stop_sim_cb()
        mock_write_text.assert_called_once_with(
            "Add CAN Configurations before starting the Simulation.\n"
        )

    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.write_text")
    def test_start_stop_sim_cb_active(self, mock_write_text: MagicMock):
        """Test 'start_stop_sim_cb' function when sim_active is True"""
        self.frame.can_bus_bms = MagicMock()
        self.frame.can_bus_unit = MagicMock()
        self.frame.sim_active = True
        self.frame.start_stop_sim_cb()
        mock_write_text.assert_called_once_with("Stopping the Simulation...\n")

    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.write_text")
    def test_start_stop_sim_cb_active_can_com(self, mock_write_text: MagicMock):
        """Test 'start_stop_sim_cb' function when sim_active is True
        and 'can_com_*' attributes exist
        """
        self.frame.can_bus_bms = MagicMock()
        self.frame.can_bus_unit = MagicMock()
        self.frame.can_com_bms = MagicMock()
        self.frame.can_com_unit = MagicMock()
        self.frame.sim_active = True
        self.frame.start_stop_sim_cb()
        mock_write_text.assert_called_once_with("Stopping the Simulation...\n")
        self.frame.can_com_bms.shutdown.assert_called_once_with(block=True, timeout=1)
        self.frame.can_com_unit.shutdown.assert_called_once_with(block=True, timeout=1)

    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.write_text")
    @patch("cli.cmd_gui.frame_sim.sim_gui.Thread")
    def test_start_stop_sim_cb_inactive(
        self, mock_thread: MagicMock, mock_write_text: MagicMock
    ):
        """Test 'start_stop_sim_cb' function when sim_active is False
        and 'can_com_*' attributes exist
        """
        mock_bms_process = MagicMock()
        mock_unit_process = MagicMock()
        mock_thread.side_effect = [mock_bms_process, mock_unit_process]
        self.frame.can_bus_bms = MagicMock()
        self.frame.can_bus_unit = MagicMock()
        self.frame.sim_active = False
        self.frame.check_threads = MagicMock()
        self.frame.start_stop_sim_cb()
        mock_write_text.assert_called_once_with("Starting the Simulation...\n")
        self.assertTrue(self.frame.sim_active)
        mock_bms_process.start.assert_called_once()
        mock_unit_process.start.assert_called_once()
        self.frame.check_threads.assert_called_once()

    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.write_text")
    def test_send_msg_cb_inactive(self, mock_write_text: MagicMock):
        """Test 'send_msg_cb' function when sim_active is False"""
        self.frame.sim_active = False
        self.frame.send_msg_cb()
        mock_write_text.assert_called_once_with("Simulation has to be started first.\n")

    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.write_text")
    def test_send_msg_cb_error(self, mock_write_text: MagicMock):
        """Test 'send_msg_cb' function when sim_active is True and sending causes an error"""
        self.frame.sim_active = True
        self.frame.can_com_unit = MagicMock()
        self.frame.can_com_unit.write.side_effect = Exception("Error")
        self.frame.send_msg_cb()
        mock_write_text.assert_has_calls([call("Sending message.\n"), call("Error")])

    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.write_text")
    def test_send_msg_cb_active(self, mock_write_text: MagicMock):
        """Test 'send_msg_cb' function when sim_active is True"""
        self.frame.sim_active = True
        self.frame.can_com_unit = MagicMock()
        self.frame.send_msg_cb()
        mock_write_text.assert_called_once_with("Sending message.\n")
        self.frame.can_com_unit.write.assert_called_once()


class TestSimulateBmsFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the SimulateBmsFrame class"""

    def setUp(self):
        self.start_time = datetime.now(tz=UTC)
        PATH_GUI.mkdir(parents=True, exist_ok=True)

    def tearDown(self):
        remove_data(self.start_time)

    def test_write_text_empty(self):
        """Test 'write_text' function when the file is empty"""
        mock_sim_frame = MagicMock()
        mock_sim_frame.parent.nametowidget.return_value = mock_sim_frame  # pylint: disable=no-member,useless-suppression
        mock_sim_frame.file_path = Path(PATH_GUI / "output_sim_write_text_empty.txt")
        mock_sim_frame.text = MagicMock()
        mock_sim_frame.text_index = 0
        mock_sim_frame.file_path.touch()
        sim_gui.SimulateBmsFrame.write_text(mock_sim_frame)
        mock_sim_frame.text.insert.assert_called_once_with(tk.END, "")
        self.assertEqual(mock_sim_frame.text_index, 0)

    def test_write_text(self):
        """Test 'write_text' function when the file is not empty"""
        mock_sim_frame = MagicMock()
        mock_sim_frame.parent.nametowidget.return_value = mock_sim_frame  # pylint: disable=no-member,useless-suppression
        mock_sim_frame.file_path = Path(PATH_GUI / "output_sim_write_text.txt")
        mock_sim_frame.text = MagicMock()
        mock_sim_frame.text_index = 0
        mock_sim_frame.file_path.write_text("New content.", encoding="utf-8")
        sim_gui.SimulateBmsFrame.write_text(mock_sim_frame)
        mock_sim_frame.text.insert.assert_called_once_with(tk.END, "New content.")
        self.assertEqual(mock_sim_frame.text_index, 12)

    def test_write_text_not_selected(self):
        """Test 'write_text' function when SimulateBmsFrame is not selected"""
        mock_sim_frame = MagicMock()
        mock_sim_frame.parent.select.return_value = ""  # pylint: disable=no-member,useless-suppression
        mock_sim_frame.file_path = Path(
            PATH_GUI / "output_sim_write_text_not_selected.txt"
        )
        mock_sim_frame.text = MagicMock()
        mock_sim_frame.text_index = 0
        mock_sim_frame.file_path.write_text("New content.", encoding="utf-8")
        sim_gui.SimulateBmsFrame.write_text(mock_sim_frame)
        mock_sim_frame.text.insert.assert_not_called()
        self.assertEqual(mock_sim_frame.text_index, 0)

    def test_write_text_string(self):
        """Test 'write_text' function when a string is passed"""
        mock_sim_frame = MagicMock()
        mock_sim_frame.parent.nametowidget.return_value = mock_sim_frame  # pylint: disable=no-member,useless-suppression
        mock_sim_frame.file_path = Path(PATH_GUI / "output_sim_write_text_input.txt")
        mock_sim_frame.text = MagicMock()
        mock_sim_frame.text_index = 0
        mock_sim_frame.file_path.touch()
        sim_gui.SimulateBmsFrame.write_text(mock_sim_frame, "New content.")
        mock_sim_frame.text.insert.assert_called_once_with(tk.END, "New content.")
        self.assertEqual(mock_sim_frame.text_index, 12)

    @patch("cli.cmd_gui.frame_sim.sim_gui.CAN")
    @patch("cli.cmd_gui.frame_sim.sim_gui.sim_unit")
    def test_run_sim_unit(self, mock_sim_unit: MagicMock, mock_can: MagicMock):
        """Test 'run_sim_unit' function"""
        mock_shutdown = MagicMock()
        mock_can.return_value = "can"
        mock_write_text = MagicMock()
        mock_sim_frame = MagicMock()
        mock_sim_frame.can_bus_unit = "can_config"
        mock_sim_frame.write_text = mock_write_text
        mock_sim_frame.can_com_bms.shutdown = mock_shutdown
        sim_gui.SimulateBmsFrame.run_sim_unit(mock_sim_frame)

        mock_can.assert_called_once_with("CAN Bus Unit", "can_config")
        mock_sim_unit.assert_called_once_with("can", mock_write_text)
        mock_shutdown.assert_called_once_with(block=True, timeout=1)

    @patch("cli.cmd_gui.frame_sim.sim_gui.CAN")
    @patch("cli.cmd_gui.frame_sim.sim_gui.sim_bms")
    def test_run_sim_bms(self, mock_sim_bms: MagicMock, mock_can: MagicMock):
        """Test 'run_sim_bms' function"""
        mock_shutdown = MagicMock()
        mock_can.return_value = "can"
        mock_write_text = MagicMock()
        mock_sim_frame = MagicMock()
        mock_sim_frame.can_bus_bms = "can_config"
        mock_sim_frame.write_text = mock_write_text
        mock_sim_frame.can_com_unit.shutdown = mock_shutdown
        sim_gui.SimulateBmsFrame.run_sim_bms(mock_sim_frame)

        mock_can.assert_called_once_with("CAN Bus BMS", "can_config")
        mock_sim_bms.assert_called_once_with("can", mock_write_text)
        mock_shutdown.assert_called_once_with(block=True, timeout=1)

    def test_check_threads_alive(self):
        """Test 'check_threads' function when the Threads are still alive"""
        mock_bms_process = MagicMock()
        mock_bms_process.is_alive.return_value = True
        mock_unit_process = MagicMock()
        mock_unit_process.is_alive.return_value = True
        mock_sim_frame = MagicMock()
        mock_sim_frame.bms_process = mock_bms_process
        mock_sim_frame.unit_process = mock_unit_process
        sim_gui.SimulateBmsFrame.check_threads(mock_sim_frame)
        mock_sim_frame.after.assert_called_once_with(50, mock_sim_frame.check_threads)
        mock_bms_process.is_alive.assert_called_once()
        mock_unit_process.is_alive.assert_not_called()
        mock_sim_frame.write_text.assert_called_once()

    def test_check_threads_dead_alive(self):
        """Test 'check_threads' function when only one Thread is alive"""
        mock_bms_process = MagicMock()
        mock_bms_process.is_alive.return_value = False
        mock_unit_process = MagicMock()
        mock_unit_process.is_alive.return_value = True
        mock_sim_frame = MagicMock()
        mock_sim_frame.bms_process = mock_bms_process
        mock_sim_frame.unit_process = mock_unit_process
        sim_gui.SimulateBmsFrame.check_threads(mock_sim_frame)
        mock_sim_frame.after.assert_called_once_with(50, mock_sim_frame.check_threads)
        mock_bms_process.is_alive.assert_called_once()
        mock_unit_process.is_alive.assert_called_once()
        mock_sim_frame.write_text.assert_called_once()

    def test_check_threads_dead(self):
        """Test 'check_threads' function when both Threads are dead"""
        mock_bms_process = MagicMock()
        mock_bms_process.is_alive.return_value = False
        mock_unit_process = MagicMock()
        mock_unit_process.is_alive.return_value = False
        mock_sim_frame = MagicMock()
        mock_sim_frame.bms_process = mock_bms_process
        mock_sim_frame.unit_process = mock_unit_process
        mock_sim_frame.sim_active = True
        sim_gui.SimulateBmsFrame.check_threads(mock_sim_frame)
        mock_bms_process.is_alive.assert_called_once()
        mock_unit_process.is_alive.assert_called_once()
        mock_sim_frame.write_text.assert_has_calls(
            [call(), call("Simulation terminated.\n")]
        )
        self.assertFalse(mock_sim_frame.sim_active)


def remove_data(start_time: datetime) -> None:
    """Remove all data from the gui directory if it as been created after start_time"""
    if PATH_GUI.is_dir():
        if get_birthtime(PATH_GUI) >= start_time:
            shutil.rmtree(PATH_GUI)
        else:
            children = list(PATH_GUI.iterdir())
            for child in children:
                if get_birthtime(child) >= start_time:
                    if child.is_dir():
                        shutil.rmtree(child)
                    else:
                        child.unlink()


def get_birthtime(object_name: Path) -> datetime:
    """Return the birthtime of the given object"""
    try:
        birthtime = datetime.fromtimestamp(object_name.stat().st_birthtime, tz=UTC)
    except AttributeError:
        birthtime = datetime.fromtimestamp(object_name.stat().st_atime, tz=UTC)
    return birthtime


if __name__ == "__main__":
    unittest.main()
