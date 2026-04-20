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

"""Testing file 'hatch_build.py'."""

import re
import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, mock_open, patch

from hatchling.metadata.core import ProjectMetadata

try:
    import hatch_build
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[2]))
    import hatch_build


class TestExtractVersion(unittest.TestCase):
    """Tests the 'extract_version' function"""

    def test_no_match(self):
        """Regex does not match"""
        txt = "Line 1\nLine 2\nLine 3"
        mock_pattern = MagicMock()
        mock_pattern.search.return_value = None
        with self.assertRaises(SystemExit) as cm:
            hatch_build.extract_version(txt, mock_pattern)
        self.assertEqual(cm.exception.code, "Could not determine foxBMS 2 version.")

    def test_version_found(self):
        """Regex matches"""
        txt = """Text 2\nLine 5\nLine 7 2"""
        result = hatch_build.extract_version(txt, re.compile(r"Line (\d{1,} \d{1,})"))
        self.assertEqual(result, "7 2")


@patch("hatch_build.extract_version")
class TestGetVersion(unittest.TestCase):
    """Tests the 'get_version' function"""

    @patch("hatch_build.Path.read_text")
    @patch("hatch_build.re.compile")
    def test_get_version(
        self,
        mock_compile: MagicMock,
        mock_read_text: MagicMock,
        mock_extract_version: MagicMock,
    ):
        """Test the function"""
        txt = '''VERSION = "x.y.z"'''
        mock_read_text.return_value = txt
        regex = "pattern"
        mock_compile.return_value = regex
        hatch_build.get_version()
        mock_extract_version.assert_called_once_with(txt, regex)


@patch("hatch_build.get_version")
class TestGetNumericVersion(unittest.TestCase):
    """Tests the 'get_numeric_version' function"""

    def test_letters(self, mock_get_version: MagicMock):
        """Test converting from letters to numbers"""
        mock_get_version.return_value = "x.y.z"
        result = hatch_build.get_numeric_version()
        self.assertEqual(result, "120.121.122")

    def test_numbers(self, mock_get_version: MagicMock):
        """Test function when version is given with numbers"""
        mock_get_version.return_value = "1.2.3"
        result = hatch_build.get_numeric_version()
        self.assertEqual(result, "1.2.3")


class TestGetDependencies(unittest.TestCase):
    """Tests the 'get_dependencies' function"""

    @patch(
        "builtins.open",
        new_callable=mock_open,
        read_data="Line1\nLine2\nLine3 #\nLine4",
    )
    def test_get_dependencies(self, _: MagicMock):
        """Test the function"""
        result = hatch_build.get_dependencies()
        self.assertEqual(result, ["Line1", "Line2", "Line3", "Line4"])


class TestCustomBuildHook(unittest.TestCase):
    """Tests the 'CustomBuildHook' class"""

    def setUp(self):  # noqa:D102
        self.build_hook = hatch_build.CustomBuildHook(
            ".", {}, None, ProjectMetadata(".", None), ".", "."
        )

    @patch("hatch_build.BuildHookInterface.initialize")
    def test_initialize(self, mock_initialize: MagicMock):
        """Tests the function 'initialize'"""
        build_data = {"artifacts": []}
        self.build_hook.initialize("version", build_data)
        self.assertEqual(build_data["artifacts"], ["version.py"])
        mock_initialize.assert_called_once()

    @patch("hatch_build.BuildHookInterface.finalize")
    def test_finalize_is_file(self, mock_finalize: MagicMock):
        """Tests the function 'finalize' when the version file exists"""
        version_file = Path(__file__).parents[2] / "cli" / "version.py"
        version_file.touch()
        self.build_hook.finalize("version", {}, ".")
        mock_finalize.assert_called_once()
        self.assertFalse(version_file.is_file())

    @patch("hatch_build.BuildHookInterface.finalize")
    def test_finalize_not_file(self, mock_finalize: MagicMock):
        """Tests the function 'finalize' when the version file exists"""
        self.build_hook.finalize("version", {}, ".")
        mock_finalize.assert_called_once()


class TestCustomMetaDataHook(unittest.TestCase):
    """Tests the 'CustomMetaDataHook' class"""

    @patch("hatch_build.get_numeric_version")
    @patch("hatch_build.get_dependencies")
    def test_update(self, mock_dependencies: MagicMock, mock_version: MagicMock):
        """Tests the function 'update'"""
        build_hook = hatch_build.CustomMetaDataHook(".", {})
        dependencies = ["dependency 1", "dependency 2"]
        mock_dependencies.return_value = dependencies
        mock_version.return_value = "version_1"
        metadata = {}
        build_hook.update(metadata)
        self.assertIn("version", metadata)
        self.assertEqual(metadata["version"], "version_1")
        self.assertIn("dependencies", metadata)
        self.assertEqual(metadata["dependencies"], dependencies)
        version_file = Path(__file__).parents[2] / "cli" / "version.py"
        self.assertTrue(version_file.is_file())
        with open(version_file, encoding="utf-8") as f:
            self.assertEqual(f.read(), 'VERSION = "version_1"')
        version_file.unlink()


if __name__ == "__main__":
    unittest.main()
