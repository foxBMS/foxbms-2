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

"""Testing file 'cli/db/__init__.py'."""

import io
import sys
import tempfile
import unittest
import zipfile
from contextlib import redirect_stderr
from pathlib import Path
from unittest.mock import patch

try:
    from cli.db import Cell, CellSpec, FoxDB
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.db import Cell, CellSpec, FoxDB

VALID_SPEC = {
    "name": "X1",
    "manufacturer": "ACME",
    "shape": "prismatic",
    "chemistry": "NMC",
    "height": 0.1,
    "length": 0.2,
    "width": 0.03,
    "weight": 0.5,
    "temperature_max": 50.0,
    "temperature_min": -10.0,
    "voltage_min": 2.5,
    "voltage_max": 4.2,
    "voltage_nom": 3.7,
}
VALID_MODELS = [{"name": "current limits", "param_a": 123}]


class DummyModel:  # pylint: disable=too-few-public-methods
    """Minimal stand-in for a model-parameter object used in tests.

    It simply stores the keyword arguments it was constructed with.
    """

    def __init__(self, **kwargs) -> None:
        self.kwargs = kwargs


def create_cell_directory(base: Path, directory_name: str) -> Path:
    """Create a minimal valid cell directory structure under base/directory_name for tests.

    The function creates the required files and directories; JSON contents are
    placeholders because parsing is patched in the tests.

    Returns:
        Path: Path to the created cell directory.
    """
    cell_directory = base / directory_name
    cell_directory.mkdir()
    (cell_directory / "sources").mkdir()
    (cell_directory / "cell_datasheet.pdf").write_bytes(b"%PDF-1.4 dummy")
    (cell_directory / "README.md").write_text("# Dummy", encoding="utf-8")
    # Empty json file, content created by read_json patch)
    (cell_directory / "cell_spec.json").write_text("{}", encoding="utf-8")
    (cell_directory / "model_parameter.json").write_text("[]", encoding="utf-8")
    return cell_directory


class TestFoxDBInit(unittest.TestCase):
    """Tests for FoxDB initialization behavior."""

    def test_init_loads_cells_from_directory(self) -> None:
        """Ensure FoxDB loads cells from a root directory and validates the DB."""
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            create_cell_directory(root, "cellA")
            create_cell_directory(root, "cellB")

            with (
                patch(
                    "cli.db.read_json",
                    side_effect=[VALID_SPEC, VALID_MODELS, VALID_SPEC, VALID_MODELS],
                ),
                patch("cli.db.creates_models", return_value=[DummyModel()]),
                patch("cli.db.FoxDB._check_db") as mock_check,
            ):
                db = FoxDB(root)
                self.assertEqual(len(db.cells), 2)
                mock_check.assert_called_once()

    def test_init_raises_on_duplicate_cells(self) -> None:
        """Ensure initialization fails with an error on duplicate cells."""
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            create_cell_directory(root, "cellA")
            create_cell_directory(root, "cellB")
            dup_spec = {**VALID_SPEC}
            with (
                patch(
                    "cli.db.read_json",
                    side_effect=[dup_spec, VALID_MODELS, dup_spec, VALID_MODELS],
                ),
                patch("cli.db.creates_models", return_value=[DummyModel()]),
            ):
                buf = io.StringIO()
                with redirect_stderr(buf), self.assertRaises(SystemExit):
                    FoxDB(root)
                self.assertIn(
                    "Database not valid, because at least one cell duplicate.",
                    buf.getvalue(),
                )


class TestFoxDBListCells(unittest.TestCase):
    """Tests for listing cell identifiers."""

    def test_list_cells_returns_identifiers(self) -> None:
        """Return manufacturer-name identifiers for all cells."""
        spec1 = CellSpec(**VALID_SPEC)
        spec2 = CellSpec(**{**VALID_SPEC, "name": "X2"})
        db = FoxDB.__new__(FoxDB)  # Avoid init
        db.cells = [Cell(spec1, [DummyModel()]), Cell(spec2, [DummyModel()])]
        self.assertEqual(db.list_cells(), ["ACME-X1", "ACME-X2"])


class TestFoxDBShowCell(unittest.TestCase):
    """Tests for retrieving a single cell by its identifier."""

    def test_show_cell_returns_matching_cell(self) -> None:
        """Return the matching Cell for a known identifier, or None otherwise."""
        spec1 = CellSpec(**VALID_SPEC)
        spec2 = CellSpec(**{**VALID_SPEC, "name": "X2"})
        c1 = Cell(spec1, [DummyModel()])
        c2 = Cell(spec2, [DummyModel()])
        db = FoxDB.__new__(FoxDB)  # Avoid init
        db.cells = [c1, c2]

        self.assertIs(db.show_cell("ACME-X1"), c1)
        self.assertIs(db.show_cell("ACME-X2"), c2)
        self.assertIsNone(db.show_cell("ACME-UNKNOWN"))


class TestFoxDBGetCells(unittest.TestCase):
    """Tests for resolving the input (dir/zip) and delegating iteration."""

    def test_get_cells_from_directory(self) -> None:
        """Delegate to _iterate_root_directory when given a directory."""
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)

            # pylint: disable=protected-access
            with patch.object(FoxDB, "_iterate_root_directory") as mock_iter:
                db = FoxDB.__new__(FoxDB)
                db._get_cells(root)
                mock_iter.assert_called_once_with(root)

    def test_get_cells_from_zip(self) -> None:
        """Open a valid .zip and delegate to _iterate_root_directory."""
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            # Create zip with root directory structure
            zip_path = tmp / "cells.zip"
            root_name = "cells_root"
            cell_name = "cellA"
            with zipfile.ZipFile(zip_path, "w") as zf:
                # create needed files
                base = f"{root_name}/{cell_name}"
                zf.writestr(f"{base}/sources/", "")
                zf.writestr(f"{base}/cell_spec.json", "{}")
                zf.writestr(f"{base}/model_parameter.json", "[]")
                zf.writestr(f"{base}/cell_datasheet.pdf", "%PDF-1.4 dummy")
                zf.writestr(f"{base}/README.md", "# Dummy")

            with (
                zipfile.ZipFile(zip_path) as zf,
                patch.object(FoxDB, "_iterate_root_directory") as mock_iter,
            ):
                db = FoxDB.__new__(FoxDB)
                db._get_cells(zip_path)  # pylint: disable=protected-access
                mock_iter.assert_called_once()

    def test_get_cells_with_bad_zip(self) -> None:
        """Exit with an error message when the .zip archive is invalid."""
        with tempfile.TemporaryDirectory() as tmpdir:
            tmp = Path(tmpdir)
            # create needed files
            zip_path = tmp / "cells.zip"
            with open(zip_path, "w", encoding="utf-8") as f:
                f.write("Now the file has more content!")

            buf = io.StringIO()
            with (
                redirect_stderr(buf),
                patch.object(FoxDB, "_iterate_root_directory") as mock_iter,
                self.assertRaises(SystemExit),
            ):
                db = FoxDB.__new__(FoxDB)
                db._get_cells(zip_path)  # pylint: disable=protected-access
            mock_iter.assert_not_called()
            self.assertIn("Provided .zip file is not valid.", buf.getvalue())


class TestFoxDBIterateRootDirectory(unittest.TestCase):
    """Tests for iterating the root directory and collecting cells."""

    def test_iterate_root_directory_collects_cells(self) -> None:
        """Collect one Cell per subdirectory and validate each directory."""
        with tempfile.TemporaryDirectory() as tmpdir:
            root = Path(tmpdir)
            create_cell_directory(root, "cellA")
            create_cell_directory(root, "cellB")
            with (
                patch.object(
                    FoxDB,
                    "_get_cell",
                    return_value=Cell(CellSpec(**VALID_SPEC), [DummyModel()]),
                ),
                patch.object(
                    FoxDB, "_check_cell_directory", return_value=None
                ) as mock_check,
            ):
                db = FoxDB.__new__(FoxDB)
                cells = db._iterate_root_directory(root)  # pylint: disable=protected-access
                self.assertEqual(len(cells), 2)
                self.assertEqual(mock_check.call_count, 2)

    def test_iterate_root_directory_raises_if_not_dir(self) -> None:
        """Exit with an error if the provided path is not a directory."""
        with tempfile.TemporaryDirectory() as tmpdir:
            file_path = Path(tmpdir) / "not_a_dir.txt"
            file_path.write_text("x", encoding="utf-8")
            db = FoxDB.__new__(FoxDB)
            buf = io.StringIO()
            with redirect_stderr(buf), self.assertRaises(SystemExit):
                db._iterate_root_directory(file_path)  # pylint: disable=protected-access
                self.assertIn(
                    "Provided database/zip archive is not a directory!", buf.getvalue()
                )


class TestFoxDBGetCell(unittest.TestCase):
    """Tests for building a Cell object from a cell directory."""

    def test_get_cell_constructs_cell(self) -> None:
        """Construct a Cell from spec and model files."""
        with tempfile.TemporaryDirectory() as tmpdir:
            directory = Path(tmpdir) / "cellA"
            create_cell_directory(Path(tmpdir), "cellA")
            with (
                patch("cli.db.read_json", side_effect=[VALID_SPEC, VALID_MODELS]),
                patch("cli.db.creates_models", return_value=[DummyModel()]),
            ):
                db = FoxDB.__new__(FoxDB)
                cell = db._get_cell(directory)  # pylint: disable=protected-access
                self.assertIsInstance(cell, Cell)
                self.assertEqual(cell.spec.manufacturer, "ACME")
                self.assertEqual(len(cell.model_parameters), 1)

    def test_get_cell_raises_on_invalid_spec_typeerror(self) -> None:
        """Exit with an error when the cell spec is structurally invalid (TypeError)."""
        with tempfile.TemporaryDirectory() as tmpdir:
            directory = create_cell_directory(Path(tmpdir), "cellA")
            invalid_spec = {**VALID_SPEC}
            invalid_spec.pop("name")  # missing key -> TypeError in dataclass-init
            with (
                patch("cli.db.read_json", side_effect=[invalid_spec, VALID_MODELS]),
                patch("cli.db.creates_models", return_value=[DummyModel()]),
            ):
                db = FoxDB.__new__(FoxDB)
                buf = io.StringIO()
                with redirect_stderr(buf), self.assertRaises(SystemExit):
                    db._get_cell(directory)  # pylint: disable=protected-access
                    self.assertIn(
                        "parameter has the wrong type, is missing or unexpected", buf
                    )

    def test_get_cell_raises_on_invalid_model_valueerror(self) -> None:
        """Exit with an error when model creation raises ValueError."""
        with tempfile.TemporaryDirectory() as tmpdir:
            directory = create_cell_directory(Path(tmpdir), "cellA")
            with (
                patch("cli.db.read_json", side_effect=[VALID_SPEC, VALID_MODELS]),
                patch("cli.db.creates_models", side_effect=ValueError("bad model")),
            ):
                db = FoxDB.__new__(FoxDB)
                buf = io.StringIO()
                with redirect_stderr(buf), self.assertRaises(SystemExit):
                    db._get_cell(directory)  # pylint: disable=protected-access
                    self.assertIn("a parameter has the wrong value", buf)


class TestFoxDBCheckDb(unittest.TestCase):
    """Tests for overall database validation."""

    def test_check_db_raises_on_duplicates(self) -> None:
        """Exit with an error when duplicate cells are present."""
        spec1 = CellSpec(**VALID_SPEC)
        c1 = Cell(spec1, [DummyModel()])
        c2 = Cell(spec1, [DummyModel()])  # duplicate
        db = FoxDB.__new__(FoxDB)
        db.cells = [c1, c2]
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit):
            db._check_db()  # pylint: disable=protected-access
            self.assertIn(
                "Database not valid, because at least one cell duplicate.", buf
            )

    def test_check_db_ok(self) -> None:
        """Succeed silently when no duplicates exist."""
        spec1 = CellSpec(**VALID_SPEC)
        spec2 = CellSpec(**{**VALID_SPEC, "name": "X2"})
        db = FoxDB.__new__(FoxDB)
        db.cells = [Cell(spec1, [DummyModel()]), Cell(spec2, [DummyModel()])]
        # No error expected
        db._check_db()  # pylint: disable=protected-access


class TestFoxDBCheckCellDirectory(unittest.TestCase):
    """Tests for validating the structure of a single cell directory."""

    def test_check_cell_directory_ok(self) -> None:
        """Succeed when all required files and directories are present."""
        with tempfile.TemporaryDirectory() as tmpdir:
            directory = create_cell_directory(Path(tmpdir), "cellA")
            # No error should be raised
            FoxDB._check_cell_directory(directory)  # pylint: disable=protected-access

    def test_check_cell_directory_raises_if_not_dir(self) -> None:
        """Exit with an error when a non-directory is encountered in the root."""
        with tempfile.TemporaryDirectory() as tmpdir:
            file_path = Path(tmpdir) / "file.txt"
            file_path.write_text("x", encoding="utf-8")
            buf = io.StringIO()
            with redirect_stderr(buf), self.assertRaises(SystemExit):
                FoxDB._check_cell_directory(file_path)  # pylint: disable=protected-access
                self.assertIn(
                    "Files in database root found. Invalid database format!", buf
                )

    def test_check_cell_directory_raises_if_missing_file(self) -> None:
        """Exit with an error when required files are missing from the cell directory."""
        with tempfile.TemporaryDirectory() as tmpdir:
            directory = Path(tmpdir) / "cellA"
            directory.mkdir()
            (directory / "sources").mkdir()
            (directory / "cell_spec.json").write_text("{}", encoding="utf-8")
            (directory / "model_parameter.json").write_text("[]", encoding="utf-8")
            # missing: cell_datasheet.pdf and README.md
            buf = io.StringIO()
            with redirect_stderr(buf), self.assertRaises(SystemExit):
                FoxDB._check_cell_directory(directory)  # pylint: disable=protected-access
                self.assertIn("is missing 'cell_datasheet.pdf'.", buf)


if __name__ == "__main__":
    unittest.main()
