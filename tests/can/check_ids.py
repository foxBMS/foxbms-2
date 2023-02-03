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

"""Python script to check if all callbacks defined in the .dbc file are
implemented in callback functions."""
import argparse
import logging
import os
import re
import sys
from pathlib import Path
from dataclasses import dataclass
from enum import Enum, auto

import cantools
import tabulate

HAVE_GIT = False
try:
    from git import Repo
    from git.exc import InvalidGitRepositoryError

    HAVE_GIT = True
except ImportError:
    pass

FILE_RE = r"\(in:([a-z_-]{1,}\.c):([A-Z]{2,5}_.*), fv:((tx)|(rx))\)"
FILE_RE_COMPILED = re.compile(FILE_RE)


def get_git_root(path: str) -> str:
    """helper function to find the repository root

    Args:
        path (string): path of test_f_guidelines

    Returns:
        root (string): root path of the git repository
    """
    root = os.path.join(os.path.dirname(path), "..", "..", "..")
    if HAVE_GIT:
        try:
            repo = Repo(path, search_parent_directories=True)
            root = repo.git.rev_parse("--show-toplevel")
        except InvalidGitRepositoryError:
            pass
    return root


ROOT = Path(get_git_root(os.path.realpath(__file__)))
BDC_DIR_REL = Path(os.path.join("tools", "dbc"))
SRC_DIR_REL = Path(os.path.join("src", "app"))
DRIVER_CONFIGURATION_FILES = Path(os.path.join("driver", "config"))
RX_MESSAGES = (
    ROOT
    / SRC_DIR_REL
    / DRIVER_CONFIGURATION_FILES
    / Path("can_cfg_rx-message-definitions.h")
)
TX_MESSAGES = (
    ROOT
    / SRC_DIR_REL
    / DRIVER_CONFIGURATION_FILES
    / Path("can_cfg_tx-message-definitions.h")
)


class RxTx(Enum):
    """Defines whether a message is to be received or transmitted"""

    Tx = auto()  # pylint: disable=invalid-name
    Rx = auto()  # pylint: disable=invalid-name


@dataclass
class ExpectedCanMessageDefines:
    """container for defines"""

    msg_name: str
    exp_message_id_macro: str
    exp_period_macro: str
    exp_phase_macro: str
    exp_full_message_macro: str
    msg_id: str
    direction: RxTx


@dataclass
class FoundCanMessageDefine:
    """container for defines"""

    define_name: str
    msg_id: str
    where: str
    not_periodic: bool = False


def construct_msg_define(msg) -> ExpectedCanMessageDefines:
    """Create the base expected define name for the message"""
    define_name: str = msg.name
    if define_name.lower().startswith("foxbms_"):
        define_name = define_name[7:]
    basic_define_name = define_name[0].upper()
    message_macro = basic_define_name
    for i, char in enumerate(define_name[1:]):
        if define_name[i].islower() and char.isupper():  # i!!!
            message_macro = message_macro + "_"
        elif define_name[i].isnumeric() and char.isupper():  # i!!!
            message_macro = message_macro + "_"
        message_macro = message_macro + char.upper()
    message_id_macro = message_macro + "_ID"
    period_macro = message_macro + "_PERIOD_ms"
    phase_macro = message_macro + "_PHASE_ms"
    full_message_macro = message_macro + "_MESSAGE"

    # once we are here, we know that this regex will match
    m = FILE_RE_COMPILED.search(msg.comment)  # pylint: disable=invalid-name
    if m.group(3).lower() == "rx":
        direction = RxTx.Rx
        pref = "CANRX_"
        phase_macro = "Rx has no phase"  # receive messages have ne phase
        message_id_macro = pref + message_id_macro
        period_macro = pref + period_macro
        full_message_macro = pref + full_message_macro
    elif m.group(3).lower() == "tx":
        direction = RxTx.Tx
        pref = "CANTX_"
        if msg.cycle_time:
            phase_macro = pref + phase_macro
            period_macro = pref + period_macro
            message_id_macro = pref + message_id_macro
            full_message_macro = pref + full_message_macro
        else:
            # there will be at least one whitespace in the file, and therefore
            # we just search for that and treat that as 'macro'
            phase_macro = " "
            period_macro = " "
            message_id_macro = pref + message_id_macro
            full_message_macro = " "
    else:
        sys.exit("Something went wrong.")

    logging.debug(f"created define '{message_id_macro}' for '{msg.name}'.")
    logging.debug(f"created define '{period_macro}' for '{msg.name}'.")
    if phase_macro != "Rx has no phase":
        logging.debug(f"created define '{phase_macro}' for '{msg.name}'.")
    logging.debug(f"created define '{full_message_macro}' for '{msg.name}'.")
    return ExpectedCanMessageDefines(
        msg.name,
        message_id_macro,
        period_macro,
        phase_macro,
        full_message_macro,
        hex(msg.frame_id),
        direction,
    )


def get_defines_from_file(
    file_to_check: Path, selector: str
) -> list[FoundCanMessageDefine]:
    """Generate a list of specific CAN defines found in a provided file."""
    defines = []
    with open(file_to_check, "r", encoding="utf-8") as f:
        txt = f.read()
    for i, line in enumerate(txt.splitlines()):
        line: str = line.strip()
        if not line.startswith("#define"):
            continue
        if not selector in line:
            continue
        line = line.split()
        found_define = FoundCanMessageDefine(
            line[1], line[2][1:-1], f"{file_to_check}:{i-1}"
        )
        if found_define.msg_id.endswith("u"):
            found_define.msg_id = found_define.msg_id[:-1]
        if "check_ids:not-periodic" in line:
            found_define.not_periodic = True
        defines.append(found_define)
        logging.debug(found_define)
    return defines


def log_found_msgs(
    selector: str, implemented_defines: list[FoundCanMessageDefine]
) -> None:
    """Logging helper for found messages"""
    header = ["Name", "Define name", "ID"]
    table = []
    for i in implemented_defines:
        table.append([i.where, i.define_name, i.msg_id])
    table = tabulate.tabulate(table, header, tablefmt="pipe")
    logging.info(f"Implemented {selector} defines are:\n{table}\n")


def log_found(expected_define_name, where) -> None:
    """Logging helper for found messages"""
    logging.debug(f"-> Found expected: {expected_define_name} @ {where}")


def log_not_found(exp: ExpectedCanMessageDefines, i: str, expected_file: str) -> None:
    """Logging helper for NOT found messages"""
    logging.error(
        f"Did not find {exp.direction.name} implementation of expected macro '{i}' for"
        f"'{exp.exp_message_id_macro} ({exp.msg_id})' in '{expected_file}'."
    )


def main():  # pylint: disable=too-many-branches
    """This script checks that the CAN message IDs that are defined in the dbc
    file are correctly implemented."""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v",
        "--verbosity",
        dest="verbosity",
        action="count",
        default=0,
        help="set verbosity level",
    )
    parser.add_argument(
        "-i",
        "--input-file",
        dest="input_file",
        action="store",
        default=BDC_DIR_REL / "foxbms.dbc",
        help="DBC file to be verified.",
    )
    parser.add_argument(
        "-t",
        "--tx-message-definition-file",
        dest="tx_message_definition_file",
        action="store",
        default=TX_MESSAGES,
        help="Path to file containing the implementation TX CAN message IDs",
    )
    parser.add_argument(
        "-r",
        "--rx-message-definition-file",
        dest="rx_message_definition_file",
        action="store",
        default=RX_MESSAGES,
        help="Path to file containing the implementation RX CAN message IDs",
    )
    args = parser.parse_args()

    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)
    logging.debug(args)

    if not isinstance(args.input_file, Path):
        args.input_file = Path(args.input_file)
    input_file = args.input_file.absolute()
    can_db = cantools.database.load_file(input_file)
    sorted_messages = sorted(can_db.messages, key=lambda x: x.frame_id)

    errors = 0
    for i in sorted_messages:
        if not i.comment:
            errors += 1
            logging.error(
                f"Could not find any comment for message '{i.name}' ({hex(i.frame_id)})."
            )
            continue
        logging.info(
            f"Found comment for message '{i.name}' ({hex(i.frame_id)}): '{i.comment}'."
        )
        m = FILE_RE_COMPILED.search(i.comment)  # pylint: disable=invalid-name
        if not m:
            errors += 1
            logging.error(
                "Could not find comment for message "
                f"'{i.name}' ({hex(i.frame_id)}) that matches '{FILE_RE}'."
            )
            continue
    if errors:
        return errors

    if not isinstance(args.tx_message_definition_file, Path):
        args.tx_message_definition_file = Path(args.tx_message_definition_file)
    if not args.tx_message_definition_file.is_file():
        sys.exit(f"'{args.tx_message_definition_file}' is not a valid file path.")

    if not isinstance(args.rx_message_definition_file, Path):
        args.rx_message_definition_file = Path(args.rx_message_definition_file)
    if not args.rx_message_definition_file.is_file():
        sys.exit(f"'{args.rx_message_definition_file}' is not a valid file path.")

    expected_defines = [construct_msg_define(msg) for msg in sorted_messages]
    header = [
        "Name",
        "Expected message macro",
        "Expected period macro",
        "Expected phase macro",
        "Full message macro",
        "ID",
        "Direction",
    ]
    table = []
    for i in expected_defines:
        table.append(
            [
                i.msg_name,
                i.exp_message_id_macro,
                i.exp_period_macro,
                i.exp_phase_macro,
                i.exp_full_message_macro,
                i.msg_id,
                i.direction.name,
            ]
        )
    table = tabulate.tabulate(table, header, tablefmt="pipe")
    logging.info(f"Expected defines are:\n{table}\n")

    all_tx_defines = get_defines_from_file(args.tx_message_definition_file, "CANTX")
    implemented_tx_defines = [
        i
        for i in all_tx_defines
        if (i.define_name.startswith("CANTX_") and i.define_name.endswith("_ID"))
    ]
    log_found_msgs("TX", implemented_tx_defines)

    all_rx_defines = get_defines_from_file(args.rx_message_definition_file, "CANRX")
    implemented_rx_defines = [
        i
        for i in all_rx_defines
        if (i.define_name.startswith("CANRX_") and i.define_name.endswith("_ID"))
    ]
    log_found_msgs("RX", implemented_rx_defines)
    errors = len(expected_defines) * 4
    for exp in expected_defines:
        for i in [
            exp.exp_message_id_macro,
            exp.exp_period_macro,
            exp.exp_phase_macro,
            exp.exp_full_message_macro,
        ]:
            if i == " ":
                errors -= 1
                continue
            found = False
            if exp.direction == RxTx.Rx:
                implemented_macros = all_rx_defines
                expected_file = args.rx_message_definition_file
                if i == "Rx has no phase":  # rx messages do not have a phase
                    found = True
                    errors -= 1
                    continue
            elif exp.direction == RxTx.Tx:
                implemented_macros = all_tx_defines
                expected_file = args.tx_message_definition_file
            else:
                sys.exit("Something went really wrong...")
            logging.debug(f"Searching {i} ({exp.msg_id}) in {expected_file}")
            for macro in implemented_macros:
                logging.debug(f"  Comparing against {macro.define_name}")
                if macro.define_name == i:
                    log_found(i, macro.where)
                    errors -= 1
                    found = True
                    break
            if not found:
                log_not_found(exp, i, expected_file)

        # check if the id, period and phase macro appear twice in the message
        # definition file. If so, we can assume that it has been defined once
        # and used a second time.
        not_periodic_signals = [
            i.define_name for i in all_tx_defines + all_rx_defines if i.not_periodic
        ]
        for i in [
            exp.exp_message_id_macro,
            exp.exp_period_macro,
            exp.exp_phase_macro,
        ]:
            if i in not_periodic_signals:
                continue
            if exp.direction == RxTx.Rx:
                expected_file = args.rx_message_definition_file
                if i == "Rx has no phase":  # rx messages do not have a phase
                    continue
            elif exp.direction == RxTx.Tx:
                expected_file = args.tx_message_definition_file
            else:
                sys.exit("Something went really wrong...")

            with open(expected_file, "r", encoding="utf-8") as f:
                txt = f.read()
            # some message might start with the same name, therefore checking
            # if it occurs two times is not good, but modulo 2 and no remainder
            # is!
            if not txt.count(i) % 2 == 0 and i != " ":
                errors += 1
                logging.error(
                    f"Expected to find {i} twice: definition and in *_MESSAGE usage.\n"
                    f"Found it {txt.count(i)} times."
                )

    return errors


if __name__ == "__main__":
    nr_of_errors = main()
    if nr_of_errors:
        logging.error(f"{nr_of_errors} errors found.")
    sys.exit(nr_of_errors)
