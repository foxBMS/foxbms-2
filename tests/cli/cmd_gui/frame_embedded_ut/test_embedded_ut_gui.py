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

"""Testing file 'cli/cmd_gui/frame_embedded_ut/embedded_ut_gui.py'."""

import os
import sys
import tkinter as tk
import unittest
from pathlib import Path
from unittest.mock import MagicMock

try:
    from cli.cmd_gui.frame_embedded_ut import embedded_ut_gui
    from cli.helpers.misc import PROJECT_BUILD_ROOT
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_gui.frame_embedded_ut import embedded_ut_gui
    from cli.helpers.misc import PROJECT_BUILD_ROOT

RUN_TESTS = os.environ.get("DISPLAY", False) or sys.platform.startswith("win32")


@unittest.skipUnless(RUN_TESTS, "Non graphical tests only")
class TestEmbeddedUtFrame(unittest.TestCase):
    """Test of the EmbeddedUtFrame class"""

    def setUp(self):
        self.root = tk.Tk()
        self.root.withdraw()
        text = tk.Text()
        self.frame = embedded_ut_gui.EmbeddedUtFrame(self.root, text)

    def tearDown(self):
        self.root.update()
        self.root.destroy()
        file_output = Path(PROJECT_BUILD_ROOT / "output_gui_ut.txt")
        if file_output.exists():
            file_output.unlink()

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
        """Test 'write_text' function when EmbeddedUtFrame is not selected"""
        mock_select = MagicMock()
        mock_select.return_value = ""
        self.frame.parent.select = mock_select
        self.frame.file_path.write_text("New content.", encoding="utf-8")
        self.frame.write_text()
        self.assertEqual("\n", self.frame.text.get("1.0", tk.END))
        self.assertEqual(0, self.frame.text_index)


class TestEmbeddedUtFrameNoUiTestableMethods(unittest.TestCase):
    """Test of the EmbeddedUtFrame class"""

    def setUp(self):
        PROJECT_BUILD_ROOT.mkdir(parents=True, exist_ok=True)

    def tearDown(self):
        for file_path in PROJECT_BUILD_ROOT.glob("output_ut_*"):
            if file_path.suffix == ".txt" and file_path.exists():
                file_path.unlink()

    def test_write_text_empty(self):
        """Test 'write_text' function when the file is empty"""
        mock_embedded_ut_frame = MagicMock()
        mock_embedded_ut_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_ut_write_text_empty.txt"
        )
        mock_embedded_ut_frame.file_path.touch()
        mock_embedded_ut_frame.text = MagicMock()
        mock_embedded_ut_frame.text_index = MagicMock()
        embedded_ut_gui.EmbeddedUtFrame.write_text(mock_embedded_ut_frame)

    def test_write_text(self):
        """Test 'write_text' function when the file is not empty"""
        mock_embedded_ut_frame = MagicMock()
        mock_embedded_ut_frame.file_path = Path(
            PROJECT_BUILD_ROOT / "output_ut_write_text.txt"
        )
        mock_embedded_ut_frame.file_path.write_text("New content.", encoding="utf-8")
        mock_embedded_ut_frame.text = MagicMock()
        mock_embedded_ut_frame.text_index = MagicMock()
        embedded_ut_gui.EmbeddedUtFrame.write_text(mock_embedded_ut_frame)


if __name__ == "__main__":
    unittest.main()
