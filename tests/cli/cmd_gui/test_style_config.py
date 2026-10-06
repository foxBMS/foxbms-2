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

"""Testing file 'cli/cmd_gui/style_config.py'."""

import os
import sys
import tkinter as tk
import unittest
from pathlib import Path
from unittest.mock import MagicMock, call, patch

try:
    from cli.cmd_gui import style_config
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_gui import style_config


RUN_TESTS = os.environ.get("DISPLAY", None) or sys.platform.startswith("win32")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestStyles(unittest.TestCase):
    """Test of the style configurations"""

    def test_get_text_font(self) -> None:
        """Test of the 'get_text_font' function"""
        font = style_config.get_text_font().actual()
        self.assertEqual(font["slant"], "roman")
        self.assertEqual(font["weight"], "normal")

    @patch("cli.cmd_gui.style_config.ttk.Style.configure")
    @patch("cli.cmd_gui.style_config.ttk.Style.map")
    def test_configure_styles(
        self, mock_map: MagicMock, mock_configure: MagicMock
    ) -> None:
        """Test of the 'configure_styles' function"""
        style_config.configure_styles()
        mock_map.assert_called_once_with(
            "TNotebook.Tab", foreground=[("selected", "darkblue")]
        )
        mock_configure.assert_has_calls(
            [
                call("Multiline.TButton", justify=tk.CENTER),
                call("Heading.TButton", justify=tk.CENTER, font="HeadingFont"),
                call("TNotebook.Tab", padding=(5, 0)),
            ]
        )


@patch("cli.cmd_gui.style_config.get_text_font")
@patch("cli.cmd_gui.style_config.font")
class TestStylesNoUiTestableMethods(unittest.TestCase):
    """Test of the style configuration without tkinter"""

    def test_get_heading_font(
        self, mock_font: MagicMock, mock_get_text_font: MagicMock
    ) -> None:
        """Test of the 'get_heading_font' function"""
        text_font = {"family": "family", "size": "0"}
        mock_get_text_font.return_value.actual.return_value = text_font
        style_config.get_heading_font()
        mock_get_text_font.assert_called_once()
        mock_font.Font.assert_called_once_with(
            name="HeadingFont", family="family", size=1, weight="bold"
        )

    def test_get_heading_font_error(
        self, mock_font: MagicMock, mock_get_text_font: MagicMock
    ) -> None:
        """Test of the 'get_heading_font' function
        when creating the font throws an error
        """
        text_font = {"family": "family", "size": "0"}
        mock_get_text_font.return_value.actual.return_value = text_font
        mock_font.Font.side_effect = tk.TclError()
        mock_font.nametofont.return_value = "HeadingFont"
        font = style_config.get_heading_font()
        self.assertEqual(font, "HeadingFont")
        mock_get_text_font.assert_called_once()
        mock_font.nametofont.assert_called_once_with("HeadingFont")
        mock_font.Font.assert_called_once_with(
            name="HeadingFont", family="family", size=1, weight="bold"
        )

    def test_get_italic_font(
        self, mock_font: MagicMock, mock_get_text_font: MagicMock
    ) -> None:
        """Test of the 'get_italic_font' function"""
        text_font = {"family": "family", "size": "0"}
        mock_get_text_font.return_value.actual.return_value = text_font
        style_config.get_italic_font()
        mock_get_text_font.assert_called_once()
        mock_font.Font.assert_called_once_with(
            name="ItalicFont", family="family", size="0", slant="italic"
        )

    def test_get_italic_font_error(
        self, mock_font: MagicMock, mock_get_text_font: MagicMock
    ) -> None:
        """Test of the 'get_italic_font' function
        when creating the font throws an error
        """
        text_font = {"family": "family", "size": "0"}
        mock_get_text_font.return_value.actual.return_value = text_font
        mock_font.Font.side_effect = tk.TclError()
        mock_font.nametofont.return_value = "ItalicFont"
        font = style_config.get_italic_font()
        self.assertEqual(font, "ItalicFont")
        mock_get_text_font.assert_called_once()
        mock_font.nametofont.assert_called_once_with("ItalicFont")
        mock_font.Font.assert_called_once_with(
            name="ItalicFont", family="family", size="0", slant="italic"
        )


if __name__ == "__main__":
    unittest.main()
