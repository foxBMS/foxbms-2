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

"""Testing file 'cli/pre_commit_scripts/check_license_info.py'."""

# pylint: disable=protected-access

import io
import sys
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.pre_commit_scripts import check_license_info
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.pre_commit_scripts import check_license_info

TYPE_TO_EXT = {
    "asm": "asm",
    "c": "c",
    "pwsh": "ps1",
    "py": "py",
    "shell": "sh",
    "toml": "toml",
    "yaml": "yml",
}


class TestCheckLicenseInfo(unittest.TestCase):
    """Test of the main function"""

    @classmethod
    def setUpClass(cls):
        cls.tests_dir = Path(__file__).parent / Path(__file__).stem

    def test_valid_files(self):
        """Test with valid files"""
        for i in ["asm", "c", "pwsh", "py", "shell", "toml", "yaml"]:
            argv = [
                str(self.tests_dir / f"valid-license.{TYPE_TO_EXT[i]}"),
                "--license-type",
                "BSD-3-Clause",
            ]
            result = check_license_info.main(argv)
            self.assertEqual(result, 0)

    def test_invalid_license(self):
        """Test with an invalid file"""
        test = self.tests_dir / "invalid-license.c"
        argv = [
            str(test),
            "--license-type",
            "BSD-3-Clause",
        ]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_license_info.main(argv)
        self.assertEqual(result, 1)
        self.assertIn(
            "Line 6: Expected: ' * SPDX-License-Identifier: BSD-3-Clause'\n"
            "Line 6: Actual:   ' * some text'",
            buf.getvalue(),
        )

    def test_missing_license_information(self):
        """Test with an file that misses license information"""
        test = self.tests_dir / "no-license.c"
        argv = [
            str(test),
            "--license-type",
            "BSD-3-Clause",
        ]
        buf = io.StringIO()
        with redirect_stderr(buf):
            result = check_license_info.main(argv)
        self.assertEqual(result, 2)

    @staticmethod
    def _valid_c_content(license_type: check_license_info.LicenseTypes) -> str:
        prolog = [
            "/**",
            " *",
            # pylint: disable-next=line-too-long
            " * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
            " * All rights reserved.",
            " *",
        ]
        epilog = [" *", " */"]
        char = " * "
        txt = [
            (char + i.replace("®", "&reg;")).rstrip()
            for i in check_license_info.LICENSE_TYPE_TO_LICENSE_BASE_TEXT[license_type]
        ]
        return "\n".join(prolog + txt + epilog + ["int dummy = 0;"])

    @staticmethod
    def _valid_py_content(license_type: check_license_info.LicenseTypes) -> str:
        prolog = [
            "#!/usr/bin/env python3",
            "#",
            # pylint: disable-next=line-too-long
            "# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.",
            "# All rights reserved.",
            "#",
        ]
        char = "# "
        txt = [
            (char + i).rstrip()
            for i in check_license_info.LICENSE_TYPE_TO_LICENSE_BASE_TEXT[license_type]
        ]
        return "\n".join(prolog + txt + ["def build(ctx):", "    return None"])

    @patch("pathlib.Path.exists")
    @patch("pathlib.Path.read_text")
    def test_check_src_unit_test_license_valid(self, mock_read_text, mock_exists):
        """Check src+tests/unit license headers when option is enabled."""
        valid = self._valid_c_content("BSD-3-Clause")
        source = Path("src/app/main/main.c")
        mock_exists.return_value = True
        mock_read_text.side_effect = [valid, valid]

        result = check_license_info.check_c(
            files=[str(source)],
            license_type="BSD-3-Clause",
        )
        self.assertEqual(result, 0)

    @patch("pathlib.Path.exists")
    @patch("pathlib.Path.read_text")
    def test_check_src_unit_test_license_invalid(self, mock_read_text, mock_exists):
        """Fail when accompanying tests/unit C file has a different header."""
        valid = self._valid_c_content("BSD-3-Clause")
        invalid = valid.replace(
            "SPDX-License-Identifier: BSD-3-Clause", "SPDX-License-Identifier: some"
        )
        source = Path("src/app/main/main.c")
        mock_exists.return_value = True
        mock_read_text.side_effect = [valid, invalid]

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            result = check_license_info.check_c(
                files=[str(source)],
                license_type="BSD-3-Clause",
            )
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(result, 1)
        self.assertEqual(
            "tests/unit/app/main/test_main.c: License header is not correct.\n"
            "The following lines differ\n"
            "Line 6: Expected: ' * SPDX-License-Identifier: BSD-3-Clause'\n"
            "Line 6: Actual:   ' * SPDX-License-Identifier: some'\n",
            err,
        )
        self.assertEqual("", out)

    @patch("pathlib.Path.exists", return_value=False)
    @patch("pathlib.Path.read_text")
    def test_check_src_unit_test_license_missing_test_file(
        self, mock_read_text, _mock_exists
    ):
        """Do not fail if accompanying tests/unit file does not exist."""
        source = Path("src/app/main/main.c")
        valid = self._valid_c_content("BSD-3-Clause")
        mock_read_text.return_value = valid

        result = check_license_info.check_c(
            files=[str(source)],
            license_type="BSD-3-Clause",
        )
        self.assertEqual(result, 0)

    @patch("pathlib.Path.exists")
    @patch("pathlib.Path.read_text")
    def test_check_src_unit_test_license_non_src_file_ignored(
        self, mock_read_text, mock_exists
    ):
        """Option should not trigger companion lookup for non-src C files."""
        file = Path("tests/unit/app/main/test_main.c")
        valid = self._valid_c_content("BSD-3-Clause")
        mock_read_text.return_value = valid

        result = check_license_info.check_c(
            files=[str(file)],
            license_type="BSD-3-Clause",
        )
        self.assertEqual(result, 0)
        mock_exists.assert_not_called()

    @patch("pathlib.Path.exists")
    @patch("pathlib.Path.read_text")
    def test_check_src_unit_test_wscript_license_valid(
        self, mock_read_text, mock_exists
    ):
        """Check src+tests/unit wscript license headers when option is enabled."""
        valid = self._valid_py_content("BSD-3-Clause")
        source = Path("src/app/main/wscript")
        mock_exists.return_value = True
        mock_read_text.side_effect = [valid, valid]

        result = check_license_info.check_py(
            files=[str(source)],
            license_type="BSD-3-Clause",
        )
        self.assertEqual(result, 0)

    @patch("pathlib.Path.exists")
    @patch("pathlib.Path.read_text")
    def test_check_src_unit_test_wscript_license_invalid(
        self, mock_read_text, mock_exists
    ):
        """Fail when accompanying tests/unit wscript has a different header."""
        valid = self._valid_py_content("BSD-3-Clause")
        invalid = valid.replace(
            "SPDX-License-Identifier: BSD-3-Clause", "SPDX-License-Identifier: some"
        )
        source = Path("src/app/main/wscript")
        mock_exists.return_value = True
        mock_read_text.side_effect = [valid, invalid]

        _err, _out = io.StringIO(), io.StringIO()
        with redirect_stderr(_err), redirect_stdout(_out):
            result = check_license_info.check_py(
                files=[str(source)],
                license_type="BSD-3-Clause",
            )
        err, out = _err.getvalue(), _out.getvalue()
        self.assertEqual(result, 1)
        self.assertEqual(
            "tests/unit/app/main/wscript: License header is not correct.\n"
            "The following lines differ\n"
            "Line 6: Expected: '# SPDX-License-Identifier: BSD-3-Clause'\n"
            "Line 6: Actual:   '# SPDX-License-Identifier: some'\n",
            err,
        )
        self.assertEqual("", out)

    @patch("pathlib.Path.exists", return_value=False)
    @patch("pathlib.Path.read_text")
    def test_check_src_unit_test_wscript_license_missing_test_file(
        self, mock_read_text, _mock_exists
    ):
        """Do not fail if accompanying tests/unit wscript does not exist."""
        source = Path("src/app/main/wscript")
        valid = self._valid_py_content("BSD-3-Clause")
        mock_read_text.return_value = valid

        result = check_license_info.check_py(
            files=[str(source)],
            license_type="BSD-3-Clause",
        )
        self.assertEqual(result, 0)

    @patch("pathlib.Path.exists")
    @patch("pathlib.Path.read_text")
    def test_check_src_unit_test_wscript_non_src_file_ignored(
        self, mock_read_text, mock_exists
    ):
        """Companion lookup should not trigger for non-src wscript files."""
        file = Path("tests/unit/app/main/wscript")
        valid = self._valid_py_content("BSD-3-Clause")
        mock_read_text.return_value = valid

        result = check_license_info.check_py(
            files=[str(file)],
            license_type="BSD-3-Clause",
        )
        self.assertEqual(result, 0)
        mock_exists.assert_not_called()


class TestCheckLicenseInfoHelpers(unittest.TestCase):
    """Tests for helper branches in check_license_info."""

    @patch("pathlib.Path.read_text", return_value="line-1\nline-2")
    def test_read_text_utf8_lines_success(self, _mock_read_text):
        """UTF-8 helper should return split lines on success."""
        ret = check_license_info._read_text_utf8_lines(Path("some/file.txt"))
        self.assertEqual(ret, ["line-1", "line-2"])

    @patch(
        "pathlib.Path.read_text",
        side_effect=UnicodeDecodeError("utf-8", b"\xff", 0, 1, "invalid start byte"),
    )
    def test_read_text_utf8_lines_unicode_error(self, _mock_read_text):
        """UTF-8 helper should return None and print an error on decode failure."""
        err = io.StringIO()
        with redirect_stderr(err):
            ret = check_license_info._read_text_utf8_lines(Path("bad/file.txt"))
        self.assertIsNone(ret)
        self.assertEqual("bad/file.txt: Cannot read file as UTF-8.\n", err.getvalue())

    @patch("pathlib.Path.read_text", side_effect=FileNotFoundError("missing"))
    def test_read_text_utf8_lines_file_not_found_error(self, _mock_read_text):
        """UTF-8 helper should return None and print read error on missing files."""
        err = io.StringIO()
        with redirect_stderr(err):
            ret = check_license_info._read_text_utf8_lines(Path("missing/file.txt"))
        self.assertIsNone(ret)
        self.assertEqual(
            "missing/file.txt: Cannot read file (missing).\n",
            err.getvalue(),
        )

    @patch("pathlib.Path.read_text", side_effect=PermissionError("denied"))
    def test_read_text_utf8_lines_permission_error(self, _mock_read_text):
        """UTF-8 helper should return None and print read error on permission issues."""
        err = io.StringIO()
        with redirect_stderr(err):
            ret = check_license_info._read_text_utf8_lines(Path("protected/file.txt"))
        self.assertIsNone(ret)
        self.assertEqual(
            "protected/file.txt: Cannot read file (denied).\n",
            err.getvalue(),
        )

    @patch("pathlib.Path.relative_to", side_effect=ValueError)
    def test_as_repo_relative_value_error_fallback(self, _mock_relative_to):
        """Absolute path that cannot be relativized should be returned unchanged."""
        path = Path("D:/outside/src/file.c")
        ret = check_license_info._as_repo_relative(path)
        self.assertEqual(ret, path)

    @patch(
        "cli.pre_commit_scripts.check_license_info._as_repo_relative",
        return_value=Path("src/app/main/main.h"),
    )
    def test_source_to_test_file_non_c_suffix(self, _mock_relative):
        """Non-.c files should not be mapped to test files."""
        ret = check_license_info._source_to_test_file(Path("ignored.h"))
        self.assertIsNone(ret)

    @patch(
        "cli.pre_commit_scripts.check_license_info._as_repo_relative",
        return_value=Path("other/place/file.c"),
    )
    def test_source_to_test_file_without_src_segment(self, _mock_relative):
        """Path without a src segment should return None."""
        ret = check_license_info._source_to_test_file(Path("ignored.c"))
        self.assertIsNone(ret)

    @patch(
        "cli.pre_commit_scripts.check_license_info._as_repo_relative",
        return_value=MagicMock(suffix=".c", parts=("src",)),
    )
    def test_source_to_test_file_with_only_src_segment(self, _mock_relative):
        """Path equal to src only should return None."""
        ret = check_license_info._source_to_test_file(Path("ignored.c"))
        self.assertIsNone(ret)

    @patch(
        "cli.pre_commit_scripts.check_license_info._as_repo_relative",
        return_value=Path("src/app/main/main.py"),
    )
    def test_source_to_test_wscript_non_wscript_name(self, _mock_relative):
        """Non-wscript files should not be mapped to test wscript files."""
        ret = check_license_info._source_to_test_wscript(Path("ignored.py"))
        self.assertIsNone(ret)

    @patch(
        "cli.pre_commit_scripts.check_license_info._as_repo_relative",
        return_value=Path("other/place/wscript"),
    )
    def test_source_to_test_wscript_without_src_segment(self, _mock_relative):
        """Wscript path without a src segment should return None."""
        ret = check_license_info._source_to_test_wscript(Path("ignored/wscript"))
        self.assertIsNone(ret)

    @patch("cli.pre_commit_scripts.check_license_info._as_repo_relative")
    def test_source_to_test_wscript_with_only_src_segment(self, _mock_relative):
        """Wscript path equal to src only should return None."""
        mock_path = MagicMock()
        mock_path.name = "wscript"
        mock_path.parts = ("src",)
        _mock_relative.return_value = mock_path
        ret = check_license_info._source_to_test_wscript(Path("ignored/wscript"))
        self.assertIsNone(ret)


class TestMainDispatch(unittest.TestCase):
    """Tests for main dispatch behavior."""

    @patch("cli.pre_commit_scripts.check_license_info.check_yaml", return_value=7)
    @patch("cli.pre_commit_scripts.check_license_info.check_c", return_value=5)
    @patch("cli.pre_commit_scripts.check_license_info.check_py", return_value=3)
    def test_dispatches_by_extension(
        self,
        mock_check_py,
        mock_check_c,
        mock_check_yaml,
    ):
        """Main should route files to checker functions by extension."""
        ret = check_license_info.main(
            [
                "cli/pre_commit_scripts/check_license_info.py",
                "src/app/main/main.c",
                "docs/example.yml",
                "README.md",
                "--license-type",
                "BSD-3-Clause",
            ]
        )
        self.assertEqual(ret, 15)
        mock_check_py.assert_called_once_with(
            files=["cli/pre_commit_scripts/check_license_info.py"],
            license_type="BSD-3-Clause",
        )
        mock_check_c.assert_called_once_with(
            files=["src/app/main/main.c"],
            license_type="BSD-3-Clause",
        )
        mock_check_yaml.assert_called_once_with(
            files=["docs/example.yml"],
            license_type="BSD-3-Clause",
        )


class TestCheckerReadErrorHandling(unittest.TestCase):
    """Tests checker behavior when UTF-8 file reads fail."""

    @patch("cli.pre_commit_scripts.check_license_info.compare_header", return_value=7)
    @patch(
        "cli.pre_commit_scripts.check_license_info._read_text_utf8_lines",
        side_effect=[None, ["ok"]],
    )
    def test_check_asm_read_error_increments_and_continues(
        self, _mock_read, mock_compare
    ):
        """Assembler checker should count decode errors and continue with next file."""
        ret = check_license_info.check_asm(["bad.asm", "ok.asm"], "BSD-3-Clause")
        self.assertEqual(ret, 8)
        mock_compare.assert_called_once()

    @patch("cli.pre_commit_scripts.check_license_info.compare_header", return_value=3)
    @patch(
        "cli.pre_commit_scripts.check_license_info._read_text_utf8_lines",
        side_effect=[None, ["ok"]],
    )
    def test_check_py_read_error_increments_and_continues(
        self, _mock_read, mock_compare
    ):
        """Python checker should count decode errors and continue with next file."""
        ret = check_license_info.check_py(["bad.py", "ok.py"], "BSD-3-Clause")
        self.assertEqual(ret, 4)
        mock_compare.assert_called_once()

    @patch("cli.pre_commit_scripts.check_license_info.compare_header", return_value=2)
    @patch(
        "cli.pre_commit_scripts.check_license_info._read_text_utf8_lines",
        side_effect=[None, ["ok"]],
    )
    def test_check_yaml_read_error_increments_and_continues(
        self, _mock_read, mock_compare
    ):
        """YAML checker should count decode errors and continue with next file."""
        ret = check_license_info.check_yaml(["bad.yml", "ok.yml"], "BSD-3-Clause")
        self.assertEqual(ret, 3)
        mock_compare.assert_called_once()

    @patch("cli.pre_commit_scripts.check_license_info.compare_header")
    @patch(
        "cli.pre_commit_scripts.check_license_info._read_text_utf8_lines",
        return_value=None,
    )
    def test_check_c_source_read_error_skips_compare(self, _mock_read, mock_compare):
        """C checker should skip compare when source decoding fails."""
        ret = check_license_info.check_c(["src/app/main/main.c"], "BSD-3-Clause")
        self.assertEqual(ret, 1)
        mock_compare.assert_not_called()

    @patch("pathlib.Path.exists", return_value=True)
    @patch("cli.pre_commit_scripts.check_license_info.compare_header", return_value=4)
    @patch(
        "cli.pre_commit_scripts.check_license_info._source_to_test_file",
        return_value=Path("tests/unit/app/main/test_main.c"),
    )
    @patch(
        "cli.pre_commit_scripts.check_license_info._read_text_utf8_lines",
        side_effect=[["ok"], None],
    )
    def test_check_c_companion_read_error_counts_after_source_compare(
        self, _mock_read, _mock_map, mock_compare, _mock_exists
    ):
        """C checker should count companion decode errors after source compare."""
        ret = check_license_info.check_c(["src/app/main/main.c"], "BSD-3-Clause")
        self.assertEqual(ret, 5)
        mock_compare.assert_called_once()

    @patch("pathlib.Path.exists", return_value=True)
    @patch("cli.pre_commit_scripts.check_license_info.compare_header", return_value=4)
    @patch(
        "cli.pre_commit_scripts.check_license_info._source_to_test_wscript",
        return_value=Path("tests/unit/app/main/wscript"),
    )
    @patch(
        "cli.pre_commit_scripts.check_license_info._read_text_utf8_lines",
        side_effect=[["ok"], None],
    )
    def test_check_py_companion_read_error_counts_after_source_compare(
        self, _mock_read, _mock_map, mock_compare, _mock_exists
    ):
        """Python checker should count companion decode errors after source compare."""
        ret = check_license_info.check_py(["src/app/main/wscript"], "BSD-3-Clause")
        self.assertEqual(ret, 5)
        mock_compare.assert_called_once()

    @patch("cli.pre_commit_scripts.check_license_info.check_py", return_value=0)
    def test_does_not_filter_by_detected_license(self, mock_check_py):
        """Main should always check passed files for the requested license type."""
        ret = check_license_info.main(
            [
                "cli/pre_commit_scripts/check_license_info.py",
                "--license-type",
                "BSD-3-Clause",
            ]
        )
        self.assertEqual(ret, 0)
        mock_check_py.assert_called_once_with(
            files=["cli/pre_commit_scripts/check_license_info.py"],
            license_type="BSD-3-Clause",
        )


if __name__ == "__main__":
    unittest.main()
