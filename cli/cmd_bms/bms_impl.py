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

"""Implementation of the functionalities behind the 'bms' command"""

from dataclasses import asdict
from datetime import datetime
from multiprocessing import Process, Queue, sharedctypes, synchronize
from queue import Empty
from signal import SIG_IGN, SIGINT, signal
from time import sleep, time
from typing import cast

from can import Bus, CanInitializationError, CanOperationError, Message
from cantools import database
from cantools.database.can.database import Database
from click import secho

from cli.helpers.misc import APP_DBC_FILE

from ..helpers.click_helpers import recho
from ..helpers.fcan import CanBusConfig

APP_DBC: Database = cast(Database, database.load_file(APP_DBC_FILE))
MAX_CAN_OPERATION_ERRORS_PER_HOUR = 10
MESSAGE: database.can.message.Message = APP_DBC.get_message_by_name("f_Debug")
MUX_NAME = "f_Debug_Mux"
MUX_VALUES = {
    "VersionInfo": 0x00,
    "Rtc": 0x01,
    "SoftwareReset": 0x02,
    "FramInitialization": 0x03,
    "TimeInfo": 0x04,
    "UptimeInfo": 0x05,
}


class StopShell(Exception):
    """Stop the shell"""


def receive_send_can_message(
    rec_q: "Queue[Message]",
    send_q: "Queue[Message]",
    bus_cfg: CanBusConfig,
    network_ok: synchronize.Event,
) -> None:
    """Receives and sends the CAN messages from and to the bus."""
    signal(SIGINT, SIG_IGN)
    try:
        with Bus(**asdict(bus_cfg)) as bus:
            network_ok.set()
            op_errs: list[float] = []
            while True:
                try:
                    while True:
                        if msg := bus.recv(timeout=0.001):
                            rec_q.put(msg)
                        try:
                            msg = send_q.get(block=False)
                            bus.send(msg)
                        except Empty:
                            pass
                        if not network_ok.is_set():
                            raise StopShell
                except CanOperationError:
                    err_time = time()
                    op_errs = [i for i in op_errs if not (err_time - i) > 3600] + [
                        err_time
                    ]
                    if len(op_errs) >= MAX_CAN_OPERATION_ERRORS_PER_HOUR:
                        recho(
                            "Too many errors occurred while receiving and sending messages."
                        )
                        raise StopShell  # pylint: disable=raise-missing-from
    except CanInitializationError:
        recho("Could not initialize CAN bus.")
    except StopShell:
        secho("Stop receiving and sending messages.")
    network_ok.clear()


def read_can_message(
    rec_q: "Queue[Message]",
    network_ok: synchronize.Event,
    msg_id: sharedctypes.Synchronized,
    msg_num: sharedctypes.Synchronized,
) -> None:
    """Reads the CAN message and prints it if needed."""
    signal(SIGINT, SIG_IGN)
    first_timestamp: float = 0
    try:
        while network_ok.is_set():
            try:
                msg = rec_q.get(block=False)
                first_timestamp = msg.timestamp
                msg.timestamp = 0
                if (msg_id.value == msg.arbitration_id) and (msg_num.value > 0):
                    raw_msg = APP_DBC.get_message_by_frame_id(msg_id.value)
                    secho(f"Message ID {msg_id.value}: {raw_msg.decode(msg.data)}")
                    msg_num.value -= 1
                    if msg_num.value == 0:
                        secho(
                            f"All messages with ID {msg_id.value} have been printed.",
                            fg="green",
                        )
                        msg_id.value = 0
                break
            except Empty:
                pass
        while network_ok.is_set():
            try:
                msg = rec_q.get(block=False)
                msg.timestamp = msg.timestamp - first_timestamp
                if (msg_id.value == msg.arbitration_id) and (msg_num.value > 0):
                    raw_msg = APP_DBC.get_message_by_frame_id(msg_id.value)
                    secho(f"Message ID {msg_id.value}: {raw_msg.decode(msg.data)}")
                    msg_num.value -= 1
                    if msg_num.value == 0:
                        secho(
                            f"All messages with ID {msg_id.value} have been printed.",
                            fg="green",
                        )
                        msg_id.value = 0
            except Empty:
                pass
    finally:
        if first_timestamp:
            while not rec_q.empty():
                msg = rec_q.get(block=False)
                msg.timestamp = msg.timestamp - first_timestamp
                if (msg_id.value == msg.arbitration_id) and (msg_num.value > 0):
                    raw_msg = APP_DBC.get_message_by_frame_id(msg_id.value)
                    secho(f"Message ID {msg_id.value}: {raw_msg.decode(msg.data)}")
                    msg_num.value -= 1
                    if msg_num.value == 0:
                        secho(
                            f"All messages with ID {msg_id.value} have been printed.",
                            fg="green",
                        )
                        msg_id.value = 0


def initialization(  # pylint: disable=too-many-arguments, too-many-positional-arguments
    rec_q: "Queue[Message]",
    send_q: "Queue[Message]",
    bus_cfg: CanBusConfig,
    network_ok: synchronize.Event,
    msg_id: sharedctypes.Synchronized,
    msg_num: sharedctypes.Synchronized,
) -> bool | tuple[Process, Process]:
    """Initializes the process."""
    p_recv = Process(
        target=receive_send_can_message,
        args=(rec_q, send_q, bus_cfg, network_ok),
        name="p_recv",
        daemon=True,
    )
    try:
        p_recv.start()
    except (OSError, RuntimeError, ValueError):
        recho("Could not start receive and send process.\nExiting...")
        p_recv.terminate()
        return False
    if not network_ok.wait(3):
        recho("Could not initialize CAN bus. Timeout\nShutdown...")
        return False
    secho(
        "Started the receive and send process and initialized the CAN bus.",
        fg="green",
    )
    p_read = Process(
        target=read_can_message,
        args=(rec_q, network_ok, msg_id, msg_num),
        name="p_read",
    )
    p_read.start()
    secho("Started the read process.", fg="green")
    return (p_recv, p_read)


def shutdown(p_recv: Process, p_read: Process, network_ok: synchronize.Event) -> None:
    """Shuts the process down."""
    network_ok.clear()
    sleep(0.5)
    p_recv.join(timeout=2)
    if p_recv.is_alive():
        recho("Could not cancel the receive and send process gracefully.")
        recho("Terminating...")
        p_recv.terminate()
    p_read.join(timeout=2)
    if p_read.is_alive():
        recho("Could not cancel the read process gracefully.")
        recho("Terminating...")
        p_read.terminate()
    secho("Shutdown...", fg="green")


def set_debug_message(msg_data: dict[str, int]) -> Message:
    """Sets the FRAM initialization trigger message"""
    data = MESSAGE.encode(msg_data, padding=True)
    return Message(arbitration_id=MESSAGE.frame_id, data=data, is_extended_id=False)


def reinitialize_fram(send_q: "Queue[Message]") -> None:
    """Creates message to reinitialize FRAM"""
    msg_data = {MUX_NAME: MUX_VALUES["FramInitialization"], "InitializeFram": 1}
    send_q.put(set_debug_message(msg_data))


def set_rtc_time(send_q: "Queue[Message]") -> None:
    """Creates message to set the rtc to the current time"""
    date = datetime.now()
    msg_data = {
        MUX_NAME: MUX_VALUES["Rtc"],
        "SetDay": date.day,
        "SetHours": date.hour,
        "SetHundredthOfSeconds": date.microsecond // 10000,
        "SetMinutes": date.minute,
        "SetMonth": date.month,
        "SetSeconds": date.second,
        "SetWeekday": date.isoweekday() % 7,
        "SetYear": (date.year - 2000),
    }
    send_q.put(set_debug_message(msg_data))


def get_rtc_time(send_q: "Queue[Message]") -> None:
    """Creates message to get rtc time"""
    msg_data = {
        MUX_NAME: MUX_VALUES["TimeInfo"],
        "RequestRtcTime": 1,
        "RequestBootTimestamp": 0,
    }
    send_q.put(set_debug_message(msg_data))


def get_boot_timestamp(send_q: "Queue[Message]") -> None:
    """Creates message to get Boot Timestamp"""
    msg_data = {
        MUX_NAME: MUX_VALUES["TimeInfo"],
        "RequestRtcTime": 0,
        "RequestBootTimestamp": 1,
    }
    send_q.put(set_debug_message(msg_data))


def reset_software(send_q: "Queue[Message]") -> None:
    """Creates message to reset the software"""
    msg_data = {MUX_NAME: MUX_VALUES["SoftwareReset"], "TriggerSoftwareReset": 1}
    send_q.put(set_debug_message(msg_data))


def get_uptime(send_q: "Queue[Message]") -> None:
    """Creates message to get uptime"""
    msg_data = {MUX_NAME: MUX_VALUES["UptimeInfo"], "RequestUptime": 1}
    send_q.put(set_debug_message(msg_data))


def get_build_configuration(send_q: "Queue[Message]") -> None:
    """Creates message to get build configuration"""
    msg_data = {
        MUX_NAME: MUX_VALUES["VersionInfo"],
        "GetBuildConfiguration": 1,
        "GetCommitHash": 0,
        "GetMcuWaferInformation": 0,
        "GetMcuLotNumber": 0,
        "GetMcuUniqueDieId": 0,
        "GetBmsSoftwareVersion": 0,
    }
    send_q.put(set_debug_message(msg_data))


def get_commit_hash(send_q: "Queue[Message]") -> None:
    """Creates message to get commit hash"""
    msg_data = {
        MUX_NAME: MUX_VALUES["VersionInfo"],
        "GetBuildConfiguration": 0,
        "GetCommitHash": 1,
        "GetMcuWaferInformation": 0,
        "GetMcuLotNumber": 0,
        "GetMcuUniqueDieId": 0,
        "GetBmsSoftwareVersion": 0,
    }
    send_q.put(set_debug_message(msg_data))


def get_mcu_wafer_info(send_q: "Queue[Message]") -> None:
    """Creates message to get MCU Wafer information"""
    msg_data = {
        MUX_NAME: MUX_VALUES["VersionInfo"],
        "GetBuildConfiguration": 0,
        "GetCommitHash": 0,
        "GetMcuWaferInformation": 1,
        "GetMcuLotNumber": 0,
        "GetMcuUniqueDieId": 0,
        "GetBmsSoftwareVersion": 0,
    }
    send_q.put(set_debug_message(msg_data))


def get_mcu_lot_number(send_q: "Queue[Message]") -> None:
    """creates message to get MCU lot number"""
    msg_data = {
        MUX_NAME: MUX_VALUES["VersionInfo"],
        "GetBuildConfiguration": 0,
        "GetCommitHash": 0,
        "GetMcuWaferInformation": 0,
        "GetMcuLotNumber": 1,
        "GetMcuUniqueDieId": 0,
        "GetBmsSoftwareVersion": 0,
    }
    send_q.put(set_debug_message(msg_data))


def get_mcu_id(send_q: "Queue[Message]") -> None:
    """Creates message to get MCU ID"""
    msg_data = {
        MUX_NAME: MUX_VALUES["VersionInfo"],
        "GetBuildConfiguration": 0,
        "GetCommitHash": 0,
        "GetMcuWaferInformation": 0,
        "GetMcuLotNumber": 0,
        "GetMcuUniqueDieId": 1,
        "GetBmsSoftwareVersion": 0,
    }
    send_q.put(set_debug_message(msg_data))


def get_software_version(send_q: "Queue[Message]") -> None:
    """Creates message to get BMS software version"""
    msg_data = {
        MUX_NAME: MUX_VALUES["VersionInfo"],
        "GetBuildConfiguration": 0,
        "GetCommitHash": 0,
        "GetMcuWaferInformation": 0,
        "GetMcuLotNumber": 0,
        "GetMcuUniqueDieId": 0,
        "GetBmsSoftwareVersion": 1,
    }
    send_q.put(set_debug_message(msg_data))
