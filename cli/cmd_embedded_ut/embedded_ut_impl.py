#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Implements the functionalities behind the 'ceedling' command"""

import filecmp
import logging
import pickle
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import get_args

from click import secho

from ..helpers.host_platform import PLATFORM
from ..helpers.misc import (
    PROJECT_ROOT,
    get_multiple_files_hash_str,
    terminal_link_print,
)
from ..helpers.spr import SubprocessResult, run_process
from .embedded_ut_constants import (
    CEEDLING_BINARY,
    UNIT_TEST_BUILD_DIR_APP,
    UNIT_TEST_BUILD_DIR_BL,
    EmbeddedUnitTestVariants,
)

_CEEDLING_PROJECT_FILE_SRC_APP = PROJECT_ROOT / "conf/unit/app_project_<platform>.yml"
_CEEDLING_PROJECT_FILE_SRC_BL = (
    PROJECT_ROOT / "conf/unit/bootloader_project_<platform>.yml"
)
CEEDLING_PROJECT_FILE_TGT_APP = UNIT_TEST_BUILD_DIR_APP / "project.yml"
CEEDLING_PROJECT_FILE_TGT_BL = UNIT_TEST_BUILD_DIR_BL / "project.yml"
HALCOGEN_HCG_SRC_APP = PROJECT_ROOT / "conf/hcg/app.hcg"
HALCOGEN_HCG_SRC_BL = PROJECT_ROOT / "conf/hcg/bootloader.hcg"
HALCOGEN_HCG_TGT_APP = UNIT_TEST_BUILD_DIR_APP / "app.hcg"
HALCOGEN_HCG_TGT_BL = UNIT_TEST_BUILD_DIR_BL / "bootloader.hcg"
HALCOGEN_DIL_SRC_APP = PROJECT_ROOT / "conf/hcg/app.dil"
HALCOGEN_DIL_SRC_BL = PROJECT_ROOT / "conf/hcg/bootloader.dil"
HALCOGEN_DIL_TGT_APP = UNIT_TEST_BUILD_DIR_APP / "app.dil"
HALCOGEN_DIL_TGT_BL = UNIT_TEST_BUILD_DIR_BL / "bootloader.dil"

if PLATFORM == "linux":
    CEEDLING_PROJECT_FILE_SRC_APP = Path(
        str(_CEEDLING_PROJECT_FILE_SRC_APP).replace("<platform>", "posix")
    )
    CEEDLING_PROJECT_FILE_SRC_BL = Path(
        str(_CEEDLING_PROJECT_FILE_SRC_BL).replace("<platform>", "posix")
    )

elif PLATFORM == "win32":
    CEEDLING_PROJECT_FILE_SRC_APP = Path(
        str(_CEEDLING_PROJECT_FILE_SRC_APP).replace("<platform>", "win32")
    )
    CEEDLING_PROJECT_FILE_SRC_BL = Path(
        str(_CEEDLING_PROJECT_FILE_SRC_BL).replace("<platform>", "win32")
    )
else:  # pragma: no cover
    sys.exit(f"Something went wrong in '{Path(__file__).stem}'")


@dataclass
class SourceTargetPair:
    """Source-Target"""

    src: Path
    tgt: Path


SOURCE_TARGET_PAIRS = {
    "app": [
        SourceTargetPair(CEEDLING_PROJECT_FILE_SRC_APP, CEEDLING_PROJECT_FILE_TGT_APP),
        SourceTargetPair(HALCOGEN_HCG_SRC_APP, HALCOGEN_HCG_TGT_APP),
        SourceTargetPair(HALCOGEN_DIL_SRC_APP, HALCOGEN_DIL_TGT_APP),
    ],
    "bootloader": [
        SourceTargetPair(CEEDLING_PROJECT_FILE_SRC_BL, CEEDLING_PROJECT_FILE_TGT_BL),
        SourceTargetPair(HALCOGEN_HCG_SRC_BL, HALCOGEN_HCG_TGT_BL),
        SourceTargetPair(HALCOGEN_DIL_SRC_BL, HALCOGEN_DIL_TGT_BL),
    ],
}


def _cleanup_hcg_sources(base_dir: Path) -> None:
    # this list shall be in sync with
    # - ./wscript for all unit test build variants
    # - ./src/app/hal/wscript for the target build
    # - ./src/bootloader/hal/wscript for the target build
    remove = [
        "source/HL_sys_startup.c",
        "source/HL_sys_main.c",
        "source/HL_sys_link.cmd",
    ]

    freertos_config_file = base_dir / "include/FreeRTOSConfig.h"
    if freertos_config_file.is_file():
        for line in freertos_config_file.read_text(encoding="utf-8").splitlines():
            mach = re.search(
                r"#define configCPU_CLOCK_HZ.*\( \( unsigned portLONG \) ([0-9]+) \)",
                line,
            )
            if mach:
                frequency = mach.group(1)
                break
        if not frequency:
            secho("Could not determine clock frequency.", color="red", err=True)

        # needs to be aligned with build tool defintion and the unit tests
        tmp = base_dir / "include/config_cpu_clock_hz.h"
        define_guard = tmp.name.upper().replace(".H", "_H_")
        tmp.write_text(
            f"#ifndef {define_guard}\n"
            f"#define {define_guard}\n"
            f"#define HALCOGEN_CPU_CLOCK_HZ ({frequency})\n"
            f"#endif /* {define_guard} */\n",
            encoding="utf-8",
        )

        # remove generated files we do not need (FreeRTOS (we ship our own copy))
        remove.extend(
            [
                freertos_config_file,
                base_dir / "include/FreeRTOS.h",
            ]
            + list(base_dir.glob("source/os_*.asm"))
            + list(base_dir.glob("source/os_*.c"))
            + list(base_dir.glob("include/os_*.h"))
        )

    for i in remove:
        (base_dir / i).unlink()


def make_unit_test_dir(project: EmbeddedUnitTestVariants = "app") -> None:
    """Create the unit test directory, if it does not exist"""
    if project == "app":
        base = UNIT_TEST_BUILD_DIR_APP
    elif project == "bootloader":
        base = UNIT_TEST_BUILD_DIR_BL
    else:
        sys.exit(
            "Something went wrong.\nExpect argument from list "
            f"{get_args(EmbeddedUnitTestVariants)}, but got '{project}'."
        )
    base.mkdir(parents=True, exist_ok=True)
    (base / "include").mkdir(parents=True, exist_ok=True)


def run_embedded_tests(
    args: None | list[str] = None,
    project: EmbeddedUnitTestVariants = "app",
    stderr=None,
    stdout=None,
) -> SubprocessResult:
    """Run the embedded sources tests"""

    secho(f"Testing project: '{project}'")
    secho("Running 'ceedling' directly: all arguments are passed to ceedling verbatim")

    if project == "app":
        base_dir = UNIT_TEST_BUILD_DIR_APP
        input_hcg = HALCOGEN_HCG_TGT_APP
        input_dil = HALCOGEN_DIL_SRC_APP
        output_dil = HALCOGEN_DIL_TGT_APP
    elif project == "bootloader":
        base_dir = UNIT_TEST_BUILD_DIR_BL
        input_hcg = HALCOGEN_HCG_TGT_BL
        input_dil = HALCOGEN_DIL_SRC_BL
        output_dil = HALCOGEN_DIL_TGT_BL
    else:
        sys.exit(
            "Something went wrong.\nExpect argument from list "
            f"{get_args(EmbeddedUnitTestVariants)}, but got '{project}'."
        )

    if not args:
        args = ["help"]
    logging.debug("arguments to ceedling are: %s", args)

    make_unit_test_dir(project)
    hcg_needs_to_run = False

    # update input files if needed
    for i in SOURCE_TARGET_PAIRS[project]:
        if not Path(i.tgt).is_file() or not filecmp.cmp(i.src, i.tgt):
            shutil.copyfile(i.src, i.tgt)
            hcg_needs_to_run = True
    # if only help is requested, no need to run HALCoGen:
    if args == ["help"]:
        return _run_ceedling(args, cwd=base_dir, stderr=stderr, stdout=stdout)

    # check if HALCoGen has run and the files are uptodate
    hcg_files_hash_pickle = base_dir / "manual_runner.pickle"
    if hcg_files_hash_pickle.is_file():
        with open(hcg_files_hash_pickle, "rb") as f:
            try:
                old_hcg_files_hash: str = pickle.load(f)
            except AttributeError:
                old_hcg_files_hash = ""
                hcg_needs_to_run = True
    else:
        old_hcg_files_hash = ""
        hcg_needs_to_run = True

    current_hcg_files_hash = get_multiple_files_hash_str(
        list(base_dir.glob("source/*.c")) + list(base_dir.glob("include/*.h"))
    )

    if old_hcg_files_hash != current_hcg_files_hash:
        hcg_needs_to_run = True

    halcogen = shutil.which("halcogen")
    if not halcogen:
        secho("Could not find program 'HALCogen'.", color="red", err=True)
        secho("Assuming HALCoGen sources are available...", color="yellow", err=True)

    # check if need to re-run HALCoGen
    # if any([stored_file_hashes.hcg.changed, stored_file_hashes.dil.changed]):
    if halcogen and hcg_needs_to_run:
        cmd = [halcogen, "-i", str(input_hcg)]
        cwd = base_dir
        logging.debug("running subprocess '%s' in '%s'.", cmd, cwd)
        with subprocess.Popen(cmd, cwd=cwd) as p:
            p.communicate()
        if p.returncode:
            return SubprocessResult(
                p.returncode,
                out="",
                err="Could not successfully run HALCoGen, exiting...",
            )
        _cleanup_hcg_sources(base_dir)
        current_hcg_files_hash = get_multiple_files_hash_str(
            list(base_dir.glob("source/*.c")) + list(base_dir.glob("include/*.h"))
        )

        with open(hcg_files_hash_pickle, "wb") as f:
            pickle.dump(current_hcg_files_hash, f)

        # HALCoGen alters the dil file when generating the code, therefore we
        # need copy it again...
        shutil.copyfile(input_dil, output_dil)

    # copy sources and generating files worked fine, let's run ceedling
    err = _run_ceedling(args, cwd=base_dir, stderr=stderr, stdout=stdout)

    report_link = base_dir / "artifacts/gcov/gcovr/GcovCoverageResults.html"
    if not err.returncode:
        secho(f"\nThe {project} unit tests were successful.", fg="green")
    else:
        secho(f"The {project} unit tests were not successful.", fg="red")
    if (
        report_link.is_file()
        and not err.returncode
        and any(True for i in args if i.startswith("gcov"))
    ):
        secho(f"\ncoverage report: {terminal_link_print(report_link)}", fg="green")
    return err


def _run_ceedling(
    args: list[str], cwd: Path = UNIT_TEST_BUILD_DIR_APP, stderr=None, stdout=None
) -> SubprocessResult:
    """Runs ceedling with the provided arguments"""
    ruby = shutil.which("ruby")
    if not ruby:
        secho("Could not find program 'ruby'.", fg="red", err=True)
        return SubprocessResult(1)
    cmd = [ruby, str(CEEDLING_BINARY)] + args
    if not CEEDLING_BINARY.is_file():
        secho("Could not find program 'ceedling'.", fg="red", err=True)
        return SubprocessResult(1)
    return run_process(cmd, cwd=cwd, stderr=stderr, stdout=stdout)
