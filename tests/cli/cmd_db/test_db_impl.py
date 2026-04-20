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

"""Testing file 'cli/cmd_db/db_impl.py'."""

import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.cmd_db.db_impl import db_list, db_show
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_db.db_impl import db_list, db_show


class TestDbList(unittest.TestCase):
    """Tests for db_list()."""

    def test_lists_and_prints_cells(self):
        """Should instantiate FoxDB, list cells, and print them via secho."""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)

            with (
                patch("cli.cmd_db.db_impl.FoxDB") as mock_db,
                patch("cli.cmd_db.db_impl.secho") as mock_secho,
            ):
                # Mock FoxDB instance and list_cells result
                db_instance = MagicMock()
                db_instance.list_cells.return_value = ["acme-X100", "mega-Y200"]
                mock_db.return_value = db_instance

                db_list(root)

                # FoxDB was constructed with provided root
                mock_db.assert_called_once_with(root)

                # secho first prints the header, then each cell id
                self.assertEqual(len(mock_secho.call_args_list), 3)
                self.assertEqual(
                    mock_secho.call_args_list[0].args[0],
                    "Cell database contains the following cells:",
                )
                self.assertEqual(mock_secho.call_args_list[1].args[0], "acme-X100")
                self.assertEqual(mock_secho.call_args_list[2].args[0], "mega-Y200")

    def test_systemexit_bubbles_up_and_no_output(self):
        """Should propagate SystemExit from FoxDB constructor and not print anything."""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)

            with (
                patch("cli.cmd_db.db_impl.FoxDB") as mock_db,
                patch("cli.cmd_db.db_impl.secho") as mock_secho,
            ):
                mock_db.side_effect = SystemExit(1)

                with self.assertRaises(SystemExit) as ctx:
                    db_list(root)
                self.assertEqual(ctx.exception.code, 1)

                # No output expected when constructor fails
                mock_secho.assert_not_called()


class TestDbShow(unittest.TestCase):
    """Tests for db_show()."""

    def test_prints_cell_when_found(self):
        """Should print the string representation of the cell when found."""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)

            class DummyCell:  # pylint: disable=too-few-public-methods
                """Dummy Cell"""

                def __str__(self):
                    return "CELL(ACME-X100)"

            with (
                patch("cli.cmd_db.db_impl.FoxDB") as mock_db,
                patch("cli.cmd_db.db_impl.secho") as mock_secho,
            ):
                db_instance = MagicMock()
                db_instance.show_cell.return_value = DummyCell()
                mock_db.return_value = db_instance

                db_show(root, "acme-X100")

                # FoxDB was constructed and show_cell called with the id
                mock_db.assert_called_once_with(root)
                db_instance.show_cell.assert_called_once_with("acme-X100")

                # secho prints the cell's string
                mock_secho.assert_called_once()
                self.assertEqual(mock_secho.call_args.args[0], "CELL(ACME-X100)")

    def test_prints_not_found_message_when_missing(self):
        """Should print 'not found' message when the cell is missing."""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)

            with (
                patch("cli.cmd_db.db_impl.FoxDB") as mock_db,
                patch("cli.cmd_db.db_impl.secho") as mock_secho,
            ):
                db_instance = MagicMock()
                db_instance.show_cell.return_value = None
                mock_db.return_value = db_instance

                db_show(root, "missing-id")

                mock_db.assert_called_once_with(root)
                db_instance.show_cell.assert_called_once_with("missing-id")

                mock_secho.assert_called_once()
                self.assertEqual(
                    mock_secho.call_args.args[0], "Cell not found in database."
                )


if __name__ == "__main__":
    unittest.main()
