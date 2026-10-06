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

"""Open the embedded unit test report in the webbrowser."""

import webbrowser
from pathlib import Path

from ..helpers.project_context import (
    CLI_UNIT_TEST_INDEX_FILE,
    EMBEDDED_UNIT_TEST_COVERAGE_INDEX_FILE,
)

_ERROR_HTML_TEMPLATE = """\
<html>
    <body>
        <h1>{_type} unit test report not found</h1>
        <p>Expected file: {expected_file}</p>
    </body>
</html>
"""


def _worker(index_file: Path = Path("index.html"), _type: str = "unknown") -> None:
    """Open the unit test report in the webbrowser."""
    tmp = index_file
    if not index_file.is_file():
        index_file.parent.mkdir(parents=True, exist_ok=True)
        tmp = index_file.parent / "error.html"
        tmp.write_text(
            _ERROR_HTML_TEMPLATE.format(_type=_type, expected_file=index_file)
        )
    webbrowser.open(str(tmp), new=1)


def open_embedded_unit_test_report(
    index_file: Path = EMBEDDED_UNIT_TEST_COVERAGE_INDEX_FILE,
) -> None:
    """Open the embedded unit test report in the webbrowser."""
    _worker(index_file, "Embedded")


def open_cli_unit_test_report(
    index_file: Path = CLI_UNIT_TEST_INDEX_FILE,
) -> None:
    """Open the CLI unit test report in the webbrowser."""
    _worker(index_file, "CLI")
