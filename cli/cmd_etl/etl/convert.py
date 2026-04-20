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

# cspell:ignore skipfooter

"""Convert files of a specific format to .csv or .parquet format."""

import sys
from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path

import pandas as pd

from ...helpers.click_helpers import recho, secho
from ...helpers.logger import logger
from . import OutputFormats


class InputFormats(Enum):
    """Definition of all supported input formats."""

    GAMRY = 0
    GRAPHTEC = 1


@dataclass
class ConversionSettings:
    """Container for all conversion-related settings.

    Attributes:
        input_format: The expected source file format (e.g., InputFormats.GAMRY).
        output_format: The desired target format (CSV or PARQUET).
        additional: Optional additional parameters understood by specific readers.
            For Gamry DTA reading, the following key is supported:

            - "skip_footer": int number of lines to skip at the end of the file.

    """

    input_format: InputFormats
    output_format: OutputFormats
    additional: dict = field(default_factory=dict)


class Converter:
    """High-level converter that discovers files and transforms them.

    The converter accepts either a single file path or a directory. When a directory
    is provided, it can optionally traverse subdirectories to find files that match
    the configured input format. Each discovered file is read and written in the
    configured output format. Errors (e.g., missing paths, permission issues) are
    reported and cause the process to exit with a non-zero status.
    """

    def __init__(
        self, data_path: Path, recursive: bool, conversion_settings: ConversionSettings
    ) -> None:
        """Initialize a new Converter.

        Args:
            data_path: Path to a single file to convert or a directory to scan.
            recursive: If True and data_path is a directory, search subdirectories
                for matching files.
            conversion_settings: Configuration specifying input/output formats and
                reader-specific options.
        """
        self._data_path = data_path
        self._recursive = recursive
        self._conversion_settings = conversion_settings
        logger.info("Initialize new Converter")
        logger.debug("data_path %s", data_path)
        logger.debug("recursive %s", recursive)
        logger.debug(conversion_settings)

    def convert(self) -> None:
        """Discover and convert all matching files.

        Behavior:

            - Verifies that the provided data path exists.
            - If it's a file, converts exactly that file.

            - If it's a directory, collects all files with the expected input suffix.

              When 'recursive' is True, includes subdirectories.

            - For each discovered file, delegates to '_convert'.

        Exits:

            - Prints an error and exits with status 1 if the path does not exist

              or an OS-related error occurs (including permission issues).
        """
        try:
            if not self._data_path.exists():
                recho("Provided data path does not exist!")
                sys.exit(1)
            data_suffix = self._conversion_settings.input_format.name.lower()
            match self._conversion_settings.input_format:
                case InputFormats.GAMRY:
                    data_suffix = "dta"
                    data_files = self._get_data_files(data_suffix)
                    for data_file in data_files:
                        new_file_path = data_file
                        df = self.read_gamry(data_file)
                        if df is not None:
                            self._write(df, new_file_path)
                            secho(f"Converted file: {data_file}")
                case InputFormats.GRAPHTEC:
                    data_suffix = "csv"
                    data_files = self._get_data_files(data_suffix)
                    for data_file in data_files:
                        new_name = f"{data_file.stem}_converted"
                        new_file_path = data_file.parent / new_name
                        df = self.read_graphtec(data_file)
                        if df is not None:
                            self._write(df, new_file_path)
                            secho(f"Converted file: {data_file}")
                case _:
                    recho(
                        f"Input format '{self._conversion_settings.input_format}' not known."
                    )
                    sys.exit(1)
        except OSError as e:
            # OSError also handles permission errors
            recho(str(e))
            sys.exit(1)

    def _get_data_files(self, data_suffix: str) -> list[Path]:
        """Looks for the desired data files

        Args:
            data_suffix: File type considered for the search.
        """
        if self._data_path.is_file():
            return [self._data_path]
        if self._recursive:
            return list(self._data_path.glob(f"**/*.{data_suffix}"))
        return list(self._data_path.glob(f"*.{data_suffix}"))

    def _write(self, df: pd.DataFrame, new_file_path: Path) -> None:
        """Write file to the desired format.

        This method routes the given file to the appropriate reader according to
        the selected input format and writes the result using the selected output
        format.

        Args:
            df: Dataframe to write.
            new_file_path: Path to file that should be written.

        Exits:

            - Prints an error and exits with status 1 if the input format is unknown.

        Side effects:

            - Writes the converted file next to the source using the appropriate

              extension (.csv or .parquet).

            - Logs a success message after writing.

        """
        match self._conversion_settings.output_format:
            case OutputFormats.CSV:
                df.to_csv(new_file_path.with_suffix(".csv"), index=False)
            case OutputFormats.PARQUET:
                df.to_parquet(new_file_path.with_suffix(".parquet"), index=False)
            case _:
                recho(
                    f"Output format '{self._conversion_settings.output_format}' not known."
                )
                sys.exit(1)

    def read_gamry(self, file_path: Path) -> pd.DataFrame | None:
        """Read a Gamry .DTA file and return its content as a DataFrame.

        The method detects the start of the tabular section by scanning for the line
        containing the literal 'TABLE'. All lines before (and including) that line
        are skipped. The data is then parsed via pandas.read_csv with:

            - Tab as field separator.
            - Two-row header (MultiIndex), where the second header row is dropped.

            - Comma as decimal separator.
            - Optional 'skip_footer' lines ignored at the end (if provided).

        The resulting DataFrame is post-processed by:

            - Dropping the second header level.
            - Dropping the first column 'Pt', which is considered an index/counter.

        Args:
            file_path: Path to the .dta file.

        Returns:
            A pandas DataFrame with cleaned columns and without the 'Pt' column.
            Returns None if the 'TABLE' marker cannot be found.

        Notes:
            'skip_footer' is read from 'conversion_settings.additional'. If not set or
            falsy, it defaults to 0.
        """
        header_offset = None
        with open(file_path, encoding="ANSI") as f:
            for i, line in enumerate(f):
                # The header of a .dta file starts after the word TABLE
                if "TABLE" in line:
                    header_offset = i + 1
        if header_offset is None:
            secho(f"Skipped {file_path} because header offset could not be found.")
            return None
        if self._conversion_settings.additional["skip_footer"]:
            skip_footer = self._conversion_settings.additional["skip_footer"]
        else:
            skip_footer = 0
        df = pd.read_csv(
            file_path,
            sep="\t",
            skiprows=header_offset,
            encoding="ANSI",
            header=[0, 1],
            decimal=",",
            skipfooter=skip_footer,
            engine="python",
        )
        # removes second line of the header and the first column (unnamed)
        return df.droplevel(1, axis=1).iloc[:, 1:]

    def read_graphtec(self, file_path: Path) -> pd.DataFrame | None:
        """Read a Graphtec CSV export and return a cleaned DataFrame.

        The reader expects a CSV with a two-row column header and a 32-line
        preamble. It flattens the header, removes the leading non-data/index
        column, combines the "Date&Time" and "ms" columns into a single
        pandas datetime, and drops the original "ms" column.

        Parameters
        ----------
        file_path : Path
            Path to the Graphtec CSV file.

        Returns
        -------
        pd.DataFrame or None
            DataFrame where:

            - "Date&Time" is datetime64[ns] (date/time plus milliseconds).
            - Measurement channels remain as subsequent columns.

            - The "ms" column is removed.

            Returns None if upstream logic intends to signal no data.

        Raises
        ------
        FileNotFoundError
            If the file does not exist.
        pandas.errors.ParserError
            If the CSV cannot be parsed.
        KeyError
            If required columns ("Date&Time", "ms") are missing.

        Notes
        -----
        - The header offset is fixed at 32 lines; adjust if your export differs.
        - Columns are assumed to include "Date&Time" and "ms".

        """
        try:
            if self._conversion_settings.additional["skip"] is not None:
                skip = self._conversion_settings.additional["skip"]
            else:
                skip = 32
            df = pd.read_csv(
                file_path,
                skiprows=skip,
                header=[0, 1],
            )
            df = df.droplevel(1, axis=1).iloc[:, 1:]
            df["Date&Time"] = pd.to_datetime(df["Date&Time"]) + pd.to_timedelta(
                df["ms"], unit="ms"
            )
            return df.drop(["ms"], axis=1)
        except KeyError as e:
            recho(
                f"Needed column in '{file_path}' not found: {e}. Skip parameter wrong ?"
            )
            return None
