#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""This is the foxBMS GUI

https://foxbms.org
"""

import os
import webbrowser
import wx
import wx.adv

import fgui

from log_parser import LogParserFrame
from info_dialog import FoxbmsInfoDialog

__version__ = fgui.__version__
__appname__ = fgui.__appname__
__author__ = fgui.__author__
__copyright__ = fgui.__copyright__
__author__ = fgui.__author__
__email__ = fgui.__email__

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
FOXBMS_LOGO = os.path.join(SCRIPT_DIR, "..", "..", "docs", "_static", "foxbms250px.png")


class foxBMSMainFrame(wx.Frame):
    """Main frame to construct the foxBMS GUI frame"""

    # pylint: disable=too-many-ancestors,invalid-name

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.initialize_gui()

    def initialize_gui(self):
        """initializes the GUI elements"""
        self.panel = wx.Panel(self, wx.ID_ANY)

        menu_bar = wx.MenuBar()
        file_menu = wx.Menu()
        exit_menu_item = file_menu.Append(
            wx.Window.NewControlId(), "Exit", "Exit the application"
        )
        menu_bar.Append(file_menu, "&File")
        self.Bind(wx.EVT_MENU, self.cb_on_exit, exit_menu_item)

        log_parser_menu = wx.Menu()
        log_parser_item = log_parser_menu.Append(
            wx.Window.NewControlId(), "Start", "Start a new Log Parser"
        )
        menu_bar.Append(log_parser_menu, "&Log Parser")
        self.Bind(wx.EVT_MENU, self.cb_start_log_parser, log_parser_item)

        help_menu = wx.Menu()
        show_info_item = help_menu.Append(wx.Window.NewControlId(), "Info", "Info")
        open_documentation_item = help_menu.Append(
            wx.Window.NewControlId(), "Documentation", "Documentation"
        )
        menu_bar.Append(help_menu, "?")
        self.Bind(wx.EVT_MENU, self.cb_show_info, show_info_item)
        self.Bind(wx.EVT_MENU, self.cb_open_documentation, open_documentation_item)
        self.SetMenuBar(menu_bar)

        # Add logo
        _icon = wx.Icon()
        logo_img = wx.Image(FOXBMS_LOGO)
        logo_img_size = logo_img.GetSize()
        resized = logo_img_size / 5
        logo_img.Rescale(resized[0], resized[1])
        image = wx.Bitmap(logo_img)
        _icon.CopyFromBitmap(image)
        self.SetIcon(_icon)

        # all gui elements are now initalized, so we can show the GUI
        self.SetTitle("foxBMS")
        self.Centre()
        self.Show(True)

    def cb_on_exit(self, event):
        """Closes the application window"""
        self.Close()

    @classmethod
    def cb_start_log_parser(cls, event):
        """Start Log Parser"""
        title = "Log Parser"
        LogParserFrame(title=title)

    @classmethod
    def cb_open_documentation(cls, event):
        """Shows the foxBMS documentation from local source if it exists, from
        web if it does not"""
        doc = os.path.join(SCRIPT_DIR, "..", "..", "build", "docs", "index.html")
        if not os.path.isfile(doc):
            doc = "https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/docs/html/latest/"
        webbrowser.open(doc)

    @classmethod
    def cb_show_info(cls, event):
        """Shows the program information"""
        about_dialog = FoxbmsInfoDialog(None, title="About foxBMS 2")
        about_dialog.ShowModal()
        about_dialog.Destroy()


def main():
    """Starts the application"""
    app = wx.App(False)
    foxBMSMainFrame(None)
    app.MainLoop()


if __name__ == "__main__":
    main()
