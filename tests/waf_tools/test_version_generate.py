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

"""Testing file 'tools/waf_tools/version_generate.py'."""

import hashlib
import sys
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import MagicMock, patch

ROOT = Path(__file__).parents[2]
FIXTURES = ROOT / "tests/waf_tools/fixtures/version_generate"
FIXED_DATE = "2026-03-23"

for i in [
    ROOT / "tools/waf_tools",
    ROOT / "tools/waf3-2.1.9-beba77c244731800bf15a003232e7040",  # Windows
    ROOT / "tools/.waf3-2.1.9-beba77c244731800bf15a003232e7040",  # Linux
]:
    if i.exists():
        sys.path.insert(0, str(i))

# pylint: disable=wrong-import-position

from vcs import VcsInformation  # noqa: E402
from version_generate import (  # noqa: E402
    create_version_c,
    create_version_file,
    create_version_source,
)
from waflib.Task import RUN_ME, SKIP_ME  # noqa: E402

# pylint: enable


def _make_task() -> create_version_source:
    """Build a create_version_source task with minimal mocked waf context."""
    task = create_version_source.__new__(create_version_source)
    task.generator = SimpleNamespace(bld=MagicMock())
    task.outputs = [MagicMock(), MagicMock()]
    task.inputs = [MagicMock()]
    return task


class CreateVersionSourceCreateVersionHash(unittest.TestCase):
    """Tests for create_version_source Task class methods."""

    @patch("version_generate.fast_get_version_info_git", return_value="VERSION")
    def test_create_version_hash_hashes_version_repr(self, _: MagicMock) -> None:
        """Compute deterministic SHA256 digest from version repr."""
        task = _make_task()
        expected_hash = hashlib.sha256(repr("VERSION").encode("utf-8")).hexdigest()
        result = task.create_version_hash()
        self.assertEqual(result, expected_hash)


class CreateVersionSourceRunnableStatus(unittest.TestCase):
    """Tests for create_version_source Task class methods."""

    @patch("version_generate.Task.runnable_status", return_value=SKIP_ME)
    def test_runnable_status_forwards_non_run_me(self, _: MagicMock) -> None:
        """Return super status directly when task is not ready to run."""
        task = _make_task()
        task.create_version_hash = MagicMock()
        self.assertEqual(task.runnable_status(), SKIP_ME)
        task.create_version_hash.assert_not_called()

    @patch("version_generate.Task.runnable_status", return_value=RUN_ME)
    def test_runnable_status_returns_skip_when_hash_unchanged(
        self, _: MagicMock
    ) -> None:
        """Skip task when previously stored hash equals current hash."""
        task = _make_task()
        task.create_version_hash = MagicMock(return_value="same-hash")
        task.outputs[
            0
        ].parent.find_or_declare.return_value.read.return_value = "same-hash"

        self.assertEqual(task.runnable_status(), SKIP_ME)
        task.outputs[0].parent.make_node.assert_not_called()

    @patch("version_generate.Task.runnable_status", return_value=RUN_ME)
    def test_runnable_status_writes_hash_and_runs_when_hash_differs(
        self, _: MagicMock
    ) -> None:
        """Write new hash file and execute when hash changed or absent."""
        task = _make_task()
        task.create_version_hash = MagicMock(return_value="new-hash")
        task.outputs[
            0
        ].parent.find_or_declare.return_value.read.side_effect = FileNotFoundError()

        self.assertEqual(task.runnable_status(), RUN_ME)
        task.outputs[0].parent.find_or_declare.assert_called_once_with("version.hash")
        task.outputs[
            0
        ].parent.find_or_declare.return_value.write.assert_called_once_with(
            "new-hash", encoding="utf-8"
        )


class CreateVersionFile(unittest.TestCase):
    """Tests for create_version_file task-generator helper."""

    def _make_taskgen(self) -> SimpleNamespace:
        src_node = object()
        version_c_node = object()
        no_clang_node = object()
        taskgen = SimpleNamespace(
            version=True,
            env=SimpleNamespace(PROJECT_ROOT=["."]),
            path=SimpleNamespace(
                ctx=SimpleNamespace(
                    root=SimpleNamespace(find_node=MagicMock(return_value=src_node))
                ),
                find_or_declare=MagicMock(side_effect=[version_c_node, no_clang_node]),
            ),
            create_task=MagicMock(
                return_value=SimpleNamespace(outputs=[version_c_node])
            ),
            source=["main.c"],
        )
        taskgen.src_node = src_node
        taskgen.version_c_node = version_c_node
        taskgen.no_clang_node = no_clang_node
        return taskgen

    def test_create_version_file_returns_early_without_version(self) -> None:
        """Do nothing when version generation is disabled."""
        taskgen = self._make_taskgen()
        taskgen.version = False

        create_version_file(taskgen)

        taskgen.create_task.assert_not_called()
        self.assertEqual(taskgen.source, ["main.c"])

    def test_create_version_file_adds_generated_source(self) -> None:
        """Use fixed template source and fixed version.c target node."""
        taskgen = self._make_taskgen()

        create_version_file(taskgen)

        taskgen.path.ctx.root.find_node.assert_called_once_with("./conf/tpl/c.c")
        taskgen.path.find_or_declare.assert_any_call("version.c")
        taskgen.path.find_or_declare.assert_any_call(".clang-format")
        taskgen.create_task.assert_called_once_with(
            "create_version_source",
            src=taskgen.src_node,
            tgt=[taskgen.version_c_node, taskgen.no_clang_node],
        )
        self.assertEqual(taskgen.source, ["main.c", taskgen.version_c_node])

    def test_create_version_file_handles_non_list_source(self) -> None:
        """Fallback path: convert scalar source into list before append."""
        taskgen = self._make_taskgen()
        taskgen.source = "main.c"
        create_version_file(taskgen)
        self.assertEqual(taskgen.source, ["main.c", taskgen.version_c_node])


class CreateVersionCTests(unittest.TestCase):
    """Fixture-based tests for version_generate_c."""

    @patch("version_generate.datetime")
    def test_render_matches_expected_fixture(self, mock_datetime: MagicMock) -> None:
        """Render the template and compare against a committed golden file."""
        mock_datetime.now.return_value.date.return_value.strftime.return_value = (
            FIXED_DATE
        )

        version = VcsInformation()
        version.major = 7
        version.minor = 8
        version.patch = 9
        version.distance = 5
        version.full_hash = "deadbeef"
        version.remote = "origin"
        version.under_version_control = True
        version.dirty = False

        ctx = MagicMock()
        ctx.gather_and_validate_version_info.return_value = version
        ctx.env.VERSION = "7.8.9"

        template = (ROOT / "conf/tpl/c.c").read_text(encoding="utf-8")
        expected = (FIXTURES / "expected-version.c").read_text(encoding="utf-8")

        result = create_version_c(ctx, template)
        self.assertMultiLineEqual(expected, result)


if __name__ == "__main__":
    unittest.main()
