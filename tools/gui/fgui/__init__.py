#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""sets the version package etc."""

import os
from pathlib import Path

import httplib2

__version__ = "1.5.1"
__appname__ = "foxBMS 2"
__author__ = "The foxBMS Team"
__copyright__ = "(c) 2010 - 2023 foxBMS"
__author__ = "The foxBMS Team"
__email__ = "info@foxbms.org"

MODULE_DIR: Path = Path(os.path.dirname(os.path.realpath(__file__))).resolve()
PROJECT_ROOT = (MODULE_DIR / ".." / ".." / "..").resolve()
MODULE_README_FILE: Path = (MODULE_DIR / ".." / "README.md").resolve()
FOXBMS_URL = "https://foxbms.org"
FOXBMS_DOC_BASE_URL = "https://iisb-foxbms.iisb.fraunhofer.de/"
# try the correct version
FOXBMS_DOC_URL = f"{FOXBMS_DOC_BASE_URL}/foxbms/gen2/docs/html/v{__version__}/"
html = httplib2.Http()
try:
    resp = html.request(FOXBMS_DOC_URL, "HEAD")
except TimeoutError:
    resp = []
    resp.append({"status": 401})
if int(resp[0]["status"]) >= 400:
    FOXBMS_DOC_URL = f"{FOXBMS_DOC_BASE_URL}/foxbms/gen2/docs/html/latest/"
# use latest if it correct version does not exist
FOXBMS_LICENSE_FALLBACK_URL = (
    f"{FOXBMS_DOC_BASE_URL}foxbms/gen2/docs/html/latest/general/license.html"
)
# use local doc build if we are on an development branch
if __version__ == "x.y.z":
    FOXBMS_DOC_URL_LOCAL = PROJECT_ROOT / "build" / "docs" / "index.html"
    if FOXBMS_DOC_URL_LOCAL.exists():
        FOXBMS_DOC_URL = FOXBMS_DOC_URL_LOCAL
    else:
        FOXBMS_DOC_URL = f"{FOXBMS_DOC_BASE_URL}/foxbms/gen2/docs/html/latest/"
MODULE_LICENSE_FILE: Path = (PROJECT_ROOT / "LICENSE.md").resolve()
DEFAULT_DBC_DIRECTORY_IF_IN_PROJECT_LAYOUT = (
    MODULE_DIR / ".." / ".." / "dbc"
).resolve()
if not DEFAULT_DBC_DIRECTORY_IF_IN_PROJECT_LAYOUT.is_dir():
    DEFAULT_DBC_DIRECTORY_IF_IN_PROJECT_LAYOUT = Path(os.getcwd())
DEFAULT_DBC_FILENAME_IF_IN_PROJECT_LAYOUT = "foxbms.dbc"
