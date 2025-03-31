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
from multiprocessing import Event, Process, Queue, Value, sharedctypes

from can import Message
from click import secho

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

from ..helpers.click_helpers import recho
from ..helpers.fcan import CanBusConfig


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
    send_q: "Queue[Message]" = Queue()
    rec_q: "Queue[Message]" = Queue()
    initialized: bool = False
    msg_id: sharedctypes.Synchronized = Value("i", 0)
    msg_num: sharedctypes.Synchronized = Value("i", 0)

    def do_init(self, _: str) -> bool:
        """Starts the receive and send process, the read process and initializes the CAN bus."""
        if self.initialized:
            secho("The CAN bus has already been initialized.")
            return False
        res = initialization(
            self.rec_q,
            self.send_q,
            self.bus_cfg,
            self.network_ok,
            self.msg_id,
            self.msg_num,
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
            if self.msg_id.value != 0:
                recho(
                    f"Waiting for message with ID {self.msg_id.value} has been stopped."
                )
            self.msg_id.value = 769
            self.msg_num.value = 1
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
            if self.msg_id.value != 0:
                recho(
                    f"Waiting for message with ID {self.msg_id.value} has been stopped."
                )
            self.msg_id.value = 769
            self.msg_num.value = 1
            get_boot_timestamp(self.send_q)
            secho("Boot Timestamp has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_getrtc(self, _: str) -> None:
        """Gets the rtc time"""
        if self.initialized:
            if self.msg_id.value != 0:
                recho(
                    f"Waiting for message with ID {self.msg_id.value} has been stopped."
                )
            self.msg_id.value = 769
            self.msg_num.value = 1
            get_rtc_time(self.send_q)
            secho("RTC time has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_uptime(self, _: str) -> None:
        """Get the Uptime information"""
        if self.initialized:
            if self.msg_id.value != 0:
                recho(
                    f"Waiting for message with ID {self.msg_id.value} has been stopped."
                )
            self.msg_id.value = 769
            self.msg_num.value = 1
            get_uptime(self.send_q)
            secho("Uptime has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_buildconfig(self, _: str) -> None:
        """Gets the Build Configuration"""
        if self.initialized:
            if self.msg_id.value != 0:
                recho(
                    f"Waiting for message with ID {self.msg_id.value} has been stopped."
                )
            self.msg_id.value = 771
            self.msg_num.value = 19
            get_build_configuration(self.send_q)
            secho("Build Configuration has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_commithash(self, _: str) -> None:
        """Gets the Commit Hash the software version was built with"""
        if self.initialized:
            if self.msg_id.value != 0:
                recho(
                    f"Waiting for message with ID {self.msg_id.value} has been stopped."
                )
            self.msg_id.value = 769
            self.msg_num.value = 2
            get_commit_hash(self.send_q)
            secho("Commit Hash has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_mcuwaferinfo(self, _: str) -> None:
        """Gets the wafer information of the MCU"""
        if self.initialized:
            if self.msg_id.value != 0:
                recho(
                    f"Waiting for message with ID {self.msg_id.value} has been stopped."
                )
            self.msg_id.value = 769
            self.msg_num.value = 1
            get_mcu_wafer_info(self.send_q)
            secho("MCU Wafer information has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_mculotnumber(self, _: str) -> None:
        """Gets the lot number of the MCU"""
        if self.initialized:
            if self.msg_id.value != 0:
                recho(
                    f"Waiting for message with ID {self.msg_id.value} has been stopped."
                )
            self.msg_id.value = 769
            self.msg_num.value = 1
            get_mcu_lot_number(self.send_q)
            secho("MCU lot number has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_mcuid(self, _: str) -> None:
        """Gets the unique ID of the MCU"""
        if self.initialized:
            if self.msg_id.value != 0:
                recho(
                    f"Waiting for message with ID {self.msg_id.value} has been stopped."
                )
            self.msg_id.value = 769
            self.msg_num.value = 1
            get_mcu_id(self.send_q)
            secho("MCU ID has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_softwareversion(self, _: str) -> None:
        """Gets the software version of the BMS"""
        if self.initialized:
            if self.msg_id.value != 0:
                recho(
                    f"Waiting for message with ID {self.msg_id.value} has been stopped."
                )
            self.msg_id.value = 769
            self.msg_num.value = 1
            get_software_version(self.send_q)
            secho("Software version has been requested.")
        else:
            recho("CAN bus has to be initialized: INIT")

    def do_log(self, arg: int | str) -> bool:
        """Logs message with the given ID as often as specified: LOG [ID] [#, DEFAULT: 1]"""
        if self.initialized:
            if arg == "stop":
                secho(
                    f"Waiting for message with ID {self.msg_id.value} has been stopped.",
                    fg="green",
                )
                self.msg_id.value = 0
                self.msg_num.value = 0
                return False
            try:
                match arg:
                    case int():
                        self.msg_id.value = int(arg)
                        self.msg_num.value = 1
                    case str():
                        args = arg.split()
                        if len(args) > 1:
                            self.msg_num.value = int(args[1])
                        else:
                            self.msg_num.value = 1
                        if args[0][-1] == "h":
                            self.msg_id.value = int(args[0][:-1], 16)
                        elif args[0][:2] == "0x":
                            self.msg_id.value = int(args[0][2:], 16)
                        else:
                            self.msg_id.value = int(args[0])
                    case _:
                        raise TypeError
                secho(
                    f"Waiting for message with ID {self.msg_id.value}.",
                    fg="green",
                )
                secho("To stop logging enter LOG STOP", fg="green")
            except (TypeError, ValueError):
                self.msg_id.value = 0
                self.msg_num.value = 0
                recho(
                    "Message ID has to be given as an integer\n"
                    "or as a hexadecimal number in the format '300h' or '0x300'."
                )
            except IndexError:
                self.msg_num.value = 0
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
