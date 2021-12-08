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

"""helper for the info dialog"""

import os
import pathlib
import wx
import wx.adv
import wx.html
import wx.grid
import wx.lib.agw.hyperlink as hl
import markdown

import fgui

__version__ = fgui.__version__
__appname__ = fgui.__appname__
__author__ = fgui.__author__
__copyright__ = fgui.__copyright__
__author__ = fgui.__author__
__email__ = fgui.__email__

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
README_FILE = os.path.join(SCRIPT_DIR, "README.md")
LICENSE_FILE = os.path.join(SCRIPT_DIR, "..", "..", "LICENSE.md")
FOXBMS_LOGO = os.path.join(SCRIPT_DIR, "..", "..", "docs", "_static", "foxbms250px.png")
FOXBMS_URL = "https://foxbms.org"
BASE_URL = "https://iisb-foxbms.iisb.fraunhofer.de/"
LICENSE_FALLBACK_URL = f"{BASE_URL}foxbms/gen2/docs/html/latest/general/license.html"


class FoxbmsInfoDialog(wx.Dialog):
    """Dialog box for 'Info' message"""

    # pylint: disable=too-many-ancestors

    def __init__(self, *args, **kw):
        super().__init__(*args, **kw)
        self.SetSize((605, 1000))
        self.SetTitle("About foxBMS 2")
        self.license_file = self.get_file(LICENSE_FILE)
        self.readme_file = self.get_file(README_FILE)
        self.license_text = self.get_license_text()
        self.readme_text = self.get_readme_text()
        self.init_foxbms_dialog()
        self.SetPosition((0, 0))

    def init_foxbms_dialog(self):
        """Setup the text of the dialog box"""
        # Add icon
        _icon = wx.Icon()
        logo_img = wx.Image(FOXBMS_LOGO)
        logo_img_size = logo_img.GetSize()
        resized = logo_img_size / 5
        logo_img.Rescale(resized[0], resized[1])
        image = wx.Bitmap(logo_img)
        _icon.CopyFromBitmap(image)
        self.SetIcon(_icon)
        # fill panel
        panel = wx.Panel(self, -1)
        sizer = wx.GridBagSizer(4, 2)
        # logo and heading
        png = wx.Image(FOXBMS_LOGO, wx.BITMAP_TYPE_ANY).ConvertToBitmap()
        logo = wx.StaticBitmap(self, -1, png, (5, 5), (png.GetWidth(), png.GetHeight()))
        heading = wx.TextCtrl(
            self,
            -1,
            f"foxBMS 2 GUI - {fgui.__version__}",
            size=(275, 30),
            style=wx.TE_READONLY | wx.BORDER_NONE,
        )
        # pylint:disable=no-member
        font = wx.Font(20, wx.DECORATIVE, wx.NORMAL, wx.BOLD)
        heading.SetFont(font)
        heading.SetBackgroundColour(panel.GetBackgroundColour())
        # copyright hyperlink to foxbms.org
        foxbms_copyright = wx.TextCtrl(
            self,
            -1,
            __copyright__,
            (5, 5),
            size=(275, 30),
            style=wx.TE_READONLY | wx.BORDER_NONE,
        )
        foxbms_url = hl.HyperLinkCtrl(panel, -1, "foxbms.org", URL=FOXBMS_URL)
        # License text
        license_html = markdown.markdown(self.license_text, output_format="html5")
        license_ctrl = wx.html.HtmlWindow(
            self,
            -1,
            pos=(5, 5),
            size=(600, 550),
            style=wx.TE_READONLY | wx.TE_MULTILINE | wx.TE_NO_VSCROLL | wx.BORDER_NONE,
        )
        license_ctrl.SetPage(license_html)
        # GUI description
        readme_html = markdown.markdown(self.readme_text, output_format="html5")
        description_ctrl = wx.html.HtmlWindow(
            self,
            -1,
            pos=(5, 5),
            size=(600, 400),
            style=wx.TE_READONLY | wx.TE_MULTILINE | wx.BORDER_NONE,
        )
        description_ctrl.SetBackgroundColour(panel.GetBackgroundColour())
        description_ctrl.SetPage(readme_html)
        # add stuff to sizer
        sizer.Add(logo, pos=(0, 0), flag=wx.ALIGN_CENTER)
        sizer.Add(heading, pos=(0, 1), flag=wx.ALIGN_CENTER)
        sizer.Add(foxbms_copyright, pos=(1, 0), flag=wx.ALIGN_CENTER)
        sizer.Add(foxbms_url, pos=(1, 1), flag=wx.ALIGN_CENTER)
        sizer.Add(license_ctrl, pos=(2, 0), span=(1, 2), flag=wx.EXPAND)
        sizer.Add(description_ctrl, pos=(3, 0), span=(1, 2), flag=wx.EXPAND)
        panel.SetSizerAndFit(sizer)

    @staticmethod
    def get_file(_file):
        """Returns the file, if it exists."""
        _file = pathlib.Path(_file)
        if not _file.is_file():
            _file = None
        return _file

    def get_license_text(self):
        """Get the license text, if this is not possible link to the online
        documentation of the license"""

        if self.license_file:
            license_text = self.license_file.read_text(encoding="utf-8")
        else:
            license_text = (
                "Could not find foxBMS 2 license file.\n"
                f"Please check {LICENSE_FALLBACK_URL}."
            )
            self.license_file_missing_msg_box = wx.MessageBox(
                license_text, "License file missing", wx.OK | wx.ICON_WARNING
            )
            self.Bind(wx.EVT_BUTTON, self.license_file_missing_msg_box)
        return license_text

    def get_readme_text(self):
        """returns the text of the README or a default one."""
        if self.readme_file:
            readme_text = self.readme_file.read_text(encoding="utf-8")
        else:
            readme_text = "# foxBMS 2 GUI"
        return readme_text
