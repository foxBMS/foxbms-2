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

"""Testing file 'cli/commands/c_db.py'."""

import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from click.testing import CliRunner

try:
    from cli.commands import c_db
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.commands import c_db


class TestCmdList(unittest.TestCase):
    """Tests for cmd_list command."""

    def test_invokes_db_list_and_exits_zero(self):
        """Should call db_list with provided Path and exit with code 0."""
        runner = CliRunner()
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            with patch("cli.commands.c_db.db_list") as mock_db_list:
                result = runner.invoke(c_db.cmd_list, [str(root)])
                self.assertEqual(result.exit_code, 0)
                mock_db_list.assert_called_once()
                # First (and only) positional arg is the Path
                called_root = mock_db_list.call_args.args[0]
                self.assertIsInstance(called_root, Path)
                self.assertEqual(called_root, root)


class TestCmdShow(unittest.TestCase):
    """Tests for cmd_show command."""

    def test_invokes_db_show_and_exits_zero(self):
        """Should call db_show with provided Path and cell-id, then exit 0."""
        runner = CliRunner()
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            with patch("cli.commands.c_db.db_show") as mock_db_show:
                result = runner.invoke(
                    c_db.cmd_show,
                    [str(root), "--cell-id", "LG Chem-INR18650-MJ1"],
                )
                self.assertEqual(result.exit_code, 0)
                mock_db_show.assert_called_once()
                args = mock_db_show.call_args.args
                self.assertIsInstance(args[0], Path)
                self.assertEqual(args[0], root)
                self.assertEqual(args[1], "LG Chem-INR18650-MJ1")


class TestDbGroup(unittest.TestCase):
    """Tests for db group registration and behavior."""

    def test_help_lists_subcommands(self):
        """Should show 'list' and 'show' in group help."""
        runner = CliRunner()
        result = runner.invoke(c_db.db, ["--help"])
        self.assertEqual(result.exit_code, 0)
        self.assertIn("Commands:", result.output)
        self.assertIn("list", result.output)
        self.assertIn("show", result.output)

    def test_run_list_via_group(self):
        """Should run 'db list' and call db_list with provided path."""
        runner = CliRunner()
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            with patch("cli.commands.c_db.db_list") as mock_db_list:
                result = runner.invoke(c_db.db, ["list", str(root)])
                self.assertEqual(result.exit_code, 0)
                mock_db_list.assert_called_once()
                self.assertEqual(mock_db_list.call_args.args[0], root)

    def test_run_show_via_group(self):
        """Should run 'db show' and call db_show with provided path and cell-id."""
        runner = CliRunner()
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            with patch("cli.commands.c_db.db_show") as mock_db_show:
                result = runner.invoke(
                    c_db.db, ["show", str(root), "--cell-id", "ACME-X100"]
                )
                self.assertEqual(result.exit_code, 0)
                mock_db_show.assert_called_once()
                args = mock_db_show.call_args.args
                self.assertEqual(args[0], root)
                self.assertEqual(args[1], "ACME-X100")


if __name__ == "__main__":
    unittest.main()
