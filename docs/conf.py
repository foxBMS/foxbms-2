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

"""Sphinx configuration file for the documentation"""

import csv
import hashlib
import json
import os
import re
import shutil
import sys
import time
from dataclasses import dataclass
from multiprocessing.pool import ThreadPool
from pathlib import Path
from subprocess import PIPE, Popen

import cantools.database
import sphinx_rtd_theme  # noqa: F401 pylint: disable=unused-import
from sphinx.application import Sphinx
from sphinx.config import Config

ROOT = Path(__file__).resolve().parents[1]

FILE_RE = r"\(in:([a-z_\-0-9]{1,}\.c):([A-Z]{2,5}_.*), fv:((tx)|(rx)), type:(.*)\)"
FILE_RE_COMPILED = re.compile(FILE_RE)
WAF_VERSION = "2.1.6-6a38d8c49406d2fef32d6f6600c8f033"
sys.path = [
    str(ROOT.resolve()),
    str((ROOT / "docs").resolve()),
    str((ROOT / f"tools/waf3-{WAF_VERSION}/waflib").resolve()),
    str((ROOT / f"tools/.waf3-{WAF_VERSION}/waflib").resolve()),
    str((ROOT / "tools/waf-tools").resolve()),
] + sys.path

# pylint: disable-next=wrong-import-position
from cli.foxbms_version import __version__  # noqa: E402

# pylint: disable-next=wrong-import-position
from cli.helpers.package_helpers import PACKAGE_COMMANDS  # noqa: E402

project = f"foxBMS 2 - {__version__}"
project_copyright = (
    "2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten "
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

bibtex_bibfiles = [(ROOT / "docs/references.bib").absolute()]
bibtex_default_style = "alpha"

autodoc_mock_imports = ["waflib"]

linkcheck_ignore = [
    "https://docs.foxbms.org",
    r"https:\/\/iisb-foxbms\.iisb\.fraunhofer\.de\/.*[\d|z]\/",
    # linkcheck can not handle the line highlighting
    # pylint: disable-next=line-too-long,useless-suppression
    "https://gitlab.com/ita1024/waf/-/blob/3f8bb163290eb8fbfc3b26d61dd04aa5a6a29d4a/waf-light#L6-30",
    "https://www.misra.org.uk/",
]

html_theme_options = {
    "logo_only": False,
    "prev_next_buttons_location": "bottom",
}
autosummary_generate = True

sphinx_tabs_valid_builders = ["linkcheck"]

if sys.platform.lower() == "linux":
    drawio_headless = False


# At this point, we are done with setting up Sphinx, and we implement checks
# and documentation source code generators


def log_err(msg: str) -> None:
    """Print an erorr message to stderr"""
    print(msg, file=sys.stderr)


def get_directory_hash(directory: Path) -> str:
    """Calculate a SHA256 hash for all files in a given directory (recursively).

    Args:
        directory: Path to the directory.

    Returns:
        The SHA256 hex digest representing the contents of all files.
    """
    print(f"Hashing directory: {directory}")
    hash_obj = hashlib.sha256()
    for root, _, files in os.walk(directory):
        for filename in sorted(files):
            if filename.endswith(".pyc"):
                continue
            filepath = os.path.join(root, filename)
            with open(filepath, "rb") as f:
                while chunk := f.read(8192):
                    hash_obj.update(chunk)
    return hash_obj.hexdigest()


def has_directory_changed(directory: Path, hash_file: Path) -> bool:
    """Check if the directory contents have changed since the last check.
    Compares the current hash to the stored hash in a file.

    Args:
        directory: Path to the directory.
        hash_file: Path to the file storing the previous hash.

    Returns:
        True if the directory has changed, False otherwise.
    """
    new_hash = get_directory_hash(directory)
    if hash_file.exists():
        txt = hash_file.read_text(encoding="utf-8")
        old_hash = json.loads(txt).get("hash")
        if new_hash == old_hash:
            return False
    hash_file.write_text(json.dumps({"hash": new_hash}), encoding="utf-8")
    return True


def create_version_info() -> int:
    """Create the version macro replacement."""
    with open("version_macro.txt", "w", encoding="utf-8") as f:
        f.write(f".. |version_foxbms| replace:: ``{__version__}``")
    return 0


def cleanup_autosummary() -> int:
    """Remove the autodoc created summary to force a rebuild."""
    autosummary_src = Path(__file__).parent.parent / "cli"
    autosummary_out = Path(__file__).parent / "developer-manual/fox-cli/_autosummary"
    autosummary_out.mkdir(exist_ok=True, parents=True)

    if has_directory_changed(
        autosummary_src, autosummary_out.parent / "_autosummary_hash.json"
    ):
        print("Remove existing autosummary, to rerun code generation.")
        shutil.rmtree(autosummary_out.absolute())
    else:
        print("Autosummary sources are uptodate.")
    return 0


def validate_bms_fatal_error_messages() -> int:
    """Validate that the DBC file and the source code are aligned with respect
    to the fatal errors.
    """
    # get all diagnosis IDs from the sources
    txt = (ROOT / "src/app/engine/config/diag_cfg.h").read_text(encoding="ascii")

    pattern = r"typedef enum \{.*?\} DIAG_ID_e;"
    matches = re.search(pattern, txt, re.DOTALL)
    x = matches.group(0).splitlines()

    # the index in the array corresponds to the ID as the enumeration starts
    # with 0 in C
    diag_ids = [
        i.split(",")[0].strip()
        for i in x
        if i.strip().startswith("DIAG_ID") and i.split(",")[0].strip() != "DIAG_ID_MAX"
    ]

    txt = (ROOT / "src/app/engine/config/diag_cfg.c").read_text(encoding="ascii")

    pattern = r"DIAG_ID_CFG_s diag_diagnosisIdConfiguration\[\] = \{.*?\n\};\n"
    matches = re.search(pattern, txt, re.DOTALL)
    x = matches.group(0).splitlines()
    tmp = [i.strip() for i in x if i.strip().startswith("{DIAG_ID_")]
    diag_cfg = {}
    for i in tmp:
        name, _, fatal, *_ = i.split(",")
        name = name[1:].strip()
        fatal = fatal.strip()
        if fatal == "DIAG_FATAL_ERROR":
            diag_cfg[name] = diag_ids.index(name)
    db = cantools.database.load_file(ROOT / "tools/dbc/foxbms.dbc")

    if not isinstance(db, cantools.database.can.database.Database):
        log_err("DBC file is not of type 'Database'.")
        return 1
    err = 0
    message = db.get_message_by_name("f_BmsFatalError")
    signal_name = "FatalErrorCode"
    signal = next(sig for sig in message.signals if sig.name == signal_name)

    if not signal.choices:
        log_err(f"Signal '{signal_name}' does not support choices.")
        return 1

    for name, _id in diag_cfg.items():
        if _id != signal.choice_to_number(name):
            err += 1
            log_err(
                f"Signal '{name}' and its id '{_id}' are not correctly "
                "documented in the DBC file.",
            )
    return err


def document_can_messages() -> int:
    """Remove the autodoc created summary to force a rebuild."""
    txt = """Supported CAN Messages\n~~~~~~~~~~~~~~~~~~~~~~\n"""
    with open("supported_can_messages.txt", "w", encoding="utf-8") as f:
        f.write(txt)

    db = cantools.database.load_file(ROOT / "tools/dbc/foxbms.dbc")

    if not isinstance(db, cantools.database.can.database.Database):
        log_err("DBC file is not of type 'Database'.")
        return 1
    tmp = []
    errors = 0

    # Parse messages
    for message in db.messages:
        if message.name == "f_BootloaderVersionInfo":
            continue
        if not message.comment:
            errors += 1
            log_err(
                "Could not find comment for message "
                f"'{message.name}' ({hex(message.frame_id)}) that matches "
                f"'{FILE_RE}'.",
            )
            continue

        comment = FILE_RE_COMPILED.sub("", message.comment)
        m = FILE_RE_COMPILED.search(message.comment)

        if not m:
            errors += 1
            log_err(
                "Could not find comment for message "
                f"'{message.name}' ({hex(message.frame_id)}) that matches "
                f"'{FILE_RE}'.",
            )
            continue
        message_type = m[6]
        imp_file = m[1]
        hex_id = f"{message.frame_id:0>3X}h"
        tmp.append((hex_id, message.name, comment, imp_file, message_type))
    tmp.sort(key=lambda x: x[0])

    # Categorizing the entries
    categorized_data: dict[str, list[tuple[str, str, str, str, str]]] = {}
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
    infix = ""
    if command:
        infix = "_" + command.replace(" ", "_")
    infix += "_"
    with open(f"fox{infix}help.txt", "w", encoding="utf-8") as f:
        f.write(".. code-block:: text\n\n")
        f.write("\n".join([f"   {i}" for i in out.splitlines()]))
    return 0


def gen_fox_bms_help() -> int:
    """Create bms usage file."""
    return _runner("bms")


def gen_fox_bootloader_help() -> int:
    """Create bootloader usage file."""
    return _runner("bootloader")


def gen_fox_build_help() -> int:
    """Create build usage file."""
    return _runner("waf")


def gen_fox_cli_unittest_help() -> int:
    """Create cli-unittest usage file."""
    return _runner("cli-unittest")


def gen_fox_cli_embedded_ut_help() -> int:
    """Create ceedling usage file."""
    return _runner("ceedling")


def gen_fox_com_can_help() -> int:
    """Create com-test can usage file."""
    return _runner("com-test can")


def gen_fox_com_mqtt_help() -> int:
    """Create com-test mqtt usage file."""
    return _runner("com-test mqtt")


def gen_fox_etl_help() -> int:
    """Create etl usage file."""
    return _runner("etl")


def gen_fox_etl_filter_help() -> int:
    """Create etl filter usage file."""
    return _runner("etl filter")


def gen_fox_etl_decode_help() -> int:
    """Create etl decode usage file."""
    return _runner("etl decode")


def gen_fox_etl_table_help() -> int:
    """Create etl table usage file."""
    return _runner("etl table")


def gen_fox_etl_convert_help() -> int:
    """Create etl convert usage file."""
    return _runner("etl convert")


def gen_fox_etl_convert_gamry_help() -> int:
    """Create etl convert gamry usage file."""
    return _runner("etl convert gamry")


def gen_fox_etl_convert_graphtec_help() -> int:
    """Create etl convert graphtec usage file."""
    return _runner("etl convert graphtec")


def gen_fox_gui_help() -> int:
    """Create gui usage file."""
    return _runner("gui")


def gen_generic_help() -> int:
    """Create 'fox.py' and package usage files."""
    exit_code = _runner("")
    if exit_code:
        return exit_code
    try:
        with open(ROOT / "build/docs/fox_help.txt", encoding="utf-8") as f:
            txt = f.read()
        with open("fox_cli_package_help.txt", "w", encoding="utf-8") as f:
            for line in txt.splitlines():
                if "fox.py" in line:
                    parts = line.split("fox.py")
                    line = parts[0] + "fox-cli" + line.split("fox.py")[1]
                if line.strip().split(" ")[0] in PACKAGE_COMMANDS["unsupported"]:
                    continue
                f.write(f"{line}\n")
    except FileNotFoundError:
        return 1
    return 0


def gen_fox_ide_help() -> int:
    """Create ide usage file."""
    return _runner("ide")


def gen_fox_install_help() -> int:
    """Create install usage file."""
    return _runner("install")


def gen_fox_log_help() -> int:
    """Create log usage file."""
    return _runner("log")


def gen_fox_misc_help() -> int:
    """Create misc usage file."""
    return _runner("misc")


def gen_fox_modbus_client_help() -> int:
    """Create modbus client usage file."""
    return _runner("com-test modbus client")


def gen_fox_modbus_device_help() -> int:
    """Create modbus device usage file."""
    return _runner("com-test modbus device")


def gen_fox_plot_help() -> int:
    """Create plot usage file."""
    return _runner("plot")


def gen_fox_pre_commit_help() -> int:
    """Create pre-commit usage file."""
    return _runner("pre-commit")


def gen_fox_release_help() -> int:
    """Create release usage file."""
    return _runner("release")


def gen_fox_run_program_help() -> int:
    """Create run-program usage file."""
    return _runner("run-program")


def gen_fox_run_script_help() -> int:
    """Create run-script usage file."""
    return _runner("run-script")


def gen_fox_plot() -> int:
    """Executes the plot tool to create generate the example for the documentation."""
    src_dir = ROOT / "docs/tools/fox/plot/img"
    hash_file = ROOT / "build/docs/fox_plot.json"
    hash_file.parent.mkdir(exist_ok=True, parents=True)
    if has_directory_changed(src_dir, hash_file):
        cmd = [
            sys.executable,
            ROOT / "fox.py",
            "plot",
            "--data-config",
            ROOT / "docs/tools/fox/plot/img/csv_config.yaml",
            "--plot-config",
            ROOT / "docs/tools/fox/plot/img/plot_config.yaml",
            "--output",
            ROOT / "build/docs",
            ROOT / "docs/tools/fox/plot/img/example_data.csv",
        ]

        with Popen(cmd, cwd=ROOT) as p:
            p.communicate()
        if p.returncode:
            return p.returncode
    else:
        print("fox plot examples are uptodate.")
    return 0


def gen_fox_mqtt() -> int:
    """Create mqtt usage file."""
    return _runner("mqtt")


def wrapper(func: ...) -> int:
    """Wrapper"""
    return func()


def validate_python_version_consistency() -> int:
    """Validate the documented Python version."""
    # Linux reference: python3.12
    # Windows reference: py -3.12
    err = 0

    install_summary = ROOT / "INSTALL.md"
    install_summary_txt = install_summary.read_text(encoding="utf-8")
    expected_references = 9
    if install_summary_txt.count("3.12") != expected_references:
        log_err(
            f"File '{install_summary}' does not reference the expected "
            "Python version 3.12.",
        )
        err += 1

    python_setup = ROOT / "cli/helpers/python_setup.py"
    python_setup_txt = python_setup.read_text(encoding="utf-8")
    expected_references = 5
    if python_setup_txt.count("3.12") != expected_references:
        log_err(
            f"File '{python_setup}' does not reference the expected Python version 3.12.",
        )
        err += 1

    docs = [
        (ROOT / "docs/getting-started/software-installation.rst", 10),
        (
            ROOT
            / "docs/developer-manual/update-processes/external/update-python-packages.txt",
            3,
        ),
    ]
    for i, refs in docs:
        txt = i.read_text(encoding="utf-8")
        if len(re.findall(r"((py -)|(python))3\.12", txt)) != refs:
            log_err(
                f"File '{i}' does not reference the expected Python version 3.12.",
            )
            err += 1
    return err


def validate_python_scripts() -> int:
    """Validate syntax in documentation test files."""
    err = 0
    scripts = [
        i.absolute()
        for i in [
            ROOT / "docs/developer-manual/fox-cli/test_foo.py",
            ROOT / "docs/developer-manual/style-guide/examples/python-006.py",
            ROOT / "docs/developer-manual/style-guide/examples/python-007.py",
        ]
    ]
    for script in scripts:
        cmd = [sys.executable, script]
        with Popen(cmd, cwd=ROOT, universal_newlines=True) as p:
            p.communicate()
        err += p.returncode
    return err


def validate_wrapper_invocation() -> int:
    """Validate the platform/terminal specific wrapper invocation."""
    err = 0
    any_fox_wrapper = re.compile(r"\.(\\|\/)fox\.(sh|ps1)")
    invalid_wrapper_invocation = re.compile(r"(\./fox\.ps1|\.\\fox\.sh)")

    for file_type in ("*.rst", "*.txt"):
        for file in (ROOT / "docs").rglob(file_type):
            for ln, line in enumerate(file.read_text(encoding="utf-8").splitlines()):
                if "fox." not in line:
                    continue
                if not any_fox_wrapper.search(line):
                    continue
                # 1. we have found a wrapper reference that looks like an
                #   invocation on the terminal
                # 2. make sure that .ps1 and .sh use the correct notation
                if invalid_wrapper_invocation.search(line):
                    err += 1
                    log_err(f"{file}:{ln + 1}: Invalid wrapper references.")
    return err


def validate_environment_name() -> int:
    """Validate the documented Python environment name."""
    err = 0
    docs = [
        (ROOT / "fox.ps1", 1),
        (ROOT / "fox.sh", 1),
        (ROOT / "INSTALL.md", 3),
        (ROOT / "docs/getting-started/software-installation.rst", 7),
        (ROOT / "docs/software/build-environment/build-environment.rst", 1),
    ]
    expected_env_name = "2025-11-pale-fox"
    for i, refs in docs:
        txt = i.read_text(encoding="utf-8")
        if txt.count(expected_env_name) != refs:
            log_err(f"File '{i}' does not reference the expected environment name.")
            err += 1
    return err


def validate_requirements_txt_versions() -> int:
    """Validate the versions specified in the requirements file match the
    versions in the license information file.
    """
    err = 0
    requirements: list[tuple[str, str]] = []
    for i in (ROOT / "requirements.txt").read_text(encoding="utf-8").splitlines():
        txt = i.rsplit("#")[0]
        txt = txt.rsplit(";")[0]
        pkg, pkg_version = txt.split("==", maxsplit=2)
        requirements.append((pkg.strip(), pkg_version.strip()))

    _license: list[tuple[str, str]] = []
    with open(
        ROOT / "docs/general/license-tables/external/license-info_python-packages.csv",
        encoding="utf-8",
    ) as f:
        reader = csv.reader(f, delimiter=";")
        next(reader)  # skip heading
        for line in reader:
            pkg, pkg_version = line[0], line[1]
            _license.append((pkg.strip(), pkg_version.strip()))

    for req, lic in zip(requirements, _license, strict=True):
        if req != lic:
            log_err(f"Missmatch:\n Requirements file: {req}\n license file: {lic}")
            err += 1
    return err


def add_supported_afes_to_toc() -> int:
    """Create AFE TOC"""
    afe_root = ROOT / "docs/software/modules/driver/afe"
    afes = []
    for i in afe_root.glob("*"):
        if i.is_dir():
            afes.extend(afe.relative_to(afe_root).as_posix() for afe in i.glob("*.rst"))
    afes_txt = """.. toctree::
   :maxdepth: 1
   :caption: List of supported AFEs\n\n"""
    afes_txt += "".join(f"   ./{i}\n" for i in afes)

    Path("supported_afes.txt").write_text(afes_txt, encoding="utf-8")

    return 0


@dataclass
class AppModule:
    """Store the 'module' path and the expected docs"""

    src: Path
    module: str = ""
    expected_doc: Path = Path()  # dummy value
    found: bool = False
    referenced: bool = False

    def __post_init__(self) -> None:
        """Derive relative document links."""
        infix = self.src.relative_to(ROOT / "src/app")
        self.module = self.src.name
        self.expected_doc = ROOT / f"docs/software/modules/{infix}/{self.src.name}.rst"


def validate_app_modules() -> int:
    """Validate every module has an accompanying documentation"""
    err = 0
    src_dirs = [
        i
        for i in (ROOT / ("src/app")).glob("*")
        if i.is_dir() and i.name not in (".vscode")
    ]
    modules: list[AppModule] = []
    for i in src_dirs:
        modules.extend(
            AppModule(i)
            for i in (i).glob("*")
            if i.is_dir() and i.name not in ("config", "include")
        )

    txt = (ROOT / "docs/software/modules/modules.rst").read_text(encoding="utf-8")

    for i in modules:
        if i.expected_doc.is_file():
            i.found = True
        else:
            log_err(
                f"Module '{i.module}': Implementation '{i.src}' is missing "
                f"documentation source file at '{i.expected_doc}'.",
            )
        expected_ref = (
            "./"
            + (i.expected_doc.relative_to(ROOT / "docs/software/modules/")).as_posix()
        )
        if expected_ref in txt:
            i.referenced = True
        else:
            log_err(
                f"Module '{i.module}': Expected reference '{expected_ref}' is missing "
                f"in {i.expected_doc}.",
            )
    if not all([i.found for i in modules] + [i.referenced for i in modules]):
        err = 1
    return err


def create_doc_sources(_app: Sphinx, _cfg: Config) -> None:
    """Run source generators and consistency checkers."""
    functions = [
        validate_app_modules,
        validate_python_version_consistency,
        validate_environment_name,
        validate_requirements_txt_versions,
        create_version_info,
        add_supported_afes_to_toc,
        cleanup_autosummary,
        document_can_messages,
        validate_bms_fatal_error_messages,
        gen_fox_bms_help,
        gen_fox_bootloader_help,
        gen_fox_build_help,
        gen_fox_cli_unittest_help,
        gen_fox_cli_embedded_ut_help,
        gen_fox_com_can_help,
        gen_fox_com_mqtt_help,
        gen_fox_etl_help,
        gen_fox_etl_filter_help,
        gen_fox_etl_decode_help,
        gen_fox_etl_table_help,
        gen_fox_etl_convert_help,
        gen_fox_etl_convert_gamry_help,
        gen_fox_etl_convert_graphtec_help,
        gen_fox_gui_help,
        gen_generic_help,
        gen_fox_ide_help,
        gen_fox_install_help,
        gen_fox_log_help,
        gen_fox_modbus_client_help,
        gen_fox_modbus_device_help,
        gen_fox_misc_help,
        gen_fox_plot_help,
        gen_fox_pre_commit_help,
        gen_fox_release_help,
        gen_fox_run_program_help,
        gen_fox_run_script_help,
        gen_fox_plot,
        validate_wrapper_invocation,
        validate_python_scripts,
    ]

    with ThreadPool() as pool:
        exit_codes = pool.map(wrapper, functions)

    if any(exit_codes):
        sys.exit(1)


def setup(app: Sphinx) -> None:
    """Custoimze build"""
    app.connect("config-inited", create_doc_sources)


def main() -> int:
    """For debugging purposes"""
    return validate_bms_fatal_error_messages()


if __name__ == "__main__":
    sys.exit(main())
