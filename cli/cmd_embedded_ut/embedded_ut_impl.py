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

from ..helpers.ansi_colors import RED, YELLOW
from ..helpers.misc import PROJECT_ROOT, eprint, get_multiple_files_hash_str
from ..helpers.spr import SubprocessResult, run_process
from .embedded_ut_constants import CEEDLING_BINARY, UNIT_TEST_BUILD_DIR

_CEEDLING_PROJECT_FILE_SRC = PROJECT_ROOT / "conf/unit/project_<platform>.yml"
CEEDLING_PROJECT_FILE_TGT = UNIT_TEST_BUILD_DIR / "project.yml"
HALCOGEN_HCG_SRC = PROJECT_ROOT / "conf/hcg/hcg.hcg"
HALCOGEN_HCG_TGT = UNIT_TEST_BUILD_DIR / "hcg.hcg"
HALCOGEN_DIL_SRC = PROJECT_ROOT / "conf/hcg/hcg.dil"
HALCOGEN_DIL_TGT = UNIT_TEST_BUILD_DIR / "hcg.dil"

if sys.platform.lower() == "linux":
    CEEDLING_PROJECT_FILE_SRC = Path(
        str(_CEEDLING_PROJECT_FILE_SRC).replace("<platform>", "posix")
    )

elif sys.platform.lower() == "win32":
    CEEDLING_PROJECT_FILE_SRC = Path(
        str(_CEEDLING_PROJECT_FILE_SRC).replace("<platform>", "win32")
    )
else:
    CEEDLING_PROJECT_FILE_SRC = Path(
        str(_CEEDLING_PROJECT_FILE_SRC).replace("<platform>", "posix")
    )
    sys.exit(f"Something went wrong at '{Path(__file__).stem}'")


@dataclass
class SourceTargetPair:
    """Source-Target"""

    src: Path
    tgt: Path


SOURCE_TARGET_PAIRS = [
    SourceTargetPair(CEEDLING_PROJECT_FILE_SRC, CEEDLING_PROJECT_FILE_TGT),
    SourceTargetPair(HALCOGEN_HCG_SRC, HALCOGEN_HCG_TGT),
    SourceTargetPair(HALCOGEN_DIL_SRC, HALCOGEN_DIL_TGT),
]


def _cleanup_hcg_sources() -> None:
    freertos_config_file = UNIT_TEST_BUILD_DIR / "include/FreeRTOSConfig.h"

    if not freertos_config_file.is_file():
        eprint("Could not find 'FreeRTOSConfig.h'.", err=True, color=RED)

    for line in freertos_config_file.read_text(encoding="utf-8").splitlines():
        mach = re.search(
            r"#define configCPU_CLOCK_HZ.*\( \( unsigned portLONG \) ([0-9]+) \)",
            line,
        )
        if mach:
            frequency = mach.group(1)
            break
    if not frequency:
        eprint("Could not determine clock frequency.", err=True, color=RED)

    tmp = UNIT_TEST_BUILD_DIR / "include/config_cpu_clock_hz.h"
    define_guard = tmp.name.upper().replace(".H", "_H_")
    tmp.write_text(
        f"#ifndef {define_guard}\n"
        f"#define {define_guard}\n"
        f"#define HALCOGEN_CPU_CLOCK_HZ ({frequency})\n"
        f"#endif /* {define_guard} */\n",
        encoding="utf-8",
    )

    # remove generated files we do not need (FreeRTOS (we ship our own copy))
    (UNIT_TEST_BUILD_DIR / "source/HL_sys_link.cmd").unlink()
    (UNIT_TEST_BUILD_DIR / "include/FreeRTOS.h").unlink()
    (UNIT_TEST_BUILD_DIR / "include/FreeRTOSConfig.h").unlink()
    for i in UNIT_TEST_BUILD_DIR.glob("source/os_*.asm"):
        i.unlink()
    for i in UNIT_TEST_BUILD_DIR.glob("source/os_*.c"):
        i.unlink()
    for i in UNIT_TEST_BUILD_DIR.glob("include/os_*.h"):
        i.unlink()


def make_unit_test_dir() -> None:
    """Create the unit test directory,if it does not exist"""
    UNIT_TEST_BUILD_DIR.mkdir(parents=True, exist_ok=True)
    return (UNIT_TEST_BUILD_DIR / "include").mkdir(parents=True, exist_ok=True)


def run_embedded_tests(args: list[str], stderr=None, stdout=None) -> SubprocessResult:
    """Run the embedded sources tests"""
    eprint("Running 'ceedling' directly: all arguments are passed to ceedling verbatim")
    if not args:
        args = ["help"]
    logging.debug("arguments to ceedling are: %s", args)

    make_unit_test_dir()
    hcg_needs_to_run = False

    # update input files if needed
    for i in SOURCE_TARGET_PAIRS:
        if not Path(i.tgt).is_file() or not filecmp.cmp(i.src, i.tgt):
            shutil.copyfile(i.src, i.tgt)
            hcg_needs_to_run = True
    # if only help is requested, no need to run HALCoGen:
    if args == ["help"]:
        return _run_ceedling(args, stderr=stderr, stdout=stdout)

    # check if HALCoGen has run and the files are uptodate
    hcg_files_hash_pickle = UNIT_TEST_BUILD_DIR / "manual_runner.pickle"
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
        list(UNIT_TEST_BUILD_DIR.glob("source/*.c"))
        + list(UNIT_TEST_BUILD_DIR.glob("include/*.h"))
    )

    if old_hcg_files_hash != current_hcg_files_hash:
        hcg_needs_to_run = True

    halcogen = shutil.which("halcogen")
    if not halcogen:
        eprint("Could not find program 'HALCogen'.", err=True, color=RED)
        eprint("Assuming HALCoGen sources are available...", err=True, color=YELLOW)

    # check if need to re-run HALCoGen
    # if any([stored_file_hashes.hcg.changed, stored_file_hashes.dil.changed]):
    if halcogen and hcg_needs_to_run:
        cmd = [halcogen, "-i", str(HALCOGEN_HCG_TGT)]
        cwd = UNIT_TEST_BUILD_DIR
        logging.debug("running subprocess '%s' in '%s'.", cmd, cwd)
        with subprocess.Popen(cmd, cwd=cwd) as p:
            p.communicate()
        if p.returncode:
            return SubprocessResult(
                p.returncode,
                out="",
                err="Could not successfully run HALCoGen, exiting...",
            )
        _cleanup_hcg_sources()
        current_hcg_files_hash = get_multiple_files_hash_str(
            list(UNIT_TEST_BUILD_DIR.glob("source/*.c"))
            + list(UNIT_TEST_BUILD_DIR.glob("include/*.h"))
        )

        with open(hcg_files_hash_pickle, "wb") as f:
            pickle.dump(current_hcg_files_hash, f)

        # HALCoGen alters the dil file when generating the code, therefore we
        # need copy it again...
        shutil.copyfile(HALCOGEN_DIL_SRC, HALCOGEN_DIL_TGT)

    # copy sources and generating files worked fine, let's run ceedling
    err = _run_ceedling(args, stderr=stderr, stdout=stdout)
    return err


def _run_ceedling(args: list[str], stderr=None, stdout=None) -> SubprocessResult:
    """Runs ceedling with the provided arguments"""
    ruby = shutil.which("ruby")
    if not ruby:
        eprint("Could not find program 'ceedling'.", err=True, color=RED)
        return SubprocessResult(1)
    cmd = [ruby, str(CEEDLING_BINARY)] + args
    return run_process(cmd, cwd=UNIT_TEST_BUILD_DIR, stderr=stderr, stdout=stdout)
