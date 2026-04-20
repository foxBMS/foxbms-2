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


# cspell: ignore gabcdef gdeadbeef
"""Testing file 'tools/waf-tools/vcs_git.py'."""

import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

from git.exc import GitCommandError, InvalidGitRepositoryError

ROOT = Path(__file__).parents[2]

for i in [
    ROOT / "tools/waf-tools",
    ROOT / "tools/waf3-2.1.6-6a38d8c49406d2fef32d6f6600c8f033",  # Windows
    ROOT / "tools/waf.3-2.1.6-6a38d8c49406d2fef32d6f6600c8f033",  # Linux
]:
    if i.exists():
        sys.path.insert(0, str(i))

import vcs_git  # noqa: E402 pylint: disable=wrong-import-position
from vcs import VcsInformation  # noqa: E402 pylint: disable=wrong-import-position


class WarnPatchedTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls._warn_patcher = patch.object(vcs_git.Logs, "warn")
        cls.warn_mock = cls._warn_patcher.start()

    @classmethod
    def tearDownClass(cls):
        cls._warn_patcher.stop()

    def setUp(self):
        self.warn_mock.reset_mock()


class TestGetRepo(WarnPatchedTestCase):
    def test_get_repo_success(self):
        with patch.object(vcs_git, "Repo", return_value="foo"):
            repo = vcs_git.get_repo("/tmp/repo", False)  # noqa: S108
        self.assertEqual(repo, "foo")
        self.warn_mock.assert_not_called()

    def test_get_repo_invalid_repo(self):
        with patch.object(
            vcs_git, "Repo", side_effect=[InvalidGitRepositoryError("foo")]
        ):
            repo = vcs_git.get_repo("/tmp/repo", False)  # noqa: S108
        self.assertIsNone(repo)
        self.assertEqual(
            "Not a git repository. Proceeding without version information.",
            self.warn_mock.mock_calls[0].args[0],
        )

    def test_get_repo_generic_exception(self):
        with patch.object(vcs_git, "Repo", side_effect=[Exception("foo")]):
            repo = vcs_git.get_repo("/tmp/repo", False)  # noqa: S108
        self.assertIsNone(repo)
        self.assertEqual(
            "An unexpected error occurred:\nfoo\nProceeding without version information.",
            self.warn_mock.mock_calls[0].args[0],
        )


class TestGetTag(WarnPatchedTestCase):
    def test_get_tag_success(self):
        repo = MagicMock()
        repo.git.describe.return_value = "v1.2.3-0-gabcdef"
        self.assertEqual(vcs_git.get_tag(repo), "v1.2.3-0-gabcdef")
        self.warn_mock.assert_not_called()

    def test_get_tag_failure(self):
        repo = MagicMock()
        repo.git.describe.side_effect = [GitCommandError("foo")]
        self.assertEqual(vcs_git.get_tag(repo), "")
        self.assertEqual(
            "Could not determine tags", self.warn_mock.mock_calls[0].args[0]
        )


class TestGetRemote(WarnPatchedTestCase):
    def test_get_remote_success(self):
        repo = MagicMock()
        repo.git.ls_remote.return_value = "foo/repo.git"
        self.assertEqual(vcs_git.get_remote(repo, MagicMock()), "foo/repo.git")
        self.warn_mock.assert_not_called()

    def test_get_remote_failure(self):
        version = VcsInformation()
        version.remote = "fallback-url"
        repo = MagicMock()
        repo.git.ls_remote.side_effect = [GitCommandError("foo")]
        self.assertEqual(vcs_git.get_remote(repo, version), "fallback-url")
        self.assertEqual(
            "Remote could not be determined", self.warn_mock.mock_calls[0].args[0]
        )


class TestGetMasterCommitDistance(unittest.TestCase):
    def test_get_master_commit_distance_success(self):
        version = VcsInformation()
        version._short_hash = "abc123"  # pylint: disable=protected-access
        repo = MagicMock()
        repo.git.rev_list.return_value = "5"
        self.assertEqual(vcs_git.get_master_commit_distance(repo, version), 5)
        repo.git.rev_list.assert_called_with("--count", "abc123", "^master")

    def test_get_master_commit_distance_cmd_error(self):
        version = VcsInformation()
        version.distance = 42
        repo = MagicMock()
        repo.git.rev_list.side_effect = [GitCommandError("foo")]
        self.assertEqual(vcs_git.get_master_commit_distance(repo, version), 42)

    def test_get_master_commit_distance_value_error(self):
        version = VcsInformation()
        version.distance = 7
        repo = MagicMock()
        repo.git.rev_list.return_value = "abc"
        self.assertEqual(vcs_git.get_master_commit_distance(repo, version), 7)


class TestGetBareTag(unittest.TestCase):
    def test_get_bare_tag_cases(self):
        cases = [
            ("v1.2.3", "1.2.3"),
            ("v1.2.3-4-gabcdef", "1.2.3"),
            ("gh-2.0.0-1-gdeadbeef", "2.0.0"),
            ("gh-2.0.0", "2.0.0"),
            ("1.2.3", ""),
            ("gh-", ""),
            ("v", ""),
        ]
        for input_txt, expected in cases:
            with self.subTest(input=input_txt):
                self.assertEqual(vcs_git.get_bare_tag(input_txt), expected)


class TestValidateTagVsVersion(unittest.TestCase):
    def test_validate_tag_vs_version_match(self):
        version = VcsInformation()
        version.major = 1
        version.minor = 2
        version.patch = 3
        version.tag = "1.2.3"
        self.assertTrue(vcs_git.validate_tag_vs_version(version))

    def test_validate_tag_vs_version_mismatch(self):
        version = VcsInformation()
        version.major = 1
        version.minor = 2
        version.patch = 4
        version.tag = "1.2.3"
        self.assertFalse(vcs_git.validate_tag_vs_version(version))

    def test_validate_tag_vs_version_placeholder(self):
        version = VcsInformation()
        version.major = 120
        version.minor = 121
        version.patch = 122
        version.tag = ""
        self.assertTrue(vcs_git.validate_tag_vs_version(version))

        version = VcsInformation()
        version.major = "x"
        version.minor = "y"
        version.patch = "z"
        version.tag = ""
        self.assertTrue(vcs_git.validate_tag_vs_version(version))


if __name__ == "__main__":
    unittest.main()
