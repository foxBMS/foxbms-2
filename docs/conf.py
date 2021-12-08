#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

import os
import sys
import time
import sphinx_rtd_theme

# pylint: skip-file

sys.path = [
    os.path.abspath("."),
    os.path.abspath("./../tools/gui"),
    os.path.abspath("./../tools/waf3-2.0.22-1241519b19b496207abef1f72bbf61c2/waflib"),
    os.path.abspath("./../tools/.waf3-2.0.22-1241519b19b496207abef1f72bbf61c2/waflib"),
    os.path.abspath("./../tools/waf-tools"),
    os.path.abspath("./../tests/scripts/waf-tools/f_guidelines"),
    os.path.abspath("./../tests/scripts/waf-tools/f_hcg"),
] + sys.path

project = "foxBMS 2"
copyright = (
    "2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten "
    "Forschung e.V. All rights reserved. See license section for further "
    "information"
)
author = "The foxBMS Team"

extensions = [
    "sphinx.ext.graphviz",
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.intersphinx",
    "sphinx.ext.napoleon",
    "sphinxcontrib.bibtex",
    "sphinxcontrib.mermaid",
    "sphinxcontrib.spelling",
]


source_suffix = ".rst"
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

spelling_lang = "en_US"
tokenizer_lang = "en_US"
spelling_word_list_filename = ["spelling_wordlist.txt"]
spelling_ignore_pypi_package_names = False
spelling_ignore_wiki_words = True
spelling_show_suggestions = True
spelling_exclude_patterns = [
    "misc/bibliography.rst",
    "general/team.rst",
    "general/team-ad-sc.rst",
    "general/team-dev.rst",
    "general/team-former.rst",
]
# disable git contributor scanning in spelling module if no git repository
try:
    import git

    try:
        _ = git.Repo(".").git_dir
        spelling_ignore_contributor_names = True
    except git.exc.InvalidGitRepositoryError:
        spelling_ignore_contributor_names = False
except ImportError:
    spelling_ignore_contributor_names = False

autodoc_mock_imports = ["waflib"]


linkcheck_ignore = [
    r".*_static\/doxygen",
    # linkcheck can not handle the line highlighting
    "https://gitlab.com/ita1024/waf/-/blob/3536dfecf8061c6d99bac338837997c4862ee89b/waflib/TaskGen.py#L495-527",
    "https://gitlab.com/ita1024/waf/-/blob/3f8bb163290eb8fbfc3b26d61dd04aa5a6a29d4a/waf-light#L6-30",
    "https://www.misra.org.uk/",
    "../_images/battery-system-setup.png",
    "../_images/battery-system-setup-pack.png",
    "../_images/battery-system-setup-bjb.png",
    "../_images/battery-system-setup-single-string.png",
]

html_theme_options = {
    "logo_only": False,
    "display_version": True,
    "prev_next_buttons_location": "bottom",
}
