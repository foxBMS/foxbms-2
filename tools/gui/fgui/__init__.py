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

"""sets the version package etc."""
import os

__version__ = "1.4.0"
__appname__ = "foxBMS 2"
__author__ = "The foxBMS Team"
__copyright__ = "(c) 2010 - 2022 foxBMS"
__author__ = "The foxBMS Team"
__email__ = "info@foxbms.org"

MODULE_DIR = os.path.dirname(os.path.realpath(__file__))
MODULE_FOXBMS_LOGO = os.path.join(MODULE_DIR, "_static", "logo_foxbms.png")
MODULE_FHG_IISB_LOGO = os.path.join(MODULE_DIR, "_static", "logo_fhg_iisb.png")
MODULE_README_FILE = os.path.join(MODULE_DIR, "..", "README.md")
MODULE_LICENSE_FILE = os.path.join(MODULE_DIR, "..", "LICENSE.md")
MODULE_LOCAL_DOC_PATH = os.path.join(
    MODULE_DIR, "..", "..", "..", "build", "docs", "index.html"
)
FOXBMS_URL = "https://foxbms.org"
FOXBMS_DOC_BASE_URL = "https://iisb-foxbms.iisb.fraunhofer.de/"
FOXBMS_DOC_URL = f"{FOXBMS_DOC_BASE_URL}/foxbms/gen2/docs/html/latest/"
FOXBMS_LICENSE_FALLBACK_URL = (
    f"{FOXBMS_DOC_BASE_URL}foxbms/gen2/docs/html/latest/general/license.html"
)
