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

"""Add git version information to build artifacts"""

from git import Git, Repo
from git.exc import GitCommandError, InvalidGitRepositoryError

from vcs import VcsInformation
from waflib import Logs
from waflib.Configure import ConfigurationContext, conf


@conf  # pragma: no cover
def find_git(ctx: ConfigurationContext):
    """Find the 'git' executable and determine its version."""
    ctx.find_program(["git"], var="GIT")
    ctx.env.GIT_VERSION = Git().version()


def configure(ctx: ConfigurationContext):  # pragma: no cover
    """Find 'git' executable."""
    ctx.find_git()


@conf
def gather_and_validate_version_info(ctx) -> VcsInformation:  # pragma: no cover
    """Determine version information; to be used in the build context"""
    # gather all the version information we have in 1) sources, 2) build
    # scripts and 3) from the version control system
    major, minor, patch = ctx.env.VERSION.split(".")
    version = VcsInformation()
    version.major = major
    version.minor = minor
    version.patch = patch
    if not ctx.env.GIT:
        return version

    # if developed outside a repository, no real version information can be
    # obtained; just use the one defined in the wscript
    try:
        cwd = ctx.env.PROJECT_ROOT[0]
        search_parent_directories = False
    except IndexError:
        cwd = ctx.path.abspath()
        search_parent_directories = True
    repo = get_repo(cwd, search_parent_directories)
    if not repo:
        return version

    version.under_version_control = True
    version.full_hash = repo.head.commit.hexsha

    version.remote = get_remote(repo, version)
    version.dirty = repo.is_dirty()
    version.distance = get_master_commit_distance(repo, version)
    version.tag = get_bare_tag(repo.git.describe("--tags"))
    if not validate_tag_vs_version(version):
        err = (
            f"Extracted version from git repo ({version.tag}) "
            f"does not match version defined in waf ({ctx.env.VERSION})."
        )
        ctx.fatal(err)

    return version


# helper functions that are used in the build process
# MUST NOT use a waf context


def get_repo(path, search_parent_directories) -> Repo | None:
    """Open a repository at the given path.

    Args:
        path: Filesystem path from which to try opening the repository.
        search_parent_directories: Whether to walk up parent directories to
            find a repository.

    Returns:
        The opened repository if successful; otherwise None. Emits warnings on
        failure.
    """
    try:
        return Repo(path, search_parent_directories=search_parent_directories)
    except InvalidGitRepositoryError:
        Logs.warn("Not a git repository. Proceeding without version information.")
    except Exception as e:  # noqa: BLE001 E722
        Logs.warn(
            f"An unexpected error occurred:\n{e}\nProceeding without version information."
        )
    return None


def get_tag(repo: Repo) -> str:
    """Return the description of the current commit as produced by 'git describe --tags'.

    Args:
        repo: The repository from which to read the tag description.

    Returns:
        The describe string for the current commit, or an empty string if no
        tag can be determined.
    """
    try:
        return repo.git.describe("--tags")
    except GitCommandError:
        Logs.warn("Could not determine tags")
        return ""


def get_remote(repo: Repo, version: VcsInformation) -> str | None:
    """Determine the 'origin' remote URL.

    Args:
        repo: The repository from which to read the remote information.
        version: The version container used to provide a fallback if the remote
            cannot be read.

    Returns:
        The remote URL if it can be determined; otherwise the current value
        stored on the version object.
    """
    try:
        return repo.git.ls_remote("--get-url")
    except GitCommandError:
        Logs.warn("Remote could not be determined")
    return version.remote


def get_master_commit_distance(repo: Repo, version: VcsInformation) -> int:
    """Compute how many commits the current commit is ahead of 'master'.

    Args:
        repo: The repository used for running the revision query.
        version: The version container providing the short hash and a fallback
            value.

    Returns:
        The number of commits reachable from the current commit but not from
        'master'. If the command fails or the result cannot be parsed, returns
        the existing value from the version container.
    """
    try:
        distance = repo.git.rev_list("--count", version.short_hash, "^master")
    except GitCommandError:
        return version.distance
    try:
        return int(distance)
    except ValueError:
        return version.distance


def get_bare_tag(txt: str) -> str:
    """Extract the plain tag (major.minor.patch) from a describe-like string.

    Args:
        txt: A string that may start with 'v' or 'gh-' and optionally include a
            '-<distance>-<sha>' suffix.

    Returns:
        The plain tag without prefixes and suffixes if present; otherwise an
        empty string.
    """
    tag = ""
    if txt.startswith(("v", "gh-")):
        # internal releases are tagged by        'v<version>'
        # while GitHub releases are tagged with  'gh-<version>'
        # This is not a typo, at some point we chose it that way and we
        # keep it like that.
        txt = txt.removeprefix("v") if txt.startswith("v") else txt.removeprefix("gh-")
        # we are on a branch with a tagged version
        try:
            tag, _ = txt.split("-", 1)
        except ValueError:
            tag = txt
    return tag


def validate_tag_vs_version(version: VcsInformation) -> bool:
    """Check that the extracted tag matches the configured version.

    Args:
        version: The version container holding major, minor, patch and the
            extracted tag.

    Returns:
        True if the extracted tag equals the configured version, or if the
        configured version equals the special placeholder '120.121.122';
        otherwise False.
    """
    tmp = (version.major, version.minor, version.patch)
    waf_version = ".".join(str(i) for i in tmp)
    if waf_version not in (version.tag, "120.121.122"):
        return False
    return True
