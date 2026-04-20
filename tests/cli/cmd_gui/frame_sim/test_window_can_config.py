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

"""Testing file 'cli/cmd_gui/frame_sim/window_can_config.py'."""

import os
import sys
import tkinter as tk
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_gui.frame_sim import sim_gui, window_can_config
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_sim import sim_gui, window_can_config

RUN_TESTS = os.environ.get("DISPLAY", False) or sys.platform.startswith("win32")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestInit(unittest.TestCase):
    """Test Initialization of CanConfigWindow class"""

    @classmethod
    def setUpClass(cls):
        cls.root = tk.Tk()
        cls.root.withdraw()
        cls.frame = sim_gui.SimulateBmsFrame(cls.root, MagicMock())

    @classmethod
    def tearDownClass(cls):
        cls.root.update()
        cls.root.destroy()

    def test_init_new(self):
        """Test init when 'can_bus_*' attributes do not exist"""
        window = window_can_config.CanConfigWindow(self.frame, self.root)
        window.withdraw()

    @patch(
        "cli.cmd_gui.frame_sim.window_can_config.CanConfigWindow.change_interface_cb"
    )
    def test_init(self, _: MagicMock):
        """Test init when 'can_bus_*' attributes exist"""
        self.frame.can_bus_bms = MagicMock()
        self.frame.can_bus_bms.interface = "interface"
        self.frame.can_bus_bms.channel = "channel"
        self.frame.can_bus_bms.bitrate = "bitrate"
        self.frame.can_bus_unit = MagicMock()
        self.frame.can_bus_unit.interface = "interface"
        self.frame.can_bus_unit.channel = "channel"
        self.frame.can_bus_unit.bitrate = "bitrate"

        window = window_can_config.CanConfigWindow(self.frame, self.root)
        window.withdraw()

        del self.frame.can_bus_bms
        del self.frame.can_bus_unit


class TestChangeInterfaceCb(unittest.TestCase):
    """Test 'change_interface_cb' function"""

    def test_default(self):
        """Interface is in 'DEFAULT_CHANNELS"""
        mock_config = MagicMock()
        mock_config["interface"] = MagicMock()
        mock_config["interface"].get.return_value = "pcan"
        mock_config["channel"] = MagicMock()
        mock_window = MagicMock()
        window_can_config.CanConfigWindow.change_interface_cb(
            mock_window, tk.Event(), mock_config
        )
        mock_config["interface"].get.assert_called_once()
        mock_config["channel"].set.assert_called_once()

    def test_custom(self):
        """Interface is not in 'DEFAULT_CHANNELS"""
        mock_config = MagicMock()
        mock_config["interface"] = MagicMock()
        mock_config["interface"].get.return_value = "custom"
        mock_config["channel"] = MagicMock()
        mock_window = MagicMock()
        window_can_config.CanConfigWindow.change_interface_cb(
            mock_window, tk.Event(), mock_config
        )
        mock_config["interface"].get.assert_called_once()
        mock_config["channel"].set.assert_not_called()


class TestAddCanBusCb(unittest.TestCase):
    """Test 'add_can_bus_cb' function"""

    @patch("cli.cmd_gui.frame_sim.window_can_config.fcan.CanBusConfig")
    def test_error(self, mock_can_bus_config: MagicMock):
        """Create CanBusConfig raises SystemExit"""
        mock_can_bus_config.side_effect = SystemExit("Error")
        mock_interface = MagicMock()
        mock_interface.get.return_value = "interface"
        mock_channel = MagicMock()
        mock_channel.get.return_value = "channel"
        mock_bitrate = MagicMock()
        mock_interface.get.return_value = "0"
        config = {
            "interface": mock_interface,
            "channel": mock_channel,
            "bitrate": mock_bitrate,
        }
        mock_window = MagicMock()
        window_can_config.CanConfigWindow.add_can_bus_cb(
            mock_window, tk.Event(), config
        )
        mock_can_bus_config.assert_called_once()
        mock_window.parent.write_text.assert_called_once_with("Invalid input: Error.\n")

    @patch("cli.cmd_gui.frame_sim.window_can_config.fcan.CanBusConfig")
    def test_bms(self, mock_can_bus_config: MagicMock):
        """Create CanBusConfig for BMS"""
        mock_interface = MagicMock()
        mock_interface.get.return_value = "interface"
        mock_channel = MagicMock()
        mock_channel.get.return_value = "channel"
        mock_bitrate = MagicMock()
        mock_interface.get.return_value = "0"
        config = {
            "interface": mock_interface,
            "channel": mock_channel,
            "bitrate": mock_bitrate,
            "can_bus_name": "bms",
        }
        mock_window = MagicMock()
        window_can_config.CanConfigWindow.add_can_bus_cb(
            mock_window, tk.Event(), config
        )
        mock_can_bus_config.assert_called_once()
        mock_window.parent.write_text.assert_called_once_with(
            "Configuration has been added.\n"
        )

    @patch("cli.cmd_gui.frame_sim.window_can_config.fcan.CanBusConfig")
    def test_unit(self, mock_can_bus_config: MagicMock):
        """Create CanBusConfig for unit"""
        mock_interface = MagicMock()
        mock_interface.get.return_value = "interface"
        mock_channel = MagicMock()
        mock_channel.get.return_value = "channel"
        mock_bitrate = MagicMock()
        mock_interface.get.return_value = "0"
        config = {
            "interface": mock_interface,
            "channel": mock_channel,
            "bitrate": mock_bitrate,
            "can_bus_name": "unit",
        }
        mock_window = MagicMock()
        window_can_config.CanConfigWindow.add_can_bus_cb(
            mock_window, tk.Event(), config
        )
        mock_can_bus_config.assert_called_once()
        mock_window.parent.write_text.assert_called_once_with(
            "Configuration has been added.\n"
        )

    @patch("cli.cmd_gui.frame_sim.window_can_config.fcan.CanBusConfig")
    def test_invalid(self, mock_can_bus_config: MagicMock):
        """Create CanBusConfig for invalid CAN Bus"""
        mock_interface = MagicMock()
        mock_interface.get.return_value = "interface"
        mock_channel = MagicMock()
        mock_channel.get.return_value = "channel"
        mock_bitrate = MagicMock()
        mock_interface.get.return_value = "0"
        config = {
            "interface": mock_interface,
            "channel": mock_channel,
            "bitrate": mock_bitrate,
            "can_bus_name": "invalid",
        }
        mock_window = MagicMock()
        window_can_config.CanConfigWindow.add_can_bus_cb(
            mock_window, tk.Event(), config
        )
        mock_can_bus_config.assert_called_once()
        mock_window.parent.write_text.assert_called_once_with(
            "Configuration has been added.\n"
        )


if __name__ == "__main__":
    unittest.main()
