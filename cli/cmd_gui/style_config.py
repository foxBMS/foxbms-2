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

"""Configures all fonts and styles for the GUI"""

import tkinter as tk
from tkinter import font, ttk


def get_text_font() -> font.Font:
    """Set and return standard text font"""
    return font.nametofont("TkTextFont")


def get_italic_font() -> font.Font:
    """Set and return italic font"""
    text_font = get_text_font().actual()
    try:
        italic_font = font.Font(
            name="ItalicFont",
            family=text_font["family"],
            size=text_font["size"],
            slant="italic",
        )
    except tk.TclError:
        italic_font = font.nametofont("ItalicFont")
    return italic_font


def get_heading_font() -> font.Font:
    """Set and return font for headings"""
    text_font = get_text_font().actual()
    try:
        heading_font = font.Font(
            name="HeadingFont",
            family=text_font["family"],
            size=int(text_font["size"]) + 1,
            weight="bold",
        )
    except tk.TclError:
        heading_font = font.nametofont("HeadingFont")
    return heading_font


def configure_styles() -> None:
    """Configure all styles used in the GUI"""
    style = ttk.Style()
    # Buttons
    style.configure("Multiline.TButton", justify=tk.CENTER)
    style.configure("Heading.TButton", justify=tk.CENTER, font="HeadingFont")

    # Notebook
    style.map("TNotebook.Tab", foreground=[("selected", "darkblue")])
    style.configure("TNotebook.Tab", padding=(5, 0))
