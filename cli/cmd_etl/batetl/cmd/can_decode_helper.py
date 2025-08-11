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

"""CAN decode subcommand implementation"""

import sys
from io import TextIOWrapper
from pathlib import Path

from cantools import database
from cantools.database.can.database import Database

from ....helpers.click_helpers import recho
from ..etl.can_decode import CANDecode


def can_decode_setup(config: dict) -> CANDecode:
    """Reads config file and creates the CANDecode object

    :param config: Path to the configuration file
    :return: CANDecode object
    """
    validate_decode_config(config)
    config["dbc"] = get_cantools_database(config["dbc"])
    return CANDecode(**config)


def get_cantools_database(dbc_path: Path) -> Database:
    """Loads cantools database and returns it

    :param dbc_path: Path to the dbc file
    :return: Cantools database
    """
    try:
        db = database.load_file(dbc_path, database_format="dbc", encoding="utf-8")
        if not isinstance(db, database.can.database.Database):
            recho("Not a 'Database'.")
            raise SystemExit(1)
        return db
    except database.UnsupportedDatabaseFormatError:
        recho("Invalid DBC file.")
        raise SystemExit(1) from None
    except FileNotFoundError:
        recho("DBC file not found.")
        raise SystemExit(1) from None


def validate_decode_config(config: dict) -> None:
    """Validates the configuration file of the decode subcommand

    :param config: Dictionary with configurations
    """
    dbc = config["dbc"]
    if dbc is None or not isinstance(dbc, Path):
        recho("'dbc' configuration is not a Path.")
        sys.exit(1)
    timestamp_pos = config["timestamp_pos"]
    if timestamp_pos is None or not isinstance(timestamp_pos, int):
        recho("'timestamp_pos' is not an integer.")
        sys.exit(1)
    id_pos = config["id_pos"]
    if id_pos is None or not isinstance(id_pos, int):
        recho("'id_pos' is not an integer.")
        sys.exit(1)
    data_pos = config["data_pos"]
    if data_pos is None or not isinstance(data_pos, int):
        recho("'data_pos' is not an integer.")
        sys.exit(1)


def run_decode2stdout(decode_obj: CANDecode) -> None:
    """Executes the can decode step

    :param decode_obj: Object which handles the decoding
    """
    one_message_decoded = False
    try:
        for msg in sys.stdin:
            msg_name, msg_decoded = decode_obj.decode_msg(msg)
            if msg_name and msg_decoded:
                one_message_decoded = True
                sys.stdout.write(msg_decoded)
    finally:
        if not one_message_decoded:
            recho("No CAN message was decoded. Check configuration file.")


def run_decode2file(decode_obj: CANDecode) -> None:
    """Executes the can decode step

    :param decode_obj: Object which handles the decoding
    :param output_directory: Directory in which the decoded data should be
        saved
    """
    # due to the generic setup 'output_directory' could also be None. This can
    # not happen on this branch (but on the one that calls
    # 'run_decode2stdout'), but let's check it nevertheless.
    if not isinstance(decode_obj.output_directory, Path):
        recho("Provided output directory is invalid.")
        sys.exit(1)
    decode_obj.output_directory.mkdir(parents=True, exist_ok=True)
    output_files: dict[str, TextIOWrapper] = {}
    one_message_decoded = False
    try:
        for msg in sys.stdin:
            msg_name, msg_decoded = decode_obj.decode_msg(msg)
            if msg_name and msg_decoded:
                one_message_decoded = True
                if msg_name in output_files:
                    output_files[msg_name].write(msg_decoded)
                else:
                    file_name = decode_obj.output_directory / f"{msg_name}.json"
                    f = open(file_name, mode="w", encoding="utf-8")  # pylint: disable=consider-using-with
                    output_files[msg_name] = f
                    f.write(msg_decoded)
    finally:
        if not one_message_decoded:
            recho("No CAN message was decoded. Check configuration file.")
        for f in output_files.values():
            f.close()
