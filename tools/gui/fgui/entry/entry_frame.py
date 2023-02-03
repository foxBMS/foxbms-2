#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Wrapper frame as central entry point to the foxBMS GUI software to select
the actions that should be taken:
- LiveViewAndControl
- LogParser
"""

import logging

import wx
from fgui import PROJECT_ROOT, __appname__, __version__
from fgui.entry.helper import INSTRUCTIONS
from fgui.log_parser.log_parser import LogParserFrame
from fgui.lvac.lvac_frame import LiveViewAndControlMainFrame
from fgui.misc.gui_helpers import cb_open_documentation, cb_show_info, get_icon
from fgui.misc.logo import logo_foxbms
from fgui.workers.can_node_worker import CanAdapterProcess

FRAME_STYLE = wx.DEFAULT_FRAME_STYLE & (~wx.MAXIMIZE_BOX) ^ wx.RESIZE_BORDER


class EntryFrame(wx.Frame):  # pylint: disable=too-many-ancestors
    """Main frame to construct the foxBMS GUI frame"""

    def __init__(
        self,
        parent=None,
        title=f"{__appname__} - {__version__}",
        can_process: CanAdapterProcess = None,
    ):
        wx.Frame.__init__(self, parent, -1, title, style=FRAME_STYLE)
        self._can_process = can_process
        self._lvac_frame = None
        self._lp_frame = None
        self.status_bar = None
        self.initialize_gui()

    def start_lvac_cb(self, event):
        """start the LVAC frame and locks the accompanying subprocess."""
        if self._can_process:
            if self._can_process.locked.acquire(block=False):
                self._lvac_frame = LiveViewAndControlMainFrame(
                    can_process=self._can_process
                )
            else:
                logging.debug("CAN process already in use.")
        else:
            wx.MessageBox(
                (
                    "foxBMS GUI has been started without selecting a CAN "
                    "adapter.\nIn oder to use LiveViewAndControl restart the "
                    "application and chose a CAN adapter."
                ),
                "Error",
                wx.OK | wx.ICON_ERROR,
            )

    def start_lp_cb(self, event):
        """Start Log Parser"""
        if self._lp_frame:
            self._lp_frame.SetFocus()
        else:
            self._lp_frame = LogParserFrame()

    def initialize_gui(self):
        """Add GUI layout"""
        self.SetIcon(get_icon())
        self._add_menu_bar()
        self._add_status_bar()

        self.panel = wx.Panel(self)

        sizer = wx.BoxSizer(wx.VERTICAL)

        # -----------------------
        sub_sizer1 = wx.BoxSizer(wx.HORIZONTAL)

        text1 = wx.StaticText(self.panel, label=INSTRUCTIONS)
        sub_sizer1.Add(text1, flag=wx.ALL, border=5)

        logo = wx.Image(logo_foxbms.GetImage())
        image_bitmap = wx.StaticBitmap(self.panel, wx.ID_ANY, wx.Bitmap(logo))

        sub_sizer1.Add(image_bitmap, proportion=1, flag=wx.ALL, border=5)
        # -----------------------

        sizer.Add(sub_sizer1, flag=wx.TOP | wx.LEFT | wx.EXPAND, border=10)

        self.panel.SetSizer(sizer)
        self.SetSize(500, 220)
        self.Centre()
        self.Show()

    def _add_menu_bar(self):
        mb = wx.MenuBar()  # pylint: disable=invalid-name

        # 'File' menu
        fm = wx.Menu()  # pylint: disable=invalid-name

        ctrl_id = wx.Window.NewControlId()
        fmi_lvac = fm.Append(ctrl_id, "LiveViewAndControl", "Show BMS values")
        self.Bind(wx.EVT_MENU, self.start_lvac_cb, fmi_lvac)

        ctrl_id = wx.Window.NewControlId()
        fmi_lp = fm.Append(ctrl_id, "LogParser", "Start a new Log Parser")
        self.Bind(wx.EVT_MENU, self.start_lp_cb, fmi_lp)

        ctrl_id = wx.Window.NewControlId()
        fmi_exit = fm.Append(ctrl_id, "Exit", "Exit the application")
        self.Bind(wx.EVT_MENU, self.cb_exit_prog, fmi_exit)

        mb.Append(fm, "File")

        # '?' menu
        hm = wx.Menu()  # pylint: disable=invalid-name

        ctrl_id = wx.Window.NewControlId()
        show_info_item = hm.Append(ctrl_id, "Info", "Info")
        self.Bind(wx.EVT_MENU, cb_show_info, show_info_item)

        ctrl_id = wx.Window.NewControlId()
        open_documentation_item = hm.Append(ctrl_id, "Documentation", "Documentation")
        self.Bind(wx.EVT_MENU, cb_open_documentation, open_documentation_item)

        mb.Append(hm, "?")

        # menu bau fully constructed
        self.SetMenuBar(mb)

    def _add_status_bar(self):
        self.status_bar: wx.StatusBar = self.CreateStatusBar(2)
        self.status_bar.SetStatusText(f"Project root: {PROJECT_ROOT}", 0)
        adapter_name = None
        if self._can_process:
            adapter_name = self._can_process.init_adapter
        self.status_bar.SetStatusText(f"CAN adapter: {adapter_name}", 1)

    def cb_exit_prog(self, event):
        """Wrapper for menu exit."""
        self.Destroy()

    def Destroy(self):  # pylint: disable=invalid-name
        """Global exit function of this."""
        if self._lvac_frame and self._lvac_frame.is_running:
            wx.MessageBox(
                (
                    "Live View and Control is still running.\n"
                    "Close it before closing the main application."
                ),
                "Error",
                wx.OK | wx.ICON_ERROR,
            )
            return False

        if self._lp_frame and self._lp_frame.is_running:
            wx.MessageBox(
                (
                    "Log Parser is still running.\n"
                    "Close it before closing the main application."
                ),
                "Error",
                wx.OK | wx.ICON_ERROR,
            )
            return False

        if self._lvac_frame:
            self._lvac_frame.CloseFrameSafely(None)
        if self._lp_frame:
            self._lp_frame.Destroy()

        return super().Destroy()


def main():
    """Run the entry parser frame as own GUI"""
    app = wx.App()
    wx.Locale(wx.LANGUAGE_ENGLISH)
    EntryFrame()
    app.MainLoop()


if __name__ == "__main__":
    main()
