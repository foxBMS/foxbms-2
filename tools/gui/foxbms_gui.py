#!/usr/bin/env python
# -*- coding: utf-8 -*-

# @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der
#   angewandten Forschung e.V. All rights reserved.
#
# BSD 3-Clause License
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1.  Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
# 3.  Neither the name of the copyright holder nor the names of its
#     contributors may be used to endorse or promote products derived from this
#     software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# We kindly request you to use one or more of the following phrases to refer to
# foxBMS in your hardware, software, documentation or advertising materials:
#
# &Prime;This product uses parts of foxBMS&reg;&Prime;
#
# &Prime;This product includes parts of foxBMS&reg;&Prime;
#
# &Prime;This product is derived from foxBMS&reg;&Prime;

"""This is the foxBMS GUI
It has many good features and so on

https://foxbms.org
"""

import os
import webbrowser
import wx
import wx.adv

import fgui  # pylint: disable=unused-import

from log_parser import LogParserFrame

__author__ = "The foxBMS Team"
__version__ = "0.0.1"


class FoxBMSMainFrame(wx.Frame):  # pylint: disable=too-many-ancestors
    """Main frame to construct the foxBMS GUI frame"""

    foxbms_license_file = os.path.join(
        os.path.dirname(os.path.realpath(__file__)), "..", "..", "LICENSE"
    )
    with open(foxbms_license_file, "r", encoding="utf-8") as f:
        foxbms_license = f.read()

    foxbms_logo = os.path.join(
        os.path.dirname(os.path.realpath(__file__)),
        "..",
        "..",
        "docs",
        "_static",
        "foxbms250px.png",
    )

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

        # Add a
        _icon = wx.Icon()
        logo_img = wx.Image(self.foxbms_logo)
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
        doc = os.path.join(
            os.path.dirname(os.path.realpath(__file__)),
            "..",
            "..",
            "build",
            "docs",
            "index.html",
        )
        if not os.path.isfile(doc):
            doc = "https://foxbms.org"
        webbrowser.open(doc)

    def cb_show_info(self, event):
        """Shows the program information"""
        about_info = wx.adv.AboutDialogInfo()
        about_info.SetName("foxBMS")
        about_info.SetVersion(__version__)
        about_info.Copyright = "(C) 2010 - 2019 foxBMS"
        about_info.SetDescription(__doc__)
        about_info.SetWebSite = ("https://foxbms.org", "foxbms.org")
        about_info.License = self.foxbms_license
        _icon = wx.Icon()
        logo_img = wx.Image(self.foxbms_logo)
        logo_img_size = logo_img.GetSize()
        resized = logo_img_size / 3
        logo_img.Rescale(resized[0], resized[1])
        image = wx.Bitmap(logo_img)
        _icon.CopyFromBitmap(image)
        about_info.SetIcon(_icon)
        wx.adv.AboutBox(about_info)


def main():
    """Starts the application"""
    app = wx.App(False)
    FoxBMSMainFrame(None)
    app.MainLoop()


if __name__ == "__main__":
    main()
