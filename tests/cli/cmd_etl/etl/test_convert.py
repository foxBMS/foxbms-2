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

"""Testing file 'cli/cmd_etl/etl/convert.py'."""


# test_convert.py

import sys
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest.mock import MagicMock, mock_open, patch

import pandas as pd
from pandas.testing import assert_frame_equal

try:
    from cli.cmd_etl.etl import OutputFormats
    from cli.cmd_etl.etl.convert import (
        ConversionSettings,
        Converter,
        InputFormats,
    )
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_etl.etl import OutputFormats
    from cli.cmd_etl.etl.convert import (
        ConversionSettings,
        Converter,
        InputFormats,
    )


class TestConverterInit(unittest.TestCase):
    """Tests for Converter.__init__."""

    def test_init_sets_attributes(self):
        """Ensure __init__ correctly assigns provided arguments to instance attributes."""
        settings = ConversionSettings(
            input_format=InputFormats.GAMRY,
            output_format=OutputFormats.CSV,
            additional={"skip_footer": 0},
        )
        data_path = Path("/tmp/data")  # noqa: S108
        recursive = True

        conv = Converter(
            data_path=data_path, recursive=recursive, conversion_settings=settings
        )

        self.assertEqual(conv._data_path, data_path)  # pylint:disable=protected-access
        self.assertTrue(conv._recursive)  # pylint:disable=protected-access
        self.assertEqual(conv._conversion_settings, settings)  # pylint:disable=protected-access


class TestConvert(unittest.TestCase):
    """Tests for Converter.convert"""

    def test_nonexistent_path_exits(self):
        """If the provided path does not exist, convert should exit with status 1."""
        bogus = Path("this_path_should_not_exist_12345")
        settings = ConversionSettings(
            input_format=InputFormats.GAMRY,
            output_format=OutputFormats.CSV,
            additional={"skip_footer": 0},
        )
        conv = Converter(bogus, recursive=False, conversion_settings=settings)
        with (
            self.assertRaises(SystemExit) as cm,
            patch("cli.cmd_etl.etl.convert.recho") as mock_recho,
        ):
            conv.convert()
        self.assertEqual(cm.exception.code, 1)
        mock_recho.assert_called_once_with("Provided data path does not exist!")

    def test_directory_dta_calls_read_and_write(self):
        """For DTA format, convert should discover .dta files, read them and call _write."""
        with TemporaryDirectory() as tmp:
            base = Path(tmp)
            f1 = base / "a.dta"
            f2 = base / "b.dta"
            f1.write_text("dummy")
            f2.write_text("dummy")

            settings = ConversionSettings(
                input_format=InputFormats.GAMRY,
                output_format=OutputFormats.CSV,
                additional={"skip_footer": 0},
            )
            conv = Converter(base, recursive=False, conversion_settings=settings)

            # Mock read_gamry to return a simple DataFrame; mock _write to track calls
            df = pd.DataFrame({"x": [1]})
            with (
                patch.object(Converter, "read_gamry", return_value=df),
                patch.object(Converter, "_write") as mock_write,
                patch("cli.cmd_etl.etl.convert.secho"),
            ):
                conv.convert()
                # _write should be called for each discovered file
            self.assertEqual(mock_write.call_count, 2)
            called_paths = [args[0][1] for args in mock_write.call_args_list]
            self.assertIn(f1, called_paths)
            self.assertIn(f2, called_paths)

    def test_directory_dta_is_none(self):
        """Test convert dta with empty dataframe."""
        with TemporaryDirectory() as tmp:
            base = Path(tmp)
            f1 = base / "a.dta"
            f1.write_text("dummy")
            settings = ConversionSettings(
                input_format=InputFormats.GAMRY,
                output_format=OutputFormats.CSV,
                additional={"skip_footer": 0},
            )
            conv = Converter(base, recursive=False, conversion_settings=settings)
            with (
                patch.object(Converter, "read_gamry", return_value=None),
                patch.object(Converter, "_write") as mock_write,
            ):
                conv.convert()
                # _write should be called for each discovered file
            mock_write.assert_not_called()

    def test_directory_graphtec_calls_read_and_write_with_renamed_target(self):
        """For Graphtec format, convert should discover .csv and use renamed
        target base before writing.
        """
        with TemporaryDirectory() as tmp:
            base = Path(tmp)
            f1 = base / "g1.csv"
            f2 = base / "g2.csv"
            f1.write_text("dummy")
            f2.write_text("dummy")

            settings = ConversionSettings(
                input_format=InputFormats.GRAPHTEC,
                output_format=OutputFormats.CSV,
                additional={"skip": 0},
            )
            conv = Converter(base, recursive=False, conversion_settings=settings)

            df = pd.DataFrame({"x": [1]})
            with (
                patch.object(Converter, "read_graphtec", return_value=df),
                patch.object(Converter, "_write") as mock_write,
                patch("cli.cmd_etl.etl.convert.secho"),
            ):
                conv.convert()
                # Target base name is stem + '_converted' + data_suffix (without dot)
                called_paths = [args[0][1] for args in mock_write.call_args_list]
                expected1 = base / "g1_converted"
                expected2 = base / "g2_converted"
            self.assertIn(expected1, called_paths)
            self.assertIn(expected2, called_paths)

    def test_directory_graphtec_is_none(self):
        """Test convert graphtec with empty dataframe."""
        with TemporaryDirectory() as tmp:
            base = Path(tmp)
            f1 = base / "a.csv"
            f1.write_text("dummy")
            settings = ConversionSettings(
                input_format=InputFormats.GRAPHTEC,
                output_format=OutputFormats.CSV,
                additional={"skip": 0},
            )
            conv = Converter(base, recursive=False, conversion_settings=settings)
            with (
                patch.object(Converter, "read_graphtec", return_value=None),
                patch.object(Converter, "_write") as mock_write,
            ):
                conv.convert()
                # _write should be called for each discovered file
            mock_write.assert_not_called()

    def test_unkown_format(self):
        """Test convert with unkown Inputformat"""
        with TemporaryDirectory() as tmp:
            base = Path(tmp)
            f1 = base / "a.csv"
            f1.write_text("dummy")
            mock_input_format = MagicMock(name="UNKOWN")
            settings = ConversionSettings(
                input_format=mock_input_format,
                output_format=OutputFormats.CSV,
                additional={"skip_footer": 0},
            )
            conv = Converter(base, recursive=False, conversion_settings=settings)
            with (
                self.assertRaises(SystemExit) as cm,
                patch("cli.cmd_etl.etl.convert.recho") as mock_recho,
            ):
                conv.convert()
                # _write should be called for each discovered file
            self.assertEqual(cm.exception.code, 1)
            mock_recho.assert_called_once()

    def test_permission_error(self):
        """For DTA format, convert should discover .dta files, read them and call _write."""
        with TemporaryDirectory() as tmp:
            base = Path(tmp)
            f1 = base / "a.csv"
            f1.write_text("dummy")
            settings = ConversionSettings(
                input_format=InputFormats.GRAPHTEC,
                output_format=OutputFormats.CSV,
                additional={"skip": 0},
            )
            conv = Converter(base, recursive=False, conversion_settings=settings)
            with (
                self.assertRaises(SystemExit) as cm,
                patch.object(Converter, "read_graphtec", side_effect=PermissionError),
                patch("cli.cmd_etl.etl.convert.recho") as mock_recho,
            ):
                conv.convert()
                # _write should be called for each discovered file
            self.assertEqual(cm.exception.code, 1)
            mock_recho.assert_called_once()


class TestGetDataFiles(unittest.TestCase):
    """Tests for Converter._get_data_files"""

    def setUp(self):
        """Minimal settings needed for instantiation"""
        self.settings = ConversionSettings(
            input_format=InputFormats.GAMRY,
            output_format=OutputFormats.CSV,
            additional={"skip_footer": 0},
        )

    def test_find_single_file(self):
        """Ensure a single file is handle correctly."""
        with TemporaryDirectory() as tmp:
            base = Path(tmp)
            file_path = base / "a.dta"
            file_path.write_text("dummy")

            conv = Converter(
                file_path, recursive=False, conversion_settings=self.settings
            )
            files = conv._get_data_files("dta")  # pylint: disable=protected-access
            self.assertIn(file_path, files)

    def test_non_recursive_finds_only_top_level(self):
        """Ensure non-recursive search returns only files in the top directory."""
        with TemporaryDirectory() as tmp:
            base = Path(tmp)
            # Create files
            (base / "a.dta").write_text("dummy")
            (base / "b.dta").write_text("dummy")
            (base / "c.csv").write_text("dummy")
            # Create subdir with matching files
            sub = base / "sub"
            sub.mkdir()
            (sub / "d.dta").write_text("dummy")

            conv = Converter(base, recursive=False, conversion_settings=self.settings)
            files = conv._get_data_files("dta")  # pylint: disable=protected-access
            self.assertEqual({p.name for p in files}, {"a.dta", "b.dta"})

    def test_recursive_finds_nested(self):
        """Ensure recursive search returns files from all subdirectories."""
        with TemporaryDirectory() as tmp:
            base = Path(tmp)
            (base / "a.dta").write_text("dummy")
            sub = base / "sub"
            sub.mkdir()
            (sub / "b.dta").write_text("dummy")
            deeper = sub / "deeper"
            deeper.mkdir()
            (deeper / "c.dta").write_text("dummy")

            conv = Converter(base, recursive=True, conversion_settings=self.settings)
            files = conv._get_data_files("dta")  # pylint: disable=protected-access
            self.assertEqual({p.name for p in files}, {"a.dta", "b.dta", "c.dta"})


class TestWrite(unittest.TestCase):
    """Tests for Converter._write"""

    def test_write_csv_creates_file(self):
        """Writing CSV should produce a .csv file next to target base path."""
        with TemporaryDirectory() as tmp:
            base = Path(tmp)
            df = pd.DataFrame({"a": [1], "b": [2]})
            new_base = base / "out"

            settings = ConversionSettings(
                input_format=InputFormats.GAMRY,
                output_format=OutputFormats.CSV,
                additional={"skip_footer": 0},
            )
            conv = Converter(base, recursive=False, conversion_settings=settings)
            conv._write(df, new_base)  # pylint: disable=protected-access
            self.assertTrue((base / "out.csv").exists())

    def test_write_parquet_uses_to_parquet(self):
        """Writing Parquet should call DataFrame.to_parquet with a .parquet suffix."""
        with TemporaryDirectory() as tmp:
            base = Path(tmp)
            df = pd.DataFrame({"a": [1], "b": [2]})
            new_base = base / "out"

            settings = ConversionSettings(
                input_format=InputFormats.GAMRY,
                output_format=OutputFormats.PARQUET,
                additional={"skip_footer": 0},
            )
            conv = Converter(base, recursive=False, conversion_settings=settings)

            # Mock to_parquet to avoid dependency on pyarrow/fastparquet
            with patch.object(
                pd.DataFrame, "to_parquet", autospec=True
            ) as mock_parquet:
                conv._write(df, new_base)  # pylint: disable=protected-access
                mock_parquet.assert_called_once()
                # First arg to bound method is df itself; path is second arg
                called_path = mock_parquet.call_args[0][1]
                self.assertTrue(str(called_path).endswith(".parquet"))

    def test_write_unknown_output_format_exits(self):
        """Unknown output format should print an error and exit with status 1."""
        with TemporaryDirectory() as tmp:
            base = Path(tmp)
            df = pd.DataFrame({"a": [1]})
            new_base = base / "out"

            # Use a bogus output format that won't match CSV/PARQUET
            bogus_output = "BOGUS"
            settings = ConversionSettings(
                input_format=InputFormats.GAMRY,
                output_format=bogus_output,
                additional={"skip_footer": 0},
            )
            conv = Converter(base, recursive=False, conversion_settings=settings)

            with (
                self.assertRaises(SystemExit) as cm,
                patch("cli.cmd_etl.etl.convert.recho") as mock_recho,
            ):
                conv._write(df, new_base)  # pylint: disable=protected-access
            mock_recho.assert_called_once_with("Output format 'BOGUS' not known.")
            self.assertEqual(cm.exception.code, 1)


class TestConverterReadGamry(unittest.TestCase):
    """Tests for Converter.read_gamry."""

    def setUp(self):
        """Prepare default settings with skip_footer=0 for read_gamry tests."""
        self.settings = ConversionSettings(
            input_format=InputFormats.GAMRY,
            output_format=OutputFormats.CSV,
            additional={"skip_footer": 0},
        )

    def test_returns_none_when_no_table_line(self):
        """Return None and log a message if the 'TABLE' marker cannot be found."""
        conv = Converter(Path("dummy"), False, self.settings)
        m = mock_open(read_data="HEADER\nNO_table_HERE\n")
        with (
            patch("cli.cmd_etl.etl.convert.open", m),
            patch("cli.cmd_etl.etl.convert.secho") as mock_secho,
        ):
            result = conv.read_gamry(Path("file.dta"))

            self.assertIsNone(result)
            self.assertTrue(mock_secho.called)
            self.assertIn(
                "header offset could not be found", mock_secho.call_args[0][0]
            )

    def test_reads_with_skip_footer_and_drops_columns(self):
        """Read with non-zero skip_footer, then drop the second header level
        and the first column.
        """
        self.settings.additional = {"skip_footer": 5}
        conv = Converter(Path("dummy"), False, self.settings)

        file_content = "Line0\nLine1\nTABLE\nDataStart\n"
        m = mock_open(read_data=file_content)

        arrays = [
            ["Pt", "Col1", "Col2"],
            ["", "u1", "u2"],
        ]
        tuples = list(zip(*arrays, strict=True))
        multi_cols = pd.MultiIndex.from_tuples(tuples)
        df_in = pd.DataFrame([[0, 10, 20], [1, 11, 21]], columns=multi_cols)

        expected = df_in.droplevel(1, axis=1).iloc[:, 1:]

        with (
            patch("cli.cmd_etl.etl.convert.open", m),
            patch(
                "cli.cmd_etl.etl.convert.pd.read_csv", return_value=df_in
            ) as mock_read_csv,
        ):
            result = conv.read_gamry(Path("file.dta"))

            mock_read_csv.assert_called_once()
            _, kwargs = mock_read_csv.call_args
            self.assertEqual(kwargs["sep"], "\t")
            self.assertEqual(kwargs["skiprows"], 3)
            self.assertEqual(kwargs["encoding"], "ANSI")
            self.assertEqual(kwargs["header"], [0, 1])
            self.assertEqual(kwargs["decimal"], ",")
            self.assertEqual(kwargs["skipfooter"], 5)
            self.assertEqual(kwargs["engine"], "python")

            assert_frame_equal(result, expected)

    def test_reads_with_skip_footer_zero(self):
        """Ensure explicit skip_footer=0 is passed through to pandas.read_csv."""
        conv = Converter(Path("dummy"), False, self.settings)
        file_content = "A\nB\nTABLE\nX\n"
        m = mock_open(read_data=file_content)

        arrays = [["Pt", "A"], ["", ""]]
        tuples = list(zip(*arrays, strict=True))
        multi_cols = pd.MultiIndex.from_tuples(tuples)
        df_in = pd.DataFrame([[0, 1]], columns=multi_cols)

        with (
            patch("cli.cmd_etl.etl.convert.open", m),
            patch(
                "cli.cmd_etl.etl.convert.pd.read_csv", return_value=df_in
            ) as mock_read_csv,
        ):
            _ = conv.read_gamry(Path("file.dta"))
            self.assertEqual(mock_read_csv.call_args.kwargs["skipfooter"], 0)


class TestConverterReadGraphtec(unittest.TestCase):
    """Tests for Converter.read_graphtec."""

    def setUp(self):
        """Setup conversion settings"""
        self.settings = ConversionSettings(
            input_format=InputFormats.GRAPHTEC,
            output_format=OutputFormats.CSV,
            additional={"skip": 32},
        )

    def _write_graphtec_csv(self, path: Path, with_ms: bool = True):
        preamble = "\n".join([f"# header line {i}" for i in range(32)])
        if with_ms:
            header1 = "Index,Date&Time,ms,Ch1,Ch2"
            header2 = ",,,Volt,Volt"
            data = "0,2023-01-02 03:04:05,123,1.1,2.2"
        else:
            header1 = "Index,Date&Time,Ch1"
            header2 = ",,Volt"
            data = "0,2023-01-02 03:04:05,1.1"
        content = f"{preamble}\n{header1}\n{header2}\n{data}\n"
        path.write_text(content, encoding="utf-8")

    def test_read_graphtec_happy_path(self):
        """Read graphtec .csv and convert the file."""
        with TemporaryDirectory() as tmpdir:
            file_path = Path(tmpdir) / "graphtec.csv"
            self._write_graphtec_csv(file_path, with_ms=True)
            conv = Converter(file_path, False, self.settings)
            df = conv.read_graphtec(file_path)
            self.assertListEqual(df.columns.tolist(), ["Date&Time", "Ch1", "Ch2"])
            self.assertTrue(pd.api.types.is_datetime64_ns_dtype(df["Date&Time"].dtype))
            expected_ts = pd.Timestamp("2023-01-02 03:04:05") + pd.to_timedelta(
                123, unit="ms"
            )
            self.assertEqual(df.loc[df.index[0], "Date&Time"], expected_ts)
            self.assertAlmostEqual(df.loc[df.index[0], "Ch1"], 1.1)
            self.assertAlmostEqual(df.loc[df.index[0], "Ch2"], 2.2)

    def test_read_graphtec_key_error(self):
        """Read graphtec .csv and convert the file."""
        with TemporaryDirectory() as tmpdir:
            file_path = Path(tmpdir) / "graphtec.csv"
            self._write_graphtec_csv(file_path, with_ms=False)
            conv = Converter(file_path, False, self.settings)
            with patch("cli.cmd_etl.etl.convert.recho") as mock_recho:
                result = conv.read_graphtec(file_path)
            self.assertIsNone(result)
            mock_recho.assert_called_once()

    def test_read_graphtec_without_skip(self):
        """Read graphtec .csv and convert the file."""
        with TemporaryDirectory() as tmpdir:
            file_path = Path(tmpdir) / "graphtec.csv"
            self._write_graphtec_csv(file_path, with_ms=True)
            self.settings.additional["skip"] = None
            conv = Converter(file_path, False, self.settings)
            df = conv.read_graphtec(file_path)
            self.assertListEqual(df.columns.tolist(), ["Date&Time", "Ch1", "Ch2"])


if __name__ == "__main__":
    unittest.main()
