#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2022, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Template for Python scripts"""

import sys
import argparse
import os
import re
import logging
from pathlib import Path
from datetime import date


from git import Repo


def get_git_root(path: str = os.path.realpath(__file__)) -> Path:
    """helper function to find the repository root

    Args:
        path (string): path of test_f_guidelines

    Returns:
        root (string): root path of the git repository
    """
    repo = Repo(path, search_parent_directories=True)
    root = Path(repo.git.rev_parse("--show-toplevel"))
    return root


def date_get_today() -> str:
    """returns the date string for the output"""
    today = date.today()
    return today.strftime("%Y-%m-%d")


def main():  # pylint: disable=too-many-statements
    """This script does this and that"""
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
    root = Path(get_git_root())
    docs = root / "docs"
    iso_date_today = date_get_today()
    if args.verbosity == 1:
        logging.basicConfig(level=logging.INFO)
    elif args.verbosity > 1:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.ERROR)

    # *.c, *.h files
    all_c_h_files = (
        list((root / "conf").rglob("**/*.[c|h]"))
        + list((root / "docs").rglob("**/*.[c|h]"))
        + list((root / "src").rglob("**/*.[c|h]"))
        + list((root / "tests").rglob("**/*.[c|h]"))
    )
    updated = re.compile(r" \* @updated (\d{4}-\d{2}-\d{2}) \(date of last update\)")
    updated_new = f" * @updated {iso_date_today} (date of last update)"
    old_version = f" * @version v{_from}"
    new_version = f" * @version v{_to}"
    logging.debug(f"from: {old_version}")
    logging.debug(f"to: {new_version}")
    for i in all_c_h_files:
        txt = i.read_text(encoding="utf-8")
        if " * @author  foxBMS Team" in txt:
            # re-read file to ensure ascii encoding on our files
            txt = i.read_text(encoding="utf-8")
            logging.debug(f"Found foxBMS 2 file: {i}")
            if old_version in txt:
                logging.debug("...Replacing version")
                txt = txt.replace(old_version, new_version, 1)
                logging.debug("...Replacing updated")
                txt = updated.sub(updated_new, txt)
                i.write_text(txt, encoding="ascii")

    wscript = root / "wscript"
    txt = wscript.read_text(encoding="utf-8")
    txt = txt.replace(f'VERSION = "{_from}"', f'VERSION = "{_to}"')
    wscript.write_text(txt, encoding="utf-8")

    citation = root / "CITATION.cff"
    txt = citation.read_text(encoding="utf-8")
    txt = txt.replace(f'version: "{_from}"', f'version: "{_to}"')
    txt = re.sub(
        r"date-released: \d{4}-\d{2}-\d{2}", f"date-released: {iso_date_today}", txt
    )
    citation.write_text(txt, encoding="utf-8")

    changelog = docs / "general" / "changelog.rst"
    logging.debug(f"Patching {changelog}")
    changelog_regex = re.compile(
        rf"\[{_from}\][ ]-[ ]([0-9]{{4}})[-](([0-9]{{2}})|(xx))-(([0-9]{{2}})|(xx))"
    )
    logging.debug(f"Using {changelog_regex}")
    txt = changelog.read_text(encoding="utf-8")
    txt = changelog_regex.sub(
        f"[{_to}] - {iso_date_today}",
        txt,
    )
    changelog.write_text(txt, encoding="utf-8")

    releases = docs / "general" / "releases.csv"
    txt = releases.read_text(encoding="utf-8")
    txt = txt.replace(f"v{_from};", f"v{_to};", 1)
    txt = re.sub(r"\d{4}-\d{2}-\d{2}", iso_date_today, txt, 1)
    txt = txt.replace(f"/v{_from}/", f"/v{_to}/", 1)
    releases.write_text(txt, encoding="utf-8")

    software_installation = docs / "getting-started" / "software-installation.rst"
    txt = software_installation.read_text(encoding="utf-8")
    txt = txt.replace(_from, _to, 4)
    software_installation.write_text(txt, encoding="utf-8")

    macros = docs / "macros.txt"
    txt = macros.read_text(encoding="utf-8")
    txt = txt.replace(
        f".. |version_foxbms| replace:: ``{_from}``",
        f".. |version_foxbms| replace:: ``{_to}``",
        1,
    )
    macros.write_text(txt, encoding="utf-8")

    fgui = root / "tools" / "gui" / "fgui" / "__init__.py"
    txt = fgui.read_text(encoding="utf-8")
    txt = txt.replace(
        f'__version__ = "{_from}"',
        f'__version__ = "{_to}"',
        1,
    )
    fgui.write_text(txt, encoding="utf-8")


if __name__ == "__main__":
    main()
