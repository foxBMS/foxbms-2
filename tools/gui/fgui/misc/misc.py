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

"""Miscellaneous configurations etc. for all parts of the GUI"""

import logging
import re
from pathlib import Path

LOG_LEVELS = {
    0: logging.CRITICAL,
    1: logging.ERROR,
    2: logging.WARN,
    3: logging.INFO,
    4: logging.DEBUG,
}


class BatterySystemConfiguration:  # pylint: disable=too-few-public-methods
    """Battery System configuration as read from the C sources."""

    RE_CONFIG = r"#[\s]{0,1}define\s+{{FIND_CONFIG}}\s+\((\d+)+u?\)"

    def __init__(self, config_file: Path) -> None:
        txt = config_file.read_text(encoding="utf-8")
        self.bs_nr_of_strings = self._get_nr_of_strings(txt)
        self.bs_nr_of_modules_per_string = self._get_nr_of_modules_per_string(txt)
        self.bs_nr_of_cell_blocks_per_module = self._get_nr_of_cell_blocks_per_module(
            txt
        )
        self.bs_nr_of_temp_sensors_per_module = self._get_nr_of_temp_sensors_per_module(
            txt
        )

    def _get_nr_of_strings(self, txt):
        """returns the number of strings in the battery system"""
        reg = re.compile(
            BatterySystemConfiguration.RE_CONFIG.replace(
                "{{FIND_CONFIG}}", "BS_NR_OF_STRINGS"
            )
        )

        m = reg.search(txt)
        return self.__get_int_value_from_match(m)

    def _get_nr_of_modules_per_string(self, txt):
        """returns the number of modules per string in the battery system"""
        reg = re.compile(
            BatterySystemConfiguration.RE_CONFIG.replace(
                "{{FIND_CONFIG}}", "BS_NR_OF_MODULES_PER_STRING"
            )
        )

        m = reg.search(txt)
        return self.__get_int_value_from_match(m)

    def _get_nr_of_cell_blocks_per_module(self, txt):
        """returns the number of cell blocks per module in the battery system"""
        reg = re.compile(
            BatterySystemConfiguration.RE_CONFIG.replace(
                "{{FIND_CONFIG}}", "BS_NR_OF_CELL_BLOCKS_PER_MODULE"
            )
        )

        m = reg.search(txt)
        return self.__get_int_value_from_match(m)

    def _get_nr_of_temp_sensors_per_module(self, txt):
        """returns the number of temperature sensors per module in the battery system"""
        reg = re.compile(
            BatterySystemConfiguration.RE_CONFIG.replace(
                "{{FIND_CONFIG}}", "BS_NR_OF_TEMP_SENSORS_PER_MODULE"
            )
        )

        m = reg.search(txt)
        return self.__get_int_value_from_match(m)

    @staticmethod
    def __get_int_value_from_match(m):
        """returns the matched value"""
        val = 1
        if m:
            val = int(m.group(1))
        else:
            logging.error(
                "Something went wrong during parsing the "
                "battery system configuration..."
            )
        return val

    def __str__(self):
        config = (
            f"strings:                    {self.bs_nr_of_strings}\n"
            f"modules_per_string:         {self.bs_nr_of_modules_per_string}\n"
            f"cell_blocks_per_module:     {self.bs_nr_of_cell_blocks_per_module}\n"
            f"temp_sensors_per_module:    {self.bs_nr_of_temp_sensors_per_module}\n"
        )
        return config
