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

"""Testing file 'cli/pre_commit_scripts/sort_test_config_lists.py'."""

import io
import json
import sys
import tempfile
import unittest
from contextlib import redirect_stderr
from pathlib import Path

try:
    from cli.pre_commit_scripts import sort_test_config_lists
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.pre_commit_scripts import sort_test_config_lists


class TestSortTestConfigLists(unittest.TestCase):
    """Tests for test configuration list sorting validation."""

    def _write_json_temp_file(self, content: object) -> Path:
        """Write a JSON document to a temporary file and return its path."""
        with tempfile.NamedTemporaryFile(
            mode="w",
            suffix=".json",
            delete=False,
            encoding="utf-8",
        ) as temp_file:
            json.dump(content, temp_file, indent=2)
            temp_file.write("\n")
            return Path(temp_file.name)

    def test_sorted_file(self) -> None:
        """No error for sorted lists."""
        test_file = self._write_json_temp_file(
            [
                {
                    "includes": ["a.h", "b.h", "c.h"],
                    "mocks": ["a_mock.h", "b_mock.h"],
                    "source": ["a.c", "b.c"],
                }
            ]
        )
        self.addCleanup(test_file.unlink)

        result = sort_test_config_lists.main([str(test_file)])
        self.assertEqual(result, 0)

    def test_unsorted_file(self) -> None:
        """Error for unsorted lists, including nested paths."""
        test_file = self._write_json_temp_file(
            [
                {"mocks": ["b_mock.h", "a_mock.h"]},
                {"nested": {"includes": ["x.h", "a.h"]}},
            ]
        )
        self.addCleanup(test_file.unlink)

        err = io.StringIO()
        with redirect_stderr(err):
            result = sort_test_config_lists.main([str(test_file)])

        self.assertEqual(result, 2)
        self.assertIn(
            f"{test_file.as_posix()}: list at [0].mocks is not sorted.\n",
            err.getvalue(),
        )
        self.assertIn(
            f"{test_file.as_posix()}: list at [1].nested.includes is not sorted.\n",
            err.getvalue(),
        )

    def test_only_target_keys_are_checked(self) -> None:
        """Only source/includes/mocks lists are validated."""
        test_file = self._write_json_temp_file(
            [
                {
                    "source": ["z.c", "a.c"],
                    "numbers": [3, 1, 2],
                    "objects": [{"a": 2}, {"a": 1}],
                }
            ]
        )
        self.addCleanup(test_file.unlink)

        err = io.StringIO()
        with redirect_stderr(err):
            result = sort_test_config_lists.main([str(test_file)])

        self.assertEqual(result, 1)
        self.assertIn(
            f"{test_file.as_posix()}: list at [0].source is not sorted.\n",
            err.getvalue(),
        )
        self.assertNotIn("numbers", err.getvalue())
        self.assertNotIn("objects", err.getvalue())

    def test_invalid_json(self) -> None:
        """Invalid JSON in a temporary file reports an error."""
        with tempfile.TemporaryDirectory() as tmp:
            test_file = Path(tmp) / "invalid.json"
            test_file.write_text(
                '[\n  {\n    "mocks": [\n      "a.h",\n      "b.h"\n    ]\n  }\n',
                encoding="utf-8",
            )

            err = io.StringIO()
            with redirect_stderr(err):
                result = sort_test_config_lists.main([str(test_file)])

        self.assertEqual(result, 1)
        self.assertIn(f"{test_file.as_posix()}: invalid JSON:", err.getvalue())

    def test_main_without_files(self) -> None:
        """No files should return no errors."""
        result = sort_test_config_lists.main([])
        self.assertEqual(result, 0)

    def test_error_count_is_capped_to_255(self) -> None:
        """More than 255 findings are capped as documented."""
        entries = [{"source": ["b.c", "a.c"]} for _ in range(300)]
        test_file = self._write_json_temp_file(entries)
        self.addCleanup(test_file.unlink)

        err = io.StringIO()
        with redirect_stderr(err):
            result = sort_test_config_lists.main([str(test_file)])

        self.assertEqual(result, 255)

    def test_top_level_list_message_path(self) -> None:
        """Top-level list message formatting is covered explicitly."""
        with tempfile.TemporaryDirectory() as tmp:
            test_file = Path(tmp) / "test.json"
            err = io.StringIO()
            with redirect_stderr(err):
                # pylint: disable-next=protected-access
                result = sort_test_config_lists._check_sorted_lists(
                    ["b", "a"],
                    "$",
                    test_file,
                    current_key="source",
                )

        self.assertEqual(result, 1)
        self.assertIn(
            f"{test_file.as_posix()}: top-level list is not sorted.\n",
            err.getvalue(),
        )

    def test_format_location_branches(self) -> None:
        """All _format_location branches are covered explicitly."""
        # pylint: disable-next=protected-access
        self.assertEqual(sort_test_config_lists._format_location("$"), "top-level list")
        # pylint: disable-next=protected-access
        self.assertEqual(sort_test_config_lists._format_location("$.mocks"), "mocks")

        self.assertEqual(  # pylint: disable-next=protected-access
            sort_test_config_lists._format_location("$[0].mocks"), "[0].mocks"
        )
        # pylint: disable-next=protected-access
        self.assertEqual(sort_test_config_lists._format_location("mocks"), "mocks")

    def test_fix_rewrites_unsorted_lists_and_returns_one(self) -> None:
        """--fix sorts target lists, rewrites the file, and returns one."""
        test_file = self._write_json_temp_file(
            [
                {
                    "source": ["z.c", "a.c"],
                    "includes": ["z.h", "a.h"],
                    "mocks": ["b_mock.h", "a_mock.h"],
                    "unchanged": [2, 1],
                },
                {"nested": {"includes": ["y.h", "x.h"]}},
            ]
        )
        self.addCleanup(test_file.unlink)

        err = io.StringIO()
        with redirect_stderr(err):
            result = sort_test_config_lists.main(["--fix", str(test_file)])

        # There are multiple changed lists, but a fixing hook must return
        # exactly 1 so pre-commit stops and the user can stage the changes.
        self.assertEqual(result, 1)

        self.assertEqual(
            json.loads(test_file.read_text(encoding="utf-8")),
            [
                {
                    "source": ["a.c", "z.c"],
                    "includes": ["a.h", "z.h"],
                    "mocks": ["a_mock.h", "b_mock.h"],
                    "unchanged": [2, 1],
                },
                {"nested": {"includes": ["x.h", "y.h"]}},
            ],
        )

    def test_fix_sorted_file_returns_zero_without_rewriting(self) -> None:
        """--fix leaves an already sorted file unchanged."""
        with tempfile.TemporaryDirectory() as tmp:
            test_file = Path(tmp) / "sorted.json"

            # Intentionally compact formatting proves that a no-op fix does
            # not rewrite the file just to reformat it.
            original = '[{"source":["a.c","b.c"]}]\n'
            test_file.write_text(original, encoding="utf-8")

            err = io.StringIO()
            with redirect_stderr(err):
                result = sort_test_config_lists.main(["--fix", str(test_file)])

            self.assertEqual(result, 0)
            self.assertEqual(err.getvalue(), "")
            self.assertEqual(test_file.read_text(encoding="utf-8"), original)

    def test_fix_does_not_overwrite_invalid_json(self) -> None:
        """--fix reports invalid JSON without modifying the input file."""
        with tempfile.TemporaryDirectory() as tmp:
            test_file = Path(tmp) / "invalid.json"
            original = '{"source": ["b.c", "a.c",]}\n'
            test_file.write_text(original, encoding="utf-8")

            err = io.StringIO()
            with redirect_stderr(err):
                result = sort_test_config_lists.main(["--fix", str(test_file)])

            self.assertEqual(result, 1)
            self.assertEqual(test_file.read_text(encoding="utf-8"), original)
            self.assertIn(
                f"{test_file.as_posix()}: invalid JSON:",
                err.getvalue(),
            )


if __name__ == "__main__":
    unittest.main()
