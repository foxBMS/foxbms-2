#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Definition of the HandlerInterface"""

import sys
from abc import ABC, abstractmethod
from pathlib import Path

import pandas as pd
from pyarrow.lib import ArrowInvalid  # pylint: disable=no-name-in-module

from ...helpers.click_helpers import recho
from ...helpers.file_tracker import FileTracker
from ...helpers.tmp_handler import TmpHandler


class DataHandlerInterface(ABC):
    """Interface defining methods to read a data file and return the data
    as pandas.DataFrame
    """

    @abstractmethod
    def get_data(self, file_path: Path, no_tmp: bool = True) -> pd.DataFrame:
        """Read the given file and returns the contained data."""

    @staticmethod
    def get_tmp_data(file_path: Path, no_tmp: bool) -> pd.DataFrame | None:
        """Return temporary data if available"""
        try:
            tmp_handler = TmpHandler(file_path.parent)
            file_tracker = FileTracker(tmp_handler.tmp_dir)
            # Check whether the data has already been saved as a parquet file
            parquet_file_path = tmp_handler.check_for_tmp_file(file_path, "parquet")
            data_file_changed = file_tracker.check_file_changed(file_path)
            if parquet_file_path is not None and not data_file_changed and not no_tmp:
                return pd.read_parquet(parquet_file_path, engine="pyarrow")
        except ArrowInvalid as e:
            recho(f"Parquet Error: {e}")
            sys.exit(1)
        return None

    @staticmethod
    def write_tmp_file(data: pd.DataFrame, file_path: Path) -> None:
        """Write a pandas Dataframe as parquet file into the temporary directory"""
        tmp_handler = TmpHandler(file_path.parent)
        parquet_file_name = tmp_handler.get_hash_name(file_path, "parquet")
        data.to_parquet((tmp_handler.tmp_dir / parquet_file_name), engine="pyarrow")
