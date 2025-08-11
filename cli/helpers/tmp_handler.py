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

"""Implementation of the TmpHandler class to handle a temporary folder and
its files."""

import hashlib
import tempfile
from pathlib import Path

from .dirs import CACHE_DIR


class TmpHandler:
    """Class to handle temporary files"""

    def __init__(
        self,
        tmp_dir_parent: Path = CACHE_DIR,
        tmp_folder_prefix: str = "temp_data_foxcli_",
    ) -> None:
        """Initialise the TmpHandler"""
        self._tmp_dir_parent = tmp_dir_parent.resolve()
        self._tmp_folder_prefix = tmp_folder_prefix
        current_tmp_dir = self._check_for_tmp_directory()
        if current_tmp_dir is None:
            self.tmp_dir = self._create_tmp_directory()
        else:
            self.tmp_dir = current_tmp_dir

    def check_for_tmp_file(self, file_path: Path, file_extension: str) -> Path | None:
        """Check whether there the data is already saved in the temporary directory."""
        file_name = TmpHandler.get_hash_name(file_path, file_extension)
        file_path = self.tmp_dir / file_name
        if file_path.exists():
            return file_path
        return None

    def _check_for_tmp_directory(self) -> Path | None:
        """Check whether there is a temporary directory to save the data in."""
        dirs = [d for d in self._tmp_dir_parent.iterdir() if d.is_dir()]
        for d in dirs:
            if self._tmp_folder_prefix in d.name:
                return d.resolve()
        return None

    def _create_tmp_directory(self) -> Path:
        """Create a temporary directory to save the data in."""
        return Path(
            tempfile.mkdtemp(prefix=self._tmp_folder_prefix, dir=self._tmp_dir_parent)
        )

    @staticmethod
    def get_hash_name(file_path: Path, file_extension: str) -> Path:
        """get_hash_name generates a file name based on the
        absolute path of a file
        """
        file_path_hash = hashlib.sha256(
            str(file_path.resolve()).encode("utf-8")
        ).hexdigest()
        file_name = "_".join([file_path_hash, file_path.stem])
        return Path(f"{file_name}.{file_extension}")
