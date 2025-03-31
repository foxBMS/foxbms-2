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

"""Sphinx configuration file for the documentation"""

import logging
import os
import re
import shutil
import sys
import time
from multiprocessing.pool import ThreadPool
from pathlib import Path
from subprocess import PIPE, Popen

import cantools.database
import sphinx_rtd_theme  # noqa: F401 pylint: disable=unused-import

logging.getLogger("can.pcan").setLevel(logging.CRITICAL)

# Sphinx uses lower case for all its variable/setup names
# pylint: disable=invalid-name

ROOT = Path(__file__).resolve().parents[1]

FILE_RE = r"\(in:([a-z_\-0-9]{1,}\.c):([A-Z]{2,5}_.*), fv:((tx)|(rx)), type:(.*)\)"
FILE_RE_COMPILED = re.compile(FILE_RE)

sys.path = [
    os.path.abspath("."),
    os.path.abspath(str(ROOT)),
    os.path.abspath("./../tools/waf3-2.0.22-1241519b19b496207abef1f72bbf61c2/waflib"),
    os.path.abspath("./../tools/.waf3-2.0.22-1241519b19b496207abef1f72bbf61c2/waflib"),
    os.path.abspath("./../tools/waf-tools"),
] + sys.path

# pylint: disable-next=no-name-in-module,import-error,wrong-import-position
from cli.foxbms_version import __version__  # noqa: E402

project = f"foxBMS 2 - {__version__}"
project_copyright = (
    "2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten "
    "Forschung e.V. All rights reserved. See license section for further "
    "information."
)
author = "The foxBMS Team"

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.graphviz",
    "sphinx.ext.intersphinx",
    "sphinx.ext.napoleon",
    "sphinx_tabs.tabs",
    "sphinxcontrib.bibtex",
    "sphinxcontrib.drawio",
]

source_suffix = {".rst": "restructuredtext"}
master_doc = "index"

html_favicon = "_static/favicon.ico"

version = time.ctime() + " " + time.tzname[time.daylight]
release = version

pygments_style = "sphinx"

html_theme = "sphinx_rtd_theme"

html_logo = "_static/foxbms250px.png"
html_static_path = ["_static"]
html_css_files = [
    "css/theme_overrides.css",  # override wide tables in RTD theme
]

templates_path = ["_templates"]

numfig = True

bibtex_bibfiles = ["references.bib"]
bibtex_default_style = "alpha"

autodoc_mock_imports = ["waflib"]

linkcheck_ignore = [
    "https://docs.foxbms.org",
    r"https:\/\/iisb-foxbms\.iisb\.fraunhofer\.de\/.*[\d|z]\/",
    # linkcheck can not handle the line highlighting
    "https://gitlab.com/ita1024/waf/-/blob/3536dfecf8061c6d99bac338837997c4862ee89b/waflib/TaskGen.py#L495-527",  # pylint: disable=line-too-long
    "https://gitlab.com/ita1024/waf/-/blob/3f8bb163290eb8fbfc3b26d61dd04aa5a6a29d4a/waf-light#L6-30",  # pylint: disable=line-too-long
    "https://www.misra.org.uk/",
    "../_images/battery-system-setup.png",
    "../_images/battery-system-setup-pack.png",
    "../_images/battery-system-setup-bjb.png",
    "../_images/battery-system-setup-single-string.png",
    "../_images/battery-system-precharging.png",
]

html_theme_options = {
    "logo_only": False,
    "prev_next_buttons_location": "bottom",
}
autosummary_generate = True

sphinx_tabs_valid_builders = ["linkcheck"]

if sys.platform.lower() == "linux":
    drawio_headless = False


def create_version_info(*_) -> int:
    """Create the version marcro replacement."""
    with open("version_macro.txt", "w", encoding="utf-8") as f:
        f.write(f".. |version_foxbms| replace:: ``{__version__}``")
    return 0


def cleanup_autosummary(*_) -> int:
    """Remove the autodoc created summary to force a rebuild."""
    autosummary_path = Path(__file__).parent / "tools/_autosummary"
    if autosummary_path.exists():
        shutil.rmtree(autosummary_path.absolute())
    return 0


def document_can_messages(*_) -> int:
    """Remove the autodoc created summary to force a rebuild."""
    txt = """Supported CAN Messages\n~~~~~~~~~~~~~~~~~~~~~~\n"""
    with open("supported_can_messages.txt", "w", encoding="utf-8") as f:
        f.write(txt)

    db = cantools.database.load_file(ROOT / "tools/dbc/foxbms.dbc")

    if not isinstance(db, cantools.database.can.database.Database):
        logging.error("DBC file is not of type 'Database'.")
        return 1
    tmp = []
    errors = 0

    # Parse messages
    for message in db.messages:
        comment = FILE_RE_COMPILED.sub("", message.comment)
        m = FILE_RE_COMPILED.search(message.comment)

        if not m:
            errors += 1
            logging.error(
                "Could not find comment for message '%s' (%s) that matches '%s'.",
                message.name,
                hex(message.frame_id),
                FILE_RE,
            )
            continue
        message_type = m[6]
        imp_file = m[1]
        hex_id = f"{message.frame_id:0>3X}h"
        tmp.append((hex_id, message.name, comment, imp_file, message_type))
    tmp.sort(key=lambda x: x[0])

    # Categorizing the entries
    categorized_data = {}
    for entry in tmp:
        message_type = entry[4]  # Get the Type of the CAN message
        if message_type not in categorized_data:
            # Create a new list for new categories
            categorized_data[message_type] = []
        # Append entry to the category list
        categorized_data[message_type].append(entry)

    txt = """Supported CAN Messages\n~~~~~~~~~~~~~~~~~~~~~~\n\n"""
    with open("supported_can_messages.txt", "w", encoding="utf-8") as txt_file:
        txt_file.write(txt)

    # Save data in seperate .csv files
    for message_type, entries in categorized_data.items():
        entries.sort(key=lambda x: x[0])
        title = message_type + "\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n\n"
        add_csv = (
            ".. csv-table::\n   :delim: ;\n   :file: ./../../build/docs/"
            + message_type
            + ".csv\n   :header-rows: 1\n   :widths: 40, 140, 120, 120\n\n"
        )
        with open(message_type + ".csv", "w", encoding="utf-8") as f:
            f.write("ID; Name; Comment; Implementation file\n")
            f.writelines([";".join([str(j) for j in i[:-1]]) + "\n" for i in entries])
        with open("supported_can_messages.txt", "a", encoding="utf-8") as txt_file:
            txt_file.write(title)
            txt_file.write(add_csv)
    return errors


def _runner(command: str) -> int:
    cmd = [sys.executable, "fox.py", *command.split(), "-h"]
    with Popen(cmd, cwd=ROOT, stdout=PIPE, universal_newlines=True) as p:
        out = p.communicate()[0]
    if p.returncode:
        return p.returncode

    with open(f"fox_{command.replace(' ', '_')}_help.txt", "w", encoding="utf-8") as f:
        f.write(".. code-block:: text\n\n")
        f.write("\n".join([f"   {i}" for i in out.splitlines()]))
    return 0


def gen_fox_axivion_help(*_) -> int:
    """Create axivion usage file."""
    return _runner("axivion")


def gen_fox_bootloader_help(*_) -> int:
    """Create axivion usage file."""
    return _runner("bootloader")


def gen_fox_build_help(*_) -> int:
    """Create build usage file."""
    return _runner("waf")


def gen_fox_ci_help(*_) -> int:
    """Create ci usage file."""
    return _runner("ci")


def gen_fox_cli_unittest_help(*_) -> int:
    """Create cli-unittest usage file."""
    return _runner("cli-unittest")


def gen_fox_cli_embedded_ut_help(*_) -> int:
    """Create ceedling usage file."""
    return _runner("ceedling")


def gen_fox_etl_help(*_) -> int:
    """Create etl usage file."""
    return _runner("etl")


def gen_fox_etl_filter_help(*_) -> int:
    """Create etl filter usage file."""
    return _runner("etl filter")


def gen_fox_etl_decode_help(*_) -> int:
    """Create etl decode usage file."""
    return _runner("etl decode")


def gen_fox_etl_table_help(*_) -> int:
    """Create etl table usage file."""
    return _runner("etl table")


def gen_fox_ide_help(*_) -> int:
    """Create ide usage file."""
    return _runner("ide")


def gen_fox_install_help(*_) -> int:
    """Create install usage file."""
    return _runner("install")


def gen_fox_plot_help(*_) -> int:
    """Create plot usage file."""
    return _runner("plot")


def gen_fox_log_help(*_) -> int:
    """Create log usage file."""
    return _runner("log")


def gen_fox_misc_help(*_) -> int:
    """Create misc usage file."""
    return _runner("misc")


def gen_fox_pre_commit_help(*_) -> int:
    """Create pre-commit usage file."""
    return _runner("pre-commit")


def gen_fox_release_help(*_) -> int:
    """Create release usage file."""
    return _runner("release")


def gen_fox_plot(*_) -> int:
    """Executes the plot tool to create generate the example for the documentation."""
    cmd = [
        sys.executable,
        "fox.py",
        "plot",
        "--data-config",
        "docs/tools/fox/plot/img/csv_config.yaml",
        "--plot-config",
        "docs/tools/fox/plot/img/plot_config.yaml",
        "--output",
        "build/docs",
        "docs/tools/fox/plot/img/example_data.csv",
    ]

    with Popen(cmd, cwd=ROOT) as p:
        p.communicate()
    if p.returncode:
        return p.returncode

    dirs = [
        d
        for d in (Path(__file__).parent / Path("tools/fox/plot/img")).iterdir()
        if d.is_dir()
    ]
    shutil.rmtree(dirs[0])
    return 0


def wrapper(func):
    """Wrapper"""
    return func()


def create_doc_sources(*_):
    """Run source generators"""
    functions = [
        create_version_info,
        cleanup_autosummary,
        document_can_messages,
        gen_fox_axivion_help,
        gen_fox_bootloader_help,
        gen_fox_build_help,
        gen_fox_ci_help,
        gen_fox_cli_unittest_help,
        gen_fox_cli_embedded_ut_help,
        gen_fox_etl_help,
        gen_fox_etl_filter_help,
        gen_fox_etl_decode_help,
        gen_fox_etl_table_help,
        gen_fox_ide_help,
        gen_fox_install_help,
        gen_fox_log_help,
        gen_fox_misc_help,
        gen_fox_plot_help,
        gen_fox_pre_commit_help,
        gen_fox_release_help,
        gen_fox_plot,
    ]

    with ThreadPool() as pool:
        exit_codes = pool.map(wrapper, functions)

    if any(exit_codes):
        sys.exit(1)


def setup(app):
    """Custoimze build"""
    app.connect("config-inited", create_doc_sources)


def main():
    """for debugging purposes"""
    app = 0
    config = 0
    document_can_messages(app, config)


if __name__ == "__main__":
    sys.exit(main())
