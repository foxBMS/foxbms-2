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

"""Validate the foxBMS unit test configuration files.

This module provides the top-level validation entry point for the unit-test
configurations loaded by the foxBMS build system. It scans the configured test
directories for C unit test files (matching the pattern ``test_*.c``) and
aborts the build with a descriptive error message if a test file is missing
from the loaded configurations.

The validation ensures that every written unit test is compiled and executed by
cross-referencing the files in the file system with the resolved test source
nodes. This prevents scenarios where a developer adds a unit test but misses
registering it in the accompanying configuration file.

It serves as the single entry point called from waf build scripts to verify
the completeness of the test configuration before test compilation and
execution proceed.

Usage
-----

Call :func:`validate_test_configuration` during the waf build phase::

    bld.load_test_configs()
    bld.validate_test_configuration()

    # Proceed to compile and run tests only if validation passes

Dependencies
------------
- :mod:`waf_tools.config_validate_utils` - JSON file reading utilities.

"""

from waflib.Build import BuildContext
from waflib.Configure import conf


@conf
def validate_test_configuration(self: BuildContext) -> None:
    """Validate the loaded unit test configurations.

    Verifies that every C file matching the pattern ``test_*.c`` in a
    configured test directory is present in the resolved test sources.
    Aborts the build with a descriptive error message if a test file is
    missing.
    """
    # A directory can contain several JSON entries, so validate them together.
    existing_tests_by_path: dict[str, set[str]] = {}
    configured_tests_by_path: dict[str, set[str]] = {}
    config_file_by_path: dict[str, str] = {}

    # Inspect every directory containing a test.json, including configurations
    # without entries, so unreferenced test files cannot remain undetected.
    for config_node in getattr(self, "test_config_nodes", []):
        config_path = config_node.parent
        path_key = config_path.abspath()
        existing_tests_by_path[path_key] = {
            node.name for node in config_path.ant_glob("test_*.c")
        }
        configured_tests_by_path[path_key] = set()
        config_file_by_path[path_key] = config_node.path_from(self.srcnode)

    # Use all entries so platform-specific tests are validated even when they
    # are excluded from the current platform's build configuration.
    for config in getattr(self, "all_test_configs", []):
        config_path = config["path"]
        path_key = config_path.abspath()
        existing_tests_by_path.setdefault(
            path_key,
            {node.name for node in config_path.ant_glob("test_*.c")},
        )
        configured_tests_by_path.setdefault(path_key, set()).add(config["test"].name)

    for config in getattr(self, "test_configs", []):
        config_path = config["path"]
        # Use resolved source nodes so validation follows the build's actual inputs.
        configured_tests = {
            node.name
            for node in config["source"]
            if node.name.startswith("test_") and node.name.endswith(".c")
        }
        path_key = config_path.abspath()
        # Discover test files for configurations created by older loaders.
        existing_tests_by_path.setdefault(
            path_key,
            {node.name for node in config_path.ant_glob("test_*.c")},
        )
        configured_tests_by_path.setdefault(path_key, set())
        # Combine references from all configurations sharing this directory.
        configured_tests_by_path[path_key].update(configured_tests)

    for path, existing_tests in existing_tests_by_path.items():
        # Any on-disk test without a resolved source entry would be skipped by Waf.
        missing_tests = existing_tests - configured_tests_by_path[path]
        if missing_tests:
            missing_str = "\n - ".join(sorted(missing_tests))
            config_file = config_file_by_path.get(path, path)
            self.fatal(
                f"The following unit test files are not referenced in "
                f"{config_file}:\n"
                f" - {missing_str}\n"
                f"Please add them to {config_file} to ensure they are tested."
            )
