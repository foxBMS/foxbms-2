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

"""Implements an app to view the CAN messages that come from the BMS and
send CAN messages to the BMS to control the BMS behavior."""

import logging
import time
from pathlib import Path
from typing import Union

import can
import click
import wx
from cantools.database.can import Message
from wx.lib.masked import NumCtrl

from .. import PROJECT_ROOT, __appname__, __version__
from ..misc.can.can_helpers import get_dbc_dialog, try_to_select_can_adapter
from ..misc.gui_helpers import get_icon
from ..misc.misc import LOG_LEVELS, BatterySystemConfiguration
from ..misc.program_arguments import (
    C_SETUP_BAUD_RATE,
    C_SETUP_CAN_ADAPTERS,
    C_SETUP_LOGGING,
    C_SETUP_VERBOSITY,
    chose_baud_rate,
    chose_can_adapter,
    chose_logging_dir,
    validate_argument_combination,
)
from ..workers.can_node_worker import CanAdapterProcess
from ..workers.gui_sync_worker import SyncThread
from ..workers.send_worker import PeriodicSendThread
from .cell_temperatures import CellTemperatureFrame
from .cell_voltages import CellVoltageFrame
from .default_messages import (
    _get_balancing_threshold_limits,
    _get_balancing_threshold_signal,
    _get_bms_state_request_message,
    _get_cell_temperatures_msg_id,
    _get_cell_voltages_msg_id,
    _get_debug_response_msg_id,
    _get_message_type,
    _get_state_request_signal_mode,
    _set_bms_state_request_message,
)
from .debug_response import decode_debug_response
from .msgs.msg_debug import DebugMessage, DebugMessageState

TITLE = f"{__appname__} - Live View and Control - {__version__}"
SIZE = wx.Size(500, 800)

BS_CONFIG_FILE = (
    PROJECT_ROOT / "src" / "app" / "application" / "config" / "battery_system_cfg.h"
)


class LiveViewAndControlMainFrame(  # pylint:disable=too-many-ancestors,too-many-instance-attributes
    wx.Frame
):
    """Control frame of LVAC"""

    def __init__(self, parent=None, can_process: Union[None, CanAdapterProcess] = None):
        wx.Frame.__init__(self, parent, -1, title=TITLE, size=SIZE)
        self.SetPosition((10, 10))
        self.SetIcon(get_icon())
        self.Bind(wx.EVT_CLOSE, self.CloseFrameSafely)
        self.Bind(wx.EVT_SET_FOCUS, self.onLvacFocus)

        # for the parent to know, whether we need to close the frame
        self.is_running = True

        # setup the shared CAN process
        self.can_process = can_process
        # lock so that no one else can use it! --> use real in future!
        # this is currently only some whack gui introduced lock, not a real one
        # self.can_process.locked = True
        self.can_process.g_idle.clear()  # tell the CAN process that there will be work
        self.can_process.app_paused.set()
        self.can_process.app_canceled.clear()

        self.sync_worker = SyncThread(
            self.worker_process_function, self.can_process.receive_queue
        )
        self.send_worker = PeriodicSendThread(
            self.set_send_data, self.can_process.send_queue
        )
        self.panel = wx.Panel(self)
        self.vbox_main = wx.BoxSizer(wx.VERTICAL)
        self.panel.SetSizer(self.vbox_main)
        self.vbox_main.AddStretchSpacer()
        self.controls: list[wx.Button] = []

        self.btn_start_action = False
        self._add_start_btn()
        self._add_bms_state_request_btn()

        self.debug_message_state = DebugMessageState()
        self._add_set_rtc_time_btn()
        self._add_get_rtc_time_btn()
        self._add_fram_init_btn()
        self._add_get_software_version_btn()

        self.bms_state_request_last_entry = 0
        self.start_sending_requests = False
        self.dbc, _ = get_dbc_dialog()
        if not self.dbc:
            self.CloseFrameSafely(None)
            return
        bs_config = BatterySystemConfiguration(BS_CONFIG_FILE)
        self.f_cell_voltages = CellVoltageFrame(
            title="Cell Voltages",
            parent=self.GetParent(),
            dbc=self.dbc,
            pos=(520, 10),
            strings=bs_config.bs_nr_of_strings,
            modules=bs_config.bs_nr_of_modules_per_string,
            cells_per_module=bs_config.bs_nr_of_cell_blocks_per_module,
        )
        self.f_cell_voltages.Show()

        self.f_cell_temperatures = CellTemperatureFrame(
            title="Cell Temperatures",
            parent=self.GetParent(),
            dbc=self.dbc,
            pos=(520, self.f_cell_voltages.GetSize()[1] + 30),
            strings=bs_config.bs_nr_of_strings,
            modules=bs_config.bs_nr_of_modules_per_string,
            temps_per_module=bs_config.bs_nr_of_temp_sensors_per_module,
        )
        self.f_cell_temperatures.Show()

        # DBC/CAN stuff
        self.cell_voltages_msg_id = _get_cell_voltages_msg_id(self.dbc)
        self.cell_temperatures_msg_id = _get_cell_temperatures_msg_id(self.dbc)
        # construct BMS State Request Message
        self.state_request_msg: Message = _get_bms_state_request_message(self.dbc)
        self.state_request_signal_mode = _get_state_request_signal_mode(
            self.state_request_msg
        )
        # set Debug message
        self.debug_msg = DebugMessage(self.dbc)

        self.debug_response_msg_id = _get_debug_response_msg_id(self.dbc)

        # Balancing
        self.balancing_threshold_signal = _get_balancing_threshold_signal(
            self.state_request_msg
        )
        self.balancing_threshold_limits = _get_balancing_threshold_limits(
            self.balancing_threshold_signal
        )
        self._add_balancing_ckb()
        self._add_mode_request_rbtn()
        self._add_balancing_nctrl()
        self.activate_balancing = False
        self.balancing_threshold = 0
        self.mode_request = 0
        # on start up all controls shall not indicate that they are usable - as
        # they are not (the start button must be pressed before!)
        self.disable_controls()
        self.Show()

    def _add_start_btn(self) -> None:
        """Adds a global start/stop button for the app"""
        # this button shall work at every time - do not add it to the controls
        # attribute!
        self.btn_start = wx.Button(self.panel, label="Start")
        self.vbox_main.Add(self.btn_start, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 5)
        self.btn_start.Bind(wx.EVT_BUTTON, self.thread_wrapper_cb)

    def _add_bms_state_request_btn(self) -> None:
        """Add a button to start/stop sending requests to the BMS."""
        self.bms_state_request_btn = wx.Button(
            self.panel, label="Start BMS State Request"
        )
        self.vbox_main.Add(
            self.bms_state_request_btn, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 5
        )
        self.bms_state_request_btn.Disable()
        self.bms_state_request_btn.Bind(wx.EVT_BUTTON, self._set_bms_state_request_cb)
        self.controls.append(self.bms_state_request_btn)

    def _add_set_rtc_time_btn(self) -> None:
        """Add a button to set the RTC on the BMS."""
        self.bms_set_rtc_time_btn = wx.Button(self.panel, label="Set RTC Time")
        self.vbox_main.Add(
            self.bms_set_rtc_time_btn, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 5
        )
        self.bms_set_rtc_time_btn.Disable()
        self.bms_set_rtc_time_btn.Bind(wx.EVT_BUTTON, self._set_rtc_time_cb)
        self.controls.append(self.bms_set_rtc_time_btn)

    def _add_get_rtc_time_btn(self) -> None:
        """Add a button to set the RTC on the BMS."""
        self.bms_get_rtc_time_btn = wx.Button(self.panel, label="Get RTC Time")
        self.vbox_main.Add(
            self.bms_get_rtc_time_btn, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 5
        )
        self.bms_get_rtc_time_btn.Disable()
        self.bms_get_rtc_time_btn.Bind(wx.EVT_BUTTON, self._get_rtc_time_cb)
        self.controls.append(self.bms_get_rtc_time_btn)

    def _add_fram_init_btn(self) -> None:
        """Add a button to send a message to initialize the FRAM."""
        self.bms_initialize_fram_btn = wx.Button(self.panel, label="Initialize FRAM")
        self.vbox_main.Add(
            self.bms_initialize_fram_btn, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 5
        )
        self.bms_initialize_fram_btn.Disable()
        self.bms_initialize_fram_btn.Bind(wx.EVT_BUTTON, self._get_initialize_fram_cb)
        self.controls.append(self.bms_initialize_fram_btn)

    def _add_get_software_version_btn(self) -> None:
        """Add to send the CAN message for retrieving the BMS software version"""
        self.bms_get_software_version_btn = wx.Button(
            self.panel, label="Get software version"
        )
        self.vbox_main.Add(
            self.bms_get_software_version_btn, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 5
        )
        self.bms_get_software_version_btn.Disable()
        self.bms_get_software_version_btn.Bind(
            wx.EVT_BUTTON, self._get_software_version_cb
        )
        self.controls.append(self.bms_get_software_version_btn)

    def _add_balancing_ckb(self) -> None:
        """Add a checkbox to enable/disable balancing"""
        self.balancing_ckb = wx.CheckBox(self.panel, label="Balancing")
        self.vbox_main.Add(
            self.balancing_ckb, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 5
        )
        self.balancing_ckb.SetValue(False)
        self.balancing_ckb.Bind(wx.EVT_CHECKBOX, self._balancing_cb)
        self.controls.append(self.balancing_ckb)

    def _add_balancing_nctrl(self):
        """Add a GUI element that only accepts numbers"""
        self.flt1 = NumCtrl(self.panel, style=wx.TE_PROCESS_ENTER)
        self.flt1.SetValue(0)
        self.flt1.SetMin(self.balancing_threshold_limits[0])
        self.flt1.SetMax(self.balancing_threshold_limits[1])
        self.flt1.Bind(wx.EVT_KEY_DOWN, self._get_balancing_threshold_key)
        self.controls.append(self.flt1)

    def _get_balancing_threshold_key(self, event: wx.Event):
        """Gets the balancing threshold value from the control element."""
        keycode = event.GetKeyCode()
        if keycode in (wx.WXK_RETURN, wx.WXK_NUMPAD_ENTER, wx.WXK_TAB):
            self.balancing_threshold = self.flt1.GetValue()
            event.EventObject.Navigate()
        event.Skip()

    def _balancing_cb(self, event: wx.Event):
        """Activate balancing"""
        self.activate_balancing = event.GetEventObject().GetValue()

    def _add_mode_request_rbtn(self):
        mode_requests = []
        for _, value in self.state_request_signal_mode.choices.items():
            mode_requests.append(value.name)

        self.rb_request_choices = wx.RadioBox(
            self.panel, label="State Request", choices=mode_requests, majorDimension=1
        )
        self.vbox_main.Add(
            self.rb_request_choices, 0, wx.ALIGN_CENTER_HORIZONTAL | wx.ALL, 7
        )
        self.rb_request_choices.Bind(wx.EVT_RADIOBOX, self.select_mode_cb)
        self.controls.append(self.rb_request_choices)

    def select_mode_cb(self, event=None):
        """wrapper to retrieve the requested BMS mode bay an *CallAfter* function."""
        wx.CallAfter(self.select_mode_ca, event)

    def select_mode_ca(self, event=None):
        """Sets the requested BMS mode"""
        event_object = event.GetEventObject()
        requested_mode_as_str = event_object.GetString(event_object.GetSelection())
        for key, value in self.state_request_signal_mode.choices.items():
            if value == requested_mode_as_str:
                self.mode_request = key
                break

    def _set_bms_state_request_cb(self, event=None):
        wx.CallAfter(self._set_bms_state_request_ca, event)

    def _set_bms_state_request_ca(self, event=None):  # pylint: disable=unused-argument
        """Start/stop sending BMS state request messages"""
        if not self.start_sending_requests:
            self.bms_state_request_btn.SetLabel("Stop BMS State Request")
            self.start_sending_requests = True
        else:
            self.bms_state_request_btn.SetLabel("Start BMS State Request")
            self.start_sending_requests = False

    def _set_rtc_time_cb(self, event=None):
        wx.CallAfter(self._set_rtc_time_ca, event)

    def _get_rtc_time_cb(self, event=None):
        wx.CallAfter(self._get_rtc_time_ca, event)

    def _get_initialize_fram_cb(self, event=None):
        wx.CallAfter(self._get_initialize_fram_ca, event)

    def _get_software_version_cb(self, event=None):
        wx.CallAfter(self._get_software_version_ca, event)

    def _set_rtc_time_ca(self, event=None):  # pylint: disable=unused-argument
        """Set RTC time"""
        self.debug_message_state.set_rtc_time += 1
        self.debug_message_state.number_of_requests += 1

    def _get_rtc_time_ca(self, event=None):  # pylint: disable=unused-argument
        """Request to get RTC time"""
        self.debug_message_state.get_rtc_time += 1
        self.debug_message_state.number_of_requests += 1

    def _get_initialize_fram_ca(self, event=None):  # pylint: disable=unused-argument
        """Request to initialize the FRAM"""
        self.debug_message_state.fram_initialization += 1
        self.debug_message_state.number_of_requests += 1

    def _get_software_version_ca(self, event=None):  # pylint: disable=unused-argument
        """Request to retrieve the software version"""
        self.debug_message_state.get_software_version += 1
        self.debug_message_state.number_of_requests += 1

    def enable_controls(self):
        """Enable all control buttons"""
        for i in self.controls:
            i.Enable()

    def disable_controls(self):
        """Disable all control buttons"""
        for i in self.controls:
            i.Disable()

    def thread_wrapper_cb(self, event=None):
        """Start/Stop the threads"""
        if not self.btn_start_action:
            self.btn_start.SetLabel("Stop")
            self.btn_start_action = True
            self.start_threads()
            self.start_process()
            self.enable_controls()
        else:
            self.disable_controls()
            self.start_sending_requests = False
            self.btn_start.SetLabel("Start")
            self.bms_state_request_btn.SetLabel("Start BMS State Request")
            self.btn_start_action = False
            self.stop_thread()
            self.stop_process()

    def start_threads(self):
        """Start the GUI sync threads."""
        self.sync_worker = SyncThread(
            self.worker_process_function, self.can_process.receive_queue
        )
        self.send_worker = PeriodicSendThread(
            self.set_send_data, self.can_process.send_queue
        )
        self.sync_worker.start()
        self.send_worker.start()

    def stop_thread(self):
        """Stop the GUI sync threads."""
        self.sync_worker.cancel()
        time.sleep(0.1)
        self.sync_worker.join(timeout=0.5)
        self.send_worker.cancel()
        time.sleep(0.1)
        self.send_worker.join(timeout=0.5)

    def start_process(self):
        """Used to inform CAN process, that the App has started, and the process
        should no longer be paused."""
        self.can_process.app_paused.clear()

    def stop_process(self):
        """Used to inform CAN process, that the App is paused."""
        self.can_process.app_paused.set()

    def set_send_data(self) -> Union[list[can.Message], None]:
        """Returns CAN messages to be sent, composed with information obtained
        from the GUI."""
        msgs = []
        if self.start_sending_requests:
            time_ms = round(time.time() * 1000)
            if time_ms - self.bms_state_request_last_entry > 95:
                if time_ms - self.bms_state_request_last_entry > 105:
                    pass  # todo
                if self.activate_balancing:
                    balancing_threshold = self.balancing_threshold
                else:
                    balancing_threshold = 0
                msg = _set_bms_state_request_message(
                    self.state_request_msg,
                    self.mode_request,
                    self.activate_balancing,
                    balancing_threshold,
                )
                self.bms_state_request_last_entry = time_ms
                msgs.append(msg)
        if self.debug_message_state.number_of_requests:
            msgs.extend(self.debug_msg.process_debug_messages(self))
        return msgs

    def worker_process_function(self, value):
        """wrapper to call the GUI update function as an *CallAfter* function."""
        wx.CallAfter(self.update_process_label, value)

    def update_process_label(self, value: can.message.Message):
        """Calls some callbacks that update the respective GUI elements."""
        if _get_message_type(value, self.cell_voltages_msg_id):
            self.f_cell_voltages.update_cell_voltages(value)
        elif _get_message_type(value, self.cell_temperatures_msg_id):
            self.f_cell_temperatures.update_cell_temperatures(value)
        elif _get_message_type(value, self.debug_response_msg_id):
            decode_debug_response(value, self.dbc)

    def CloseFrameSafely(self, event: wx.Event):  # pylint: disable=invalid-name
        """Close the frame safely:

        - move the CAN process to idle state
        - tell the process, that this(!) app has been canceled
        - Destroy all generated sub frames
        - stop all threads that where used to update the GUI"""
        # use g_idle first to omit race condition
        self.can_process.g_idle.set()
        self.can_process.app_canceled.set()
        self.can_process.locked.release()
        self.is_running = False
        if getattr(self, "f_cell_temperatures", None):
            self.f_cell_temperatures.NewDestroy()
        if getattr(self, "f_cell_voltages", None):
            self.f_cell_voltages.NewDestroy()
        if (self.sync_worker and self.sync_worker.is_alive()) or (
            self.send_worker and self.send_worker.is_alive()
        ):
            self.stop_thread()
        if event:
            event.Skip()
        return super().Destroy()

    def onLvacFocus(  # pylint: disable=invalid-name,unused-argument
        self, event: wx.Event
    ):
        """Focus all LVAC frames and bring them to top"""
        if getattr(self, "f_cell_temperatures", None):
            self.f_cell_temperatures.Raise()
            self.f_cell_temperatures.Iconize(False)
        if getattr(self, "f_cell_voltages", None):
            self.f_cell_voltages.Raise()
            self.f_cell_voltages.Iconize(False)
        self.Raise()


@click.command()
@click.help_option("--help", "-h")
@click.option("-v", "--verbose", **C_SETUP_VERBOSITY)
@click.option("-c", "--can-adapter", **C_SETUP_CAN_ADAPTERS)
@click.option("-b", "--baud-rate", **C_SETUP_BAUD_RATE)
@click.option("-l", "--logging", "logging_dir", **C_SETUP_LOGGING)
def main(verbose: int, can_adapter: str, baud_rate: str, logging_dir: click.Path):
    """main entry point for the LVAC GUI application"""
    log_level = LOG_LEVELS[min(verbose, max(LOG_LEVELS.keys()))]
    logging.basicConfig(level=log_level)
    logging.debug(f"Logging level: {logging.getLevelName(log_level)}")

    app = wx.App()
    wx.Locale(wx.LANGUAGE_ENGLISH)

    if not can_adapter:
        can_adapter = chose_can_adapter()
    else:
        can_adapter = try_to_select_can_adapter(can_adapter)
    logging.debug(f"CAN adapter: {can_adapter}")

    if can_adapter and not baud_rate:
        baud_rate = chose_baud_rate()
    logging.debug(f"BAUD rate: {baud_rate}")

    if can_adapter and baud_rate:
        logging_dir = chose_logging_dir(logging_dir)
    logging.debug(f"CAN Logging: {logging_dir}")

    validate_argument_combination(can_adapter, baud_rate, logging_dir)

    can_process = None
    if can_adapter and baud_rate:
        can_process = CanAdapterProcess(can_adapter, baud_rate, Path(logging_dir))
        can_process.locked.acquire(block=False)
        can_process.daemon = True
        can_process.start()
        LiveViewAndControlMainFrame(can_process=can_process)
    app.MainLoop()

    if can_process:
        if not can_process.g_canceled.is_set():
            logging.debug("(1/2) start canceling CanAdapterProcess")
            can_process.g_cancel()
            logging.debug("(2/2) done canceling CanAdapterProcess")
        can_process.join()
    logging.debug("exit")


if __name__ == "__main__":
    main()  # pylint: disable=no-value-for-parameter
