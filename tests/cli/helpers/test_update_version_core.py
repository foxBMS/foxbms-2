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

"""Testing file 'cli/helpers/update_version_core.py'."""

import importlib.util
import shutil
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

try:
    from cli.helpers import update_version_core
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.helpers import update_version_core


class TestUpdateVersionCore(unittest.TestCase):  # pylint: disable=too-many-public-methods
    """Tests for low-level release version update helpers."""

    def setUp(self) -> None:  # noqa: D102
        self._tmpdir = tempfile.mkdtemp()
        self.root = Path(self._tmpdir)

    def tearDown(self) -> None:  # noqa: D102
        shutil.rmtree(self._tmpdir)

    def _write(self, relative: str, content: str, encoding: str = "utf-8") -> Path:
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding=encoding)
        return path

    def test_date_get_today_iso_format(self) -> None:
        """Return date in ISO yyyy-mm-dd format."""
        today = update_version_core.date_get_today()
        self.assertRegex(today, r"^\d{4}-\d{2}-\d{2}$")

    def test_get_previous_release_found(self) -> None:
        """Return first non-placeholder release in releases.csv."""
        self._write(
            "docs/general/releases.csv",
            "foxBMS 2;Date;Link\n"
            "vx.y.z;1970-01-01;https://example/vx.y.z/\n"
            "v1.2.3;2025-01-01;https://example/v1.2.3/\n",
        )

        self.assertEqual("1.2.3", update_version_core.get_previous_release(self.root))

    def test_get_previous_release_empty_if_no_release_found(self) -> None:
        """Return empty string if only placeholder release exists."""
        self._write(
            "docs/general/releases.csv",
            "foxBMS 2;Date;Link\nvx.y.z;1970-01-01;https://example/vx.y.z/\n",
        )

        self.assertEqual("", update_version_core.get_previous_release(self.root))

    def test_update_c_h_files_replaces_target_content(self) -> None:
        """Update @version and @updated in recognized foxBMS files."""
        candidate = self._write(
            "src/app/sample.c",
            " * @author  foxBMS Team\n"
            " * @updated 2024-01-01 (date of last update)\n"
            " * @version v1.2.3\n",
            encoding="ascii",
        )

        update_version_core.update_c_h_files(self.root, "2026-03-12", "1.2.3", "1.2.4")

        text = candidate.read_text(encoding="ascii")
        self.assertIn(" * @version v1.2.4", text)
        self.assertIn(" * @updated 2026-03-12 (date of last update)", text)

    def test_update_c_h_files_replaces_when_only_version_matches(self) -> None:
        """Update files selected through the explicit old-version match."""
        candidate = self._write(
            "src/app/version-only.h",
            " * @updated 2024-01-01 (date of last update)\n * @version v1.2.3\n",
            encoding="ascii",
        )

        update_version_core.update_c_h_files(self.root, "2026-03-12", "1.2.3", "1.2.4")

        text = candidate.read_text(encoding="ascii")
        self.assertIn(" * @version v1.2.4", text)
        self.assertIn(" * @updated 2026-03-12 (date of last update)", text)

    def test_update_c_h_files_handles_non_utf8_and_no_replace(self) -> None:
        """Skip files that cannot be decoded or are not replacement candidates."""
        undecodable = self.root / "src/app/invalid.c"
        undecodable.parent.mkdir(parents=True, exist_ok=True)
        undecodable.write_bytes(b"\xff\xfe\x00")

        untouched = self._write(
            "src/app/not-foxbms.h",
            " * @updated 2024-01-01 (date of last update)\n * @version v9.9.9\n",
            encoding="ascii",
        )

        update_version_core.update_c_h_files(self.root, "2026-03-12", "1.2.3", "1.2.4")

        self.assertNotIn("v1.2.4", untouched.read_text(encoding="ascii"))

    def test_update_wscript_updates_version(self) -> None:
        """Replace from-version with to-version in wscript."""
        wscript = self._write("wscript", 'VERSION = "1.2.3"\n')
        update_version_core.update_wscript(self.root, "1.2.3", "1.2.4")
        self.assertIn('VERSION = "1.2.4"', wscript.read_text(encoding="utf-8"))

    def test_update_citation_to_release_version(self) -> None:
        """Update citation version and release date on concrete release."""
        citation = self._write(
            "CITATION.cff",
            f'version: "1.2.3"\ndate-released: "{update_version_core.MAGIC_DATE}"\n',
        )
        update_version_core.update_citation(self.root, "2026-03-12", "1.2.3", "1.2.4")
        text = citation.read_text(encoding="utf-8")
        self.assertIn('version: "1.2.4"', text)
        self.assertIn('date-released: "2026-03-12"', text)

    def test_update_citation_to_placeholder_version(self) -> None:
        """Reset date to magic date when switching back to x.y.z."""
        citation = self._write(
            "CITATION.cff",
            f'version: "1.2.4"\ndate-released: "{update_version_core.MAGIC_DATE}"\n',
        )
        update_version_core.update_citation(self.root, "2026-03-12", "1.2.4", "x.y.z")
        text = citation.read_text(encoding="utf-8")
        self.assertIn('version: "x.y.z"', text)
        self.assertIn(f'date-released: "{update_version_core.MAGIC_DATE}"', text)

    def test_update_changelog_to_placeholder_version(self) -> None:
        """Insert x.y.z placeholder section when preparing next release."""
        lines = [f"line-{index}" for index in range(30)]
        changelog = self._write("docs/general/changelog.rst", "\n".join(lines) + "\n")

        update_version_core.update_changelog(self.root, "2026-03-12", "1.2.3", "x.y.z")

        text = changelog.read_text(encoding="utf-8")
        heading = f"[x.y.z] - {update_version_core.MAGIC_DATE}"
        separator = "*" * len(heading)
        self.assertIn(f"{separator}\n{heading}\n{separator}", text)
        self.assertIn(".. include:: ./changelog-entries/vx.y.z.txt", text)

    def test_update_changelog_to_release_version(self) -> None:
        """Promote vx.y.z changelog entry to concrete version and recreate template."""
        self._write(
            "docs/general/changelog.rst",
            "********************\n"
            f"[1.2.3] - {update_version_core.MAGIC_DATE}\n"
            "********************\n"
            ".. include:: ./changelog-entries/vx.y.z.txt\n",
        )
        self._write(
            "docs/general/changelog-entries/vx.y.z.txt",
            "Current release notes\n",
        )

        update_version_core.update_changelog(
            self.root, "2026-03-12", "1.2.3", "12.34.56"
        )

        promoted = self.root / "docs/general/changelog-entries/v12.34.56.txt"
        self.assertTrue(promoted.exists())
        recreated = self.root / "docs/general/changelog-entries/vx.y.z.txt"
        self.assertEqual(
            update_version_core.RELEASE_ENTRY_TEMPLATE,
            recreated.read_text(encoding="utf-8"),
        )
        changelog_text = (self.root / "docs/general/changelog.rst").read_text(
            encoding="utf-8"
        )
        heading = "[12.34.56] - 2026-03-12"
        separator = "*" * len(heading)
        self.assertIn(f"{separator}\n{heading}\n{separator}", changelog_text)
        self.assertIn(".. include:: ./changelog-entries/v12.34.56.txt", changelog_text)

    def test_update_changelog_release_heading_line_lengths_match(self) -> None:
        """Use heading-length dependent overline/underline after version replacement."""
        changelog_file = "docs/general/changelog.rst"
        self._write(
            changelog_file,
            "********************\n"
            f"[1.2.3] - {update_version_core.MAGIC_DATE}\n"
            "********************\n"
            "\n"
            ".. include:: ./changelog-entries/vx.y.z.txt\n",
        )
        self._write(
            "docs/general/changelog-entries/vx.y.z.txt",
            "Current release notes\n",
        )

        update_date = "2026-03-12"
        _from = "1.2.3"
        _to = "123.456.789.1011"
        update_version_core.update_changelog(self.root, update_date, _from, _to)

        changelog_lines = (
            (self.root / changelog_file).read_text(encoding="utf-8").splitlines()
        )
        heading = f"[{_to}] - {update_date}"
        heading_index = changelog_lines.index(heading)
        overline = changelog_lines[heading_index - 1]
        underline = changelog_lines[heading_index + 1]

        self.assertEqual("*" * len(heading), overline)
        self.assertEqual("*" * len(heading), underline)

    def test_update_commit_fragments_placeholder_keeps_template(self) -> None:
        """Do not create release file when to-version is x.y.z."""
        next_release = self._write(
            "docs/general/commit-msgs/next-release.txt",
            "<Major/Minor/Bugfix> release of foxBMS 2 (v1.2.3)\n"
            "https://example/v1.2.3/general/changelog.html\n",
        )

        update_version_core.update_commit_fragments(
            self.root, "1.2.2", "1.2.3", "x.y.z"
        )

        self.assertFalse((next_release.parent / "release-vx.y.z.txt").exists())
        reset_text = next_release.read_text(encoding="utf-8")
        self.assertIn("vx.y.z", reset_text)

    def test_update_commit_fragments_bugfix_minor_and_major(self) -> None:
        """Classify change type based on previous and target versions."""
        next_release = self._write(
            "docs/general/commit-msgs/next-release.txt",
            "<Major/Minor/Bugfix> release of foxBMS 2 (v1.2.3)\n"
            "https://example/v1.2.3/general/changelog.html\n",
        )

        update_version_core.update_commit_fragments(
            self.root, "1.2.2", "1.2.3", "1.2.4"
        )
        bugfix = (next_release.parent / "release-v1.2.4.txt").read_text(
            encoding="utf-8"
        )
        self.assertIn("Bugfix", bugfix)

        next_release.write_text(
            "<Major/Minor/Bugfix> release of foxBMS 2 (v1.2.3)\n"
            "https://example/v1.2.3/general/changelog.html\n",
            encoding="utf-8",
        )
        update_version_core.update_commit_fragments(
            self.root, "1.2.3", "1.2.3", "1.3.0"
        )
        minor = (next_release.parent / "release-v1.3.0.txt").read_text(encoding="utf-8")
        self.assertIn("Minor", minor)

        next_release.write_text(
            "<Major/Minor/Bugfix> release of foxBMS 2 (v1.2.3)\n"
            "https://example/v1.2.3/general/changelog.html\n",
            encoding="utf-8",
        )
        update_version_core.update_commit_fragments(
            self.root, "1.2.3", "1.2.3", "2.0.0"
        )
        major = (next_release.parent / "release-v2.0.0.txt").read_text(encoding="utf-8")
        self.assertIn("Major", major)

    def test_update_commit_fragments_invalid_version_raises(self) -> None:
        """Raise SystemExit on non-numeric version values."""
        self._write(
            "docs/general/commit-msgs/next-release.txt",
            "<Major/Minor/Bugfix> release of foxBMS 2 (v1.2.3)\n"
            "https://example/v1.2.3/general/changelog.html\n",
        )

        with self.assertRaisesRegex(SystemExit, "unexpected version identifier"):
            update_version_core.update_commit_fragments(
                self.root, "1.2.2", "1.2.3", "1.2.x"
            )

    def test_update_release_csv_to_placeholder(self) -> None:
        """Insert vx.y.z release row at top when preparing next release."""
        releases = self._write(
            "docs/general/releases.csv",
            "foxBMS 2;Date;Link\nv1.2.3;1970-01-01;https://example/v1.2.3/\n",
        )

        update_version_core.update_release_csv(
            self.root, "2026-03-12", "1.2.3", "x.y.z"
        )

        text = releases.read_text(encoding="utf-8")
        self.assertIn("vx.y.z;     1970-01-01;", text)

    def test_update_release_csv_placeholder_duplicate_raises(self) -> None:
        """Raise SystemExit when vx.y.z already exists as first data line."""
        self._write(
            "docs/general/releases.csv",
            "foxBMS 2;Date;Link\nvx.y.z;1970-01-01;https://example/vx.y.z/\n",
        )

        with self.assertRaisesRegex(SystemExit, "already sets 'vx.y.z'"):
            update_version_core.update_release_csv(
                self.root, "2026-03-12", "1.2.3", "x.y.z"
            )

    def test_update_release_csv_to_concrete_version(self) -> None:
        """Update from-version row, date and URL for concrete release."""
        releases = self._write(
            "docs/general/releases.csv",
            "foxBMS 2;Date;Link\nv1.2.3;1970-01-01;https://example/v1.2.3/\n",
        )

        update_version_core.update_release_csv(
            self.root, "2026-03-12", "1.2.3", "1.2.4"
        )

        text = releases.read_text(encoding="utf-8")
        self.assertIn("v1.2.4;2026-03-12;https://example/v1.2.4/", text)

    @patch("cli.helpers.update_version_core.update_release_csv")
    @patch("cli.helpers.update_version_core.update_commit_fragments")
    @patch("cli.helpers.update_version_core.update_changelog")
    @patch("cli.helpers.update_version_core.update_citation")
    @patch("cli.helpers.update_version_core.update_wscript")
    @patch("cli.helpers.update_version_core.update_c_h_files")
    @patch("cli.helpers.update_version_core.date_get_today", return_value="2026-03-12")
    @patch("cli.helpers.update_version_core.get_previous_release", return_value="1.2.2")
    def test_apply_update_version(  # noqa: PLR0913
        self,
        get_previous_release: MagicMock,
        date_get_today: MagicMock,
        update_c_h_files: MagicMock,
        update_wscript: MagicMock,
        update_citation: MagicMock,
        update_changelog: MagicMock,
        update_commit_fragments: MagicMock,
        update_release_csv: MagicMock,
    ) -> None:
        """Run all low-level update steps in sequence with shared date value."""
        update_version_core.apply_update_version(self.root, "1.2.3", "1.2.4")

        get_previous_release.assert_called_once_with(self.root)
        date_get_today.assert_called_once_with()
        update_c_h_files.assert_called_once_with(
            self.root, "2026-03-12", "1.2.3", "1.2.4"
        )
        update_wscript.assert_called_once_with(self.root, "1.2.3", "1.2.4")
        update_citation.assert_called_once_with(
            self.root, "2026-03-12", "1.2.3", "1.2.4"
        )
        update_changelog.assert_called_once_with(
            self.root, "2026-03-12", "1.2.3", "1.2.4"
        )
        update_commit_fragments.assert_called_once_with(
            self.root, "1.2.2", "1.2.3", "1.2.4"
        )
        update_release_csv.assert_called_once_with(
            self.root, "2026-03-12", "1.2.3", "1.2.4"
        )

    @patch("cli.helpers.update_version_core.get_previous_release", return_value="")
    def test_apply_update_version_missing_previous_release(
        self, get_previous_release_mock: MagicMock
    ) -> None:
        """Raise SystemExit when previous release cannot be determined."""
        with self.assertRaisesRegex(SystemExit, "Could not determine previous version"):
            update_version_core.apply_update_version(self.root, "1.2.3", "1.2.4")
        get_previous_release_mock.assert_called_once_with(self.root)

    def test_parse_args(self) -> None:
        """Parse standalone CLI arguments for script execution."""
        argv = [
            "update_version_core.py",
            "--root",
            str(self.root),
            "--from-version",
            "1.2.3",
            "--to-version",
            "1.2.4",
        ]
        with patch.object(sys, "argv", argv):
            args = update_version_core._parse_args()  # pylint: disable=protected-access

        self.assertEqual(str(self.root), args.root)
        self.assertEqual("1.2.3", args.from_version)
        self.assertEqual("1.2.4", args.to_version)

    @patch("cli.helpers.update_version_core.apply_update_version")
    @patch("cli.helpers.update_version_core._parse_args")
    def test_main(
        self, mock_parse_args: MagicMock, mock_apply_update_version: MagicMock
    ) -> None:
        """Run standalone main and call update workflow."""
        mock_parse_args.return_value = type(
            "Args",
            (),
            {
                "root": str(self.root),
                "from_version": "1.2.3",
                "to_version": "1.2.4",
            },
        )

        ret = update_version_core.main()

        self.assertEqual(0, ret)
        mock_apply_update_version.assert_called_once_with(self.root, "1.2.3", "1.2.4")

    def test_standalone_import_fallback_logger(self) -> None:
        """Load module standalone to exercise fallback logger import path."""
        module_path = Path(__file__).parents[3] / "cli/helpers/update_version_core.py"
        module_name = "update_version_core_standalone_test"
        spec = importlib.util.spec_from_file_location(module_name, module_path)
        if spec is None or spec.loader is None:
            self.fail("Failed to create module spec for standalone import test.")
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)

        self.assertEqual(module_name, module.logger.name)


if __name__ == "__main__":
    unittest.main()
