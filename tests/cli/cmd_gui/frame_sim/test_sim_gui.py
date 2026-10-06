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

import importlib
import os
import shutil
import sys
import tkinter as tk
import unittest
from datetime import UTC, datetime
from pathlib import Path
from unittest.mock import MagicMock, call, patch

try:
    from cli.cmd_gui import frame_base
    from cli.cmd_gui.frame_sim import sim_gui
    from cli.helpers.project_context import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui import frame_base
    from cli.cmd_gui.frame_sim import sim_gui
    from cli.helpers.project_context import PROJECT_BUILD_ROOT

RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")
PATH_GUI = PROJECT_BUILD_ROOT / "sim_frame"


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
@patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.update_text")
class TestCheckThreads(unittest.TestCase):
    """Test of the 'check_threads' function of the SimulateBmsFrame class"""

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        self.frame = sim_gui.SimulateBmsFrame(self.root, text)

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.after")
    def test_check_threads_alive(
        self, mock_after: MagicMock, mock_update_text: MagicMock
    ) -> None:
        """Both Threads are alive"""
        self.frame.bms_process = MagicMock()
        self.frame.bms_process.is_alive.return_value = True
        self.frame.unit_process = MagicMock()
        self.frame.unit_process.is_alive.return_value = True
        self.frame.check_threads()

        mock_update_text.assert_called_once()
        self.frame.bms_process.is_alive.assert_called_once()
        self.frame.unit_process.is_alive.assert_not_called()
        mock_after.assert_called_once_with(50, self.frame.check_threads)

    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.after")
    def test_check_threads_unit_alive(
        self, mock_after: MagicMock, mock_update_text: MagicMock
    ) -> None:
        """Unit Thread is alive"""
        self.frame.bms_process = MagicMock()
        self.frame.bms_process.is_alive.return_value = False
        self.frame.unit_process = MagicMock()
        self.frame.unit_process.is_alive.return_value = True
        self.frame.check_threads()

        mock_update_text.assert_called_once()
        self.frame.bms_process.is_alive.assert_called_once()
        self.frame.unit_process.is_alive.assert_called_once()
        mock_after.assert_called_once_with(50, self.frame.check_threads)

    def test_check_threads_dead(self, mock_update_text: MagicMock) -> None:
        """Threads are not alive"""
        self.frame.bms_process = MagicMock()
        self.frame.bms_process.is_alive.return_value = False
        self.frame.unit_process = MagicMock()
        self.frame.unit_process.is_alive.return_value = False
        self.frame.sim_active = True
        self.frame.check_threads()

        self.frame.bms_process.is_alive.assert_called_once()
        self.frame.unit_process.is_alive.assert_called_once()
        mock_update_text.assert_has_calls(
            [call(), call("Simulation has terminated.\n")]
        )
        self.assertFalse(self.frame.sim_active)

    def test_check_threads_dead_can_com(self, mock_update_text: MagicMock) -> None:
        """Threads are not alive and the 'can_com_*' attributes exist"""
        self.frame.bms_process = MagicMock()
        self.frame.bms_process.is_alive.return_value = False
        self.frame.unit_process = MagicMock()
        self.frame.unit_process.is_alive.return_value = False
        self.frame.sim_active = True
        self.frame.can_com_bms = MagicMock()
        self.frame.can_com_unit = MagicMock()
        self.frame.check_threads()

        self.frame.bms_process.is_alive.assert_called_once()
        self.frame.unit_process.is_alive.assert_called_once()
        mock_update_text.assert_has_calls(
            [call(), call("Simulation has terminated.\n")]
        )
        self.assertFalse(self.frame.sim_active)
        self.assertFalse(hasattr(self.frame, "can_com_bms"))
        self.assertFalse(hasattr(self.frame, "can_com_unit"))


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
@patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.update_text")
class TestStartStop(unittest.TestCase):
    """Test of the 'start_stop_sim_cb' function of the SimulateBmsFrame class"""

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        self.frame = sim_gui.SimulateBmsFrame(self.root, text)

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    def test_start_stop_sim_no_config(self, mock_update_text: MagicMock) -> None:
        """Missing CAN Bus configurations"""
        self.frame.start_stop_sim_cb()
        mock_update_text.assert_called_once_with(
            "Add CAN Configurations before starting the Simulation.\n"
        )

    def test_start_stop_sim_active(self, mock_update_text: MagicMock) -> None:
        """sim_active is True"""
        self.frame.can_bus_bms = MagicMock()
        self.frame.can_bus_unit = MagicMock()
        self.frame.sim_active = True
        self.frame.start_stop_sim_cb()
        mock_update_text.assert_called_once_with("Stopping the Simulation...\n")

    def test_start_stop_sim_active_can_com(self, mock_update_text: MagicMock) -> None:
        """sim_active is True and 'can_com_*' attributes exist"""
        self.frame.can_bus_bms = MagicMock()
        self.frame.can_bus_unit = MagicMock()
        self.frame.can_com_bms = MagicMock()
        self.frame.can_com_unit = MagicMock()
        self.frame.sim_active = True
        self.frame.start_stop_sim_cb()
        mock_update_text.assert_called_once_with("Stopping the Simulation...\n")
        self.frame.can_com_bms.shutdown.assert_called_once_with(block=True, timeout=1)
        self.frame.can_com_unit.shutdown.assert_called_once_with(block=True, timeout=1)

    @patch("cli.cmd_gui.frame_sim.sim_gui.Thread")
    @patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.check_threads")
    def test_start_stop_sim_inactive(
        self,
        mock_check_threads: MagicMock,
        mock_thread: MagicMock,
        mock_update_text: MagicMock,
    ) -> None:
        """sim_active is False and 'can_com_*' attributes exist"""
        mock_bms_process = MagicMock()
        mock_unit_process = MagicMock()
        mock_thread.side_effect = [mock_bms_process, mock_unit_process]
        self.frame.can_bus_bms = MagicMock()
        self.frame.can_bus_unit = MagicMock()
        self.frame.sim_active = False
        self.frame.start_stop_sim_cb()

        mock_update_text.assert_called_once_with("Starting the Simulation...\n")
        self.assertTrue(self.frame.sim_active)
        mock_bms_process.start.assert_called_once()
        mock_unit_process.start.assert_called_once()
        mock_check_threads.assert_called_once()


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
@patch("cli.cmd_gui.frame_sim.sim_gui.SimulateBmsFrame.update_text")
class TestSendMsg(unittest.TestCase):
    """Test of the 'send_msg_cb' function of the SimulateBmsFrame class"""

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        with patch("cli.helpers.project_context.PROJECT_BUILD_ROOT", new=PATH_GUI):
            importlib.reload(frame_base)

    def setUp(self) -> None:  # noqa: D102
        self.start_time = datetime.now(tz=UTC)
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        self.frame = sim_gui.SimulateBmsFrame(self.root, text)

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        importlib.reload(frame_base)

    def tearDown(self) -> None:  # noqa: D102
        self.root.update()
        self.root.destroy()
        remove_data(self.start_time)

    def test_send_msg_cb_inactive(self, mock_update_text: MagicMock) -> None:
        """sim_active is False"""
        self.frame.sim_active = False
        self.frame.send_msg_cb()
        mock_update_text.assert_called_once_with(
            "Start the Simulation before sending messages.\n"
        )

    def test_send_msg_cb_error(self, mock_update_text: MagicMock) -> None:
        """sim_active is True and sending causes an error"""
        self.frame.sim_active = True
        self.frame.can_com_unit = MagicMock()
        self.frame.can_com_unit.write.side_effect = Exception("Error")
        self.frame.send_msg_cb()
        self.frame.can_com_unit.write.assert_called_once()
        mock_update_text.assert_has_calls([call("Sending message.\n"), call("Error\n")])

    def test_send_msg_cb_active(self, mock_update_text: MagicMock) -> None:
        """sim_active is True"""
        self.frame.sim_active = True
        self.frame.can_com_unit = MagicMock()
        self.frame.send_msg_cb()
        mock_update_text.assert_called_once_with("Sending message.\n")
        self.frame.can_com_unit.write.assert_called_once()


class TestUpdateText(unittest.TestCase):
    """Test of the 'update_text' function of the SimulateBmsFrame class"""

    def test_no_input(self) -> None:
        """No file input given"""
        mock_sim_frame = MagicMock()
        sim_gui.SimulateBmsFrame.update_text(mock_sim_frame)
        mock_sim_frame.write_text_from_queue.assert_called_once()
        mock_sim_frame.write_text.assert_called_once()

    @patch("cli.cmd_gui.frame_sim.sim_gui.threading")
    def test_sub_thread(self, mock_threading: MagicMock) -> None:
        """current_thread is not main_thread"""
        mock_sim_frame = MagicMock()
        mock_threading.current_thread.return_value = "sub thread"
        mock_threading.main_thread.return_value = "main thread"
        sim_gui.SimulateBmsFrame.update_text(mock_sim_frame, "file input")
        mock_sim_frame.log_queue.put.assert_called_once_with("file input")
        mock_sim_frame.write_text_from_queue.assert_not_called()
        mock_sim_frame.write_text.assert_not_called()

    @patch("cli.cmd_gui.frame_sim.sim_gui.threading")
    def test_main_thread(self, mock_threading: MagicMock) -> None:
        """current_thread is main_thread"""
        mock_sim_frame = MagicMock()
        mock_threading.current_thread.return_value = "main thread"
        mock_threading.main_thread.return_value = "main thread"
        sim_gui.SimulateBmsFrame.update_text(mock_sim_frame, "file input")
        mock_sim_frame.write_text_from_queue.assert_called_once()
        mock_sim_frame.write_text.assert_has_calls([call("file input"), call()])


class TestSimulateBmsFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the SimulateBmsFrame class"""

    @patch("cli.cmd_gui.frame_sim.sim_gui.CAN")
    @patch("cli.cmd_gui.frame_sim.sim_gui.sim_unit")
    def test_run_unit_sim(self, mock_sim_unit: MagicMock, mock_can: MagicMock) -> None:
        """Test 'run_unit_sim' function"""
        mock_can.return_value = "can"
        mock_sim_frame = MagicMock()
        mock_sim_frame.can_bus_unit = "can_config"
        sim_gui.SimulateBmsFrame.run_unit_sim(mock_sim_frame)

        mock_can.assert_called_once_with("CAN Bus Unit", "can_config")
        mock_sim_unit.assert_called_once_with("can", mock_sim_frame.log_queue)
        mock_sim_frame.can_com_bms.shutdown.assert_called_once_with(
            block=True, timeout=1
        )

    @patch("cli.cmd_gui.frame_sim.sim_gui.CAN")
    @patch("cli.cmd_gui.frame_sim.sim_gui.sim_bms")
    def test_run_bms_sim(self, mock_sim_bms: MagicMock, mock_can: MagicMock) -> None:
        """Test 'run_bms_sim' function"""
        mock_can.return_value = "can"
        mock_sim_frame = MagicMock()
        mock_sim_frame.can_bus_bms = "can_config"
        sim_gui.SimulateBmsFrame.run_bms_sim(mock_sim_frame)

        mock_can.assert_called_once_with("CAN Bus BMS", "can_config")
        mock_sim_bms.assert_called_once_with("can", mock_sim_frame.log_queue)
        mock_sim_frame.can_com_unit.shutdown.assert_called_once_with(
            block=True, timeout=1
        )

    def test_check_threads_alive(self) -> None:
        """Test 'check_threads' function when the Threads are alive"""
        mock_bms_process = MagicMock()
        mock_bms_process.is_alive.return_value = True
        mock_unit_process = MagicMock()
        mock_unit_process.is_alive.return_value = True
        mock_sim_frame = MagicMock()
        mock_sim_frame.bms_process = mock_bms_process
        mock_sim_frame.unit_process = mock_unit_process
        sim_gui.SimulateBmsFrame.check_threads(mock_sim_frame)

        mock_sim_frame.update_text.assert_called_once()
        mock_sim_frame.after.assert_called_once_with(50, mock_sim_frame.check_threads)
        mock_bms_process.is_alive.assert_called_once()
        mock_unit_process.is_alive.assert_not_called()

    def test_check_threads_dead_alive(self) -> None:
        """Test 'check_threads' function when only one Thread is alive"""
        mock_bms_process = MagicMock()
        mock_bms_process.is_alive.return_value = False
        mock_unit_process = MagicMock()
        mock_unit_process.is_alive.return_value = True
        mock_sim_frame = MagicMock()
        mock_sim_frame.bms_process = mock_bms_process
        mock_sim_frame.unit_process = mock_unit_process
        sim_gui.SimulateBmsFrame.check_threads(mock_sim_frame)

        mock_sim_frame.update_text.assert_called_once()
        mock_sim_frame.after.assert_called_once_with(50, mock_sim_frame.check_threads)
        mock_bms_process.is_alive.assert_called_once()
        mock_unit_process.is_alive.assert_called_once()

    def test_check_threads_dead(self) -> None:
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
        mock_sim_frame.update_text.assert_has_calls(
            [call(), call("Simulation has terminated.\n")]
        )
        self.assertFalse(mock_sim_frame.sim_active)

    def test_write_text_from_queue(self) -> None:
        """Test 'write_text_from_queue' function"""
        mock_sim_frame = MagicMock()
        mock_log_queue = MagicMock()
        mock_log_queue.empty.side_effect = [False, True]
        mock_log_queue.get_nowait.return_value = "content"
        mock_sim_frame.log_queue = mock_log_queue
        sim_gui.SimulateBmsFrame.write_text_from_queue(mock_sim_frame)
        mock_sim_frame.write_text.assert_called_once_with("content")

    @patch("builtins.super")
    def test_on_close_active(self, mock_super: MagicMock) -> None:
        """Test 'on_close' function when simulation is active"""
        mock_sim_frame = MagicMock()
        mock_sim_frame.sim_active = True
        sim_gui.SimulateBmsFrame.on_close(mock_sim_frame)
        mock_sim_frame.start_stop_sim_cb.assert_called_once()
        mock_super.return_value.on_close.assert_called_once()

    @patch("builtins.super")
    def test_on_close_inactive(self, mock_super: MagicMock) -> None:
        """Test 'on_close' function when simulation is inactive"""
        mock_sim_frame = MagicMock()
        mock_sim_frame.sim_active = False
        sim_gui.SimulateBmsFrame.on_close(mock_sim_frame)
        mock_sim_frame.start_stop_sim_cb.assert_not_called()
        mock_super.return_value.on_close.assert_called_once()


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
