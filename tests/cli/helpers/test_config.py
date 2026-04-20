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

"""Testing file 'cli/helpers/config.py'."""

import sys
import tempfile
import unittest
from pathlib import Path

import yaml

try:
    from cli.helpers.config import read_config
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.helpers.config import read_config


class TestReadConfig(unittest.TestCase):
    """Unit tests for the read_config function."""

    def setUp(self):
        """Set up a temporary directory and create a valid YAML file
        for testing.
        """
        # pylint: disable=R1732
        self.temp_dir = tempfile.TemporaryDirectory()
        self.yaml_path = Path(self.temp_dir.name) / "test_config.yml"
        self.test_data = {"key": "value", "list": [1, 2, 3]}
        with open(self.yaml_path, mode="w", encoding="utf-8") as f:
            yaml.dump(self.test_data, f)

    def tearDown(self):
        """Clean up the temporary directory after tests."""
        self.temp_dir.cleanup()

    def test_read_config_success(self):
        """Test that read_config correctly reads a valid YAML file
        and returns its contents.
        """
        result = read_config(self.yaml_path)
        self.assertEqual(result, self.test_data)

    def test_file_not_found(self):
        """Test that read_config raises FileNotFoundError if the
        file does not exist.
        """
        with self.assertRaises(FileNotFoundError):
            read_config(Path(self.temp_dir.name) / "non_existent.yml")

    def test_yaml_error(self):
        """Test that read_config raises yaml.YAMLError if the YAML
        file content is invalid.
        """
        invalid_yaml_path = Path(self.temp_dir.name) / "invalid.yml"
        with open(invalid_yaml_path, "w", encoding="utf-8") as f:
            f.write("key: [unclosed-list\n")
        with self.assertRaises(yaml.YAMLError):
            read_config(invalid_yaml_path)


if __name__ == "__main__":
    unittest.main()
