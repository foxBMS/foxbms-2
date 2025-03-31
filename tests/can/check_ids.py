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

"""Python script to check if all callbacks defined in the .dbc file are
implemented in callback functions."""

import argparse
import dataclasses
import json
import logging
import os
import re
import sys
from enum import Enum, auto
from pathlib import Path

import cantools
from git import Repo
from git.exc import InvalidGitRepositoryError

FILE_RE = r"\(in:([a-z_\-0-9]{1,}\.c):([A-Z]{2,5}_.*), fv:((tx)|(rx)), type:(.*)\)"
FILE_RE_COMPILED = re.compile(FILE_RE)


def get_git_root(path: str) -> str:
    """helper function to find the repository root

    Args:
        path (string): path of file in git repository

    Returns:
        root (string): root path of the git repository
    """
    root = os.path.join(os.path.dirname(path), "..", "..", "..")
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
TX_CYCLIC_MESSAGES = (
    ROOT
    / SRC_DIR_REL
    / DRIVER_CONFIGURATION_FILES
    / Path("can_cfg_tx-cyclic-message-definitions.h")
)

TX_ASYNC_MESSAGES = (
    ROOT
    / SRC_DIR_REL
    / DRIVER_CONFIGURATION_FILES
    / Path("can_cfg_tx-async-message-definitions.h")
)


class RxTx(Enum):
    """Defines whether a message is to be received or transmitted"""

    Tx = auto()  # pylint: disable=invalid-name
    Rx = auto()  # pylint: disable=invalid-name


@dataclasses.dataclass
class ExpectedCanMessageDefines:  # pylint: disable=too-many-instance-attributes
    """container for defines"""

    dbc_name: str
    dbc_id: str
    dbc_direction: RxTx
    dbc_cyclic: bool
    exp_id_macro: list[str, str]
    exp_id_type_macro: list[str, str]
    exp_period_macro: list[str, str]
    exp_phase_macro: list[str, str]
    exp_endianness_macro: list[str, str]
    exp_dlc_macro: list[str, str]
    exp_full_msg_macro: list[str, str]


@dataclasses.dataclass
class FoundCanMessageDefine:
    """container for defines"""

    define_name: str
    msg_id: str
    where: str
    cyclic: bool = True


def construct_msg_define(msg) -> ExpectedCanMessageDefines:
    """Create the base expected define name for the message"""
    define_name: str = msg.name
    if define_name.lower().startswith("f_"):
        define_name = define_name[2:]
    basic_define_name = define_name[0].upper()
    message_macro = basic_define_name
    # Split define name
    for i, char in enumerate(define_name[1:]):
        if define_name[i].islower() and char.isupper():  # i!!!
            message_macro = message_macro + "_"
        elif define_name[i].isnumeric() and char.isupper():  # i!!!
            message_macro = message_macro + "_"
        message_macro = message_macro + char.upper()
    # fmt: off
    exp_id_macro         = message_macro + "_ID"
    exp_id_type_macro    = message_macro + "_ID_TYPE"
    exp_period_macro     = message_macro + "_PERIOD_ms"
    exp_phase_macro      = message_macro + "_PHASE_ms"
    exp_endianness_macro = message_macro + "_ENDIANNESS"
    exp_dlc_macro        = message_macro + "_DLC"
    exp_full_msg_macro   = message_macro + "_MESSAGE"
    # fmt: on

    # once we are here, we know that this regex will match
    m = FILE_RE_COMPILED.search(msg.comment)  # pylint: disable=invalid-name
    if m.group(3).lower() == "rx":
        direction = RxTx.Rx
        pref = "CANRX_"
        exp_phase_macro = "Rx - ND"
        exp_period_macro = pref + exp_period_macro
        exp_full_msg_macro = pref + exp_full_msg_macro
    elif m.group(3).lower() == "tx":
        direction = RxTx.Tx
        pref = "CANTX_"

        if msg.cycle_time:
            exp_phase_macro = pref + exp_phase_macro
            exp_period_macro = pref + exp_period_macro
            exp_full_msg_macro = pref + exp_full_msg_macro
        else:
            exp_phase_macro = "Tx - async - ND"
            exp_period_macro = "Tx - async - ND"
            exp_full_msg_macro = "Tx - async - ND"
    else:
        sys.exit("Something went wrong.")

    exp_id_macro = pref + exp_id_macro
    exp_id_type_macro = pref + exp_id_type_macro
    exp_endianness_macro = pref + exp_endianness_macro
    exp_dlc_macro = pref + exp_dlc_macro
    # now we have all defines we need to search for

    logging.debug("%s:\n", msg.name)
    logging.debug("  created define '%s' for '%s'.", exp_id_macro, msg.name)
    logging.debug("  created define '%s' for '%s'.", exp_id_type_macro, msg.name)
    if not exp_period_macro.endswith("- ND"):
        logging.debug("  created define '%s' for '%s'.", exp_period_macro, msg.name)
    if not exp_phase_macro.endswith("- ND"):
        logging.debug("  created define '%s' for '%s'.", exp_phase_macro, msg.name)
    logging.debug("  created define '%s' for '%s'.", exp_endianness_macro, msg.name)
    logging.debug("  created define '%s' for '%s'.", exp_dlc_macro, msg.name)
    if not exp_full_msg_macro.endswith("- ND"):
        logging.debug("  created define '%s' for '%s'.\n", exp_full_msg_macro, msg.name)
    cyclic = False
    if msg.cycle_time:
        cyclic = True
    return ExpectedCanMessageDefines(
        msg.name,
        hex(msg.frame_id).upper().replace("X", "x"),
        direction.name.lower(),
        cyclic,
        (exp_id_macro, ""),
        (exp_id_type_macro, ""),
        (exp_period_macro, ""),
        (exp_phase_macro, ""),
        (exp_endianness_macro, ""),
        (exp_dlc_macro, ""),
        (exp_full_msg_macro, ""),
    )


def get_defines_from_file(
    file_to_check: Path,
    selector: str,
    cyclic: bool = False,
) -> list[FoundCanMessageDefine]:
    """Generate a list of specific CAN defines found in a provided file."""
    defines = []
    with open(file_to_check, encoding="utf-8") as f:
        txt = f.read()
    for i, line in enumerate(txt.splitlines()):
        line: str = line.strip()
        if not line.startswith("#define"):
            continue
        if selector not in line:
            continue
        line = line.split()
        found_define = FoundCanMessageDefine(
            line[1], line[2][1:-1], f"{file_to_check}:{i + 1}"
        )
        if found_define.msg_id.endswith("u"):
            found_define.msg_id = found_define.msg_id[:-1]
        found_define.cyclic = cyclic
        defines.append(found_define)
        logging.debug(found_define)
    return defines


def log_found_msgs(
    selector: str, implemented_defines: list[FoundCanMessageDefine]
) -> None:
    """Logging helper for found messages"""
    out_str = ""
    for i in implemented_defines:
        out_str += f"{i.where}: {i.define_name} : {i.msg_id} [{i.cyclic}]\n"
    logging.info("Implemented %s defines are:\n%s\n", selector, out_str)


def log_found(expected_define_name, where) -> None:
    """Logging helper for found messages"""
    logging.debug("-> Found expected: %s @ %s", expected_define_name, where)


def log_not_found(exp: ExpectedCanMessageDefines, i: str, expected_file: str) -> None:
    """Logging helper for NOT found messages"""
    logging.error(
        "Did not find expected macro implementation '%s' for '%s' (%s) for in '%s'.",
        i,
        exp.dbc_name,
        exp.dbc_id,
        expected_file,
    )


# pylint: disable-next=too-many-branches,too-many-locals,too-many-statements
def main():
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
        "--tx-cyclic-message-definition-file",
        dest="tx_cyclic_message_definition_file",
        action="store",
        default=TX_CYCLIC_MESSAGES,
        help="Path to file containing the implementation TX CAN message IDs",
    )
    parser.add_argument(
        "--tx-async-message-definition-file",
        dest="tx_async_message_definition_file",
        action="store",
        default=TX_ASYNC_MESSAGES,
        help="Path to file containing the implementation TX CAN message IDs",
    )
    parser.add_argument(
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
                "Could not find any comment for message '%s' (%s).",
                i.name,
                hex(i.frame_id),
            )
            continue
        logging.info(
            "Found comment for message '%s' (%s): '%s'.",
            i.name,
            hex(i.frame_id),
            i.comment,
        )
        m = FILE_RE_COMPILED.search(i.comment)  # pylint: disable=invalid-name
        if not m:
            errors += 1
            logging.error(
                "Could not find comment for message '%s' (%s) that matches '%s'.",
                i.name,
                hex(i.frame_id),
                FILE_RE,
            )
            continue
    if errors:
        sys.exit(errors)

    if not isinstance(args.rx_message_definition_file, Path):
        args.rx_message_definition_file = Path(args.rx_message_definition_file)
    if not isinstance(args.tx_async_message_definition_file, Path):
        args.tx_async_message_definition_file = Path(
            args.tx_async_message_definition_file
        )
    if not isinstance(args.tx_cyclic_message_definition_file, Path):
        args.tx_cyclic_message_definition_file = Path(
            args.tx_cyclic_message_definition_file
        )

    for i in (
        args.rx_message_definition_file,
        args.tx_async_message_definition_file,
        args.tx_cyclic_message_definition_file,
    ):
        if not i.is_file():
            errors += 1
            sys.exit(f"'{i}' is not a valid file path.")
    if errors:
        sys.exit(errors)

    expected_defines = [construct_msg_define(msg) for msg in sorted_messages]
    dump = {}
    for i in expected_defines:
        dump[i.dbc_name] = dataclasses.asdict(i)
    with open("expected-defines.json.log", "w", encoding="utf-8") as f:
        f.write(json.dumps(dump, indent=4))

    all_tx_defines = get_defines_from_file(
        args.tx_cyclic_message_definition_file,
        selector="CANTX",
        cyclic=True,
    )
    implemented_tx_defines = [
        i
        for i in all_tx_defines
        if (i.define_name.startswith("CANTX_") and i.define_name.endswith("_ID"))
    ]
    all_tx_defines.extend(
        get_defines_from_file(
            args.tx_async_message_definition_file,
            selector="CANTX",
            cyclic=False,
        )
    )
    implemented_tx_defines.extend(
        [
            i
            for i in all_tx_defines
            if (i.define_name.startswith("CANTX_") and i.define_name.endswith("_ID"))
        ]
    )
    log_found_msgs("TX", implemented_tx_defines)

    all_rx_defines = get_defines_from_file(
        args.rx_message_definition_file, "CANRX", cyclic=True
    )
    implemented_rx_defines = [
        i
        for i in all_rx_defines
        if (i.define_name.startswith("CANRX_") and i.define_name.endswith("_ID"))
    ]
    log_found_msgs("RX", implemented_rx_defines)
    # check that the ID is assigned to the correct macro
    all_implemented_defines = implemented_tx_defines + implemented_rx_defines
    for message, values in dump.items():
        for i in all_implemented_defines:
            if not values["exp_id_macro"][0] == i.define_name:
                continue
            expected_id = int(values["dbc_id"], 16)
            implemented_id = int(i.msg_id, 16)
            if implemented_id != expected_id:
                sys.exit(
                    f"The message '{message}' expects the macro "
                    f"'{values['exp_id_macro'][0]}' to implement the ID "
                    f"'{hex(expected_id)}', but it implements ID "
                    f"'{hex(implemented_id)}'."
                )

    # We expect 7 defines in the 'best' case:
    # - *_ID
    # - *_ID_TYPE
    # - *_PERIOD_ms     - only if cyclic (tx and rx)
    # - *_PHASE_ms      - only if cyclic tx
    # - *_ENDIANNESS
    # - *_DLC
    # - *_MESSAGE       - only if cyclic /tx and rx)
    errors = len(expected_defines) * 7
    for exp in expected_defines:
        for i, _ in [
            exp.exp_id_macro,
            exp.exp_id_type_macro,
            exp.exp_period_macro,
            exp.exp_phase_macro,
            exp.exp_endianness_macro,
            exp.exp_dlc_macro,
            exp.exp_full_msg_macro,
        ]:
            # all macros that do not need to be defined end marked like that.
            if i.endswith("- ND"):
                errors -= 1
                continue

            found = False
            if exp.dbc_direction == "rx":
                implemented_macros = all_rx_defines
                expected_file = args.rx_message_definition_file
            elif exp.dbc_direction == "tx":
                implemented_macros = all_tx_defines
                if exp.dbc_cyclic:
                    expected_file = args.tx_cyclic_message_definition_file
                else:
                    expected_file = args.tx_async_message_definition_file
            else:
                sys.exit("Something went really wrong when searching for macros...")

            logging.debug("Searching %s (%s) in %s", i, exp.dbc_id, expected_file)
            for macro in implemented_macros:
                logging.debug("  Comparing against %s", macro.define_name)
                if macro.define_name == i:
                    posix_where = Path(macro.where).as_posix()
                    if i == exp.exp_id_macro[0]:
                        exp.exp_id_macro = (exp.exp_id_macro[0], posix_where)
                    elif i == exp.exp_id_type_macro[0]:
                        exp.exp_id_type_macro = (exp.exp_id_type_macro[0], posix_where)
                    elif i == exp.exp_period_macro[0]:
                        exp.exp_period_macro = (exp.exp_period_macro[0], posix_where)
                    elif i == exp.exp_phase_macro[0]:
                        exp.exp_phase_macro = (exp.exp_phase_macro[0], posix_where)
                    elif i == exp.exp_endianness_macro[0]:
                        exp.exp_endianness_macro = (
                            exp.exp_endianness_macro[0],
                            posix_where,
                        )
                    elif i == exp.exp_dlc_macro[0]:
                        exp.exp_dlc_macro = (exp.exp_dlc_macro[0], posix_where)
                    elif i == exp.exp_full_msg_macro[0]:
                        exp.exp_full_msg_macro = (
                            exp.exp_full_msg_macro[0],
                            posix_where,
                        )
                    log_found(i, posix_where)
                    errors -= 1
                    found = True
                    break
            if not found:
                log_not_found(exp, i, expected_file)
    if errors:
        sys.exit(errors)

    dump = {}
    for i in expected_defines:
        dump[i.dbc_name] = dataclasses.asdict(i)
    with open("found-defines.json.log", "w", encoding="utf-8") as f:
        f.write(json.dumps(dump, indent=4))

    for i in expected_defines:
        last_match = 0
        nr_ = 1
        for j in dataclasses.asdict(i):
            attr = getattr(i, j)
            if not isinstance(attr, tuple):
                continue
            if not attr[0].startswith(("CANTX_", "CANRX_")):
                continue
            if attr[0].endswith("_MESSAGE"):
                continue
            if last_match > 0:
                if last_match + nr_ != int(attr[1].rsplit(":", maxsplit=1)[-1]):
                    logging.error("wrong line for %s", attr)
                nr_ += 1
            else:
                last_match = attr[1].rsplit(":", maxsplit=1)
                last_match = last_match[-1]
                last_match = int(last_match)
    # check if the id, period and phase macro appear twice in the message
    # definition file. If so, we can assume that it has been defined once
    # and used a second time.

    expected_tx_txt = args.tx_cyclic_message_definition_file.read_text(encoding="utf-8")
    expected_rx_txt = args.rx_message_definition_file.read_text(encoding="utf-8")

    end_re = r"\s+}\n"
    for i in expected_defines:
        if not i.dbc_cyclic and i.dbc_direction == "tx":
            continue
        start_re = (
            rf"#define\s+{i.exp_full_msg_macro[0]}\s+\\\n"
            rf"\s+{{\s+\\\n"
            rf"\s+\.id\s+=\s+{i.exp_id_macro[0]},\s+\\\n"
            rf"\s+\.idType\s+=\s+{i.exp_id_type_macro[0]},\s+\\\n"
            rf"\s+\.dlc\s+=\s+{i.exp_dlc_macro[0]},\s+\\\n"
            rf"\s+\.endianness\s+=\s+{i.exp_endianness_macro[0]},\s+\\\n"
            rf"\s+}},\s+\\\n"
            r"\s+{\s+\\\n"
        )
        if i.dbc_direction == "tx":
            message_re = (
                start_re
                + rf"\s+\.period\s+=\s+{i.exp_period_macro[0]},"
                + rf"\s+\.phase\s+=\s+{i.exp_phase_macro[0]}\s+\\\n"
                + end_re
            )
            expected_txt = expected_tx_txt
        elif i.dbc_direction == "rx":
            message_re = (
                start_re + rf"\s+\.period\s+=\s+{i.exp_period_macro[0]}\s+\\\n" + end_re
            )
            expected_txt = expected_rx_txt
        else:
            logging.error(
                "variable 'message_re' did not get defined for %s. Something went wrong",
                i,
            )
            errors += 1
            continue
        m = re.search(message_re, expected_txt, flags=re.MULTILINE)
        if not m:
            errors += 1
            logging.error(
                "Could not find expected message definition for %s.",
                i.exp_full_msg_macro[0],
            )

    return errors


if __name__ == "__main__":
    nr_of_errors = main()
    if nr_of_errors:
        logging.error("%s errors found.", nr_of_errors)
    sys.exit(nr_of_errors)
