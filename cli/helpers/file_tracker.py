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

"""Implementation of the FileTracker class to track the change of specified
files"""

import json
import sys
from pathlib import Path

from .click_helpers import recho
from .misc import get_sha256_file_hash_str


class FileTracker:  # pylint: disable=too-few-public-methods
    """Class to track changes in files"""

    def __init__(self, tmp_dir: Path) -> None:
        """Initialise the FileTracker"""
        if not tmp_dir.exists():
            recho(f"FileTracker: Directory {tmp_dir} for hash file does not exist!")
            sys.exit(1)
        self.tmp_dir = tmp_dir
        self._path_to_hash_json = tmp_dir / Path("hash.json")
        if not self._path_to_hash_json.exists():
            self._update_hash_json({})

    def check_file_changed(self, file_path: Path) -> bool:
        """Checks whether the passed file was previously changed based
        on hash values"""
        file_path = file_path.resolve()
        hash_dict = self._read_hash_json()
        old_file_hash = hash_dict.get(str(file_path), None)
        current_file_hash = get_sha256_file_hash_str(file_path=file_path)
        if old_file_hash:
            if old_file_hash == current_file_hash:
                return False
        # Add current hash value to hash file to track changes
        hash_dict[str(file_path)] = current_file_hash
        self._update_hash_json(hash_dict)
        return True

    def _read_hash_json(self) -> dict:
        """Reads the hash json containing the hash values of the tracked
        files"""
        with open(self._path_to_hash_json, encoding="utf-8") as f:
            return json.load(f)

    def _update_hash_json(self, hash_dict: dict) -> None:
        """Updates the hash values in the hash json file"""
        with open(self._path_to_hash_json, mode="w", encoding="utf-8") as f:
            json.dump(hash_dict, f)
