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

"""CAN filter subcommand implementation"""

import re
import sys

from ....helpers.click_helpers import recho
from ..etl.can_filter import CANFilter


def can_filter_setup(config: dict) -> CANFilter:
    """Reads config file and creates the CANFilter object

    :param config: Path to the configuration file
    :return: CANFilter object
    """
    validate_filter_config(config)
    return CANFilter(**config)


def validate_filter_config(config: dict) -> None:
    """Validates the configuration file of the filter subcommand

    :param config: Dictionary with configurations
    """
    if not isinstance(config["id_pos"], int):
        recho("'id_pos' in the configuration file is not an integer.")
        sys.exit(1)

    if not isinstance(config["ids"], list):
        recho("'ids' is not a list.")
        sys.exit(1)
    if not all(isinstance(x, str) for x in config["ids"]):
        recho("Not all ids are defined as string. Missing quotes ?")
        sys.exit(1)
    for i in config["ids"]:
        if not re.search("^[0-9A-F]+(-[0-9A-F]+){0,1}$", i):
            recho("'ids' are not defined as hexadecimal values!")
            sys.exit(1)

    if "sampling" not in config:
        return
    if not isinstance(config["sampling"], dict):
        recho("'sampling' is not a dictionary.")
        sys.exit(1)
    if not all(isinstance(x, str) for x in config["sampling"]):
        recho("Not all ids in sampling are defined as string. Missing quotes?")
        sys.exit(1)
    if config["sampling"] and not set(config["sampling"]).issubset(set(config["ids"])):
        recho("Defined sampling is not a subset of the ids.")
        sys.exit(1)


def _sanitize_args(filter_obj: CANFilter) -> None:
    """Ensure that, if input and/or outout files are provided, these files are
    readable/writeable."""
    if filter_obj.output:
        try:
            filter_obj.output.parent.mkdir(exist_ok=True, parents=True)
        except PermissionError:
            recho(f"Directory '{filter_obj.output.parent.resolve()}' is not writeable.")
            sys.exit(1)
        # we have the directory, check if we can write the file
        try:
            filter_obj.output.touch()
        except PermissionError:
            recho(f"'{filter_obj.output.resolve()}' is not writeable.")
            sys.exit(1)

    if filter_obj.input:
        if not filter_obj.input.is_file():
            recho(f"'{filter_obj.input.resolve()}' does not exist.")
            sys.exit(1)
        try:
            with open(filter_obj.input, encoding="utf-8"):
                pass
        except OSError:
            recho(f"'{filter_obj.input.resolve()}' is not readable.")
            sys.exit(1)


def run_filter(filter_obj: CANFilter) -> None:
    """Executes the filter step

    :param filter_obj: Object which handles the filtering
    """
    # All cases need to be handled separately:
    # - 1/4: stdin/stdout
    # - 2/4: stdin/file
    # - 3/4: file/stdout
    # - 4/4: file/file

    _sanitize_args(filter_obj)

    # input is valid
    # 1/4
    if filter_obj.input is None and filter_obj.output is None:
        # use one global catch for OSError when trying to write to stdout
        # as wrapping every call to sys.stdout.write into a try-expect
        # block is a bit inefficient
        try:
            for msg in sys.stdin:
                filtered_msg = filter_obj.filter_msg(msg)
                if filtered_msg:
                    sys.stdout.write(filtered_msg)
        except (OSError, TypeError, ValueError, UnicodeEncodeError):
            # If can_decoded receives an extra argument, can_filter is not
            # able to write to the stdout
            recho("Could not write to stdout.")
            sys.exit(1)
    # 2/4
    elif filter_obj.input is None and filter_obj.output:
        with open(filter_obj.output, mode="w+", encoding="utf-8") as f:
            for msg in sys.stdin:
                filtered_msg = filter_obj.filter_msg(msg)
                if filtered_msg:
                    f.write(filtered_msg)
    # 3/4
    elif filter_obj.input and filter_obj.output is None:
        with open(filter_obj.input, encoding="utf-8") as f:
            # use one global catch for OSError when trying to write to stdout
            # as wrapping every call to sys.stdout.write into a try-expect
            # block is a bit inefficient
            try:
                for msg in f:
                    filtered_msg = filter_obj.filter_msg(msg)
                    if filtered_msg:
                        sys.stdout.write(filtered_msg)
            except (OSError, TypeError, ValueError, UnicodeEncodeError):
                # If can_decoded receives an extra argument, can_filter is not
                # able to write to the stdout
                recho("Could not write to stdout.")
                sys.exit(1)
    # 4/4
    elif filter_obj.input and filter_obj.output:  # pragma: no cover
        with (
            open(filter_obj.input, encoding="utf-8") as f_in,
            open(filter_obj.output, mode="w", encoding="utf-8") as f_out,
        ):
            for msg in f_in:
                filtered_msg = filter_obj.filter_msg(msg)
                if filtered_msg:
                    f_out.write(filtered_msg)
