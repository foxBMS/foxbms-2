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

"""Shared functionality for GUI frames writing to the output text widget."""

import tkinter as tk
from tkinter import ttk
from typing import TextIO

from ..helpers.project_context import PROJECT_BUILD_ROOT


# pylint: disable-next=too-many-ancestors
class BaseFrame(ttk.Frame):
    """Base class for frames that write into a shared text widget."""

    def __init__(
        self,
        parent: ttk.Notebook,
        text_widget: tk.Text,
        output_file_name: str,
    ) -> None:
        super().__init__(parent)
        self.parent = parent
        self.text = text_widget
        self.text_index: int = 0
        self.file_path = PROJECT_BUILD_ROOT / "gui" / output_file_name
        self.file_path.parent.mkdir(parents=True, exist_ok=True)
        self.file_path.touch()
        self.file_stream: TextIO

    def write_text(self, file_input: str | None = None) -> None:
        """Write incremental log content into the shared text widget."""
        if file_input is not None:
            if hasattr(self, "file_stream") and not self.file_stream.closed:
                self.file_stream.write(file_input)
            else:
                with open(
                    self.file_path, mode="a", encoding="utf-8", errors="ignore"
                ) as f:
                    f.write(file_input)
        _select = self.parent.select()  # type: ignore[no-untyped-call]
        if self != self.parent.nametowidget(_select):
            return
        self.text.config(state=tk.NORMAL)
        with open(self.file_path, encoding="utf-8", errors="ignore") as f:
            file_content = f.read()
            text_length = len(file_content)
            self.text.insert(tk.END, file_content[self.text_index :])
            if text_length > self.text_index:
                self.text_index = text_length
                self.text.see(tk.END)
        self.text.config(state=tk.DISABLED)

    def get_log_tail(self, number_of_lines: int = 10) -> str:
        """Return the last lines of the current log file for quick diagnostics."""
        if number_of_lines <= 0:
            return ""
        with open(self.file_path, mode="rb") as f:
            f.seek(0, 2)
            file_size = f.tell()
            if file_size == 0:
                return ""

            block_size = 4096
            chunks: list[bytes] = []
            newline_count = 0
            position = file_size
            while position > 0 and newline_count <= number_of_lines:
                read_size = min(block_size, position)
                position -= read_size
                f.seek(position)
                chunk = f.read(read_size)
                chunks.insert(0, chunk)
                newline_count += chunk.count(b"\n")

        lines = b"".join(chunks).decode("utf-8", errors="ignore").splitlines()
        return "\n".join(lines[-number_of_lines:])

    def reset_text(self) -> None:
        """Empty the text widget"""
        self.text_index = 0
        self.text.config(state=tk.NORMAL)
        self.text.delete("1.0", tk.END)
        self.text.config(state=tk.DISABLED)

    def on_close(self) -> None:
        """Run a cleanup hook called when the main window shuts down."""
        if hasattr(self, "file_stream") and not self.file_stream.closed:
            self.file_stream.close()
        if self.file_path.exists():
            self.file_path.unlink()
