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

RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestInit(unittest.TestCase):
    """Test Initialization of CanConfigWindow class"""

    @classmethod
    def setUpClass(cls) -> None:  # noqa: D102
        cls.root = tk.Tk()
        cls.root.withdraw()
        with patch("pathlib.Path.mkdir"), patch("pathlib.Path.write_text"):
            cls.frame = sim_gui.SimulateBmsFrame(cls.root, MagicMock())

    @classmethod
    def tearDownClass(cls) -> None:  # noqa: D102
        cls.root.update()
        cls.root.destroy()

    def test_init_new(self) -> None:
        """Test init when 'can_bus_*' attributes do not exist"""
        window = window_can_config.CanConfigWindow(self.frame, self.root)
        window.withdraw()

    def test_init(self) -> None:
        """Test init when 'can_bus_*' attributes exist"""
        mock_can_bus_bms = MagicMock()
        mock_can_bus_bms.interface = "interface"
        mock_can_bus_bms.channel = "channel"
        mock_can_bus_bms.bitrate = "bitrate"
        mock_can_bus_unit = MagicMock()
        mock_can_bus_unit.interface = "interface"
        mock_can_bus_unit.channel = "channel"
        mock_can_bus_unit.bitrate = "bitrate"
        self.frame.can_bus_bms = mock_can_bus_bms
        self.frame.can_bus_unit = mock_can_bus_unit

        with patch.object(window_can_config.CanConfigWindow, "change_interface_cb"):
            window = window_can_config.CanConfigWindow(self.frame, self.root)
        window.withdraw()

        del self.frame.can_bus_bms
        del self.frame.can_bus_unit

    def test_init_channels(self) -> None:
        """Test init when 'can_bus_*' attributes do not exist
        and SUPPORTED_CHANNELS only has one element
        """
        correct_channels = window_can_config.fcan.SUPPORTED_CHANNELS[
            window_can_config.fcan.SUPPORTED_INTERFACES[0]
        ]
        window_can_config.fcan.SUPPORTED_CHANNELS[
            window_can_config.fcan.SUPPORTED_INTERFACES[0]
        ] = ["Channel 1"]
        window = window_can_config.CanConfigWindow(self.frame, self.root)
        window.withdraw()
        window_can_config.fcan.SUPPORTED_CHANNELS[
            window_can_config.fcan.SUPPORTED_INTERFACES[0]
        ] = correct_channels


class TestChangeInterface(unittest.TestCase):
    """Test 'change_interface_cb' function"""

    def test_default(self) -> None:
        """Interface is in 'DEFAULT_CHANNELS"""
        mock_interface = MagicMock()
        mock_interface.get.return_value = "pcan"
        mock_channel = MagicMock()
        can_bus_config = {"interface": mock_interface, "channel": mock_channel}
        mock_window = MagicMock()
        window_can_config.CanConfigWindow.change_interface_cb(
            mock_window, tk.Event(), can_bus_config
        )
        mock_interface.get.assert_called_once()
        mock_channel.set.assert_called_once()

    def test_custom(self) -> None:
        """Interface is not in 'DEFAULT_CHANNELS"""
        mock_interface = MagicMock()
        mock_interface.get.return_value = "custom"
        mock_channel = MagicMock()
        can_bus_config = {"interface": mock_interface, "channel": mock_channel}
        mock_window = MagicMock()
        window_can_config.CanConfigWindow.change_interface_cb(
            mock_window, tk.Event(), can_bus_config
        )
        mock_interface.get.assert_called_once()
        mock_channel.set.assert_not_called()


class TestAddCanBus(unittest.TestCase):
    """Test 'add_can_bus_cb' function"""

    @patch("cli.cmd_gui.frame_sim.window_can_config.fcan.CanBusConfig")
    def test_error(self, mock_can_bus_config: MagicMock) -> None:
        """Create CanBusConfig raises SystemExit"""
        mock_can_bus_config.side_effect = SystemExit("Error")
        mock_interface = MagicMock()
        mock_interface.get.return_value = "interface"
        mock_channel = MagicMock()
        mock_channel.get.return_value = "channel"
        mock_bitrate = MagicMock()
        mock_bitrate.get.return_value = "0"
        config = {
            "interface": mock_interface,
            "channel": mock_channel,
            "bitrate": mock_bitrate,
        }
        mock_window = MagicMock()
        window_can_config.CanConfigWindow.add_can_bus_cb(
            mock_window, tk.Event(), config
        )
        mock_can_bus_config.assert_called_once_with(
            interface="interface",
            bitrate=0,
            channel="channel",
            dbc=window_can_config.APP_DBC_FILE,
        )
        mock_window.parent.write_text.assert_called_once_with("Invalid input: Error.\n")

    @patch("cli.cmd_gui.frame_sim.window_can_config.fcan.CanBusConfig")
    def test_bms(self, mock_can_bus_config: MagicMock) -> None:
        """Create CanBusConfig for BMS"""
        mock_can_bus_config.return_value = "can_config"
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
        self.assertEqual(mock_window.parent.can_bus_bms, "can_config")
        mock_window.parent.write_text.assert_called_once_with(
            "Configuration has been added.\n"
        )

    @patch("cli.cmd_gui.frame_sim.window_can_config.fcan.CanBusConfig")
    def test_unit(self, mock_can_bus_config: MagicMock) -> None:
        """Create CanBusConfig for unit"""
        mock_can_bus_config.return_value = "can_config"
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
        self.assertEqual(mock_window.parent.can_bus_unit, "can_config")
        mock_window.parent.write_text.assert_called_once_with(
            "Configuration has been added.\n"
        )

    @patch("cli.cmd_gui.frame_sim.window_can_config.fcan.CanBusConfig")
    def test_invalid(self, mock_can_bus_config: MagicMock) -> None:
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
