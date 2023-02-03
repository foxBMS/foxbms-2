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

"""A GUI to parse and visualize foxBMS generated logfiles."""

import os

import matplotlib.pyplot as plt
import pandas
import wx
from fgui import (
    DEFAULT_DBC_DIRECTORY_IF_IN_PROJECT_LAYOUT,
    DEFAULT_DBC_FILENAME_IF_IN_PROJECT_LAYOUT,
    __appname__,
    __version__,
)
from fgui.misc.can.can_helpers import dbc_file_to_database, get_dbc_dialog
from fgui.misc.gui_helpers import cb_open_documentation, cb_show_info, get_icon

TITLE = f"{__appname__} - Log Parser - {__version__}"
SIZE = wx.Size(1000, 1000)


class LogParserFrame(  # pylint: disable=too-many-ancestors,disable=too-many-instance-attributes
    wx.Frame
):
    """Main frame to construct the foxBMS Log Parser frame"""

    def __init__(self, parent=None):
        """Initializes the Log Parser GUI"""
        super().__init__(parent, title=TITLE, size=SIZE)
        self.local = wx.Locale(wx.LANGUAGE_ENGLISH)
        self.log_files = []
        self.Centre()
        self.basic_gui()
        self.all_checked_sig = []
        self.plot_dfs = []
        self.all_sig = []
        self.is_running = True
        self.dbc_file = str(
            DEFAULT_DBC_DIRECTORY_IF_IN_PROJECT_LAYOUT
            / DEFAULT_DBC_FILENAME_IF_IN_PROJECT_LAYOUT
        )
        self.dbc = dbc_file_to_database(self.dbc_file)
        self._add_signals()

    def basic_gui(self):
        """Creates the layout of the GUI"""
        # Set up the menu
        self.SetIcon(get_icon())
        self._add_menu_bar()
        self._add_status_bar()
        self.panel = wx.Panel(self)

        # Set up trc file selector box
        group_box_trc = wx.StaticBox(  # pylint: disable=unused-variable
            self.panel,
            wx.ID_ANY,
            "Select a trace-file",
            size=(485, 130),
            pos=(495, 120),
        )
        self.logfield = wx.TextCtrl(self.panel, pos=(570, 160), size=(350, 20))
        wx.StaticText(self.panel, -1, "Path: ", pos=(510, 165))

        btn_search_trc = wx.Button(
            self.panel, -1, label="Search file", pos=(570, 200), size=(120, 25)
        )
        btn_search_trc.SetBackgroundColour("WHITE")
        btn_search_trc.Bind(wx.EVT_BUTTON, self.cb_on_clicked_btn_trc_src)

        # set up search in signal box
        search_field = wx.SearchCtrl(
            self.panel, -1, size=(400, 20), pos=(5, 267), style=0
        )
        search_field.Bind(wx.EVT_TEXT, self.cb_on_search)

        # set up clear selected signals button
        btn_clr_sel_sig = wx.Button(
            self.panel, -1, label="Clear selection", pos=(410, 267), size=(120, 20)
        )
        btn_clr_sel_sig.SetBackgroundColour("WHITE")
        btn_clr_sel_sig.Bind(wx.EVT_BUTTON, self.cb_on_click_sig_clear)

        # Set up generate
        btn_generate_trc = wx.Button(
            self.panel, -1, label="Generate", pos=(700, 257), size=(150, 40)
        )
        btn_generate_trc.SetBackgroundColour("WHITE")
        btn_generate_trc.Bind(wx.EVT_BUTTON, self.cb_on_clicked_btn_gen_trc)

        # Set up signals to be plotted
        wx.StaticBox(  # pylint: disable=unused-variable
            self.panel,
            wx.ID_ANY,
            "Select signals to plot",
            size=(975, 640),
            pos=(5, 300),
        )
        self.clb_select_sig = wx.CheckListBox(
            self.panel,
            -1,
            size=(965, 610),
            pos=(10, 320),
            choices=[],
            style=wx.BORDER_NONE | wx.SYS_COLOUR_HIGHLIGHT,
        )
        self.clb_select_sig.Bind(wx.EVT_CHECKLISTBOX, self.cb_on_click_checked_box)

        self.Show(True)

    def _add_menu_bar(self):
        menu_bar = wx.MenuBar()
        file_menu = wx.Menu()
        help_menu = wx.Menu()

        ctrl_id = wx.Window.NewControlId()
        fmi_of = file_menu.Append(ctrl_id, "Open dbc file", "Open dbc file")
        self.Bind(wx.EVT_MENU, self._get_dbc_cb, fmi_of)
        fmi_exit = file_menu.Append(wx.ID_EXIT, "Exit", "Exit the application")
        self.Bind(wx.EVT_MENU, self.quit_cb, fmi_exit)
        menu_bar.Append(file_menu, "File")

        help_menu = wx.Menu()
        ctrl_id = wx.Window.NewControlId()
        show_info_item = help_menu.Append(ctrl_id, "Info", "Info")
        ctrl_id = wx.Window.NewControlId()
        open_documentation_item = help_menu.Append(
            ctrl_id, "Documentation", "Documentation"
        )
        menu_bar.Append(help_menu, "?")
        self.Bind(wx.EVT_MENU, cb_show_info, show_info_item)
        self.Bind(wx.EVT_MENU, cb_open_documentation, open_documentation_item)

        self.SetMenuBar(menu_bar)

    def _add_status_bar(self):
        """status bar to display the currently loaded dbc file"""
        # pylint: disable=attribute-defined-outside-init
        self.statusbar: wx.StatusBar = self.CreateStatusBar(1)
        self.statusbar.SetStatusText("No dbc file loaded")

    def open_dialog(self, window_name, only_files):
        """Wrapper for the FileDialog class"""
        open_file_dialog = wx.FileDialog(
            self,
            window_name,
            os.path.dirname(os.path.realpath(__file__)),
            "",
            only_files,
            wx.FD_OPEN | wx.FD_FILE_MUST_EXIST,
        )

        open_file_dialog.ShowModal()
        path = open_file_dialog.GetPath()
        open_file_dialog.Destroy()
        return path

    def get_log_type(self):
        """check the file type and the header to find get the log type"""
        with open(self.logfield.GetValue(), "r", encoding="utf-8") as trace_template:
            file_type = os.path.splitext(self.logfield.GetValue())[1]
            header = []
            for _ in range(16):
                header.append(trace_template.readline())
            if file_type == ".trc":
                if (
                    header[0] == ";$FILEVERSION=1.1\n"
                    and header[6].find(";   Generated by PCAN-View") != -1
                ):
                    self.read_pcan_log_v1()
                elif (
                    header[0] == ";$FILEVERSION=2.0\n"
                    and header[6].find(";   Generated by PCAN-View") != -1
                ):
                    self.read_pcan_log_v2()
                else:
                    msg = "ERROR, trc file not supported"
                    wx.MessageBox(msg, "Error", wx.OK | wx.ICON_ERROR)
            elif file_type == ".txt":
                self.read_can_log()
            else:
                msg = "ERROR, trc file not supported"
                wx.MessageBox(msg, "Error", wx.OK | wx.ICON_ERROR)

    def get_id_name(self, checked_signal):  # pylint: disable=no-self-use
        """Get the signal id and the signal name"""
        signal_name = checked_signal[0 : checked_signal.find(" (0x")]
        id_signal = checked_signal[checked_signal.find("0x") : checked_signal.find(")")]

        id_signal = id_signal.replace("x", "")
        id_signal = id_signal.upper()
        return id_signal, signal_name

    def get_mux_id(self, id_signal, signal_name, mdb):  # pylint: disable=no-self-use
        """Get multiplexer ID of checked signal"""
        msg = mdb.get_message_by_frame_id(int(id_signal, 16))
        unit = ""
        mux_id = ""
        for i, sig in enumerate(msg.signals):  # pylint: disable=unused-variable
            if msg.signals[i].name == signal_name:
                unit = str(msg.signals[i].unit)
                if unit == "None":
                    unit = ""
                if msg.is_multiplexed():
                    if msg.signals[i].multiplexer_ids:
                        mux_id = msg.signals[i].multiplexer_ids[0]
                break
        return unit, mux_id, msg

    def append_plot_df(self, signal_timestamp, signal_val, signal_name, unit):
        """Append the signal values to the plot data frame"""
        if len(signal_timestamp) > 0 and len(signal_val) > 0:
            plot_df = pandas.DataFrame()
            plot_df.insert(0, signal_name + "_time(ms)", signal_timestamp)
            plot_df.insert(
                1, signal_name + " in " + unit.replace("Â°C", "°C"), signal_val
            )
            self.plot_dfs.append(plot_df)

    def read_pcan_log_v1(self):  # pylint: disable=too-many-locals
        """Read data to selected signals from PCAN log v1 file"""
        units = []
        # Get the data of all selected signals
        with open(self.logfield.GetValue(), "r", encoding="utf-8") as trace_template:
            for checked_signal in self.clb_select_sig.GetCheckedStrings():

                # get signal and signal id
                id_signal, signal_name = self.get_id_name(checked_signal)

                # get mux id and unit
                unit, mux_id, msg = self.get_mux_id(id_signal, signal_name, self.dbc)
                units.append(unit)

                # get lines with right signal
                cor_signal_lines = []
                for i, line_ck in enumerate(trace_template):
                    if i < 16:
                        continue
                    if id_signal in line_ck[32:36]:
                        cor_signal_lines.append(line_ck)
                if not cor_signal_lines:
                    continue

                # read lines with dest signal and decode data
                signal_timestamp = []
                signal_val = []
                for line in cor_signal_lines:
                    mux_now = int(bin(int(line[41:43], 16)), 2) & 0b1111
                    if mux_now == mux_id or not msg.is_multiplexed():
                        timestamp = line[7:19]
                        timestamp = float(timestamp)
                        byte_list = []
                        byte_list.append(int(line[41:43], 16))  # Byte 0
                        byte_list.append(int(line[44:46], 16))  # Byte 1
                        byte_list.append(int(line[47:49], 16))  # Byte 2
                        byte_list.append(int(line[50:52], 16))  # Byte 3
                        byte_list.append(int(line[53:55], 16))  # Byte 4
                        byte_list.append(int(line[56:58], 16))  # Byte 5
                        byte_list.append(int(line[59:61], 16))  # Byte 6
                        byte_list.append(int(line[62:64], 16))  # Byte 7
                        decoded_sig = msg.decode(bytes(byte_list))

                        try:
                            decoded_sig_val = decoded_sig[signal_name]
                        except KeyError:
                            msg = "Signal" + str(signal_name) + " is not in dictionary."
                            wx.MessageBox(msg, "Error", wx.OK | wx.ICON_ERROR)
                            self.plot_dfs.clear()
                            return
                        signal_timestamp.append(timestamp)
                        signal_val.append(decoded_sig_val)

                # append plot_df with new plot data
                self.append_plot_df(signal_timestamp, signal_val, signal_name, unit)

                trace_template.seek(0)
        self.plot_selected_signals(units)

    def read_pcan_log_v2(self):  # pylint: disable=too-many-locals
        """Read data to selected signals from PCAN log v2 file"""
        units = []
        # Get the data of all selected signals
        with open(self.logfield.GetValue(), "r", encoding="utf-8") as trace_template:
            for checked_signal in self.clb_select_sig.GetCheckedStrings():

                # get signal and signal id
                id_signal, signal_name = self.get_id_name(checked_signal)

                # get mux id and unit
                unit, mux_id, msg = self.get_mux_id(id_signal, signal_name, self.dbc)
                units.append(unit)

                cor_signal_lines = []
                for i, line_ck in enumerate(trace_template):
                    if i < 16:
                        continue
                    if id_signal in line_ck[29:33]:
                        cor_signal_lines.append(line_ck)
                if not cor_signal_lines:
                    continue

                # read lines with dest signal and decode data
                signal_timestamp = []
                signal_val = []
                for line in cor_signal_lines:
                    mux_now = int(bin(int(line[40:42], 16)), 2) & 0b1111
                    if mux_now == mux_id or not msg.is_multiplexed():
                        timestamp = line[7:19]
                        timestamp = float(timestamp)
                        byte_list = []
                        byte_list.append(int(line[40:42], 16))  # Byte 0
                        byte_list.append(int(line[43:45], 16))  # Byte 1
                        byte_list.append(int(line[46:48], 16))  # Byte 2
                        byte_list.append(int(line[49:51], 16))  # Byte 3
                        byte_list.append(int(line[52:54], 16))  # Byte 4
                        byte_list.append(int(line[55:57], 16))  # Byte 5
                        byte_list.append(int(line[58:60], 16))  # Byte 6
                        byte_list.append(int(line[61:63], 16))  # Byte 7
                        decoded_sig = msg.decode(bytes(byte_list))

                        try:
                            decoded_sig_val = decoded_sig[signal_name]
                        except KeyError:
                            msg = "Signal" + str(signal_name) + " is not in dictionary."
                            wx.MessageBox(msg, "Error", wx.OK | wx.ICON_ERROR)
                            self.plot_dfs.clear()
                            return
                        signal_timestamp.append(timestamp)
                        signal_val.append(decoded_sig_val)

                # append plot_df with new plot data
                self.append_plot_df(signal_timestamp, signal_val, signal_name, unit)

                trace_template.seek(0)
        self.plot_selected_signals(units)

    def read_can_log(self):  # pylint: disable=too-many-locals
        """Read data to selected signals from can file"""
        units = []
        # Get the data of all selected signals
        with open(self.logfield.GetValue(), "r", encoding="utf-8") as trace_template:
            for checked_signal in self.clb_select_sig.GetCheckedStrings():

                # get signal and signal id
                id_signal, signal_name = self.get_id_name(checked_signal)

                # get mux id and unit
                unit, mux_id, msg = self.get_mux_id(id_signal, signal_name, self.dbc)
                units.append(unit)

                cor_signal_lines = []
                for i, line_ck in enumerate(trace_template):
                    if i < 13:
                        continue
                    line_information = line_ck.split(" ", 2)
                    id_signal_dez = int(id_signal, 16)
                    line_ck_id_dez = int(line_information[1])
                    if id_signal_dez == line_ck_id_dez:
                        cor_signal_lines.append(line_ck)
                if not cor_signal_lines:
                    continue

                # read lines with dest signal and decode data
                signal_timestamp = []
                signal_val = []

                for line in cor_signal_lines:
                    line_information = line.split(" ", 3)
                    byte_list_str = line_information[3].split(" ", 7)
                    byte_list = list(map(int, byte_list_str))
                    mux_now = int(bin(int(byte_list[0])), 2) & 0b1111
                    if mux_now == mux_id or not msg.is_multiplexed():
                        line_information = line.split(" ", 3)

                        timestamp = float(line_information[0])

                        decoded_sig = msg.decode(bytes(byte_list))

                        try:
                            decoded_sig_val = decoded_sig[signal_name]
                        except KeyError:

                            msg = "Signal" + str(signal_name) + " is not in dictionary."
                            wx.MessageBox(msg, "Error", wx.OK | wx.ICON_ERROR)
                            self.plot_dfs.clear()
                            return
                        signal_timestamp.append(timestamp)
                        signal_val.append(decoded_sig_val)

                # append plot_df with new plot data
                self.append_plot_df(signal_timestamp, signal_val, signal_name, unit)

                trace_template.seek(0)
        self.plot_selected_signals(units)

    def plot_selected_signals(self, units):
        """Plot the data of the selected signals from the read log file"""
        if len(self.plot_dfs) < 1:
            msg = "No signals chosen or signals not in Log files."
            wx.MessageBox(msg, "Error", wx.OK | wx.ICON_ERROR)
            self.plot_dfs.clear()
            return

        # set figure size depending on subplot count
        subplot_count = len(set(units))
        x_size = min(subplot_count * 6, 15)
        fig = plt.figure(figsize=(x_size, 8))

        # manage subplots with units and axes to put all graphs with same unit
        # in one subplot
        axes_units = [[], []]
        first_ax = fig.add_subplot(1, subplot_count, 1)
        first_ax.set_ylabel(units[0].replace("Â°C", "°C"), rotation=0)
        plt.xlabel("Time(ms)")
        first_ax.legend(bbox_to_anchor=(0, 1.02, 1, 0.2), loc="lower left")
        axes_units[0].append(first_ax)
        axes_units[1].append(units[0])

        # check all dataframes if a new plot is needed or a suiting subplot exists
        for i_df, df in enumerate(self.plot_dfs):  # pylint: disable=invalid-name
            new_ax = True
            for i_unit, unit in enumerate(axes_units[1]):
                if unit == units[i_df]:
                    new_ax = False
                    plt_ax = axes_units[0][i_unit]
                    break
            if new_ax:
                plt_ax = fig.add_subplot(1, subplot_count, len(axes_units[0]) + 1)
                plt_ax.set_ylabel(units[i_df].replace("Â°C", "°C"), rotation=0)
                axes_units[0].append(plt_ax)
                axes_units[1].append(units[i_df])

            df.plot(x=df.columns[0], y=df.columns[1], kind="line", ax=plt_ax)
            plt.xlabel("Time(ms)")
            plt_ax.legend(bbox_to_anchor=(0, 1.02, 1, 0.2), loc="lower left")
        self.plot_dfs.clear()

        plt.show()

    # Event Handling
    def quit_cb(self, event):
        """Closes the application"""
        self.Close()

    def cb_on_clicked_btn_trc_src(self, event):
        """sets the logfile to be parsed."""
        if not self.dbc:
            style = wx.OK | wx.ICON_ERROR
            wx.MessageBox(
                ('Load a dbc file first ("File -> Open dbc File")'), "Error", style
            )
            return
        # Sets the value from the chosen trace file
        log_file_path = self.open_dialog(
            "Search for log file", "Trace or Text files (*.trc;*.txt)|*.trc;*.txt"
        )
        self.logfield.SetValue(log_file_path)

    def cb_on_click_checked_box(self, event):
        """Sets the checked signals"""
        is_checked = self.clb_select_sig.IsChecked(event.GetInt())
        if is_checked:
            self.all_checked_sig.append(event.GetString())
        else:
            self.all_checked_sig.remove(event.GetString())

    def _get_dbc_cb(self, event):
        wx.CallAfter(self._get_dbc)

    def _get_dbc(self):
        """Reads the signals from the dbc file"""
        (self.dbc, self.dbc_file) = get_dbc_dialog()
        self.statusbar.SetStatusText(str(self.dbc_file))
        self._add_signals()

    def _add_signals(self):
        self.all_sig = []
        if self.dbc:
            for msg in self.dbc.messages:
                for sig in msg.signals:
                    self.all_sig.append(f"{sig.name} ({hex(msg.frame_id)})")
            self.clb_select_sig.AppendItems(self.all_sig)
        else:
            self.clb_select_sig.Clear()

    def cb_on_clicked_btn_gen_trc(self, event):
        """tries to print the trace information (only if a valid
        file path is chosen)"""
        # check if the provided file path is valid
        if not os.path.isfile(self.logfield.GetValue()):
            msg = f"{self.logfield.GetValue()} is not a valid file."
            wx.MessageBox(msg, "Error", wx.OK | wx.ICON_ERROR)
            return
        # we are sure, we have an existing file, so we can go on
        if len(self.clb_select_sig.GetCheckedItems()) > 0:
            self.get_log_type()
        else:
            msg = "Select signals to plot."
            wx.MessageBox(msg, "Error", wx.OK | wx.ICON_ERROR)

    def cb_on_search(self, event):
        """filter signals based on typed string in search box"""
        items_to_check = []
        if event.GetString() == "":
            self.clb_select_sig.Clear()
            self.clb_select_sig.AppendItems(self.all_sig)
            self.clb_select_sig.SetCheckedStrings(self.all_checked_sig)
        else:
            searched_string = event.GetString()
            self.clb_select_sig.Clear()
            for sig in self.all_sig:
                if sig.find(searched_string) != -1:
                    self.clb_select_sig.AppendItems([sig])
                    if sig in self.all_checked_sig:
                        items_to_check.append(sig)
            self.clb_select_sig.SetCheckedStrings(items_to_check)

    def cb_on_click_sig_clear(self, event):
        """Clear signals"""
        for i in range(len(self.clb_select_sig.GetItems())):
            self.clb_select_sig.Check(i, check=False)
        self.all_checked_sig.clear()

    def Destroy(self):  # pylint: disable=invalid-name
        """Close the Frame"""
        self.is_running = False
        return super().Destroy()

    def _get_log_files_cb(self, event):
        with wx.FileDialog(
            self,
            "Open ASC file",
            wildcard="CAN log files (*.asc)|*.asc",
            style=wx.FD_OPEN | wx.FD_FILE_MUST_EXIST | wx.FD_MULTIPLE,
        ) as file_dialog:

            if file_dialog.ShowModal() == wx.ID_CANCEL:
                paths = []
            else:
                paths = file_dialog.GetPaths()
        for index, path in enumerate(paths):
            self.list_ctrl.InsertItem(index, path)
        self.log_files = paths


def main():
    """Run the log parser frame as own GUI"""
    app = wx.App()
    wx.Locale(wx.LANGUAGE_ENGLISH)
    LogParserFrame()
    app.MainLoop()


if __name__ == "__main__":
    main()
