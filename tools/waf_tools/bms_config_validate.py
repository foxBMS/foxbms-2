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

"""Validate the foxBMS BMS configuration file.

This module provides the top-level validation entry point for the ``bms.json``
configuration file used by the foxBMS build system. It reads the JSON
configuration, parses it into the typed
:class:`waf_tools.bms_config_model.System` data model, and aborts the build
with a descriptive error message if the configuration is invalid.

The actual validation of the individual configuration domains is performed by
the dataclasses of :mod:`~waf_tools.bms_config_model`:

    - **Application** - Aerosol sensor, current sensor, IMD, state estimation
      algorithms (SOC, SOE, SOF, SOH), balancing strategy, redundancy.
    - **RTOS** - Real-time operating system name and addons.
    - **BMS-Slave** - Analog front-end IC and temperature sensor incl.
      evaluation method.
    - **Debug** - Enabled debug interfaces.

It serves as the single entry point called from waf build scripts to obtain
a validated system configuration object before code generation and
environment setup proceed.

Usage
-----
Call :func:`validate_bms_configuration` during the waf build phase::

    bms_config_node = bld.srcnode.find_node(bld.env.BMS_CONFIG)
    system = bms_config_validate.validate_bms_configuration(bld, bms_config_node)
    bms_config_generate.set_env_variables(bld, system)

The returned :class:`waf_tools.bms_config_model.System` instance is then passed to
:func:`waf_tools.bms_config_generate.set_env_variables` to populate the build
environment and used to create ``foxbms_config`` task generators.

Dependencies
------------
- :mod:`waf_tools.bms_config_model` - Dataclass model and
  :func:`~waf_tools.bms_config_model.parse_config_to_system`.
- :mod:`waf_tools.config_validate_utils` - JSON file reading utilities.
"""

from bms_config_model import System, parse_config_to_system
from config_validate_utils import read_json_from_node
from waflib.Build import BuildContext
from waflib.Node import Node


def validate_bms_configuration(bld: BuildContext, bms_config_node: Node) -> System:
    """Validate the BMS configuration JSON file.

    Reads the JSON configuration from the provided node, parses it into a typed
    :class:`waf_tools.bms_config_model.System` instance, and aborts the build
    with a descriptive error message if validation fails.
    This function is the single entry point used by waf build scripts to obtain
    a validated system configuration.

    Args:
        bld: The active waf build context.
        bms_config_node: Node pointing to the ``bms.json`` configuration file.

    Returns:
        A validated :class:`waf_tools.bms_config_model.System` instance.

    Raises:
        SystemExit: Indirectly via ``bld.fatal()`` if the configuration is
            invalid.
    """
    bms_config = read_json_from_node(bld, bms_config_node)
    try:
        return parse_config_to_system(bms_config)
    except RuntimeError as e:
        bld.fatal(str(e))
        raise  # Unreachable, but satisfies pylint
