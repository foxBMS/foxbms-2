#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Update version script"""

import argparse
import csv
import logging
import os
import re
import sys
from datetime import date
from pathlib import Path

from git import Repo

MAGIC_DATE = "xxxx-xx-xx"

RELEASE_ENTRY_TEMPLATE = """\
Added
=====

Changed
=======

Deprecated
==========

Removed
=======

Fixed
=====
"""


def get_git_root(path: str = os.path.realpath(__file__)) -> Path:
    """helper function to find the repository root

    Args:
        path: path of file in git repository

    Returns:
        root: root path of the git repository
    """
    repo = Repo(path, search_parent_directories=True)
    root = Path(repo.git.rev_parse("--show-toplevel"))
    return root


def date_get_today() -> str:
    """returns the date string for the output"""
    today = date.today()
    return today.strftime("%Y-%m-%d")


def get_previous_release(root: Path) -> str:
    """Return the previous version number"""
    path = str(root / "docs/general/releases.csv")
    with open(path, encoding="utf-8") as f:
        reader = csv.DictReader(f, delimiter=";")
        for row in reader:
            if row["foxBMS 2"] != "vx.y.z":
                return row["foxBMS 2"][1:]
    return ""


def update_c_h_files(root: Path, iso_date_today: str, _from: str, _to: str):
    """update .c,  and .h files"""
    all_c_h_files = (
        list((root / "conf").rglob("**/*.[c|h]"))
        + list((root / "docs").rglob("**/*.[c|h]"))
        + list((root / "src").rglob("**/*.[c|h]"))
        + list((root / "tests").rglob("**/*.[c|h]"))
        + list((root / "tools/crc").rglob("**/*.[c|h]"))
        + [
            Path(
                "tests/cli/pre_commit_scripts/test_check_sections/unknown-file-extension.abc"
            )
        ]
    )
    updated = re.compile(r" \* @updated (\d{4}-\d{2}-\d{2}) \(date of last update\)")
    updated_new = f" * @updated {iso_date_today} (date of last update)"
    old_version = f" * @version v{_from}"
    new_version = f" * @version v{_to}"
    logging.debug("from: %s", old_version)
    logging.debug("to: %s", new_version)
    for i in all_c_h_files:
        do_replace = False
        try:
            txt = i.read_text(encoding="utf-8")
        except:  # noqa: E722 pylint: disable=bare-except
            continue
        if " * @author  foxBMS Team" in txt:
            do_replace = True
        elif "ignore-author-comment" in i.name:
            do_replace = True
        elif "no-author-comment" in i.name:
            do_replace = True
        elif "nxp_mc33775a-ll" in i.name:
            do_replace = True
        if not do_replace:
            continue
        # re-read file to ensure ascii encoding on our files
        txt = i.read_text(encoding="ascii")
        logging.debug("Found foxBMS 2 file: %s", i)
        if old_version in txt:
            logging.debug("...Replacing version")
            txt = txt.replace(old_version, new_version, 1)
            logging.debug("...Replacing updated")
            txt = updated.sub(updated_new, txt)
            i.write_text(txt, encoding="ascii")


def update_wscript(root: Path, _from: str, _to: str):
    """update wscript"""
    wscript = root / "wscript"
    txt = wscript.read_text(encoding="utf-8")
    txt = txt.replace(f'VERSION = "{_from}"', f'VERSION = "{_to}"')
    wscript.write_text(txt, encoding="utf-8")


def update_citation(root: Path, iso_date_today: str, _from: str, _to: str):
    """Update citation file"""
    citation = root / "CITATION.cff"
    txt = citation.read_text(encoding="utf-8")
    txt = txt.replace(f'version: "{_from}"', f'version: "{_to}"')
    if _to == "x.y.z":  # back to development, use magic date string
        txt = re.sub(r"date-released: .*", f"date-released: {MAGIC_DATE}", txt)
    else:
        txt = txt.replace(
            f"date-released: {MAGIC_DATE}", f"date-released: {iso_date_today}"
        )
    citation.write_text(txt, encoding="utf-8")


def update_changelog(root: Path, iso_date_today: str, _from: str, _to: str):
    """Update changelog"""
    changelog = root / "docs/general/changelog.rst"
    logging.debug("Patching %s", changelog)
    txt = changelog.read_text(encoding="utf-8")
    if _to == "x.y.z":
        tmp = txt.splitlines()
        tmp.insert(
            24,
            "********************\n[x.y.z] - xxxx-xx-xx\n********************\n\n"
            ".. include:: ./changelog-entries/vx.y.z.txt\n",
        )
        txt = "\n".join(tmp) + "\n"
    else:
        txt = txt.replace(f"[{_from}] - {MAGIC_DATE}", f"[{_to}] - {iso_date_today}")
        txt = txt.replace(
            ".. include:: ./changelog-entries/vx.y.z.txt",
            f".. include:: ./changelog-entries/v{_to}.txt",
        )
        # rename the devel changelog-entry file to the desired one
        changelog_entry = root / "docs/general/changelog-entries/vx.y.z.txt"
        changelog_entry.rename(root / f"docs/general/changelog-entries/v{_to}.txt")
        # create a new devel changelog-entry file with default text
        changelog_entry = root / "docs/general/changelog-entries/vx.y.z.txt"
        changelog_entry.write_text(RELEASE_ENTRY_TEMPLATE, encoding="utf-8")
    changelog.write_text(txt, encoding="utf-8")


def update_commit_fragments(root: Path, previous_release: str, _from: str, _to: str):
    """Update the next-release commit fragment and create the one for the release"""
    if _to == "x.y.z":
        change_type = "<Major/Minor/Bugfix>"
    else:
        try:
            _to_l = [int(i) for i in _to.split(".")]
            previous_release_l = [int(i) for i in previous_release.split(".")]
            change_type = "Bugfix"
            if _to_l[1] > previous_release_l[1]:
                change_type = "Minor"
            if _to_l[0] > previous_release_l[0]:
                change_type = "Major"
        except ValueError:
            sys.exit("unexpected version identifier")
    commit_msg_file = root / "docs/general/commit-msgs/next-release.txt"
    logging.debug("Patching %s", commit_msg_file)
    txt = commit_msg_file.read_text(encoding="utf-8")
    txt = txt.replace("<Major/Minor/Bugfix>", change_type)
    txt = txt.replace(f"foxBMS 2 (v{_from})", f"foxBMS 2 (v{_to})")
    txt = txt.replace(
        f"v{_from}/general/changelog.html",
        f"v{_to}/general/changelog.html",
    )
    if _to != "x.y.z":
        (commit_msg_file.parent / f"release-v{_to}.txt").write_text(
            txt, encoding="utf-8"
        )
    txt = """<Major/Minor/Bugfix> release of foxBMS 2 (vx.y.z)

* <add text here>

For a detailed list of changes, please refer to the documentation at
https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/docs/html/vx.y.z/general/changelog.html.
"""
    commit_msg_file.write_text(txt, encoding="utf-8")


def update_release_csv(root: Path, iso_date_today: str, _from: str, _to: str):
    """Update release.csv"""
    releases = root / "docs/general/releases.csv"
    txt = releases.read_text(encoding="utf-8")
    if _to == "x.y.z":
        txt = txt.splitlines()
        if txt[1].startswith(f"v{_to}"):
            sys.exit(f"Something went wrong. {releases} already sets 'v{_to}'.")
        txt = (
            [txt[0]]
            + [
                f"v{_to};     {MAGIC_DATE};     https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/docs/html/v{_to}/"  # pylint: disable=line-too-long
            ]
            + txt[1:]
        )
        txt = "\n".join(txt) + "\n"
    else:
        txt = txt.replace(f"v{_from};", f"v{_to};", 1)
        txt = txt.replace(f"{MAGIC_DATE};", f"{iso_date_today};", 1)
        txt = txt.replace(f"/v{_from}/", f"/v{_to}/", 1)
    releases.write_text(txt, encoding="utf-8")


def main() -> int:
    """Update the version information in all relevant files."""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v",
        "--verbosity",
        dest="verbosity",
        action="count",
        default=0,
        help="set verbosity level",
    )
    parser.add_argument(
        "--from",
        dest="from_version",
        action="store",
        default="x.y.z",
        help="from version",
    )
    parser.add_argument(
        "--to",
        dest="to_version",
        action="store",
        default=0,
        help="to version",
    )
    args = parser.parse_args()
    _from = args.from_version
    _to = args.to_version
    if not _to:
        sys.exit("Don't know to version to bump.")
    if _from == _to:
        sys.exit("--from and --to cannot be the same value")

    root = Path(get_git_root())
    previous_release = get_previous_release(root)
    if not previous_release:
        sys.exit("Could not determine previous version.")

    iso_date_today = date_get_today()
    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)

    update_c_h_files(root, iso_date_today, _from, _to)
    update_wscript(root, _from, _to)
    update_citation(root, iso_date_today, _from, _to)
    update_changelog(root, iso_date_today, _from, _to)
    update_commit_fragments(root, previous_release, _from, _to)
    update_release_csv(root, iso_date_today, _from, _to)


if __name__ == "__main__":
    main()
