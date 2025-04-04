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

"""This script is intended to be loaded with gravis --script SCRIPT_NAME.

It replaces the architecture svg in the documentation with a new render
by calling gravis, loading the architecture description, applying the
layout that is stored under version control and exporting this view as a
replacement to the original architecture svg.
"""

import os
import sys
from pathlib import Path

try:
    from bauhaus import gravis
except ImportError:
    gravis = None

# gravis members can not be checked
# pylint: disable=no-member

if __name__ == "__main__":
    if not gravis:
        print("script has to be run as a gravis startup script")
        exit(1)  # pylint: disable=consider-using-sys-exit; (Described in Axivion docs)

    if gravis.get_script_context() != "Startup":
        gravis.warning(
            f"wrong context: {gravis.get_script_context()} "
            "(run this script with gravis --script SCRIPT_NAME)"
        )
        # just exit through sys (thus not terminating gravis according to
        # gravis documentation)
        sys.exit(1)

    # extract where the script directory is
    script_dir = os.path.dirname(gravis.argv()[0])

    # derive file names
    file_architecture = os.path.join(
        script_dir,
        "..",
        "targets",
        "app",
        "architecture.gxl",
    )
    file_layout = os.path.join(
        script_dir,
        "..",
        "targets",
        "app",
        "architecture_hierarchy_belongs_to_layout.gvl",
    )
    file_output_architecture = os.path.join(
        script_dir,
        "..",
        "..",
        "..",
        "docs",
        "software",
        "architecture",
        "img",
        "axivion_architecture.svg",
    )

    gravis.open_file(file_architecture)
    gravis.open_view("Architecture", "hierarchy")
    gravis.load_layout(file_layout)
    gravis.update()
    gravis.save_as_svg(file_output_architecture)
    print(f"exported architecture to {Path(file_output_architecture).resolve()}")
    exit(0)  # pylint: disable=consider-using-sys-exit; (Described in Axivion docs)
