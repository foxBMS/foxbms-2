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


"""Table to display the cell voltages"""


import can
import wx
import wx.grid as gridlib
from cantools.database.can import Database

from ..misc.gui_helpers import get_icon

FRAME_STYLE = (
    wx.DEFAULT_FRAME_STYLE & (~wx.CLOSE_BOX) & (~wx.MAXIMIZE_BOX) ^ wx.RESIZE_BORDER
)


class CellVoltageFrame(  # pylint: disable=too-many-ancestors,too-many-instance-attributes
    wx.Frame
):
    """
    Frame to display the cell voltages in a table.
    """

    # pylint: disable=too-many-arguments
    def __init__(
        self,
        title,
        parent=None,
        dbc: Database = None,
        strings=1,
        modules=8,
        cells_per_module=12,
        pos=None,
    ):
        wx.Frame.__init__(self, parent=parent, title=title, style=FRAME_STYLE)
        self.SetPosition(pos)
        self.SetIcon(get_icon())
        panel = wx.Panel(self)
        self.strings = strings
        self.modules = modules
        self.cells_per_module = cells_per_module
        self.grid_table = gridlib.Grid(panel)
        self.grid_table.CreateGrid(self.cells_per_module, self.modules)
        for i in range(0, self.modules):
            self.grid_table.SetColLabelValue(i, f"Module {i+1}")
        for i in range(0, self.cells_per_module):
            self.grid_table.SetRowLabelValue(i, f"Cell {i+1}")
        for row in range(self.cells_per_module):
            for col in range(self.modules):
                self.grid_table.SetCellValue(row, col, "None")
        self.grid_table.EnableEditing(False)
        self.grid_table.DisableDragRowSize()
        self.grid_table.DisableDragColSize()

        sizer = wx.BoxSizer()
        sizer.Add(self.grid_table, 1, wx.EXPAND | wx.ALL)
        panel.SetSizer(sizer)
        sizer.Fit(panel)
        width, height = self.grid_table.GetSize()
        self.SetSize(wx.Size(width + 28, height + 44))

        # dbc stuff
        self.msg_name = "foxBMS_CellVoltages"
        self.mux_name = "foxBMS_CellVoltages_Mux"
        self.dbc = dbc
        self.cell_voltages_msg = self.dbc.get_message_by_name(self.msg_name)
        self.cell_voltages_mux = self.cell_voltages_msg.get_signal_by_name(
            self.mux_name
        )

    def update_cell_voltages(self, value: can.message.Message):
        """Updates the table cell entries with the current voltage values"""
        msg_data = self.dbc.decode_message(value.arbitration_id, value.data)
        mux_id = self._get_mux_id(msg_data)
        for i in range(0, 4):
            cell_id = mux_id * 4 + i
            cell_voltage = msg_data[f"cellVoltage_{str(cell_id).zfill(3)}"]
            cell_voltage = str(cell_voltage)
            cell_position_in_module = cell_id % self.cells_per_module
            cell_position_in_string = int(cell_id // self.cells_per_module)
            cell_voltage_invalid = msg_data[
                f"cellVoltage_{str(cell_id).zfill(3)}_invalidFlag"
            ]
            if not cell_id < self.cells_per_module * self.modules:
                break
            self.grid_table.SetCellValue(
                cell_position_in_module, cell_position_in_string, cell_voltage
            )
            if not cell_voltage_invalid:
                color = wx.GREEN
            else:
                color = wx.RED
            self.grid_table.SetCellBackgroundColour(
                cell_position_in_module, cell_position_in_string, color
            )
        self.grid_table.ForceRefresh()

    def _get_mux_id(self, msg_data):
        mux_id_str: str = list(msg_data.values())[0]
        mux_id = [
            k for k, v in self.cell_voltages_mux.choices.items() if v == mux_id_str
        ][0]
        return mux_id

    def Destroy(self):  # pylint: disable=invalid-name,no-self-use
        """Unbind normal destroy"""
        return True

    def NewDestroy(self):  # pylint: disable=invalid-name
        """Destroys the frame"""
        return super().Destroy()
