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

"""High-level interface for the cell database.

This module provides the FoxDB class which loads cell specifications and
associated model parameters from a directory or a ZIP archive. It validates
the database structure and exposes helper methods to list and query cells.
"""

import sys
import zipfile
from pathlib import Path

from ..helpers.click_helpers import recho
from .cell import Cell
from .cell_spec import CellSpec
from .setup import creates_models, read_json


class FoxDB:
    """Access and validate the foxBMS cell database.

    The database can be provided either as a directory or as a ZIP archive
    containing one root directory with cell subdirectories.

    Attributes:
        cells: All parsed cells in the database.
    """

    def __init__(self, db_root: Path) -> None:
        """Initialize the database from a root path.

        The root can point to a directory or a ZIP archive. The database
        is validated for duplicate entries and required files.

        Args:
            db_root: Path to the database directory or a ZIP archive.

        Raises:
            SystemExit: If the database structure is invalid or parsing fails.
        """
        self.cells = self._get_cells(db_root)
        self._check_db()

    def list_cells(self) -> list[str]:
        """List all cells as unique identifiers.

        The identifier format is "<manufacturer>-<name>".

        Returns:
            List of cell identifiers present in the database.
        """
        return [f"{x.spec.manufacturer}-{x.spec.name}" for x in self.cells]

    def show_cell(self, identifier: str) -> Cell | None:
        """Return a cell by its identifier.

        Args:
            identifier: Cell identifier in the format "<manufacturer>-<name>".

        Returns:
            The matching cell if found, otherwise ``None``.
        """
        for cell in self.cells:
            if f"{cell.spec.manufacturer}-{cell.spec.name}" == identifier:
                return cell
        return None

    def _get_cells(self, db_root: Path) -> list[Cell]:
        """Collect all cells from the database root.

        Args:
            db_root: Path to the database directory or ZIP archive.

        Returns:
            List of parsed cells from the root directory.

        Raises:
            SystemExit: If the root does not contain a valid directory structure.
        """
        if zipfile.is_zipfile(db_root.resolve()):
            with zipfile.ZipFile(db_root) as archive:
                dir_list = [i.filename for i in archive.infolist() if i.is_dir()]
                root_directory_name = min(dir_list, key=len)
                root_directory = zipfile.Path(archive, root_directory_name)
                return self._iterate_root_directory(root_directory)
        elif db_root.suffix == ".zip":
            recho("Provided .zip file is not valid.")
            sys.exit(1)
        else:
            return self._iterate_root_directory(db_root)

    def _iterate_root_directory(
        self, root_directory: zipfile.Path | Path
    ) -> list[Cell]:
        """Iterate over the database root and parse each cell directory.

        Args:
            root_directory: Root directory containing one subdirectory per cell.

        Returns:
            List of all parsed cell objects.

        Raises:
            SystemExit: If the root is not a directory or required files are
                missing.
        """
        cells = []
        if not root_directory.is_dir():
            recho(f"Database root '{root_directory}' is not a directory!")
            sys.exit(1)
        for directory in root_directory.iterdir():
            FoxDB._check_cell_directory(directory)
            cells.append(self._get_cell(directory))
        return cells

    def _get_cell(self, directory: zipfile.Path | Path) -> Cell:
        """Parse a single cell directory.

        Reads the cell specification and model parameter JSON files and builds
        the corresponding objects.

        Args:
            directory: Path to the cell directory (ZIP virtual path).

        Returns:
            The constructed cell object.

        Raises:
            SystemExit: If a parameter type/value is invalid or required files
                are missing.
        """
        cell_spec = directory / "cell_spec.json"
        cell_spec_dict = read_json(cell_spec, directory)
        models_def = directory / "model_parameter.json"
        models_dicts = read_json(models_def, directory)
        try:
            models = creates_models(models_dicts, directory)
            return Cell(spec=CellSpec(**cell_spec_dict), model_parameters=models)
        except TypeError as e:
            recho(
                f"In directory {directory} a parameter has the wrong type, "
                f"is missing or unexpected: '{e}'"
            )
            sys.exit(1)
        except ValueError as e:
            recho(f"In directory {directory} a parameter has the wrong value: '{e}'")
            sys.exit(1)

    def _check_db(self) -> None:
        """Validate that there are no duplicate cells.

        Checks for duplicate identifiers across all parsed cells.

        Raises:
            SystemExit
                If a duplicate cell is found.
        """
        cells = self.list_cells()
        if len(cells) != len(set(cells)):
            recho("Database not valid, because at least one cell duplicate.")
            sys.exit(1)

    @staticmethod
    def _check_cell_directory(directory: zipfile.Path | Path) -> None:
        """Check that a cell directory contains all required files.

        Required entries are:
        - sources (directory)
        - cell_spec.json
        - model_parameter.json
        - cell_datasheet.pdf
        - README.md

        Args:
            directory: Path to the cell's directory.

        Raises:
            SystemExit: If the directory is not a directory or any required file
                is missing.
        """
        if not directory.is_dir():
            recho("Files in database root found. Invalid database format!")
            sys.exit(1)
        files = list(directory.iterdir())
        for x in [
            "sources",
            "cell_spec.json",
            "model_parameter.json",
            "cell_datasheet.pdf",
            "README.md",
        ]:
            if directory / x not in files:
                recho(f"Cell directory {directory} is missing '{x}'.")
                sys.exit(1)
