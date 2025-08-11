#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Implementation of a custom shell for the 'bms' command"""

import cmd
from multiprocessing import Event, Manager, Process, Queue, managers
from typing import cast

from can import Message
from cantools import database
from cantools.database.can.database import Database

from cli.cmd_bms.bms_impl import (
    get_boot_timestamp,
    get_build_configuration,
    get_commit_hash,
    get_mcu_id,
    get_mcu_lot_number,
    get_mcu_wafer_info,
    get_rtc_time,
    get_software_version,
    get_uptime,
    initialization,
    reinitialize_fram,
    reset_software,
    set_rtc_time,
    shutdown,
)
from cli.helpers.misc import APP_DBC_FILE

from ..helpers.click_helpers import recho, secho
from ..helpers.fcan import CanBusConfig

APP_DBC: Database = cast(Database, database.load_file(APP_DBC_FILE))


class BMSShell(cmd.Cmd):
    """Class that defines the custom bms shell"""

    intro = (
        "Welcome to the bms shell.\n"
        "Type init to initialize the CAN bus.\n"
        "Type help or ? to list all commands.\n"
    )
    prompt = "(bms) "
    bus_cfg: CanBusConfig
    p_recv: Process
    p_read: Process
    network_ok = Event()
    msg_arr: managers.ListProxy
    send_q: "Queue[Message]" = Queue()
    rec_q: "Queue[Message]" = Queue()
    initialized: bool = False

    def do_init(self, _: str) -> bool:
        """Starts the receive and send process, the read process and initializes the CAN bus."""
        if self.initialized:
            secho("The CAN bus has already been initialized.")
            return False
        res = initialization(
            self.prompt,
            self.rec_q,
            self.send_q,
            self.bus_cfg,
            self.network_ok,
            self.msg_arr,
        )
        if isinstance(res, tuple):  # pragma: no cover
            self.p_recv, self.p_read = res
            self.initialized = True
        return not self.initialized

    def do_fram(self, _: str) -> None:
        """Reinitializes the FRAM"""
        if self.initialized:
            reinitialize_fram(self.send_q)
            secho("FRAM has been reinitialized.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_rtc(self, _: str) -> None:
        """Sets the rtc time to the current time"""
        if self.initialized:
            add_msg("f_DebugResponse", self.msg_arr)
            set_rtc_time(self.send_q)
            secho("RTC time has been set:")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_softwarereset(self, _: str) -> None:
        """Triggers a software reset"""
        if self.initialized:
            reset_software(self.send_q)
            secho("Software reset has been triggered.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_boottimestamp(self, _: str) -> None:
        """Requests the Boot Timestamp"""
        if self.initialized:
            add_msg("f_DebugResponse", self.msg_arr)
            get_boot_timestamp(self.send_q)
            secho("Boot Timestamp has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_getrtc(self, _: str) -> None:
        """Gets the rtc time"""
        if self.initialized:
            add_msg("f_DebugResponse", self.msg_arr)
            get_rtc_time(self.send_q)
            secho("RTC time has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_uptime(self, _: str) -> None:
        """Get the Uptime information"""
        if self.initialized:
            add_msg("f_DebugResponse", self.msg_arr)
            get_uptime(self.send_q)
            secho("Uptime has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_buildconfig(self, _: str) -> None:
        """Gets the Build Configuration"""
        if self.initialized:
            add_msg("f_DebugBuildConfiguration", self.msg_arr, 19)
            get_build_configuration(self.send_q)
            secho("Build Configuration has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_commithash(self, _: str) -> None:
        """Gets the Commit Hash the software version was built with"""
        if self.initialized:
            add_msg("f_DebugResponse", self.msg_arr, 2)
            get_commit_hash(self.send_q)
            secho("Commit Hash has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_mcuwaferinfo(self, _: str) -> None:
        """Gets the wafer information of the MCU"""
        if self.initialized:
            add_msg("f_DebugResponse", self.msg_arr)
            get_mcu_wafer_info(self.send_q)
            secho("MCU Wafer information has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_mculotnumber(self, _: str) -> None:
        """Gets the lot number of the MCU"""
        if self.initialized:
            add_msg("f_DebugResponse", self.msg_arr)
            get_mcu_lot_number(self.send_q)
            secho("MCU lot number has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_mcuid(self, _: str) -> None:
        """Gets the unique ID of the MCU"""
        if self.initialized:
            add_msg("f_DebugResponse", self.msg_arr)
            get_mcu_id(self.send_q)
            secho("MCU ID has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_softwareversion(self, _: str) -> None:
        """Gets the software version of the BMS"""
        if self.initialized:
            add_msg("f_DebugResponse", self.msg_arr)
            get_software_version(self.send_q)
            secho("Software version has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_log(self, arg: int | str) -> bool:
        """Logs message with the given ID as often as specified and to the
        given output: LOG [ID] [#, DEFAULT: 1] [OUTPUT, FILE / stdout]"""
        if self.initialized:
            if arg == "stop":
                secho(
                    "This doesn't work yet.",
                )
                return False
            try:
                match arg:
                    case int():
                        add_msg(int(arg), self.msg_arr)
                    case str():
                        args = arg.split()
                        msg_id: int
                        num = 1
                        output = 1
                        if args[0][-1] == "h":
                            msg_id = int(args[0][:-1], 16)
                        elif args[0][:2] == "0x":
                            msg_id = int(args[0][2:], 16)
                        else:
                            msg_id = int(args[0])
                        if len(args) > 1:
                            for i in range(1, len(args)):
                                try:
                                    num = int(args[i])
                                except (TypeError, ValueError):
                                    if "file" in args[i].lower():
                                        output = 0
                                    else:
                                        recho(
                                            "For logging to a file enter the argument 'FILE'."
                                        )
                        add_msg(msg_id, self.msg_arr, num, output)
                    case _:
                        raise TypeError
                secho(
                    f"Waiting for message with ID {hex(self.msg_arr[0][-1])}.",
                    fg="green",
                )
            except (TypeError, ValueError):
                recho(
                    "Message ID has to be given as an integer\n"
                    "or as a hexadecimal number in the format '300h' or '0x300'."
                )
            except IndexError:
                recho("ID of the message to be logged has to be specified.")
        else:
            recho("CAN bus has to be initialized: INIT")
        return False

    def do_exit(self, _: str) -> bool:
        "Terminates the child process, stops the shell and exits."
        if not self.initialized:
            secho("Exiting...", fg="green")
            return True
        shutdown(self.p_recv, self.p_read, self.network_ok)
        self.initialized = False
        return True

    def default(self, line: str) -> None:
        """Method called on an input line when the command is not recognized."""
        recho(f"Invalid command: {line}")

    def precmd(self, line: str) -> str:
        """Converts the input to lowercase."""
        return str(line.lower())

    def preloop(self) -> None:
        manager = Manager()
        self.msg_arr = manager.list([[], [], []])


def add_msg(
    msg_id: str | int, msg_arr: managers.ListProxy, amount: int = 1, output: int = 1
) -> None:
    """Adds a new message to the array"""
    array_0 = msg_arr[0]
    array_1 = msg_arr[1]
    array_2 = msg_arr[2]
    match msg_id:
        case str():
            array_0.append(APP_DBC.get_message_by_name(msg_id).frame_id)
        case int():
            array_0.append(msg_id)
        case _:
            recho("Invalid message ID")
            return
    array_1.append(amount)
    array_2.append(output)
    msg_arr[0] = array_0
    msg_arr[1] = array_1
    msg_arr[2] = array_2


def run_shell(bus_cfg: CanBusConfig) -> int:
    """Runs the bms shell"""
    shell = BMSShell()
    shell.bus_cfg = bus_cfg

    try:
        shell.cmdloop()
    except (KeyboardInterrupt, Exception) as exp:  # pylint: disable=broad-exception-caught
        recho("Error detected. Please wait for the Process to terminate.")
        shell.do_exit("")
        secho(f"The shell has been stopped. {exp}")
    return 0
