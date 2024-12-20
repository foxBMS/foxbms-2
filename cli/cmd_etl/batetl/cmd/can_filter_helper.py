#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

import logging
import re
import sys
from pathlib import Path

from ..etl.can_filter import CANFilter
from . import read_config


def can_filter_setup(config: Path) -> CANFilter:
    """Reads config file and creates the CANFilter object

    :param config: Path to the configuration file
    :return: CANFilter object
    """
    config_dict = read_config(config)
    validate_filter_config(config_dict)
    return CANFilter(**config_dict)


def validate_filter_config(config_dict: dict) -> None:
    """Validates the configuration file of the filter subcommand

    :param config_dict: Dictionary with configurations
    """
    if "id_pos" not in config_dict:
        logging.error("Configuration file is missing 'id_pos' parameter.")
        sys.exit(1)
    if not isinstance(config_dict["id_pos"], int):
        logging.error("'id_pos' in the configuration file is not an integer.")
        sys.exit(1)
    if "ids" not in config_dict:
        logging.error("'Configuration file is missing 'ids' parameter'.")
        sys.exit(1)
    if not isinstance(config_dict["ids"], list):
        logging.error("'ids' is not a list.")
        sys.exit(1)
    if not all(isinstance(x, str) for x in config_dict["ids"]):
        logging.error("Not all ids are defined as string. Missing quotes ?")
        sys.exit(1)
    for i in config_dict["ids"]:
        if not re.search("^[0-9A-F]+(-[0-9A-F]+){0,1}$", i):
            logging.error("'ids' are not defined as hexadecimal values!")
            sys.exit(1)
    if "sampling" in config_dict:
        if not isinstance(config_dict["sampling"], dict):
            logging.error("'sampling' is not a dictionary.")
            sys.exit(1)
        if not all(isinstance(x, str) for x in config_dict["sampling"]):
            logging.error(
                "Not all ids in sampling are defined as string. Missing quotes ?"
            )
            sys.exit(1)
        if not set(config_dict["sampling"]).issubset(set(config_dict["ids"])):
            logging.error("Defined sampling is not a subset of the ids.")
            sys.exit(1)


def run_filter(filter_obj: CANFilter) -> None:
    """Executes the filter step

    :param filter_obj: Object which handles the filtering
    """
    for msg in sys.stdin:
        filtered_msg = filter_obj.filter_msg(msg)
        if filtered_msg:
            sys.stdout.write(filtered_msg)
