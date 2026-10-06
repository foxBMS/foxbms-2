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

"""Miscellaneous helper functions in the build process"""

from typing import Literal


def to_define(name: str) -> str:
    """Create a C-style macro name from a provided string.

    Every alphanumeric character is upper-cased, every other character is
    replaced by an underscore. Only ASCII input is supported: non-ASCII
    characters would either change length when upper-cased (``ß`` -> ``SS``)
    or produce identifiers that are not portable across compilers, so they
    are rejected instead of being silently mangled.

    Args:
        name: ASCII string to be converted to a macro.

    Returns:
        The macro string, containing only ``A``-``Z``, ``0``-``9`` and ``_``.

    Raises:
        TypeError: If ``name`` is not a :class:`str`.
        ValueError: If ``name`` contains one or more non-ASCII characters.

    Examples:
        >>> to_define("--ABC_12_xyz")
        '__ABC_12_XYZ'
    """
    if not isinstance(name, str):
        msg = f"name must be a str, got {type(name).__name__}"
        raise TypeError(msg)

    if not name.strip():
        msg = "Cannot translate empty string to a macro name."
        raise ValueError(msg)

    if not name.isascii():
        offenders = sorted({c for c in name if not c.isascii()})
        raise ValueError(
            "name must contain ASCII characters only, found: "
            + ", ".join(repr(c) for c in offenders)
        )

    return "".join([c if c.isalnum() else "_" for c in name.upper()])


def map_python_bool_to_c_bool(val: bool) -> Literal["true", "false"]:
    """Map Python boolean value to its C equivalent.

    Args:
        val: boolean value to be mapped  to C

    Returns:
        C boolean value as string
    """
    if val:
        return "true"
    return "false"
