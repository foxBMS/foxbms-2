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


"""Display the BMS state"""

import re

import can
import wx
import wx.grid as gridlib
from cantools.database.can import Database

from ..misc.gui_helpers import get_icon

FRAME_STYLE = (
    wx.DEFAULT_FRAME_STYLE & (~wx.CLOSE_BOX) & (~wx.MAXIMIZE_BOX) ^ wx.RESIZE_BORDER
)


class BmsStateFrame(  # pylint: disable=too-many-ancestors,too-many-instance-attributes
    wx.Frame
):
    """
    Frame to display the cell voltages in a table.
    """

    # pylint: disable=too-many-arguments
    def __init__(self, title, parent=None, dbc: Database = None, pos=None):
        wx.Frame.__init__(self, parent=parent, title=title, style=FRAME_STYLE)
        self.SetPosition(pos)
        self.SetIcon(get_icon())

        # dbc stuff
        self.bms_state_msg_name = "foxBMS_BmsState"
        self.dbc = dbc
        self.bms_state_msg = self.dbc.get_message_by_name(self.bms_state_msg_name)
        panel = wx.Panel(self)
        self.grid_table = gridlib.Grid(panel)
        self.grid_table.CreateGrid(len(self.bms_state_msg.signals), 1)
        self.grid_table.SetRowLabelSize(200)
        self.grid_table.SetColSize(0, 100)
        self.msg_to_idx = {}
        for i, val in enumerate(self.bms_state_msg.signals):
            self.msg_to_idx[val.name] = i
            label_name = val.name.replace("foxBMS_", "")
            label_name = label_name.replace("_", " ")
            label_name = " ".join(re.sub(r"([A-Z])", r" \1", label_name).split())
            self.grid_table.SetRowLabelValue(i, label_name)
        self.grid_table.SetColLabelValue(0, "Status")
        self.grid_table.SetRowLabelAlignment(wx.ALIGN_LEFT, wx.ALIGN_CENTRE)
        self.grid_table.EnableEditing(False)
        self.grid_table.DisableDragRowSize()
        self.grid_table.DisableDragColSize()

        sizer = wx.BoxSizer()
        sizer.Add(self.grid_table, 1, wx.EXPAND | wx.ALL)
        panel.SetSizer(sizer)
        sizer.Fit(panel)
        width, height = self.grid_table.GetSize()
        self.SetSize(wx.Size(width + 28, height + 44))

    def update_bms_state_info(self, value: can.message.Message):
        """Updates the table cell entries with the current voltage values"""
        msg_data: dict = self.dbc.decode_message(value.arbitration_id, value.data)
        for i in msg_data.items():
            self.grid_table.SetCellValue(self.msg_to_idx[i[0]], 0, str(i[1]))

    def Destroy(self):  # pylint: disable=invalid-name,no-self-use
        """Unbind normal destroy"""
        return True

    def NewDestroy(self):  # pylint: disable=invalid-name
        """Destroys the frame"""
        return super().Destroy()
